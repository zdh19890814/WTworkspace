#include <atelier_usb_plugin.h>
# include <atelier_plugin/atelier_plugin_manager.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <pthread.h>
#include <sys/time.h>

#define VID 0x0471
#define PID 0x0999
int Trans_Flag =0;

using namespace std;

namespace atelier_plugin {
	AtelierPluginManager* plugin_manager = AtelierPluginManager::GetInstance();
	libusb_context *ctx_static=NULL;
	libusb_device_handle *dev_handle_static=init_handle(PID,VID,ctx_static);//a device handle
	static int num=0;
	pthread_mutex_t mut;
AtelierPlugin* CreateTestPlugin() {
  return new TestPlugin();
}

TestPlugin::~TestPlugin() {

}

void TestPlugin::Initialize() {
  logAtelier("TestPlugin::Initialized !!!");
}
libusb_device_handle * init_handle(int vvid,int ppid,libusb_context *ctx ){
	libusb_device **devs=NULL;          //pointer to pointer of device, used to retrieve a list of devices
	libusb_device_handle *dev_handle=NULL ;// dev_handle
	ssize_t cnt;                   //holding number of devices in list
	int r; //for return values  
	r = libusb_init(&ctx);
    if(r < 0) {  
       logAtelier("libusb_init��ʼ������\n"); //there was an error  
       return NULL;   
    }  
    libusb_set_debug(ctx, (int)LIBUSB_LOG_LEVEL_INFO); //set verbosity level to 3, as suggested in the documentation  
  
    cnt = libusb_get_device_list(ctx, &devs); //get the list of devices  
    if(cnt < 0) {  
        logAtelier("libusb_get_device_lis��ȡ�豸����\n"); //there was an error  
        return NULL;  
    }  
    logAtelier("����%d���豸.\n", cnt);
	
    dev_handle = libusb_open_device_with_vid_pid(ctx, VID, PID); 
    
    if(dev_handle == NULL){
			logAtelier("libusb_open_device_with_vid_pid�޷����豸\n");  			
			return NULL;
   	}else{
		    logAtelier("libusb_open_device_with_vid_pid�Ѿ����豸\n");  			
	}
        
    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
  
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    } 
    
    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)  
    if(r < 0) {  
        logAtelier("libusb_claim_interfaceʧ��\n");
        return NULL; 
	}
    return dev_handle;		
}


void closeAndExit(libusb_device_handle *dev_handle,libusb_context *ctx){
	if(dev_handle!=NULL)
	libusb_close(dev_handle); //close the device we opened  
    if(ctx!=NULL)
    libusb_exit(ctx); //needs to be called to end the 	
}

void * readMusicAndSend(void *hd){//����music
	int *size=NULL;//NULL
	int rec;
	int len=320; 
	unsigned char  buffer[320];
	FILE * fp=fopen("//data//music.bin","rb+");
	struct timeval t_start,t_end;//ʱ��		
	if(fp==NULL){
		printf("file is not open");
		logAtelier("�ļ���ʧ��");
        Trans_Flag = 1;
		return NULL;
        
	    }
	for(int i=0;;i++)
	{
		//fseek(fp,320*i,SEEK_SET);
		//gettimeofday(&t_start,NULL);//��ʼʱ��
		
		if(fread(buffer,32,10,fp)!=10)
		{
			fseek(fp,0,SEEK_SET);//��λ����ͷ
			//break;
		}
		else
		{
			pthread_mutex_lock(&mut);	
			if(0 == Trans_Flag )
			{
				if(buffer[319] != 0)
				{
					rec = libusb_bulk_transfer((libusb_device_handle *)hd, 2,buffer,len, size, 0);
					memset(buffer,0,320);
				}
			}
		    Trans_Flag = 1;
			pthread_mutex_unlock(&mut);
			/*if(rec==0)
			{
				std::stringstream sst;
				fpos_t pos;
				fgetpos(fp,&pos);
				sst<<"pos :"<<pos<<"  ";
				for(int j=0;j<320;j++){
					sst<<(j+1)<<":"<<(int)(buffer[j])<<" ";
				}
				sst<<"\n";
				logAtelier(sst.str().c_str());
				logAtelier("�����Ѿ�����");
		    	gettimeofday(&t_end,NULL);//����ʱ��
				usleep(20000-(t_end.tv_usec-t_start.tv_usec));
			}*/
		}	
	}
	if(fp!=NULL){
		fclose(fp);
	}
	return NULL;
}


