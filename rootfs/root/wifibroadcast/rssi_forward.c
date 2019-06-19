// rssi_forward by Rodizio (c) 2017. Licensed under GP2.
// reads video rssi from shared mem and sends it out via UDP (for FPV_VR 2018 app)
// usage: rssi_forward 192.168.2.2 5100
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <resolv.h>
#include <string.h>
#include <utime.h>
#include <unistd.h>
#include <getopt.h>
#include <endian.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include "lib.h"

typedef struct {
    uint32_t received_packet_cnt;
    int8_t current_signal_dbm;
    int8_t type; // 0 = Atheros, 1 = Ralink
} __attribute__((packed)) wifi_adapter_rx_status_forward_t;

typedef struct {
    uint32_t damaged_block_cnt; // number bad blocks video downstream
    uint32_t lost_packet_cnt; // lost packets video downstream
    uint32_t skipped_packet_cnt; // skipped packets video downstream
    uint32_t received_packet_cnt; // packets received video downstream
    uint32_t kbitrate; // live video kilobitrate per second video downstream
    uint32_t kbitrate_measured; // max measured kbitrate during tx startup
    uint32_t kbitrate_set; // set kilobitrate (measured * bitrate_percent) during tx startup
    uint32_t lost_packet_cnt_telemetry_up; // lost packets telemetry uplink
    uint32_t lost_packet_cnt_telemetry_down; // lost packets telemetry downlink
    uint32_t lost_packet_cnt_msp_up; // lost packets msp uplink (not used at the moment)
    uint32_t lost_packet_cnt_msp_down; // lost packets msp downlink (not used at the moment)
    uint32_t lost_packet_cnt_rc; // lost packets rc link
    int8_t current_signal_air; // signal strength in dbm at air pi (telemetry upstream and rc link)
    int8_t joystick_connected; // 0 = no joystick connected, 1 = joystick connected
    uint8_t cpuload_gnd; // CPU load Ground Pi
    uint8_t temp_gnd; // CPU temperature Ground Pi
    uint8_t cpuload_air; // CPU load Air Pi
    uint8_t temp_air; // CPU temperature Air Pi
    uint32_t wifi_adapter_cnt; // number of wifi adapters
    wifi_adapter_rx_status_forward_t adapter[6]; // same struct as in wifibroadcast lib.h
} __attribute__((packed)) wifibroadcast_rx_status_forward_t;


