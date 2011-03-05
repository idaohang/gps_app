/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2006 版权所有
  文件名 ：SG_Camera.c
  版本   ：1.00
  创建者 ：陈海华
  创建时间：2006-3-17
  内容描述：SG2000 摄像头信息接收分析     
  修改记录：
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sci_types.h"
#include "SG_Receive.h"
#include "tb_hal.h"
#include "SG_Camera.h"
#include "SG_Set.h"
#include "SG_Gps.h"
#include "msg.h"
#include "sio.h"
#include "SG_Timer.h"
#include "SG_Ext_Dev.h"
#include "Efs.h"
#include "mmi_descontrol.h"


unsigned char*Camera_Buf=NULL;
int Camera_BufCnt =0;
int needTest = 1;
int IndexCnt = 0;

SG_CAMERA CurrCamera;
int ReGetIndexCnt;
uint32 t_camera = 0xffffffff;// max uint32	
#define CAMERA_TASK_LIST_NUM 100


void camera_timout_check(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	
	if(!(g_set_info.bNewUart4Alarm & (UT_XGCAMERA|UT_QQCAMERA)))
		return;
	
	if(CheckTimeOut(t_camera))
	{
		SG_CreateSignal(SG_CAMERA_TIMEOUT,0, &signal_ptr); 
	}
}
/****************************************************************
  函数名：camera_construct_cmd
  功  能  ：通过串口向摄像头发送命令
  输入参数：发送命令的参数
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void camera_construct_cmd(uint8 id,uint8 p1,uint8 p2,uint8 p3,uint8 p4){
	uint8 dst[6];
	int i = 0;
	
	dst[0] = 0xAA;
	dst[1] = id;
	dst[2] = p1;
	dst[3] = p2;
	dst[4] = p3;
	dst[5] = p4;

#if __camera_debug_	
	for(i=0;i<6;i++)
		HAL_DumpPutChar(COM_DATA,dst[i]);
#else	
	SIO_WriteFrame(COM_DEBUG,dst,6);
#endif	
	return;
}

/****************************************************************
  函数名：camera_reset
  功  能  ：摄像头复位
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void camera_reset(void){

	memset(Uart_Read_Buf,0,sizeof(Uart_Read_Buf)); // 清空读取数据的buffer
	memset(Camera_Buf,0,sizeof(Camera_Buf)); // 清空存放图片的buffer
	Camera_BufCnt = 0;        
//	camera_construct_cmd(0x08,0x00,0x00,0x00,0xff);// 发送摄像头复位命令
	CurrCamera.currst = CAMERA_RESET;
	HAL_SetGPIOVal(PWR_SB,0);					// 关断摄像头电源
	t_camera =GetTimeOut(3);//restart timer
	
	return;
}

/****************************************************************
  函数名：camera_cmd_delete
  功  能  ：删除任务队列中的一个节点
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void camera_cmd_delete(void){
	CAMERA_CMD *tmp = NULL;

	if(CurrCamera.head == NULL)
	{
		SCI_TRACE_LOW("camera_cmd_delete: Head is NULL!!!\n");
		return;
	}
	
	tmp = CurrCamera.head->next;
	if(tmp == NULL)
	{
		SCI_TRACE_LOW("camera_cmd_delete: NO Item In List!!!\n");
		SCI_FREE(CurrCamera.head);
		CurrCamera.head = NULL;
		return;
	}
	else
	{
		tmp = CurrCamera.head;
		CurrCamera.head = CurrCamera.head->next;
		SCI_FREE(tmp);
		return;
	}
	return;
}

int Camera_Get_Pic_IamgeQuality(void)
{
	int ImageQuality;


	switch(CurrCamera.head->para)
	{
		case 0://较差(80*64)
			ImageQuality = 1;
			break;
		case 1://差(160*128)
			ImageQuality = 3;
			break;
		case 5:	//好(640*480) // 不允许设置质量为好的图片
			ImageQuality = 7;
			break;
		default://一般(320*240)
			ImageQuality = 5;
			break;
			
	}
	if(CurrCamera.head->saveflag == PIC_SAVE)
	{
		if(ImageQuality > 5)
			ImageQuality = 5;
	}
	return ImageQuality;

}

/****************************************************************
  函数名：camera_run
  功  能  ：启动拍摄图片的状态机
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void camera_run(void){
	int ImageQuality;
	
	if(CurrCamera.head == NULL)
	{
		SCI_TRACE_LOW("<<<<The List is Empty!!!!");
		return;
	}
	if(!(CurrCamera.currst == CAMERA_START || CurrCamera.currst == CAMERA_IDLE || CurrCamera.currst == CAMERA_WAIT))
	{
		SCI_TRACE_LOW("<<<<The State is busy!!! State = %d",CurrCamera.currst);
		return;
	}
	HAL_SetGPIOVal(PWR_SB,1);					// 设置PWR_SB 引脚低

	ImageQuality = Camera_Get_Pic_IamgeQuality();
	
	if(CurrCamera.currst == CAMERA_START || CurrCamera.currst == CAMERA_WAIT){   // 开始状态发送同步命令，与摄像头同步
		HAL_SetGPIOVal(PWR_SB,1);					// 打开摄像头电源
		CurrCamera.currst = CAMERA_WAIT_START;
		t_camera = GetTimeOut(2);
		
#if (0)
		CurrCamera.currst = CAMERA_SYNC;//等待摄像头应答
		t_camera = GetTimeOut(20);
		SCI_TRACE_LOW("<<<<Send sync msg!!!");
		camera_construct_cmd(0x0D,0X00,0X00,0X00,0X00); // 主机向摄像头发送同步
#endif /* (0) */
	}
	else if(CurrCamera.currst == CAMERA_IDLE){ // 连拍时直接发送init命令开始拍照
		CurrCamera.currst = CAMERA_INIT;//等待摄像头应答
		t_camera = GetTimeOut(CAMERA_DEFAULT_TIMEOUT);
		SCI_TRACE_LOW("<<<<Send init msg!!!");
		camera_construct_cmd(0x01,0x00,0x07,0x03,ImageQuality);// 主机向摄像头发送初始化命令(320*240)
	}
	return;
}

