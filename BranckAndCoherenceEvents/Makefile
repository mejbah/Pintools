
include /home/mejbah/pintool/pin-2.10-45467-gcc.3.4.6-ia32_intel64-linux/source/tools/makefile.gnu.config
LINKER =${CXX}
#CXXFLAGS = -I$(PIN_HOME)/InstLib -fomit-frame-pointer -Wall -Werror -Wno-unknown-pragmas $(DBG) $(OPT) -MMD
CXXFLAGS = -I$(PIN_HOME)/InstLib -fomit-frame-pointer -Wall -Wno-unknown-pragmas $(DBG) $(OPT) -MMD -std=c++0x
LINK_PTHREAD=-lpthread
OPT =${COPT} -DPRINT_LOCK -g -DPRINT_FREE

CC = g++
LD = $(CC)
TARGET = perfsim.so


SRCS = $(wildcard *.cpp)
#OBJS = $(patsubst %.cpp,%.o,$(SRCS))

PINTOOL_OBJS = $(patsubst %.cpp,%.o,$(SRCS))

#OBJECTS_AS := $(patsubst %.s,%.o,$(wildcard *.S))
#OBJECTS_AS = lowlevellock.o
all: $(TARGET) CSCOPE

#$(TARGET) : $(OBJS)
#	  $(LD) -o $@ $^ $(LDFLAGS)
%.o : %.cpp 
	  $(CC) $(OPT) $(CFLAGS) ${PIN_CXXFLAGS}  -c $<

$(TARGET) : $(PINTOOL_OBJS)
	${LINKER} ${PIN_LDFLAGS} $(LINK_DEBUG) ${LINK_OUT}$@ ${PINTOOL_OBJS} ${PIN_LPATHS} ${PIN_LIBS} $(DBG) ${LINK_LIBSTDC}



CSCOPE:
	  `find -name '*.c' -o -name '*.cpp' -o -name '*.h' -name '*.hh'> cscope.files`
		  `cscope -b -q -k`

clean:
	  rm -f $(TARGET) *.o

