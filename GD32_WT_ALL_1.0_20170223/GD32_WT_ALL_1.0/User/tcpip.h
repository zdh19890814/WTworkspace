#ifndef __TCPIP_H
#define __TCPIP_H
#include "include.h"
 typedef struct ip_dev{
	unsigned char verandihl;		    //�汾��ͷ���ȣ�1
	unsigned char typeofserver;	    //��������,1
	unsigned char totallength[2];		//�ܳ���,2
	unsigned char frameindex[2];		//IP֡���,2
	unsigned char segment[2];		  	//�ֶα�־,2
	unsigned char ttl;			        //����ʱ��,1
	unsigned char protocal;			    //��һ��Э��,1
	unsigned char crc[2];			      //У���,2
	unsigned char sourceip[4];		//ԴIP,4
	unsigned char destip[4];			//Ŀ��IP,4
	unsigned char packet[20];	    //IP��������,1480
}IP;	
void ip_data(char *data);
#endif