/****************************************************************
  函数名：camera_cmd_add
  功  能  ：添加一个任务到拍摄图片的任务队列中
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void camera_cmd_add(int type,uint32 para,int save_flag)
{
	int i=0;
	CAMERA_CMD *cmd =NULL,*tmp = NULL;
	

	cmd = SCI_ALLOC(sizeof(CAMERA_CMD));
	if(cmd == NULL)
		return;
	
	cmd->cmd = type;
	cmd->para = para;
	cmd->saveflag = save_flag;
	cmd->next = NULL;
	
	if(CurrCamera.head == NULL){
		CurrCamera.head = cmd;
		return;
	}
	tmp = CurrCamera.head;
	while(tmp->next && i< CAMERA_TASK_LIST_NUM){
		i++;
		tmp = tmp->next;
	}
	if(i >= CAMERA_TASK_LIST_NUM){
		SCI_Free(cmd);
	}
	else
		tmp->next = cmd;

	return;
}

/****************************************************************
  函数名：camera_task_count
  功  能  ：获取当前拍摄队列中的任务数
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
int camera_task_count(void)
{
	int i=0;
	CAMERA_CMD *cmd =NULL,*tmp = NULL;
	
	tmp = CurrCamera.head;
	if(tmp == NULL)
	{
		SCI_TRACE_LOW("<<<<<<The List is Empty!!");
		return 0;
	}
	else
	{
		i++;
		while(tmp->next && i< CAMERA_TASK_LIST_NUM)
		{
			i ++;
			tmp = tmp->next;
		}
		SCI_TRACE_LOW("<<<<<<There are %d items in List!!",i);
		return i;
	}
	return 0;
}
/****************************************************************
  函数名：sg_camera_request
  功  能  ：摄像头添加一个任务到队列，并唤醒拍摄图片的状态机流程
  输入参数：quality 拍摄图片的质量
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void sg_camera_request(int quality,int cmd,int save_flag)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	SCI_TraceLow("---GET ONE REQUEST  quality = %d cmd = %d save_flag %d",quality,cmd,save_flag);
	camera_cmd_add(cmd,quality,save_flag);
	

	SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文

}

/****************************************************************
  函数名：SG_Camera_Task_Check
  功  能  ：检查是否还有未监控完的任务
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
BOOLEAN SG_Camera_Task_Check(int task)
{

	if(g_set_info.sg_camera[task].setCount == 0)
		return TRUE;


	if(g_set_info.sg_camera[task].nCount > 0)
			return TRUE;
		else 
			return FALSE;
}


/****************************************************************
  函数名：SG_Camera_Interval_Check
  功  能  ：返回摄像头的监控间隔
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
int SG_Camera_Interval_Check(void)
{
	if(CurrCamera.head->cmd == CAMERA_TIME_WATCH_REQ) // 实时监控
	{
		return g_set_info.sg_camera[0].nInterval;
	}
	else  if(CurrCamera.head->cmd == CAMERA_CONDITION_WATCH_REQ)// 条件监控
	{
		return g_set_info.sg_camera[1].nInterval;
	}
	
	return 0xffff;  // 单次图像监控返回一个大数字
}

/****************************************************************
  函数名：SG_Camera_Task_Delete
  功  能  ：删除摄像头已经执行的任务
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void SG_Camera_Task_Delete(int task)
{
	SG_Set_Cal_CheckSum();
	if(task == CAMERA_TIME_WATCH_REQ) // 实时监控
	{
		if(g_set_info.sg_camera[0].nCount)
			g_set_info.sg_camera[0].nCount --;
	}
	else if(task == CAMERA_CONDITION_WATCH_REQ)// 条件监控  
	{
		if(g_set_info.sg_camera[1].nCount)
			g_set_info.sg_camera[1].nCount--;
	}
	SG_Set_Save();
}

/****************************************************************
  函数名：Camera_Condition_Watch
  功  能  ：摄像头条件监控判断
  输入参数：无
  输出参数：无
  编写者  ：郭碧莲
  修改记录：创建 2008/04/25 
****************************************************************/
void Camera_Condition_Watch(void)
{
	static int oldAlarmStatus=0,newAlarmStatus=0;

	if(!(g_set_info.bNewUart4Alarm & (UT_XGCAMERA|UT_QQCAMERA)) || ! (g_set_info.nCameraAlarm & (~0x01)))
		return; // 未启用摄像头或者未启用条件监控，立即返回
			//条件监控请求图片或者条件定次请求图片
							
		newAlarmStatus=(g_set_info.nCameraAlarm & g_state_info.alarmState&~(0x01));	
		SCI_TRACE_LOW("OLD %x new %x cam  %x as %x count %d",oldAlarmStatus,newAlarmStatus,g_set_info.nCameraAlarm,g_state_info.alarmState,g_set_info.sg_camera[1].nCount);
				
		if((oldAlarmStatus | newAlarmStatus)!=oldAlarmStatus)//新报警触发
		{		
			SG_Set_Cal_CheckSum();
			CameraConditionTimer = GetTimeOut(0); // 重置监控时间，一设置下去，立刻上拍图片

//			if(g_set_info.sg_camera[1].setCount == 0)
//				g_set_info.sg_camera[1].nCount = 3;
//			else
			g_set_info.sg_camera[1].nCount = g_set_info.sg_camera[1].setCount;
			SCI_TRACE_LOW("OLD %x new %x cam  %x as %x count %d",oldAlarmStatus,newAlarmStatus,g_set_info.nCameraAlarm,g_state_info.alarmState,g_set_info.sg_camera[1].nCount);
			SG_Set_Save();
		}

		oldAlarmStatus = newAlarmStatus;	
}

int  Camera_Save(unsigned char *Msg, int len,int type)
{
	char path[20] = "";
	SCI_FILE *fp = NULL;
	int ret;
	
#if (0)
	if(len <= 0)
	{
		SCI_TRACE_LOW("<<<Camera_Save: LEN ERR!!");
		return -1;
	}
	
	SCI_TRACE_LOW("--camera save: len %d type %d index %d",len,type,g_set_info.nPictureSaveIndex);
	if(type) //如果是新开一个图片存储
	{
		SG_Set_Cal_CheckSum();
		g_set_info.nPictureSaveIndex ++;
		if(g_set_info.nPictureSaveIndex >= SG_PICTURE_SAVE_MAX_FILE)
		{
			g_set_info.nPictureSaveIndex = 0;
		}
		SG_Set_Save();
		sprintf(path,"pic%d.set",g_set_info.nPictureSaveIndex);
		SCI_TRACE_LOW("--Camera_Save: NV ITEM %d",MMINV_PIC0_INFO + g_set_info.nPictureSaveIndex);
		ret = MNNV_WriteItem(MMINV_PIC0_INFO+g_set_info.nPictureSaveIndex,20000,Msg);
	
	 	if(ret != ERR_MNDATAMAG_NO_ERR)
	  	{		
			SCI_TRACE_LOW( "--Camera_Save:  MNNV_WriteItem error ret %d",ret);		
			return 0;
		}
		
		SG_Set_Cal_CheckSum();
		g_set_info.camera_save_list[g_set_info.nPictureSaveIndex].pic_save = 1;
		SG_Set_Save();

	}
	else
	{
		sprintf(path,"pic%d.set",g_set_info.nPictureSaveIndex);
		fp = EFS_fopen(path, "a");
		if (fp == NULL)
		{		
			SCI_TRACE_LOW( "--Camera_Save2: CREATE FILE FAIL\r\n");	
			return 0;
		}	

		//先写入长度
		if(EFS_fwrite((char *)&len,4, 1, fp) != 1)
		{
			EFS_fclose(fp);
			SCI_TRACE_LOW( "--Camera_Save2: SAVE FILE len FAIL\r\n");	
			return 0;
		}

		//再写入图片包包
		if(EFS_fwrite(Msg,len, 1, fp) != 1)
		{
			EFS_fclose(fp);
			SCI_TRACE_LOW( "--Camera_Save2: SAVE FILE Msg FAIL\r\n");	
			return 0;
		}
		
		EFS_fclose(fp);
		SG_Set_Cal_CheckSum();
		g_set_info.camera_save_list[g_set_info.nPictureSaveIndex].pic_save ++;
		SG_Set_Save();
	}
//	SCI_TRACE_LOW("--Camera_Save:path %s",path);
	SCI_TRACE_LOW("--Camera_Save:index %d savelist %d",g_set_info.nPictureSaveIndex,
		g_set_info.camera_save_list[g_set_info.nPictureSaveIndex].pic_save);
#endif /* (0) */

	if(len <= 0)
	{
		SCI_TRACE_LOW("<<<Camera_Save: LEN ERR!!");
		return -1;

	}
	SCI_TRACE_LOW("--camera save: len %d type %d index %d",len,type,g_set_info.nPictureSaveIndex);
	if(type) //如果是新开一个图片存储
	{
		SG_Set_Cal_CheckSum();
		g_set_info.nPictureSaveIndex ++;
		if(g_set_info.nPictureSaveIndex >= SG_PICTURE_SAVE_MAX_FILE)
		{
			g_set_info.nPictureSaveIndex = 0;
		}
		SG_Set_Save();
		sprintf(path,"pic%d.set",g_set_info.nPictureSaveIndex);
		
		if(g_set_info.camera_save_list[g_set_info.nPictureSaveIndex].pic_save)
		{
			EFS_Delete(path);
		}	

		fp = EFS_fopen(path, "wb");
		if (fp == NULL)
		{		
			SCI_TRACE_LOW( "--Camera_Save: CREATE FILE FAIL\r\n");	
			return 0;
		}	
		g_state_info.GpsEnable = 1;
		//先写入长度
		if(EFS_fwrite((char *)&len,4, 1, fp) != 1)
		{
			
			SCI_TRACE_LOW( "--Camera_Save: SAVE FILE len FAIL\r\n");	
			goto err;
		}

		//再写入图片包包
		if(EFS_fwrite(Msg,len, 1, fp) != 1)
		{
			SCI_TRACE_LOW( "--Camera_Save: SAVE FILE Msg FAIL\r\n");	
			goto err;

		}
		EFS_fclose(fp);
		g_state_info.GpsEnable = 0;

		SG_Set_Cal_CheckSum();
		g_set_info.camera_save_list[g_set_info.nPictureSaveIndex].pic_save = 1;
		SG_Set_Save();

	}
	else
	{
		sprintf(path,"pic%d.set",g_set_info.nPictureSaveIndex);
		fp = EFS_fopen(path, "a");
		if (fp == NULL)
		{		
			SCI_TRACE_LOW( "--Camera_Save2: CREATE FILE FAIL\r\n");	
			return 0;
			
		}	
		g_state_info.GpsEnable = 1;

		//先写入长度
		if(EFS_fwrite((char *)&len,4, 1, fp) != 1)
		{
			SCI_TRACE_LOW( "--Camera_Save2: SAVE FILE len FAIL\r\n");	
			goto err;
		}

		//再写入图片包包
		if(EFS_fwrite(Msg,len, 1, fp) != 1)
		{
			EFS_fclose(fp);
			SCI_TRACE_LOW( "--Camera_Save2: SAVE FILE Msg FAIL\r\n");	
			goto err;
		}
		g_state_info.GpsEnable = 0;
		
		EFS_fclose(fp);
		SG_Set_Cal_CheckSum();
		g_set_info.camera_save_list[g_set_info.nPictureSaveIndex].pic_save ++;
		SG_Set_Save();
	}
	SCI_TRACE_LOW("--Camera_Save:path %s",path);
	SCI_TRACE_LOW("--Camera_Save:index %d savelist %d",g_set_info.nPictureSaveIndex,
		g_set_info.camera_save_list[g_set_info.nPictureSaveIndex].pic_save);

	return 1;
err:
	EFS_fclose(fp);
	g_state_info.GpsEnable = 0;
	return 0;
}

