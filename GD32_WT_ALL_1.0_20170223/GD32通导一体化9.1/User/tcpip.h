#ifndef __TCPIP_H
#define __TCPIP_H
#include "include.h"
 typedef struct ip_dev{
	unsigned char verandihl;		    //版本与头长度，1
	unsigned char typeofserver;	    //服务类型,1
	unsigned char totallength[2];		//总长度,2
	unsigned char frameindex[2];		//IP帧序号,2
	unsigned char segment[2];		  	//分段标志,2
	unsigned char ttl;			        //生存时间,1
	unsigned char protocal;			    //下一层协议,1
	unsigned char crc[2];			      //校验和,2
	unsigned char sourceip[4];		//源IP,4
	unsigned char destip[4];			//目的IP,4
	unsigned char packet[20];	    //IP包的内容,1480
}IP;	
void ip_data(char *data);
#endif