void * recvMusicAndStore(void * hd){
	int *size=NULL;	
	unsigned char revbulk[320]={0};
	int len=320;
	int rec;
	struct timeval t_start,t_end;//ʱ��	
	while(Trans_Flag)
	{
		//gettimeofday(&t_start,NULL);
		pthread_mutex_lock(&mut);
		if(Trans_Flag = 1)
		{
			rec=libusb_bulk_transfer((libusb_device_handle *)hd, 0x81, revbulk, len, size, 0);//type=0�������䣬type=1�жϴ���
		}
		Trans_Flag = 0;
		pthread_mutex_unlock(&mut);
		if(rec==0)
		{//���ճɹ�
			/*std::stringstream sst;
			for(int j=0;j<320;j++){
				sst<<(j+1)<<":"<<(int)(revbulk[j])<<" ";
			}
			sst<<"\n";
			logAtelier(sst.str().c_str());			
			gettimeofday(&t_end,NULL);
			usleep(20000-(t_end.tv_usec-t_start.tv_usec));*/
//send to audioMC
		}
		else
		{
			logAtelier("����ʧ��");			
			//break;
		}
		memset(revbulk,0,320);
	}
	return 0;	
}
 int send_bulk(libusb_device_handle *hd,unsigned char * arr)
{
	int *size=NULL;//NULL
	int rec;
	int len=230; 
	rec = libusb_bulk_transfer(hd, 2,arr,len, size, 0);   //type :1 Ϊ�жϣ�2 Ϊ����
	if(rec==0){
		return 1;//���ͳɹ�
	}
  return 0; 
}
 	
 int rev_bulk(libusb_device_handle *hd){
	int *size=NULL;	
	unsigned char revbulk[1024]={0};
	int len=230;
	int rec=libusb_bulk_transfer(hd, 0x81, revbulk, len, size, 0);//type=0�������䣬type=1�жϴ���
	if(rec==0){//���ܳɹ�
		
	}
	return 0;
 }

void TestPlugin::Execute(std::string action, std::string callback_id, std::string args) {
  logAtelier("TestPlugin::Executexxxxxxxxxxxxx !!!  action: %s",action.c_str());
  logAtelier("TestPlugin::Executexxxxxxxxxxxxx !!!  action: %s",args.c_str());
   if(action=="sendMusic"){
	   while(num<5){
		if(dev_handle_static==NULL){
		dev_handle_static=init_handle(PID,VID,ctx_static);//���豸����ȡdev_handle
		}else{
		break;
		}
		num++;
	    }  	
		num=0;//��0	
		pthread_mutex_init(&mut,NULL);//��ʼ����
		if(dev_handle_static!=NULL){
		pthread_t tid;
		pthread_t tid1;
		pthread_create(&tid, NULL,readMusicAndSend, dev_handle_static);
		pthread_create(&tid1, NULL,recvMusicAndStore, dev_handle_static);
		plugin_manager->PostMsgToJs(callback_id, true, 1,"excuteMusic", false);
		}
		plugin_manager->PostMsgToJs(callback_id, true, 1,"test", false);
	}
	if(action=="closeUSB"){
		closeAndExit(dev_handle_static,ctx_static);//�ر�
		plugin_manager->PostMsgToJs(callback_id, true, 1,"�ر�USB�ɹ�", false);
	}
   if(action=="interrupt_rev"){		
	    plugin_manager->PostMsgToJs(callback_id, true, 1,"���豸ʧ��", false); 	
    }	
}
}//namespace atelier_plugin