void Camera_Read_Up(void)
{
	static char readindex = 0;
	SCI_FILE *fp = NULL;
	char path[20] = "";
	char *picdata = NULL;
	int picdatalen = 0;

//	picdata = SCI_ALLOC(11 *1024);
	
	SCI_TRACE_LOW("--Camera_Read_Up: %d ",readindex);
	sprintf(path,"pic%d.set",readindex);

	if(g_set_info.camera_save_list[readindex].pic_save)
	{
		fp = EFS_fopen(path, "rb");
		if (fp == NULL)
		{		
			SCI_TRACE_LOW( "--Camera_Read_Up: CREATE FILE FAIL\r\n");	
			return;
		}	
		g_state_info.GpsEnable = 1;

		while(g_set_info.camera_save_list[readindex].pic_save)
		{
			if (EFS_fread((char *)&picdatalen, 4, 1, fp) != 1)
			{
				SCI_TRACE_LOW( "--Camera_Read_Up: read len FAIL\r\n");
				goto err;
			}
			if(picdatalen == 0)
			{		
				SCI_TRACE_LOW( "--Camera_Read_Up: len zero!!!\r\n");
				goto err;
				
			}
			picdata = SCI_ALLOC(picdatalen +1);

			if(EFS_fread(picdata,picdatalen,1, fp) != 1)
			{
				SCI_TRACE_LOW( "--Camera_Read_Up: read picdata FAIL\r\n");
				goto err;
				
			}
			
			SG_Send_Put_New_Item(2,picdata, picdatalen);

			SG_Set_Cal_CheckSum();
			g_set_info.camera_save_list[readindex].pic_save--;
			SG_Set_Save();
		}
		g_state_info.GpsEnable = 0;

		EFS_fclose(fp);
		EFS_Delete(path);
	}
	else
	{
		
	}
	
	readindex ++;
	if(readindex >= SG_PICTURE_SAVE_MAX_FILE)
	{
		readindex = 0;
		PictureUpFlag = 0;
		SG_Set_Cal_CheckSum();
		g_set_info.nPictureSaveIndex = 0;
		SG_Set_Save();
	} 
	SCI_TRACE_LOW("--Camera_Read_Up: %d  %d",readindex,PictureUpFlag);

	return;
	err:
		g_state_info.GpsEnable = 0;

		PictureUpFlag = 0;
		EFS_fclose(fp);
		EFS_Delete(path);
		SG_Set_Cal_CheckSum();
		g_set_info.camera_save_list[readindex].pic_save= 0;
		SG_Set_Save();
		readindex = 0;
}

