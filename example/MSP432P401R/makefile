all: main.o
	g++ -g -o Blinky main.o 

main.o: main.cpp include/
	g++ -g -c -std=c++17 -I ../../../cadmium_v2/include -I include main.cpp -o main.o

clean:
	rm -f *.o
	rm -f *.csv
	rm -f Blinky

