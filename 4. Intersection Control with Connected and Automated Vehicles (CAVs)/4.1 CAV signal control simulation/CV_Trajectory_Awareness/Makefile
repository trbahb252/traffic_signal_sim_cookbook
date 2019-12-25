SRC = 
SRC+=CV_Trajectory_Awareness.cpp GeoCoord.cpp NMAP.cpp AccelerationSet4Way.cpp BasicVehicle.cpp Brakes.cpp PositionLocal3D.cpp Motion.cpp

TGT:=CV_Trajectory_Awareness
TOOLCHAIN=../../../usr

CC=$(TOOLCHAIN)/bin/g++
CFLAGS:= -Wall -DPRINT_LOG
INCLUDES:=-I. -I$(TOOLCHAIN)/include -I$(TOOLCHAIN)/usr/include 
LIBS = -L$(TOOLCHAIN)/lib -L$(TOOLCHAIN)/local/lib -lstdc++ -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

LIBS +=
all:	$(TGT)
$(TGT): $(OBJ)
	${CC}  $(OBJ) -o $(TGT) $(LIBS)

%.o: %.cpp
	${CC}  $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(TGT) $(OBJ) *~ *.o
