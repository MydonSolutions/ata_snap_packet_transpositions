#ifndef _MAIN_H
#define _MAIN_H

#include "ata.h"
#include "packet_unpack_struct.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OMP_THREAD_COUNT 8

static inline size_t unpack_packet_buffer_repeatedly(
	packet_unpack_struct_t *unpack_struct,
  struct timespec ts_timeout
) {	
  size_t antenna_byte_stride, channel_byte_stride, time_byte_stride;
  struct ata_snap_ibv_pkt *p_packet = NULL;
  uint8_t* payload_dest = NULL;
  uint8_t *pkt_payload;

  (*(unpack_struct->byte_stride_func))(unpack_struct->time_per_block, &antenna_byte_stride, &channel_byte_stride, &time_byte_stride);

  struct timespec ts_now = {0};
  struct timespec ts_end = {0};
  clock_gettime(CLOCK_MONOTONIC, &ts_now);
  memcpy(&ts_end, &ts_now, sizeof(struct timespec));
  ts_end.tv_sec += ts_timeout.tv_sec;
  ts_end.tv_nsec += ts_timeout.tv_nsec;

  size_t nrounds = 0;
  do{
    #pragma omp parallel for private (\
      p_packet,\
      pkt_payload,\
      payload_dest,\
    )\
    num_threads (OMP_THREAD_COUNT)
    for (size_t i = 0; i < unpack_struct->effective_payload_per_block; i++) {
      p_packet = (struct ata_snap_ibv_pkt *) (unpack_struct->databuf_in + i*unpack_struct->databuf_packet_size);
      payload_dest = unpack_struct->databuf_out
        + p_packet->snaphdr.feng_id*antenna_byte_stride // offset for antenna
        + (p_packet->snaphdr.chan-SYNTH_SCHAN)*channel_byte_stride // offset for frequency	
        + p_packet->snaphdr.timestamp*time_byte_stride; // offset for time
      pkt_payload = (uint8_t *)p_packet->payload;
      (*(unpack_struct->copy_func))(payload_dest, pkt_payload, SYNTH_PKTNCHAN, channel_byte_stride, time_byte_stride);
    }
    nrounds ++;
    clock_gettime(CLOCK_MONOTONIC, &ts_now);

  } while (ts_now.tv_sec*1e9 + ts_now.tv_nsec < ts_end.tv_sec*1e9 + ts_end.tv_nsec);
  return nrounds;
}

static inline void unpack_packet_buffer(
	packet_unpack_struct_t *unpack_struct
) {	
  struct timespec ts_timeout = {0};
  unpack_packet_buffer_repeatedly(unpack_struct, ts_timeout);
}

#endif