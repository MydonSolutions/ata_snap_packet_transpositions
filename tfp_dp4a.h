#ifndef _TFP_DP4A_H
#define _TFP_DP4A_H

// to xGPU(DP4A)-Correlator input:
//    [Slowest ---> Fastest]
//    Time        [0 ... PIPERBLK*PKTNTIME/4]
//    Channel     [0 ... NANT*NCHAN]
//    POL         [0 ... NPOL]
//    complexity  [real, imag]
//    time_minor  [0 ... 4]
//
// The transposition copies each byte, i.e. half of each sample (8re, 8im)
// The nested forloops increment `payload_dest` to effect
//  payload_dest[pkt_chan_idx*channel_stride + pkt_time_idx*time_stride + pkt_pol_idx*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE]

#include "ata.h"
#include <string.h>
#include "packet_unpack_struct.h"

static inline void set_output_byte_strides_tfp_dp4a(
	const size_t time_per_block,
	size_t *channel_byte_stride,
	size_t *time_byte_stride
) {
	*channel_byte_stride = ATASNAP_DEFAULT_PKTNPOL*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE; // `*4` keeps databuf offset logic uniform
	*time_byte_stride = SYNTH_NANTS*SYNTH_NCHAN*(*channel_byte_stride)/4; // `/4` keeps databuf offset logic uniform
}

typedef uint8_t PKT_DCP_TFP_DP4A_T; // this is the width of the one component of the complex sample (8+8i = 16bit)/2 = 8bit

static inline void copy_packet_payload_to_tfp_dp4a(
	uint8_t*  payload_dest,/*Indexed into [FENG, PKT_SCHAN, PKTIDX, 0, 0]*/
	const uint8_t*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){ 
		for(int pkt_time_idx = 0; pkt_time_idx < ATASNAP_DEFAULT_PKTNTIME/4; pkt_time_idx++){ 
			for(int pkt_pol_idx = 0; pkt_pol_idx < ATASNAP_DEFAULT_PKTNPOL; pkt_pol_idx++){ 
				for(int time_minor = 0; time_minor < 4; time_minor++){ 
					memcpy(payload_dest, pkt_payload, sizeof(PKT_DCP_TFP_DP4A_T));
					memcpy(payload_dest+4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2, pkt_payload+ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2, sizeof(PKT_DCP_TFP_DP4A_T));
					payload_dest += ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2; /*time minor offset*/
					pkt_payload += ATASNAP_DEFAULT_SAMPLE_BYTESIZE;
				}
				payload_dest -= 4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2; /*time minor reset*/
				payload_dest += 4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE; /*pol offset*/
			}
			payload_dest -= ATASNAP_DEFAULT_PKTNPOL*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE; /*pol reset*/
			payload_dest += 4*time_stride; /*time offset*/
		}
		payload_dest -= ATASNAP_DEFAULT_PKTNTIME*time_stride; /*time reset*/
		payload_dest += channel_stride; /*chan offset*/
	}
}

static inline void copy_packet_payload_to_tfp_dp4a_direct(
	PKT_DCP_TFP_DP4A_T*  payload_dest,/*Indexed into [FENG, PKT_SCHAN, PKTIDX, 0, 0]*/
	PKT_DCP_TFP_DP4A_T*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){ 
		for(int pkt_time_idx = 0; pkt_time_idx < ATASNAP_DEFAULT_PKTNTIME/4; pkt_time_idx++){ 
			for(int pkt_pol_idx = 0; pkt_pol_idx < ATASNAP_DEFAULT_PKTNPOL; pkt_pol_idx++){ 
				for(int time_minor = 0; time_minor < 4; time_minor++){ 
					*payload_dest = *pkt_payload++;
					*(payload_dest+4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2) = *pkt_payload++;
					payload_dest++; /*time minor offset*/
				}
				payload_dest -= 4; /*time minor reset*/
				payload_dest += 4*ATASNAP_DEFAULT_PKTNPOL; /*pol offset*/
			}
			payload_dest -= ATASNAP_DEFAULT_PKTNPOL*4*ATASNAP_DEFAULT_PKTNPOL; /*pol reset*/
			payload_dest += 4*time_stride; /*time offset*/
		}
		payload_dest -= ATASNAP_DEFAULT_PKTNTIME*time_stride; /*time reset*/
		payload_dest += channel_stride; /*chan offset*/
	}
}


static packet_unpack_candidate_t tfp_dp4a_unpack_candidate = {
	"TFP_DP4A",
	copy_packet_payload_to_tfp_dp4a,
	set_output_byte_strides_tfp_dp4a
};

#endif