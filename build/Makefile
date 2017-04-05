CXX = g++
RM = rm
SRC = ../src
CXXFLAGS = -Wall -std=gnu++11 -O2 -fPIC -pipe -I$(SRC)
SRCS = $(wildcard $(SRC)/*.cpp)
OBJS = $(patsubst %cpp,%o,$(notdir $(SRCS)))
TARGET = vdfs

$(TARGET) : $(OBJS)
	$(CXX) $(OBJS) -o $@

$(OBJS) : %.o : $(SRC)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

.PHONY : all clean

all : $(TARGET)

clean: 
	$(RM) *.o $(TARGET)
