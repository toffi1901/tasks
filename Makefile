CXX = g++
CXXFLAGS = -std=c++17 -Wall -g

task10: task10.o
	$(CXX) task10.o -o task10

task10.o: task10.cpp
	$(CXX) $(CXXFLAGS) -c task10.cpp

clean:
	rm -rf *.o task10