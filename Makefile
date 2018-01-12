#
# Makefile for inifile-cpp
#

EXE := test/TestIniFile.out
OUTDIR := test
INTDIR := $(OUTDIR)/obj

VPATH := src test
CFLAGS += -g -O0 -Isrc
OBJS := $(notdir $(wildcard $(addsuffix /*.cpp, $(VPATH) ) ) )
OBJS := $(addprefix $(INTDIR)/, $(OBJS:.cpp=.o) )

#-------------------------

.PHONY : all clean

all : $(EXE)

$(EXE) : $(OBJS)
	$(CXX) -o $@ $(LFLAGS) $^

$(INTDIR)/%.o : %.cpp | $(INTDIR)
	$(CXX) -o $@  $(CFLAGS) -c $<

# Create output and intermed dirs
$(INTDIR) :
	mkdir -p $@

clean :
	rm -r $(INTDIR) $(EXE)