// 每隔2秒检查队列里面米有东东的时候再放一张图片进去
void Camera_Up_Pic_Judge(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	if(PictureUpFlag == 1)
	{
		if(SecTimer %2 == 0)
		{
			int ret = 0;
			ret = SG_Send_Cal_Pic_Item();
			if(ret>1)
			{
				SCI_TRACE_LOW("<<<<Too Many Pack in list %d!!",ret);
			}
			else
			{
				SG_CreateSignal(SG_PIC_UP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
			}

		}
	}

}

int Sio_Camera_Read(uint32 port,uint8  *src, int  size,uint32 *dest_len)
{
	int ch = -1;
	int readlen = 0;
	
	while((ch=SIO_GetChar(port)) >= 0)
	{
		src[readlen++] = ch;
	}
	
	*dest_len = readlen;
	return readlen;
}



	

/****************************************************************
  函数名：SG_Lock_Uart_Read
  功  能  ：串口读函数
  输入参数：
  输出参数：无
  编写者  ：陈海华
  修改记录：创建，2006-7-7
****************************************************************/
char SG_Camera_Uart_Read_Picture(unsigned char *readbuffer,int  len)
{
   static int  bytes_left =0;
   static int  bytes_read =0;
   unsigned char *ptr=readbuffer;
   int readdatalen = 0;
   int rcvlen;

	bytes_left=len - bytes_read;
	ptr += bytes_read;

#if __camera_debug_	
	rcvlen = Sio_Camera_Read(COM_DATA,(uint8*)ptr,bytes_left,(uint32*)&readdatalen);
#else
	rcvlen = SIO_ReadFrame(COM_DEBUG,(uint8*)ptr,bytes_left,(uint32*)&readdatalen);
#endif
	if(rcvlen <= 0)
	{
		return 0; // 未收到任何数据，返回
	}
	else
	{  
		bytes_read += rcvlen;
		bytes_left-=rcvlen;
		SCI_TRACE_LOW("---rcvlen %d  bytes_left %d bytes_read %d len %d\r\n",rcvlen,bytes_left,bytes_read,len);
		if(bytes_left == 0)
		{
			bytes_read = 0;
			return 1; // 接收完毕，ok
		}
		else
		{
			return 2; // 还没有完毕
		}
	}
}


void Camera_Watch_By_Time(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	char sig[3];
	

	if(!(g_set_info.bNewUart4Alarm & (UT_XGCAMERA|UT_QQCAMERA)) || !g_set_info.nCameraAlarm) // 未启用
		return;
	
	if(g_set_info.nCameraAlarm & 0x01) // 实时监控方式
	{
		if(CheckTimeOut(CameraWatchTimer))
		{
			CameraWatchTimer = GetTimeOut(g_set_info.sg_camera[0].nInterval);
			SCI_TRACE_LOW("<<<<<<Camera ask time watch count %d  time %d",g_set_info.sg_camera[0].nCount,CameraWatchTimer);
			if(SG_Camera_Task_Check(CAMERA_TIME_WATCH_REQ))
			{
				SG_Camera_Task_Delete(CAMERA_TIME_WATCH_REQ);
				sig[0] = g_set_info.sg_camera[0].nImageQuality;
				sig[1] = CAMERA_TIME_WATCH_REQ;
				sig[2] = PIC_NOT_SAVE;
				SCI_TRACE_LOW("<<<<<<Camera ask time watch count %d  imagequality %d",g_set_info.sg_camera[0].nCount,g_set_info.sg_camera[0].nImageQuality);
				SG_CreateSignal_Para(SG_CAMERA_MORE_ASK,3, &signal_ptr,sig); // 发送信号到主循环要求处理发送队列中的报文
			}
		}
	}
 	if (g_set_info.nCameraAlarm & (~0x01)) // 条件监控
	{
		if(CheckTimeOut(CameraConditionTimer))
		{
			SCI_TRACE_LOW("<<<<<<Camera ask condition watch count %d  time %d",g_set_info.sg_camera[1].nCount,CameraConditionTimer);
			CameraConditionTimer = GetTimeOut(g_set_info.sg_camera[1].nInterval);
			if(SG_Camera_Task_Check(CAMERA_CONDITION_WATCH_REQ))
			{
				SG_Camera_Task_Delete(CAMERA_CONDITION_WATCH_REQ);
				sig[0] = g_set_info.sg_camera[1].nImageQuality;
				sig[1] = CAMERA_CONDITION_WATCH_REQ;
				if(g_set_info.bPictureSave & g_set_info.nCameraAlarm)
				{
					sig[2] = PIC_SAVE;
				}
				else
				{
					sig[2] = PIC_NOT_SAVE;
				}
				// 加入是否保存图片
				SCI_TRACE_LOW("<<<<<<Camera ask condition watch count %d  imagequality %d save flag %x nCameraAlarm %x"
				,g_set_info.sg_camera[1].nCount,g_set_info.sg_camera[1].nImageQuality,g_set_info.bPictureSave,g_set_info.nCameraAlarm);
				SG_CreateSignal_Para(SG_CAMERA_MORE_ASK,3, &signal_ptr,sig); // 发送信号到主循环要求处理发送队列中的报文
			}
		}
	}
	
}



void camera_timer_handle(){
	xSignalHeaderRec      *signal_ptr = NULL;
	int ImageQuality;

	//restat timer
	switch(CurrCamera.currst){
		case CAMERA_SYNC:
			t_camera = GetTimeOut(20);//restart timer
			break;
		case CAMERA_WAIT:
			t_camera = GetTimeOut(10);
			break;
		default:
			t_camera =GetTimeOut(CAMERA_DEFAULT_TIMEOUT);//restart timer
			break;
	}
	
	switch(CurrCamera.currst){
		case CAMERA_GET_PIC:
		case CAMERA_GET_XINDEX_PIC:
//			camera_construct_cmd(0x0E, 0x00, 0x00, ReGetIndexCnt, 0x00);//snd msg again
			CurrCamera.currst = CAMERA_START;
			HAL_SetGPIOVal(PWR_SB,0);					// 关断摄像头电源
			SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
//			camera_reset();
			break;
		case CAMERA_SYNC:
			camera_reset();
			break;
		case CAMERA_WAIT: // 等待状态每10s唤醒一次
			SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
			break;
		case CAMERA_START:
		case CAMERA_IDLE:
			break;
		case CAMERA_WAITINIT:
			ImageQuality = Camera_Get_Pic_IamgeQuality();
			CurrCamera.currst = CAMERA_INIT;		
			camera_construct_cmd(0x01,0x00,0x07,0x03,ImageQuality);// 主机向摄像头发送初始化命令(320*240)// set pic attr
			break;
		case CAMERA_RESET:
			CurrCamera.currst = CAMERA_START;
//			HAL_SetGPIOVal(PWR_SB,0);					// 关断摄像头电源
			SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
			break;
		case CAMERA_WAIT_START:// 上电后2s，再发送同步消息
			CurrCamera.currst = CAMERA_SYNC;//等待摄像头应答
			t_camera = GetTimeOut(20);
			SCI_TRACE_LOW("<<<<Send sync msg!!!");
			camera_construct_cmd(0x0D,0X00,0X00,0X00,0X00); // 主机向摄像头发送同步
			break;	
		default:
			camera_construct_cmd(0x0D,0X00,0X00,0X00,0X00); // 主机向摄像头发送同步
			if(CurrCamera.currst != CAMERA_SYNC)
				CurrCamera.currst = CAMERA_SYNC;
			t_camera = GetTimeOut(20);			
			break;
	}

}



void Camera_Uart_Handle(){
	int rcvLen =0; // 当前接收到的数据长度
	unsigned char* fb=NULL;
	static unsigned char IndexCnt;
	static unsigned int CntLen=0;  // 图像的总长度
	static unsigned int Packcnt = 0;  // 总共需要读取的包数
	static unsigned long RxCnt=0;  // 已经读取的图像长度
	unsigned char cmdbuf[20] = {0};
	int cmdbufcnt = 0;
	unsigned char CRC1=0,CRC2=0;
	int i =0;
	static unsigned char IndexError = 0;
	xSignalHeaderRec      *signal_ptr = NULL;
	int ImageQuality;



	if(CurrCamera.currst == CAMERA_GET_XINDEX_PIC)  // 当前在获取图片数据时
	{

		if(IndexCnt == Packcnt )  			// 获取最后一包
		{
			if(SG_Camera_Uart_Read_Picture(Uart_Read_Buf,CntLen-RxCnt+6)!= 1)
				return;
			t_camera =GetTimeOut(4);//restart timer
	
			SCI_TRACE_LOW("----get last packet IndexCnt %d packcnt %d\r\n",
				IndexCnt,Packcnt,CntLen,RxCnt);

			// 计算校验和
			CRC1=0;
	  		CRC2=0;
	  		for(i=0;i<CntLen-RxCnt+6-2;i++)
	  		{
	  			CRC1+=Uart_Read_Buf[i];
	  		}
	  		CRC2=Uart_Read_Buf[CntLen-RxCnt+6-2];
	  		SCI_TRACE_LOW("Camera: CRC1 %d CRC2 %d\r\n",CRC1,CRC2); 

			if(CRC1!=CRC2)
	  		{	
				// 校验和不正确 重新发送本次命令
				IndexError ++;
				if(IndexError > 3)// 超过3次发送摄像头reset命令
				{
					SG_CreateSignal(SG_CAMERA_RESET,0, &signal_ptr);
					return;
				}
				ReGetIndexCnt = IndexCnt;
				SG_CreateSignal(SG_CAMERA_TIMEOUT,0, &signal_ptr); 
				return;
	  		}
			else
			{
				IndexError = 0;
				
				memcpy(&Camera_Buf[g_state_info.CameraOffset + RxCnt],&Uart_Read_Buf[4],CntLen-RxCnt+6-2);
				memset(Uart_Read_Buf,0,sizeof(Uart_Read_Buf));
				if(SG_Camera_Interval_Check() <= 10) //低于十秒不做同步
				{
					CurrCamera.currst = CAMERA_IDLE;
				}
				else  // 回到初始状态
				{
					CurrCamera.currst = CAMERA_START;
					HAL_SetGPIOVal(PWR_SB,0);					// 关断摄像头电源
	
				}
				camera_construct_cmd(0x0E, 0x00, 0x00, 0xF0, 0xF0);// 完成图像传输
				IndexCnt = 0;
				SCI_TRACE_LOW("<<<<Get a hole picture !!!");
				g_state_info.photo_catch_cnt ++;
				// 这里post一个消息出来到main处理组消息包
				SG_CreateSignal(SG_CAMERA_MSGMAKE,0, &signal_ptr); 				
				
			}

		}
		else 
		{
			if(SG_Camera_Uart_Read_Picture(Uart_Read_Buf,512)!= 1)
				return;
			t_camera =GetTimeOut(4);//restart timer

			// 获取前面的包
			SCI_TRACE_LOW("----get a packet IndexCnt %d packcnt %d\r\n",IndexCnt,Packcnt);
			// 计算校验和
			CRC1=0;
	  		CRC2=0;
	  		for(i=0;i<512-2;i++)
	  		{
	  			CRC1+=Uart_Read_Buf[i];
	  		}
	  		CRC2=Uart_Read_Buf[512-2];
	  		SCI_TRACE_LOW("Camera: CRC1 %d CRC2 %d\r\n",CRC1,CRC2); 
	  		if(CRC1!=CRC2)
	  		{	
				// 校验和不正确 重新发送本次命令
				IndexError ++;
				if(IndexError > 3)// 超过3次发送摄像头reset命令
				{
					SG_CreateSignal(SG_CAMERA_RESET,0, &signal_ptr);
					return;
				}
				ReGetIndexCnt = IndexCnt;
				SG_CreateSignal(SG_CAMERA_TIMEOUT,0, &signal_ptr); 
				return;
	  		}
			else
			{
				IndexError = 0;
				memcpy(&Camera_Buf[g_state_info.CameraOffset+ RxCnt],&Uart_Read_Buf[4],512-6);

				memset(Uart_Read_Buf,0,sizeof(Uart_Read_Buf));
				RxCnt+=512-6;
				IndexCnt ++;
				camera_construct_cmd(0x0E, 0x00, 0x00, IndexCnt, 0x00);// 获取下一包图片数据
			}
		}
	}
	else // 接收来自摄像头的命令
	{
#if __camera_debug_		
		rcvLen = Sio_Camera_Read(COM_DATA,(uint8*)cmdbuf,20,(uint32*)&cmdbufcnt);
#else
		rcvLen = SIO_ReadFrame(COM_DEBUG,(uint8*)cmdbuf,20,(uint32*)&cmdbufcnt);
#endif
		if(rcvLen <= 0)
			return;

#if (0)
		for(i = 0;i<rcvLen;i++)
		{
			SCI_TRACE_LOW("--%02x",cmdbuf[i]);
		}
#endif /* (0) */
		
		fb = FindChar(cmdbuf,cmdbufcnt,0xaa);		
		if(fb == NULL)		
		{			
			SCI_TRACE_LOW("====== fb IS NULL ==== ");			
			return;		
		}		
		else		
		{			
			//SCI_TRACE_LOW("======fb[0]:%02x  fb[1]:%02x fb[2]:%02x", fb[0],fb[1],fb[2]);
			while(fb != NULL)
			{
				switch(fb[1])
				{
					case 0x0a:  // 得到拍摄图片的长度信息
					SCI_TRACE_LOW("<<<<<< STATE %d",CurrCamera.currst);
						if(CurrCamera.currst == CAMERA_GET_PIC){
							t_camera =GetTimeOut(10);//restart timer
							CurrCamera.currst =CAMERA_GET_XINDEX_PIC;
							IndexCnt = 0;
							CntLen = 0;
							RxCnt = 0;
							//parse x index pic info
							SCI_TRACE_LOW("<<<<Get picture len from camera,ask for next packet!!!!");

							SCI_MEMCPY((unsigned char*)&CntLen,&cmdbuf[3],3);
							CntLen = htonl(CntLen);
							// 这里要加入对图片长度的判断，若图片长度超过20k，则丢弃此张图片，再拍一张
 							Camera_BufCnt = CntLen;
						#if __cwt_
							g_state_info.CameraOffset = 0;
						#else
							if(Camera_BufCnt > CAMERA_ONE_PIC_LEN) // 大于15k的要分包
							{
								g_state_info.CameraOffset = CAMERA_OFFSET + 2; 
							}
							else // 小于等于15k的和过去一样
							{
								g_state_info.CameraOffset = CAMERA_OFFSET;
							}
						#endif
							if(CntLen%506 == 0)
							{
								Packcnt = CntLen/506-1;
							}
							else
							{
								Packcnt = CntLen/506;
							}
							SCI_TRACE_LOW("&&&&&&CntLen %d Packcnt %d CameraOffset %d \n",CntLen,Packcnt,g_state_info.CameraOffset);
							camera_construct_cmd(0x0E, 0x00, 0x00, IndexCnt, 0x00);// 主机向摄像头发送信息收取下一包图片数据
						}
						return;
					case 0x0d:   // 得到来自摄像头的同步消息
						if(CurrCamera.currst == CAMERA_WAITSYNC){
							t_camera =GetTimeOut(4);// restart timer 
							ImageQuality = Camera_Get_Pic_IamgeQuality();
							camera_construct_cmd(0x0E,0x0D,0x00,0x00,0x00);// 主机应答摄像头同步消息
							CurrCamera.currst = CAMERA_WAITINIT;
//							CurrCamera.currst = CAMERA_INIT;		
//							camera_construct_cmd(0x01,0x00,0x07,0x03,ImageQuality);// 主机向摄像头发送初始化命令(320*240)// set pic attr
							SCI_TRACE_LOW("<<<<Get sync msg from camera,go to init mode !!!!");
						}
						return;
						break;
					case 0x0e://  ACK处理

						switch (fb[2]){
							case 0x0d:
								if(CurrCamera.currst == CAMERA_SYNC){
									t_camera =GetTimeOut(CAMERA_DEFAULT_TIMEOUT);// restart timer 
									CurrCamera.currst = CAMERA_WAITSYNC;
									SCI_TRACE_LOW("<<<<Get sync answer from camera in normal mode!!!!");
								}
								break;
							case 0x01:// 初始化应答

								if(CurrCamera.currst == CAMERA_INIT){
									t_camera =GetTimeOut(5);//restart timer
									SCI_TRACE_LOW("<<<<Get init ans from camera,go to set msg len !!!!");
									CurrCamera.currst = CAMERA_WAIT_SET;
#if (0)
									CurrCamera.currst =CAMERA_SET_MSG_LEN;
									camera_construct_cmd(0x06,0x08,0x00,0x02,0x00);// 设置数据包大小(512byte)
#endif /* (0) */
								}
								return;
							case 0x06:// 设置数据包大小应答

								if(CurrCamera.currst == CAMERA_SET_MSG_LEN){
									t_camera =GetTimeOut(CAMERA_DEFAULT_TIMEOUT);//restart timer
									CurrCamera.currst  = CAMERA_WAIT_GET_PIC;
									SCI_TRACE_LOW("<<<<Get set msg len ans from camera,go to wait to get picture!!!!");
//									camera_construct_cmd(0x04,0x05,0x00,0x00,0x00);//aa 04
									camera_construct_cmd(0x05,0x00,0x14,0x00,0x00);
								}
								return;
							case 0x04:// 接收获取图片的应答
								if(CurrCamera.currst == CAMERA_GET_PIC){
									SCI_TRACE_LOW("<<<<<<Get get pic ans, goto get pic len!!!");
									t_camera =GetTimeOut(7);//restart timer
									return;
								}
							case 0x08:  // 复位应答
								// 这里要重新wakeup
								if(CurrCamera.currst == CAMERA_RESET)
								{
									t_camera =GetTimeOut(CAMERA_DEFAULT_TIMEOUT);// restart timer 
									CurrCamera.currst = CAMERA_START;
									SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文
								}
								return;
							case 0x05:// 接收等待获取图片的应答
								if(CurrCamera.currst == CAMERA_WAIT_GET_PIC){
									SCI_TRACE_LOW("<<<<Get wait get pic ans from camera,go to get picture!!!!");
									camera_construct_cmd(0x04,0x01,0x00,0x00,0x00); 
									t_camera =GetTimeOut(7);//restart timer
									CurrCamera.currst = CAMERA_GET_PIC;	
								}
								return;
							default:

								return;
								
						}
						break;
					case 0x0f: // 这里要加入nack的处理
						SG_CreateSignal(SG_CAMERA_RESET,0, &signal_ptr);
						return;
					default:
						return;
				}
				cmdbufcnt -= (fb-cmdbuf);
				memcpy(cmdbuf,&fb[6],cmdbufcnt-6);
				fb = FindChar(cmdbuf,cmdbufcnt,0xaa);
			}
			
		}

	}

}

#if (__cwt_)

void Camera_MsgMake_Handle(void)
{
	unsigned char *pOut;
	int outlen;
		
	if(MsgPictureUpAsk(g_state_info.pGpsCurrent,0,&pOut, &outlen) == MSG_TRUE)
	{
		//发送报文到发送队列
		SG_Send_Put_New_Item(2, pOut, outlen);
	}
	
}

#else

void Camera_MsgMake_Handle(void)
{
	unsigned char *pOut;
	int outlen;
	unsigned
	char packcnt = 0;
	char packno = 0;

	if(strlen(g_set_info.sOwnNo) == 0)
	{
		return;// 本机号为配置时不保存或者是放到队列，仅作为测试
	}

	if(Camera_BufCnt > CAMERA_ONE_PIC_LEN) // 图片大于15k分包放置
	{
#if (1)
		packcnt = Camera_BufCnt/CAMERA_ONE_PACK_LEN + 1;
		for(packno = 1;packno < packcnt +1;packno++)
		{
			SCI_TRACE_LOW("<<<<<<packcnt %d packno %d",packcnt,packno);
			if(packno != packcnt) // 不是最后一包
			{
				if(MsgCameraAck(Camera_Buf+(packno - 1)*CAMERA_ONE_PACK_LEN, CAMERA_ONE_PACK_LEN, &pOut,&outlen,packno, packcnt))
				{
					if(CurrCamera.head->saveflag == PIC_SAVE)
					{
						if(packno == 1)
						{
							Camera_Save(pOut,outlen,1);// 需要保存的话保存标志为1
						}
						else
						{
							Camera_Save(pOut,outlen,0);// 需要保存的话保存标志为1
						}
						if(pOut)
						{
							SCI_FREE(pOut);
							pOut == NULL;
						}
					}
					else
					{		

						//发送报文到发送队列
						SG_Send_Put_New_Item(2, pOut, outlen);
					}
				}
			}
			else
			{
				if(MsgCameraAck(Camera_Buf+(packno - 1)*CAMERA_ONE_PACK_LEN, Camera_BufCnt - (packno - 1)*CAMERA_ONE_PACK_LEN, &pOut,&outlen,packno, packcnt))
				{
					if(CurrCamera.head->saveflag == PIC_SAVE)
					{
						Camera_Save(pOut,outlen,0);// 需要保存的话保存标志为1
						if(pOut)
						{
							SCI_FREE(pOut);
							pOut == NULL;
						}

					}
					else
					{
						//发送报文到发送队列
						SG_Send_Put_New_Item(2, pOut, outlen);
					}

					memset(Camera_Buf,0,sizeof(Camera_Buf)); // 清空存放图片的buffer
					Camera_BufCnt = 0;      


				}
			}
		}
#endif /* (0) */
		
	}
	else // 小于15k的图片仍按照过去的协议发送
	{
		if (MsgCameraAck(Camera_Buf,Camera_BufCnt, &pOut, &outlen,0,0) == MSG_TRUE)
		{  
			if(CurrCamera.head->saveflag == PIC_SAVE) // 需要保存的话保存标志为1
			{
				Camera_Save(pOut,outlen,1);
				if(pOut)
				{
					SCI_FREE(pOut);
					pOut == NULL;
				}

				
			}
			else
			{
				//发送报文到发送队列
				SG_Send_Put_New_Item(2, pOut, outlen);
			}

			memset(Camera_Buf,0,sizeof(Camera_Buf)); // 清空存放图片的buffer
			Camera_BufCnt = 0;    

			
		}
	}
		
}

#endif

void Camera_Wakeup_Handle(void)
{
	int temp;

	temp = camera_task_count();

	if(temp == 0)
	{
		CurrCamera.currst == CAMERA_START;
		t_camera = 0xffffffff;
		return;
	}
	else if(SG_Send_Cal_Pic_Item() > 0)// 队列过长，暂不拍摄(队列中图片超过3张)
	{
		CurrCamera.currst = CAMERA_WAIT;
		t_camera = GetTimeOut(30);
		SCI_TRACE_LOW("<<<<Too Many Item In List!! Waiting For Send!!");
	}
	else
	{
		//青青子木摄像头
		if(g_set_info.bNewUart4Alarm&UT_QQCAMERA)
		{
			QQZM_Camera_Run();
		}
		else if(g_set_info.bNewUart4Alarm&UT_XGCAMERA)
		{
			camera_run();
		}
	}

}

void Camera_Init(void)
{
	uint32 bd =0;

	bd = SIO_GetBaudRate(COM_DEBUG);
	if(bd != BAUD_115200)
		SIO_SetBaudRate(COM_DEBUG, BAUD_115200);
	CurrCamera.currst = CAMERA_START;
	//青青子木摄像头
	if(g_set_info.bNewUart4Alarm&UT_QQCAMERA)
	{
		HAL_SetGPIOVal(PWR_SB, 1);					// 开启摄像头电源
	}
	else if(g_set_info.bNewUart4Alarm&UT_XGCAMERA)
	{
		HAL_SetGPIOVal(PWR_SB, 0);					// 关闭摄像头电源
	}

}

/****************************************************************
  函数名：SG_Camera_Request
  功  能  ：摄像头添加一个任务到队列，并唤醒拍摄图片的状态机流程
  输入参数：quality 拍摄图片的质量
  输出参数：无
  编写者  ：林兆剑
  修改记录：创建 2009/08/6
****************************************************************/
void SG_Camera_Request(int quality,int cmd,int save_flag)
{
	xSignalHeaderRec      *signal_ptr = NULL;
	
	SCI_TRACE_LOW("---GET ONE REQUEST  quality = %d cmd = %d save_flag %d",quality,cmd,save_flag);
	camera_cmd_add(cmd,quality,save_flag);
	SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); // 发送信号到主循环要求处理发送队列中的报文

}


