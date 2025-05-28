#ifndef _ATA_H
#define _ATA_H

#include <stdint.h>

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

// Packet Synthesis definitions
#define BLOCK_NTIME 65536

#define SYNTH_NANTS 28
#define SYNTH_NCHAN 1
#define SYNTH_NBITS 8
#define SYNTH_PKTNTIME 512
#define SYNTH_PKTNCHAN 1
#define SYNTH_NPOL 2
#define SYNTH_SCHAN 32
#define SYNTH_OBSNCHAN (SYNTH_NANTS*SYNTH_NCHAN)
#define XGPU_NANTS 32
#define XGPU_OBSNCHAN (XGPU_NANTS*SYNTH_NCHAN)

// Packet content constants
#define ATASNAP_DEFAULT_PKTNPOL         (     2)
#define ATASNAP_DEFAULT_PKTNTIME        (   512)
#define ATASNAP_DEFAULT_PKT_SIZE        (  2064) // 1*512*2*2*8/8
#define OBS_INFO_INVALID_SCHAN          (    -1)

#define ATASNAP_DEFAULT_SAMPLE_WIDTH_T uint16_t // this is the total width of the complex sample (8+8i = 16bit)
#define ATASNAP_DEFAULT_SAMPLE_BYTESIZE sizeof(ATASNAP_DEFAULT_SAMPLE_WIDTH_T)
#define ATASNAP_DEFAULT_PKT_TIME_BYTE_STRIDE ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE // this assumes that a packet's PKTIDX (ie timestamp) field increments in steps of NTIME
#define ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE ATASNAP_DEFAULT_PKTNTIME*ATASNAP_DEFAULT_PKT_TIME_BYTE_STRIDE

#define SYNTH_PACKET_PAYLOAD_BYTE_LENGTH (SYNTH_PKTNCHAN*SYNTH_PKTNTIME*SYNTH_NPOL*2*SYNTH_NBITS/8)

struct __attribute__ ((__packed__)) link_layer_headers {
  struct ethhdr ethhdr; //0
  struct iphdr iphdr;   //14
  struct udphdr udphdr; //34
};

struct __attribute__ ((__packed__)) ata_snap_payload_header {
  uint8_t version;
  uint8_t type;
  uint16_t n_chans;
  uint16_t chan;
  uint16_t feng_id;
  uint64_t timestamp;
};

struct __attribute__ ((__packed__)) ata_snap_ibv_pkt {
  struct link_layer_headers ll_headers; // 0
  uint8_t pad0[22];     //42
  struct ata_snap_payload_header snaphdr;      //64
  uint8_t pad1[48];     //80
	uint8_t payload[];    //128
};

#endif