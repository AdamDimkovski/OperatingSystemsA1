CXX = g++
CXXFLAGS = -Wall -Werror -std=c++11 

all: mmcopier mscopier

mmcopier: mmcopier.cpp
	$(CXX) $(CXXFLAGS) -o mmcopier mmcopier.cpp -lpthread

mscopier: mscopier.cpp
	$(CXX) $(CXXFLAGS) -o mscopier mscopier.cpp -lpthread

clean:
	rm -f mmcopier mscopier *.o