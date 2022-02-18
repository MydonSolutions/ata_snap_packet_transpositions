#ifndef _TFP_DP4A_H
#define _TFP_DP4A_H

// to xGPU(DP4A)-Correlator input:
//    [Slowest ---> Fastest]
//    Time        [0 ... PIPERBLK*PKTNTIME/4]
//    Channel     [0 ... NCHAN]
//    Antenna  		[0 ... NANT]
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
	size_t *antenna_byte_stride,
	size_t *channel_byte_stride,
	size_t *time_byte_stride
) {
	*antenna_byte_stride = ATASNAP_DEFAULT_PKTNPOL*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE; // `*4` keeps databuf offset logic uniform
	*channel_byte_stride = XGPU_NANTS*(*antenna_byte_stride); // `*4` keeps databuf offset logic uniform
	*time_byte_stride = SYNTH_NCHAN*(*channel_byte_stride)/4; // `/4` keeps databuf offset logic uniform
}

// this is the width of the one component of the complex sample (8+8i = 16bit)/2 = 8bit
#define PKT_DCP_TFP_DP4A_T uint8_t

static inline void copy_packet_payload_to_tfp_dp4a(
	uint8_t*  payload_dest,/*Indexed into [PKTIDX, PKT_SCHAN, FENG, 0, 0]*/
	uint8_t*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	//! better correct
	// for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){ 
	// 	for(int pkt_time_idx = 0; pkt_time_idx < ATASNAP_DEFAULT_PKTNTIME; pkt_time_idx++){ 
	// 		for(int pkt_pol_idx = 0; pkt_pol_idx < ATASNAP_DEFAULT_PKTNPOL; pkt_pol_idx++){ 
	// 			for(int c = 0; c < 2; c++){ 
	// 				memcpy(payload_dest +
	// 					(pkt_time_idx/4 * time_stride*4) +
	// 					pkt_chan_idx * channel_stride +
	// 					pkt_pol_idx*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE +
	// 					c*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2 +
	// 					(pkt_time_idx%4)*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2,

	// 					pkt_payload + 
	// 					pkt_chan_idx*ATASNAP_DEFAULT_PKTNTIME*ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE +
	// 					pkt_time_idx*ATASNAP_DEFAULT_PKTNPOL*ATASNAP_DEFAULT_SAMPLE_BYTESIZE+
	// 					pkt_pol_idx*ATASNAP_DEFAULT_SAMPLE_BYTESIZE +
	// 					c*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2,
	// 					sizeof(PKT_DCP_TFP_DP4A_T));
	// 			}
	// 		}
	// 	}
	// }
	for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){
		for(int pkt_time_major_idx = 0; pkt_time_major_idx < ATASNAP_DEFAULT_PKTNTIME/4; pkt_time_major_idx++){
			for(int pkt_time_minor_idx = 0; pkt_time_minor_idx < 4; pkt_time_minor_idx++){
				for(int pkt_pol_idx = 0; pkt_pol_idx < ATASNAP_DEFAULT_PKTNPOL; pkt_pol_idx++){
					for(int c = 0; c < 2; c++){
						memcpy(payload_dest +
							(pkt_time_major_idx * time_stride*4) +
							pkt_chan_idx * channel_stride +
							pkt_pol_idx*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE +
							c*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2 +
							pkt_time_minor_idx*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2,
							
							pkt_payload++,
							sizeof(PKT_DCP_TFP_DP4A_T));
					}
				}
			}
		}
	}
}

static inline void copy_packet_payload_to_tfp_dp4a_direct(
	PKT_DCP_TFP_DP4A_T*  payload_dest,/*Indexed into [PKTIDX, PKT_SCHAN, FENG, 0, 0]*/
	PKT_DCP_TFP_DP4A_T*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){ 
		for(int pkt_time_major_idx = 0; pkt_time_major_idx < ATASNAP_DEFAULT_PKTNTIME/4; pkt_time_major_idx++){ 
			for(int pkt_time_minor_idx = 0; pkt_time_minor_idx < 4; pkt_time_minor_idx++){ 
				for(int pkt_pol_idx = 0; pkt_pol_idx < ATASNAP_DEFAULT_PKTNPOL; pkt_pol_idx++){ 
					for(int c = 0; c < 2; c++){ 
						payload_dest[
							(pkt_time_major_idx * time_stride*4) +
							pkt_chan_idx * channel_stride +
							pkt_pol_idx*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE +
							c*4*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2 +
							pkt_time_minor_idx*ATASNAP_DEFAULT_SAMPLE_BYTESIZE/2
						] = *pkt_payload++;
					}
				}
			}
		}
	}
}

