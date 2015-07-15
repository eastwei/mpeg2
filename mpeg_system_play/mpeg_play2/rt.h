/*===========================================================================*
 * realtime.h                                                                   *
 *                                                                           *
 *      procedures to deal with realtime state                                   
 *                  By
 *             James Boucher
 *      Multimedia Communications Lab
 *           Boston University
 *                                                                           *
 *===========================================================================*/

/*
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
 *                       IN ADDITION:
 * IN NO EVENT SHALL THE BOSTON UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF BOSTON
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE BOSTON UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE BOSTON UNIVERSITY HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 
 */

#define DELAY_PHASE 0
#define B_PHASE 1
#define P_PHASE 2
#define I_PHASE 3 
#define DEGRADED_PHASE 4

#define COUNTING -1
#define CALIBRATE 0
#define RUN 1

#define PLAY_FRAME 1
#define SKIP_FRAME -10

#define PLAY  0
#define PAUSE 1


#define RTC_SECOND 90000     /*MPEG I 90Khz system clock*/

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
   int Degraded_skip_max;
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

  
