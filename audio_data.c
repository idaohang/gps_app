/******************************************************************************
 ** File Name:      audio_tone.c                                              *
 ** Author:         Xueliang.Wang                                             *
 ** DATE:           04/30/2003                                                *
 ** Copyright:      2003 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 04/30/2003     Xueliang.Wang    Create.                                   *
 ******************************************************************************/
/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "tb_dal.h"

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
    extern   "C"
    {
#endif

/**---------------------------------------------------------------------------*
 **                         Constant Variables                                *
 **---------------------------------------------------------------------------*/
/**********************************************************************/
//                      Alarm
/**********************************************************************/

const uint16 s_alarm_1[] =
{
    AUD_SINGLE_TONE_RING,
      32,
    2500,     90,
       0,     60,
    2500,     90,
       0,    720,
    2500,     90,
       0,     60,
    2500,     90,
       0,    720
};

/**********************************************************************/
//                      Costom tones
/**********************************************************************/
const uint16 s_custom_tone_table[][4] =
{
    // AUD_WARNING_TONE
    {1000, 0, 0, 600},
    // AUD_KEY_PRESS_TONE
    {1500, 0, 0, 200},
};

/*********************************************************************/
//                      Generic tones
/********************************************************************/
const uint16 s_gt_press_key_tone[]=
{
	AUD_SINGLE_TONE_RING,
	   4,
	1000,   0xFFFF
};

const uint16 s_gt_warning_tone[]=
{
    AUD_SINGLE_TONE_RING,
	  16,
	 450,      200,
	   0,      300,
	 450,      200,
	   0,      300
};

const uint16 s_gt_batt_warning_tone[]=
{
	AUD_SINGLE_TONE_RING,
	  16,
	2000,    250,
	   0,     50,
	2000,	 250,
	   0,     50
};

const uint16 s_gt_dial_tone[]=
{
    AUD_SINGLE_TONE_RING,
	   4,
	 450,   0xFFFF
};
	
const uint16 s_gt_special_dial_tone[]=
{
    AUD_SINGLE_TONE_RING,
	   8,
	 450,     40,
	   0,	  40
};

const uint16 s_gt_congestion_tone[]=
{
	AUD_SINGLE_TONE_RING,
	   8,
	 450,    200,
	   0,	 200
};

const uint16 s_gt_subscriber_busy_tone[]=
{
	AUD_SINGLE_TONE_RING,
	   8,
	 450,    500,
	   0,	 500
};

const uint16 s_gt_sv_call_waiting[]=
{
	AUD_SINGLE_TONE_RING,
	  16,
	 450,    200,
	   0,    600,
	 450,    200,
	   0,   3000
};

const uint16 s_gt_sv_radio_path_ack[]=
{
	AUD_SINGLE_TONE_RING,
	   4,
	 450,    200
};

const uint16 s_gt_sv_radio_path_not_avail[]=
{
	AUD_SINGLE_TONE_RING,
	   8,
	 450,    200,
	   0,    200
};

const uint16 s_gt_sv_error_info[]=
{
    AUD_SINGLE_TONE_RING,
	  16,
	 950,    330,
	1400,    330,
	1800,    330,
	   0,   1000
};

const uint16 s_gt_info_free_tone[]=
{
    AUD_SINGLE_TONE_RING,
	   8,
	 450,   1000,
	   0,   4000
};

//******************************************************************
//                      Message
//******************************************************************
const uint16 s_msg_1[] =
{      
    AUD_SINGLE_TONE_RING,
      16,
       0,    500,
    2600,    450,
       0,    300,
    2600,    450
};

const uint16 s_msg_2[] =
{
    AUD_SINGLE_TONE_RING,
      24,
       0,    500,
    1100,    130,
       0,     40,
    1400,    150,
       0,    150,
    2500,    350 
};

const uint16 s_msg_3[] =
{      
    AUD_SINGLE_TONE_RING,
      40,
       0,    500,
    1900,    100,   
       0,     80,
    1900,    100,   
       0,     80,
    1900,    100,   
       0,     80,
    1900,    100,   
       0,     80,
    1900,    100
};
#ifndef WIN32
__align(4)
#endif
const uint16 s_msg_4[] =
{
    AUD_SINGLE_TONE_RING,
      36,
       0,   1000,
     697,     40,
     750,     40,
     860,     40,
     930,     40,
    1000,     40,
    1080,     40,
       0,     60,
    2400,    250  
};


//******************************************************************
//                      Normal Rings
//******************************************************************
const uint16 s_single_tone_1[] =
{
    AUD_SINGLE_TONE_RING,
     112,
       0,    500,
    2700,     50,    
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
    2700,     50,
    2500,     50,
	   0,   1500
};

const uint16 s_single_tone_2[] =
{
    AUD_SINGLE_TONE_RING,
      88,
       0,    500,
	2200,     70,
	2700, 	  70,
	2200,     70,
	2700, 	  70,
	2200,     70,
	2700, 	  70,
       0,    200,
	2200,     70,
	2700, 	  70,
	2200,     70,
	2700,     70,
	2200,     70,
	2700, 	  70,
       0,    200,
	2200,     70,
	2700, 	  70,
	2200,     70,
	2700,     70,
	2200,     70,
	2700, 	  70,
	   0,   1500
};

const uint16 s_single_tone_3[] =
{
    AUD_SINGLE_TONE_RING,
      88,
       0,    500,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	1400,     40,
	1000,     70,
	   0,   1500
};

const uint16 s_single_tone_4[] =
{
    AUD_SINGLE_TONE_RING,
      88,
       0,    500,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	1700,     40,
	3100,     60,
	   0,   1500
};

const uint16 s_single_tone_5[] =
{
    AUD_SINGLE_TONE_RING,
      92,
       0,    500,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
       0,    200,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
    3100,     60,
    2500,     50,
	   0,   1500
};

const uint16 s_single_tone_6[] =
{
    AUD_SINGLE_TONE_RING,
      76,
       0,    500, 
	1400,     40, 
	2700,     50, 
	1400,     40, 
	2700,     50, 
	1400,     40, 
	2700,     50, 
	1400,     40, 
	2700,     50, 
       0,    200, 
	1400,     40, 
	2700,     50, 
	1400,     40, 
	2700,     50, 
	1400,     40, 
	2700,     50, 
	1400,     40, 
	2700,     50, 
	   0,   1500 
};               
//******************************************************************
//                      Midi Rings
//******************************************************************
#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_1[] = 
#include "midi_1.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_2[] = 
#include "midi_2.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_3[] = 
#include "midi_3.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_4[] = 
#include "midi_4.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_5[] = 
#include "midi_5.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_6[] = 
#include "midi_6.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_7[] = 
#include "midi_7.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_8[] = 
#include "midi_8.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_9[] = 
#include "midi_9.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_10[] = 
#include "midi_10.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_11[] = 
#include "midi_11.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_12[] = 
#include "midi_12.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_13[] = 
#include "midi_13.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_14[] = 
#include "midi_14.dat"

#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_15[] = 
#include "midi_15.dat"

//2006.08.28 added by Steven.Shao for key MIDI 
#ifndef WIN32
__align(8)
#endif
const uint8 s_midi_16[] = 
#include "key_midi.dat" 




//******************************************************************
//                      Table
//******************************************************************
// Used to generate GenericTone.
const AUD_RING_DATA_INFO_T g_generic_tone_info[] =
{
    {AUD_SINGLE_TONE_RING,  "Press Key",             sizeof(s_gt_press_key_tone), 		    (const uint8*)s_gt_press_key_tone},
    {AUD_SINGLE_TONE_RING,  "Warning",               sizeof(s_gt_warning_tone),   		    (const uint8*)s_gt_warning_tone},
    {AUD_SINGLE_TONE_RING,  "Battery Warning",       sizeof(s_gt_batt_warning_tone),        (const uint8*)s_gt_batt_warning_tone},
    {AUD_SINGLE_TONE_RING,  "Dial",                  sizeof(s_gt_dial_tone),    		    (const uint8*)s_gt_dial_tone},
    {AUD_SINGLE_TONE_RING,  "Congestion",            sizeof(s_gt_congestion_tone),    	    (const uint8*)s_gt_congestion_tone},
    {AUD_SINGLE_TONE_RING,  "Subscriber Busy",       sizeof(s_gt_subscriber_busy_tone),     (const uint8*)s_gt_subscriber_busy_tone},
    {AUD_SINGLE_TONE_RING,  "Special Dial",          sizeof(s_gt_special_dial_tone),  	    (const uint8*)s_gt_special_dial_tone},
    {AUD_SINGLE_TONE_RING,  "Call Waiting",          sizeof(s_gt_sv_call_waiting),    	    (const uint8*)s_gt_sv_call_waiting},
    {AUD_SINGLE_TONE_RING,  "Radio Path Ack",        sizeof(s_gt_sv_radio_path_ack),    	(const uint8*)s_gt_sv_radio_path_ack},
    {AUD_SINGLE_TONE_RING,  "Radio Path Not Avail",  sizeof(s_gt_sv_radio_path_not_avail),  (const uint8*)s_gt_sv_radio_path_not_avail},
    {AUD_SINGLE_TONE_RING,  "Error Info",            sizeof(s_gt_sv_error_info),    		(const uint8*)s_gt_sv_error_info},
    {AUD_SINGLE_TONE_RING,  "Info Free",             sizeof(s_gt_info_free_tone),    		(const uint8*)s_gt_info_free_tone}
}; 

// Used to message ring
const AUD_RING_DATA_INFO_T g_message_ring_info[] =
{
    {AUD_SINGLE_TONE_RING,  "Message 1",        sizeof(s_msg_1),            (const uint8*)s_msg_1         },
    {AUD_SINGLE_TONE_RING,  "Message 2",        sizeof(s_msg_2),            (const uint8*)s_msg_2         },
    {AUD_SINGLE_TONE_RING,  "Message 3",        sizeof(s_msg_3),            (const uint8*)s_msg_3         },
    {AUD_SINGLE_TONE_RING,  "Message 4",        sizeof(s_msg_4),            (const uint8*)s_msg_4     }
};
const AUD_RING_DATA_INFO_T g_alarm_ring_info[] =
{
    {AUD_SINGLE_TONE_RING,  "Alarm 1",          sizeof(s_alarm_1),      (const uint8*)s_alarm_1   }
}; 

// Used to normal ring
const AUD_RING_DATA_INFO_T g_normal_ring_table[] = 
{
    {AUD_SINGLE_TONE_RING,  "Normal ring 1",    sizeof(s_single_tone_1),    (const uint8*)s_single_tone_1 },
    {AUD_SINGLE_TONE_RING,  "Normal ring 2",    sizeof(s_single_tone_2),    (const uint8*)s_single_tone_2 },
    {AUD_SINGLE_TONE_RING,  "Normal ring 3",    sizeof(s_single_tone_3),    (const uint8*)s_single_tone_3 },
    {AUD_SINGLE_TONE_RING,  "Normal ring 4",    sizeof(s_single_tone_4),    (const uint8*)s_single_tone_4 },
    {AUD_SINGLE_TONE_RING,  "Normal ring 5",    sizeof(s_single_tone_5),    (const uint8*)s_single_tone_5 },
    {AUD_SINGLE_TONE_RING,  "Normal ring 6",    sizeof(s_single_tone_6),    (const uint8*)s_single_tone_6 }
};

// Used to midi
const AUD_RING_DATA_INFO_T g_midi_table[] = 
{
    {AUD_MIDI_RING,         "alla_turca_morzart_14",        sizeof(s_midi_1),           s_midi_1        },
    {AUD_MIDI_RING,         "anonymus_9",           		sizeof(s_midi_2),           s_midi_2        },
    {AUD_MIDI_RING,         "birthday_cake_9",           	sizeof(s_midi_3),           s_midi_3        },
    {AUD_MIDI_RING,         "bourree_9",           			sizeof(s_midi_4),           s_midi_4        },
    {AUD_MIDI_RING,         "chris_2nd_10",                 sizeof(s_midi_5),           s_midi_5        },
    {AUD_MIDI_RING,         "frog_chorus_12",           	sizeof(s_midi_6),           s_midi_6        },
    {AUD_MIDI_RING,         "furelise_12",                  sizeof(s_midi_7),           s_midi_7        },
    {AUD_MIDI_RING,         "jingle_12",           			sizeof(s_midi_8),           s_midi_8        },
    {AUD_MIDI_RING,         "lambada_15",           		sizeof(s_midi_9),           s_midi_9        },
    {AUD_MIDI_RING,         "lightly_row_10",          		sizeof(s_midi_10),          s_midi_10       },
    {AUD_MIDI_RING,         "sweetie_12",          			sizeof(s_midi_11),          s_midi_11       },
    {AUD_MIDI_RING,         "tenlittl_ind_16",          	sizeof(s_midi_12),          s_midi_12       },
    {AUD_MIDI_RING,         "traffic_cop_10",          		sizeof(s_midi_13),          s_midi_13       },
    {AUD_MIDI_RING,         "wishing_star_10",          	sizeof(s_midi_14),          s_midi_14       },
    {AUD_MIDI_RING,         "chris_15",          			sizeof(s_midi_15),          s_midi_15       },
    {AUD_MIDI_RING,         "key_midi",          		    sizeof(s_midi_16),          s_midi_16       },//added by Steven.Shao for key midi
}; 

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
    }  
#endif

