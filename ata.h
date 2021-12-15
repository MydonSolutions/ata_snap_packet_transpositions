#ifndef _ATA_H
#define _ATA_H

#include <stdint.h>

// Packet content constants
#define ATASNAP_DEFAULT_FENCHAN         (  4096)
#define ATASNAP_DEFAULT_NCHAN           (   128)
#define ATASNAP_DEFAULT_PKTNPOL         (     2)
#define ATASNAP_DEFAULT_TIME_NBITS      (     4)
#define ATASNAP_DEFAULT_PKTNCHAN        (   256)
#define ATASNAP_DEFAULT_PKTNTIME        (    16)
#define ATASNAP_DEFAULT_PKT_SIZE        (  8208)
#define ATASNAP_DEFAULT_PKT_PER_BLK     ( 16834)
#define ATASNAP_DEFAULT_PKTIDX_PER_BLK  (262144)
#define OBS_INFO_INVALID_SCHAN          (    -1)

#define ATASNAP_DEFAULT_SAMPLE_WIDTH_T uint16_t // this is the total width of the complex sample (8+8i = 16bit)
#define ATASNAP_DEFAULT_SAMPLE_BYTESIZE sizeof(ATASNAP_DEFAULT_SAMPLE_WIDTH_T)
#define ATASNAP_DEFAULT_PKT_TIME_BYTE_STRIDE ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE // this assumes that a packet's PKTIDX (ie timestamp) field increments in steps of NTIME
#define ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE ATASNAP_DEFAULT_PKTNTIME*ATASNAP_DEFAULT_PKT_TIME_BYTE_STRIDE

// Packet Synthesis definitions
#define SYNTH_NANTS 4
#define SYNTH_NCHAN 512
#define SYNTH_NBITS 8
#define SYNTH_PKTNTIME 16
#define SYNTH_PKTNCHAN 128
#define SYNTH_NPOL 2
#define SYNTH_SCHAN 256

#define SYNTH_PACKET_PAYLOAD_BYTE_LENGTH (SYNTH_PKTNCHAN*SYNTH_PKTNTIME*SYNTH_NPOL*2*SYNTH_NBITS/8)

#endif