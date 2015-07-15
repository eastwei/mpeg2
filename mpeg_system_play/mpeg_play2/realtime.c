/*************************************************************************
*  File Name:     realtime.c
*  Creation Date: 12 August 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*  Usage:         define and declare for realtime.c
*
*  Description:   This module contains the functions needed for
*  maintaining the system state based on the current realtime
*  state of the system.  Self-calibration, initialization functions,
*  and helpful print functions are also provided.
**************************************************************************/

/**************************************************************************
* Copyright (c) 1994 The Multimedia Communications Lab, Boston University.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement is
* hereby granted, provided that the above copyright notice and the following
* two paragraphs appear in all copies of this software.
*
* IN NO EVENT SHALL BOSTON UNIVERSITY BE LIABLE TO ANY PARTY FOR
* DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
* OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BOSTON
* UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* BOSTON UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
* ON AN "AS IS" BASIS, AND BOSTON UNIVERSITY HAS NO OBLIGATION TO
* PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*
* In addition, some or all of this software has been copied or modified
* from software produced at the University of California, to which the
* following applies:
*
* Copyright (c) 1994 The Regents of the University of California.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software and its
* documentation for any purpose, without fee, and without written agreement is
* hereby granted, provided that the above copyright notice and the following
* two paragraphs appear in all copies of this software.
*
* IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
* DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
* OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
* CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
* AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
* ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
* PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
**************************************************************************/

#include <stdio.h>
#include "realtime.h"
/*************************************************************************
*  Function:      init()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        Global realtime state rt_info (as *rt)
*
*  Effects:       Global rt_info (as *rt)
*
*
*  Description:   Initializes the state with default variables. The                                    
*      default variables used are based on the values arrived at if
*       a stream with 30 frames per second and a frame pattern of       
*      IBBPBBPBBI has been processed.                                                                      
*===========================================================================*/


void init(volatile RT_info *rt)
{
  rt->I_frames_seen = 0;       
  rt->P_frames_seen = 0;
  rt->B_frames_seen = 0;
  rt->I_frames_played = 0;    
  rt->P_frames_played = 0;
  rt->B_frames_played = 0;
  rt->I_frames_skipped = 0;
  rt->P_frames_skipped = 0;
  rt->B_frames_skipped = 0;
  rt->frames_per_second = (float)30.0;
  rt->time_increment = (float)1.0;
  rt->delay = 0;
  rt->delay_max = 1000;       /*estimate updated after counting phase*/
  rt->phase = B_PHASE;        /*start out playing all frames*/
  rt->mode = COUNTING;        
  rt->pause = PLAY;
  rt->B_play_max = 18;
  rt->B_skip_min = 0;
  rt->B_play_min = 1;
  rt->B_skip_max = 17;
  rt->P_play_max = 6;
  rt->P_skip_min = 0;
  rt->P_play_min = 1;
  rt->P_skip_max = 5;
  rt->I_play_max = 6;
  rt->I_skip_min = 0;
  rt->I_play_min = 1;
  rt->I_skip_max = 5;  
  rt->Degraded_skip_max = 60; /* 10 seconds worth of I frames */
  rt->play = 18;
  rt->skip = 0; 
  rt->calibration_count = 0;  /*updated during counting phase*/
  rt->max_up_swing = 15;      /* initially loose damping to quicken arrival*/
  rt->max_down_swing = 15;    /* at steady state */
  rt->frame_bias = 0;         /* frame bais is used to make minor adjustments*/
  rt->clock_90khz = 0;        /* needed to prevent major drifts from real-time*/
  rt->clock_frame = 0;
  rt->frame_increment = 3000;    /* 90000 ticks per second/ 30 frames per second*/
}

/*************************************************************************
*  Function:      setup()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       global realtime state rt_info (as *rt)
*                workload estimator based on image size and frame rate
*
*  Effects:      damping factors set, frame time values set,and a start
*                is chosen based on the workload estimator.
*
*
*  Description:   Adjusts state variables after Sequence header read                                   
*      Workload is an estimator used to set the initial phase         
*      Swings are set wide initially for rapid gains prior                                                                     
*      to calibration.
**************************************************************************/


