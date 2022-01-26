GCC				= gcc
CFLAGS =  -ggdb -fopenmp -fPIC -O3 -Wall -Werror -Wno-error=cpp -fno-strict-aliasing

anew: clean run clean_build
	rm -f *.h.gch *.o

main.o: ata.h main.c main.h ftp.h tfp.h tfp_dp4a.h packet_unpack_struct.h
	$(GCC) $(CFLAGS) -c $?

main: main.o
	$(GCC) $(CFLAGS) $< -o $@

run: main
	./main

clean_build:
	rm -f *.h.gch *.o

clean: clean_build
	rm -f main