/******************************************************************************
 ** File Name:      audio.c                                                   *
 ** Author:         Richard Yang                                              *
 ** DATE:           17/01/2002                                                *
 ** Copyright:      2002 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE            NAME            DESCRIPTION                               *
 ** 17/01/2002      Richard.Yang    Create.                                   *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "os_api.h"
#include "tb_dal.h"
#include "audio_ext.h"
#include "Layer1_audio.h"
#include "layer1_midi.h"
#include "midi16_api.h"

#include "deep_sleep.h"
#include "sc6600d_reg.h"

#include "ffs.h"
#include "ffs_error.h"
/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/
#define AUDIO_DEBUG

#ifdef AUDIO_DEBUG
	#define AUDIO_PRINT		SCI_TRACE_LOW
#else
	#define AUDIO_PRINT	
#endif

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif

/**---------------------------------------------------------------------------*
 **                            Data structure                                 *
 **---------------------------------------------------------------------------*/
/**---------------------------------------------------------------------------*
 **                         Const variables                                   *
 **---------------------------------------------------------------------------*/
//CONFIG table when playing ring								
LOCAL const VB_IOCTL_T s_vb_ring_cb[AUD_DEV_MODE_MAX] = 
{
	//AUD_DEV_MODE_HANDHOLD
	{VB_AD_MIC_INPUT, VB_DA_PRI_OUTPUT, VB_AUD_NON_STEREO, AUD_SPK_NON_AMP },//AUD_DEV_MODE_HANDHOLD
	//AUD_DEV_MODE_HANDFREE
	{VB_AD_AUX_INPUT, VB_DA_AUX_OUTPUT, VB_AUD_NON_STEREO, AUD_SPK_AMP_EN },//AUD_DEV_MODE_HANDFREE
	//AUD_DEV_MODE_EARPHONE
	{VB_AD_MIC_INPUT, VB_DA_PRI_OUTPUT, VB_AUD_STEREO	 , AUD_SPK_NON_AMP},//AUD_DEV_MODE_EARPHONE
};
        
//CONFIG table when calling
LOCAL const VB_IOCTL_T s_vb_voice_cb[AUD_DEV_MODE_MAX] = 
{
	//AUD_DEV_MODE_HANDHOLD
	{VB_AD_MIC_INPUT, VB_DA_PRI_OUTPUT, VB_AUD_NON_STEREO, AUD_SPK_NON_AMP},//AUD_DEV_MODE_HANDHOLD
	//AUD_DEV_MODE_HANDFREE
	{VB_AD_AUX_INPUT, VB_DA_AUX_OUTPUT, VB_AUD_NON_STEREO, AUD_SPK_AMP_EN },//AUD_DEV_MODE_HANDFREE
	//AUD_DEV_MODE_EARPHONE
	{VB_AD_AUX_INPUT, VB_DA_PRI_OUTPUT, VB_AUD_NON_STEREO, AUD_SPK_NON_AMP},//AUD_DEV_MODE_EARPHONE
};
/**---------------------------------------------------------------------------*
 **                         Global variables                                  *
 **---------------------------------------------------------------------------*/
LOCAL uint32  				s_aud_volume_table[AUD_DEV_MODE_MAX][AUD_TYPE_MAX];
LOCAL AUD_DEV_MODE_E    	s_cur_dev_mode;     // Save current device mode
LOCAL AUD_TYPE_E        	s_cur_aud_type;     // Save current audio type

// Play control infomation.
LOCAL AUD_RING_TYPE_E   	s_cur_ring_type;
LOCAL AUD_RING_CALLBACK_PFUNC s_ring_callback_pfunc;

LOCAL BOOLEAN           	is_voice_codec_enable;
LOCAL BOOLEAN           	is_ring_playing;

extern BOOLEAN    			g_is_start_loopback;
extern AUD_RING_DATA_INFO_T g_generic_tone_info[];
LOCAL BOOLEAN   		is_codec_enable;	// Used to save codec status
LOCAL BOOLEAN   	is_custom_tone_playing;
// Used to control playing duration.
LOCAL SCI_TIMER_PTR custom_tone_duration_timer_ptr;
// Used to control playing duration.
LOCAL SCI_TIMER_PTR dtmf_tone_duration_timer_ptr;

LOCAL BOOLEAN		is_dtmf_tone_playing;

// Play control infomation.
LOCAL AUD_PLAY_INFO_T   generic_tone_play_info;
LOCAL BOOLEAN       is_generic_tone_playing;
// Used to control playing duration.
LOCAL SCI_TIMER_PTR     generic_tone_duration_timer_ptr;
// Used to control play tone and pause.
LOCAL SCI_TIMER_PTR     generic_tone_timer_ptr;
LOCAL BOOLEAN           is_voice_record_enable;	// 

/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function stops playing custom tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopCustomTone(uint32 param);
    
/*****************************************************************************/
//  Description:    This function stops playing DTMF tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopDtmfTone(uint32 param);
    
/*****************************************************************************/
//  Description:    This function stops playing generic tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopGenericTone(uint32 param);

/*****************************************************************************/
//  Description:    This function stops playing custom tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopCustomTone(uint32 param);

/*****************************************************************************/
//  Description:    This function stops playing DTMF tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopDtmfTone(uint32 param);

/*****************************************************************************/
//  Description:    This function is used to get handle tone timer expired.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudPlayGenericTone(uint32 param);

/*****************************************************************************/
//  Description:    This function is used to get handle tone timer expired.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudPlayNormalRing(uint32 param);

/**---------------------------------------------------------------------------*
 **                    customization Function Definitions                     *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function initializes audio custome interface.
//  Author:         Xueliang.Wang
//  Note:
/*****************************************************************************/
/*****************************************************************************/
//  Description:    This function init audio interface.
//  Author:         Xueliang.Wang
//	Note:           This function should be called after initializing RTOS.
/*****************************************************************************/
PUBLIC void AUD_Init(void)
{
    AUDIO_PRINT("AUD_Init: Initialize audio interface.");
    
    is_codec_enable = SCI_FALSE;
    
    AUD_CUS_Init();  
}