void setup(volatile RT_info *rt,int workload)
{
  rt->max_up_swing =(int) (rt->frames_per_second/(float)2.0);
  rt->max_down_swing = (int)(rt->frames_per_second/(float)2.0);
  rt->frame_increment = (int)((float)RTC_SECOND/rt->frames_per_second);
  
  if(workload >= 100000){
    rt->phase = I_PHASE;
    rt->skip = rt->I_skip_min;
    rt->play = rt->I_play_max;
  } else if(workload >= 30000){
    rt->phase = P_PHASE;
    rt->skip = rt->P_skip_min;
    rt->play = rt->P_play_max;
  } else { /* B phase*/
    rt->phase = B_PHASE;
    rt->skip = rt->B_skip_min;
    rt->play = rt->B_play_max;
  } 
    

}
/*************************************************************************
*  Function:      calibrate()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:      Global realtime state rt_info (as *rt)
*
*  Effects:     Global realtime state rt_info (as *rt)
*
*
*  Description:  Adjusts system state after 1 second worth of frames seen.                                   
 *      Current state is reset to avoid finding oneself in an         
 *      illegal state after calibration.  State bounds are adjusted                                                                     
 *      to the current frame pattern as reflected in the state variables.
 *      Swings are made tighter to increase dampening.  This has the effect
 *      of smoothing the transient response.
 **************************************************************************/


void calibrate(volatile RT_info *rt)
{
  int sum;

  sum = rt->I_frames_seen + rt->P_frames_seen + rt->B_frames_seen;

  rt->B_play_max = (int)((float)rt->B_frames_seen * 
			 (rt->frames_per_second/(float)sum));
  rt->B_skip_min = 0;
  rt->B_play_min = 1;
  rt->B_skip_max = rt->B_play_max - 1;
  rt->P_play_max = (int)((float)rt->P_frames_seen * 
			 (rt->frames_per_second/(float)sum));
  rt->P_skip_min = 0;
  rt->P_play_min = 1;
  rt->P_skip_max = rt->P_play_max - 1;
  rt->I_play_max = (int)((float)rt->I_frames_seen * 
			 (rt->frames_per_second/(float)sum));
  rt->I_skip_min = 0;
  rt->I_play_min = 1;
  rt->I_skip_max = rt->I_play_max - 1; 
  
  rt->Degraded_skip_max = rt->I_play_max * 10;  /*10 seconds of I frames*/ 


  if(rt->phase == B_PHASE){          /*ensure not in unstable state*/
    if(rt->play > rt->skip){
      rt->play = rt->B_play_max;
      rt->skip = rt->B_skip_min;
    }else{
      rt->play = rt->B_play_min;
      rt->skip = rt->B_skip_max;
    }
  }else if(rt->phase == P_PHASE){
    if(rt->play > rt->skip){
      rt->play = rt->P_play_max;
      rt->skip = rt->P_skip_min;
    }else{
      rt->play = rt->P_play_min;
      rt->skip = rt->P_skip_max;
    }

  }else if(rt->phase == I_PHASE){
    if(rt->play > rt->skip){
      rt->play = rt->I_play_max;
      rt->skip = rt->I_skip_min;
    }else{
      rt->play = rt->I_play_min;
      rt->skip = rt->I_skip_max;
    }

  }else if(rt->phase == DEGRADED_PHASE){
    rt->play = 1;
    rt->skip = rt->I_play_max;
  } else{}

                /* set the maximum delay to the value corresponding to
                   instantaeous decoding of frames*/
  rt->delay_max = (int)(((float)1.0/rt->frames_per_second)*(float)rt->calibration_count);
  
  rt->max_up_swing =(int) (rt->frames_per_second/(float)4.0);
  rt->max_down_swing = (int)(rt->frames_per_second/(float)4.0);
  
  rt->mode = RUN;          /* go to normal running mode after calibaration*/
}
/*************************************************************************
*  Function:      play_or_skip()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:      Global realtime state rt_info (as *rt)
*
*  Effects:     returns a value corresponding to a PLAY or SKIP frame
*               decision.
*
*
*  Description:   
*      Decides whether to play or skip a given frame.                                   
*      It assumes that this function is only called if phase type corresponds
*      to frame type. The incrementing of frames_played and skipped is done 
*      at higher level.  The decision to play or skip must be made dynamically
*      since we may have had to skip frames we would normally
*      have played due to interframe dependencies.
**************************************************************************/