/****************************************************************
  函数名：Camera_QQZM_UartSend
  功  能  ：设置青青子木摄像头模块指令并发送
  输入参数：
  输出参数：
  编写者  ：林兆剑
  修改记录：创建 2009/08/05 
****************************************************************/
int  SG_QQZM_Camera_UartSend(int M,int N ,int P)
{
	int camera_ver = 0;

	unsigned char send_tmp[10] = {0};
	unsigned char  send_camera[20] = {0};
   	int i,j;

	
    send_tmp[0] = 0x7e;      	//帧头
	send_tmp[1] = 0x10;			//设备识别码，默认为10
	send_tmp[2] = M;
	send_tmp[3] = N;
  	send_tmp[4] = P;
      	
    j = 1;
	send_camera[0] = 0x7e;

	//转义
  	for(i=1;i<5;i++)
  	{
  		if((send_tmp[i] == 0x7e)||(send_tmp[i]==0x1b))
  		{
			send_camera[j++] = 0x1b;
			send_camera[j++] = send_tmp[i]^0x1b;
		}
		else
			send_camera[j++] = send_tmp[i];
  	}	

    //校验和
  	for(i=1;i<j;i++)
  	{
  		camera_ver += send_camera[i];
    }
    send_tmp[5] = (camera_ver&0xff00)>>8;
    send_tmp[6] = camera_ver&0x00ff;
	
	//转义
	for(i=5;i<7;i++)
    {          		
       if((send_tmp[i] == 0x7e)||(send_tmp[i]==0x1b))
       {
			send_camera[j++] = 0x1b;
			send_camera[j++] = send_tmp[i]^0x1b;
	   }
		else
			send_camera[j++] = send_tmp[i];
	}

	//帧尾
    send_camera[j] = 0x7e;
	
	SIO_WriteFrame(COM_DEBUG, send_camera, j+1);
    return 1;
	
}

