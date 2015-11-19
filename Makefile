
CC = g++
LD = $(CC)
TARGET = cache_sim


SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))

#OBJECTS_AS := $(patsubst %.s,%.o,$(wildcard *.S))
#OBJECTS_AS = lowlevellock.o
all: $(TARGET) CSCOPE

$(TARGET) : $(OBJS)
	  $(LD) -o $@ $^ $(LDFLAGS)
%.o : %.cpp 
	  $(CC) $(CFLAGS)  -c $<

CSCOPE:
	  `find -name '*.c' -o -name '*.cpp' -o -name '*.h' -name '*.hh'> cscope.files`
		  `cscope -b -q -k`

clean:
	  rm -f $(TARGET) *.o

