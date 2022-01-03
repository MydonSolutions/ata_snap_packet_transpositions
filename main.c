#include "main.h"
#include "ftp.h"
#include "tfp.h"
#include "tfp_dp4a.h"

// The transposition exercised is from a block of ATA SNAP packets,
// the headers of which specify:
//    PKTNCHAN(Number of channel in payload)
//    CHAN  	(Starting channel of payload)
//    FENG  	(AntennaID)
//    PKTIDX	(Staring Time of payload)
//
// where each SNAP packet has dimensions:
//    [Slowest ------> Fastest]
//    [PKTNCHAN, PKTNTIME, NPOL]

#define PAYLOAD_BYTE_VALUE 0x01

#define BLOCK_DATA_SIZE (120*1024*1024)

int verify_unpacked_buffer(
	uint8_t expected_data,
	packet_unpack_struct_t *unpack_struct,
	char* title
){
	size_t channel_byte_stride, time_byte_stride;
  (*(unpack_struct->byte_stride_func))(unpack_struct->time_per_block, &channel_byte_stride, &time_byte_stride);

	int test_passed = 1;
	for (size_t i = 0; i < unpack_struct->effective_block_size && test_passed == 1; i++) {
		if(unpack_struct->databuf_out[i] != expected_data) {
			printf("%s: Test failed at byte %lu (%u != %u).\n", title, i, unpack_struct->databuf_out[i], expected_data);
			printf("\tTime #%lu (Time-per-block = %lu)\n", i/time_byte_stride, unpack_struct->time_per_block);
			printf("\tChannel #%lu (Ant #%lu, Ant-chan %lu)\n", i/channel_byte_stride, (i/channel_byte_stride)/SYNTH_NCHAN, (i/channel_byte_stride)%SYNTH_NCHAN);
			test_passed = 0;
		}	
	}
	if(test_passed == 1){
		printf("%s: Test Passed.\n", title);
	}
	return test_passed;
}

int main(int argc, char* argv[]) {
	// Setup	
	int databuf_packet_size = sizeof(struct ata_snap_ibv_pkt) + SYNTH_PACKET_PAYLOAD_BYTE_LENGTH;
	int n_databuf_packets = BLOCK_DATA_SIZE/databuf_packet_size;
	printf("Padded-packet size: %i\n", databuf_packet_size);
	printf("Padded-packet per block: %i\n", n_databuf_packets);

	uint8_t *databuf_in;
	uint8_t *databuf_out;
	uint8_t *ata_ibv_pkt_bytes;
	struct ata_snap_ibv_pkt *ata_ibv_pkt;

	packet_unpack_struct_t unpack_struct = {0};

	size_t channel_sections = SYNTH_NANTS*(SYNTH_NCHAN/SYNTH_PKTNCHAN);
	size_t effective_payload_per_block = channel_sections*((BLOCK_DATA_SIZE/SYNTH_PACKET_PAYLOAD_BYTE_LENGTH)/channel_sections);
	size_t effective_block_size = effective_payload_per_block * SYNTH_PACKET_PAYLOAD_BYTE_LENGTH;
	size_t time_per_block = (effective_payload_per_block/channel_sections) * SYNTH_PKTNTIME;

	printf("Channel sections: %lu\n", channel_sections);
	printf("Theoretical payloads per block: %u\n", BLOCK_DATA_SIZE/SYNTH_PACKET_PAYLOAD_BYTE_LENGTH);
	printf("Effective payloads per block: %lu\n", effective_payload_per_block);
	printf("Using %.02f%% of BLOCK_DATA_SIZE (%lu/%u).\n", 100*((float)effective_block_size)/BLOCK_DATA_SIZE, effective_block_size, BLOCK_DATA_SIZE);
	printf("Time per block: %lu\n", time_per_block);
	printf("\n");

	// Setup: allocate memory
	databuf_in = malloc(effective_payload_per_block*databuf_packet_size);
	databuf_out = malloc(BLOCK_DATA_SIZE);

	// Setup: set universal unpack_struct fields 
	unpack_struct.databuf_in = databuf_in;
	unpack_struct.databuf_out = databuf_out;
	unpack_struct.effective_payload_per_block = effective_payload_per_block;
	unpack_struct.effective_block_size = effective_block_size;
	unpack_struct.time_per_block = time_per_block;
	unpack_struct.databuf_packet_size = databuf_packet_size;
	
	// Setup: set buffers 
	ata_ibv_pkt_bytes = malloc(databuf_packet_size);
	memset(ata_ibv_pkt_bytes, 0, databuf_packet_size);

	// Setup: write packets
	ata_ibv_pkt = (struct ata_snap_ibv_pkt *) ata_ibv_pkt_bytes;
	ata_ibv_pkt->snaphdr.n_chans = SYNTH_PKTNCHAN;
	ata_ibv_pkt->snaphdr.version = 42;
	ata_ibv_pkt->snaphdr.type = 42;
	memset(ata_ibv_pkt->payload, PAYLOAD_BYTE_VALUE, SYNTH_PACKET_PAYLOAD_BYTE_LENGTH);

	uint16_t chan_section_idx = 0;
	for (size_t p = 0; p < effective_payload_per_block; p++)
	{
		ata_ibv_pkt->snaphdr.chan = SYNTH_SCHAN + chan_section_idx*SYNTH_PKTNCHAN;
		memcpy(databuf_in + p*databuf_packet_size, ata_ibv_pkt, databuf_packet_size);

		chan_section_idx++;
		if (chan_section_idx == SYNTH_NCHAN/SYNTH_PKTNCHAN) {
			chan_section_idx = 0;
			ata_ibv_pkt->snaphdr.feng_id++;
			if (ata_ibv_pkt->snaphdr.feng_id == SYNTH_NANTS) {
				ata_ibv_pkt->snaphdr.feng_id = 0;
				ata_ibv_pkt->snaphdr.timestamp += SYNTH_PKTNTIME;
			}
		}
	}
	
	// Tests
	const packet_unpack_candidate_t candidates[] = {
		ftp_unpack_candidate,
		tfp_unpack_candidate,
		tfp_dp4a_unpack_candidate
	};

	for (size_t c = 0; c < sizeof(candidates)/sizeof(packet_unpack_candidate_t); c++)
	{
		unpack_struct.copy_func = candidates[c].copy_func;
		unpack_struct.byte_stride_func = candidates[c].byte_stride_func;
		memset(databuf_out, 0, BLOCK_DATA_SIZE);
		unpack_packet_buffer(&unpack_struct);
		verify_unpacked_buffer(
			PAYLOAD_BYTE_VALUE,
			&unpack_struct,
			candidates[c].title
		);
	}

	// Close: free memory
	free(databuf_in);
	free(databuf_out);
	free(ata_ibv_pkt_bytes);
	ata_ibv_pkt = NULL;
	return 0;
}