/****************************************************************
  函数名：QQZM_Camera_Run
  功  能  ：启动青青子木拍摄图片的状态机
  输入参数：无
  输出参数：无
  编写者  ：林兆剑
  修改记录：创建 2009/08/6 
****************************************************************/
void QQZM_Camera_Run(void)
{

	int ImageQuality;
	
	if(CurrCamera.head == NULL)
	{
		SCI_TRACE_LOW("<<<<The List is Empty!!!!");
		return;
	}
	if(!(CurrCamera.currst == CAMERA_START || CurrCamera.currst == CAMERA_WAIT || CurrCamera.currst == CAMERA_WAITINIT))
	{
		SCI_TRACE_LOW("<<<<The State is busy!!! State = %d",CurrCamera.currst);

		return;
	}


	//图片分分辨率
	switch(CurrCamera.head->para)
	{

	    case 0:
 			//一般(160*120)
 			ImageQuality = 0x15;
  	 		break;
       	case 1:
       	 	//较好(160*120)
       	 	 ImageQuality = 0x05;
  			break;
    	case 2:
           	//一般(320*240)
           	ImageQuality = 0x11;
      		break;
       	case 3:
   			//好(320*240)
   			ImageQuality = 0x02;
      		break;
       	case 4: 
       		//较好(320*240)
       		ImageQuality = 0x01;
 			break;
 		case 5:
 			//很好(640*480)
 			ImageQuality = 0x13;
    		break;
		default:
   			//好(320*240)
   			ImageQuality = 0x02;
      		break;
			
	}
	if(CurrCamera.head->saveflag == PIC_SAVE)
	{
			ImageQuality = 0x02;
	}

	if(needTest == 1)
	{
		//发送测试命令
		SG_QQZM_Camera_UartSend(1,0,0);
		CurrCamera.currst = CAMERA_SYNC;
		t_camera = GetTimeOut(8);
		memset(Camera_Buf,0,sizeof(Camera_Buf)); // 清空存放图片的buffer
		Camera_BufCnt = 0;    
	}
	else
	{
		//发送拍照指令
		SG_QQZM_Camera_UartSend(0, 0, ImageQuality);
		CurrCamera.currst = CAMERA_INIT;
		t_camera = GetTimeOut(8);

	}


}

