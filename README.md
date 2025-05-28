# ata_snap_packet_transpositions
Transposition playground for the ATA-SNAP packet payloads ([c:pkt_nchan, t:pkt_ntime=16, p:pkt_npol=2, s:complex{int8_t}]).

# Usage

A large part of the optimisations are accomplished with compilation, so many values are statically defined to this end.

Edits to the scenario can be made as follows:
- Dimension changes are localised to `SYNTH_*` and `ATASNAP_DEFAULT_PKT*` definitions in `ata.h`
- Parallelism changes are localised to the `OMP_THREAD_COUNT` definition in `main.h`

Usage is to run `$ make` which cleans, compiles and runs:

```
rm -f *.h.gch *.o
rm -f main
gcc -mssse3 -fopenmp -fPIC -O3 -Wall -Werror -Wno-error=cpp -fno-strict-aliasing -c ata.h main.c main.h ftp.h tfp.h tfp_dp4a.h packet_unpack_struct.h
gcc -mssse3 -fopenmp -fPIC -O3 -Wall -Werror -Wno-error=cpp -fno-strict-aliasing main.o -o main
./main
Padded-packet size: 4224
Padded-packet per block: 29789
Channel sections: 4
Theoretical payloads per block: 30720
Effective payloads per block: 30720
Using 100.00% of BLOCK_DATA_SIZE (125829120/125829120).
Time per block: 122880

FTP: Test Passed.
FTP: 232 blocks in 5000.000 ms (21.552 ms per block)
TFP: Test Passed.
TFP: 129 blocks in 5000.000 ms (38.760 ms per block)
TFP_DP4A: Test Passed.
TFP_DP4A: 114 blocks in 5000.000 ms (43.860 ms per block)
TFP_DP4A_DIRECT: Test Passed.
TFP_DP4A_DIRECT: 115 blocks in 5000.000 ms (43.478 ms per block)
TFP_DP4A_SSSE3: Test Passed.
TFP_DP4A_SSSE3: 346 blocks in 5000.000 ms (14.451 ms per block)
rm -f *.h.gch *.o
```

# Results

**Interestingly enough the more intensive DP4A transposition is faster than that of the TFP.**

@luigifcruz has shared some of his print-outs too, showcasing that clang compiled a comparable executable to gcc on the target AMD EPYC 7302 machines:

```
sonata@seti-node8:~/blade_test_folder/ata_snap_packet_transpositions$ sudo perf stat -B -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations ./main_gcc
Padded-packet size: 8320
Padded-packet per block: 15123
Channel sections: 16
Theoretical payloads per block: 15360
Effective payloads per block: 15360
Using 100.00% of BLOCK_DATA_SIZE (125829120/125829120).
Time per block: 15360

FTP: Test Passed.
FTP: 541 blocks in 10000.000 ms (18.484 ms per block)
TFP: Test Passed.
TFP: 82 blocks in 10000.000 ms (121.951 ms per block)
TFP_DP4A: Test Passed.
TFP_DP4A: 238 blocks in 10000.000 ms (42.017 ms per block)

 Performance counter stats for './main_gcc':

    25,196,132,026      cache-references
     2,376,136,799      cache-misses              #    9.431 % of all cache refs
    95,035,867,556      cycles
   115,460,990,064      instructions              #    1.21  insn per cycle
     5,031,062,767      branches
            61,982      faults
                 0      migrations

      30.539914329 seconds time elapsed

      30.465412000 seconds user
       0.063977000 seconds sys


(base) sonata@seti-node8:~/blade_test_folder/ata_snap_packet_transpositions$ sudo perf stat -B -e cache-references,cache-misses,cycles,instructions,branches,faults,migrations ./main_clang
Padded-packet size: 8320
Padded-packet per block: 15123
Channel sections: 16
Theoretical payloads per block: 15360
Effective payloads per block: 15360
Using 100.00% of BLOCK_DATA_SIZE (125829120/125829120).
Time per block: 15360

FTP: Test Passed.
FTP: 420 blocks in 10000.000 ms (23.810 ms per block)
TFP: Test Passed.
TFP: 83 blocks in 10000.000 ms (120.482 ms per block)
TFP_DP4A: Test Passed.
TFP_DP4A: 240 blocks in 10000.000 ms (41.667 ms per block)

 Performance counter stats for './main_clang':

    22,973,673,255      cache-references
     1,874,682,739      cache-misses              #    8.160 % of all cache refs
    94,917,248,333      cycles
   107,320,120,379      instructions              #    1.13  insn per cycle
     4,106,423,269      branches
            62,009      faults
                 1      migrations

      30.496083748 seconds time elapsed

      30.431758000 seconds user
       0.055984000 seconds sys


(base) sonata@seti-node8:~/blade_test_folder/ata_snap_packet_transpositions$ time ./main_gcc
Padded-packet size: 8320
Padded-packet per block: 15123
Channel sections: 16
Theoretical payloads per block: 15360
Effective payloads per block: 15360
Using 100.00% of BLOCK_DATA_SIZE (125829120/125829120).
Time per block: 15360

FTP: Test Passed.
time ./main_clang
FTP: 538 blocks in 10000.000 ms (18.587 ms per block)
TFP: Test Passed.
TFP: 82 blocks in 10000.000 ms (121.951 ms per block)
TFP_DP4A: Test Passed.
TFP_DP4A: 238 blocks in 10000.000 ms (42.017 ms per block)

real    0m30.567s
user    0m30.494s
sys     0m0.064s
(base) sonata@seti-node8:~/blade_test_folder/ata_snap_packet_transpositions$ time ./main_clang
Padded-packet size: 8320
Padded-packet per block: 15123
Channel sections: 16
Theoretical payloads per block: 15360
Effective payloads per block: 15360
Using 100.00% of BLOCK_DATA_SIZE (125829120/125829120).
Time per block: 15360

FTP: Test Passed.
FTP: 421 blocks in 10000.000 ms (23.753 ms per block)
TFP: Test Passed.
TFP: 83 blocks in 10000.000 ms (120.482 ms per block)
TFP_DP4A: Test Passed.
TFP_DP4A: 240 blocks in 10000.000 ms (41.667 ms per block)

real    0m30.486s
user    0m30.423s
sys     0m0.056s
```
