/******************************************************************************
 ** File Name:      sio.c                                                     *
 ** Author:         Richard Yang                                              *
 ** DATE:           04/02/2002                                                *
 ** Copyright:      2002 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the basic operation interfaces of       *
 **                 serrial device. It manages create, send, receive          *
 **                 interface of serrial port.                                *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 04/02/2002     Richard.Yang     Create.                                   *
 ** 05/13/2002     Richard.Yang     Add sio_send_packet, sio_send_log_packet  *
 ** 05/30/2003     Eric.zhou        Add AT through channel server and DEBUG   *
 **                                 and USER mode change functions            *
 ** 08/14/2003     Zhemin.Lin       add GetClkDividerValue,                   *
 **                                 SIO_UpdateClkDividerValue                 *
 **08/25/2003     Zhemin.Lin        add MIX_MODE for COM_DATA port  according to CR:MS00004213  *
 ** 08/28/2003    Zhemin.Lin        Modify according CR:MS00004031            *
 ** 09/12/2003    Zhemin.Lin        Modify according CR:MS00004678            *
 ** 09/23/2003    Xueliang.wang     Modify according CR:MS00004877            *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "tb_drv.h"
#include "tb_comm.h"
#include "sio.h"
#include "tasks_id.h"
#include "cmddef.h"
#include "cm.h"

#include "sc6600d_reg.h"

#include "nvitem.h"
#include "nv_type.h"
#include "SG_Camera.h"

//@Zhemin.Lin
#include "chip.h"

//chenhh
#include "SG_Set.h"

//@Zhemin.Lin, 09/12/2003, CR:MS00004678, must use os_api.h
#include "os_api.h"
//@Daniel.Ding,CR:MS00006039,12/4/2003,begin
#ifdef USE_NUCLEUS
    #include "nucleus.h"
    void Handle_RX_FULL_DEBUG (void);
    void Handle_RX_FULL_USER (void);
#else
    //NULL
#endif
//Daniel.end



/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif

//@Zhemin.Lin, 08/15/2003
#ifdef _SIO_DEBUG_
	#define SIO_PRINT SCI_TRACE_LOW
#else
	#define SIO_PRINT
#endif
#ifdef _5100BD_MODULE_
#define LOG_BUF_SIZE               8000 
#else
#define LOG_BUF_SIZE               22000
#endif

#ifdef BUS_MONITOR
	// @annie.chen add(2002-09-12)
	/**---------------------------------------------------------------------------*
	 **  Parallel port Register               
	 **---------------------------------------------------------------------------*/
	#define PARALLEL_PORT_REG 0x2400000

	PUBLIC void Write_Para_Port_Char( char ch );
#endif // End of BUS_MONITOR


PUBLIC BOOLEAN g_sio_enable_deep_sleep = SCI_TRUE;	//Must be inited as SCI_TRUE;


/**---------------------------------------------------------------------------*
 **  Static Variables & functions                      
 **---------------------------------------------------------------------------*/
LOCAL uint32    log_frm_flag;    // Log frame type. Indicate current sended frame
LOCAL uint32    log2_frm_flag;    // Log frame type. Indicate current sended frame 
                                 // status LOG_FRM_MED, LOG_FRM_NONE, LOG_FRM_END
LOCAL int       cur_log_frm_size;  // Current log frame size
LOCAL int       cur_log2_frm_size;  // Current log frame size
LOCAL SIO_BUF_S log_buf_info;             // Log buffer control information
LOCAL SIO_BUF_S log2_buf_info;             // Log buffer control information
LOCAL uint8     log_buf[LOG_BUF_SIZE];    // Log buffer
LOCAL uint8     log2_buf[LOG_BUF_SIZE];    // Log buffer

// Because uartutil.c will use this array, we should not define it as static array
PUBLIC SIO_INFO_S      sio_port[MAX_SIO_PORT_NUM];   // SIO information

LOCAL uint8		sio_atc_rec_buf[ATC_REC_BUF_SIZE];
#ifdef _U0_SOFTWARE_FLOW_CONTROL
LOCAL int 			gXon_on = 1;
LOCAL int  			gRemoteXon_on = 1;
#endif
#define		XOFF	0x13	// Xoff of Software flow control.
#define		XON		0x11	// Xon of Software flow control.
LOCAL uint8     sio_debug_send_buf[DEBUG_PORT_SEND_BUF_SIZE];// Debug port tx buffer
LOCAL uint8     sio_debug_rec_buf[DEBUG_PORT_REC_BUF_SIZE];  // Debug port rx buffer
LOCAL uint8     sio_data_send_buf[DATA_PORT_SEND_BUF_SIZE];  // User port tx buffer
LOCAL uint8     sio_data_rec_buf[DATA_PORT_REC_BUF_SIZE];    // User port rx buffer
#ifdef _DUART
LOCAL uint8     sio_com2_send_buf[DEBUG_PORT_SEND_BUF_SIZE];
LOCAL uint8     sio_com2_rec_buf[DEBUG_PORT_REC_BUF_SIZE];  
#endif


//@Zhemin.Lin, 08/25/2003
LOCAL uint8     sio_mix_data_rec_buf[DATA_PORT_REC_BUF_SIZE]; //User port for data rx buffer
extern SG_CAMERA CurrCamera;

/**---------------------------------------------------------------------------*
 **  Hardware functions
 **---------------------------------------------------------------------------*/
#define RECEIVE_TIMEOUT    0x3e00
#define FLOW_CONTROL_SHIFT 0x07

LOCAL SCI_EVENT_GROUP_PTR  comm_event = NULL;    //eric.zhou  Uart communication event

LOCAL void PurgeSio(uint32 port,       // Port number 'COM_DATA', 'COM_DEBUG'
                    uint32 clear_flag  // Clear method  'PURGE_TX_CLEAR', 
                                       // 'PURGE_RX_CLEAR'
                   );

/* @jim.zhang CR:MS00006386 2003/12/29 */
void uart0_Rx_INT_Enable(int enable_flag);
static int ring_space( SIO_BUF_S *pRing );
/* end CR:MS00006386 */

LOCAL int 	g_end_point = 0;				// end_point of tx buffer used by COM_DATA.
void uart_mix_set_end_point( uint32 end_point );

#ifdef _U0_FLOW_CONTROL
LOCAL SCI_TIMER_PTR   Space_timer =NULL;    // a timer to check the space of Rx buffer for COM_DATA 
LOCAL void Space_Check(void);
#define		SPACE_CHECK_DELAY		300		// time delay of space check of Rx buffer for COM_DATA
#define		EMPTY_SPACE_WATER_MARK	258		// empty sapce water mark of rx buffer 
#endif

void uart_ppp_putc( int port ,int ch );
void uart_normal_putc( int port ,int ch ); //added by steven.shao 20070611
void uart_mix_sem_give( void );
int uart_mix_sem_take(	void );
/**---------------------------------------------------------------------------*
 **  Protocol functions
 **---------------------------------------------------------------------------*/
#define  SIO_FRAME_OK     0
#define  SIO_FRAME_ESCAPE 1
#define  SIO_FRAME_NORMAL 2

typedef enum
{
	SIO_FRAME_NONE = 0,
    SIO_FRAME_HEAD,
	SIO_FRAME_ERROR
} SIO_FRAME_TYPE_E;

typedef enum
{
	SIO_CHAR_ESCAPE,
	SIO_CHAR_NORMAL
} SIO_CHAR_TYPE_E;

LOCAL SIO_FRAME_TYPE_E PPP_frame_type; // PPP frame type
LOCAL SIO_CHAR_TYPE_E  char_mask_mode; // Character mode, check if there is a 
                                       // masked character here
LOCAL int  CheckInput(int * nchar      // Character recevied from serial port
                     );

LOCAL int  EncodeMsg(SIO_BUF_S *buf_ptr, // Buffer pointer
                     uint8     *src,     // Source data
                     int       size      // Size of the data in bytes 
                    );
LOCAL void EncodeFrmHead(const unsigned char * data, // Head, no masked char
                        int size                     // Size of head
                        );
LOCAL int WriteTraceMsgToLogBuf(SIO_BUF_S *buf_ptr, uint8 *src, int size);
LOCAL void SetLen (int len);
LOCAL int GetLen (void);
/**---------------------------------------------------------------------------*
 **  Ring buffer management functions
 **---------------------------------------------------------------------------*/
LOCAL int ring_add( SIO_BUF_S *pRing, int ch );
LOCAL int ring_remove( SIO_BUF_S *pRing );

/**---------------------------------------------------------------------------*
 **  User port variables & functions
 **---------------------------------------------------------------------------*/
#define            BACKSPACE_CHAR    0x08
#define            END_CHAR          0x0D
#define            NL_CHAR          0x0A //new line
#define            N_CHAR          '\0' //new line
#define            PLUS_CHAR         '+'

#define            MAX_PLUS_NUMBER   3
#define            MODE_CHANGE_DELAY 1000 // 1000 milliseconds

LOCAL uint32       user_port_mode;  // ATC_MODE, DATA_MODE;
LOCAL char         atc_end_ch1;      // End char of ATC;
LOCAL char         atc_end_ch2;      // End char of ATC;

LOCAL char         atc_bs_ch;       // Backspace char of ATC
LOCAL BOOLEAN      atc_is_echo;
LOCAL uint32       cur_atc_frame_size; // Current Enqueued atc frame size
LOCAL ATC_CALLBACK AT_callback_ind = NULL;      // Pointer to the AT callback
LOCAL SCI_TIMER_PTR   ATC_timer =NULL;      // ATC timer
LOCAL uint32       plus_num;       // Number of '+'
LOCAL uint32       last_rx_time;   // Last receive time in user port!
LOCAL uint32       IsChangeBaudrate = 0;

LOCAL void ATC_CheckPPP(uint32 wParam); // Timer callback function to check '+++'

void uart_putc( int port, int ch );
// The MACRO below is used to reduce time cost in SIO interrupt routine
#define DECLARE_TEMP_VARS \
int          i;\
int          free_count = (FIFO_SIZE - (sio_ptr->uart->fifo_cnt >> 8) & 0x0FF);\
int          data_size; \
register int temp_end_point;\
uint8        *tx_buf_ptr;\
uint32       *tx_buf_reg;

typedef struct FRM_HEADtag
{
	uint16 reserved1;
	uint8  end_flag;
	uint8  start_flag;
	uint32 seq_num;
	uint16 len;
	uint8  type;
	uint8  subtype;
	uint16 lost_frm_num;
} FRM_HEAD_T;

LOCAL FRM_HEAD_T data = 
{
	0x0,   // Reserved
    0x7E,  // End flag
    0x7E,  // Begin Flag
	0x0,   // sequnce number 
	0x0,   // Warning:
           // Len  Because we can not set this value when we add 
           //      information here! We have to set it to ZERO, channel
           //      server and logel will care it more!
    DIAG_LOG_A, // Type
    0x0,   // Subtype 
    0x0,   // Lost frame number
};

//@Daniel.Ding,CR:MS00004577,10/8/2003,begin
#ifdef USE_NUCLEUS //Hero main branch .
    LOCAL void SIO_DebugPortIRQ(void);
    LOCAL void SIO_UserPortIRQ(void);
    #ifdef _DUART
	LOCAL void SIO_Com2PortIRQ(void);
	#endif
#else              //Bassline branch.
    LOCAL void SIO_DebugPortIRQ(uint32 int_num);
    LOCAL void SIO_UserPortIRQ(uint32 int_num);
    #ifdef _DUART
	LOCAL void SIO_Com2PortIRQ(uint32 int_num);
	#endif
#endif
//@Daniel,end

//@Zhemin.Lin,2003/08/14
LOCAL uint32 GetClkDividerValue(uint32 boudrate );

//@Zhemin.Lin, CR:MS00004031, 08/28/2003,  begin
LOCAL void DefaultAtCallback(uint32 len);
//@Shijun.Cui 2005-02-02
LOCAL void DefaultAtHandle(uint32 len);

#define DO_ATC_CALLBACK(len) \
	if ( NULL == AT_callback_ind) \
	{\
            DefaultAtHandle(len);\
	}\
	else \
	{\
	    (*AT_callback_ind)(len);\
	}
//@Zhemin.Lin, CR:MS00004031, 08/28/2003,  end

//@Zhemin.Lin, CR:MS00004213, 2003/08/22, begin

#define SIO_DATA_PACKET 0x0DD
#define SIO_ATC_PACKET 0x0AA
#define SIO_ATC_PATTERN (0x1D<<3)
#define SIO_DATA_PATTERN (0<<3)
#define SIO_STATUS_PATTERN (1<<3)
#define SIO_READY_PATTERN (2<<3)
#define SIO_BUSY_PATTERN (3<<3)

//the packet head struct of AT mix mode
#define MIX_PACKET_HEAD_LENGHT 3

#define	MIX_PROTO_FIX_HEAD_LEN	4

typedef struct
{
    uint8 type;
    uint8 length_lsb;
    uint8 subtype;
}MIX_PACKET_HEAD_T;

typedef struct
{  
    //@Zhemin.Lin, 09/12/2003, CR:MS00004678, must use os_api interface
    SCI_MUTEX_PTR mutexforwrite;        //mutex for write
    SIO_BUF_S data_rx_buf;
    uint32 recvlen;                //length has been received
    uint32 datalen;
    uint8 checksum;
    MIX_PACKET_HEAD_T packet;    //record the current receive packet head
    uint32 buf_used;
    uint8   sio_temp_buf[DATA_PORT_REC_BUF_SIZE];
}MIX_MODE_INFO;

LOCAL MIX_MODE_INFO mix_mode_info;

LOCAL SIO_NOTIFY_HANDLE notify_handle = NULL;  //notify handle
LOCAL ATC_CALLBACK data_recv_handle = NULL;  //data receive handle
LOCAL SIO_ERROR_HANDLE link_error_handle = NULL; // link error handle

//@Zhemin.Lin, 09/12/2003, CR:MS00004678, must use os_api interface, begin
//get mutex
#define SIO_SEMTAKE (SCI_GetMutex(mix_mode_info.mutexforwrite, SCI_WAIT_FOREVER))
//put mutex
#define SIO_SEMGIVE (SCI_PutMutex(mix_mode_info.mutexforwrite))
//@Zhemin.Lin, 09/12/2003, CR:MS00004678, must use os_api interface, end

#define DO_NOTIFY_HANDLE(notify_id, param)  \
	if(NULL != notify_handle) \
		(*notify_handle)(notify_id, param)

#define DO_ERROR_HANDLE(port_id, error_type) \
	if (NULL != link_error_handle) \
		(link_error_handle)(port_id, error_type)

#define RESET_MIX_MODE_INFO \
	 mix_mode_info.checksum = 0;\
        mix_mode_info.datalen = 0;\
        mix_mode_info.recvlen = 0;\
        mix_mode_info.buf_used = 0;

LOCAL void DefaultDataCallback(uint32 len);
#define DO_DATA_CALLBACK(len) \
	if (NULL == data_recv_handle) \
	{\
	    DefaultDataCallback(len);\
	}\
	else\
	{\
	    (*data_recv_handle)(len);\
	}
	

/*****************************************************************************/
// Description :    this functuion is used to deal with the SIO irq, when current mode is MIX_MODE
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
LOCAL void SIO_HandleMixIrq(uint32 status);

/*****************************************************************************/
// Description :    this function is used to handle recv data from SIO, when current mode is MIX_MODE
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
LOCAL void SIO_MixRecv(SIO_INFO_S *sio_ptr);

//@Zhemin.Lin, CR:MS00004213, 2003/08/22, end


/*@jim.zhang CR:MS8384 2004-04-02 */
extern uint8   gprs_debug;	// 1: GPRS_DEBUG  0: not defined GPRS_DEBUG.
/* end CR:MS8384 */

typedef struct
{
	MSG_HEAD_T head;
	uint8     buf[2048];
	uint32    cur_offset;
} PPP_BUF_T;

LOCAL PPP_BUF_T ppp_buf;
uint32 SIO_UpdatePPPBufSize(void);
void SIO_ClearPPPBuf(void);
uint8 * SIO_GetPPPBuf(void);

uint32 SIO_UpdatePPPBufSize()
{
	ppp_buf.head.len = sizeof(MSG_HEAD_T) + ppp_buf.cur_offset;
	
	return ppp_buf.head.len;
}

void SIO_ClearPPPBuf()
{
	ppp_buf.cur_offset = 0;
}

