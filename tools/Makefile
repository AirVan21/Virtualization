CC = g++
FLAGS = -std=c++11 -O2

all: ./bin/aucont_list ./bin/aucont_stop ./bin/aucont_exec ./bin/aucont_start
./bin/%: ./src/%.cpp 
	$(CC) $(FLAGS) $< -o $@

./bin/aucont_start: ./src/aucont_start.cpp ./src/input_parameters.h ./src/input_parameters.cpp
	$(CC) $(FLAGS) $< ./src/input_parameters.cpp -o $@

.PHONY: all clean

clean:
	rm -f ./bin/*