/*****************************************************************************/
//  Description:    This function initializes audio custome interface.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC void AUD_CUS_Init(void)
{
    uint32  i, j;

    AUDIO_PRINT("AUD_CUS_Init: Initialize custome audio interface.");
	VB_Init(0);
	*(volatile uint32*)GR_BUSCLK_ALM &= 0xffff0000; 
	
	VB_CloseVBOLDO();
	
    for (i = 0; i < AUD_DEV_MODE_MAX; i++)
    {
        for (j = 0; j < AUD_TYPE_MAX; j++)
        {
            s_aud_volume_table[i][j] = 4;
        }
    }    

    s_cur_dev_mode          = AUD_DEV_MODE_HANDHOLD;
    s_cur_aud_type          = AUD_TYPE_MAX;
    s_cur_ring_type         = AUD_RING_TYPE_MAX;
    
    is_voice_codec_enable   = SCI_FALSE;    
    is_ring_playing         = SCI_FALSE;
    
    //added for pop issue 20051215
    VB_RegDevModeVector(&s_cur_dev_mode);
    VB_RegAudTypeVector(&s_cur_aud_type); 
    //added end 
    
    MIDI_Init(SCI_PRIORITY_IDLE);
}

/*****************************************************************************/
//  Description:    This function is used to set audio device mode.
//  Author:         Xueliang.Wang
//	Note:           User should set volume after set mode.
//  Return:			If succeed, return ERR_AUD_NONE,
//                       else return error value
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_SetDevMode(
    AUD_DEV_MODE_E  mode) 	// Mode to be set
{
    AUDIO_PRINT("AUD_SetDevMode: Set to mode %d, current mode %d", mode, s_cur_dev_mode);

	// Check parameter
	SCI_ASSERT (mode<AUD_DEV_MODE_MAX);

    //if (s_cur_dev_mode != mode)
    {
	    SCI_DisableIRQ();

	    // Update current aud mode
	    s_cur_dev_mode = mode;

		// Layer1 will update vb io setting
		if ((s_cur_aud_type == AUD_TYPE_RING)||(s_cur_aud_type ==AUD_TYPE_GENERIC_TONE))
		{
			LAYER1_SwitchSoundDevice (s_vb_ring_cb[s_cur_dev_mode].vadmux,
									  s_vb_ring_cb[s_cur_dev_mode].vdacouts,
									  mode);
		}
		else
		{
			LAYER1_SwitchSoundDevice (s_vb_voice_cb[s_cur_dev_mode].vadmux,
									  s_vb_voice_cb[s_cur_dev_mode].vdacouts,
									  mode);
		}

		//AUD_ControlAmplifier(s_cur_dev_mode, s_cur_aud_type);
        if(AUD_DEV_MODE_HANDFREE == mode)
        {
           GPIO_EnableAmplifier(TRUE); //turn on Power Amplifier
        }
	    // Update VB io volume
	    if (s_cur_aud_type != AUD_TYPE_MAX)
		    AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, (s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]));

	    SCI_RestoreIRQ();
	}

    return ERR_AUD_NONE;
}
/*****************************************************************************/
//  Description:    This function enable/disable amplifier according to 
//                  current state.
//  Author:         Xueliang.Wang
//	Note:           It should be called after setting s_dev_mode, 
//                  aud_play_type or s_is_open_speaker.
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_ControlAmplifier(
    AUD_DEV_MODE_E  dev_mode,       // Current mode. 
    AUD_TYPE_E      type            // Current audio type.
    )
{	
	//del for pop issue 20051215
    //AUDIO_PRINT("AUD_ControlAmplifier: dev_mode = %d, type = %d", dev_mode, type); 
    
 	if((s_cur_aud_type == AUD_TYPE_RING)||(s_cur_aud_type ==AUD_TYPE_GENERIC_TONE))	//maybe we need consider other tones.
 	{
		GPIO_EnableAmplifier (s_vb_ring_cb[dev_mode].ampen);
 	}
	else
	{
 	 	GPIO_EnableAmplifier (s_vb_voice_cb[dev_mode].ampen);
	}
	
    return ERR_AUD_NONE;
}    


/*****************************************************************************/
//  Description:    This function is used to set audio device mode according
//                  to special hardware.
//  Author:         Xueliang.Wang
//  Note:
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_CUS_SetDevMode(// If succeed, return ERR_AUD_NONE,
                                    // else return error value
    AUD_DEV_MODE_E  dev_mode        // Mode to be set
    )
{
    AUDIO_PRINT("AUD_CUS_SetDevMode: dev_mode = %d", dev_mode);

    switch (dev_mode)
    {
        case AUD_DEV_MODE_HANDHOLD:
            LAYER1_SwitchSoundDevice(SCI_FALSE, SCI_FALSE, AUD_DEV_MODE_HANDHOLD);
        break;
        
        case AUD_DEV_MODE_HANDFREE:
            LAYER1_SwitchSoundDevice(SCI_TRUE, SCI_TRUE, AUD_DEV_MODE_HANDFREE);
        break;
        
        default:
            SCI_ASSERT(0);
            break;
    }

    return ERR_AUD_NONE;
}


/*****************************************************************************/
//  Description:    This function is used to get current audio device mode.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC AUD_DEV_MODE_E AUD_GetDevMode(void)
{
    return s_cur_dev_mode;
}

