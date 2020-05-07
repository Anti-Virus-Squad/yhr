/*
 * Copyright (c) 1999 - 2005 NetGroup, Politecnico di Torino (Italy)
 * Copyright (c) 2005 - 2006 CACE Technologies, Davis (California)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Politecnico di Torino, CACE Technologies 
 * nor the names of its contributors may be used to endorse or promote 
 * products derived from this software without specific prior written 
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifdef _MSC_VER
/*
 * we do not want the warnings about the old deprecated and unsecure CRT functions
 * since these examples can be compiled under *nix as well
 */
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "pcap.h"

/* 4 bytes IP address */
typedef struct ip_address
{
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address;

/* IPv4 header */
typedef struct ip_header {
	u_char ver_ihl; // Version (4 bits) + Internet header length(4 bits)�汾��+IP��ͷ����
		u_char tos; // Type of service ��������
	u_short tlen; // Total length �ܳ���
	u_short identification; // Identification��ʶ��
	u_short flags_fo; // Flags (3 bits) + Fragment offset(13 bits)��Ƭ�������Ƭƫ����
		u_char ttl; // Time to live��������
	u_char proto; // ProtocolЭ��
	u_short crc; // Header checksumУ���
	u_char saddr[4]; // Source addressԴIP��ַ
	u_char daddr[4]; // Destination addressĿ��IP��ַ
	u_int op_pad; // Option + Padding
} ip_header;

typedef struct mac_header {
	u_char dest_addr[6];
	u_char src_addr[6];
	u_char type[2];
} mac_header;

/* UDP header*/
typedef struct udp_header
{
	u_short sport;			// Source port
	u_short dport;			// Destination port
	u_short len;			// Datagram length
	u_short crc;			// Checksum
}udp_header;

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

#define FROM_NIC
int main()
{
	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i=0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	u_int netmask;
	char packet_filter[] = "ip and udp";
	struct bpf_program fcode;
#ifdef FROM_NIC
	/* Retrieve the device list */
	

	if(pcap_findalldevs(&alldevs, errbuf) == -1)
	{
		fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}
	
	/* Print the list */
	for(d=alldevs; d; d=d->next)
	{
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if(i==0)
	{
		printf("\nNo interfaces found! Make sure WinPcap is installed.\n");
		return -1;
	}
	
	printf("Enter the interface number (1-%d):",i);
	scanf("%d", &inum);
	
	/* Check if the user specified a valid adapter */
	if(inum < 1 || inum > i)
	{
		printf("\nAdapter number out of range.\n");
		
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* Jump to the selected adapter */
	for(d=alldevs, i=0; i< inum-1 ;d=d->next, i++);
	
	/* Open the adapter */
	if ((adhandle= pcap_open_live(d->name,	// name of the device
							 65536,			// portion of the packet to capture. 
											// 65536 grants that the whole packet will be captured on all the MACs.
							 1,				// promiscuous mode (nonzero means promiscuous)
							 1000,			// read timeout
							 errbuf			// error buffer
							 )) == NULL)
	{
		fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	/* Check the link layer. We support only Ethernet for simplicity. */
	if(pcap_datalink(adhandle) != DLT_EN10MB)
	{
		fprintf(stderr,"\nThis program works only on Ethernet networks.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	if(d->addresses != NULL)
		/* Retrieve the mask of the first address of the interface */
		netmask=((struct sockaddr_in *)(d->addresses->netmask))->sin_addr.S_un.S_addr;
	else
		/* If the interface is without addresses we suppose to be in a C class network */
		netmask=0xffffff; 


	//compile the filter
	if (pcap_compile(adhandle, &fcode, packet_filter, 1, netmask) <0 )
	{
		fprintf(stderr,"\nUnable to compile the packet filter. Check the syntax.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	//set the filter
	if (pcap_setfilter(adhandle, &fcode)<0)
	{
		fprintf(stderr,"\nError setting the filter.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}
	
	printf("\nlistening on %s...\n", d->description);
	
	/* At this point, we don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);
	
	/* start the capture */
	pcap_loop(adhandle, 0, packet_handler, NULL);
#else

	/* Open the capture file */
	if ((adhandle = pcap_open_offline("C:\\Users\\admin\\Desktop\\dns.pcap",			// name of the device
		errbuf					// error buffer
	)) == NULL)
	{
		fprintf(stderr, "\nUnable to open the file .\n");
		return -1;
	}

	/* read and dispatch packets until EOF is reached */
	pcap_loop(adhandle, 0, packet_handler, NULL);

	pcap_close(adhandle);
#endif
	return 0;
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	FILE *fp = fopen("user.csv", "w");//�����ļ�CSV��ʽ
	struct tm *ltime;
	char timestr[16];
	mac_header *mh;
	ip_header *ih;
	udp_header *uh;
	u_int ip_len;
	u_short sport,dport;
	time_t local_tv_sec;

	int length = sizeof(mac_header) + sizeof(ip_header);
	for (int i = 0; i < length; i++) {
		printf("%02X ", pkt_data[i]);
		if ((i & 0xF) == 0xF)
			printf("\n");
	}
	printf("\n");

	

	/*
	 * unused parameter
	 */
	(VOID)(param);

	/* convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	ltime=localtime(&local_tv_sec);
	strftime( timestr, sizeof timestr, "%H:%M:%S", ltime);

	/* print timestamp and length of the packet */
	printf("%s.%.6d len:%d ", timestr, header->ts.tv_usec, header->len);

	/* retireve the position of the ip header */
	ih = (ip_header *) (pkt_data +
		14); //length of ethernet header
	mh = (mac_header*)pkt_data;
	printf("mac_header:\n");
	printf("\tdest_addr: ");
	for (int i = 0; i < 6; i++) {
		printf("%02X ", mh->dest_addr[i]);
	}
	printf("\n");
	printf("\tsrc_addr: ");
	for (int i = 0; i < 6; i++) {
		printf("%02X ", mh->src_addr[i]);
	}
	printf("\n");
	//printf("\ttype: %04X", ntohs(mh->type[i]));
	printf("\n");
	/* retireve the position of the udp header */
	ih = (ip_header *)(pkt_data + sizeof(mac_header)); //length of ethernet header
		printf("ip_header\n");

	printf("\t%-10s: %02X\n", "ver_ihl", ih->ver_ihl);// Version (4 bits) + Internet header length(4 bits)�汾��+IP��ͷ����
	printf("\t%-10s: %02X\n", "tos", ih->tos);// Type of service ��������
	printf("\t%-10s: %04X\n", "tlen", ntohs(ih->tlen));// Total length �ܳ���
	printf("\t%-10s: %04X\n", "identification", ntohs(ih -> identification));// Identification��ʶ��
	printf("\t%-10s: %04X\n", "flags_fo", ntohs(ih->flags_fo)); // Flags (3 bits) + Fragment offset(13 bits)��Ƭ�������Ƭƫ����
	printf("\t%-10s: %02X\n", "ttl", ih->ttl); // Time to live��������
	printf("\t%-10s: %02X\n", "proto", ih->proto);// ProtocolЭ��
	printf("\t%-10s: %04X\n", "crc", ntohs(ih->crc));// Header checksumУ���
	printf("\t%-10s: %08X\n", "op_pad", ntohs(ih->op_pad));// Option + Padding
	printf("\t%-10s: ", "saddr:");// Source addressԴIP��ַ

	/* convert from network byte order to host byte order */
	

	/* print ip addresses and udp ports */
	for (int i = 0; i < 4; i++) {
		printf("%02X ", ih->saddr[i]);// Source addressԴIP��ַ
	}
	printf(" ");
	for (int i = 0; i < 4; i++) {
		printf("%d.", ih->saddr[i]);
	}
	printf("\n");
	printf("\t%-10s: ", "daddr");
	for (int i = 0; i < 4; i++) {
		printf("%02X ", ih->daddr[i]);//Ŀ�ĵ�ַ
	}
	printf(" ");
	for (int i = 0; i < 4; i++) {
		printf("%d.", ih->daddr[i]);
	}
	printf("\n");

	//��ͬ����������ļ�
	for (int i = 0; i < length; i++) {
		if (pkt_data[i] == 85 && pkt_data[i + 1] == 83 && pkt_data[i + 2] == 69 && pkt_data[i + 3] == 82) {
			fprintf(fp, "%s, ", timestr);
			for (int i = 0; i < 6; i++) {
				fprintf(fp,"%02X ", mh->src_addr[i]);
			}
			fprintf(fp, ", ");
			for (int i = 0; i < 4; i++) {
				fprintf(fp,"%d.", ih->saddr[i]);
			}
			fprintf(fp, ", ");
			for (int i = 0; i < 6; i++) {
				fprintf(fp,"%02X ", mh->dest_addr[i]);
			}
			fprintf(fp, ", ");
			for (int i = 0; i < 4; i++) {
				fprintf(fp,"%d.", ih->daddr[i]);
			}
			fprintf(fp, ", ");
			for (int j = i + 4; pkt_data[j] != 13; j++)fputc(pkt_data[j],fp); 
			fprintf(fp, ", ");
			fclose(fp);
		}
		else if (pkt_data[i] == 80 && pkt_data[i + 1] == 65 && pkt_data[i + 2] == 83 && pkt_data[i + 3] == 83) {
			FILE *fp1 = fopen("user.csv", "a");
			for (int j = i + 4; pkt_data[j] != 13; j++)fputc(pkt_data[j], fp);
			fprintf(fp1, ", ");
			fclose(fp1);
		}
		else if (pkt_data[i] == 50 && pkt_data[i + 1] == 51 && pkt_data[i + 2] == 48) {
			FILE *fp2 = fopen("user.csv", "a");
			fprintf(fp2, "SUCCEED ");
			fclose(fp2);
		}
		else if (pkt_data[i] == 53 && pkt_data[i + 1] == 51 && pkt_data[i + 2] == 48) {
			FILE *fp2 = fopen("user.csv", "a");
			fprintf(fp2, "FAILED ");
			fclose(fp2);
		}
		
	}
	
}