uint8 * SIO_GetPPPBuf()
{
	return ((uint8 *)&ppp_buf);
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 ** PUBLIC uint32 SIO_SetBaudRate(uint32 port,uint32  baud_rate);             *
 **                                                                           *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Set serial port's baud rate.                                          *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     NONE                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 ** Author   Eric.zhou           2003/5/20.                                   *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC uint32 SIO_SetBaudRate(uint32 port, uint32 baud_rate)
{
    //NVITEM_ERROR_E return_val;
    SIO_CONTROL_S   m_dcb;
	
    SCI_ASSERT(port < MAX_SIO_PORT_NUM);
	
    if ((baud_rate > BAUD_1200) || (baud_rate < BAUD_921600))
		return SCI_ERROR;
	
    SIO_Close(port);
	
    // Set baud rate
    m_dcb.baud_rate     = baud_rate;
    m_dcb.flow_control  = 0;

	IsChangeBaudrate    = 1;
	SIO_Create(port, sio_port[port].phy_port, &m_dcb);
	IsChangeBaudrate    = 0;

    if (COM_DEBUG == port) 
		//return_val = EFS_NvitemWrite(NV_SERIAL_0_BAUD_RATE, sizeof(uint32), (uint8 *)&baud_rate, 1);
		REFPARAM_SetUart0BaudRate(baud_rate);
	#ifdef _DUART
    else if(COM_DATA == port)
		//return_val = EFS_NvitemWrite(NV_SERIAL_1_BAUD_RATE, sizeof(uint32), (uint8 *)&baud_rate, 1);
		REFPARAM_SetUart1BaudRate(baud_rate);	
	#else
    else    //if(COM_DATA == port)
		//return_val = EFS_NvitemWrite(NV_SERIAL_1_BAUD_RATE, sizeof(uint32), (uint8 *)&baud_rate, 1);
		REFPARAM_SetUart1BaudRate(baud_rate);
	
	#endif	

	
    return SCI_SUCCESS;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     uint32 SIO_GetBaudRate(uint32 port)                                   *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **      Get Serial Port's Baud Rate.                                         *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **      Baud Rate                                                            *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 ** Author   Eric.zhou           2003/5/20.                                   *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC uint32 SIO_GetBaudRate(uint32 port)
{
	uint32         baud_rate;
	//NVITEM_ERROR_E return_val;
	
	SCI_ASSERT(port < MAX_SIO_PORT_NUM);
	
	if (COM_DEBUG == port) 
		//return_val= EFS_NvitemRead(NV_SERIAL_0_BAUD_RATE, sizeof(uint32),(uint8 *)&baud_rate);
		baud_rate = REFPARAM_GetUart0BaudRate();
	else if(COM_DATA == port)
		//return_val= EFS_NvitemRead(NV_SERIAL_1_BAUD_RATE, sizeof(uint32),(uint8 *)&baud_rate);
		baud_rate = REFPARAM_GetUart1BaudRate();
	else
		return 0;
	
	//if ((return_val != NVERR_NONE) || (baud_rate > BAUD_1200)||(baud_rate < BAUD_921600) )
	if ((baud_rate > BAUD_1200)||(baud_rate < BAUD_921600))
	{
		//restore orignal baud_rate
		if (port == COM_DEBUG)
		{
			baud_rate = BAUD_115200;
			//EFS_NvitemWrite(NV_SERIAL_0_BAUD_RATE,sizeof(uint32),(uint8 *)&baud_rate,1);
			REFPARAM_SetUart0BaudRate(baud_rate);
		}
		else
		{
			baud_rate = BAUD_115200;
			//EFS_NvitemWrite(NV_SERIAL_1_BAUD_RATE,sizeof(uint32),(uint8 *)&baud_rate,1);
			REFPARAM_SetUart1BaudRate(baud_rate);
		}
	}
	
	return baud_rate;
}

/*@jim.zhang CR:MS10775 2004-07-14 */


/*****************************************************************************/
// Description :    Change serial port's mode.
// Global resource dependence :
// Author :         Jim.zhang
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_ChangeMode(	//return the operate result:success or fail
	uint32 mode					//uart mode command.
	)
{
	uint32 curr_mode = 0xFF; 
    SIO_CONTROL_S  m_dcb;

    SCI_ASSERT(mode < MAX_SIO_PORT_NUM);
    
    SCI_TRACE_LOW("SIO:change uart0 and uart1 mode");

	curr_mode = REFPARAM_GetUartMode();

	if (mode == curr_mode)
	{
		return SCI_SUCCESS;
	}

	SCI_DisableIRQ();
	
    SIO_Close(COM_DEBUG);
	SIO_Close(COM_USER);

    m_dcb.flow_control  = 0;

	if( COM_DATA == mode )	//UART0:data  UART1:debug
    {
	    m_dcb.baud_rate      = SIO_GetBaudRate(COM_DATA);
        SIO_Create(COM_DATA, COM0, &m_dcb);
        
        m_dcb.baud_rate      = SIO_GetBaudRate(COM_DEBUG);
        SIO_Create(COM_DEBUG, COM1, &m_dcb);
	}
	else 					//UART1:data  UART0:debug
    {
	    m_dcb.baud_rate      = SIO_GetBaudRate(COM_DATA);
        SIO_Create(COM_DATA, COM1, &m_dcb);
        
        m_dcb.baud_rate      = SIO_GetBaudRate(COM_DEBUG);
        SIO_Create(COM_DEBUG, COM0, &m_dcb);
    
    }

	SCI_RestoreIRQ();

    REFPARAM_SetUartMode(mode);

    return SCI_SUCCESS;
}


/*****************************************************************************/
// Description :    Get serial port's mode.
// Global resource dependence :
// Author :         Jim.zhang
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_GetMode()
{
    uint32 sio_mode = 0xFF;

    sio_mode = REFPARAM_GetUartMode();

    return sio_mode;
}

/* end CR:MS10775 */


//dummy function

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     PUBLIC void SIO_SetMode(uint32 mode);                                 *
 **                                                                           *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Set serial port's mode.                                               *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Port mode                                                             *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 ** Author   Eric.zhou           2003/5/29                                    *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC uint32 SIO_SetMode(uint32 mode)
{
	return 0;
}

//dummy function end.


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_Create()                                                     *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Create a device control block.                                        *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     The handle of SIO device.                                             *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
//@Daniel.Ding,CR:MS00006039,12/4/2003,begin
#ifdef USE_NUCLEUS
    NU_HISR RX_FULL_ISR_USER;
    uint32  RX_FULL_HISR_stack_USER[1024 * 2];

    NU_HISR RX_FULL_ISR_DEBUG;
    uint32  RX_FULL_HISR_stack_DEBUG[1024 * 2];
#else

#endif

PUBLIC HSIO SIO_Create(uint32        port, 
                       uint32        phy_port, 
                       SIO_CONTROL_S *dcb
                       )
{
    uint32 divider;

    SCI_ASSERT(port < MAX_SIO_PORT_NUM);

	/* Check if the port is opened before. */
	if (SIO_ALREADY_OPEN == sio_port[port].open_flag)
		return NULL;
	
	sio_port[port].open_flag = SIO_ALREADY_OPEN;
	sio_port[port].phy_port  = phy_port;
	
    /* 
	We may not use it now, 
	If we used it later, we should modify it!
    */
	sio_port[port].dcb.baud_rate    = dcb->baud_rate;
	sio_port[port].dcb.parity       = 0;//ULCRNoParity;
	sio_port[port].dcb.stop_bits    = 0;//ULCRS1StopBit;
    sio_port[port].dcb.byte_size    = 0;//ULCR8bits;
	sio_port[port].dcb.flow_control = dcb->flow_control;

	ppp_buf.head.seq_num = 0;
	ppp_buf.head.type    = PPP_PACKET_A;
	ppp_buf.head.subtype = 0;
	ppp_buf.cur_offset   = 0;

	/* Get base register of uart. */
	switch (port)
	{
	case COM_DEBUG:
		if (NULL == comm_event)
			comm_event = SCI_CreateEvent("NULL"); // Create a comm event group
		
        PPP_frame_type   = SIO_FRAME_NONE;    // No PPP frame 
        log_frm_flag     = LOG_FRM_NONE;      // No log frame
        char_mask_mode   = SIO_CHAR_NORMAL;   // No masked char
		cur_log_frm_size = 0;                 // No log frame

		/* Set receive and transmit buffer. */
        sio_port[port].tx_buf.sio_buf_ptr    = sio_debug_send_buf;
        sio_port[port].tx_buf.size           = DEBUG_PORT_SEND_BUF_SIZE;
        sio_port[port].tx_buf.start_point    = 0;
        sio_port[port].tx_buf.end_point      = 0;
        sio_port[port].tx_buf.status         = 0;
        sio_port[port].tx_buf.lost_num       = 0;
		memset(sio_port[port].tx_buf.sio_buf_ptr, 0, DEBUG_PORT_SEND_BUF_SIZE);
		
        sio_port[port].rx_buf.sio_buf_ptr    = sio_debug_rec_buf;
        sio_port[port].rx_buf.size           = DEBUG_PORT_REC_BUF_SIZE;
        sio_port[port].rx_buf.start_point    = 0;
        sio_port[port].rx_buf.end_point      = 0;
        sio_port[port].rx_buf.status         = 0;
        sio_port[port].rx_buf.lost_num       = 0;
		memset(sio_port[port].rx_buf.sio_buf_ptr, 0, DEBUG_PORT_REC_BUF_SIZE);
		
        /* Set log buffer. */
        log_buf_info.sio_buf_ptr    = log_buf;
        log_buf_info.size           = LOG_BUF_SIZE;
        log_buf_info.start_point    = 0;
        log_buf_info.end_point      = 0;
        log_buf_info.status         = 0;
        log_buf_info.lost_num       = 0;
		memset(log_buf_info.sio_buf_ptr, 0, LOG_BUF_SIZE);
		if (phy_port == COM0)
		{
		    DRV_RegHandler(TB_UART0_INT, SIO_DebugPortIRQ);
		}
		else
		{
		    DRV_RegHandler(TB_UART1_INT, SIO_DebugPortIRQ);
		}

        //@Daniel.Ding,CR:MS00006039,12/4/2003,begin		
		#ifdef USE_NUCLEUS
            NU_Create_HISR(&RX_FULL_ISR_DEBUG, 
                            "kpd_ISR", 
                            Handle_RX_FULL_DEBUG, 
                            2, 
                            RX_FULL_HISR_stack_DEBUG, 
                            2048);
		#else
            //NULL
		#endif
		
		
		break;
    case COM_DATA:
		SCI_ASSERT( ATC_timer == NULL);
        ATC_timer = SCI_CreateTimer("ATC TIMER", (TIMER_FUN) ATC_CheckPPP, 
			0, MODE_CHANGE_DELAY, SCI_NO_ACTIVATE);
		
		#ifdef 	_U0_FLOW_CONTROL
		SCI_ASSERT( Space_timer == NULL );
		Space_timer =  SCI_CreateTimer("Space TIMER", (TIMER_FUN) Space_Check, 
			0, SPACE_CHECK_DELAY, SCI_AUTO_ACTIVATE);
		#endif

		if (!IsChangeBaudrate)
		{
			user_port_mode      = ATC_MODE;
			atc_end_ch1         = END_CHAR;
			atc_end_ch2         = NL_CHAR;
			atc_bs_ch           = BACKSPACE_CHAR;
			atc_is_echo         = FALSE;
		}
		cur_atc_frame_size  = 0;
		plus_num            = 0;
		last_rx_time        = 0; 
		
		/* Set receive and transmit buffer. */
        sio_port[port].tx_buf.sio_buf_ptr    = sio_data_send_buf;
        sio_port[port].tx_buf.size           = DATA_PORT_SEND_BUF_SIZE;
        sio_port[port].tx_buf.start_point    = 0;
        sio_port[port].tx_buf.end_point      = 0;
        sio_port[port].tx_buf.status         = 0;
        sio_port[port].tx_buf.lost_num       = 0;
		memset(sio_port[port].tx_buf.sio_buf_ptr, 0, DATA_PORT_SEND_BUF_SIZE);
				
        sio_port[port].rx_buf.sio_buf_ptr    = sio_data_rec_buf;
        sio_port[port].rx_buf.size           = DATA_PORT_REC_BUF_SIZE;
        sio_port[port].rx_buf.start_point    = 0;
        sio_port[port].rx_buf.end_point      = 0;
        sio_port[port].rx_buf.status         = 0;
        sio_port[port].rx_buf.lost_num       = 0;
		memset(sio_port[port].rx_buf.sio_buf_ptr, 0, DATA_PORT_REC_BUF_SIZE);
        sio_port[port].atc_buf.sio_buf_ptr    = sio_atc_rec_buf;
        sio_port[port].atc_buf.size           = ATC_REC_BUF_SIZE;
        sio_port[port].atc_buf.start_point    = 0;
        sio_port[port].atc_buf.end_point      = 0;
        sio_port[port].atc_buf.status         = 0;
        sio_port[port].atc_buf.lost_num       = 0;
		memset(sio_port[port].atc_buf.sio_buf_ptr, 0, ATC_REC_BUF_SIZE);

	//@Zhemin.Lin, CR:MS00004213, 2003/08/22, begin
       RESET_MIX_MODE_INFO;
       mix_mode_info.data_rx_buf.sio_buf_ptr    = sio_mix_data_rec_buf;
       mix_mode_info.data_rx_buf.size           = DATA_PORT_REC_BUF_SIZE;
       mix_mode_info.data_rx_buf.start_point    = 0;
       mix_mode_info.data_rx_buf.end_point      = 0;
       mix_mode_info.data_rx_buf.status         = 0;
       mix_mode_info.data_rx_buf.lost_num       = 0;
		memset(mix_mode_info.data_rx_buf.sio_buf_ptr, 0, DATA_PORT_REC_BUF_SIZE);
	//create mutex
	//@Zhemin.Lin, 09/12/2003, CR:MS00004678, must use os_api interface
	mix_mode_info.mutexforwrite = SCI_CreateMutex("mutex_sio_write",SCI_INHERIT);
	SCI_ASSERT(SCI_NULL != mix_mode_info.mutexforwrite);
	//@Zhemin.Lin, CR:MS00004213, 2003/08/22, end
   		if (phy_port == COM0)
   		{
            DRV_RegHandler(TB_UART0_INT, SIO_UserPortIRQ);
        }
        else
        {
            DRV_RegHandler(TB_UART1_INT, SIO_UserPortIRQ);
        }
		#ifdef USE_NUCLEUS
            NU_Create_HISR(&RX_FULL_ISR_USER, 
                            "kpd_ISR", 
                            Handle_RX_FULL_USER, 
                            2, 
                            RX_FULL_HISR_stack_USER, 
                            2048);
		#else
            //NULL
		#endif
		break;
	#ifdef _DUART	
	case COM2:
		if (NULL == comm_event)
			comm_event = SCI_CreateEvent("NULL"); // Create a comm event group
		
        PPP_frame_type   = SIO_FRAME_NONE;    // No PPP frame 
        log2_frm_flag     = LOG_FRM_NONE;      // No log frame
        char_mask_mode   = SIO_CHAR_NORMAL;   // No masked char
		cur_log2_frm_size = 0;                 // No log frame

		/* Set receive and transmit buffer. */
        sio_port[port].tx_buf.sio_buf_ptr    = sio_com2_send_buf;
        sio_port[port].tx_buf.size           = DEBUG_PORT_SEND_BUF_SIZE;
        sio_port[port].tx_buf.start_point    = 0;
        sio_port[port].tx_buf.end_point      = 0;
        sio_port[port].tx_buf.status         = 0;
        sio_port[port].tx_buf.lost_num       = 0;
		memset(sio_port[port].tx_buf.sio_buf_ptr, 0, DEBUG_PORT_SEND_BUF_SIZE);
		
        sio_port[port].rx_buf.sio_buf_ptr    = sio_com2_rec_buf;
        sio_port[port].rx_buf.size           = DEBUG_PORT_REC_BUF_SIZE;
        sio_port[port].rx_buf.start_point    = 0;
        sio_port[port].rx_buf.end_point      = 0;
        sio_port[port].rx_buf.status         = 0;
        sio_port[port].rx_buf.lost_num       = 0;
		memset(sio_port[port].rx_buf.sio_buf_ptr, 0, DEBUG_PORT_REC_BUF_SIZE);
		
        /* Set log buffer. */
        log2_buf_info.sio_buf_ptr    = log2_buf;
        log2_buf_info.size           = LOG_BUF_SIZE;
        log2_buf_info.start_point    = 0;
        log2_buf_info.end_point      = 0;
        log2_buf_info.status         = 0;
        log2_buf_info.lost_num       = 0;
		memset(log2_buf_info.sio_buf_ptr, 0, LOG_BUF_SIZE);
		if (phy_port == COM2)
		{
		    DRV_RegHandler(TB_UART2_INT, SIO_Com2PortIRQ);
		}

        //@Daniel.Ding,CR:MS00006039,12/4/2003,begin		
		#ifdef USE_NUCLEUS
            NU_Create_HISR(&RX_FULL_ISR_DEBUG, 
                            "kpd_ISR", 
                            Handle_RX_FULL_DEBUG, 
                            2, 
                            RX_FULL_HISR_stack_DEBUG, 
                            2048);
		#else
            //NULL
		#endif
		
		break;
	#endif
	default:
		return NULL;
	}
	
	/* Below is Hardware initialization. */
	if (phy_port == COM0)
	{
	    sio_port[port].uart = (volatile SIO_REG_S *) ARM_UART0_BASE;
		*(unsigned long int*) GR_GEN2 &= ~(BIT_8 | BIT_9);
		*(unsigned long int*) GR_GEN1 |= BIT_22;				
    }
    else if (phy_port == COM1)
    {
	    sio_port[port].uart = (volatile SIO_REG_S *) ARM_UART1_BASE;
		*(unsigned long int*) GR_GEN2 &= ~(BIT_10 | BIT_11 | BIT_14);
		*(unsigned long int*) GR_GEN1 |= BIT_23;				
	}
	
	else if (phy_port == COM2)
	{
	    sio_port[port].uart = (volatile SIO_REG_S *) ARM_UART2_BASE;
		*(unsigned long int*) GR_GEN2 &= ~(BIT_12 | BIT_13 | BIT_15);
		*(unsigned long int*) GR_GEN1 |= BIT_24;				
	}
	/*  disable uart irq   */
    (sio_port[port].uart)->ien  = 0;
	
    /* byte length  (8 bits) : stop_bit  (1bit)  */
    (sio_port[port].uart)->ctl = BYTE_SIZE_8 | STOPBIT_1;
	
    /*  Enable uart irq   */
    (sio_port[port].uart)->ien = US_RX_FULL|US_RXT_OUT;
	
    /* Set baudrate! */
    //@Zhemin.Lin,  2003/08/14, begin
    //(sio_port[port].uart)->clk0 = dcb->baud_rate & 0x0FFFF;
    //(sio_port[port].uart)->clk1 = (dcb->baud_rate >> 16) & 0x0F;
    divider = GetClkDividerValue(dcb->baud_rate);
    (sio_port[port].uart)->clk0 = divider & 0x0FFFF;
    (sio_port[port].uart)->clk1 = (divider >> 16) & 0x1F;
    //@Zhemin.Lin, 2003/08/14, end
    
 	/* @Jim.zhang CR:MS00006386 2003/12/19 */    
    #ifdef _U0_FLOW_CONTROL

    	if(COM_DATA == port)//( (volatile SIO_REG_S *)ARM_UART0_BASE == sio_port[port].uart )
    	{
     	    //@lin.liu(2004-03-19). moved to pinmap_sp7100b.c
       		//*(volatile uint32 *)0x8C000020 &= 0xFFFFF3FF;
	       	//*(volatile uint32 *)0x8C000020 |= 0x0400; // set UART0 flow control ,not jtag

		    /*  water mark   */    
        	(sio_port[port].uart)->watermark   = RX_WATER_MARK + ((TX_WATER_MARK) << 8);
	
	        /*  Set hardware_flow_control  */    
    
    	   	(sio_port[port].uart)->flow_ctl = RX_WATER_MARK | RECEIVE_TIMEOUT  // Receive time out value
                                    | BIT_7 | BIT_8;// enable Rx/Tx hardware_flow control
		}
	    else
    	{
       		(sio_port[port].uart)->watermark   = RX_WATER_MARK + ((TX_WATER_MARK) << 8);   
	   	   	(sio_port[port].uart)->flow_ctl = RECEIVE_TIMEOUT | RX_WATER_MARK; // Receive time out value
 		}

	#else
    	(sio_port[port].uart)->watermark   = RX_WATER_MARK + ((TX_WATER_MARK) << 8);   
	    (sio_port[port].uart)->flow_ctl = RECEIVE_TIMEOUT | RX_WATER_MARK;	  // Receive time out value

	#endif // _U0_FLOW_CONTROL
    /* @Jim  End.CR:MS00006386  */
    
    // Purge data in SIO FIFO
    PurgeSio(port, PURGE_TX_CLEAR | PURGE_RX_CLEAR);
	
	if (phy_port == COM0)
	{
   		*(unsigned long int*) INT_IRQ_EN |= INTCTL_UART0_IRQ;
    }	
    else if (phy_port == COM1)
    {
	    *(unsigned long int*) INT_IRQ_EN |= INTCTL_UART1_IRQ;
	}
	else if (phy_port == COM2)
	{
	    *(unsigned long int*) INT_IRQ_EN |= INTCTL_UART2_IRQ;
	}
	
	/* Return handle of the sio contol block. */
	return ((HSIO) &sio_port[port]);
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_Close(uint32 port);                                          *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     CLose SIO device.                                                     *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     The handle of SIO device.                                             *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC void SIO_Close(uint32 port)
{
    SCI_ASSERT(port < MAX_SIO_PORT_NUM);
    sio_port[port].open_flag = 0;

    /* Disable uart irq */
    sio_port[port].uart->ien  = 0;

	/* Below is Hardware close. */
    if (sio_port[port].phy_port == COM0)
    {
		DRV_UnRegHandler(TB_UART0_INT);
    }
    else if (sio_port[port].phy_port == COM1)
    {
		DRV_UnRegHandler(TB_UART1_INT);
    }
	#ifdef _DUART
	else if(sio_port[port].phy_port == COM2)
	{
		DRV_UnRegHandler(TB_UART2_INT);
    }
	#endif
	/* Below is Hardware close. */
    switch (port)
	{
	case COM_DEBUG:
		break;
	#ifdef _DUART	
	case COM2:
		break;
	#endif
    case COM_DATA:
        /* Enable Uart1! */
        if(ATC_timer != SCI_NULL)
        {
		    SCI_DeleteTimer(ATC_timer);
		    ATC_timer = SCI_NULL;
		}
		break;
	default:
		SCI_ASSERT(0);
	}
}

PUBLIC  SIO_INFO_S * SIO_GetPPPPort()
{
   return &sio_port[COM_DATA];
}
/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_ReadPPPFrame()                                               *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Read a PPP frame in SIO buffer.                                       *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     If read a completed PPP frame, returns SIO_SUCCESS, else return       *
 **     SIO_FAIL.                                                             *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC  int SIO_ReadPPPFrame(uint32 port,  // Port number COM_DATA, COM_DEBUG
                     uint8  *src,            // Data to receive
                     int    size,            // MAX size to receive 
                     uint32 *dest_len        // Actual size received
                     )
{
if(Log_Enable1 == DEBUG_ENABLE && (port == COM_DEBUG))
{   
    int    space = 0;
    int    data_size;
    uint32 end;
    uint8  *buf;
    int    buf_size;
    uint8 *plog = src;
    
    SCI_ASSERT(port < (MAX_SIO_PORT_NUM) );
    SCI_ASSERT(src != NULL); 
    SCI_ASSERT(sio_port[port].open_flag == SIO_ALREADY_OPEN);

    GET_BUF_DATA_SIZE(data_size, (&sio_port[port].rx_buf)) 

    /* Make sure that the size is less than the total size of the buffer. */
    data_size = (data_size < size) ? data_size : size;

    /* 
       We get these value first in stack, in order to speed up the program.
       In this case, all caculations are used by registers.
    */
    end      = sio_port[port].rx_buf.end_point;
    buf      = sio_port[port].rx_buf.sio_buf_ptr;
    buf_size = sio_port[port].rx_buf.size;
    
    /* Read data from buf. */
    while (space < data_size)
    {
        int   nchar;
        int status;

        nchar  = buf[end++];

        // Check if it is a valid char 
        status = CheckInput(&nchar);
        if (SIO_FRAME_OK == status)
        {
            // Get a completed frame
            if (end >= buf_size)
            {
                end = 0;
            }

            sio_port[port].rx_buf.end_point = end;
            *dest_len = space;

            return SIO_PPP_SUCCESS;
        }
        else if(SIO_FRAME_NORMAL == status)
        {
            // message body
	        *src++ = (uint8) nchar;
	        space++;
        }
        else
        {
            // masked char
            data_size--;
        }

        if (end >= buf_size)
        {
            end = 0;
        }
    }

    sio_port[port].rx_buf.end_point = end;
    *dest_len = space;
    return SIO_PPP_FAIL;
}
else
	return -1;
    
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_WritePPPFrame()                                              *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Write a PPP frame in SIO buffer.                                      *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return the size of byte written to the buffer.                        *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
/*
    //@Richard
    Well, we have to try to make sure the max time we will face.
    Because there is much time I have to disable interrupt.
*/
PUBLIC  int SIO_WritePPPFrame(uint32 port, uint8 * src, int size)
{
    int    free_size;

    SCI_ASSERT(port < MAX_SIO_PORT_NUM);
    SCI_ASSERT(src != NULL);
    SCI_ASSERT(sio_port[port].open_flag == SIO_ALREADY_OPEN);

    SCI_DisableIRQ();
    
    GET_BUF_FREE_SIZE(free_size, (&sio_port[port].tx_buf)) 

    /* Make sure there is enough free place! */
    SCI_ASSERT((free_size - size) > DEBUG_LWORD_MARK);
    
    /* Head flag.*/
    sio_port[port].tx_buf.sio_buf_ptr[sio_port[port].tx_buf.start_point++] = FLAG_BYTE;
    if (sio_port[port].tx_buf.start_point >= sio_port[port].tx_buf.size)
    {
        sio_port[port].tx_buf.start_point = 0;
    }

    /* Message body! */
    EncodeMsg(&sio_port[port].tx_buf, src, size);

    /* End flag.*/
    sio_port[port].tx_buf.sio_buf_ptr[sio_port[port].tx_buf.start_point++] = FLAG_BYTE;
    if (sio_port[port].tx_buf.start_point >= sio_port[port].tx_buf.size)
    {
        sio_port[port].tx_buf.start_point = 0;
    }

    SCI_ASSERT (sio_port[port].tx_buf.start_point != sio_port[port].tx_buf.end_point);

    ENABLE_TX_INT((&sio_port[port]))

    SCI_RestoreIRQ();

	return size;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_ClearError()                                                 *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Clear serial port error.                                              *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return -1 now.                                                        *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC  uint32 SIO_ClearError(uint32 port, 
                              uint32 *error_ptr, 
                              UARTSTAT_PTR_T * uart_stat_ptr
                              )
{
    /* We will support this function later! */
    return -1;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_SendLogPacket()                                              *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Send a log packet to the buffer.                                      *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return -1 now.                                                        *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/

PUBLIC int SIO_SendLogPacket(uint32 *head_ptr,  // Message head
                             int    head_len,   // Message head len
                             uint32 *body_ptr,  // Message body, maybe NULL
                             int    body_len    // Message bodu len           
                             )
{
if(Log_Enable1 == DEBUG_ENABLE)
{
    int     size;
    uint32  status = -1;
    int     free_size;
    
#ifdef BUS_MONITOR    
    int     i;
    static  int bOnce = 0;
    char    *pch_message;
#endif    

    SCI_ASSERT(head_ptr != NULL);
    SCI_ASSERT(head_len != 0);

    /* Get the total size of the message */
    size = head_len + body_len;

    SCI_DisableIRQ();
    
// @annie.chen add(2002-09-12)
#ifdef BUS_MONITOR
    if ( 0 == bOnce )
    {
        // Add the additional flag at the beginning
        bOnce = 1;
        Write_Para_Port_Char(data[0]);
        Write_Para_Port_Char(data[0]);
    }
    
    // Send frame header data
    for (i = 1; i < sizeof(data); i++)
    {
        Write_Para_Port_Char(data[i]);
    }
 
    // 2 byte Lost number
    Write_Para_Port_Char(0);
    Write_Para_Port_Char(0); 
       
    // Send message header
    pch_message = (char*)head_ptr; // [NOTE] Change address from 4byte to byte
    for (i = 0; i < head_len; i++)
    {
        switch (*pch_message)
        {
        case FLAG_BYTE:
        case ESCAPE_BYTE:
            Write_Para_Port_Char(ESCAPE_BYTE);
            Write_Para_Port_Char(*pch_message ^ COMPLEMENT_BYTE);
            break;
            
        default:
            Write_Para_Port_Char(*pch_message );
            break;
        }

        *pch_message++;
	}
	
    // Send message body
    pch_message = (char*)body_ptr; // [NOTE] Change address from 4byte to byte
    for (i = 0; i < body_len; i++)
    {
        switch (*pch_message)
        {
        case FLAG_BYTE:
        case ESCAPE_BYTE:
            Write_Para_Port_Char(ESCAPE_BYTE);
            Write_Para_Port_Char(*pch_message ^ COMPLEMENT_BYTE);
            break;
            
        default:
            Write_Para_Port_Char(*pch_message);
            break;
        }
        *pch_message++;
    }

	// Send frame End flag
    Write_Para_Port_Char( data[0] );

#else

    // Get free size which can be hold 
    GET_BUF_FREE_SIZE(free_size, (&log_buf_info)) 

    // Not enough size to hold, because there is some masked words we have to 
    // consider before
    if ((free_size - size) <= LOG_LWORD_MARK)
    {
        data.lost_frm_num++;
        status = -1;
    }
    else
    {
        /* We must check it first!*/
        if(cur_log_frm_size == 0)
        {
            /* Well, we need not to send 0x7E, because there is no frame now.*/
            EncodeFrmHead((unsigned char *) (&data.start_flag), 11);// (sizeof(data) - 3)
			cur_log_frm_size = 11; // (sizeof(data) - 3)

            data.seq_num++;
            data.lost_frm_num  = 0;
        }
        else if ((cur_log_frm_size + size) > LOG_MAX_FRM_SIZE)
        {
            /* Well, we have to try another frame, first write 0x7E.
               Then build another frame.*/
            EncodeFrmHead((unsigned char *) (&data.end_flag), 12); // (sizeof(data) - 2);
			cur_log_frm_size = 11;// (sizeof(data) - 3)

            data.seq_num++;
            data.lost_frm_num  = 0;
        }
        cur_log_frm_size += EncodeMsg(&log_buf_info, (uint8 *) head_ptr, head_len);
		if (body_ptr != NULL)
		{
			cur_log_frm_size += EncodeMsg(&log_buf_info, (uint8 *)body_ptr, body_len);
		}
		
		// @Xueliang.Wang add(2002-07-19)
		status = 0;
		// @Xueliang.Wang add end(2002-07-19)
    }

    ENABLE_TX_INT((&sio_port[COM_DEBUG]))
#endif

    SCI_RestoreIRQ();

    return status;
}
else 
    return 0;
}

PUBLIC int SIO_SendTracePacket(
	uint32 *src,  // Message head
	int    len   // Message head len
)
{
if(Log_Enable1 == DEBUG_ENABLE)	  
{

    uint32  status = -1;
    int     free_size;
    
#ifdef BUS_MONITOR    
    int     i;
    static  int bOnce = 0;
    char    *pch_message;
#endif    
    
    /*It can make sure that the src!=NULL and len!=0 */
    //SCI_ASSERT(src != NULL);
    //SCI_ASSERT(len != 0);

	SCI_DisableIRQ();			

// @annie.chen add(2002-09-12)
#ifdef BUS_MONITOR
	#error we need to code this.
#else

    // Get free size which can be hold 
    GET_BUF_FREE_SIZE(free_size, (&log_buf_info)) 

    // Not enough size to hold, because there is some masked words we have to 
    // consider before
    if ((free_size - len) <= LOG_LWORD_MARK)
    {
        data.lost_frm_num++;
        status = -1;
    }
    else
    {
        /* We must check it first!*/
        if(cur_log_frm_size == 0)
        {
            /* Well, we need not to send 0x7E, because there is no frame now.*/
            EncodeFrmHead((unsigned char *) (&data.start_flag), 11);// (sizeof(data) - 3)
			cur_log_frm_size = 11; // (sizeof(data) - 3)

            data.seq_num++;
            data.lost_frm_num  = 0;
        }
        else if ((cur_log_frm_size + len) > LOG_MAX_FRM_SIZE)
        {
            /* Well, we have to try another frame, first write 0x7E.
               Then build another frame.*/
            EncodeFrmHead((unsigned char *) (&data.end_flag), 12); // (sizeof(data) - 2);
			cur_log_frm_size = 11;// (sizeof(data) - 3)

            data.seq_num++;
            data.lost_frm_num  = 0;
        }
    
        cur_log_frm_size += WriteTraceMsgToLogBuf(&log_buf_info, (uint8 *) src, len);
		
		// @Xueliang.Wang add(2002-07-19)
		status = 0;
		// @Xueliang.Wang add end(2002-07-19)
    }

    ENABLE_TX_INT((&sio_port[COM_DEBUG]))
#endif

    SCI_RestoreIRQ(); 

    return status;
}
else
	return 0;
}

PUBLIC int SIO_SendPacket(void * src, int size)
{
if(Log_Enable1 == DEBUG_ENABLE)
    return SIO_WritePPPFrame(COM_DEBUG, ((uint8 *)src) , size);
else
	return 0;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void PurgeSio()                                                       *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function discards all characters from the output or input buffer *
 **     of a specified communications resource. It can also terminate pending *
 **     read or write operations on the resource.                             *
 **                                                                           *
 ** OUTPUT                                                                    *
 **     None                                                                  *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
LOCAL void PurgeSio(uint32 port,       // Port number 'COM_DATA', 'COM_DEBUG'
                    uint32 clear_flag  // Clear method  'PURGE_TX_CLEAR', 
                                       // 'PURGE_RX_CLEAR'
                   )
{
    int nchar;

    SCI_ASSERT(port < MAX_SIO_PORT_NUM);

    // Clear send buffer
    if (clear_flag & PURGE_TX_CLEAR)
    {
        // Wait until all data sent from FIFO
        while( ( (sio_port[port].uart)->fifo_cnt >> 8 ) & 0x0FF );
    }

    // Clear receive buffer
    if (clear_flag & PURGE_RX_CLEAR)
    {
        while( (sio_port[port].uart)->fifo_cnt & 0x0FF )
        {
            // Get a char from FIFO
            nchar = (int) ((sio_port[port].uart)->rxd);
        }
    }
}

void SIO_ResetBuf(uint32 port)
{
	SCI_DisableIRQ();

	switch (port)
	{
	case COM_DEBUG:
		PPP_frame_type   = SIO_FRAME_NONE;    // No PPP frame 
        log_frm_flag     = LOG_FRM_NONE;      // No log frame
        char_mask_mode   = SIO_CHAR_NORMAL;   // No masked char
		cur_log_frm_size = 0;                 // No log frame

		sio_port[port].uart = (volatile SIO_REG_S *) ARM_UART0_BASE; // Get the hardware addr

        /* Set log buffer. */
        log_buf_info.sio_buf_ptr    = log_buf;
        log_buf_info.size           = LOG_BUF_SIZE;
        log_buf_info.start_point    = 0;
        log_buf_info.end_point      = 0;
        log_buf_info.status         = 0;
        log_buf_info.lost_num       = 0;
		break;
    case COM_DATA:
		break;
	default:
		break;
	}

	SCI_RestoreIRQ();
}
/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     int CheckInput()                                                      *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function checks if the input char is a valid PPP char, and       *
 **     returns the PPP frame status.                                         *
 **                                                                           *
 ** OUTPUT                                                                    *
 **     None                                                                  *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Returns SIO_FRAME_OK if a completed PPP frame is received, the frame  *
 **     is like this 0x7E, ..., 0x7E; returns SIO_FRAME_ESCAPE if a masked    *
 **     character 0x7D or 0x7E is received; returns SIO_FRAME_NORMAL 2 if it  *
 **     is a normal useful char.                                              *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     PPP_frame_type,  char_mask_mode                                       *
 **                                                                           *
 **---------------------------------------------------------------------------*/
LOCAL int CheckInput(int * nchar  // Character recevied from serial port
                    )
{
    // Initial value for status, we do not want this char
    int status = SIO_FRAME_ESCAPE;
    
    if (*nchar == FLAG_BYTE)
    {
        switch (PPP_frame_type)
        {
        case SIO_FRAME_NONE:
            // Now we begin to receive the PPP frame
            PPP_frame_type     = SIO_FRAME_HEAD;
            break;
        case SIO_FRAME_HEAD:
            // Now is the end of the PPP frame
            {
                status         = SIO_FRAME_OK;
                PPP_frame_type = SIO_FRAME_NONE;
            }
            break;
        case SIO_FRAME_ERROR:
        default:
            PPP_frame_type = SIO_FRAME_NONE;
            break;
        }
        
        return status;
    }
    
    /* Get Escape */
    if (*nchar == ESCAPE_BYTE)
    {
        // It is a masked byte, we do not need it
        char_mask_mode = SIO_CHAR_ESCAPE;
        return status;
    }
    
    /* Last time is escape */
    if (char_mask_mode == SIO_CHAR_ESCAPE)
    {
        *nchar         = *nchar ^ COMPLEMENT_BYTE;
        char_mask_mode = SIO_CHAR_NORMAL;
    }
    
    switch (PPP_frame_type)
    {
    case SIO_FRAME_NONE:
        // we are not ready to receive this char
        PPP_frame_type = SIO_FRAME_ERROR;
        break;
    case SIO_FRAME_HEAD:
        status   = SIO_FRAME_NORMAL;
        break;
    case SIO_FRAME_ERROR:
    default:
        break;
    }
    
    return status;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     int SIO_DumpGetChar()                                                 *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function scanf a char from uart receive register directly.       *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Returns a char from uart receive register.                            *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC int SIO_DumpGetChar(uint32 port)
{
    SIO_INFO_S  * sio_ptr = (SIO_INFO_S *)&sio_port[port];
    int32 nchar;
    uint32 status;
    
    /*
        check parity error, and framing error
    */
    status = (sio_ptr->uart->sts0 );
    if( status )
    {
        /* the char is invalid, try next  */
        sio_ptr->uart->iclr = 0xff;
    }

    /* 
        rx fifo full or timeout,
        try read char from device
    */
    while( !(sio_ptr->uart->fifo_cnt & 0x0FF) );
    
    nchar = (int) ( sio_ptr->uart->rxd );
    
    return ((int) nchar);
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_DumpPutChar()                                                *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function put a char to the uart send register directly.          *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC void SIO_DumpPutChar(uint32 port, char nchar)
{
    SIO_INFO_S  * sio_ptr = (SIO_INFO_S *)&sio_port[port];

    /* tx fifo is empty, so try send char  */
    while( FIFO_SIZE <= ((sio_ptr->uart->fifo_cnt >> 8) & 0x0FF) );

    /* write to tx_fifo */
    sio_ptr->uart->txd = (char)nchar;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void WriteTraceMsgToLogBuf()                                          *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function does not encode message since we send ASCII code        *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return the size put into the SIO buffer.                              *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
LOCAL int WriteTraceMsgToLogBuf(SIO_BUF_S *buf_ptr, uint8 *src, int size)
{
if(Log_Enable1 == DEBUG_ENABLE)
{
	register int    i;
	register int    start, end;
	uint8           *buf;
	register int    buf_size;
	/* 
	    Warning:
    	We must make sure there is no interrupt here!
    */
	
    /* We can make sure the call routine is right! */
//    SCI_ASSERT(buf_ptr != NULL);
//    SCI_ASSERT(src     != NULL);

    start    = buf_ptr->start_point;
    end      = buf_ptr->end_point;
    buf      = buf_ptr->sio_buf_ptr;
    buf_size = buf_ptr->size;
    
	for (i = 0; i < size; i++)
	{
        buf[start++] = *src++;
         
        if (start >= buf_size)
        {
            start = 0;
        }
        
        /* Well, there is some problem when we write mask word, 
           we have only DEBUG_LOW_MARK space for mask word.*/
        SCI_ASSERT (start != end);
	}

    buf_ptr->start_point = start;
    
	return size;
}
else
	return 0;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void EncodeMsg()                                                      *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function encode message as a PPP fram body, masked 0x7E to 0x7D  *
 **     0x5E; 0x7D to 0x7D 0x5D.                                              *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return the size put into the SIO buffer.                              *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
LOCAL int EncodeMsg(SIO_BUF_S *buf_ptr, uint8 *src, int size)
{
	register int    data_size = 0;
	register int    start, end;
	uint8           *buf;
	register int    buf_size;
	
	/* 
	    Warning:
    	We must make sure there is no interrupt here!
    */
	
    /* We can make sure the call routine is right! */
//    SCI_ASSERT(buf_ptr != NULL);
//    SCI_ASSERT(src     != NULL);

    start    = buf_ptr->start_point;
    end      = buf_ptr->end_point;
    buf      = buf_ptr->sio_buf_ptr;
    buf_size = buf_ptr->size;
    
	while(size != 0)
	{
        switch (*src)
        {
        case FLAG_BYTE:
        case ESCAPE_BYTE:
            buf[start++] = ESCAPE_BYTE;
	        if (start >= buf_size)
	        {
	            start = 0;
	        }
            buf[start++] = *src ^ COMPLEMENT_BYTE;
            data_size += 2;
            break;
        default:
            buf[start++] = *src;
            data_size++;  
            break;
        }

        src++;
         
        if (start >= buf_size)
        {
            start = 0;
        }
        
        /* Well, there is some problem when we write mask word, 
           we have only DEBUG_LOW_MARK space for mask word.*/
        SCI_ASSERT (start != end);

        size--;
	}

    buf_ptr->start_point = start;
    
	return data_size;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void EncodeFrmHead()                                                  *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function encode the head of the message as a PPP frame head,     *
 **     masked 0x7E to 0x7D 0x5E; 0x7D to 0x7D 0x5D.                          *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return the size put into the SIO buffer.                              *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
LOCAL void EncodeFrmHead(const unsigned char * data, int size)
{
    int    i;
	int    start;
	uint8  *buf;
	int    buf_size;
    uint32 data_size;

    start    = log_buf_info.start_point;
    buf      = log_buf_info.sio_buf_ptr;
    buf_size = log_buf_info.size;
    
	if (size == 12)
	{
		for (i = 0; i < 2; i++)
		{
			buf[start++] = *data++;

			if (start >= buf_size)
			{
				start = 0;
			}
		}
	}
	else
	{
        buf[start++] = *data++;

        if (start >= buf_size)
        {
            start = 0;
        }
	}

    for (i = 0; i < 10; i++)
    {
        switch (*data)
        {
        case FLAG_BYTE:
        case ESCAPE_BYTE:
            buf[start++] = ESCAPE_BYTE;
	        if (start >= buf_size)
	        {
	            start = 0;
	        }
            buf[start++] = *data ^ COMPLEMENT_BYTE;
            data_size += 2;
            break;
        default:
            buf[start++] = *data;
            data_size++;  
            break;
        }

        data++;
     
        if (start >= buf_size)
        {
            start = 0;
        }
    }

    log_buf_info.start_point = start;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_WaitEvent()                                                  *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     This function wait a comm event from a specific port.                 *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return the True or Flase.                                             *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC uint32 SIO_WaitEvent(uint32 port, uint32 * event_mask)
{
    uint32 actual_event;
    uint32 status;

    status = SCI_GetEvent(comm_event, 
                          (port << 1), 
                          SCI_AND_CLEAR, 
                          &actual_event, 
                          SCI_WAIT_FOREVER);

    /* @Richard, I will modified this tell upper task. */
    *event_mask = 0;

    return status;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_DebugPortIRQ()                                               *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Debug port IRQ.                                                       *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
//@Daniel.Ding,CR:MS00004239,9/2/2003,begin
#ifdef USE_NUCLEUS
    LOCAL void SIO_DebugPortIRQ(void)
#else
    LOCAL void SIO_DebugPortIRQ(uint32 int_num)
#endif
//@Daniel,end
{
    uint32      status;
    int         nchar;
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM_DEBUG];
 	xSignalHeaderRec      *signal_ptr = NULL;//	static uint8 cpu_led = 0;
 	
    /* Get interrupt source.*/
    status = (sio_ptr->uart->sts0);

	if (status & US_BREAK)
	{
	     sio_ptr->uart->iclr = US_BREAK;
	
	     while( sio_ptr->uart->fifo_cnt & 0x0FF )
	     {
	     	// Get a char from FIFO
	        status = (int) (sio_ptr->uart->rxd);
	     }
	}
	
    /* Recieve chars from rx buffer.*/
    if (( (status & US_RX_FULL) ) || (status & US_RXT_OUT)) 
    {
        /* 
            rx fifo full or timeout,
            try read char from device
        */
        int   fifo_count = sio_ptr->uart->fifo_cnt & 0x0FF;
        int   free_data_size;
        int   copy_data_size;
        int   start_point = (&sio_port[COM_DEBUG].rx_buf)->start_point;
        int   buf_size = (&sio_port[COM_DEBUG].rx_buf)->size;
        int   i;
        uint8 * rx_buf_ptr = (&sio_port[COM_DEBUG].rx_buf)->sio_buf_ptr;
        uint32 *rx_reg_ptr = (uint32 *) &sio_ptr->uart->rxd;

        /* Get Current rx data buffer size */
		GET_BUF_FREE_SIZE(free_data_size, (&sio_port[COM_DEBUG].rx_buf))

        copy_data_size = free_data_size < fifo_count ? free_data_size : fifo_count;

        /* Check if can direct copy to the circle buffer */
        if (start_point + copy_data_size <= buf_size)
        {
            for (i = start_point; i < start_point + copy_data_size; i++)
            {
                nchar = (int) *rx_reg_ptr;

                rx_buf_ptr[i] = nchar;
            }

            /* Check if pointer is at the end of the circle buffer. */
			if (i == buf_size) 
			{ 
				(&sio_port[COM_DEBUG].rx_buf)->start_point = 0; 
			} 
			else 
			{ 
				(&sio_port[COM_DEBUG].rx_buf)->start_point = i; 
			}
        }
        else
        {
            for (i = start_point; i < buf_size; i++)
            {
                nchar = (int) *rx_reg_ptr;

                rx_buf_ptr[i] = nchar;
            }
            
            for (i = 0; i < start_point + copy_data_size - buf_size; i++)
            {
                nchar = (int) *rx_reg_ptr;

                rx_buf_ptr[i] = nchar;
            }
			(&sio_port[COM_DEBUG].rx_buf)->start_point = i; 
        }

        /* Now if there is some more char which can not be put in, we have to ingnore */
        if (copy_data_size < fifo_count)
        {
            for (i = 0; i < fifo_count - copy_data_size; i++)
            {
                nchar = (int) *rx_reg_ptr;
            }
        }
        if (0 < copy_data_size)
        {
            uint32 status_E;
            /* Now we should let Upper task to know there is some chars received */
            //@Daniel.Ding,CR:MS00006039,12/4/2003,begin
            #ifdef USE_NUCLEUS
                NU_Activate_HISR(&RX_FULL_ISR_DEBUG);
            #else
                status_E = SCI_SetEvent(comm_event, (COM_DEBUG << 1), SCI_OR);
            #endif
            //endif
        }
		// 发送一个信号到主循环，等待接收手柄信号
		#ifdef _DUART
		if(Log_Enable1 == DEV_ENABLE)
		{
			if(g_set_info.bNewUart4Alarm & UT_QQCAMERA)
			{
				if(nchar == 0x7e)
				{
					SG_CreateSignal(SG_HH_BUF,0, &signal_ptr);
				}
			}		
			else
			{
				SG_CreateSignal(SG_HH_BUF,0, &signal_ptr);
			}
		}
		#else
		if(((Log_Enable1 == HH_ENABLE)&&(nchar == 0x7e))||Log_Enable1 ==DEV_ENABLE  )
		{
			if((g_set_info.bNewUart4Alarm & UT_XGCAMERA) && CurrCamera.currst == CAMERA_GET_XINDEX_PIC)
			{
			}		
			else
			{
				SG_CreateSignal(SG_HH_BUF,0, &signal_ptr);
			}
		}		
		#endif


    }
    /* Send chars from rx buffer.*/
    else if (status & US_TX_EMPTY)
    {
        int     nchar;
        int     buf_size;
        /* Send log meddle part of frame */
        if (LOG_FRM_MED == log_frm_flag)
        {
			DECLARE_TEMP_VARS
			GET_BUF_DATA_SIZE(data_size, (&log_buf_info))
				
			/* Get data size in buffer */
			data_size = data_size < free_count ? data_size : free_count; 

            buf_size = ( &log_buf_info )->size;
			/* None zero */
            if (0 != data_size)
			{
                /* 
                    We need these temp vars here to reduce memory load time,
                    In this case, all temp vars are register operation!
                */
                temp_end_point = ( &log_buf_info )->end_point;
                tx_buf_ptr     = ( &log_buf_info )->sio_buf_ptr;
                tx_buf_reg     = (uint32 *) &sio_ptr->uart->txd;

				if (data_size + temp_end_point <= buf_size)
				{ 
    				/* We can copy till end of the circle buffer! */

					for (i = temp_end_point; i < data_size + temp_end_point; i++)
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
						if (nchar ==  FLAG_BYTE ) 
						{ 
                            /* 
                                OK, we get a whole log frame, we should stop and 
                                check if there is any command in cmd buffer.
                            */
							log_frm_flag =  LOG_FRM_END; 
                            
                            /* we should add it to caculate the right buffer ned point.*/
							i++;
							break; 
						} 
					} 
                    /* Check if pointer is at the end of the circle buffer. */
					if (i == buf_size) 
					{ 
						( &log_buf_info )->end_point = 0; 
					} 
					else 
					{ 
						( &log_buf_info )->end_point = i; 
					}
				}
				else
				{ 
                    /* There are two parts to be load from buffer. */
					for (i = temp_end_point; i < buf_size; i++)
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 

						if (nchar ==  FLAG_BYTE ) 
						{ 
                            /* 
                                OK, we get a whole log frame, we should stop and 
                                check if there is any command in cmd buffer.
                            */
							log_frm_flag =  LOG_FRM_END ; 
                            /* we should add it to caculate the right buffer ned point.*/
							i++;
							if (i == buf_size) 
							{ 
								( &log_buf_info )->end_point = 0; 
							} 
							else 
							{ 
								( &log_buf_info )->end_point = i; 
							}
							break; 
						} 
					}
                    /* Well, we must load the last data from the begin of the circle buffer */
					if (log_frm_flag != LOG_FRM_END)
					{
						for (i = 0; i < data_size + temp_end_point - buf_size; i++) 
						{ 
                            /* Load char from buffer. */
						    nchar = tx_buf_ptr[i]; 

                            /* Wirte to hardware fifo. */
						    *tx_buf_reg = nchar; 

							if (nchar ==  FLAG_BYTE ) 
							{ 
                                /* 
                                    OK, we get a whole log frame, we should stop and 
                                    check if there is any command in cmd buffer.
                                */
							    log_frm_flag =  LOG_FRM_END; 
                            
                                /* we should add it to caculate the right buffer ned point.*/
							    i++;
								break; 
							} 
						}
                        /* Well, it is impossible that i will be at the end of the circle buffer */
						( &log_buf_info )->end_point = i;
					}
				} 
			}

            if (log_frm_flag != LOG_FRM_END)
            {
                /* We did not find the frame end flag! */
                if (data_size < free_count)
                {
					/* Well, log buffer is NULL, we should add 0x7E to enable this frame!
					*/
                    log_frm_flag       = LOG_FRM_NONE;
                    cur_log_frm_size       = 0;
                    sio_ptr->uart->txd = (char) FLAG_BYTE;
                }
                /* If load data size is equal to fifo count, 
                   that means maybe we have some other data to be load.

                   Note:
                   If it happens that there is no data in the circle buffer, we have to wait for 
                   the next interrupt to send the end flag.
                */
            }
        }
        else
        {
            /* Try Command buffer first! */
			DECLARE_TEMP_VARS
			
			GET_BUF_DATA_SIZE(data_size, (&sio_ptr->tx_buf))
			
            data_size = data_size < free_count ? data_size : free_count; 
            buf_size = ( &sio_ptr->tx_buf )->size;

			if (0 != data_size)
			{
                /* 
                    We need these temp vars here to reduce memory load time,
                    In this case, all temp vars are register operation!
                */
                temp_end_point = ( &sio_ptr->tx_buf )->end_point;
                tx_buf_ptr     = ( &sio_ptr->tx_buf )->sio_buf_ptr;
                tx_buf_reg     = (uint32 *) &sio_ptr->uart->txd;

				if (data_size + temp_end_point <= buf_size)
				{ 
    				/* We can copy till end of the circle buffer! */
					for (i = temp_end_point; i < data_size + temp_end_point; i++) 
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
					} 

                    /* Check if pointer is at the end of the circle buffer. */
					if (i == buf_size) 
					{ 
						( &sio_ptr->tx_buf )->end_point = 0; 
					} 
					else 
					{ 
						( &sio_ptr->tx_buf )->end_point = i; 
					}
				}
				else
				{ 
                    /* There are two parts to be load from buffer. */
					for (i = temp_end_point; i < buf_size; i++) 
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
					}
                    /* Copy the second part from the begin of the circle buffer */
					for (i = 0; i < data_size + temp_end_point - buf_size; i++) 
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
					}
                    /* Calculate the end point */
					( &sio_ptr->tx_buf )->end_point = i;
				} 
			}
            
			/* Check if command buffer is NULL*/
            if (data_size < free_count)
            {
                /* OK, cmd buffer is null, we have to search for log buffer */
				DECLARE_TEMP_VARS
				GET_BUF_DATA_SIZE(data_size, (&log_buf_info))
					
				data_size = data_size < free_count ? data_size : free_count; 
                buf_size = ( &log_buf_info )->size;

                if (0 != data_size)
				{
                    /* 
                        We need these temp vars here to reduce memory load time,
                        In this case, all temp vars are register operation!
                    */
                    temp_end_point = ( &log_buf_info )->end_point;
                    tx_buf_ptr     = ( &log_buf_info )->sio_buf_ptr;
                    tx_buf_reg     = (uint32 *) &sio_ptr->uart->txd;

                    /* We should set the value first! */
					log_frm_flag = LOG_FRM_MED;
                    /* Assume that the first value in the circle buffer is 0x7E.*/
					*tx_buf_reg = (char)  FLAG_BYTE;
                    /* Check if reach the end of the circle buffer. */
					temp_end_point++;
					if (temp_end_point == buf_size)
					{
						temp_end_point = 0;
					}
					
					
                    /* We have sent one data, so sub one */
					data_size--;
					
					if (data_size + temp_end_point <= buf_size)
					{ 
        				/* We can copy till end of the circle buffer! */
						for (i = temp_end_point; i < data_size + temp_end_point; i++) 
						{ 
							nchar = tx_buf_ptr[i]; 
							*tx_buf_reg = nchar; 
							
							
							if (nchar ==  FLAG_BYTE ) 
							{ 
                                /* OK, we find the end flag, so stop and check if there is cmd need to be sent.*/
								log_frm_flag =  LOG_FRM_END ; 
								i++;
								break; 
							} 
						} 
                        /* Calculate the new end point. */
						if (i == buf_size) 
						{ 
							( &log_buf_info )->end_point = 0; 
						} 
						else 
						{ 
							( &log_buf_info )->end_point = i; 
						}
					}
					else
					{ 
                        /* First copy the head */
						for (i = temp_end_point; i < buf_size; i++) 
						{ 
							nchar = tx_buf_ptr[i]; 
							*tx_buf_reg = nchar; 

                            /* Check if a frame is OK */
							if (nchar ==  FLAG_BYTE ) 
							{ 
								log_frm_flag =  LOG_FRM_END ; 
								i++;
								if (i == buf_size) 
								{ 
									( &log_buf_info )->end_point = 0; 
								} 
								else 
								{ 
									( &log_buf_info )->end_point = i; 
								}
								break; 
							} 
						} 
						if (log_frm_flag != LOG_FRM_END)
						{
                            /* Copy the remain data */
							for (i = 0; i < data_size + temp_end_point - buf_size; i++) 
							{ 
								nchar = tx_buf_ptr[i]; 
								*tx_buf_reg = nchar; 

								if (nchar ==  FLAG_BYTE ) 
								{ 
									log_frm_flag =  LOG_FRM_END ; 
									i++;
									break; 
								} 
							} 
							( &log_buf_info )->end_point = i;
						}
					} 
					
					if (log_frm_flag != LOG_FRM_END)
					{
						if (data_size < free_count - 1)
						{
							/* Well, log buffer is NULL, we should add 0x7E to enable this frame! */
							log_frm_flag       = LOG_FRM_NONE;
							cur_log_frm_size       = 0;
							*tx_buf_reg = (char) FLAG_BYTE;
							
             				/* no char in buffer, so disable tx irq  */
							sio_ptr->uart->ien &= ~(US_TX_EMPTY);
						}
						else
						{
							log_frm_flag = LOG_FRM_MED;
						}
					}
				}
				else
				{
					/* no char in buffer, so disable tx irq  */
					log_frm_flag       = LOG_FRM_NONE;
					cur_log_frm_size       = 0;
					sio_ptr->uart->ien &= ~(US_TX_EMPTY);
				}
			}
		}
    }
    else
    {
        /* other interrupt , now we don't process it  */
    }
    sio_port[COM_DEBUG].uart->iclr = 0x0FF;
}

//@Daniel.Ding,CR:MS00006039,12/4/2003,begin
void Handle_RX_FULL_DEBUG (void)
{
    SCI_SetEvent(comm_event, (COM_DEBUG << 1), SCI_OR);
}

#ifdef _DUART
//Daniel.end
/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_DebugPortIRQ()                                               *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Debug port IRQ.                                                       *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
//@Daniel.Ding,CR:MS00004239,9/2/2003,begin
#ifdef USE_NUCLEUS
    LOCAL void SIO_Com2PortIRQ(void)
#else
    LOCAL void SIO_Com2PortIRQ(uint32 int_num)
#endif
//@Daniel,end
{
    uint32      status;
    int         nchar;
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM2];
 	xSignalHeaderRec      *signal_ptr = NULL;//	static uint8 cpu_led = 0;
 	


	/* Get interrupt source.*/
    status = (sio_ptr->uart->sts0);

	if (status & US_BREAK)
	{
	    sio_ptr->uart->iclr = US_BREAK;

	    while( sio_ptr->uart->fifo_cnt & 0x0FF )
	    {
	       // Get a char from FIFO
	       status = (int) (sio_ptr->uart->rxd);

	    }
	}

    /* Recieve chars from rx buffer.*/
    if (( (status & US_RX_FULL) ) || (status & US_RXT_OUT)) 
    {
        /* 
            rx fifo full or timeout,
            try read char from device
        */
        int   fifo_count = sio_ptr->uart->fifo_cnt & 0x0FF;
        int   free_data_size;
        int   copy_data_size;
        int   start_point = (&sio_port[COM2].rx_buf)->start_point;
        int   buf_size = (&sio_port[COM2].rx_buf)->size;
        int   i;
        uint8 * rx_buf_ptr = (&sio_port[COM2].rx_buf)->sio_buf_ptr;
        uint32 *rx_reg_ptr = (uint32 *) &sio_ptr->uart->rxd;


        /* Get Current rx data buffer size */
		GET_BUF_FREE_SIZE(free_data_size, (&sio_port[COM2].rx_buf))

        copy_data_size = free_data_size < fifo_count ? free_data_size : fifo_count;

        /* Check if can direct copy to the circle buffer */
        if (start_point + copy_data_size <= buf_size)
        {
            for (i = start_point; i < start_point + copy_data_size; i++)
            {
                nchar = (int) *rx_reg_ptr;

                rx_buf_ptr[i] = nchar;
            }

            /* Check if pointer is at the end of the circle buffer. */
			if (i == buf_size) 
			{ 
				(&sio_port[COM2].rx_buf)->start_point = 0; 
			} 
			else 
			{ 
				(&sio_port[COM2].rx_buf)->start_point = i; 
			}
        }
        else
        {
            for (i = start_point; i < buf_size; i++)
            {
                nchar = (int) *rx_reg_ptr;

                rx_buf_ptr[i] = nchar;
            }

            for (i = 0; i < start_point + copy_data_size - buf_size; i++)
            {
                nchar = (int) *rx_reg_ptr;

                rx_buf_ptr[i] = nchar;
            }
			(&sio_port[COM2].rx_buf)->start_point = i; 
        }

        /* Now if there is some more char which can not be put in, we have to ingnore */
        if (copy_data_size < fifo_count)
        {
            for (i = 0; i < fifo_count - copy_data_size; i++)
            {
                nchar = (int) *rx_reg_ptr;
            }
        }
        if (0 < copy_data_size)
        {
            uint32 status_E;
            /* Now we should let Upper task to know there is some chars received */
            //@Daniel.Ding,CR:MS00006039,12/4/2003,begin
            #ifdef USE_NUCLEUS
                NU_Activate_HISR(&RX_FULL_ISR_DEBUG);
            #else
                status_E = SCI_SetEvent(comm_event, (COM2<< 1), SCI_OR);
            #endif
            //endif
        }
		// 发送一个信号到主循环，等待接收手柄信号
#ifdef _DUART
		if(((Log_Enable2 == HH_ENABLE)&&(nchar == 0x7e)) || (Log_Enable2 == LED2_ENABLE))
		{
			SG_CreateSignal(SG_COM2_BUF,0, &signal_ptr);
		}
#endif /* _DUART */

    }
    /* Send chars from rx buffer.*/
#if(1)
    else if (status & US_TX_EMPTY)
    {
        int     nchar;
        int     buf_size;
        /* Send log meddle part of frame */
        if (LOG_FRM_MED == log2_frm_flag)
        {
			DECLARE_TEMP_VARS
			GET_BUF_DATA_SIZE(data_size, (&log2_buf_info))

			/* Get data size in buffer */
			data_size = data_size < free_count ? data_size : free_count; 

            buf_size = ( &log2_buf_info )->size;
			/* None zero */
            if (0 != data_size)
			{
                /* 
                    We need these temp vars here to reduce memory load time,
                    In this case, all temp vars are register operation!
                */
                temp_end_point = ( &log2_buf_info )->end_point;
                tx_buf_ptr     = ( &log2_buf_info )->sio_buf_ptr;
                tx_buf_reg     = (uint32 *) &sio_ptr->uart->txd;

				if (data_size + temp_end_point <= buf_size)
				{ 
    				/* We can copy till end of the circle buffer! */

					for (i = temp_end_point; i < data_size + temp_end_point; i++)
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
						if (nchar ==  FLAG_BYTE ) 
						{ 
                            /* 
                                OK, we get a whole log frame, we should stop and 
                                check if there is any command in cmd buffer.
                            */
							log2_frm_flag =  LOG_FRM_END; 

                            /* we should add it to caculate the right buffer ned point.*/
							i++;
							break; 
						} 
					} 
                    /* Check if pointer is at the end of the circle buffer. */
					if (i == buf_size) 
					{ 
						( &log2_buf_info )->end_point = 0; 
					} 
					else 
					{ 
						( &log2_buf_info )->end_point = i; 
					}
				}
				else
				{ 
                    /* There are two parts to be load from buffer. */
					for (i = temp_end_point; i < buf_size; i++)
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 

						if (nchar ==  FLAG_BYTE ) 
						{ 
                            /* 
                                OK, we get a whole log frame, we should stop and 
                                check if there is any command in cmd buffer.
                            */
							log2_frm_flag =  LOG_FRM_END ; 
                            /* we should add it to caculate the right buffer ned point.*/
							i++;
							if (i == buf_size) 
							{ 
								( &log2_buf_info )->end_point = 0; 
							} 
							else 
							{ 
								( &log2_buf_info )->end_point = i; 
							}
							break; 
						} 
					}
                    /* Well, we must load the last data from the begin of the circle buffer */
					if (log2_frm_flag != LOG_FRM_END)
					{
						for (i = 0; i < data_size + temp_end_point - buf_size; i++) 
						{ 
                            /* Load char from buffer. */
						    nchar = tx_buf_ptr[i]; 

                            /* Wirte to hardware fifo. */
						    *tx_buf_reg = nchar; 

							if (nchar ==  FLAG_BYTE ) 
							{ 
                                /* 
                                    OK, we get a whole log frame, we should stop and 
                                    check if there is any command in cmd buffer.
                                */
							    log2_frm_flag =  LOG_FRM_END; 

                                /* we should add it to caculate the right buffer ned point.*/
							    i++;
								break; 
							} 
						}
                        /* Well, it is impossible that i will be at the end of the circle buffer */
						( &log2_buf_info )->end_point = i;
					}
				} 
			}

            if (log2_frm_flag != LOG_FRM_END)
            {
                /* We did not find the frame end flag! */
                if (data_size < free_count)
                {
					/* Well, log buffer is NULL, we should add 0x7E to enable this frame!
					*/
                    log2_frm_flag       = LOG_FRM_NONE;
                    cur_log2_frm_size       = 0;
                    sio_ptr->uart->txd = (char) FLAG_BYTE;
                }
                /* If load data size is equal to fifo count, 
                   that means maybe we have some other data to be load.

                   Note:
                   If it happens that there is no data in the circle buffer, we have to wait for 
                   the next interrupt to send the end flag.
                */
            }
        }
        else
        {
            /* Try Command buffer first! */
			DECLARE_TEMP_VARS

			GET_BUF_DATA_SIZE(data_size, (&sio_ptr->tx_buf))

            data_size = data_size < free_count ? data_size : free_count; 
            buf_size = ( &sio_ptr->tx_buf )->size;

			if (0 != data_size)
			{
                /* 
                    We need these temp vars here to reduce memory load time,
                    In this case, all temp vars are register operation!
                */
                temp_end_point = ( &sio_ptr->tx_buf )->end_point;
                tx_buf_ptr     = ( &sio_ptr->tx_buf )->sio_buf_ptr;
                tx_buf_reg     = (uint32 *) &sio_ptr->uart->txd;

				if (data_size + temp_end_point <= buf_size)
				{ 
    				/* We can copy till end of the circle buffer! */
					for (i = temp_end_point; i < data_size + temp_end_point; i++) 
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
					} 

                    /* Check if pointer is at the end of the circle buffer. */
					if (i == buf_size) 
					{ 
						( &sio_ptr->tx_buf )->end_point = 0; 
					} 
					else 
					{ 
						( &sio_ptr->tx_buf )->end_point = i; 
					}
				}
				else
				{ 
                    /* There are two parts to be load from buffer. */
					for (i = temp_end_point; i < buf_size; i++) 
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
					}
                    /* Copy the second part from the begin of the circle buffer */
					for (i = 0; i < data_size + temp_end_point - buf_size; i++) 
					{ 
                        /* Load char from buffer. */
						nchar = tx_buf_ptr[i]; 

                        /* Wirte to hardware fifo. */
						*tx_buf_reg = nchar; 
					}
                    /* Calculate the end point */
					( &sio_ptr->tx_buf )->end_point = i;
				} 
			}

			/* Check if command buffer is NULL*/
            if (data_size < free_count)
            {
                /* OK, cmd buffer is null, we have to search for log buffer */
				DECLARE_TEMP_VARS
				GET_BUF_DATA_SIZE(data_size, (&log2_buf_info))

				data_size = data_size < free_count ? data_size : free_count; 
                buf_size = ( &log2_buf_info )->size;

                if (0 != data_size)
				{
                    /* 
                        We need these temp vars here to reduce memory load time,
                        In this case, all temp vars are register operation!
                    */
                    temp_end_point = ( &log2_buf_info )->end_point;
                    tx_buf_ptr     = ( &log2_buf_info )->sio_buf_ptr;
                    tx_buf_reg     = (uint32 *) &sio_ptr->uart->txd;

                    /* We should set the value first! */
					log2_frm_flag = LOG_FRM_MED;
                    /* Assume that the first value in the circle buffer is 0x7E.*/
					*tx_buf_reg = (char)  FLAG_BYTE;
                    /* Check if reach the end of the circle buffer. */
					temp_end_point++;
					if (temp_end_point == buf_size)
					{
						temp_end_point = 0;
					}


                    /* We have sent one data, so sub one */
					data_size--;

					if (data_size + temp_end_point <= buf_size)
					{ 
        				/* We can copy till end of the circle buffer! */
						for (i = temp_end_point; i < data_size + temp_end_point; i++) 
						{ 
							nchar = tx_buf_ptr[i]; 
							*tx_buf_reg = nchar; 


							if (nchar ==  FLAG_BYTE ) 
							{ 
                                /* OK, we find the end flag, so stop and check if there is cmd need to be sent.*/
								log2_frm_flag =  LOG_FRM_END ; 
								i++;
								break; 
							} 
						} 
                        /* Calculate the new end point. */
						if (i == buf_size) 
						{ 
							( &log2_buf_info )->end_point = 0; 
						} 
						else 
						{ 
							( &log2_buf_info )->end_point = i; 
						}
					}
					else
					{ 
                        /* First copy the head */
						for (i = temp_end_point; i < buf_size; i++) 
						{ 
							nchar = tx_buf_ptr[i]; 
							*tx_buf_reg = nchar; 

                            /* Check if a frame is OK */
							if (nchar ==  FLAG_BYTE ) 
							{ 
								log2_frm_flag =  LOG_FRM_END ; 
								i++;
								if (i == buf_size) 
								{ 
									( &log2_buf_info )->end_point = 0; 
								} 
								else 
								{ 
									( &log2_buf_info )->end_point = i; 
								}
								break; 
							} 
						} 
						if (log2_frm_flag != LOG_FRM_END)
						{
                            /* Copy the remain data */
							for (i = 0; i < data_size + temp_end_point - buf_size; i++) 
							{ 
								nchar = tx_buf_ptr[i]; 
								*tx_buf_reg = nchar; 

								if (nchar ==  FLAG_BYTE ) 
								{ 
									log2_frm_flag =  LOG_FRM_END ; 
									i++;
									break; 
								} 
							} 
							( &log2_buf_info )->end_point = i;
						}
					} 

					if (log2_frm_flag != LOG_FRM_END)
					{
						if (data_size < free_count - 1)
						{
							/* Well, log buffer is NULL, we should add 0x7E to enable this frame! */
							log2_frm_flag       = LOG_FRM_NONE;
							cur_log2_frm_size       = 0;
							*tx_buf_reg = (char) FLAG_BYTE;

             				/* no char in buffer, so disable tx irq  */
							sio_ptr->uart->ien &= ~(US_TX_EMPTY);
						}
						else
						{
							log2_frm_flag = LOG_FRM_MED;
						}
					}
				}
				else
				{
					/* no char in buffer, so disable tx irq  */
					log2_frm_flag       = LOG_FRM_NONE;
					cur_log2_frm_size       = 0;
					sio_ptr->uart->ien &= ~(US_TX_EMPTY);
				}
			}
		}
    }
#endif
    else
    {
        /* other interrupt , now we don't process it  */
    }
    sio_port[COM2].uart->iclr = 0x0FF;
}

#endif

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_UserPortIRQ()                                                *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     User port IRQ.                                                        *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
//@Daniel.Ding,CR:MS00004577,10/8/2003,begin
#ifdef USE_NUCLEUS
    LOCAL void SIO_UserPortIRQ(void)
#else
    LOCAL void SIO_UserPortIRQ(uint32 int_num)
#endif
//Daniel,end
{
    /* We will modify it later! */
    uint32         status;
    uint32      enable_status;
    unsigned char  nchar;
    volatile SIO_REG_S  *sio_ptr = sio_port[COM_DATA].uart;
//dongjz add
 	xSignalHeaderRec      *signal_ptr = NULL;//	static uint8 cpu_led = 0;
   
	// uint32 count; ---> status replace it in order to save register
    SIO_BUF_S * pRing = &sio_port[COM_DATA].tx_buf;
    
    /* @jim.zhang CR:MS00006386 2003/12/29 */
    enable_status = (sio_ptr->ien);
	/* end CR:MS00006386 */    
	
	/* Get interrupt source.*/
    status = (sio_ptr->sts0);

	/* @jim.zhang CR:MS00006386 2003/12/29 */
	if ( status & US_BREAK )
	/* end CR:MS00006386 */
	{
	     sio_ptr->iclr = US_BREAK;
	
	     while( sio_ptr->fifo_cnt & 0x0FF )
	     {
	     	// Get a char from FIFO
	        status = (int) (sio_ptr->rxd);
	     }
	}
	//@Zhemin.Lin, 2003/08/25, begin
	else if (MIX_MODE == user_port_mode)
	{
            SIO_HandleMixIrq(status);
            return;
	}
	//@Zhemin.Lin, 2003/08/25, end
	
    /* Recieve chars from rx buffer.*/
	/* @jim.zhang CR:MS00006386 2003/12/29 */
    else if (( (status & US_RX_FULL) && (enable_status & US_RX_FULL) ) ||
        ((status & US_RXT_OUT) &&(enable_status & US_RXT_OUT)))
    /* end CR:MS00006386 */
    {
        uint32      cur_rx_time = SCI_GetTickCount();

		#ifdef _U0_FLOW_CONTROL
		if ( ring_space(&(sio_port[COM_DATA].rx_buf)) <= 256 )
		{
			//SCI_TRACE_LOW("GPRS flow control:SIO disable UserPort");
			uart0_Rx_INT_Enable(0); // Disable Rx interrupt of uart0.
		}
		#endif
		
		
		#ifdef _U0_SOFTWARE_FLOW_CONTROL

		if ( (1 == gXon_on) && (ring_space(&(sio_port[COM_DATA].rx_buf)) <= DATA_PORT_REC_BUF_SIZE*3/4) )
		{
			SCI_TRACE_LOW("Software flow control:send XOFF");
			gXon_on = 0;
			*(volatile uint32 *)0x84000000 = XOFF;
		}
		#endif
		
        // If it is possible that we got three '+'
       // if (MAX_PLUS_NUMBER >= (sio_ptr->fifo_cnt & 0x0FF)
       if (MAX_PLUS_NUMBER == (sio_ptr->fifo_cnt & 0x0FF)
            && (MODE_CHANGE_DELAY < (cur_rx_time - last_rx_time))
            && DATA_MODE == user_port_mode)
        {
            int i;
            int plus_char[MAX_PLUS_NUMBER];
            int plus_num = MAX_PLUS_NUMBER;

            // Fisrt get the char from FIFO
            for (i = 0; i < MAX_PLUS_NUMBER; i++)
            {
                plus_char[i] = (int) ( sio_ptr->rxd );

			#ifdef _U0_SOFTWARE_FLOW_CONTROL				
				if ( plus_char[i] == XON )  				
					gRemoteXon_on = 1;
				else if ( plus_char[i] == XOFF )
					gRemoteXon_on = 0;
			#endif
                // Check if it is a plus char
                if (PLUS_CHAR != plus_char[i])
                {
                    plus_num = 0;
                    break;//added by steven.shao 20070406
                }
            }

            if (MAX_PLUS_NUMBER == plus_num)
            {
                // Set Timer to check if we received a mode change command.
                uint8 temp[]={0x0d,0x0a,0x4f,0x4b,0x0d,0x0a};//added by steven.shao
                 SCI_DeactiveTimer(ATC_timer);
                SCI_ChangeTimer(ATC_timer, ATC_CheckPPP, MODE_CHANGE_DELAY + 100);  // Add 100 to make the timer response longer, so that the timeout logic will be OK.
                SCI_ActiveTimer(ATC_timer);
               //added by steven.shao     
	           if(user_port_mode == DATA_MODE)
	           {
                 SIO_ATC_SetDataMode(FALSE);
                 SIO_ATC_WriteCmdRes(temp,6);
	           }
	           //end added

            }
            else
            {
                // No, so we have to put the data back to sio buffer
                for (i = 0; i < MAX_PLUS_NUMBER; i++)
                {
                    if ( ring_add( &sio_port[COM_DATA].rx_buf, plus_char[i] ) == -1 ) 
                    {
                        continue;
                    }
                }
            }
        }
        else
        {
            /* 
                rx fifo full or timeout,
                try read char from device
            */
            while( (sio_ptr->fifo_cnt & 0x0FF) )
            {
                nchar = (unsigned char) ( sio_ptr->rxd );
                
                #ifdef _U0_SOFTWARE_FLOW_CONTROL
				if ( nchar == XON )  				
					gRemoteXon_on = 1;
				else if ( nchar == XOFF )
					gRemoteXon_on = 0;
			 	#endif
                
                if ( user_port_mode == ATC_MODE)
                {
                	if ( ring_add( &sio_port[COM_DATA].atc_buf, nchar ) == -1 ) 
                	{
                		//Hyman.wu 
                		//too long at command,it is should not valid at commands!
                		//so just clear buffer,else will lead to overrun
                		sio_port[COM_DATA].atc_buf.end_point = sio_port[COM_DATA].atc_buf.start_point;
                		continue;
                	}
                }
                else if ( ring_add( &sio_port[COM_DATA].rx_buf, nchar ) == -1 ) 
                {
                	//Hyman.wu,2005-1-24
            		//too long data will lead to overrun and deep sleep assert
            		//so discard it,this is the bad flow control but not assert
            		sio_port[COM_DATA].rx_buf.end_point = sio_port[COM_DATA].rx_buf.start_point;
                    continue;
                }

                if (user_port_mode == ATC_MODE)
                {
        //            cur_atc_frame_size++;
      	#if (__camera_debug_ || __gps_debug_)
					if((g_set_info.bNewUart4Alarm & UT_XGCAMERA) && CurrCamera.currst == CAMERA_GET_XINDEX_PIC)
					{
						//放到定时器里面每隔0.1秒发一次消息
					}						
					else{
						SG_CreateSignal(SG_GPS_BUF,0, &signal_ptr);
					}
		#else
#if (1)
                    if (((nchar == NL_CHAR) || (nchar == END_CHAR) || (N_CHAR == nchar) 
						||((g_state_info.GpsDownloadFlag ==1) && (nchar == 0X01 ||nchar == 0X43 ||nchar == 0X15 || nchar == 0X6 || nchar == 0X4 || nchar == 0X18))) 
						&& (g_state_info.GpsEnable == 0))
#endif /* (0) */
                    {
						SG_CreateSignal(0xffd/*SG_GPS_BUF*/,0, &signal_ptr);
                    }

				
		#endif

                }
            }
        }

        // Get the time now!
        last_rx_time = SCI_GetTickCount();
    }
    /* Send chars from rx buffer.*/
	/* @jim.zhang CR:MS00006386 2003/12/29 */
    else if ( (status & US_TX_EMPTY) && (enable_status & US_TX_EMPTY) )
    /* end CR:MS00006386 */
    {
      
        /* Send log meddle part of frame */
        // Here status used as counter
	    //volatile uint32 * tx_reg_addr = &sio_ptr->txd;
	    uint32 s_start_point, s_size, s_end_point;
	    uint8 * s_buf;
	    
	    status = (sio_ptr->fifo_cnt >> 8) & 0x0FF;
        
        s_start_point = pRing->start_point;
        s_end_point   = pRing->end_point;
        s_size        = pRing->size;
        s_buf         = pRing->sio_buf_ptr;
       
 #ifdef _U0_SOFTWARE_FLOW_CONTROL       
        if ( gRemoteXon_on ) // Remote device can receive data.
        {
 #endif
        while( 108 > status )
                 
        {
		    if ( s_end_point == s_start_point )
		    {
                /* no char in buffer, so disable tx irq  */
                sio_ptr->ien &= ~(US_TX_EMPTY);
                break;
		    }

		    nchar = s_buf[s_start_point++];

			if ( s_start_point >= s_size )
		        s_start_point = 0;

            sio_ptr->txd = nchar;
            status++;
//            sio_ptr->tx_char++;
        }/* End while */
        
        pRing->start_point = s_start_point;
#ifdef _U0_SOFTWARE_FLOW_CONTROL
    	}
#endif
    }
    else
    {
        /* other interrupt , now we don't process it  */
    }
    
    sio_port[COM_DATA].uart->iclr = 0x0FF;
}
LOCAL buffer_len ;
LOCAL void SetLen (int len)
{
    buffer_len = len ;
}

LOCAL int GetLen (void)
{
    return buffer_len;
}

void Handle_RX_FULL_USER (void)
{
	int len ;
	len = GetLen ();
	
	if ( NULL == AT_callback_ind) \
	{\
            DefaultAtCallback(len);\
	}\
	else \
	{\
	    (*AT_callback_ind)(len);\
	}
}

/* -----------------------------------------------------------------------------------------------

   Ring Buffer Management Routines

   Each ring buffer has:-
   An 'in' pointer, which points to the next location that new data will be inserted at.
   An 'out' pointer, which points to the next character to be removed from the buffer.
   A 'limit' value, which is the size of the data array.
   A 'data' array, of 0..(limit-1) bytes.

   When in == out, the buffer is considered empty.
   When (in + 1 % limit) == out, the buffer is considered full.

   Note that if you have a 'limit' of 10, you will only be able to store 9 characters before
   the buffer full condition is reached.

   ----------------------------------------------------------------------------------------------- */

/*
** Returns the number of characters that can be added to this ring before it becomes full.
*/
static int ring_space( SIO_BUF_S *pRing )
{
  int ret = pRing->start_point - pRing->end_point - 1;
  return (ret >= 0) ? ret : (ret + pRing->size);
}

/*
** Returns the number of continuous memory space that can be memcpy to ring buf at one time.
*/
static int ring_memcpy_space( SIO_BUF_S *pRing )
{
    int ret = pRing->start_point - pRing->end_point - 1;
    if(ret>=0)
        return ret;
    else
    {
        if(pRing->start_point == 0)
            return pRing->size - pRing->end_point - 1;
        else
        {
            return pRing->size - pRing->end_point;
        }
    }
}

/*
** This function is for buf end point move only a memcpy operation on ring buf! 
** after the memcpy,procedure should call this function to modify the end point!
** Please use this function in careful!!!
*/
static void ring_end_point_move( SIO_BUF_S *pRing, int nNum ) 
{
    pRing->end_point += nNum;

    if(pRing->end_point >= pRing->size)
        pRing->end_point -= pRing->size;
}

/*
** Add 'ch' to this ring.
** Returns -1 if there is not enough room.
** Returns 0 otherwise.
*/
static int ring_add( SIO_BUF_S *pRing, int ch )
{
    if ( ring_space( pRing ) == 0 ) {
        return -1;
    }

    pRing->sio_buf_ptr[pRing->end_point++] = ch;

    if ( pRing->end_point >= pRing->size )
        pRing->end_point = 0;

    return 0;
}

/*
** Remove a character from this ring.
** Returns -1 if there are no characters in this ring.
** Returns the character removed otherwise.
*/
static int ring_remove( SIO_BUF_S *pRing )
{
    int ret;

    if ( pRing->end_point == pRing->start_point ) {
        return -1;
    }

    ret = pRing->sio_buf_ptr[pRing->start_point++];

    if ( pRing->start_point >= pRing->size )
        pRing->start_point = 0;

    return ret;
}

int uart_init( int unit )
{
   return 0;
}

void uart_closehook( void )
{
  //  irq_Disable( IRQSerialA | IRQSerialB );
}

#ifndef dputchar
void dputchar( int ch )
{
}
#endif

void uart_putc( int port, int ch )
{

	if ( !gprs_debug )
	{
    	SIO_INFO_S *pUart = &sio_port[port];
    
        // Get the uart1 status
	    uint32 flag = *(volatile uint32 *) GR_GEN0;

    	if (!((flag & 0x1000) && ((uint32) pUart->uart == (uint32) ARM_UART1_BASE)))
    	{
    		// #richard.yang added start
    		if ( (ring_space(&pUart->tx_buf) == pUart->tx_buf.size - 1) && (pUart->uart->fifo_cnt >> 8) <= 100)
    		{
    			pUart->uart->txd = (char) ch;
    			return;
    		}
    		// #richard.yang added end
    	
    	    SCI_DisableIRQ();
    
        	//if (ring_space(&pUart->tx_buf) == 0 )
        	if( sio_port[port].tx_buf.start_point - sio_port[port].tx_buf.end_point - 1 == 0)
        	{
    
            	/* note we had to wait for this port */
	    //        pUart->tx_wait++;
    
    	        /* turn IRQs back off */
        	    SCI_RestoreIRQ();
    
	            /* 
    	         * wait for the ISR to drain some data from the ring buffer
        	     * (let's just hope that 'volatile' really works, shall we?)
            	 * NB: It is important that this loop is NOT critical.
	             */
    	        //while ( ring_space(&pUart->tx_buf) == 0 );
            	while( sio_port[port].tx_buf.start_point - sio_port[port].tx_buf.end_point - 1 == 0);
            
            	SCI_DisableIRQ();
        	    
            	ring_add( &pUart->tx_buf, ch );
        
            	//ENABLE_TX_INT((&sio_port[port]))
            
    			SCI_RestoreIRQ();
        	}
        	else
        	{
            	//ring_add( &pUart->tx_buf, ch );
            	sio_port[port].tx_buf.sio_buf_ptr[sio_port[port].tx_buf.end_point++] = ch;

	    		if ( sio_port[port].tx_buf.end_point >= sio_port[port].tx_buf.size )
    	    	{
        			sio_port[port].tx_buf.end_point = 0;
        		}	
        
            	//ENABLE_TX_INT((&sio_port[port]))
        
       		    SCI_RestoreIRQ();
        	}
    	}
	}
	else
	{
		ppp_buf.buf[ppp_buf.cur_offset++] = (uint8) ch;
	}
}


#ifdef _U0_FLOW_CONTROL


/*
** timer callback
** check the space. when space is lager than 8888888, then enable rx interrupt.
*/
LOCAL void Space_Check(void)
{
    if ( !(*(volatile uint32 *)0x83000010 & 0x1) )
	{
		if ( ring_space(&pUart->rx_buf) > EMPTY_SPACE_WATER_MARK )
		{
			uart0_Rx_INT_Enable(1); // Enable Rx interrupt of uart0.
            //SCI_TRACE_LOW("GPRS flow control:rx enable for userport.");
		}
	}
}

#endif // U0_FLOW_CONTROL


/*
** take the mutex to protect the tx buffer of PPP. 
**1 : success  0: fail
*/
int uart_mix_sem_take(void)
{
	if ( MIX_MODE == user_port_mode )
	{
	    	if ( SCI_SUCCESS != SIO_SEMTAKE)
    		{
		    	SCI_TRACE_LOW("SIO writecmdres, mutex operate error\n");
		   		return 0;
    		}
	}
	
	return 1;
}


/*
** give the mutex to protect the tx buffer of ppp
*/
void uart_mix_sem_give(void)
{
	if( MIX_MODE == user_port_mode )
		SIO_SEMGIVE;
}

/*
** Fill the protocol header of Mix mode.
*/
void uart_ppp_fill_header(int end_point)
{
	if ( MIX_MODE == user_port_mode )
	{
		int i = 0;
		int len = 0;
		int fill_point = end_point;
		uint8 checksum = 0;
		SIO_BUF_S *pTxbuffer = & sio_port[COM_DATA].tx_buf;

		// get the length of the ppp data.
		len = ( g_end_point >= end_point ) ? 
			 (g_end_point - end_point -3) : 
			 (g_end_point - end_point + pTxbuffer->size -3) ;
		
		// ppp type
		pTxbuffer->sio_buf_ptr[fill_point ++ ] = SIO_DATA_PACKET;	
		if ( fill_point >= pTxbuffer->size ) 
		{
			fill_point = 0;
		}
			
		// length (LSB 8 bits)
		pTxbuffer->sio_buf_ptr[fill_point ++ ] = len & 0xFF;	
		if ( fill_point >= pTxbuffer->size ) 
		{
			fill_point = 0;
		}
				
		// type | MSB 3 bits of len
		pTxbuffer->sio_buf_ptr[fill_point ++ ] = (len >> 8) & 0x07 | SIO_DATA_PATTERN;					

		fill_point = end_point;
	
		for (i=0; i< len+3; i++)
		{
			checksum += pTxbuffer->sio_buf_ptr[fill_point ++ ]; 

			if ( fill_point >= pTxbuffer->size ) 
				fill_point = 0;
		}

		pTxbuffer->sio_buf_ptr[ g_end_point ++ ] = checksum;
		
		if ( g_end_point >= pTxbuffer->size )
			g_end_point =  g_end_point - pTxbuffer->size;

	}

}

/*
** Get the end_point of tx buffer of PPP
*/
int uart_mix_get_end_point()
{
	int tmp;
	SIO_BUF_S * pTxBuffer = &sio_port[COM_DATA].tx_buf;

	tmp = pTxBuffer->end_point;

	g_end_point = tmp;

	if ( MIX_MODE == user_port_mode )
	{
		g_end_point += MIX_PACKET_HEAD_LENGHT;  
		if ( g_end_point >= pTxBuffer->size )
			g_end_point =  g_end_point - pTxBuffer->size;
	}

	return tmp;

}


/*
** Set the end_point of Tx buffer of PPP
** and SO can send the data out.
*/
void uart_mix_set_end_point( uint32 end_point )
{
	if ( end_point < DATA_PORT_SEND_BUF_SIZE )
	{
		sio_port[COM_DATA].tx_buf.end_point  =  end_point;
		g_end_point = end_point;
	}
	else
	{
		sio_port[COM_DATA].tx_buf.end_point  =  g_end_point;
	}
}

/*
** Check the buffer lenght for PPP data send out.
*/
int uart_mix_check( int len )
{
	if ( !gprs_debug )
	{
		int space = 0;
		SIO_BUF_S * pTxBuffer = &sio_port[COM_DATA].tx_buf;
		uint32 flag = *(volatile uint32 *) 0x8b00002c;	    // Get the uart1 status

	    if (((flag & 0x4000) && ((uint32) sio_port[COM_DATA].uart == (uint32) ARM_UART1_BASE)))
    		return 0;

		// get the ring space 
		space = (pTxBuffer->start_point) - (pTxBuffer->end_point)  - 1 ;
		space = ( space >= 0 ) ? space : (space + pTxBuffer->size ) ;

  		if ( MIX_MODE == user_port_mode )
  		{
			if ( space < len + MIX_PROTO_FIX_HEAD_LEN )
			{
				SCI_TRACE_LOW("Length 1 of Tx buffer is not long enought.the buffer size is %d , the space is %d \n", pTxBuffer->size, space);
				return 0;
			}
			else 
				return 1;
  		}
  		else
  		{
			if ( space < len )
			{
				SCI_TRACE_LOW("Length 2 of Tx buffer is not long enought. the buffer size is %d , the space is %d \n", pTxBuffer->size, space);
				return 0;
			}
			else
				return 1;
  		}
	}
	else
	{
		return 1;
	}

}


/* 
** Send the char of PPP out.
** It's the function called by PPP.
*/
__inline void uart_ppp_putc( int port, int ch )
{
	
		//shijun add
	if(user_port_mode == ATC_MODE)
	{
	    SCI_TRACE_LOW("atc mode discard ppp data");
	    return;
	}
	
	if( !gprs_debug )
	{    
    	SIO_BUF_S * pBuf = &sio_port[port].tx_buf;
    
	    pBuf->sio_buf_ptr[g_end_point++] = ch;
	
		if ( g_end_point >= pBuf->size )
    	{
    		g_end_point = 0;
    	}	
	}
 	else
	{
		ppp_buf.buf[ppp_buf.cur_offset++] = (uint8) ch;
	}
}


/* 
** Send the char of non-PPP out.
** Added by steven.shao to distinguish ppp data and normal atc command results code
*/
__inline void uart_normal_putc( int port, int ch )
{
	
	if( !gprs_debug )
	{    
    	SIO_BUF_S * pBuf = &sio_port[port].tx_buf;
    
	    pBuf->sio_buf_ptr[g_end_point++] = ch;
	
		if ( g_end_point >= pBuf->size )
    	{
    		g_end_point = 0;
    	}	
	}
 	else
	{
		ppp_buf.buf[ppp_buf.cur_offset++] = (uint8) ch;
	}
}



/*
** Get character from modem uart. Returns -1 if no such
** character is available.
*/
int uart_ppp_getc( void )
{
	/* @jim.zhang CR:MS7087 */
    int ch = -1;
    SIO_BUF_S 	*pRx_buf = &sio_port[COM_DATA].rx_buf;
    

    if (user_port_mode == ATC_MODE)
        return ch;	
	/* end CR:MS7087 */
	
    //ch = ring_remove( &pUart->rx_buf );
    if ( pRx_buf->start_point == pRx_buf->end_point )
    {
    	ch = -1; // no char to get.
    }
    else
    {
    	ch = pRx_buf->sio_buf_ptr[pRx_buf->start_point++];
    
 		if ( pRx_buf->start_point >= pRx_buf->size )
    	{
    		pRx_buf->start_point = 0;
    	}
    }

    return ch;
}


/*
** Enable / Disable the interrupt used by PPP
*/
void uart_ppp_tx_int_enable(int enable_flag)
{
    if (sio_port[COM_DATA].phy_port == COM0)
    {
	if ( enable_flag )	// enable tx interrupt.
		*(volatile uint32 *)(ARM_UART0_BASE+0x10) |= 0x2;  
	else	// disable Rx interrupt.
		*(volatile uint32 *)(ARM_UART0_BASE+0x10) &= 0xFD;
    }
    else
    {
	if ( enable_flag )	// enable tx interrupt.
		*(volatile uint32 *)(ARM_UART1_BASE+0x10) |= 0x2;  
	else	// disable Rx interrupt.
		*(volatile uint32 *)(ARM_UART1_BASE+0x10) &= 0xFD;
    }
}


/*
** Return ability of uart to accept more data
*/
int uart_ready( int port )
{
    return ring_space( &sio_port[port].tx_buf );
}

int getch( void )
{
    return -1;
}

int kbhit( void )
{
    return -1;
}


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     VOID SIO_ATC_ReadCmdLine                                              *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Read AT command from sio                                              *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void SIO_ATC_ReadCmdLine( 
        uint8 * data_ptr,      // Output argument, point to the buf which is alloced
                               // by caller 
        uint length,           // The length of cmd line which should be read from
                               // sio
        uint * read_length_ptr // The actual read length
        )
{
#if ATC_ENABLE
    int i;
    uint8 ch;
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM_DATA];
	
	for (i = 0; i < length; i++)
	{
		ch = ring_remove( &sio_ptr->atc_buf );
		
		if (ch != -1)
		{
			if (ch == atc_end_ch1 || ch == atc_end_ch2)
			{
				*data_ptr = ch;
				*read_length_ptr += 1;
				break;
			}
			else if (ch == atc_bs_ch)
			{
				data_ptr--;
				*read_length_ptr -= 1;
			}
			else
			{
				*data_ptr++ = ch;
				*read_length_ptr += 1;
			}
		}
		else
		{
			break;
		}
	}
#endif 
}


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     VOID SIO_ATC_ReadCmdLine                                              *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Read clear atc sio buffer content,the function only used in ISR	      *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void SIO_ATC_ClrCmdLine( 
        void
        )
{
#if ATC_ENABLE
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM_DATA];
	
	sio_ptr->atc_buf.end_point = sio_ptr->atc_buf.start_point;
#endif	
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void SIO_ATC_WriteCmdRes()                                            *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Write AT command to the sio                                           *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void SIO_ATC_WriteCmdRes(
        uint8 * res_data_ptr,  // Point to the response infomation buffer
        uint    length         // The length of response infomation buffer
        )
{

#if ATC_ENABLE
	if ( !gprs_debug )
	{
    	int i;
	    uint8 checksum;
		int end_point;
	
	    /* to see if the user port is in the DATA MODE */
	    if ( DATA_MODE == user_port_mode )
    	{
    	    SCI_TRACE_LOW("SIO: discard AT response in DATA_MODE \n");
    	    return;
    	}
		
		/* to see if the ringspace of tx buffer is enough to hold the data packet */
		if ( ! uart_mix_check(length ) )
		{
			SCI_TRACE_LOW("jim: in SIO_ATC_WriteCmdRes : uart_mix_check \n");
			return;		
		}


		if ( ! uart_mix_sem_take() )
		{
			SCI_TRACE_LOW("jim: in SIO_ATC_WriteCmdRes : uart_mix_sem_take \n");
			return;
		}
		
	 	end_point = uart_mix_get_end_point();
		uart_mix_set_end_point( end_point );

		//uart_ppp_tx_int_enable( 0 );	// disable tx interrupt to avoid sending an half wrong packet.

	
    	/* @Xueliang.Wang add for riyue; CR5362; (2003-10-30) */
		#ifdef _RIYUE_    
	    SCI_ATC_WakeUpOtherMcu();
		#endif  // _RIYUE_    
	    /* End; @Xueliang.Wang add for riyue; CR5362; (2003-10-30) */

    	if (  MIX_MODE == user_port_mode)
    	{
        	//send packet head
			uart_ppp_putc( COM_DATA, SIO_ATC_PACKET);
			uart_ppp_putc( COM_DATA, 0x0FF & length);
			uart_ppp_putc( COM_DATA, SIO_ATC_PATTERN |((length>>8) & 7));
	
		    checksum = SIO_ATC_PACKET + (0x0FF & length) + (SIO_ATC_PATTERN |((length>>8) & 7));

			//send packet body
		
			for (i = 0; i < length; i++ )
			{
	    		checksum += *res_data_ptr;
			    uart_ppp_putc( COM_DATA, *res_data_ptr++);
			}
			//send checksum
			uart_ppp_putc( COM_DATA, checksum);
    	}
	    else
    	{
			for (i = 0; i < length; i++)
			{
				//uart_ppp_putc( COM_DATA, *res_data_ptr++);
			   uart_normal_putc( COM_DATA, *res_data_ptr++);//modified by steven.shao 20070611
			}
		}

		/* 
		 * rewirte the end_point of tx buffer,so the packet can 
		 * be send out now.
		 */
		uart_mix_set_end_point( 0xFFFFFFFF );
	
    	uart_ppp_tx_int_enable( 1 );	// disable tx interrupt to avoid sending an half wrong packet.

	   //give mutex
    	uart_mix_sem_give();	//SIO_SEMGIVE;
	}
	else
	{
		ppp_buf.head.len = sizeof(MSG_HEAD_T) + length;
	
		SCI_MEMCPY(ppp_buf.buf, res_data_ptr, length);
	
		SIO_SendPacket(&ppp_buf, ppp_buf.head.len);
	}
	
#endif
}


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void SIO_ATC_SetCmdLineTerminateChar( )                               *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Set terminate char of of AT command                                   *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void SIO_ATC_SetCmdLineTerminateChar( 
        uint8 terminate_char1,   // Default terminate char is '\n' 
        uint8 terminate_char2    // Default terminate char is '\0'
        )
{
	#if 0 // dongjz del 
#if ATC_ENABLE
    atc_end_ch1 = terminate_char1;
    atc_end_ch2 = terminate_char2;
#endif
	#endif
}


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void SIO_ATC_SetCmdLineBackSpaceChar()                                *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Set backspace char of the AT command                                  *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void SIO_ATC_SetCmdLineBackSpaceChar( 
        uint8 back_space_char  // Default backspace char value is 8
        )
{
#if ATC_ENABLE
    atc_bs_ch = back_space_char;
#endif
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void SIO_ATC_SetDataMode()                                            *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Tell SIO change the state to data mode                                *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void SIO_ATC_SetDataMode(BOOLEAN mode)
{
#if	ATC_ENABLE

	if( user_port_mode == MIX_MODE ) 
		return;

	if (mode)
	{
    	user_port_mode = DATA_MODE;
    }
    else
    {
    	user_port_mode = ATC_MODE;
    }
#endif

	user_port_mode = ATC_MODE;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void SIO_ATC_SetEcho()                                                *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Tell SIO to echo the char it receives                                 *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
PUBLIC void SIO_ATC_SetEcho(BOOLEAN IsEcho)
{
#if ATC_ENABLE
	atc_is_echo = IsEcho;
#endif
}
/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void SIO_ATC_SetCallback()                                            *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Set callback function from SIO.                                       *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void SIO_ATC_SetCallback(ATC_CALLBACK callback)
{
#if ATC_ENABLE
    SCI_ASSERT(SCI_NULL != callback);

    AT_callback_ind = callback;
	SCI_TRACE_LOW("SIO_ATC_SetCallback %p,%p",AT_callback_ind,callback);
#endif
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void ATC_CheckPPP()                                                   *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Callback to change the mode.                                          *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
LOCAL void ATC_CheckPPP(uint32 wParam)
{
#if ATC_ENABLE
    uint32 cur_time = SCI_GetTickCount();

    if (MODE_CHANGE_DELAY < (cur_time - last_rx_time))
    {
        user_port_mode = ATC_MODE;
    }
#endif
}

PUBLIC void SIO_BuildDCB(uint32 port, SIO_CONTROL_S * dcb)
{
    uint32 divider;
    /* 
       We may not use it now, 
       If we used it later, we should modify it!
    */
	sio_port[port].dcb.baud_rate    = dcb->baud_rate;
	sio_port[port].dcb.parity       = 0;//ULCRNoParity;
	sio_port[port].dcb.stop_bits    = 0;//ULCRS1StopBit;
    sio_port[port].dcb.byte_size    = 0;//ULCR8bits;
	sio_port[port].dcb.flow_control = dcb->flow_control;

    /* Set baudrate! */
    //@Zhemin.Lin, 2003/08/14, begin
    //(sio_port[port].uart)->clk0 = dcb->baud_rate & 0x0FFFF;
    //(sio_port[port].uart)->clk1 = (dcb->baud_rate >> 16) & 0x0F;
    divider = GetClkDividerValue(dcb->baud_rate);
    (sio_port[port].uart)->clk0 = divider & 0x0FFFF;
    (sio_port[port].uart)->clk1 = (divider >> 16) & 0x1F;
    //@Zhemin.Lin, 2003/08/14, end
    

        /*  Set hardware_flow_control  */
    (sio_port[port].uart)->flow_ctl = RECEIVE_TIMEOUT |  // Receive time out value
                                      RX_WATER_MARK |    // Flow control receive threashold  
                                      sio_port[port].dcb.flow_control << FLOW_CONTROL_SHIFT; // Flow control flag

}

#ifdef BUS_MONITOR
/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  * 
 **     void Write_Para_Port_Char()                                           *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Write byte to parallel port.                                          *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
PUBLIC void Write_Para_Port_Char( char ch )
{
    *((char*)PARALLEL_PORT_REG) = ch;       /* Transmit character */
}
#endif  //BUS_MONITOR

void SIO_SaveToPPPBuf(char * data, uint32 len)
{
	int i;
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM_DATA];
    	
	for (i = 0; i < len; i++)
	{
        if(user_port_mode == ATC_MODE )
        {
        	if ( ring_add( &sio_ptr->atc_buf, *(data + i)) == -1 )
        	{
        		continue;
        	}
        }
        else if ( ring_add( &sio_ptr->rx_buf, *(data + i)) == -1 )
        {
        	continue;
        }
        
	    if (user_port_mode == ATC_MODE)	
	    {
            // Check if it is a plus char
            if (PLUS_CHAR != *(data + i))
            {
                plus_num = 0;
            }
            else
            {
            	plus_num ++;
            	if (plus_num == 3)
            	{
           		    user_port_mode = DATA_MODE;
            	}
            }

	        cur_atc_frame_size ++;
	        if (*(data + i) == atc_end_ch1 || *(data + i) == atc_end_ch2)
	        {
	            // OK, we should transmit this message to
	            //@Zhemin.Lin, CR:MS00004031, 08/29/2003, begin
	            //SCI_ASSERT(SCI_NULL != AT_callback_ind);
	            //(*AT_callback_ind) (cur_atc_frame_size);
	            DO_ATC_CALLBACK(cur_atc_frame_size);
	            //@Zhemin.Lin, CR:MS00004031, 08/29/2003, end
	
	            cur_atc_frame_size = 0;
	        }
	    }
	}

}

/*****************************************************************************/
// Description :    this function calculate the value of SIO clock divider
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/

LOCAL uint32 GetClkDividerValue(uint32 baudrate )
{
    uint32 divider;
    uint32 sysclk;

    sysclk = CHIP_GetArmClk();

    divider = sysclk / ARM_CLK_13M * baudrate;

    return divider;

}
/*****************************************************************************/
// Description :    Register error handle, when sio link error,  driver will call this handle to deal
//                       the errors
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_RegisterErrorHandle(SIO_ERROR_HANDLE errorhandle)
{
    if (NULL != errorhandle)
    {
        link_error_handle = errorhandle;
        return SCI_SUCCESS;
    }
    return SCI_ERROR;

}

/*****************************************************************************/
// Description :    Register notify handle, under the mix mode, when sio drv receive a ready
//                       packet or busy packet or status packet, it will call the notify handle
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_RegisterNotifyHandle(SIO_NOTIFY_HANDLE notifyhandle)
{
    if (notifyhandle != NULL)
    {
        notify_handle = notifyhandle;
        return SCI_SUCCESS;
    }
    return SCI_ERROR;
}

/*****************************************************************************/
// Description :    Register data receive function
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_RegisterDataRecvHandle(ATC_CALLBACK recvhandle)
{
    if (recvhandle!= NULL)
    {
        data_recv_handle = recvhandle;
        return SCI_SUCCESS;
    }
    return SCI_ERROR;
}


/*****************************************************************************/
// Description :    under the MIX_MODE,  use this function to send data packet
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_SendDataPacket(uint8 *res_data_ptr, uint32 length)
{
    uint32 i;
    uint8 checksum;
   	int end_point;
	
    if ((res_data_ptr == NULL)|(length > DATA_PORT_SEND_BUF_SIZE))
    {
        return SCI_ERROR;
    }

	/* to see if the ringspace of tx buffer is enough to hold the data packet */
	if ( ! uart_mix_check(length ) )
	{
		SCI_TRACE_LOW("jim: in SIO_SendDataPacket : uart_mix_check \n");
		return SCI_ERROR;		
	}

	if ( ! uart_mix_sem_take() )
	{
		SCI_TRACE_LOW("jim: in SIO_SendDataPacket : uart_mix_sem_take \n");
		return SCI_ERROR;
	}
		
 	end_point = uart_mix_get_end_point();
	uart_mix_set_end_point( end_point );

	//uart_ppp_tx_int_enable( 0 );	// disable tx interrupt to avoid sending an half wrong packet.

   
    if (  MIX_MODE == user_port_mode)
    {
        //send packet head
		uart_ppp_putc( COM_DATA, SIO_DATA_PACKET);
		uart_ppp_putc( COM_DATA, 0x0FF & length);
		uart_ppp_putc( COM_DATA, SIO_DATA_PATTERN |((length>>8) & 7));

	    checksum = SIO_DATA_PACKET + (0x0FF & length) + (SIO_DATA_PATTERN |((length>>8) & 7));

	    //send packet body
	    for (i = 0; i < length; i++ )
	    {
	        checksum = checksum + (*res_data_ptr);
	        uart_ppp_putc( COM_DATA, *res_data_ptr++);
	    }
	    //send checksum
	    uart_ppp_putc( COM_DATA, checksum);
    }
    else
    {
	    for (i = 0; i < length; i++)
	    {
		    //uart_ppp_putc( COM_DATA, *res_data_ptr++);
		    uart_normal_putc( COM_DATA, *res_data_ptr++);//modified by steven.shao 20070611
	    }
    }
    
   	/* 
	 * rewirte the end_point of tx buffer,so the packet can 
	 * be send out now.
	 */
	uart_mix_set_end_point( 0xFFFFFFFF);
  
    uart_ppp_tx_int_enable( 1 );	// disable tx interrupt to avoid sending an half wrong packet.

   //give mutex
    uart_mix_sem_give();	//SIO_SEMGIVE;
    
    return SCI_SUCCESS;
}

/*****************************************************************************/
// Description :    under the MIX_MODE,  use this function to send status packet
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_SendStatusPacket(uint8 status)
{
    uint8 checksum;
	int end_point;
	  
	  
    if (  MIX_MODE == user_port_mode)
    {
		/* to see if the ringspace of tx buffer is enough to hold the data packet */
		if ( ! uart_mix_check(1) )
		{
			return SCI_ERROR;		
		}


		if ( ! uart_mix_sem_take() )
		{
			return SCI_ERROR;
		}
		
	 	end_point = uart_mix_get_end_point();
		uart_mix_set_end_point( end_point );

		//uart_ppp_tx_int_enable( 0 );	// disable tx interrupt to avoid sending an half wrong packet.

        //send packet head
		uart_ppp_putc( COM_DATA, SIO_DATA_PACKET);
		uart_ppp_putc( COM_DATA, 1);
		uart_ppp_putc( COM_DATA, SIO_STATUS_PATTERN);

	    checksum = SIO_DATA_PACKET + 1 + SIO_DATA_PATTERN + status;

	    //send packet body
	    uart_ppp_putc( COM_DATA, status);
		//send checksum
		uart_ppp_putc( COM_DATA, checksum);
	   
	   
	   	/* 
		 * rewirte the end_point of tx buffer,so the packet can 
		 * be send out now.
		 */
		uart_mix_set_end_point( 0xFFFFFFFF);
	
    	uart_ppp_tx_int_enable( 1 );	// disable tx interrupt to avoid sending an half wrong packet.
	   //give mutex
	    uart_mix_sem_give();	//SIO_SEMGIVE;
    }
    else
    {
        return SCI_ERROR;
    }
    return SCI_SUCCESS;
}

/*****************************************************************************/
// Description :    under the MIX_MODE,  use this function to send ready packet
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_SendReadyPacket(void)
{
    uint8 checksum;
    int end_point;

    if (  MIX_MODE == user_port_mode)
    {
		/* to see if the ringspace of tx buffer is enough to hold the data packet */
		if ( ! uart_mix_check(0) )
		{
			return SCI_ERROR;		
		}


		if ( ! uart_mix_sem_take() )
		{
			return SCI_ERROR;
		}

	 	end_point = uart_mix_get_end_point();
		uart_mix_set_end_point( end_point );

		//uart_ppp_tx_int_enable( 0 );	// disable tx interrupt to avoid sending an half wrong packet.


        //send packet head
		uart_ppp_putc( COM_DATA, SIO_DATA_PACKET);
		uart_ppp_putc( COM_DATA, 0);
		uart_ppp_putc( COM_DATA, SIO_READY_PATTERN);

	    checksum = SIO_DATA_PACKET + 0 + SIO_READY_PATTERN;
		//send checksum
		uart_ppp_putc( COM_DATA, checksum);

		/* 
		 * rewirte the end_point of tx buffer,so the packet can 
		 * be send out now.
		 */
		uart_mix_set_end_point( 0xFFFFFFFF );

    	uart_ppp_tx_int_enable( 1 );	// disable tx interrupt to avoid sending an half wrong packet.
	    //give mutex
	    uart_mix_sem_give();	//SIO_SEMGIVE;
    
    }
    else
    {
        return SCI_ERROR;
    }
    return SCI_SUCCESS;
}

/*****************************************************************************/
// Description :    under the MIX_MODE,  use this function to send busy packet
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_SendBusyPacket(void)
{
    uint8 checksum;
	int end_point;    

    if (  MIX_MODE == user_port_mode)
    {
		/* to see if the ringspace of tx buffer is enough to hold the data packet */
		if ( ! uart_mix_check(0) )
		{
			return SCI_ERROR;		
		}


		if ( ! uart_mix_sem_take() )
		{
			return SCI_ERROR;
		}

	 	end_point = uart_mix_get_end_point();
		uart_mix_set_end_point( end_point );

		//uart_ppp_tx_int_enable( 0 );	// disable tx interrupt to avoid sending an half wrong packet.


        //send packet head
		uart_ppp_putc( COM_DATA, SIO_DATA_PACKET);
		uart_ppp_putc( COM_DATA, 0);
		uart_ppp_putc( COM_DATA, SIO_BUSY_PATTERN);

	    checksum = SIO_DATA_PACKET + 1 + SIO_BUSY_PATTERN;
		//send checksum
		uart_ppp_putc( COM_DATA, checksum);

		/* 
		 * rewirte the end_point of tx buffer,so the packet can 
		 * be send out now.
		 */
		uart_mix_set_end_point( 0xFFFFFFFF );

    	uart_ppp_tx_int_enable( 1 );	// disable tx interrupt to avoid sending an half wrong packet.
	    //give mutex
	    uart_mix_sem_give();	//SIO_SEMGIVE;
    
    }
    else
    {
        return SCI_ERROR;
    }
    return SCI_SUCCESS;
}

/*****************************************************************************/
// Description :    this functuion is used to deal with the SIO irq, when current mode is MIX_MODE
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
LOCAL void SIO_HandleMixIrq(uint32 status)
{
    int         nchar;
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM_DATA];

    /* Recieve chars from rx buffer.*/
    if (( (status & US_RX_FULL) ) || (status & US_RXT_OUT))
    {
       if (!(sio_ptr->uart->fifo_cnt&0x0FF))
       {
        //timeout
       }
       else
       {
       		//
			#ifdef _U0_FLOW_CONTROL
			if ( ring_space(&(sio_port[COM_DATA].rx_buf)) <= EMPTY_SPACE_WATER_MARK )
			{
				//SCI_TRACE_LOW("GPRS flow control:SIO disable UserPort");
				uart0_Rx_INT_Enable(0); // Disable Rx interrupt of uart0.
			}
			#endif
			//
           SIO_MixRecv(sio_ptr);
       }
    }
    /* Send chars from rx buffer.*/
    else if (status & US_TX_EMPTY)
    {
        /* Send log meddle part of frame */
        while( FIFO_SIZE > ((sio_ptr->uart->fifo_cnt >> 8) & 0x0FF) )
        {
            nchar = ring_remove( &sio_ptr->tx_buf );

            if ( nchar == -1 )      /* no more data to send! */
            {
                /* no char in buffer, so disable tx irq  */
                sio_ptr->uart->ien &= ~(US_TX_EMPTY);
                break;
            }
            sio_ptr->uart->txd = (char)nchar;
        }
    }
    else
    {
        /* other interrupt , now we don't process it  */
    }
    
    sio_ptr->uart->iclr = 0x0FF;

}

/*****************************************************************************/
// Description :    this function is used to handle recv data from SIO, when current mode is MIX_MODE
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
LOCAL void SIO_MixRecv(SIO_INFO_S *sio_ptr)
{
   uint8 nchar;
   uint8 *ptr = (uint8 *)&(mix_mode_info.packet);
   SIO_BUF_S  *sio_buf_ptr;
   int i;

   while(sio_ptr->uart->fifo_cnt&0x0FF)
   {
       nchar = sio_ptr->uart->rxd;
       //fill packet head
       if (mix_mode_info.recvlen < MIX_PACKET_HEAD_LENGHT)
       {
           if ( (SIO_DATA_PACKET != nchar)
            &&(SIO_ATC_PACKET != nchar)
            &&(0 == mix_mode_info.recvlen))
           {
               
               continue;
           }
           
           ptr[mix_mode_info.recvlen] = nchar;
           mix_mode_info.recvlen++;
           mix_mode_info.checksum += nchar;
           if (mix_mode_info.recvlen == MIX_PACKET_HEAD_LENGHT) 
           {
                 mix_mode_info.datalen = mix_mode_info.packet.length_lsb + ((mix_mode_info.packet.subtype&7)<<8);
                 SCI_TRACE_LOW("sio:datalen = %d",mix_mode_info.datalen);
                 
                 //@Shijun.cui 2005-05-18 add for stronger in mix mode
                 if (mix_mode_info.datalen > DATA_PORT_REC_BUF_SIZE)
                 {
                     RESET_MIX_MODE_INFO;
                     SCI_TRACE_LOW("sio:reset mix 1.");
                 }
                 else if ( SIO_DATA_PACKET == mix_mode_info.packet.type)
                 {
                     if ((mix_mode_info.packet.subtype & 0xF8) > 
                         (SIO_DATA_PATTERN | 
                          SIO_STATUS_PATTERN | 
                          SIO_READY_PATTERN | 
                          SIO_BUSY_PATTERN))
                     {
                         RESET_MIX_MODE_INFO;
                         SCI_TRACE_LOW("sio:reset mix 2.");
                     }
                 }
                 else if (SIO_ATC_PACKET == mix_mode_info.packet.type)
                 {
                     if (SIO_ATC_PATTERN != (mix_mode_info.packet.subtype & 0xF8))
                     {
                         RESET_MIX_MODE_INFO;
                         SCI_TRACE_LOW("sio:reset mix 3.");
                     }
                 }
           }
           continue;
       }

       if  ( SIO_DATA_PACKET == mix_mode_info.packet.type)
       {
           sio_buf_ptr = &(mix_mode_info.data_rx_buf);
       }
       else if ( SIO_ATC_PACKET == mix_mode_info.packet.type)
       {
           sio_buf_ptr = &(sio_ptr->atc_buf);
       }
       else
       {
       		SCI_TRACE_LOW("mix mode data type error.");
       		//hyman, assert case
       		SCI_ASSERT(0);
       }

       //current recev is checksum
       if (( mix_mode_info.recvlen - MIX_PACKET_HEAD_LENGHT) == mix_mode_info.datalen)
       {
            if (nchar != mix_mode_info.checksum)
            {
                 //call error handle
                 DO_ERROR_HANDLE(COM_DATA, SIO_CHECKSUM_ERROR);
            }
            else
            {
                if (SIO_DATA_PACKET == mix_mode_info.packet.type)//DATA packet
                {
                    if (mix_mode_info.packet.subtype == SIO_READY_PATTERN)
                    {
                         DO_NOTIFY_HANDLE(SIO_NOTIFY_READY, 0); 
                    }
                    else if (mix_mode_info.packet.subtype == SIO_BUSY_PATTERN)
                    {
                         DO_NOTIFY_HANDLE(SIO_NOTIFY_BUSY, 0); 
                    }
                    else if (mix_mode_info.packet.subtype == SIO_STATUS_PATTERN)
                    {
                         DO_NOTIFY_HANDLE(SIO_NOTIFY_STATUS, mix_mode_info.sio_temp_buf[0]); 
                    }
                    else //data frame
                    {
                        uint32 copy_buf_size = 0;
                        uint32 copied_size = 0;
                        SIO_BUF_S *pRx_buf = &sio_port[COM_DATA].rx_buf;
                        
                        /*Modified by Shijun.Cui 2005-01-21 CR18757, avoiding longer time in UART IRQ*/
						copy_buf_size = ring_memcpy_space(pRx_buf);
						if ( mix_mode_info.datalen > copy_buf_size )
						{
						    if ( copy_buf_size )
						    {
						    	SCI_MEMCPY( &pRx_buf->sio_buf_ptr[pRx_buf->end_point], &mix_mode_info.sio_temp_buf[0], copy_buf_size);
						    	ring_end_point_move(pRx_buf, copy_buf_size);
						    	copied_size = copy_buf_size;
						    }
						    
						    copy_buf_size = ring_memcpy_space(pRx_buf);
						    if ( copy_buf_size )
						    {
						        uint32 to_copy_len;
						        to_copy_len = (mix_mode_info.datalen - copied_size) > copy_buf_size ? copy_buf_size : mix_mode_info.datalen - copied_size;
						        SCI_MEMCPY( &pRx_buf->sio_buf_ptr[pRx_buf->end_point], &mix_mode_info.sio_temp_buf[copied_size], to_copy_len);
						        ring_end_point_move(pRx_buf, to_copy_len);
						        copied_size += to_copy_len;
						    }
						    
						    if(copied_size < mix_mode_info.datalen)
						    {
						    	SCI_TRACE_LOW("MIX SIO: buffer full!");
						    }
						}
						else
						{
								    						   
					    	SCI_MEMCPY( &pRx_buf->sio_buf_ptr[pRx_buf->end_point], &mix_mode_info.sio_temp_buf[0], mix_mode_info.datalen);
					    	ring_end_point_move(pRx_buf, mix_mode_info.datalen);
						    
						}
						//end CR18757
                        
                        //DO_DATA_CALLBACK(mix_mode_info.datalen);
                    }
                }
                else if ( SIO_ATC_PACKET == mix_mode_info.packet.type)	//ATC packet
                {
					/*@Jim.zhang CR:MS9076 2004-05-13 */
					
					uint32 datalength = 0;			//the data length of integrated AT command in one package.
					uint32 newatstart = 0;			//a new AT command start index(the length of all last at in the package)
					static uint32 lastlength = 0;	//the data length of last unintegrated AT command. 
						
					uint32 copy_buf_size = 0;
                    uint32 copied_size = 0;
                    uint32 ring_space_size = 0;
                    
                    /////////////////////////////////////////////////////////////
                    //Jason.cui 2005-01-25
                    ring_space_size = ring_space(sio_buf_ptr);
                    
                    if ( ring_space_size >= mix_mode_info.datalen )
                    {
                        copy_buf_size = ring_memcpy_space(sio_buf_ptr);
                        if ( mix_mode_info.datalen > copy_buf_size )
                        {
                            SCI_MEMCPY(&sio_buf_ptr->sio_buf_ptr[sio_buf_ptr->end_point], &mix_mode_info.sio_temp_buf[0], copy_buf_size);
                            copied_size = copy_buf_size;
                            ring_end_point_move(sio_buf_ptr, copied_size);
                            
                            //@Shijun.cui 2005-04-13
                            copy_buf_size = ring_memcpy_space(sio_buf_ptr);
                            if (mix_mode_info.datalen - copied_size <= copy_buf_size)
                            {
                                SCI_MEMCPY(&sio_buf_ptr->sio_buf_ptr[sio_buf_ptr->end_point], &mix_mode_info.sio_temp_buf[copied_size], mix_mode_info.datalen - copied_size);
                                ring_end_point_move(sio_buf_ptr, mix_mode_info.datalen - copied_size);
                            
                            }
                            else
                            {
                                //will not go here
                                SCI_ASSERT(0);
                                //sio_buf_ptr->sio_buf_ptr[sio_buf_ptr->end_point] = atc_end_ch1; //avoid AT out of order!
                                //SCI_TRACE_LOW("SIO: discard AT command");//no buf here, discard AT COMMAND
                            }
                            //end 
                            
                        }
                        else
                        {
                            SCI_MEMCPY(&sio_buf_ptr->sio_buf_ptr[sio_buf_ptr->end_point], &mix_mode_info.sio_temp_buf[0], mix_mode_info.datalen);
                            ring_end_point_move(sio_buf_ptr, mix_mode_info.datalen);
                        }
                        
                        
                        for(i=0; i<mix_mode_info.datalen; i++)
                        {
                            nchar = mix_mode_info.sio_temp_buf[i];
                            
                            if (nchar == atc_end_ch1 || nchar == atc_end_ch2)
    	            	    {
    	              		    if ( lastlength )	//there is unintegrated AT command in last pacakge.
    	              		    {
	    	              		    datalength = i + 1 + lastlength; //add last unintegrated at command 
	    	              		    lastlength = 0;
    	              		    }
    	              		    else				//last pacakge is a integrated AT package.(have terminate char)
    	              		    {
    	              			    datalength = i + 1 - newatstart; 
    	              		    } 
        	          		
        	          		    DO_ATC_CALLBACK( datalength );	// send message to upper layer.
        	          		
        	          		    newatstart = i + 1;
        	          	    }
                        }
                        
                        //the current package is an unintegrated at package.
					    if ((nchar != atc_end_ch1) && (nchar != atc_end_ch2))
					    {
						    lastlength += mix_mode_info.datalen - newatstart;
					    }
					    /* end CR:MS9076 */	
                    }
                    else
                    {
                        //Jason.cui, 2005-01-25 no enough memory to hold AT packet
                        //discard all the pending AT packet!
                        sio_buf_ptr->end_point = sio_buf_ptr->start_point;
                        SCI_TRACE_LOW("SIO: discard the pending AT packets.");
                    }
				}
            }
            
            //reset mix_mode_info
           RESET_MIX_MODE_INFO;
            
       }
       else  //recv databody of a packet
       {
           SCI_ASSERT(mix_mode_info.buf_used < DATA_PORT_REC_BUF_SIZE);
           mix_mode_info.sio_temp_buf[mix_mode_info.buf_used] = nchar;
           mix_mode_info.buf_used++;
           mix_mode_info.recvlen++;
           mix_mode_info.checksum += nchar;
       }

    }

}

/*****************************************************************************/
// Description :    this function changes userport from one mode to another,
//                       following mode transition can be done by this funcition:
//                       MIX_MODE-->ATC_MODE
//                       MIX_MODE-->DATA_MODE
//                       ATC_MODE-->MIX_MODE
//                       ATC_MODE-->DATA_MODE
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_SetUserPortMode(uint32 mode)
{
    if (user_port_mode == mode)
    {
        return SCI_SUCCESS;
    }
    
     switch (user_port_mode)
     {
         case MIX_MODE:
         	user_port_mode = mode;
         	break;
         case ATC_MODE:
         	if (mode == MIX_MODE)
         	{
         	    SCI_DisableIRQ();
         	    RESET_MIX_MODE_INFO;
         	    SIO_ATC_ClrCmdLine();//Shijun.cui 2005-02-02 Added.
         	             	         //there similarly may be some data in AT BUF while reboot after module assert
         	             	         //Should clear it!
         	    SCI_RestoreIRQ();
         	}
         	user_port_mode = mode;
         	break;
         case DATA_MODE:
         	SIO_PRINT("SIO SIO_SetUserPortMode error, current mode is DATA_MODE\n");
         	return SCI_ERROR;
         default:
         	return SCI_ERROR;
     }
     return SCI_SUCCESS;
}

/*****************************************************************************/
// Description :    read a data packet body from recev ring buffer
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC void SIO_ReadDataPacket(uint8 *data_ptr, uint32 length, uint32 *read_length_ptr)
{
    int i;
    uint8 ch;
	
	for (i = 0; i < length; i++)
	{
		ch = ring_remove( &(mix_mode_info.data_rx_buf));
		
		if (ch != -1)
		{
                    *data_ptr++ = ch;
			*read_length_ptr += 1;
		}
		else
		{
			break;
		}
	}
}

/*****************************************************************************/
// Description :    get the mode of COM_DATA port
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC uint32 SIO_GetUserPortMode(void)
{
    return (user_port_mode);
}

/*****************************************************************************/
// Description :    read a char from data receive buffer, the port must be COM_DATA, 
//                       and current mode is MIX_MODE, or DATA_MODE.
// Global resource dependence :

// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC  int   SIO_GetChar( uint32 port )
{
    SIO_BUF_S *pbuf;
    
    if (COM_DATA  != port)
    {
        return (-1);
    }

    if (MIX_MODE == user_port_mode)
    {
        pbuf = &(mix_mode_info.data_rx_buf);
        return(ring_remove(pbuf));
    }
    else if (ATC_MODE == user_port_mode)
    {
        pbuf = &(sio_port[COM_DATA].atc_buf);
        return(ring_remove(pbuf));
    }
    else if (DATA_MODE == user_port_mode)
    {
        pbuf = &(sio_port[COM_DATA].rx_buf);
        return(ring_remove(pbuf));
    }

    return (-1);
}

/*****************************************************************************/
// Description :    put a char to data send buffer, the port must be COM_DATA, 
//                       and current mode is MIX_MODE, or DATA_MODE.
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
PUBLIC  int   SIO_PutChar( uint32 port , uint8 nchar)
{
    //SIO_BUF_S *pbuf;
    
    if ((MIX_MODE == user_port_mode)&&(COM_DATA == port))
    {
        if (SCI_SUCCESS == SIO_SendDataPacket(&nchar, 1))
        {
            return 0;
        }
        else
        {
            return (-1);
        }
    }

    //uart_ppp_putc(port, nchar); 
    uart_normal_putc(port, nchar); //modified by steven.shao 20070611

   	/* 
	 * rewirte the end_point of tx buffer,so the packet can 
	 * be send out now.
	 */
	uart_mix_set_end_point( 0xFFFFFFFF );
    
    return 0;
}

/*****************************************************************************/
// Description :    default handle for data receive, just discarding this frame
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
LOCAL void DefaultDataCallback(uint32 len)
{
    uint32 i;
    
    //discard this frame
    for (i = 0; i < len; i++)
    {
        ring_remove(&(mix_mode_info.data_rx_buf));
    }
    
}

//@Zhemin.Lin, CR:MS00004213, 2003/08/22, end

/*****************************************************************************/
// Description :    default handle for AT receive, just discarding this frame
//                  DefaultAtCallback function is NOT adapt for this operation
// Global resource dependence :
// Author :         Shijun.Cui 2005-02-02
// Note :           
/*****************************************************************************/
LOCAL void DefaultAtHandle(uint32 len)
{
    uint32 i;
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM_DATA];
    
    //discard this frame
    for (i = 0; i < len; i++)
    {
        ring_remove(&(sio_ptr->atc_buf));
    }
    
}

//@Zhemin.Lin, CR:MS00004031, 08/28/2003, begin

/*****************************************************************************/
// Description :    default handle for AT receive, just discarding this frame
// Global resource dependence :
// Author :         Zhemin.Lin
// Note :           
/*****************************************************************************/
LOCAL void DefaultAtCallback(uint32 len)
{
    uint32 i;
    SIO_INFO_S  *sio_ptr = (SIO_INFO_S *)&sio_port[COM_DATA];
    
    //discard this frame
    for (i = 0; i < len; i++)
    {
        ring_remove(&(sio_ptr->rx_buf));
    }
    
}

void SIO_SendRemainLogMessage(void)
{
//	DECLARE_TEMP_VARS
	char nchar;
	
    SIO_INFO_S  * sio_ptr = (SIO_INFO_S *)&sio_port[COM_DEBUG];

	if (log_buf_info.end_point == log_buf_info.start_point)
	{
		return;
	}

    /* tx fifo is empty, so try send char  */
	while (1)
	{
		while( FIFO_SIZE <= ((sio_ptr->uart->fifo_cnt >> 8) & 0x0FF) );
		
		nchar = log_buf_info.sio_buf_ptr[log_buf_info.end_point++];
		/* write to tx_fifo */
		sio_ptr->uart->txd = (char)nchar;

		if (log_buf_info.end_point == log_buf_info.size)		
		{
			log_buf_info.end_point = 0;
		}
		
		if (log_buf_info.end_point == log_buf_info.start_point)
		{
			sio_ptr->uart->txd = (char) 0x7E;
			break;
		}
	}
}
//@Zhemin.Lin, CR:MS00004031, 08/28/2003, end




/*@jim.zhang CR:MS00008486 04/06/2004 */

/*****************************************************************************/
// Description :   get the mix used state for GPRS.
// Global resource dependence :
// Author :        Jim.zhang
// Note : return 1 means PPP and Log are mixed, 0 means not mixed.          
/*****************************************************************************/
PUBLIC BOOLEAN SIO_IsPPPMixedLogel(void)
{
	return gprs_debug;
}

/* end CR:MS00008486 */


/*****************************************************************************/
// Description :   check the DSR and set the DTR to enable/disable deep sleep
// Global resource dependence :
// Author :        Jim.zhang
// Note : 		   If DSR = 1: Disable deep sleep, DTR = 1
//                 If DSR = 0, Enable deep sleep,  DTR = 0          
/*****************************************************************************/
PUBLIC void SIO_CheckDeepSleepFlag(void)
{

#ifdef _SIO_WAKEUP_ARM
	
	if (g_sio_enable_deep_sleep)
	{
		// if DSR turn to 1, then disable deep sleep and DTR = 1 
		if ((*(volatile uint32*)(ARM_UART0_BASE + 0x08)) & BIT_8)
		{
			g_sio_enable_deep_sleep = SCI_FALSE;					//Disable deep sleep
			
			*(volatile uint32*)(ARM_UART0_BASE + 0x18) |= BIT_8;	//DTR = 1 to Send response to terminal
		}
		else
		{
			*(volatile uint32*)(ARM_UART0_BASE + 0x18) &= ~BIT_8;	//DTR = 0
		}
	}
	else
	{
		// if DSR turn to 0, then enable deep sleep and DTR = 0
		
		if (!((*(volatile uint32*)(ARM_UART0_BASE + 0x08)) & BIT_8))
		{
			g_sio_enable_deep_sleep = SCI_TRUE;						//Enable deep sleep
			
			*(volatile uint32*)(ARM_UART0_BASE + 0x18) &= ~BIT_8;	//DTR = 0
		}
		else
		{
			*(volatile uint32*)(ARM_UART0_BASE + 0x18) |= BIT_8;	//DTR = 1
		}
	}

#endif	// _SIO_WAKEUP_ARM

}

/*****************************************************************************/
// Description :   Use the given baudrate to init the uart0 register.
//                 and can receivce/send data via it.
// Global resource dependence :
// Author :        Lin.liu
// Input:
//        baudrate : the baudrate need to be set( 115200, 9600 etc ).
// Return:
//        None
// Note : 		       
/*****************************************************************************/
PUBLIC void SIO_InitUart0LowLevel( uint32 baudrate )
{
    *(unsigned long int*)(0x8B00002C) &= ~(BIT_8 | BIT_9);
	*(unsigned long int*)(0x8B000018) |= BIT_22;
    
    *(volatile uint32 *)((getRMUartBaseAddr()) + 0x10) = 0x00;
    *(volatile uint32 *)((getRMUartBaseAddr()) + 0x18) = 0x1C;
    *(volatile uint32 *)((getRMUartBaseAddr()) + 0x1C) = 0x00;
    
    *(volatile uint32 *)((getRMUartBaseAddr()) + 0x24) = 
        ( CHIP_GetArmClk() + (baudrate / 2)) / baudrate;        

    *(volatile uint32 *)((getRMUartBaseAddr()) + 0x28) = 0x00;
}

/*****************************************************************************/
//  Description:    This function used to set AT uart port mix mode
//  Author:
//  Parameter:  mix_enable  TRUE - set AT uart port in mix mode
//                          FALSE - set AT uart port not in mix mode
//  Note:              
/*****************************************************************************/
PUBLIC void SIO_ATC_SetMixMode(
               BOOLEAN mix_enable
               )
{
    gprs_debug = mix_enable;
}

/*****************************************************************************/
//  Description:    This function used to get AT uart port mix mode
//  Author:
//  Note:         
//  Return:    FALSE - mix mode disable
//             TRUE  - mix mode enable     
/*****************************************************************************/
PUBLIC BOOLEAN SIO_ATC_GetMixMode(
               void
               )
{
    return gprs_debug;
}
/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_ReadFrame()                                               *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Read a PPP frame in SIO buffer.                                       *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     If read a completed PPP frame, returns SIO_SUCCESS, else return       *
 **     SIO_FAIL.                                                             *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
PUBLIC  int SIO_ReadFrame(uint32 port,  // Port number COM_DATA, COM_DEBUG
                     uint8  *src,            // Data to receive
                     int    size,            // MAX size to receive 
                     uint32 *dest_len        // Actual size received
                     )
{
#ifdef _DUART
if(((Log_Enable2 == HH_ENABLE)&&(port == COM2)) || (g_set_info.bNewUart4Alarm&UT_PROLOCK))
#else
if(Log_Enable1 == HH_ENABLE)	 
#endif	
{ 

    int    space = 0;
    int    data_size;
    uint32 end;
    uint8  *buf;
    int    buf_size;
	uint8 *plog = src;


    SCI_ASSERT(port < (MAX_SIO_PORT_NUM) );
    SCI_ASSERT(src != NULL); 
    SCI_ASSERT(sio_port[port].open_flag == SIO_ALREADY_OPEN);

    GET_BUF_DATA_SIZE(data_size, (&sio_port[port].rx_buf)) 

    /* Make sure that the size is less than the total size of the buffer. */
    data_size = (data_size < size) ? data_size : size;

    /* 
       We get these value first in stack, in order to speed up the program.
       In this case, all caculations are used by registers.
    */
    end      = sio_port[port].rx_buf.end_point;
    buf      = sio_port[port].rx_buf.sio_buf_ptr;
    buf_size = sio_port[port].rx_buf.size;
    
    /* Read data from buf. */
    while (space < data_size)
    {
        int   nchar;
        int status;

        nchar  = buf[end++];

        // Check if it is a valid char 
        status = CheckInput(&nchar);
        if (SIO_FRAME_OK == status)
        {
            // Get a completed frame
            if (end >= buf_size)
            {
                end = 0;
            }

            sio_port[port].rx_buf.end_point = end;
            *dest_len = space;
 
            return SIO_PPP_SUCCESS;
        }
        else if(SIO_FRAME_NORMAL == status)
        {
            // message body
	        *src++ = (uint8) nchar;
	        space++;
        }
        else
        {
            // masked char
            data_size--;
        }

        if (end >= buf_size)
        {
            end = 0;
        }
    }

    sio_port[port].rx_buf.end_point = end;
    *dest_len = space;    
    return SIO_PPP_FAIL;
}
else if((Log_Enable1 == DEV_ENABLE) || (Log_Enable2 == LED2_ENABLE))
{ 

    int    space = 0;
    int    data_size;
    uint32 end;
    uint8  *buf;
    int    buf_size;
	uint8 *plog = src;


    SCI_ASSERT(port < (MAX_SIO_PORT_NUM) );
    SCI_ASSERT(src != NULL); 
    SCI_ASSERT(sio_port[port].open_flag == SIO_ALREADY_OPEN);

    GET_BUF_DATA_SIZE(data_size, (&sio_port[port].rx_buf)) 

    /* Make sure that the size is less than the total size of the buffer. */
    data_size = (data_size < size) ? data_size : size;

    /* 
       We get these value first in stack, in order to speed up the program.
       In this case, all caculations are used by registers.
    */
    end      = sio_port[port].rx_buf.end_point;
    buf      = sio_port[port].rx_buf.sio_buf_ptr;
    buf_size = sio_port[port].rx_buf.size;
    
    /* Read data from buf. */
    while (space < data_size)
    {
        int   nchar;
        int status;

        nchar  = buf[end++];
		*src++ = (uint8) nchar;
		space++;
        // Check if it is a valid char 
        if (end >= buf_size)
        {
            end = 0;
        }
    }

    sio_port[port].rx_buf.end_point = end;
    *dest_len = space;    
    return space;
}
else
	return -1;
}


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void SIO_WriteFrame()                                              *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Write a  frame in SIO buffer.                                      *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     Return the size of byte written to the buffer.                        *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **     None                                                                  *
 **                                                                           *
 **---------------------------------------------------------------------------*/
/*
    //@Richard
    Well, we have to try to make sure the max time we will face.
    Because there is much time I have to disable interrupt.
*/
PUBLIC  int SIO_WriteFrame(uint32 port, uint8 * src, int size)
{
    int    free_size,i=0;

    SCI_ASSERT(port < MAX_SIO_PORT_NUM);
    SCI_ASSERT(src != NULL);
    SCI_ASSERT(sio_port[port].open_flag == SIO_ALREADY_OPEN);

    SCI_DisableIRQ();
    
    GET_BUF_FREE_SIZE(free_size, (&sio_port[port].tx_buf)) 

    /* Make sure there is enough free place! */
    SCI_ASSERT((free_size - size) > DEBUG_LWORD_MARK);

	for(i=0;i<size;i++){
	    sio_port[port].tx_buf.sio_buf_ptr[sio_port[port].tx_buf.start_point++] = src[i];
	    if (sio_port[port].tx_buf.start_point >= sio_port[port].tx_buf.size)
	    {
	        sio_port[port].tx_buf.start_point = 0;
	    }
		SCI_ASSERT (sio_port[port].tx_buf.start_point != sio_port[port].tx_buf.end_point);
	}

    ENABLE_TX_INT((&sio_port[port]))

    SCI_RestoreIRQ();

	return size;
	
}
	

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif /* End of sio.c*/
