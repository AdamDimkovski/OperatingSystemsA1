CC = g++
CFLAGS = -Wall -Werror

all: mmcopier mscopier

mmcopier: mmcopier.cpp
	$(CC) $(CFLAGS) -o mmcopier mmcopier.cpp -lpthread

mscopier: mscopier.cpp
	$(CC) $(CFLAGS) -o mscopier mscopier.cpp -lpthread

clean:
	rm -f mmcopier mscopier *.o
