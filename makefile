prog: middle.o
	g++ -o prog middle.o -lpthread
	
middle.o: middle.cpp
	g++ -c middle.cpp
