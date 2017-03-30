#include <atelier_usb_plugin.h>
#include <atelier_plugin/atelier_plugin_manager.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <pthread.h>
#include <sys/time.h>

#define VID 0x0471
#define PID 0x0999

#define EP_ISO_IN       0x81
#define EP_ISO_OUT      0x02


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
       logAtelier("libusb_init初始化错误\n"); //there was an error  
       return NULL;   
    }  
    libusb_set_debug(ctx, (int)LIBUSB_LOG_LEVEL_INFO); //set verbosity level to 3, as suggested in the documentation  
  
    cnt = libusb_get_device_list(ctx, &devs); //get the list of devices  
    if(cnt < 0) {  
        logAtelier("libusb_get_device_lis获取设备错误\n"); //there was an error  
        return NULL;  
    }  
    logAtelier("共有%d个设备.\n", cnt);
	
    dev_handle = libusb_open_device_with_vid_pid(ctx, VID, PID); 
    
    if(dev_handle == NULL){
			logAtelier("libusb_open_device_with_vid_pid无法打开设备\n");  			
			return NULL;
   	}else{
		    logAtelier("libusb_open_device_with_vid_pid已经打开设备\n");  			
	}
        
    libusb_free_device_list(devs, 1); //free the list, unref the devices in it
  
    if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    } 
    
    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)  
    if(r < 0) {  
        logAtelier("libusb_claim_interface失败\n");
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

void * readMusicAndSend(void *hd){//发送music
	//int *size=NULL;//NULL
	int rec;
	//int len=320; 
	unsigned char  buffer[320];
    static struct libusb_transfer *xfr_out;
    int num_iso_pack = 2;
	FILE * fp=fopen("//data//music.bin","rb+");
	struct timeval t_start,t_end;//时间	
    unsigned int diff_msec; 	
	if(fp==NULL){
		printf("file is not open");
		logAtelier("文件打开失败");
		return NULL;
	    }
	for(int i=0;;i++){
		//fseek(fp,320*i,SEEK_SET);
         xfr_out = libusb_alloc_transfer(num_iso_pack);
        if (!xfr_out)
            return -ENOMEM;
        libusb_fill_iso_transfer(xfr_out, (libusb_device_handle *)hd, EP_ISO_OUT, buffer,
                                sizeof(buffer), num_iso_pack, NULL, NULL, 0);  //The first null -- callback
        libusb_set_iso_packet_lengths(xfr_out, sizeof(buffer)/num_iso_pack);
		gettimeofday(&t_start,NULL);//开始时间		
		if(fread(buffer,32,10,fp)!=10){
			fseek(fp,0,SEEK_SET);//定位到开头
			//break;
		}else{
		pthread_mutex_lock(&mut);					
		//rec = libusb_bulk_transfer((libusb_device_handle *)hd, 2,buffer,len, size, 0);
        rec = libusb_submit_transfer(xfr_out);        
		if(rec==0){
		
			/*std::stringstream sst;
			fpos_t pos;
			fgetpos(fp,&pos);
			sst<<"pos :"<<pos<<"  ";
			for(int j=0;j<320;j++){
				sst<<(j+1)<<":"<<(int)(buffer[j])<<" ";
			}
			memset(buffer,320,0);
			sst<<"\n";
			logAtelier(sst.str().c_str());*/
			logAtelier("数据已经发送");

            gettimeofday(&t_end,NULL);//结束时间
            diff_msec = (t_end.tv_sec - t_start.tv_sec)*1000;
            diff_msec += (t_end.tv_usec - t_start.tv_usec)/1000;
            usleep(20000 - diff_msec * 1000);
            
			//usleep(20000-(t_end.tv_usec-t_start.tv_usec));
		}else{
            //do what when send error??
        }
        pthread_mutex_unlock(&mut);

		}	
	}
	if(fp!=NULL){
		fclose(fp);
	}
	return NULL;
}


void * recvMusicAndStore(void * hd){
	//int *size=NULL;	
	unsigned char revbulk[320]={0};
    static struct libusb_transfer *xfr_in;
    int num_iso_pack = 2;
    
	//int len=320;
	int rec;
	struct timeval t_start,t_end;//时间	
    unsigned int diff_msec;

	while(1){
        
        xfr_in = libusb_alloc_transfer(num_iso_pack);
        if (!xfr_in)
            return -ENOMEM;
        libusb_fill_iso_transfer(xfr_in, (libusb_device_handle *)hd, EP_ISO_IN, revbuf,
                                sizeof(revbuf), num_iso_pack, NULL, NULL, 0);  //cb_xfr_in callback
        libusb_set_iso_packet_lengths(xfr_in, sizeof(revbuf)/num_iso_pack);
		gettimeofday(&t_start,NULL);
		pthread_mutex_lock(&mut);
		//rec=libusb_bulk_transfer((libusb_device_handle *)hd, 0x81, revbulk, len, size, 0);//type=0批量传输，type=1中断传输
        rec = libusb_submit_transfer(xfr_in);        
        // zong gan jue zhe yang bu da dui 
                   
		if(rec==0){//接收成功
			/* std::stringstream sst;
			for(int j=0;j<320;j++){
				sst<<(j+1)<<":"<<(int)(revbulk[j])<<" ";
			}

			sst<<"\n";
			logAtelier(sst.str().c_str());	 */
            
		gettimeofday(&t_end,NULL);
		diff_msec = (t_end.tv_sec - t_start.tv_sec)*1000;
        diff_msec += (t_end.tv_usec - t_start.tv_usec)/1000;
        usleep(20000 - diff_msec * 1000);
        //调用后边采集音频的函数
        //usleep(20000-(t_end.tv_usec-t_start.tv_usec));	
        memset(revbulk,0,320);
		}else{
			logAtelier("接收失败");		           
			//break;
		}
         pthread_mutex_unlock(&mut);
	}
	return 0;	
}


/*  int send_bulk(libusb_device_handle *hd,unsigned char * arr)
{
	int *size=NULL;//NULL
	int rec;
	int len=230; 
	rec = libusb_bulk_transfer(hd, 2,arr,len, size, 0);   //type :1 为中断，2 为批量
	if(rec==0){
		return 1;//发送成功
	}
  return 0; 
}
 	
 int rev_bulk(libusb_device_handle *hd){
	int *size=NULL;	
	unsigned char revbulk[1024]={0};
	int len=230;
	int rec=libusb_bulk_transfer(hd, 0x81, revbulk, len, size, 0);//type=0批量传输，type=1中断传输
	if(rec==0){//接受成功
		
	}
	return 0;
 } */

void TestPlugin::Execute(std::string action, std::string callback_id, std::string args) {
  logAtelier("TestPlugin::Executexxxxxxxxxxxxx !!!  action: %s",action.c_str());
  logAtelier("TestPlugin::Executexxxxxxxxxxxxx !!!  action: %s",args.c_str());
   if(action=="sendMusic"){
	   while(num<5){
		if(dev_handle_static==NULL){
		dev_handle_static=init_handle(PID,VID,ctx_static);//打开设备，获取dev_handle
		}else{
		break;
		}
		num++;
	    }  	
		num=0;//归0	
		pthread_mutex_init(&mut,NULL);//初始化锁
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
		closeAndExit(dev_handle_static,ctx_static);//关闭
		plugin_manager->PostMsgToJs(callback_id, true, 1,"关闭USB成功", false);
	}
   if(action=="interrupt_rev"){		
	    plugin_manager->PostMsgToJs(callback_id, true, 1,"打开设备失败", false); 	
    }	
}
}//namespace atelier_plugin
