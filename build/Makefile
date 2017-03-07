CC = g++
TARGET = ../bin
SRC = ../src
CXXFLAGS = -Wall -O2

vdfs : buffer.o disk.o vdfs.o shell.o main.o
	$(CC) $(CXXFLAGS) buffer.o disk.o vdfs.o shell.o main.o -o $@

buffer.o : $(SRC)/buffer.cpp $(SRC)/buffer.h
	$(CC) $(CXXFLAGS) -c $< -o $@

disk.o : $(SRC)/disk.cpp $(SRC)/disk.h
	$(CC) $(CXXFLAGS) -c $< -o $@

vdfs.o : $(SRC)/vdfs.cpp $(SRC)/vdfs.h
	$(CC) $(CXXFLAGS) -c $< -o $@

shell.o : $(SRC)/shell.cpp $(SRC)/shell.h
	$(CC) $(CXXFLAGS) -c $< -o $@

main.o : $(SRC)/main.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@

deploy: vdfs
	rm $(TARGET)/vdfs
	cp vdfs $(TARGET)/vdfs

clean: 
	rm ./*.o
	rm vdfs