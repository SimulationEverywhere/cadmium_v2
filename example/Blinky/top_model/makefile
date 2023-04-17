embedded:
	mbed compile --target NUCLEO_F401RE --toolchain GCC_ARM --profile ../cadmium_logging_off.json --flash --sterm
#	mbed compile --target NUCLEO_F401RE --toolchain GCC_ARM --profile ../cadmium_logging_on.json --flash
#	mbed compile --target NUCLEO_F103RB --toolchain GCC_ARM --profile ../cadmium_logging_off.json
#	mbed compile --target NUCLEO_F103RB --toolchain GCC_ARM --profile ../cadmium_logging_on.json

clean-embedded:
	rm -rf ../BUILD/

all: main.o
	g++ -g -o Blinky main.o 

main.o: main_rt_model.cpp include/
	g++ -g -c -std=c++17 -I ../../cadmium_v2_refactored/include -I include main_rt_model.cpp -o main.o

clean:
	rm -f *.o
	rm -f *.csv
	rm -f Blinky

