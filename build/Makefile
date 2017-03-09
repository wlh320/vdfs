CXX = g++
LD = ld
TARGET = ../bin
SRC = ../src
CXXFLAGS = -Wall -std=gnu++11 -O2 -fPIC -pipe -I$(SRC)

vdfs : buffer.o disk.o vdfs.o shell.o main.o
	$(CXX) buffer.o disk.o vdfs.o shell.o main.o -o $@

buffer.o : $(SRC)/buffer.cpp $(SRC)/buffer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

disk.o : $(SRC)/disk.cpp $(SRC)/disk.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

vdfs.o : $(SRC)/vdfs.cpp $(SRC)/vdfs.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

shell.o : $(SRC)/shell.cpp $(SRC)/shell.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

main.o : $(SRC)/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install:
	rm $(TARGET)/vdfs
	cp vdfs $(TARGET)/vdfs

clean: 
	rm ./*.o
	rm vdfs
