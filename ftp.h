#ifndef _FTP_H
#define _FTP_H

#include "ata.h"
#include <string.h>

// to FTP input:
//    [Slowest ---> Fastest]
//    Channel     [0 ... NANT*NCHAN]
//    Time        [0 ... PIPERBLK*PKTNTIME]
//    POL         [0 ... NPOL]
//
// The transposition takes each PKTNTIME*NPOL pols together, i.e. 16x2x (8re+8im)

static inline void set_output_byte_strides_ftp(
	const size_t time_per_block,
	size_t *time_byte_stride,
	size_t *channel_byte_stride
) {
	*time_byte_stride = ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE;
	*channel_byte_stride = time_per_block*(*time_byte_stride);
}

static inline void N_copy_packet_payload_to_ftp(
	const size_t  n_copies,
	uint8_t*  payload_dest,/*Indexed into [FENG, PKT_SCHAN, PKTIDX, 0, 0]*/
	const uint8_t*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	for(int n = 0; n < n_copies; n++){
    for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){
      memcpy(
        payload_dest + channel_stride*pkt_chan_idx,
        pkt_payload + pkt_chan_idx*ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE,
        ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE
      );
    }
	}
}

typedef struct __attribute__ ((__packed__)) {ATASNAP_DEFAULT_SAMPLE_WIDTH_T num[ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_PKTNTIME];} PKT_DCP_FTP_T; // sizeof(PKT_DCP_T) == ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE

static inline void N_copy_packet_payload_to_ftp_direct(
	const size_t  n_copies,
	PKT_DCP_FTP_T*  payload_dest,/*Indexed into [FENG, PKT_SCHAN, PKTIDX, 0, 0]*/
	PKT_DCP_FTP_T*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	for(int n = 0; n < n_copies; n++){
    for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){
      *(payload_dest) = *pkt_payload++;
      payload_dest += channel_stride;
    }
	}
}

#endif