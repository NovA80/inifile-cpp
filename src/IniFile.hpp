/*
 * IniFile.hpp
 *
 * Created on: 26 Dec 2015
 *     Author: Fabian Meyer
 *   Modified: Andrey Novikov, 02 Feb 2018
 *    License: MIT
 */

#ifndef INIFILE_HPP_
#define INIFILE_HPP_

#include <map>

#include <sstream>
#include <stdexcept>
#include <fstream>
#include <iostream>

namespace ini
{
	/************************************
	 *          IniField
	 ************************************/
	class IniField
	{
	private:
		std::string value_;
		std::string comment_;

	public:
		IniField(){ ; }
		IniField(const std::string &value) : value_(value), comment_("") { ; }
		IniField(const IniField &f) : value_(f.value_), comment_(f.comment_) { ; }
		~IniField(){ ; }

		const std::string &asString() const {
			return value_;
		}

		int asInt() const {
			int n;
			if( ! (std::istringstream(value_) >> n) )
				throw std::domain_error("field is not an int");
			return n;
		}

		double asDouble() const {
			double x;
			if( ! (std::istringstream(value_) >> x) )
				throw std::domain_error("field is not a double");
			return x;
		}

		bool asBool() const {
			#ifdef WIN32
				#define  strcasecmp  _stricmp
			#endif

			const char* TRUE[4]  = { "true", "yes", "y", "1" };
			const char* FALSE[4] = { "false", "no", "n", "0" };
			for( short i = 0; i < 4; ++i ){
				if( strcasecmp(value_.c_str(), TRUE[i]) == 0 )
					return true;
				if( strcasecmp(value_.c_str(), FALSE[i]) == 0 )
					return false;
			}
			throw std::domain_error("field is not a bool");
		}

		const std::string& comment() const {
			return comment_;
		}

		IniField &operator=(const IniField &field) {
			value_ = field.value_;
			comment_ = field.comment_;
			return *this;
		}

		IniField &operator=(const int value) {
			std::stringstream ss;   ss << value;
			value_ = ss.str();
			return *this;
		}

		IniField &operator=(const double value) {
			std::stringstream ss;   ss << value;
			value_ = ss.str();
			return *this;
		}

		IniField &operator=(const bool value){
			value_ = (value) ? "true" : "false";
			return *this;
		}

		IniField &operator=(const std::string &value){
			value_ = value;
			return *this;
		}

		void comment(const std::string& comment){
			comment_ = comment;
			// cut last '\n'
			const std::size_t n = comment.length() - 1;
			if( comment[n] == '\n' )
				comment_.resize(n);
		}
	};

	/************************************
	 *          IniSection
	 ************************************/
	class IniSection : public std::map<std::string, IniField>
	{
	private:
		std::string comment_;

	public:
		IniSection() { ; }
		~IniSection() { ; }

		const std::string& comment() const {
			return comment_;
		}
		void comment(const std::string& comment) {
			comment_ = comment;
			// cut last '\n'
			const std::size_t n = comment.length() - 1;
			if( comment[n] == '\n' )
				comment_.resize(n);
		}

		bool hasField(const std::string &key) const {
			return (find(key) != end());
		}
	};


	/************************************
	 *          IniFile
	 ************************************/

	class IniFile: public std::map<std::string, IniSection>
	{
	private:
		char fieldSep_;
		char commentChar_;

	public:
		IniFile(const char fieldSep = '=', const char comment = ';')
			: fieldSep_(fieldSep), commentChar_(comment) {
		}
		IniFile(const std::string &fileName, const char fieldSep = '=', const char comment = ';')
			: fieldSep_(fieldSep), commentChar_(comment) {
			load(fileName);
		}

		IniFile(std::istream &is, const char fieldSep = '=', const char comment = ';')
			: fieldSep_(fieldSep), commentChar_(comment) {
			decode(is);
		}

		~IniFile(){ ; }

		bool hasSection(const std::string &sec) const {
			return ( find(sec) != end() );
		}

		void decode(std::istream &is) {
			clear();
			int lineNo = 0;
			IniSection *currentSection = NULL;
			std::string currentComment;
			// iterate file by line
			while(!is.eof() && !is.fail()) {
				std::string line;
				std::getline(is, line, '\n');
				++lineNo;

				// skip if line is empty
				if(line.size() == 0)
					continue;
				// if line is a comment append to comment lines
				if(line[0] == commentChar_) {
					currentComment += line.substr(1, std::string::npos) + "\n";
				} else if(line[0] == '[') {
					// line is a section
					// check if the section is also closed on same line
					std::size_t pos = line.find("]");
					if(pos == std::string::npos) {
						std::stringstream ss;
						ss << "l" << lineNo
						   << ": ini parsing failed, section not closed";
						throw std::logic_error(ss.str());
					}
					// check if the section name is empty
					if(pos == 1) {
						std::stringstream ss;
						ss << "l" << lineNo
						   << ": ini parsing failed, section name is empty";
						throw std::logic_error(ss.str());
					}
					// check if there is a newline following closing bracket
					if(pos + 1 != line.length()) {
						std::stringstream ss;
						ss << "l" << lineNo
						   << ": ini parsing failed, no end of line after section";
						throw std::logic_error(ss.str());
					}

					// retrieve section name
					std::string secName = line.substr(1, pos - 1);
					currentSection = &((*this)[secName]);
					if( ! currentComment.empty() ) {
						currentSection->comment(currentComment);
						currentComment.clear();
					}
				} else {
					// line is a field definition
					// check if section was already opened
					if(currentSection == NULL) {
						// field has no section -> create one with empty name
						currentSection = &((*this)[""]);
					}

					// find key value separator
					std::size_t pos = line.find(fieldSep_);
					if(pos == std::string::npos) {
						std::stringstream ss;
						ss << "l" << lineNo << ": ini parsing failed, no '=' found";
						throw std::logic_error(ss.str());
					}
					// retrieve field name and value
					std::string name = line.substr(0, pos);
					std::string value = line.substr(pos + 1, std::string::npos);

					IniField& field = (*currentSection)[name];
					field = value;
					if( ! currentComment.empty() ) {
						field.comment(currentComment);
						currentComment.clear();
					}
				}
			}
		}

		void decode(const std::string &content) {
			std::istringstream ss(content);
			decode(ss);
		}

		void encode(std::ostream &os)
		{
			IniFile::iterator it;
			// iterate through all sections in this file
			for(it = this->begin(); it != this->end(); ++it) {
				if( ! it->second.comment().empty() )
					os << ";" << it->second.comment() << std::endl;
				if( ! it->first.empty() )
					os << "[" << it->first << "]" << std::endl;

				IniSection::iterator secIt;
				// iterate through all fields in the section
				for(secIt = it->second.begin(); secIt != it->second.end(); ++secIt) {
					const std::string& comment = secIt->second.comment();
					if( ! comment.empty() )
						os << ";" << comment << std::endl;

					os << secIt->first << fieldSep_ << secIt->second.asString()
					   << std::endl;
				}
			}
		}

		std::string encode(){
			std::ostringstream ss;
			encode(ss);
			return ss.str();
		}

		void load(const std::string &fileName) {
			std::ifstream is(fileName.c_str());
			decode(is);
		}

		void save(const std::string &fileName) {
			std::ofstream os(fileName.c_str());
			encode(os);
		}
	};
}

#endif
