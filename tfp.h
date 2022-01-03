#ifndef _TFP_H
#define _TFP_H

// to TFP (xGPU-Correlator) input:
//    [Slowest ---> Fastest]
//    Time        [0 ... PIPERBLK*PKTNTIME]
//    Channel     [0 ... NANT*NCHAN]
//    POL         [0 ... NPOL]
//
// The transposition takes each NPOL pols together, i.e. 2x (8re+8im)

#include "ata.h"
#include <string.h>
#include "packet_unpack_struct.h"

static inline void set_output_byte_strides_tfp(
	const size_t time_per_block,
	size_t *channel_byte_stride,
	size_t *time_byte_stride
) {
	*channel_byte_stride = ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE;
	*time_byte_stride = SYNTH_NANTS*SYNTH_NCHAN*(*channel_byte_stride);
}

static inline void copy_packet_payload_to_tfp(
	uint8_t*  payload_dest,/*Indexed into [FENG, PKT_SCHAN, PKTIDX, 0, 0]*/
	uint8_t*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	// for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){
	// 	for(int pkt_timeXnpol_idx = 0; pkt_timeXnpol_idx < ATASNAP_DEFAULT_PKTNTIME; pkt_timeXnpol_idx++){
	// 		memcpy(
	// 			payload_dest + 
	// 				pkt_chan_idx*channel_stride + pkt_timeXnpol_idx*time_stride,
	// 			pkt_payload + (pkt_chan_idx*ATASNAP_DEFAULT_PKTNTIME + pkt_timeXnpol_idx)*ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE, 
	// 			ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE 
	// 		);
	// 	}
	// }
	for(int pkt_npol_sample_idx = 0; pkt_npol_sample_idx < pkt_nchan*ATASNAP_DEFAULT_PKTNTIME; pkt_npol_sample_idx++){ 
		memcpy(
			payload_dest +
					(pkt_npol_sample_idx/ATASNAP_DEFAULT_PKTNTIME)*channel_stride + (pkt_npol_sample_idx%ATASNAP_DEFAULT_PKTNTIME)*time_stride,
			pkt_payload + pkt_npol_sample_idx*ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE,
			ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE
		);
	}
}

typedef struct __attribute__ ((__packed__)) {ATASNAP_DEFAULT_SAMPLE_WIDTH_T num[ATASNAP_DEFAULT_PKTNPOL];} PKT_DCP_TFP_T;

static inline void copy_packet_payload_to_tfp_direct(
	PKT_DCP_TFP_T*  payload_dest,/*Indexed into [FENG, PKT_SCHAN, PKTIDX, 0, 0]*/
	PKT_DCP_TFP_T*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){
		for(int pkt_timeXnpol_idx = 0; pkt_timeXnpol_idx < ATASNAP_DEFAULT_PKTNTIME; pkt_timeXnpol_idx++){
			*(payload_dest + pkt_timeXnpol_idx*time_stride) = *pkt_payload++;
		}
		payload_dest += channel_stride;
	}
}

static packet_unpack_candidate_t tfp_unpack_candidate = {
	"TFP",
	copy_packet_payload_to_tfp,
	set_output_byte_strides_tfp
};

#endif