int play_or_skip(volatile RT_info *rt) 
{ 
  int decision ;
  float play_ratio,played_ratio;
 
  if(rt->skip == 0){
    return(PLAY_FRAME);
  }
   

         /* assumes skip not equal to zero */
  switch(rt->phase){
  case B_PHASE:

    play_ratio = (float)rt->play/((float)rt->skip );
    played_ratio = (float)rt->B_frames_played/
      ((float)rt->B_frames_skipped +(float)1.0);
    
    if(play_ratio <= played_ratio){
      decision = SKIP_FRAME;
    } else {
      decision = PLAY_FRAME;
    }
    break;
  case P_PHASE:   /* NOTE: interframe dependencies may force us to
		     skip a frame we would decide to play in this */ 
    
    play_ratio = (float)rt->play/((float)rt->skip );
    played_ratio = (float)rt->P_frames_played/
      ((float)rt->P_frames_skipped +(float)1.0);
    
    if(play_ratio < played_ratio){
      decision = SKIP_FRAME;
    } else {
      decision = PLAY_FRAME;
    }

    break;
  case I_PHASE:
    play_ratio = (float)rt->play/((float)rt->skip );
    played_ratio = (float)rt->I_frames_played/
      ((float)rt->I_frames_skipped +(float)1.0);
    
    if(play_ratio < played_ratio){
      decision = SKIP_FRAME;
    } else {
      decision = PLAY_FRAME;
    }
    
    break;
  case DEGRADED_PHASE:
    play_ratio = (float)rt->play/((float)rt->skip );
    played_ratio = (float)rt->I_frames_played/
      ((float)rt->I_frames_skipped +(float)1.0);
    
    if(play_ratio < played_ratio){
      decision = SKIP_FRAME;
    } else {
      decision = PLAY_FRAME;
    }
    
    break;
  default:  /*delay phase*/
    decision = PLAY_FRAME;
  }

  return(decision);
}
/*************************************************************************
*  Function:      next_B_phase()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:    Global realtime state (as *rt)
*
*  Effects:   Global realtime state (as *rt)
*
*
*  Description: Given the current phase , goto the next phase.                                
*      The decision as to which phase to go to depends on the       
*      current state, the number of frames seen since the last
*      interval, the frame bias set for realtime adjustments,
*      and the damping variable settings. It is important to
*      avoid illogical states.  These are states which use
*      frame types that aren't included in the pattern. Range
*      checking is done to ensure wide swings can be accomodated.                                                                     *
**************************************************************************/


void next_B_phase(volatile RT_info *rt)
{ 
  int frames;
  int diff,delta;
  
  frames =     rt->I_frames_played + rt->P_frames_played + 
    rt->B_frames_played +
      rt->I_frames_skipped + rt->P_frames_skipped + 
	rt->B_frames_skipped - rt->frame_bias;
  if(frames < 0) frames = 0;
  
  diff = ((int)(rt->time_increment*rt->frames_per_second)) - frames;
  
  if((rt->clock_90khz - rt->clock_frame) > 0){
    delta =((int)(rt->clock_90khz - rt->clock_frame))/rt->frame_increment;
    diff += delta;
  }
  if(diff < -rt->max_down_swing){
    diff = -rt->max_down_swing;
  }else if(diff > rt->max_up_swing){
    diff = rt->max_up_swing;
  }else{} 
  
  if(diff == 0){
    /* all is well*/
  } else if(diff < 0){    /* need to slow down*/
   
    if((rt->skip + diff) >= rt->B_skip_min){
      rt->skip +=  diff;
      rt->play -=  diff;
    } else {
      delta = rt->skip + diff - rt->B_skip_min ;
      rt->phase = DELAY_PHASE;
      rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			 ((float)1.0/(rt->frames_per_second -
				      (float)delta)))*
			(float)rt->calibration_count);
      if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
      rt->play = (int)rt->frames_per_second;
      rt->skip = 0;
   }
  } else {            /* diff > 0 - need to speed up*/
    
    if(rt->skip + diff <= rt->B_skip_max){
      rt->skip += diff;
      rt->play -= diff;
    } else {
      delta = rt->skip + diff - rt->B_skip_max - 1;
      if ((delta < (rt->P_skip_max + 1)) &&( rt->P_frames_seen != 0)){
        rt->phase = P_PHASE;
        rt->delay = 0;
        rt->skip = rt->P_skip_min + delta;
        rt->play = rt->P_play_max - delta;
      } else { 
	if(rt->P_frames_seen != 0){
	  delta -= (rt->P_skip_max + 1);
	}
        rt->phase = I_PHASE;
        rt->delay = 0;
        rt->skip = rt->I_skip_min + delta;
        rt->play = rt->I_play_max - delta;
        if(rt->play < rt->I_play_min){
          rt->play = 1;
          if(rt->skip > rt->Degraded_skip_max)
	    rt->skip = rt->Degraded_skip_max;
          rt->phase = DEGRADED_PHASE;
          rt->delay = 0;}
      }
      
    } /* end outer else*/ 
  }    /* end first if*/
  return;
}    /* end function*/