#ifdef __SSSE3__
#include <tmmintrin.h>
// from
//    TIME        [0 ... 4]
//    POL         [0 ... NPOL]
//    complexity  [real, imag]
// C0T0P0R C0T0P0I C0T0P1R C0T0P1I			000:031
// C0T1P0R C0T1P0I C0T1P1R C0T1P1I			032:063
// C0T2P0R C0T2P0I C0T2P1R C0T2P1I			064:095
// C0T3P0R C0T3P0I C0T3P1R C0T3P1I			096:127
// 	00 01 02 03
// 	04 05 06 07
// 	08 09 10 11
// 	12 13 14 15
//
// to DP4A
//    POL         [0 ... NPOL]
//    complexity  [real, imag]
//    time_minor  [0 ... 4]
// C0T0P0R C0T1P0R C0T2P0R C0T1P0R			000:031
// C0T0P0I C0T1P0I C0T2P0I C0T1P0I			032:063
// C0T0P1R C0T1P1R C0T2P1R C0T1P1R			064:095
// C0T0P1I C0T1P1I C0T2P1I C0T1P1I			096:127
// 	00 04 08 12
// 	01 05 09 13
// 	02 06 10 14
// 	03 07 11 15
static inline void copy_packet_payload_to_tfp_dp4a_ssse3(
	uint8_t*  payload_dest,/*Indexed into [PKTIDX, PKT_SCHAN, FENG, 0, 0]*/
	uint8_t*  pkt_payload,
	const uint16_t  pkt_nchan,
	const uint32_t  channel_stride, /*= PIPERBLK*ATASNAP_DEFAULT_PKTIDX_STRIDE/ATASNAP_DEFAULT_PKT_CHAN_BYTE_STRIDE */
	const uint32_t  time_stride /* Unused as copy strides TIME*POLE */
) {
	const __m128i corner_turn_shuffle_mask = _mm_setr_epi8(
		0, 4,  8, 12,
		1, 5,  9, 13,
		2, 6, 10, 14,
		3, 7, 11, 15
	);

	__m128i* v_payload_dest = (__m128i*) payload_dest;
	__m128i* v_pkt_payload = (__m128i*) pkt_payload;

	for(int pkt_chan_idx = 0; pkt_chan_idx < pkt_nchan; pkt_chan_idx++){ 
		for(int pkt_time_major_idx = 0; pkt_time_major_idx < ATASNAP_DEFAULT_PKTNTIME/4; pkt_time_major_idx++){
			v_payload_dest[(
				(pkt_time_major_idx * time_stride*4) +
				pkt_chan_idx * channel_stride
			)/16] = _mm_shuffle_epi8(*v_pkt_payload++, corner_turn_shuffle_mask);
		}
	}
}

static packet_unpack_candidate_t tfp_dp4a_ssse3_unpack_candidate = {
	"TFP_DP4A_SSSE3",
	copy_packet_payload_to_tfp_dp4a_ssse3,
	set_output_byte_strides_tfp_dp4a
};
#endif // __SSSE3__ support

static packet_unpack_candidate_t tfp_dp4a_unpack_candidate = {
	"TFP_DP4A",
	copy_packet_payload_to_tfp_dp4a,
	set_output_byte_strides_tfp_dp4a
};

static packet_unpack_candidate_t tfp_dp4a_direct_unpack_candidate = {
	"TFP_DP4A_DIRECT",
	copy_packet_payload_to_tfp_dp4a_direct,
	set_output_byte_strides_tfp_dp4a
};

#endif