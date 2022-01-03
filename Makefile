GCC				= gcc
CFLAGS =  -ggdb -fopenmp -fPIC -O3 -Wall -Werror -Wno-error=cpp -fno-strict-aliasing

anew: clean run

main.o: ata.h main.c main.h ftp.h tfp.h tfp_dp4a.h packet_unpack_struct.h
	$(GCC) $(CFLAGS) -c $?

main: main.o
	$(GCC) $(CFLAGS) $< -o $@

clean:
	rm -f *.h.gch *.o main

run: main
	./main