/*************************************************************************
*  Function:      next_P_phase()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       Global realtime state (as *rt)
*
*  Effects:      Global realtime state (as *rt)
*
*
*  Description: Given the current phase , goto the next phase.                                
*      The decision as to which phase to go to depends on the       
*      current state, the number of frames seen since the last
*      interval, the frame bias set for realtime adjustments,
*      and the damping variable settings. It is important to
*      avoid illogical states.  These are states which use
*      frame types that aren't included in the pattern. Range
*      checking is done to ensure wide swings can be accomodated.                                                                     *
**************************************************************************/


void next_P_phase(volatile RT_info *rt)
{
  int frames;
  int diff,delta;
  double Dampen_scale_factor;
  int bound_up,bound_down;
 
  frames = rt->I_frames_played + rt->P_frames_played + 
    rt->B_frames_played +
      rt->I_frames_skipped + rt->P_frames_skipped + 
	rt->B_frames_skipped  - rt->frame_bias;
  if(frames < 0)frames = 0;             

  diff = ((int)(rt->time_increment*rt->frames_per_second)) - frames;
  
  if((rt->clock_90khz - rt->clock_frame) > 0){
    delta =((int)(rt->clock_90khz - rt->clock_frame))/rt->frame_increment;
    diff += delta;
  }

  Dampen_scale_factor = (double)2.0; 

  bound_up = (int)((double)rt->max_up_swing/Dampen_scale_factor);
  bound_down =(int)((double)rt->max_down_swing/Dampen_scale_factor);
 
  if(diff < -bound_down){
    diff = -bound_down;
  }else if(diff > bound_up){
    diff = bound_up;
  }else{} 
 
 
  if(diff == 0){
    /* all is well*/
  } else if(diff < 0){    /* need to slow down*/
   
    if((rt->skip + diff) >= rt->P_skip_min){
      rt->skip += diff;
      rt->play -= diff;
    } else {
      delta = rt->skip + diff - rt->P_skip_min + 1;
      if((rt->B_frames_seen !=0)&&((rt->B_skip_max + delta) >= 
				   rt->B_skip_min)){
        rt->phase = B_PHASE;
        rt->delay = 0;
        rt->skip = rt->B_skip_max + delta;
        rt->play = rt->B_play_min - delta;
      } else {
	if(((rt->B_skip_max + delta) < rt->B_skip_min)&&
	   (rt->B_frames_seen  != 0)){
	  delta += (rt->B_skip_max + 1);}
       
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
      }
    }
  } else {            /* diff > 0 - need to speed up*/
    
    if(rt->skip + diff < rt->P_skip_max){
      rt->skip += diff;
      rt->play -= diff;
    } else {
      delta = rt->skip + diff - rt->P_skip_max - 1;
      rt->phase = I_PHASE;
      rt->delay = 0;
      rt->skip = rt->I_skip_min + delta;
      rt->play = rt->I_play_max - delta;
      if(rt->play < rt->I_play_min){
	rt->play = 1;
	if(rt->skip > rt->Degraded_skip_max)
	  rt->skip = rt->Degraded_skip_max;
	rt->phase = DEGRADED_PHASE;
	rt->delay = 0;}
    }  
   
  }  /* end outer else*/  
  return;
}   /* end function*/
/*************************************************************************
*  Function:      next_I_phase()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:      Global realtime state (as *rt)
*
*  Effects:     Global realtime state (as *rt)
*
*
*  Description: Given the current phase , goto the next phase.                                
*      The decision as to which phase to go to depends on the       
*      current state, the number of frames seen since the last
*      interval, the frame bias set for realtime adjustments,
*      and the damping variable settings. It is important to
*      avoid illogical states.  These are states which use
*      frame types that aren't included in the pattern. Range
*      checking is done to ensure wide swings can be accomodated.                                                                     *
**************************************************************************/


