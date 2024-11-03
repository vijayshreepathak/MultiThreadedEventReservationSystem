CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall
SRC = src/main.cpp src/event_reservation.cpp  # List all your source files here
OBJ = $(SRC:.cpp=.o)
TARGET = bin/event_reservation_system

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)

