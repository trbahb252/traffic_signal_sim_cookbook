SRC = 
SRC+=TrafficControllerInterface.cpp Signal.cpp Array.cpp Config.cpp

TGT:=TrafficControllerInterface
TOOLCHAIN=../../../usr

CC=$(TOOLCHAIN)/bin/g++
CFLAGS:= -Wall -DPRINT_LOG
INCLUDES:=-I. -I$(TOOLCHAIN)/include -I$(TOOLCHAIN)/local/include 
LIBS = -L$(TOOLCHAIN)/lib -L$(TOOLCHAIN)/local/lib -lstdc++ -lm -lnetsnmp 

OBJ = $(patsubst %.cpp, %.o, ${SRC})

LIBS +=
all:	$(TGT)
$(TGT): $(OBJ)
	${CC}  $(OBJ) -o $(TGT) $(LIBS)

%.o: %.cpp
	${CC}  $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(TGT) $(OBJ) *~ *.o
