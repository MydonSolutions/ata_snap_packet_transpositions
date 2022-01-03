#ifndef _MAIN_H
#define _MAIN_H

#include "ata.h"
#include "packet_unpack_struct.h"

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

static inline void unpack_packet_buffer_repeatedly(
	packet_unpack_struct_t *unpack_struct,
  uint32_t nrounds
) {	
  size_t channel_byte_stride, time_byte_stride;
  struct ata_snap_ibv_pkt *p_packet = NULL;
  uint8_t* payload_dest = NULL;
  uint8_t *pkt_payload;

  (*(unpack_struct->byte_stride_func))(unpack_struct->time_per_block, &channel_byte_stride, &time_byte_stride);

  for (size_t n = 0; n < nrounds; n++) {
    for (size_t i = 0; i < unpack_struct->effective_payload_per_block; i++) {
      p_packet = (struct ata_snap_ibv_pkt *) (unpack_struct->databuf_in + i*unpack_struct->databuf_packet_size);
      payload_dest = unpack_struct->databuf_out
        + p_packet->snaphdr.timestamp*time_byte_stride // offset for time
        + (p_packet->snaphdr.feng_id*SYNTH_NCHAN + (p_packet->snaphdr.chan-SYNTH_SCHAN))*channel_byte_stride; // offset for frequency	
      pkt_payload = (uint8_t *)p_packet->payload;
      (*(unpack_struct->copy_func))(payload_dest, pkt_payload, SYNTH_PKTNCHAN, channel_byte_stride, time_byte_stride);
    }
  }
}

static inline void unpack_packet_buffer(
	packet_unpack_struct_t *unpack_struct
) {	
  unpack_packet_buffer_repeatedly(unpack_struct, 1);
}

#endif