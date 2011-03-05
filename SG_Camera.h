/****************************************************************
  版权   ：福建星网锐捷通讯有限公司 2002-2005 版权所有
  文件名 ：SG_Camera.h
  版本   ：1.00
  创建者 ：陈海华
  创建时间：2006-3-17
  内容描述：SG2000 Camera信息接收分析     
  修改记录：
***************************************************************/
#ifndef _SG_Camera_H_
#define _SG_Camera_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CAMERA_OFFSET 37

#define SG_PICTURE_SAVE_MAX_FILE 5		//图片存储文件

#define PIC_NOT_SAVE 0 // 图片不进行存储
#define PIC_SAVE 1 // 图片进行存储

#define CAMERA_TIME_WATCH_REQ 0
#define CAMERA_CONDITION_WATCH_REQ 1
#define CAMERA_ONE_REQ 2

#define CAMERA_ONE_PIC_LEN 10000
#define CAMERA_ONE_PACK_LEN 5120



typedef struct _CAMERA_CMD_
{
	unsigned long cmd;//当前调用者 
	unsigned long para;//
	unsigned char saveflag;
	struct _CAMERA_CMD_ *next;
}CAMERA_CMD;



typedef  enum  SG_CAMERA_STATE
{
	CAMERA_START = 0, // 开始状态
	CAMERA_SYNC =1,//发送同步等待应答   发送测试命令等待应答(青青子木)
	CAMERA_WAITSYNC=2,//等待对方发送同步
	CAMERA_WAITINIT=3,//等待初始化状态
	CAMERA_INIT=4,//初始化   发送拍照命令等待应答(青青子木)
	CAMERA_SET_MSG_LEN=5,//设置数据包大小
	CAMERA_WAIT_GET_PIC=6,//等待获取图像
	CAMERA_GET_PIC=7,//获取图像
	CAMERA_GET_XINDEX_PIC=8,//拍摄完毕，获取第n包数据
	CAMERA_WAIT=9, // 等待状态
	CAMERA_IDLE=10, // 空闲状态
	CAMERA_IDLE_SYNC=11,//idle状态下发送同步等待应答
	CAMERA_IDLE_WAITSYNC=12, // idle状态下等待同步状态
	CAMERA_RESET=13, // 复位状态
	CAMERA_WAIT_START,
	CAMERA_WAIT_SET
	
}sg_camera_state;

typedef struct SG_CAMERA_
{
	sg_camera_state	currst;//
	CAMERA_CMD	*head;
}SG_CAMERA;



void camera_construct_cmd(uint8 id,uint8 p1,uint8 p2,uint8 p3,uint8 p4);

void sg_camera_request(int quality,int cmd,int save_flag);
void SG_Camera_Task_Delete(int task);
void camera_reset(void);
BOOLEAN SG_Camera_Task_Check(int task);
int SG_Camera_Interval_Check(void);
void Camera_Condition_Watch(void);
int Camera_Get_Pic_IamgeQuality(void);
void camera_run(void);
void camera_cmd_delete(void);
int camera_task_count(void);
void camera_timout_check(void);
void Camera_Wakeup_Handle(void);
void QQZM_Camera_Run(void);


#ifdef __cplusplus
}
#endif

#endif //_SG_Camera_H_
