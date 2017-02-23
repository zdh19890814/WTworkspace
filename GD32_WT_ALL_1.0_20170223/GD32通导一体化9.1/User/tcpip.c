
#include "include.h"

void ip_data(char *data)
{	int i;
	IP ip;
	unsigned char ip_host[4]={0xc0,0xa8,0x01,0x12};
	ip.verandihl=data[0];ip.typeofserver=data[1];ip.totallength[0]=data[2];ip.totallength[1]=data[3];
	ip.frameindex[0]=data[4];ip.frameindex[1]=data[5];ip.segment[0]=data[6];ip.segment[1]=data[7];
	ip.ttl=data[8];ip.protocal=data[9];ip.crc[0]=data[10];ip.crc[1]=data[11];
	ip.sourceip[0]=data[12];
	ip.sourceip[1]=data[13];
	ip.sourceip[2]=data[14];
	ip.sourceip[3]=data[15];
  DebugOut("\r\n源IP地址\r\n");
	DebugOut("%d.%d.%d.%d",ip.sourceip[0],ip.sourceip[1],ip.sourceip[2],ip.sourceip[3]);
	ip.destip[0]=data[16];
	ip.destip[1]=data[17];
	ip.destip[2]=data[18];
	ip.destip[3]=data[19];
	DebugOut("\r\n目的IP地址\r\n");
	DebugOut("%d.%d.%d.%d",ip.destip[0],ip.destip[1],ip.destip[2],ip.destip[3]);
	for(i=0;i<20;i++)
	ip.packet[i]=data[i+20];
	if((ip_host[0]==ip.destip[0])&&(ip_host[1]==ip.destip[1])&&(ip_host[2]==ip.destip[2])&&(ip_host[3]==ip.destip[3]))
	DebugOut("\r\nTCP数据\r\n");
  DebugOut("%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d.%d",ip.packet[0],ip.packet[1],ip.packet[2],ip.packet[3],\
	                             ip.packet[4],ip.packet[5],ip.packet[6],ip.packet[7],ip.packet[8],ip.packet[9],ip.packet[10],\
                               ip.packet[11],ip.packet[12],ip.packet[13],ip.packet[14],ip.packet[15],ip.packet[16],\
	                             ip.packet[17],ip.packet[18],ip.packet[19]);
}
