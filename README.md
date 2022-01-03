# ata_snap_packet_transpositions
Transposition playground for the ATA-SNAP packet payloads ([c:pkt_nchan, t:pkt_ntime=16, p:pkt_npol=2, s:complex{int8_t}]).

`$ make`
```
rm -f *.h.gch *.o
rm -f main
gcc -ggdb -fopenmp -fPIC -O3 -Wall -Werror -Wno-error=cpp -fno-strict-aliasing -c ata.h main.c main.h ftp.h tfp.h tfp_dp4a.h packet_unpack_struct.h
gcc -ggdb -fopenmp -fPIC -O3 -Wall -Werror -Wno-error=cpp -fno-strict-aliasing main.o -o main
./main
Padded-packet size: 8320
Padded-packet per block: 15123
Channel sections: 16
Theoretical payloads per block: 15360
Effective payloads per block: 15360
Using 100.00% of BLOCK_DATA_SIZE (125829120/125829120).
Time per block: 15360

FTP: Test Passed.
FTP: 492 blocks in 10000.000 ms (20.325 ms per block)
TFP: Test Passed.
TFP: 86 blocks in 10000.000 ms (116.279 ms per block)
TFP_DP4A: Test Passed.
TFP_DP4A: 250 blocks in 10000.000 ms (40.000 ms per block)
```