/****************************************************************
  函数名：QQZM_Camera_TimeOut_Handle
  功  能  ：青青子木拍摄图片超时处理
  输入参数：无
  输出参数：无
  编写者  ：林兆剑
  修改记录：创建 2009/08/6 
****************************************************************/
void QQZM_Camera_TimeOut_Handle(void)
{

	xSignalHeaderRec      *signal_ptr = NULL;


	switch(CurrCamera.currst)
	{
		//测试指令应答超时
		case CAMERA_SYNC:
			{
				needTest = 1;
				CurrCamera.currst = CAMERA_START;
				SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 

			}
			break;
		//拍照指令应答超时
		case CAMERA_INIT:
			{
				needTest = 1;
				CurrCamera.currst = CAMERA_START;
				SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 

			}
			break;
		//等待获取图象超时
		case CAMERA_WAIT_GET_PIC:
			{
				needTest = 1;
				CurrCamera.currst = CAMERA_START;
				SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 

			}
			break;
		//等待发送队列空闲
		case CAMERA_WAIT:
			{
				SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 

			}
			break;
		default:
			break;
			
	}
	

}


/****************************************************************
  函数名：QQZM_Camera_GetPicture_Send
  功  能  发送取图片指令
  输入参数：无
  输出参数：无
  编写者  ：林兆剑
  修改记录：创建 2009/08/6 
****************************************************************/
void QQZM_Camera_GetPicture_Send(void)
{
	
	//发送拍照指令
	SG_QQZM_Camera_UartSend(2,0,IndexCnt);
	t_camera = GetTimeOut(8);

	//SIO_WriteFrame(COM2, "444", 4);
}

/**********************************************************************
功能:  对于接收到的数据进行ESC检查
***********************************************************************/
int  ParseImage(unsigned char* RxBuf,int  len,unsigned char* imageBuf,int *imglen)
{
	int  i,j=0;
	
	for(i=0;i<len;i++)
    	{
       	if(*(RxBuf+i)==0x1b)
       	{
        		*( imageBuf+(j++))=(*(RxBuf+i+1))^0x1b;
			i++;
       	 }
        	else 
          		*( imageBuf+(j++))=*(RxBuf+i);
    	 }
	*imglen=j;
	return 1;
  }

/**************************************************************
查询双字节字符
****************************************************************/
  unsigned char * FindCharTwo(unsigned char *buf,int len,unsigned char c,unsigned char d)
  {
	  int i = 0;
	  while (i < len) {
		  if(*(buf+i) == c)
		  {
			  if(*(buf+i+1)==d)
			  {
				  return (unsigned char *)(buf+i);
			  }
		  }
		  i++;
	  }
	  return NULL;
  }