void next_I_phase(volatile RT_info *rt)
{
  int frames;
  int diff,delta;
  double Dampen_scale_factor;
  int bound_up,bound_down;
 
  frames =     rt->I_frames_played + rt->P_frames_played + 
    rt->B_frames_played +
      rt->I_frames_skipped + rt->P_frames_skipped + 
	rt->B_frames_skipped  - rt->frame_bias;
  if(frames < 0)frames = 0;             

  diff = ((int)(rt->time_increment*rt->frames_per_second)) - frames;
  
  if((rt->clock_90khz - rt->clock_frame) > 0){
    delta =((int)(rt->clock_90khz - rt->clock_frame))/rt->frame_increment;
    diff += delta;
  }

  Dampen_scale_factor = 3.0;

  bound_up = (int)((double)rt->max_up_swing/Dampen_scale_factor);
  bound_down =(int)((double)rt->max_down_swing/Dampen_scale_factor);
 
  if(diff < -bound_down){
    diff = -bound_down;
  }else if(diff > bound_up){
    diff = bound_up;
  }else{} 
 
 
  if(diff == 0){
  /* all is well*/
  } else if(diff < 0){    /* need to slow down*/
   
    if((rt->skip + diff) >= rt->I_skip_min){
      rt->skip += diff;
      rt->play -= diff;
    } else {
      delta = rt->skip + diff - rt->I_skip_min ;
      if((rt->P_frames_seen !=0)&&((rt->P_skip_max + delta + 1) >= 
				   rt->P_skip_min)){
        rt->phase = P_PHASE;
        rt->delay = 0;
        rt->skip = rt->P_skip_max + delta + 1;
        rt->play = rt->P_play_min - delta - 1;
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen == 0)&&
		((rt->B_skip_max + delta + 1) >= rt->B_skip_min)){
        rt->phase = B_PHASE;
        rt->delay = 0;
        rt->skip = rt->B_skip_max + delta + 1;
        rt->play = rt->B_play_min - delta - 1;
	
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen != 0)&&
		((rt->B_skip_max + delta + rt->P_skip_max +2) >=
		 rt->B_skip_min)){
        rt->phase = B_PHASE;
        rt->delay = 0;
        rt->skip = rt->B_skip_max + delta + rt->P_skip_max + 2;
        rt->play = rt->B_play_min - delta - rt->P_skip_max - 2;

      } else if((rt->B_frames_seen == 0)&&(rt->P_frames_seen == 0)){
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen == 0)){
	delta = delta  + rt->B_skip_max + 1;
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
      } else if((rt->B_frames_seen == 0)&&(rt->P_frames_seen != 0)){
	delta = delta + rt->P_skip_max + 1;
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen != 0)){
	delta = delta + rt->P_skip_max + rt->B_skip_max + 2;
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
	
      } else {
	fprintf(stdout,"unexpected combo\n");
	return;
      }
    }
  } else {            /* diff > 0 - need to speed up*/
   
  
    rt->skip += diff;
    rt->play -= diff;

    if(rt->play < rt->I_play_min){
      rt->play = 1;
      if(rt->skip > rt->Degraded_skip_max)
	rt->skip = rt->Degraded_skip_max;
      rt->phase = DEGRADED_PHASE;
      rt->delay = 0;}
   
  }  /* end outer else*/  
  return;
}   /* end function*/

