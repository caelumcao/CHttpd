CXX = g++
CXXFLAGS = -std=c++11 -Wall -g
LIBS = -lpthread
TARGET = CHttpd
RM = rm -f
SRC = config.cpp main.cpp requestprotocol.cpp server.cpp socketqueue.cpp utilities.cpp
all: $(SRC)
	$(CXX) -o $(TARGET) $(SRC) $(CXXFLAGS) $(LIBS)
clean:
	$(RM) $(TARGET)
