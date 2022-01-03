#ifndef _PACKET_UNPACK_STRUCT_H
#define _PACKET_UNPACK_STRUCT_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
	uint8_t* databuf_in;
	uint8_t* databuf_out;
	size_t effective_payload_per_block;
	size_t effective_block_size;
	size_t time_per_block;
	int databuf_packet_size;
	void (*copy_func)(
		uint8_t*  payload_dest,
		uint8_t*  pkt_payload,
		const uint16_t  pkt_nchan,
		const uint32_t  channel_stride,
		const uint32_t  time_stride);
	void (*byte_stride_func)(
		const size_t time_per_block,
		size_t*  channel_stride,
		size_t*  time_stride);
} packet_unpack_struct_t;

typedef struct {
	char* title;
	void (*copy_func)(
		uint8_t*  payload_dest,
		uint8_t*  pkt_payload,
		const uint16_t  pkt_nchan,
		const uint32_t  channel_stride,
		const uint32_t  time_stride);
	void (*byte_stride_func)(
		const size_t time_per_block,
		size_t*  channel_stride,
		size_t*  time_stride);
} packet_unpack_candidate_t;

#endif