/*************************************************************************
*  Function:      next_Delay_phase()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:      Global realtime state (as *rt)
*
*  Effects:     Global realtime state (as *rt)
*
*
*  Description: Given the current phase , goto the next phase.                                
*      The decision as to which phase to go to depends on the       
*      current state, the number of frames seen since the last
*      interval, the frame bias set for realtime adjustments,
*      and the damping variable settings. It is important to
*      avoid illogical states.  These are states which use
*      frame types that aren't included in the pattern. Range
*      checking is done to ensure wide swings can be accomodated.                                                                     *
*      The amount of change is converted from frame increments       
*      to time/delay loop increments for delay calculations.                                                                     *
**************************************************************************/


void next_Delay_phase(volatile RT_info *rt)
{
  int frames;
  float diff;
  float frame_time,actual_time;
  int delta;
  
  frames =     rt->I_frames_played + rt->P_frames_played + 
    rt->B_frames_played +
      rt->I_frames_skipped + rt->P_frames_skipped + 
	rt->B_frames_skipped  - rt->frame_bias;
  if(frames < 0)frames = 0;             

  frame_time = (float)1.0/rt->frames_per_second;
  actual_time =(float)1.0/(float)frames;
  diff = frame_time - actual_time;
 
  if((diff < 0.001)&&(diff > -0.001)){
    /* alls fine*/
  } else{ 
 
    if(diff < 0.0){      /* too slow - need to reduce delay or skip data*/
      rt->delay = rt->delay + (int)(diff*((float)rt->calibration_count));
      if(rt->delay < 0){ 
	delta = (int)(rt->frames_per_second - 
		      ((float)1.0/(((float)1.0/rt->frames_per_second)
				   -((float)rt->delay/
				     (float)rt->calibration_count))));
	rt->delay = 0; 
	
	if(delta > rt->max_up_swing){
	  delta = rt->max_up_swing;
	} 
        
	if((rt->B_frames_seen != 0)&&(rt->B_skip_min +delta <= 
				      rt->B_skip_max)){
	  rt->phase = B_PHASE;
	  rt->delay = 0;
	  rt->skip = rt->B_skip_min + delta;
	  rt->play = rt->B_play_max - delta;
	} else if((rt->P_frames_seen != 0)&&(rt->B_frames_seen != 0)&&
		  (rt->P_skip_min + delta - rt->B_play_max <= rt->P_skip_max)){
	  rt->phase = P_PHASE;
	  rt->delay = 0;
	  rt->skip = rt->P_skip_min + delta - rt->B_play_max;
	  rt->play = rt->P_play_max - delta + rt->B_play_max; 
	} else if((rt->P_frames_seen != 0)&&(rt->B_frames_seen == 0)&&
		  (rt->P_skip_min + delta <= rt->P_skip_max)){
	  rt->phase = P_PHASE;
	  rt->delay = 0;
	  rt->skip = rt->P_skip_min + delta;
	  rt->play = rt->P_play_max - delta; 
	} else if((rt->P_frames_seen == 0)&&(rt->B_frames_seen == 0)){
	  rt->phase = I_PHASE;
	  rt->delay = 0;
	  rt->skip = rt->I_skip_min + delta;
	  rt->play = rt->I_play_max - delta;
	  if(rt->play < rt->I_play_min){
	    rt->play = 1;
	    if(rt->skip > rt->Degraded_skip_max)
              rt->skip = rt->Degraded_skip_max;
	    rt->phase = DEGRADED_PHASE;
	    rt->delay = 0;}
	} else if((rt->P_frames_seen != 0)&&(rt->B_frames_seen == 0)){
	  rt->phase = I_PHASE;
	  rt->delay = 0;
	  delta = delta - rt->P_play_max;
	  rt->skip = rt->I_skip_min + delta;
	  rt->play = rt->I_play_max - delta;
	  if(rt->play < rt->I_play_min){
	    rt->play = 1;
	    if(rt->skip > rt->Degraded_skip_max)
              rt->skip = rt->Degraded_skip_max;
	    rt->phase = DEGRADED_PHASE;
	    rt->delay = 0;}
	} else if((rt->P_frames_seen == 0)&&(rt->B_frames_seen != 0)){
	  rt->phase = I_PHASE;
	  rt->delay = 0;
	  delta = delta -rt->B_play_max;
	  rt->skip = rt->I_skip_min + delta;
	  rt->play = rt->I_play_max - delta;
	  if(rt->play < rt->I_play_min){
	    rt->play = 1;
	    if(rt->skip > rt->Degraded_skip_max)
              rt->skip = rt->Degraded_skip_max;
	    rt->phase = DEGRADED_PHASE;
	    rt->delay = 0;}
          
	} else if((rt->P_frames_seen != 0)&&(rt->B_frames_seen != 0)){
	  rt->phase = I_PHASE;
	  rt->delay = 0;
	  delta = delta - rt->P_play_max - rt->B_play_max;
	  rt->skip = rt->I_skip_min + delta;
	  rt->play = rt->I_play_max - delta ;
	  if(rt->play < rt->I_play_min){
	    rt->play = 1;
	    if(rt->skip > rt->Degraded_skip_max)
              rt->skip = rt->Degraded_skip_max;
	    rt->phase = DEGRADED_PHASE;
	    rt->delay = 0;}
	} else {
	  fprintf(stdout,"unexpected combo\n");
	  return;
	}
      }
    } else {  /* diff >0 - too fast -> increase delay*/
      rt->delay = rt->delay + (int)(diff*((float)rt->calibration_count));
      if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
    }
 
  } /* end else*/ 
  return;
} /*end program*/
/*************************************************************************
*  Function:      next_Degraded_phase()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:      Global realtime state (as *rt)
*
*  Effects:     Global realtime state (as *rt)
*
*  Description:
*      Given the current phase , goto the next phase.                                
*      Since the degraded phases have a sum of frames to both       
*      skip and play the  is greater than the frames per second
*      the interval timer values must be streached in the interrupt                                                                     
*      handle for this phase.
**************************************************************************/


