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

int main(int argc, char* argv[]) {
	// Setup
  struct ata_snap_ibv_pkt *p_packet = NULL;
  uint8_t* payload_dest = NULL;

	char test_passed = 1;
	
	int databuf_packet_size = sizeof(struct ata_snap_ibv_pkt) + SYNTH_PACKET_PAYLOAD_BYTE_LENGTH;
	int n_databuf_packets = BLOCK_DATA_SIZE/databuf_packet_size;
	printf("Padded-packet size: %i\n", databuf_packet_size);
	printf("Padded-packet per block: %i\n", n_databuf_packets);

	uint8_t *databuf_in;
	uint8_t *databuf_out;
	uint8_t *ata_ibv_pkt_bytes;
	uint8_t *pkt_payload;
	struct ata_snap_ibv_pkt *ata_ibv_pkt;

	size_t time_byte_stride;
	size_t channel_byte_stride;

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

	// Setup: set data 
	ata_ibv_pkt_bytes = malloc(databuf_packet_size);
	memset(ata_ibv_pkt_bytes, 0, databuf_packet_size);

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
	
	// Test: FTP
	set_output_byte_strides_ftp(time_per_block, &time_byte_stride, &channel_byte_stride);
	memset(databuf_out, 0, BLOCK_DATA_SIZE);
	for (size_t i = 0; i < effective_payload_per_block; i++) {
		p_packet = (struct ata_snap_ibv_pkt *) (databuf_in + i*databuf_packet_size);
		payload_dest = databuf_out
			+ p_packet->snaphdr.timestamp*time_byte_stride // offset for time
			+ (p_packet->snaphdr.feng_id*SYNTH_NCHAN + (p_packet->snaphdr.chan-SYNTH_SCHAN))*channel_byte_stride; // offset for frequency	
		pkt_payload = (uint8_t *)p_packet->payload;
		copy_packet_payload_to_ftp(payload_dest, pkt_payload, SYNTH_PKTNCHAN, channel_byte_stride, time_byte_stride);
	}

	test_passed = 1;
	for (size_t i = 0; i < effective_block_size && test_passed == 1; i++) {
		if(databuf_out[i] != PAYLOAD_BYTE_VALUE) {
			printf("FTP Test failed at byte %lu (%u != %u).\n", i, databuf_out[i], PAYLOAD_BYTE_VALUE);
			printf("\tTime #%lu (Time-per-block = %lu)\n", i/time_byte_stride, time_per_block);
			printf("\tChannel #%lu (Ant #%lu, Ant-chan %lu)\n", i/channel_byte_stride, (i/channel_byte_stride)/SYNTH_NCHAN, (i/channel_byte_stride)%SYNTH_NCHAN);
			test_passed = 0;
		}	
	}
	if(test_passed == 1){
		printf("FTP Test Passed.\n");
	}
	
	// Test: TFP
	set_output_byte_strides_tfp(time_per_block, &time_byte_stride, &channel_byte_stride);
	memset(databuf_out, 0, BLOCK_DATA_SIZE);
	for (size_t i = 0; i < effective_payload_per_block; i++) {
		p_packet = (struct ata_snap_ibv_pkt *) (databuf_in + i*databuf_packet_size);
		payload_dest = databuf_out
			+ p_packet->snaphdr.timestamp*time_byte_stride // offset for time
			+ (p_packet->snaphdr.feng_id*SYNTH_NCHAN + (p_packet->snaphdr.chan-SYNTH_SCHAN))*channel_byte_stride; // offset for frequency	
		pkt_payload = (uint8_t *)p_packet->payload;
		copy_packet_payload_to_tfp(payload_dest, pkt_payload, SYNTH_PKTNCHAN, channel_byte_stride, time_byte_stride);
	}

	test_passed = 1;
	for (size_t i = 0; i < effective_block_size && test_passed == 1; i++) {
		if(databuf_out[i] != PAYLOAD_BYTE_VALUE) {
			printf("TFP Test failed at byte %lu (%u != %u).\n", i, databuf_out[i], PAYLOAD_BYTE_VALUE);
			printf("\tTime #%lu (Time-per-block = %lu)\n", i/time_byte_stride, time_per_block);
			printf("\tChannel #%lu (Ant #%lu, Ant-chan %lu)\n", i/channel_byte_stride, (i/channel_byte_stride)/SYNTH_NCHAN, (i/channel_byte_stride)%SYNTH_NCHAN);
			test_passed = 0;
		}	
	}
	if(test_passed == 1){
		printf("TFP Test Passed.\n");
	}
	
	// Test: TFP_DP4A
	set_output_byte_strides_tfp_dp4a(time_per_block, &time_byte_stride, &channel_byte_stride);
	memset(databuf_out, 0, BLOCK_DATA_SIZE);
	for (size_t i = 0; i < effective_payload_per_block; i++) {
		p_packet = (struct ata_snap_ibv_pkt *) (databuf_in + i*databuf_packet_size);
		payload_dest = databuf_out
			+ p_packet->snaphdr.timestamp*time_byte_stride // offset for time
			+ (p_packet->snaphdr.feng_id*SYNTH_NCHAN + (p_packet->snaphdr.chan-SYNTH_SCHAN))*channel_byte_stride; // offset for frequency	
		pkt_payload = (uint8_t *)p_packet->payload;
		copy_packet_payload_to_tfp_dp4a(payload_dest, pkt_payload, SYNTH_PKTNCHAN, channel_byte_stride, time_byte_stride);
	}

	test_passed = 1;
	for (size_t i = 0; i < effective_block_size && test_passed == 1; i++) {
		if(databuf_out[i] != PAYLOAD_BYTE_VALUE) {
			printf("TFP_DP4A Test failed at byte %lu (%u != %u).\n", i, databuf_out[i], PAYLOAD_BYTE_VALUE);
			printf("\tTime #%lu (Time-per-block = %lu)\n", i/time_byte_stride, time_per_block);
			printf("\tChannel #%lu (Ant #%lu, Ant-chan %lu)\n", i/channel_byte_stride, (i/channel_byte_stride)/SYNTH_NCHAN, (i/channel_byte_stride)%SYNTH_NCHAN);
			test_passed = 0;
		}	
	}
	if(test_passed == 1){
		printf("TFP_DP4A Test Passed.\n");
	}

	// Close: free memory
	free(databuf_in);
	free(databuf_out);
	free(ata_ibv_pkt_bytes);
	ata_ibv_pkt = NULL;
	return 0;
}