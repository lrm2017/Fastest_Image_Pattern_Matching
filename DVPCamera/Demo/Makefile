CC = gcc
CXX = g++

TARGET = IPConfigDemo Demo Dvp2StreamCallback

all:$(TARGET)

LIBS += -L . ../libdvp.so ../libhzd.so -lpthread -Wl,-rpath=. -std=c++11

Demo:Demo.cpp
	$(CXX) -o $@ $< $(LIBS) 

IPConfigDemo:IPConfigDemo.cpp
	$(CXX) -o $@ $< $(LIBS) 
 
Dvp2StreamCallback:Dvp2StreamCallback.cpp
	$(CXX) -o $@ $< $(LIBS)  

clean: 
	$(RM) -f Demo IPConfigDemo Dvp2StreamCallback 

  