void next_Degraded_phase(volatile RT_info *rt)
{
  int frames;
  int diff,delta;
  double Dampen_scale_factor;
  int bound_up,bound_down;
 
  frames =     rt->I_frames_played + rt->P_frames_played + 
    rt->B_frames_played +
      rt->I_frames_skipped + rt->P_frames_skipped + 
	rt->B_frames_skipped  - rt->frame_bias;
  if(frames < 0)frames = 0;             

  diff = ((int)(rt->time_increment*rt->frames_per_second)) - frames;
 
  if((rt->clock_90khz - rt->clock_frame) > 0 ){
    delta =((int)(rt->clock_90khz - rt->clock_frame))/rt->frame_increment;
    diff += delta;
  }

  Dampen_scale_factor = (double)4.0;

  bound_up = (int)((double)rt->max_up_swing/Dampen_scale_factor);
  bound_down =(int)((double)rt->max_down_swing/Dampen_scale_factor);
 
  if(diff < -bound_down){
    diff = -bound_down;
  }else if(diff > bound_up){
    diff = bound_up;
  }else{} 
 
  if(diff == 0){
    /* all is well*/
  } else if(diff < 0){    /* need to slow down*/
    if((rt->skip + diff) >= rt->I_play_max){
      rt->skip += diff;
    } else if((rt->skip +diff) >= rt->I_skip_min){
      rt->phase = I_PHASE;
      rt->skip = rt->skip +diff;
      rt->play = rt->I_play_max - rt->skip;
    } else {
      delta = rt->skip + diff - rt->I_skip_min ;
      if((rt->P_frames_seen !=0)&&((rt->P_skip_max + delta + 1) >= 
				   rt->P_skip_min)){
        rt->phase = P_PHASE;
        rt->delay = 0;
        rt->skip = rt->P_skip_max + delta + 1;
        rt->play = rt->P_play_min - delta - 1;
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen == 0)&&
		((rt->B_skip_max + delta + 1) >= rt->B_skip_min)){
        rt->phase = B_PHASE;
        rt->delay = 0;
        rt->skip = rt->B_skip_max + delta + 1;
        rt->play = rt->B_play_min - delta - 1;
	
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen != 0)&&
		((rt->B_skip_max + delta + rt->P_skip_max +2) >=
		 rt->B_skip_min)){
        rt->phase = B_PHASE;
        rt->delay = 0;
        rt->skip = rt->B_skip_max + delta + rt->P_skip_max + 2;
        rt->play = rt->B_play_min - delta - rt->P_skip_max - 2;

      } else if((rt->B_frames_seen == 0)&&(rt->P_frames_seen == 0)){
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen == 0)){
	delta = delta  + rt->B_skip_max + 1;
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
      } else if((rt->B_frames_seen == 0)&&(rt->P_frames_seen != 0)){
	delta = delta + rt->P_skip_max + 1;
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
      } else if((rt->B_frames_seen != 0)&&(rt->P_frames_seen != 0)){
	delta = delta + rt->P_skip_max + rt->B_skip_max + 2;
	rt->phase = DELAY_PHASE;
	rt->delay = (int)((((float)1.0/rt->frames_per_second) -
			   ((float)1.0/(rt->frames_per_second -
					(float)delta)))*
			  (float)rt->calibration_count);
	if(rt->delay > rt->delay_max) rt->delay = rt->delay_max;  
	rt->play = (int)rt->frames_per_second;
	rt->skip = 0;
       
      } else {
	fprintf(stdout,"unexpected combo\n");
	return;
      }
    }
  } else {            /* diff > 0 - need to speed up*/
   
 
    rt->skip += diff;

    if(rt->skip > rt->Degraded_skip_max)
      rt->skip = rt->Degraded_skip_max;
   
  }  /* end outer else*/  
  return;
}   /* end function*/
/*************************************************************************
*  Function:      print_rt_info()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:     Global realtime state (as *rt)
*
*  Effects:    none
*
*
*  Description:   Print out current state. Only called if complied with                                
*     the RT_ANALYSIS compiler definition.        
*                                                                           
**************************************************************************/


