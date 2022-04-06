.PHONY: all clean

all: demo

demo: FLIRCamera.o main.o
	$(CXX) FLIRCamera.o main.o -L/opt/spinnaker/lib -lSpinnaker${D} -o ./bin/camera_interface

FLIRCamera.o: FLIRCamera.cpp
	$(CXX) -std=c++11 -c FLIRCamera.cpp -I/opt/spinnaker/include

main.o: main.cpp
	$(CXX) -std=c++11 -c main.cpp -I/opt/spinnaker/include

clean:
	rm -rf demo *.o