wifibroadcast_rx_status_t *status_memory_open() {
	int fd;
	fd = shm_open("/wifibroadcast_rx_status_0", O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { fprintf(stderr,"ERROR: Could not open wifibroadcast_rx_status_0"); exit(1); }
	//if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	return (wifibroadcast_rx_status_t*)retval;
}

wifibroadcast_rx_status_t *status_memory_open_tdown() {
	int fd;
	fd = shm_open("/wifibroadcast_rx_status_1", O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { fprintf(stderr,"ERROR: Could not open wifibroadcast_rx_status_1"); exit(1);}
	//if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t_1)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	return (wifibroadcast_rx_status_t*)retval;
}

wifibroadcast_rx_status_t_sysair *status_memory_open_sysair() {
	int fd;
	fd = shm_open("/wifibroadcast_rx_status_sysair", O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { fprintf(stderr,"ERROR: Could not open wifibroadcast_rx_status_sysair"); exit(1); }
	//if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t_sysair)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t_sysair), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	return (wifibroadcast_rx_status_t_sysair*)retval;
}

wifibroadcast_rx_status_t_rc *status_memory_open_rc() {
	int fd;
	fd = shm_open("/wifibroadcast_rx_status_rc", O_RDWR, S_IRUSR | S_IWUSR);
	if(fd < 0) { fprintf(stderr,"ERROR: Could not open wifibroadcast_rx_status_rc"); exit(1); }
	//if (ftruncate(fd, sizeof(wifibroadcast_rx_status_t_rc)) == -1) { perror("ftruncate"); exit(1); }
	void *retval = mmap(NULL, sizeof(wifibroadcast_rx_status_t_rc), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (retval == MAP_FAILED) { perror("mmap"); exit(1); }
	return (wifibroadcast_rx_status_t_rc*)retval;
}


int main(int argc, char *argv[]) {
	int16_t port = atoi(argv[2]);
	int j = 0;
	int cardcounter = 0;

	fprintf(stderr,"rssi_forward started\n");

	struct sockaddr_in si_other_rssi;
	int s_rssi, slen_rssi=sizeof(si_other_rssi);
	si_other_rssi.sin_family = AF_INET;
	si_other_rssi.sin_port = htons(port);
	si_other_rssi.sin_addr.s_addr = inet_addr(argv[1]);
	memset(si_other_rssi.sin_zero, '\0', sizeof(si_other_rssi.sin_zero));

	wifibroadcast_rx_status_t *t = status_memory_open();
	wifibroadcast_rx_status_t *t_tdown = status_memory_open_tdown();
	wifibroadcast_rx_status_t_sysair *t_sysair = status_memory_open_sysair();
	wifibroadcast_rx_status_t_rc *t_rc = status_memory_open_rc();

	wifibroadcast_rx_status_forward_t wbcdata;

	int number_cards = t->wifi_adapter_cnt;

	wbcdata.damaged_block_cnt = 0;
	wbcdata.lost_packet_cnt = 0;
	wbcdata.skipped_packet_cnt = 0;
	wbcdata.received_packet_cnt = 0;
	wbcdata.kbitrate = 0;
	wbcdata.kbitrate_measured = 0;
	wbcdata.kbitrate_set = 0;
	wbcdata.lost_packet_cnt_telemetry_up = 0;
	wbcdata.lost_packet_cnt_telemetry_down = 0;
	wbcdata.lost_packet_cnt_msp_up = 0;
	wbcdata.lost_packet_cnt_msp_down = 0;
	wbcdata.lost_packet_cnt_rc = 0;
	wbcdata.current_signal_air = 0;
	wbcdata.joystick_connected = 0;
	wbcdata.cpuload_gnd = 0;
	wbcdata.temp_gnd = 0;
	wbcdata.cpuload_air = 0;
	wbcdata.temp_air = 0;
	wbcdata.wifi_adapter_cnt = 0;

	for(j=0; j<6; ++j) {
	    wbcdata.adapter[j].current_signal_dbm = 0;
	    wbcdata.adapter[j].received_packet_cnt = 0;
	    wbcdata.adapter[j].type = 0;
	}

	if ((s_rssi=socket(PF_INET, SOCK_DGRAM, 0))==-1) printf("ERROR: Could not create UDP socket!");

	for(;;) {
	    wbcdata.damaged_block_cnt = t->damaged_block_cnt;
	    wbcdata.lost_packet_cnt = t->lost_packet_cnt;
	    wbcdata.skipped_packet_cnt = t_sysair->skipped_fec_cnt;
	    wbcdata.received_packet_cnt = t->received_packet_cnt;
	    wbcdata.kbitrate = t->kbitrate;
	    wbcdata.kbitrate_measured = t_sysair->bitrate_kbit;
	    wbcdata.kbitrate_set = t_sysair->bitrate_measured_kbit;
	    wbcdata.lost_packet_cnt_telemetry_up = 0;
	    wbcdata.lost_packet_cnt_telemetry_down = t_tdown->lost_packet_cnt;
	    wbcdata.lost_packet_cnt_msp_up = 0;
	    wbcdata.lost_packet_cnt_msp_down = 0;
	    wbcdata.lost_packet_cnt_rc = t_rc->lost_packet_cnt;
	    wbcdata.current_signal_air = t_rc->adapter[0].current_signal_dbm;
	    wbcdata.joystick_connected = 0;
	    wbcdata.cpuload_gnd = 0;
	    wbcdata.temp_gnd = 0;
	    wbcdata.cpuload_air = t_sysair->cpuload;
	    wbcdata.temp_air = t_sysair->temp;
	    wbcdata.wifi_adapter_cnt = t->wifi_adapter_cnt;

	    for(cardcounter=0; cardcounter<number_cards; ++cardcounter) {
		wbcdata.adapter[cardcounter].current_signal_dbm = t->adapter[cardcounter].current_signal_dbm;
		wbcdata.adapter[cardcounter].received_packet_cnt = t->adapter[cardcounter].received_packet_cnt;
		wbcdata.adapter[cardcounter].type = t->adapter[cardcounter].type;
	    }

	    if (sendto(s_rssi, &wbcdata, 94, 0, (struct sockaddr*)&si_other_rssi, slen_rssi)==-1) printf("ERROR: Could not send RSSI data!");
	    usleep(250000);
	}
	return 0;
}