/*****************************************************************************/
//  Description:    This function sets volume of specified mode and type.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_SetVolume( // If succeed, return ERR_AUD_NONE,
                                // else return error value
    AUD_DEV_MODE_E  mode,       // Specified mode 
    AUD_TYPE_E      type,       // Specified audio type
    uint32          speaker_vol // Volume value of speaker, 
                                // (0<= speaker_vol <=AUD_MAX_SPEAKER_VOLUME).
    )
{
    AUDIO_PRINT("AUD_SetVolume: mode = %d, type = %d, speaker_vol = %d", mode, type, speaker_vol);
    
    if (speaker_vol > AUD_MAX_SPEAKER_VOLUME)
    {
        speaker_vol = AUD_MAX_SPEAKER_VOLUME;
    }
	
    // Save to volume table.
    if ((mode < AUD_DEV_MODE_MAX) && (type < AUD_TYPE_MAX))
    {
	    s_aud_volume_table[mode][type] = speaker_vol;
	    
	    if ((mode == s_cur_dev_mode) && (type == s_cur_aud_type))
	    {
	        AUD_CUS_SetVolume(mode, type, s_aud_volume_table[mode][type]);
	    }
    }    
    
    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function sets volume by mode.
//  Author:         Xueliang.Wang
//  Note:
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_CUS_SetVolume( // If succeed, return ERR_AUD_NONE,
                                    // else return error value
    AUD_DEV_MODE_E  mode,       // Specified mode
    AUD_TYPE_E      type,       // Specified audio type
    uint16          speaker_vol // Volume value of speaker,
                                // (0<= speaker_vol <=AUD_MAX_SPEAKER_VOLUME).
    )
{
    ERR_AUD_E		status = ERR_AUD_NONE;
    AUD_RING_TYPE_E ring_type = AUD_GetRingType();

	AUDIO_PRINT("AUD_CUS_SetVolume: mode = %d, type = %d, speaker_vol = %d", mode, type, speaker_vol);

    if ((mode == s_cur_dev_mode) && (type == s_cur_aud_type)
    	&& ((mode != AUD_DEV_MODE_MAX) && (type != AUD_TYPE_MAX)))
    {
        if ((type == AUD_TYPE_RING) && ((ring_type == AUD_MIDI_RING) || (ring_type == AUD_SMAF_RING)))
        {
            MIDI_SetVolume(speaker_vol);
        }
        else
        {
            LAYER1_SetVolume(speaker_vol);
        }        
        status = ERR_AUD_NONE;
    }
    else
    {
        status = ERR_AUD_PARAM;
    }

    return status;
}

/*****************************************************************************/
//  Description:    This function get Aud volume value. It should
//					be called in VB_BY_ARM to set vb PGA 
//  Author:         Jimmy.Jia
//	Note:           
/*****************************************************************************/
PUBLIC uint32 AUD_GetAudVolume (void)
{
	return (s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
}

/*****************************************************************************/
//  Description:    This function is used to prepare play new audio. It should
//					be called before play new audio.
//  Author:         Xueliang.Wang
//	Note:
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_CUS_PreparePlay(
    AUD_TYPE_E      new_type,
    AUD_RING_TYPE_E ring_type
    )
{
    ERR_AUD_E  status = ERR_AUD_NONE;

    AUDIO_PRINT("AUD_CUS_PreparePlay: cur_type = %d, new_type = %d !", s_cur_aud_type, new_type);

   // SCI_DisableIRQ();

    // Audio priorities
    switch (s_cur_aud_type)
    {
        case AUD_TYPE_CUSTOM_TONE:
            status = AUD_StopCustomTone();
        	break;

        case AUD_TYPE_DTMF_TONE:
            status = AUD_StopDtmfTone();
        	break;

        case AUD_TYPE_GENERIC_TONE:
            status = AUD_StopGenericTone();
        	break;

        case AUD_TYPE_RING:
            if ((new_type == AUD_TYPE_RING) || (new_type == AUD_TYPE_RECORD))
            {
	            status = AUD_StopRing();
            }
            else // new_type == AUD_TYPE_VOICE DTMF or other tone or AUD_TYPE_MAX
            {
                status = ERR_AUD_CONFLICT;
            }
        	break;

        case AUD_TYPE_VOICE:
            if ((new_type == AUD_TYPE_RING)||(new_type == AUD_TYPE_RING))
            {
                status = ERR_AUD_CONFLICT;
            }
            break;

        case AUD_TYPE_RECORD:
            if ((new_type == AUD_TYPE_VOICE) || (new_type == AUD_TYPE_DTMF_TONE))
            {
                status = ERR_AUD_CONFLICT;
            }
        	break;

        default:
        break;
    }

    if (status == ERR_AUD_NONE)
    {
        // Update new aud type, including ring and record
        s_cur_aud_type = new_type;

        if((new_type == AUD_TYPE_RING)||(new_type == AUD_TYPE_GENERIC_TONE))
        {
            is_ring_playing = SCI_TRUE;
            s_cur_ring_type = ring_type;
        }
        else if (new_type == AUD_TYPE_VOICE)
        {
            // Set vb sample rate to avoid wrong vb clk setting
            CHIP_SetVbSampleRate (8000);

			is_voice_codec_enable = SCI_TRUE;
        }
        else if (new_type == AUD_TYPE_RECORD)
        {
            // Set vb sample rate to avoid wrong vb clk setting
            CHIP_SetVbSampleRate (8000);

        	// Voice prior to record
        	if (is_voice_codec_enable == SCI_TRUE)
        	{
        		s_cur_aud_type = AUD_TYPE_VOICE;
        	}

            is_voice_record_enable = SCI_TRUE;
        }
		
		//del for pop issue 20051215
        //AUD_ControlAmplifier(s_cur_dev_mode, s_cur_aud_type);
        
        //added for pop issue 20051215
        if(s_cur_dev_mode == AUD_DEV_MODE_EARPHONE)
        {
            VB_SetRestrainSwitchNoise(SCI_TRUE);
        }
        else
        {
            VB_SetRestrainSwitchNoise(SCI_FALSE);
        }
		//added end 
		
        AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][new_type]);
        //GPIO_EnableAmplifier(TRUE);  //turn on Power Amplifier
        SCI_AUD_EnableDeepSleep(DISABLE_DEEP_SLEEP);

        //VB_OpenVBOLDO();
    }

  //  SCI_RestoreIRQ();

    return status;
}

