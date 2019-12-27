prog: middle.o
	g++ -o prog middle.o -lpthread -lssl -lcrypto
		
middle.o: middle.cpp
	g++ -c middle.cpp