void print_rt_info(volatile RT_info rt)
{
 
  fprintf(stdout,"phase: %d play: %d skip %d frame_bias %d\n", rt.phase,
	  rt.play,rt.skip,rt.frame_bias);
  fprintf(stdout,"seen:    I %d P %d B %d \n",rt.I_frames_seen,
	  rt.P_frames_seen,rt.B_frames_seen);
  fprintf(stdout,"played:  I %d P %d B %d \n",rt.I_frames_played,
	  rt.P_frames_played,rt.B_frames_played);
  fprintf(stdout,"skipped: I %d P %d B %d \n",rt.I_frames_skipped,
	  rt.P_frames_skipped,rt.B_frames_skipped);
  fprintf(stdout,"fps: %f delay: %d mode: %d cal_count: %d \n",
	  rt.frames_per_second,rt.delay,rt.mode,rt.calibration_count);
  fprintf(stdout,"time_increment %f max_delay: %d\n",rt.time_increment,rt.delay_max);
  fprintf(stdout,"  play_max skip_min play_min skip_max\n");
  fprintf(stdout,"I: %d       %d      %d        %d\n",
	  rt.I_play_max,rt.I_skip_min, rt.I_play_min,
	  rt.I_skip_max);
  fprintf(stdout,"P: %d       %d      %d        %d\n",
	  rt.P_play_max,rt.P_skip_min, rt.P_play_min,
	  rt.P_skip_max);
  fprintf(stdout,"B: %d       %d      %d        %d     %d\n",
	  rt.B_play_max,rt.B_skip_min, rt.B_play_min,
	  rt.B_skip_max,rt.Degraded_skip_max);
  fprintf(stdout,"max_up: %d max_down: %d frame_inc %d\n",
	  rt.max_up_swing,rt.max_down_swing,rt.frame_increment);
  fprintf(stdout,"pause: %d RTC: %d frame_clock: %d \n\n",
	  rt.pause,rt.clock_90khz,rt.clock_frame);                
}


