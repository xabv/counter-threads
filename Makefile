bubble: bubble.o sem.o
	g++ -o bubble bubble.o sem.o
bubble.o: bubble.cpp sem.h
	g++ -c bubble.cpp
sem.o: sem.cpp sem.h
	g++ -c sem.cpp
