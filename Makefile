#
# Makefile for inifile-cpp
#

EXE := test/TestIniFile.out

SRCs := test/TestIniFile.cpp src/IniFile.cpp
DEPs := test/catch.hpp

CFLAGS += -g -O0 -Isrc

#-------------------------

.PHONY : all clean

all : $(EXE)

$(EXE) : $(SRCs) | $(DEPs)
	$(CXX) -o $@ $(CFLAGS) $^

$(DEPs) :
	wget -O $@ https://github.com/catchorg/Catch2/releases/download/v1.12.0/catch.hpp

clean :
	rm -r $(EXE) $(DEPs)