/****************************************************************
  函数名：QQZM_Camera_GetPicture_Handle
  功  能  处理读取的图片数据
  输入参数： 
  输出参数：无
  编写者  ：林兆剑
  修改记录：创建 2009/08/6 
****************************************************************/
void QQZM_Camera_GetPicture_Handle(unsigned char* buf, int len)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	unsigned char *posi=NULL;
	unsigned char check1=0;
	unsigned char check2=0;
	unsigned char check3=0;
	unsigned char endFlag = 0;
	
	unsigned char imageBuf[1024]={0};
	unsigned char RxBuf[1024]={0};
	int imageLen = 0;
	int RxLen = 0;
	int indexlen = 7;

	check1 = buf[len-3];
	check2 = buf[len-4];
	check3 = buf[len-5];

	if((check1==0x1b)&&(check3==0x1b))   //校验two 0x1b
	{
		indexlen = 9;
		if((buf[len-6]==0xff)&&(buf[len-7]==0xff))
		{
			int nret=0;
			unsigned char *posi = NULL;
      
			posi=FindCharTwo(buf, len, 0xff, 0xd9);
			if(posi)
			{
				endFlag = 1;
				nret = posi-buf-2;
				if(buf[3]==0x1b)
	  			{
	 				memcpy(&RxBuf[RxLen],&buf[5],nret-1);
	   				RxLen += nret-1;
	  			}
				else
				{
		   			memcpy(&RxBuf[RxLen],&buf[4],nret);
		   			RxLen += nret;
				}
			}

		}
		

	}
 	else if((check1==0x1b)||(check2==0x1b)) //校验one 0x1b
 	{
		indexlen = 8;

		if((buf[len-5]==0xff)&&(buf[len-6]==0xff))
		{
			int nret=0;
			unsigned char *posi=NULL;
      
			posi=FindCharTwo(buf, len, 0xff,0xd9);
			if(posi)
			{
				endFlag = 1;
				nret=posi-buf-2;
				if(RxBuf[3]==0x1b)
	  			{
	 				memcpy(&RxBuf[RxLen],&buf[5],nret-1);
	   				RxLen += nret-1;
	  			}
				else
				{
		   			memcpy(&RxBuf[RxLen],&RxBuf[4],nret);
		   			RxLen += nret;
				}
			}
		}

	}
	else  //校验无0x1b
	{
		indexlen = 7;
		if((buf[len-4]==0xff)&&(buf[len-5]==0xff))
		{
			int nret=0;
			unsigned char *posi=NULL;
			

			posi=FindCharTwo(buf,len,0xff,0xd9);
			if(posi)
			{
				endFlag = 1;
				nret=posi-buf-2;
				if(buf[3]==0x1b)
	  			{
	 				memcpy(&RxBuf[RxLen],&buf[5],nret-1);
	   				RxLen += nret-1;
	  			}
				else
				{
		   			memcpy(&RxBuf[RxLen],&buf[4],nret);
		   			RxLen += nret;
				}
			}

		}

	}

	if((len > 7) && (endFlag == 0))  //每包要大于7个字节
   	{
		int nret = 0;

	   	nret = len - indexlen;
		if(buf[3]==0x1b)
		{
			memcpy(&RxBuf[RxLen],&buf[5],nret-1);
			RxLen += nret-1;
		}
		else
		{
   			memcpy(&RxBuf[RxLen],&buf[4],nret);
   			RxLen += nret;
		}
   	}

	if(ParseImage(RxBuf, RxLen, imageBuf, &imageLen))
	{
		SCI_MEMCPY(Camera_Buf+Camera_BufCnt+39, imageBuf, imageLen);
		Camera_BufCnt += imageLen;
		
	}

	if(endFlag == 0)
	{
		IndexCnt++;
		if(IndexCnt > 68)
		{
			SCI_MEMSET(Uart_Read_Buf, 0, UartReadCnt);
			needTest = 1;
			CurrCamera.currst = CAMERA_START;
			SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 
			return ;
		}
		else
		{
			SG_CreateSignal(SG_QQZM_PICTURE,0, &signal_ptr); 	
		}
	
	}
	else
	{
		CurrCamera.currst = CAMERA_START;
		g_state_info.photo_catch_cnt ++;
		SG_CreateSignal(SG_CAMERA_MSGMAKE, 0, &signal_ptr); 	

	}
	






}




/****************************************************************
  函数名：QQZM_Camera_Uart_Read
  功  能  青青子木串口数据处理
  输入参数：无
  输出参数：无
  编写者  ：林兆剑
  修改记录：创建 2009/08/6 
****************************************************************/
void QQZM_Camera_Uart_Handle(void)
{
	xSignalHeaderRec      *signal_ptr = NULL;

	unsigned char *framePtr = NULL;
	unsigned char *frameEnd = NULL;
	unsigned char buf[1024]="";
	int len = 0;



	framePtr = FindChar(Uart_Read_Buf,UartReadCnt+1,0x7e);
	if(framePtr == NULL)
	{
		SCI_MEMSET(Uart_Read_Buf, 0, UartReadCnt);
		needTest = 1;
		CurrCamera.currst = CAMERA_START;
		SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 
		//SIO_WriteFrame(COM2, "006", 4);
		return ;
	}
	
	frameEnd = FindChar(framePtr+1,UartReadCnt-(framePtr-Uart_Read_Buf)+1,0x7e);
	if(frameEnd == NULL)
	{
		SCI_MEMSET(Uart_Read_Buf, 0, UartReadCnt);
		needTest = 1;
		CurrCamera.currst = CAMERA_START;
		SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 
		//SIO_WriteFrame(COM2, "005", 4);
		return ;
	}
	
	//帧长度
	len = frameEnd-framePtr+1;

	SCI_MEMSET(buf, 0, 1024);
	SCI_MEMCPY(buf, framePtr, len);
	SCI_MEMSET(Uart_Read_Buf, 0, 2048);
	
#if (0)
	SIO_WriteFrame(COM2, buf, 7);
#endif /* (0) */

	switch(CurrCamera.currst)
	{
		//测试指令应答
		case CAMERA_SYNC:
			{
				if((buf[4] == 1)&&(len == 6))
				{
					needTest = 0;
					CurrCamera.currst = CAMERA_WAITINIT;
					SG_CreateSignal(SG_CAMERA_WAKEUP,0, &signal_ptr); 	
					t_camera = 0xffffffff;
				}

			}
			break;
		//拍照指令应答
		case CAMERA_INIT:
			{
				if((buf[len-1] == 0x7e)&&(len == 6))
				{
					IndexCnt = 0;
					CurrCamera.currst = CAMERA_WAIT_GET_PIC;
					SG_CreateSignal(SG_QQZM_PICTURE,0, &signal_ptr); 	
					//QQZM_Camera_GetPicture_Send();
				}

			}
			break;
		//获取图象
		case CAMERA_WAIT_GET_PIC:
			{
				QQZM_Camera_GetPicture_Handle(buf, len);
				//QQZM_Camera_GetPicture_Send();
			}
			break;
		default:
			break;
	}





}

/****************************************************************
  函数名：QQZM_Camera_Uart_Read
  功  能  ：青青子木串口数据读取
  输入参数：无
  输出参数：无
  编写者  ：林兆剑
  修改记录：创建 2009/08/6 
****************************************************************/
void QQZM_Camera_Uart_Read(void)
{

#if (1)
	int count = 0;
	while(SIO_ReadFrame(COM_DEBUG,Uart_Read_Buf+UartReadCnt,UART_READ_BUF,(uint32*)&count) > 0)
	{
	
		UartReadCnt += count;

		if(((CurrCamera.currst != CAMERA_WAIT_GET_PIC)&&(UartReadCnt>= 6)) || ((CurrCamera.currst == CAMERA_WAIT_GET_PIC)&&(UartReadCnt >= 512)))
		{
			QQZM_Camera_Uart_Handle();
			UartReadCnt = 0;
		}
	}	
#endif /* (0) */

}


