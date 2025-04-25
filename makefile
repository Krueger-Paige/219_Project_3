here: main.o hello.o
	g++ -o here main.o hello.o

main.o: main.cpp hello.h
	g++ -c main.cpp

hello.o: hello.h hello.cpp
	g++ -c hello.cpp

clean:
	rm *.o here