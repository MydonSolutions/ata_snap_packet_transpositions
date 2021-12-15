#ifndef _MAIN_H
#define _MAIN_H

#include "ata.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

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