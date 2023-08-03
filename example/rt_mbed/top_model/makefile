embedded:
	# mbed compile --target NUCLEO_F401RE --toolchain GCC_ARM --profile ../cadmium.json --flash --sterm
	mbed compile --target NUCLEO_F446RE --toolchain GCC_ARM --profile ../cadmium.json --flash --sterm

clean-embedded:
	rm -rf ../BUILD/

all: main.o
	g++ -g -o Blinky main.o 

main.o: main_rt_model.cpp include/
	g++ -g -c -std=c++17 -I ../../../include -I include main_rt_model.cpp -o main.o

clean:
	rm -f *.o
	rm -f *.csv
	rm -f Blinky