/*****************************************************************************/
//  Description:    This function is used to set some flag after stop audio.
//  Author:         Xueliang.Wang
//	Note:
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_CUS_ExitPlay
	(
    AUD_TYPE_E      exit_type
    )
{
    ERR_AUD_E  status = ERR_AUD_NONE;


    AUDIO_PRINT("AUD_CUS_2ExitPlay: cur_type = %d, exit_type = %d !", s_cur_aud_type, exit_type);

   // SCI_DisableIRQ();

    if (exit_type == s_cur_aud_type)
    {
	    switch (s_cur_aud_type)
	    {
	        case AUD_TYPE_CUSTOM_TONE:
	        case AUD_TYPE_DTMF_TONE:
	        case AUD_TYPE_GENERIC_TONE:
	        	if (is_voice_record_enable)
	        	{
	                s_cur_aud_type = AUD_TYPE_RECORD;
	                AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
	        	}
	        	else if (is_voice_codec_enable)
	            {
	                s_cur_aud_type = AUD_TYPE_VOICE;
	                AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
	            }
	            else
	            {
	                s_cur_aud_type = AUD_TYPE_MAX;
	            }
	        break;

	        case AUD_TYPE_RING:
		        is_ring_playing = SCI_FALSE;
		        s_cur_ring_type = AUD_RING_TYPE_MAX;
		        s_cur_aud_type  = AUD_TYPE_MAX;
	    		break;

	        case AUD_TYPE_VOICE:
	            is_voice_codec_enable   = SCI_FALSE;

	        	if (is_voice_record_enable)
	        	{
	                s_cur_aud_type = AUD_TYPE_RECORD;
	                AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
	        	}
	        	else
	        	{
		            s_cur_aud_type = AUD_TYPE_MAX;
	        	}

	        break;

	        case AUD_TYPE_RECORD:
				is_voice_record_enable   = SCI_FALSE;

	        	if (is_voice_codec_enable)
	        	{
	                s_cur_aud_type = AUD_TYPE_VOICE;
	                AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
	        	}
	        	else
	        	{
		            s_cur_aud_type = AUD_TYPE_MAX;
	        	}
	        break;

	        default:
			    AUDIO_PRINT("AUD_CUS_ExitPlay: wrong exit aud type");

	        break;
	    }
	}
	else	//exit_type != s_cur_aud_type
	{
		if ((s_cur_aud_type == AUD_TYPE_VOICE) && (exit_type == AUD_TYPE_RECORD))
		{
			is_voice_record_enable   = SCI_FALSE;

        	if (is_voice_codec_enable)
        	{
                s_cur_aud_type = AUD_TYPE_VOICE;
                AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
        	}
        	else
        	{
	            s_cur_aud_type = AUD_TYPE_MAX;
        	}
		}
		else
		{
			AUDIO_PRINT("AUD_CUS_ExitPlay: wrong exit aud type");
		}
	}

	//del for pop issue 20051215
    //AUD_ControlAmplifier(s_cur_dev_mode, s_cur_aud_type);

    // Enable deepsleep
    if (s_cur_aud_type == AUD_TYPE_MAX)
    {
    	GPIO_EnableAmplifier(FALSE); //turn off Power Amplifier
    	//del for pop issue 20051215
	    //VB_ARM_DA_ShutDown();
	    //VB_CloseVBOLDO();
		//added for pop issue 20051215
       //if(exit_type != AUD_TYPE_RING)
        {
            VB_Hold_by_ARM(16000);
        }
        VB_ShutDownSlowly(VB_GetRestrainSwitchNoise());
        //added end 
        SCI_AUD_EnableDeepSleep(ENABLE_DEEP_SLEEP);
    }

	//SCI_RestoreIRQ();

    return status;
}



/*****************************************************************************/
//  Description:    This function is used to set voice codec enable/disable.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_EnableVoiceCodec(  // If succeed, return ERR_AUD_NONE,
                                    	// else return error value
    BOOLEAN is_enable               	// SCI_TRUE: Enable,   SCI_FALSE: Disable
    )
{
    ERR_AUD_E  status;
    
    AUDIO_PRINT("AUD_EnableVoiceCodec: is_enable = %d", is_enable);

   // SCI_DisableIRQ();
    if (is_enable)
    {
        status = AUD_CUS_PreparePlay(AUD_TYPE_VOICE, 0);
        VB_Hold_by_DSP(8000);//added for pop issue 20051215
        //modify for it can't control the volume
		AUD_SetDevMode(s_cur_dev_mode);
		AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
        if (status == ERR_AUD_NONE)
        {            
            is_codec_enable = SCI_TRUE;
        }
        else
        {
            SCI_ASSERT(status == ERR_AUD_NONE);
        }
    }
    else
    {
        if (is_codec_enable)
        {
            is_codec_enable = SCI_FALSE;
            AUD_CUS_ExitPlay(AUD_TYPE_VOICE);
        }
    }
    LAYER1_SetCodecEnable(is_enable);

    return ERR_AUD_NONE;    
}

