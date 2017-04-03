CXX = g++
LD = ld
TARGET = ../bin
SRC = ../src
CXXFLAGS = -Wall -std=gnu++11 -O2 -fPIC -pipe -I$(SRC)

vdfs : utils.o buffer.o disk.o inode.o fs.o file.o vdfs.o shell.o main.o
	$(CXX) utils.o buffer.o disk.o inode.o fs.o file.o vdfs.o shell.o main.o -o $@

utils.o : $(SRC)/utils.cpp $(SRC)/utils.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

buffer.o : $(SRC)/buffer.cpp $(SRC)/buffer.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

disk.o : $(SRC)/disk.cpp $(SRC)/disk.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

inode.o : $(SRC)/inode.cpp $(SRC)/inode.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

fs.o : $(SRC)/filesystem.cpp $(SRC)/filesystem.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

file.o : $(SRC)/file.cpp $(SRC)/file.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

vdfs.o : $(SRC)/vdfs.cpp $(SRC)/vdfs.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

shell.o : $(SRC)/shell.cpp $(SRC)/shell.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

main.o : $(SRC)/main.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY : install clean

install:
	rm $(TARGET)/vdfs
	cp vdfs $(TARGET)/vdfs

clean: 
	rm ./*.o
	rm vdfs
