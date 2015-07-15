/*************************************************************************
*  File Name:     realtime.h
*  Creation Date: 12 August 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*  Usage:         define and declare for realtime.c
*
*  Description:   This header declares the function prototypes
* required to use realtime.c. The required external(global)
* variables and structures are also declared.
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


/*#define RT_ANALYSIS*/  /* uncomment this if you want the realtime state
                        printed out*/
/*#define SHORT_ANALYSIS*/  /* for minimal analysis data*/

/* define four phases of operation*/                        
#define DELAY_PHASE 0   /* when platform can play at faster than req'd*/
#define B_PHASE 1       /* when only skipping B frames*/
#define P_PHASE 2       /* when only skipping P and B frames*/
#define I_PHASE 3       /* when skipping I, B, and P frames*/
#define DEGRADED_PHASE 4 /* when we can't even play one I frame per second(Yuck)*/

/* define three modes of operation*/
#define COUNTING -1     /* counting dummy loops for delay estimation*/
#define CALIBRATE 0     /* adapting step resolution to specific frame pattern*/
#define RUN 1           /* normal running state*/

#define PLAY_FRAME 1     /* switches used to return play or skip decision*/
#define SKIP_FRAME -10

#define PLAY  0      /* toggle that prevents overshooting of real-time*/
#define PAUSE 1


#define RTC_SECOND 90000     /*MPEG I 90Khz system clock*/

/* Massive realtime state structure*/
typedef struct Rt_info {
   int I_frames_seen;       /* not reset - global tally */
   int P_frames_seen;
   int B_frames_seen;
   int I_frames_played;    /*reset on interval */
   int P_frames_played;
   int B_frames_played;
   int I_frames_skipped;
   int P_frames_skipped;
   int B_frames_skipped;
   float frames_per_second;   /* set when sequence header read*/
   float time_increment;   /* realtime interval = 1.0 except in Deg. mode*/
   int delay;        /* frame delay when running too fast in count loops*/
   int delay_max;  /* delay per frame if decoding instantaneous*/
   int phase;     /* I,P,B, Delay or Degraded*/
   int mode;       /* counting,calibrate or run */ 
   int pause;      /* pause if more than a second over realtime*/
   int B_play_max;    /* boundary conditions for state variables*/
   int B_skip_min;
   int B_play_min;
   int B_skip_max;
   int P_play_max;
   int P_skip_min;
   int P_play_min;
   int P_skip_max;
   int I_play_max;
   int I_skip_min;
   int I_play_min;
   int I_skip_max;
   int Degraded_skip_max;  /* put a bound on how bad things can get*/
   int play;            /*current state with respect to phase*/
   int skip;
   int calibration_count;   /* delay*calibration_count = 1.0 seconds*/ 
   int max_up_swing;     /* used for dampening transient response*/
   int max_down_swing;
   int frame_bias;     /* used for minor adjustments to maintain realtime*/
   long clock_90khz;  /* realtime clock based on intervals*/
   long clock_frame;    /* clock value based on frames played*/
   int frame_increment;  /* clock ticks per frame*/
   } RT_info; 

/* SYSTEM_PLAY time stamp structure used to accest shared memory*/
typedef struct Time_Stamps{
  long SCR;                 /* System Clock Reference*/
  long DTS[48];                /*Decoded Time Stamp*/
  long PTS[48];                /*Presentation Time Stamp*/
  long start_SCR;
  long start_PTS[48];
  long start_DTS[48];
  long audio_frame_clk;
  int  started[48];
  int  mixer_pid;
}TIME_stamps;

extern int mixer_pid; /* SYSTEM_PLAY audio mixer process identifier*/
   
extern void init(volatile RT_info *);
extern void setup(volatile RT_info *,int);
extern void calibrate(volatile RT_info *);
extern int  play_or_skip(volatile RT_info *);
extern void next_B_phase(volatile RT_info *);
extern void next_P_phase(volatile RT_info *);
extern void next_I_phase(volatile RT_info *);
extern void next_Delay_phase(volatile RT_info *);
extern void next_Degraded_phase(volatile RT_info *);
extern void print_rt_info(volatile RT_info);

  