/*****************************************************************************/
//  Description:    This function is used to enable/disable mute.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_EnableMute(    // If succeed, return ERR_AUD_NONE,
                                    // else return error value
    BOOLEAN is_enable               // SCI_TRUE: Mute,      SCI_FALSE: Don't mute
    )
{
    AUDIO_PRINT("AUD_EnableMute: is_enable = %d", is_enable);
    
    LAYER1_MuteSwitch(is_enable,0);
    
    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function is used to enable/disable downlink mute.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_EnableDownlinkMute(// If succeed, return ERR_AUD_NONE,
                                        // else return error value
    BOOLEAN is_enable   // SCI_TRUE: Mute,      SCI_FALSE: Don't mute
    )
{
    AUDIO_PRINT("AUD_EnableDownlinkMute: is_enable = %d", is_enable);
    
    LAYER1_DownlinkMuteEnable(is_enable,0);
    
    return ERR_AUD_NONE;
}
    
/*****************************************************************************/
//  Description:    This function is used to set engineering audio parameters.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_SetEngineeringParameter(  	// If succeed, return ERR_AUD_NONE,
                                            	// else return error value
    AUD_DEV_MODE_E  		parameter_type,
    AUDIO_TEST_DATA_T       parameter_data
    )
{
    AUDIO_PRINT("AUD_SetEngineeringParameter: parameter_type = %d", parameter_type);
    
    LAYER1_SetAudioEngineeringParameter(parameter_type, parameter_data);
    
    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function switchs layer1 mode to test mode.
//  Author:         fancier.fan
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_SwitchTestMode(// If succeed, return ERR_AUD_NONE,
                                    // else return error value
	BOOLEAN is_test_mode
	)
{
    AUDIO_PRINT("AUD_SwitchTestMode: is_test_mode = %d", is_test_mode); 
    
    LAYER1_SwitchTestMode(is_test_mode);
    
    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function is used to enable/disable volice loopback 
//                  test.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_EnableVoiceLoopback(   // If succeed, return ERR_AUD_NONE,
                                            // else return error value
    BOOLEAN is_enable,      // 0: disable  1:enable
    uint16  delay_time      // millisecones
    )
{
    ERR_AUD_E status;
    
    AUDIO_PRINT("AUD_EnableVoiceLoopback: is_enable = %d, delay_time = %d", is_enable, delay_time);
    
   // SCI_DisableIRQ();
    if (is_enable)
    {
        status = AUD_CUS_PreparePlay(AUD_TYPE_VOICE, 0);
		VB_Hold_by_DSP(8000);//added for pop issue 20051215
        AUD_SetDevMode(s_cur_dev_mode);
        if (status == ERR_AUD_NONE)
        {            
            is_codec_enable = SCI_TRUE;
        }
        else
        {
            SCI_ASSERT(status == ERR_AUD_NONE);
        }
    }
    else
    {
        if (is_codec_enable)
        {
            is_codec_enable = SCI_FALSE;
            AUD_CUS_ExitPlay(AUD_TYPE_VOICE);
        }
    }
 
    LAYER1_EnableVoiceLoopback(is_enable, delay_time);
   // SCI_RestoreIRQ();

    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function is used to enable/disable echo test.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_EnableEchoTest(    // If succeed, return ERR_AUD_NONE,
                                        // else return error value
    BOOLEAN is_enable,                  // 0: disable  1:enable
    uint16	freq,                       // Test freq. [0, 3000]Hz
    AUD_ECHOCALLBACK_PFUNC echo_callback    // This function will be called
    	                                    // after finish echo testing.
    )
{
    ERR_AUD_E status;
    
    AUDIO_PRINT("AUD_EnableEchoTest: is_enable = %d, freq = %d, echo_callback = 0x%x", is_enable, freq, echo_callback);
    
    SCI_ASSERT(echo_callback != PNULL);

   // SCI_DisableIRQ();
    if (is_enable)
    {
        status = AUD_CUS_PreparePlay(AUD_TYPE_VOICE, 0);
        VB_Hold_by_DSP(8000);//added for pop issue 20051215
        AUD_SetDevMode(s_cur_dev_mode);
        if (status == ERR_AUD_NONE)
        {            
            is_codec_enable = SCI_TRUE;
        }
        else
        {
            SCI_ASSERT(status == ERR_AUD_NONE);
        }
    }
    else
    {
        if (is_codec_enable)
        {
            is_codec_enable = SCI_FALSE;
            AUD_CUS_ExitPlay(AUD_TYPE_VOICE);
        }
    }
   
    LAYER1_EnableEchoTest(is_enable, freq, echo_callback);
    
   // SCI_RestoreIRQ();
    
    return ERR_AUD_NONE;
}
    
/*****************************************************************************/
//  Description:    This function plays customized tone.
//  Author:         Xueliang.Wang
//	Note:           This function does nothing if it is called when playing
//                  GenericTone or Ring.
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_PlayCustomTone(   	// If succeed, return ERR_AUD_NONE,
                                    	// else return error value
    uint32  freq1,      // Freq to be played. 0 or 250 <= freq1 <= 3400Hz
    uint32  freq2,      // Freq to be played. 0 or 250 <= freq2 <= 3400Hz
    uint32  freq3,      // Freq to be played. 0 or 250 <= freq3 <= 3400Hz
    uint32  duration    // Number of milliseconds of playing this tone.
    )
{
    ERR_AUD_E  status;
    
    AUDIO_PRINT("AUD_PlayCustomTone: freq1 = %d,  freq2 = %d, freq3 = %d, duration = %d", freq1, freq2, freq3, duration);

    // Check input parameters.
    if (0 == duration)
    {
        SCI_ASSERT(duration != 0);
        return ERR_AUD_PARAM;
    }
    
   // SCI_DisableIRQ();    
    status = AUD_CUS_PreparePlay(AUD_TYPE_CUSTOM_TONE, 0);
	//VB_Hold_by_DSP(8000);//added for pop issue 20051215
    if (status  == ERR_AUD_NONE)
    {
		VB_Hold_by_DSP(8000);//added for pop issue 20051215
		AUD_SetDevMode(s_cur_dev_mode);
		AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
    
        is_custom_tone_playing = SCI_TRUE;
        
        LAYER1_PlayGenericTone(freq1, freq2, freq3);        
        AUD_SetTimer(&custom_tone_duration_timer_ptr, AudStopCustomTone, duration);
    }
  //  SCI_RestoreIRQ();
    
    return status;
}

/*****************************************************************************/
//  Description:    This function stops playing custom tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_StopCustomTone(void)  	// If succeed, return ERR_AUD_NONE,
                                        	// else return error value
{
    AUDIO_PRINT("AUD_StopCustomTone: is_custom_tone_playing = %d !", is_custom_tone_playing);

    AudStopCustomTone(0);

    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function stops playing custom tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopCustomTone(uint32 param)
{
 //   SCI_DisableIRQ();

    if (is_custom_tone_playing)
    {
        is_custom_tone_playing = SCI_FALSE;
                
        AUD_StopTimer(custom_tone_duration_timer_ptr);
        LAYER1_StopTone();    
        AUD_CUS_ExitPlay(AUD_TYPE_CUSTOM_TONE);
    }
        
  //  SCI_RestoreIRQ();    
}

/*****************************************************************************/
//  Description:    This function plays DTMF tone.
//  Author:         Xueliang.Wang
//	Note:           This function does nothing if it is called when playing
//                  GenericTone or Ring.
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_PlayDtmfTone(	// If succeed, return ERR_AUD_NONE,
                                    // else return error value
    AUD_DTMF_TONE_ID_E  tone_id,  	// Dtmf tone enumeration value.
    uint32  			duration,   // Number of milliseconds of playing this tone.
    BOOLEAN 			is_uplink   // If uplink this tone.(SCI_TRUE: Uplink)
    )
{
    ERR_AUD_E  status;
    
    AUDIO_PRINT("AUD_PlayDtmfTone: tone_id = %d,  duration = %d, is_uplink = %d", tone_id, duration, is_uplink);
    
    // Check input parameters.
    if (0 == duration)
    {
        SCI_ASSERT(duration != 0);
        return ERR_AUD_PARAM;
    }
    
    // Change temporarily because of Layer1 and DSP is not OK !
    // It should be deleted later.
    is_uplink = SCI_FALSE;
    
    //SCI_DisableIRQ();
    status = AUD_CUS_PreparePlay(AUD_TYPE_DTMF_TONE, 0);
    //VB_Hold_by_DSP(8000);//added for pop issue 20051215
    if (status  == ERR_AUD_NONE)
    {
		VB_Hold_by_DSP(8000);//added for pop issue 20051215
    	AUD_SetDevMode(s_cur_dev_mode);
		AUD_CUS_SetVolume(s_cur_dev_mode, AUD_TYPE_DTMF_TONE, s_aud_volume_table[s_cur_dev_mode][AUD_TYPE_DTMF_TONE]);
        is_dtmf_tone_playing = SCI_TRUE;
        
        LAYER1_PlaySpecialTone(tone_id, is_uplink);        
        AUD_SetTimer(&dtmf_tone_duration_timer_ptr, AudStopDtmfTone, duration);
    }
   // SCI_RestoreIRQ();
    
    return status;
} 
/*****************************************************************************/
//  Description:    This function stops playing DTMF tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_StopDtmfTone(void) // If succeed, return ERR_AUD_NONE,
                                        // else return error value
{
    AUDIO_PRINT("AUD_StopDtmfTone: is_dtmf_tone_playing = %d !", is_dtmf_tone_playing);

    AudStopDtmfTone(0);

    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function stops playing DTMF tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopDtmfTone(uint32 param)
{
	AUDIO_PRINT("AudStopDtmfTone");
 //   SCI_DisableIRQ();


    if (is_dtmf_tone_playing)
    {
        is_dtmf_tone_playing = SCI_FALSE;
                
        AUD_StopTimer(dtmf_tone_duration_timer_ptr);
        LAYER1_StopTone();    
        AUD_CUS_ExitPlay(AUD_TYPE_DTMF_TONE);       
    }
        
 //   SCI_RestoreIRQ();    
}

/*****************************************************************************/
//  Description:    This function plays generic tone.
//  Author:         Xueliang.Wang
//	Note:           This function does nothing if it is called when playing
//                  Ring.
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_PlayGenericTone(   // If succeed, return ERR_AUD_NONE,
                                        // else return error value
    AUD_GENERIC_TONE_ID_E tone_id,      // Generic tone enumeration value.
    uint32  duration    // Number of milliseconds of playing this tone.
    )
{
    ERR_AUD_E	status;
    
    AUDIO_PRINT("AUD_PlayGenericTone: tone_id = %d,  duration = %d", tone_id, duration);

    // Check input parameters.
    if ((tone_id >= AUD_GENERIC_TONE_ID_MAX) || (0 == duration))
    {
        AUDIO_PRINT("AUD_PlayGenericTone: tone_id or duration is invalid.");
        AUDIO_ASSERT(tone_id < AUD_GENERIC_TONE_ID_MAX);
        return ERR_AUD_PARAM;
    }
    
    if (g_generic_tone_info[tone_id].type != AUD_SINGLE_TONE_RING)
    {
        AUDIO_PRINT("AUD_PlayGenericTone: GenericTone data is invalid.");
        SCI_ASSERT(0);
        return ERR_AUD_PARAM;
    }

   // SCI_DisableIRQ();
    status = AUD_CUS_PreparePlay(AUD_TYPE_GENERIC_TONE, 0);
    //VB_Hold_by_DSP(8000);//added for pop issue 20051215
    if (status  == ERR_AUD_NONE)
    {
		VB_Hold_by_DSP(8000);//added for pop issue 20051215
		AUD_SetDevMode(s_cur_dev_mode);
		AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
    
        is_generic_tone_playing = SCI_TRUE;
        
        generic_tone_play_info.type      = g_generic_tone_info[tone_id].type;
        generic_tone_play_info.tone_num  = g_generic_tone_info[tone_id].data_len/sizeof(AUD_SINGLE_TONE_T);
        generic_tone_play_info.tone_ptr  = g_generic_tone_info[tone_id].data_ptr;
        generic_tone_play_info.offset    = 0;
        generic_tone_play_info.play_times= AUD_PLAY_FOREVER;
        
        AudPlayGenericTone(0);
        
        AUD_SetTimer(&generic_tone_duration_timer_ptr, AudStopGenericTone, duration);
    }
    //SCI_RestoreIRQ();

    return status;    
}

/*****************************************************************************/
//  Description:    This function is used to get handle tone timer expired.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudPlayGenericTone(uint32 param)
{
    uint16  offset;
    uint16  play_freq1;
    uint16  play_duration;

//    SCI_DisableIRQ();
    
    if (generic_tone_play_info.offset == generic_tone_play_info.tone_num)
    {
        generic_tone_play_info.offset = 0;
    }    
    offset  = generic_tone_play_info.offset;
    generic_tone_play_info.offset ++; 
    
    if (generic_tone_play_info.type == AUD_SINGLE_TONE_RING)
    {
        AUD_SINGLE_TONE_T *single_tone_ptr = (AUD_SINGLE_TONE_T *)(generic_tone_play_info.tone_ptr + 4 + offset*sizeof(AUD_SINGLE_TONE_T));
        play_freq1      = single_tone_ptr->freq;
        play_duration   = single_tone_ptr->duration;
    }
    else
    {
        SCI_ASSERT(0);
    }
    
    LAYER1_PlayGenericTone(play_freq1, 0, 0);    
    AUD_SetTimer(&generic_tone_timer_ptr, AudPlayGenericTone, play_duration);
    
 //   SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    This function stops playing generic tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_StopGenericTone(void)  // If succeed, return ERR_AUD_NONE,
                                            // else return error value
{
    AUDIO_PRINT("AUD_StopGenericTone: is_generic_tone_playing = %d !", is_generic_tone_playing);

    AudStopGenericTone(0);

    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function stops playing generic tone.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudStopGenericTone(uint32 param)
{
    AUDIO_PRINT("AudStopGenericTone: is_generic_tone_playing = %d !", is_generic_tone_playing);

  //  SCI_DisableIRQ();
    if (is_generic_tone_playing)
    {
        is_generic_tone_playing = SCI_FALSE;
        
        AUD_StopTimer(generic_tone_duration_timer_ptr);
        AUD_StopTimer(generic_tone_timer_ptr);
        LAYER1_StopTone();
        
        SCI_MEMSET(&generic_tone_play_info, 0, sizeof(AUD_PLAY_INFO_T));
        
        AUD_CUS_ExitPlay(AUD_TYPE_GENERIC_TONE);
    }
  //  SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    This function plays ring.
//  Author:         Xueliang.Wang
//	Note:
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_PlayRing(  // If succeed, return ERR_AUD_NONE,
                                // else return error value
    AUD_RING_TYPE_E ring_type,  // Ring type enum.
    uint32  ring_data_len,      // Number of bytes of the ring data.
    const uint8 *ring_data_ptr, // Pointer to ring data.
    uint32  play_times,         // Number of times of playing this ring.
    AUD_RING_CALLBACK_PFUNC callback_pfunc  // Ring callback function, it will
                                            // be called when ring is finished.
    )
{
    ERR_AUD_E  status = ERR_AUD_NONE;

    AUDIO_PRINT("AUD_PlayRing: ring_type = %d, ring_data_len = %d, ring_data_ptr = 0x%p, play_times = %d, callback_pfunc = %d!", \
        ring_type, ring_data_len, ring_data_ptr, play_times, callback_pfunc);

    // Check input parameters.
    if ((ring_data_ptr == SCI_NULL) || (play_times == 0))
    {
        return ERR_AUD_PARAM;
    }

    if (ring_type == AUD_MIDI_RING)
    {
        status = MIDI_Play(MIDI_MID, ring_data_len, ring_data_ptr, play_times, callback_pfunc);
    }
    else if (ring_type == AUD_SMAF_RING)
    {
        status = MIDI_Play(MIDI_MMF, ring_data_len, ring_data_ptr, play_times, callback_pfunc);
    }
    else if (ring_type == AUD_SINGLE_TONE_RING)
    {
        status = AUD_PlayNormalRing(ring_type, ring_data_len, ring_data_ptr, play_times, callback_pfunc);
    }
    else
    {
	    status = ERR_AUD_NONE;
        SCI_ASSERT(0);
    }

    return status;
}

/*****************************************************************************/
//  Description:    This function plays normal ring.
//                  Ring data type:
//                          AUD_SINGLE_TONE_RING
//                          AUD_DUAL_TONE_RING
//                          AUD_TRIPLE_TONE_RING
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_PlayNormalRing(// If succeed, return ERR_AUD_NONE,
                                    // else return error value
    AUD_RING_TYPE_E ring_type,  // Ring type enum.
    uint32  ring_data_len,      // Number of bytes of the ring data.
    const uint8 *ring_data_ptr, // Pointer to ring data.
    uint32  play_times,         // Number of times of playing this ring.
    AUD_RING_CALLBACK_PFUNC callback_pfunc  // Ring callback function, it will 
                                            // be called when ring is finished.
    )
{
    ERR_AUD_E status;
    
    AUDIO_PRINT("AUD_PlayNormalRing: ring_type = %d, ring_data_len = %d, ring_data_ptr = 0x%p, play_times = %d!", \
        ring_type, ring_data_len, ring_data_ptr, play_times);

    if (*(uint16 *)ring_data_ptr != ring_type)
    {
        AUDIO_PRINT("AUD_PlayNormalRing: Ring data is invalid.");
        SCI_ASSERT(0);
        return ERR_AUD_PARAM;
    }

   // SCI_DisableIRQ();  //piaotq delete disable irq num
    
    status = AUD_CUS_PreparePlay(AUD_TYPE_GENERIC_TONE, AUD_SINGLE_TONE_RING);
    
    if (status  == ERR_AUD_NONE)
    {
       	VB_Hold_by_DSP(8000);//added for pop issue 20051215
    	AUD_SetDevMode(s_cur_dev_mode);
  		AUD_CUS_SetVolume(s_cur_dev_mode, s_cur_aud_type, s_aud_volume_table[s_cur_dev_mode][s_cur_aud_type]);
    	
        is_generic_tone_playing = SCI_TRUE;
        
	    generic_tone_play_info.type       = ring_type;    
	    generic_tone_play_info.tone_ptr   = ring_data_ptr;
	    generic_tone_play_info.offset     = 0;
	    generic_tone_play_info.play_times = play_times;
	    if (ring_type == AUD_SINGLE_TONE_RING)
	    {
	        generic_tone_play_info.tone_num  = (ring_data_len - 4)/sizeof(AUD_SINGLE_TONE_T);
	    }
	    else if (ring_type == AUD_DUAL_TONE_RING)
	    {
	        generic_tone_play_info.tone_num  = (ring_data_len - 4)/sizeof(AUD_DUAL_TONE_T);
	    }
	    else if (ring_type == AUD_TRIPLE_TONE_RING)
	    {
	        generic_tone_play_info.tone_num  = (ring_data_len - 4)/sizeof(AUD_TRIPLE_TONE_T);
	    }
	    else
	    {
	        AUDIO_PRINT("AUD_PlayNormalRing: Ring data is invalid.");
	        SCI_ASSERT(0);
	        return ERR_AUD_PARAM;
	    } 
	    
        AudPlayNormalRing(0);
    }
   // SCI_RestoreIRQ();//piaotq delete disable irq num

    return status;    
}

/*****************************************************************************/
//  Description:    This function is used to get handle tone timer expired.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL void AudPlayNormalRing(uint32 param)
{
    uint16  offset;
    uint16  play_freq1;
    uint16  play_freq2;
    uint16  play_freq3;
    uint16  play_duration;

    
    AUDIO_PRINT("AudPlayNormalRing");	
	//disable irq after trace 
 //   SCI_DisableIRQ();
    
    if (generic_tone_play_info.offset == generic_tone_play_info.tone_num)
    {
        if (generic_tone_play_info.play_times > 0)
        {
            // Change playing param and start to play next times.
            if (AUD_TYPE_GENERIC_TONE == s_cur_aud_type)
            {
                if (AUD_PLAY_FOREVER != generic_tone_play_info.play_times)
                {
                    generic_tone_play_info.play_times --;
                }
            }
            generic_tone_play_info.offset = 0;
        }
        else
        {
            // Playing is finished.
            is_ring_playing = SCI_FALSE;
            AUD_StopNormalRing();                
            AUD_RingFinished();
          //  SCI_RestoreIRQ();//piaotq add disable irq 
            return;
        }
    }

    offset  = generic_tone_play_info.offset;
    generic_tone_play_info.offset ++; 

    if (generic_tone_play_info.type == AUD_SINGLE_TONE_RING)
    {
        AUD_SINGLE_TONE_T *single_tone_ptr = (AUD_SINGLE_TONE_T *)(generic_tone_play_info.tone_ptr + 4 + offset*sizeof(AUD_SINGLE_TONE_T));
        play_freq1      = single_tone_ptr->freq;
        play_freq2      = 0;
        play_freq3      = 0;
        play_duration   = single_tone_ptr->duration;
    }
    else if (generic_tone_play_info.type == AUD_DUAL_TONE_RING)
    {
        AUD_DUAL_TONE_T *dual_tone_ptr = (AUD_DUAL_TONE_T *)(generic_tone_play_info.tone_ptr + 4 + offset*sizeof(AUD_DUAL_TONE_T));
        play_freq1      = dual_tone_ptr->freq1;
        play_freq2      = dual_tone_ptr->freq2;
        play_freq3      = 0;
        play_duration   = dual_tone_ptr->duration;
    }

    else if (generic_tone_play_info.type == AUD_TRIPLE_TONE_RING)
    {
        AUD_TRIPLE_TONE_T *triple_tone_ptr = (AUD_TRIPLE_TONE_T *)(generic_tone_play_info.tone_ptr + 4 + offset*sizeof(AUD_TRIPLE_TONE_T));
        play_freq1      = triple_tone_ptr->freq1;
        play_freq2      = triple_tone_ptr->freq2;
        play_freq3      = triple_tone_ptr->freq3;
        play_duration   = triple_tone_ptr->duration;
    }
    else
    {
        SCI_ASSERT(0);
    }
    
    LAYER1_PlayGenericTone(play_freq1, play_freq2, play_freq3);
    AUD_SetTimer(&generic_tone_timer_ptr, AudPlayNormalRing, play_duration);

 //   SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    This function stops playing ring.
//  Author:         Xueliang.Wang
//	Note:
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_StopRing(void) // If succeed, return ERR_AUD_NONE,
                                    // else return error value
{
    AUD_RING_TYPE_E ring_type;
    
    AUDIO_PRINT("AUD_StopRing");
  //  SCI_DisableIRQ();
    if((is_ring_playing)||(is_generic_tone_playing))
    {
       
        ring_type = AUD_GetRingType();
        
        is_ring_playing = SCI_FALSE;
        if ((ring_type == AUD_MIDI_RING) || (ring_type == AUD_SMAF_RING))
        {
		    MIDI_Stop();    // It will call AUD_RingFinished()
		    //AUD_CUS_ExitPlay(AUD_TYPE_RING);//close vb ldo to avoid audio noise
        }
        else if ((ring_type == AUD_SINGLE_TONE_RING) || (ring_type == AUD_DUAL_TONE_RING)
              || (ring_type == AUD_TRIPLE_TONE_RING))
        {
	        AUD_StopNormalRing();
	        AUD_RingFinished();
        }
        else    
        {
            AUDIO_PRINT("AUD_StopRing: last ring type =  %d !!!", ring_type);
            
            SCI_ASSERT(0);
        }
        
    }    
   // SCI_RestoreIRQ();

    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function gets ring type plaied currently.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC AUD_RING_TYPE_E AUD_GetRingType(void)
{
    //AUDIO_PRINT("AUD_GetRingType: s_cur_ring_type = %d !", s_cur_ring_type);//be not able add trace statement in vb_isr_handler
    //this function will 
    return s_cur_ring_type;
}

/*****************************************************************************/
//  Description:    This function stops playing normal ring.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC ERR_AUD_E AUD_StopNormalRing(void)  	// If succeed, return ERR_AUD_NONE,
                                        	// else return error value
{
	AUDIO_PRINT("AUD_StopNormalRing is_generic_tone_playing is %d",is_generic_tone_playing);
   // SCI_DisableIRQ();
    if (is_generic_tone_playing)
    {
        is_generic_tone_playing = SCI_FALSE;
        
        AUD_StopTimer(generic_tone_timer_ptr);
        LAYER1_StopTone();
        
        SCI_MEMSET(&generic_tone_play_info, 0, sizeof(AUD_PLAY_INFO_T));
        
        AUD_CUS_ExitPlay(AUD_TYPE_GENERIC_TONE);
    }
   // SCI_RestoreIRQ();

    return ERR_AUD_NONE;
}

/*****************************************************************************/
//  Description:    This function stops playing ring.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC void AUD_RingFinished(void)
{
	//AUDIO_PRINT("AUD_RingFinished");
    s_cur_ring_type = AUD_RING_TYPE_MAX;
    s_cur_aud_type  = AUD_TYPE_MAX;
    //AUD_ControlAmplifier(s_cur_dev_mode, s_cur_aud_type); 
    
    // Call ring callback function.
    if (s_ring_callback_pfunc != SCI_NULL)
    {
        (*s_ring_callback_pfunc)();

    }
    GPIO_EnableAmplifier(FALSE); //turn off Power Amplifier
    SCI_AUD_EnableDeepSleep(ENABLE_DEEP_SLEEP);
}

/*****************************************************************************/
//  Description:    This function set audio timer.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC void AUD_SetTimer(
    SCI_TIMER_PTR *timer_pptr,  // Timer pointer to be set.
    TIMER_FUN func_ptr,         // Timer expired callback function.
    uint32 milsec               // Number of milliseconds before expired.
    )
{
	if (0 == milsec)
	{
		milsec = 1;
		AUDIO_PRINT("AUD_SetTimer: Time value to be set is 0. Timer_Ptr = %p !!!", *timer_pptr);
	}
    
    SCI_DisableIRQ();
    if (SCI_NULL == *timer_pptr)
    {
    	if (AUD_PLAY_FOREVER != milsec)
        {
            *timer_pptr = SCI_CreateTimer("AUDIO Timer", func_ptr, 0, milsec, SCI_AUTO_ACTIVATE);
        }
    }
    else
    {
        if (SCI_IsTimerActive(*timer_pptr))
        {
            SCI_DeactiveTimer(*timer_pptr);
        }

        if (AUD_PLAY_FOREVER != milsec)
        {
            SCI_ChangeTimer(*timer_pptr, func_ptr, milsec);
            SCI_ActiveTimer(*timer_pptr);
        }        
    }
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    This function stops audio timer.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC void AUD_StopTimer(
    SCI_TIMER_PTR timer_ptr     // Timer pointer to be stopped.
    )
{
    SCI_DisableIRQ();
    if (SCI_NULL != timer_ptr)
    {
        if (SCI_IsTimerActive(timer_ptr))
        {
            SCI_DeactiveTimer(timer_ptr);
        }
    }
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    This function set VB ioctl before playring. It should
//					be called before play new audio.
//  Author:         Jimmy.Jia
//	Note:           
/*****************************************************************************/
PUBLIC VB_IOCTL_T *AUD_GetRingVBCtl (void)
{
	return (VB_IOCTL_T *)&(s_vb_ring_cb[s_cur_dev_mode]);
}

/*****************************************************************************/
//  Description:    This function set VB ioctl before playring. It should
//					be called before play new audio.
//  Author:         Jimmy.Jia
//	Note:           
/*****************************************************************************/
PUBLIC VB_IOCTL_T *AUD_GetVoiceVBCtl (void)
{
	return (VB_IOCTL_T *)&(s_vb_voice_cb[s_cur_dev_mode]);	
}

/*****************************************************************************/
//  Description:    This function is used to set EQ parameter.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/

PUBLIC void AUD_SetEqParameter(AUD_EQ_TYPE_E eq_type)
{
    LAYER1_SetEqParameter(eq_type);
}

/*****************************************************************************/
//  Description:    This function is used to get engineering audio parameters.
//  Author:         Sunsome.Ju
//	Note:           
/*****************************************************************************/

PUBLIC ERR_AUD_E AUD_GetEngineeringParameter(  	// If succeed, return ERR_AUD_NONE,
                                            	// else return error value
    AUD_DEV_MODE_E  		parameter_type,
    AUDIO_TEST_DATA_T       *parameter_data
    )
{
    AUDIO_PRINT("AUD_GetEngineeringParameter: parameter_type = %d", parameter_type);
    
    LAYER1_GetAudioEngineeringParameter(parameter_type, parameter_data);
    
    return ERR_AUD_NONE;
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif
