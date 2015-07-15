 /*
 * Copyright (c) 1992 The Regents of the University of California.
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
 */
#include "video.h"
#include "proto.h"
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "realtime.h"
#ifndef MIPS
#include <netinet/in.h>
#else
#include <bsd/netinet/in.h>
#endif

#include "util.h"
#include "dither.h"


/* Define buffer length. */
#ifdef SYSTEM_PLAY
#define BUF_LENGTH 5000
#else
#define BUF_LENGTH 80000
#endif

/*define the audio buffer time bias*/


#ifdef INDIGO 
#define AUDIO_BIAS -18750  /*20KB * 90000/48000*2 bytes/sample*/
#endif

#ifdef LINUX
#define AUDIO_BIAS -4082  /*4KB * 90000/44100*2 bytes/sample*/
#endif

#ifdef SPARC1PX
#define AUDIO_BIAS -90000 /*8KB * 90000/(8000 * 1 byte/sample)*/
#endif

#ifndef AUDIO_BIAS
#define AUDIO_BIAS 0
#endif


/* define control states*/
#define CTL_STOP 1
#define CTL_PLAY 2
#define CTL_PAUSE 3

/* Function return type declarations */
void usage();

/* External declaration of main decoding call. */

extern VidStream *mpegVidRsrc();
extern VidStream *NewVidStream();

/* Globals to hold command line parameters*/
int realtime = NON_REALTIME;    
int x_position = 200,y_position = 200;
int win_height = 128, win_width = 160;
char *win_title = "MPEG Play";
int shm_id;   /* shared memory identifier*/
int sem_id;   /* semaphore identifier*/
TIME_stamps *time_stamps;
int stream_id ;  /*video identifies in range 0 to M-1*/
int N; /*number of audio channels*/
int M; /*number of video channels*/
int mixer_pid;  /* system_play audio mixer process identifier*/

/* Global structure for realtime info*/
volatile RT_info rt_info;

/* Global for control state*/
volatile int control1;
volatile int control2;

/* Declaration of global variable to hold dither info. */

int ditherType;
/* Global file pointer to incoming data. */
FILE *input;

/* End of File flag. */
static int EOF_flag = 0;

/* Loop flag. */
int loopFlag = 0;

/* Shared memory flag. */
int shmemFlag = 0;

/* Quiet flag. */

#ifdef SYSTEM_PLAY
int quietFlag = 1;
#else

int quietFlag = 0;
#endif

/* Display image on screen? */
int noDisplayFlag = 0;

/* Setjmp/Longjmp env. */
jmp_buf env;


/*
 *--------------------------------------------------------------
 *
 * get_more_data --
 *
 *	Called by correct_underflow in bit parsing utilities to
 *      read in more data.
 *
 * Results:
 *	Input buffer updated, buffer length updated.
 *      Returns 1 if data read, 0 if EOF, -1 if error.
 *
 * Side effects:
 *      None.
 *
 *--------------------------------------------------------------
 */

int 
get_more_data(buf_start, max_length, length_ptr, buf_ptr)
     unsigned int *buf_start;
     int max_length;
     int *length_ptr;
     unsigned int **buf_ptr;
{
  
  int length, num_read, i, request;
  unsigned char *buffer, *mark;
  unsigned int *lmark;
  int diff,frames;
  int diff_read; 
  int num_read_rounded;
  unsigned char *index;
  unsigned char *mark2;

  if (EOF_flag) return 0;

  length = *length_ptr;
  buffer = (unsigned char *) *buf_ptr;

  if (length > 0) {
    memcpy((unsigned char *) buf_start, buffer, (length*4));
    mark = ((unsigned char *) (buf_start + length));
  }
  else {
    mark = (unsigned char *) buf_start;
    length = 0;
  }

  request = (max_length-length)*4;

frames = rt_info.I_frames_seen +rt_info.P_frames_seen+rt_info.B_frames_seen;

  num_read = -10;
  while(((num_read == 0)&&(errno == 4))||(num_read < 0)||((frames < 3)&&(num_read <= 0))){  
    errno = 0;   
    num_read = fread( mark, 1, request, input);
/*    fprintf(stderr,"num read %d err %d\n",num_read,errno); 
    perror("now");
*/   
  }



  /* Paulo Villegas - 26/1/1993: Correction for 4-byte alignment */
  

  num_read_rounded = 4*(num_read/4);
  diff =  4 - num_read +num_read_rounded;
 
  /* this can happen only if num_read<request; */
  if( num_read_rounded < num_read )
    { 

      mark2 = (unsigned char *)(mark + num_read);
      while(diff >0){
	diff_read = -10;

	while(diff_read < 0){
	  diff_read = fread( mark2, 1, diff, input);
/*	fprintf(stderr,"diff_read %d\n",diff_read);
*/	}

	if(diff_read == 0)break; 
	mark2 = (unsigned char *)(mark2 + diff_read);
	diff -= diff_read;
	num_read += diff_read;
      }
    }

 
  if (num_read == 0) {
    *buf_ptr = buf_start;
    
    /* Make 32 bits after end equal to 0 and 32
       bits after that equal to seq end code
       in order to prevent messy data from infinite
       recursion.
    */

    *(buf_start + length) = 0x0;
    *(buf_start + length+1) = SEQ_END_CODE;

    EOF_flag = 1;
    return 0;
  }

  lmark = (unsigned int *) mark;

  num_read = num_read/4;

  for (i=0; i<num_read; i++) {
    *lmark = htonl(*lmark);
    lmark++;
  }

  *buf_ptr = buf_start;
  *length_ptr = length + num_read;
 
  return 1;
}

/*
 *--------------------------------------------------------------
 *
 * int_handler --
 *
 *	Handles Cntl-C interupts..
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */
void
int_handler()
{
  if (!quietFlag) {
    fprintf(stderr, "Interrupted!\n");
  }

   /* tell audio mixer that a video process is cancelled*/
#ifdef SYSTEM_PLAY
  kill(mixer_pid, SIGUSR1);
#endif

  if (curVidStream != NULL)
    DestroyVidStream(curVidStream);
  exit(1);
}

/*************************************************************************
*  Function:      control_usr1()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        signal number
*                
*
*  Effects:      control2 toggles PLAY/PAUSE
*                
*  Description:  Reception of the SIGUSR1 causes the system state
*                to toggle from PLAY to PAUSE and back. This
*                signal is generated in the audio-mixer/interface
*                process when the buttons PAUSE/RESUME/PAUSE ALL/
*                RESUME ALL are pressed.                                 
**************************************************************************/


static void control_usr1(int);

static void control_usr1(int signo)
{

  if(control2 == CTL_PLAY){
    control2 = CTL_PAUSE;
  }else{
    control2 = CTL_PLAY;
  }

  if(signal(SIGUSR1,control_usr1) == SIG_ERR){
    perror("can't set control handler");
  }
  return;
}

/*************************************************************************
*  Function:      control_usr2()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        signal number
*                
*
*  Effects:      control1 toggles PLAY/STOP
*                
*  Description:  Reception of the SIGUSR1 causes the system state
*                to toggle from PLAY to STOP and back. This
*                signal is generated in the audio-mixer/interface
*                process when the buttons START ALL/
*                STOP ALL are pressed.                                 
**************************************************************************/

static void control_usr2(int);

static void control_usr2(int signo)
{

  if(control1 == CTL_STOP){
    control1 = CTL_PLAY;
  }else{
    control1 = CTL_STOP;
  }

  if(signal(SIGUSR2,control_usr2) == SIG_ERR){
    perror("can't set control handler");
  }
  return;
}

/*************************************************************************
*  Function:      alarm_handler()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        signal number
*                
*
*  Effects:      global realtime state rt_info
*                
*  Description:  An interval timer causes this interrupt to be called
*                once a second(normally). The realtime state is
*                adjusted to attept to match the realtime clock
*                (or audio frame clock in the case of SYSTEM_PLAY)
*                to the video frame clock.
**************************************************************************/

static void alarm_handler(int);

static void alarm_handler(int signo)
{
  int sum;  /*total number of frames seen proir to calibration*/
  struct timeval interval2,zero2;  /* two timer values */
  struct itimerval timer2; /* interval timer for realtime*/
  struct itimerval otimer2; /* output interval timer */
/* shared Memory and Semaphore stuff*/
  key_t key;
  int nsems;
  int flag;
  int semnum;
  int cmd;
  union semun arg;
  int semrtn;
  struct sembuf semoparray[16];
  size_t nops;
  int rtnpid;
  int *shmptr;
  int shmrtn;
  struct shmid_ds *buff;
  int size,temp;
  long start_bias;

#ifdef SHORT_ANALYSIS
  int fs,fp;

fp = rt_info.I_frames_played +rt_info.P_frames_played +rt_info.B_frames_played;
fs = fp +rt_info.I_frames_skipped +rt_info.P_frames_skipped +rt_info.B_frames_skipped; 
#endif

#ifdef RT_ANALYSIS
  print_rt_info(rt_info);
#endif


  if(rt_info.pause == PAUSE)rt_info.pause = PLAY;

  if(rt_info.mode == COUNTING){
    rt_info.mode = CALIBRATE;
    if(signal(SIGALRM,alarm_handler)== SIG_ERR){
      perror("can't catch the alarm");
    }
#ifdef RT_ANALYSIS
    print_rt_info(rt_info);
    fprintf(stdout,"-------------------------------------\n");
#endif
    return;
  }



/* we only calibrate if we have seen 1 seconds worth of frames*/
  if(rt_info.mode == CALIBRATE){
    sum = rt_info.I_frames_seen+rt_info.P_frames_seen
      + rt_info.B_frames_seen;
    if(sum >= rt_info.frames_per_second){
      calibrate(&rt_info);
      rt_info.mode = RUN;
    }
  }

  if((control1 != CTL_STOP)&&(control2 != CTL_PAUSE)){
    /* Calculate next state*/ 
    if(rt_info.phase == B_PHASE){
      next_B_phase(&rt_info);
    }else if(rt_info.phase == P_PHASE){
      next_P_phase(&rt_info);
    }else if(rt_info.phase == I_PHASE){
      next_I_phase(&rt_info);
    } else if(rt_info.phase == DEGRADED_PHASE){
      next_Degraded_phase(&rt_info);
    }else if(rt_info.phase == DELAY_PHASE){
      next_Delay_phase(&rt_info);
    }else{
      fprintf(stderr,"invalid phase: %d\n",rt_info.phase);
    }
  }

  /* increment realtime clock */

  if(control1 != CTL_STOP){

#ifdef SYSTEM_PLAY

  /*access shared memory*/
  /*set semaphore*/
    nops = 1;
    semoparray[0].sem_num = 0;      /* semaphore id within set*/
    semoparray[0].sem_op = -1;        /*set the semaphore*/
    semoparray[0].sem_flg = SEM_UNDO;  /* ensure it goes away*/

    semrtn = semop(sem_id, semoparray,nops);
    if(semrtn <0){
      perror("could not  set");
    }
    start_bias = time_stamps->start_PTS[N+stream_id];

    rt_info.clock_90khz = time_stamps->audio_frame_clk
      + AUDIO_BIAS - start_bias;
    mixer_pid = getppid();

   /* release semaphore*/
    semoparray[0].sem_op = 1;        /*release the semaphore*/
    semrtn = semop(sem_id, semoparray,nops);
    if(semrtn <0){
      perror("could not  release");
    }


#else

    rt_info.clock_90khz += (int)(rt_info.time_increment*(float)RTC_SECOND);
#endif
  }

/* slip realtime interval if running in degraded mode*/
  if((rt_info.phase == DEGRADED_PHASE)&&(rt_info.mode == RUN)){
    rt_info.time_increment = ((float)(rt_info.B_play_max+rt_info.P_play_max + 
				      rt_info.skip +1))/
					rt_info.frames_per_second;
    interval2.tv_sec = (int)(rt_info.time_increment);
    interval2.tv_usec = (int)((rt_info.time_increment - 
			       (float)interval2.tv_sec)*((float)1000000.0));
    zero2.tv_sec = 0;
    zero2.tv_usec = 0;
    timer2.it_interval = interval2;
    timer2.it_value = interval2;
    otimer2.it_interval = zero2;
    otimer2.it_value = zero2;
      /* reset interval timer*/
    if(setitimer(ITIMER_REAL, &timer2,&otimer2)<0){
      perror("interval timer error");
    }
  }else if(rt_info.time_increment != (float)1.0){
    rt_info.time_increment = (float)1.0;
    interval2.tv_sec = 1;
    interval2.tv_usec = 0;
    zero2.tv_sec = 0;
    zero2.tv_usec = 0;
    timer2.it_interval = interval2;
    timer2.it_value = interval2;
    otimer2.it_interval = zero2;
    otimer2.it_value = zero2;
       /* reset interval timer */
    if(setitimer(ITIMER_REAL, &timer2,&otimer2)<0){
      perror("interval timer error");
    }
  }else{}

  /* reset frame states */
  if((control1 != CTL_STOP)&&(control2 != CTL_PAUSE)){
    rt_info.B_frames_played = 0;
    rt_info.B_frames_skipped = 0;
    rt_info.P_frames_played = 0;
    rt_info.P_frames_skipped = 0;
    rt_info.I_frames_played = 0;
    rt_info.I_frames_skipped = 0;
    rt_info.frame_bias = 0;
  }
#ifdef SHORT_ANALYSIS
fprintf(stdout,"%f %d %d %f\n",((float)rt_info.clock_90khz/(float)90000),fs,fp,((float)(rt_info.clock_90khz-rt_info.clock_frame)/(float)90000));
#endif

#ifdef RT_ANALYSIS
  print_rt_info(rt_info);
  fprintf(stdout,"-------------------------------------\n");
#endif

  if(signal(SIGALRM,alarm_handler)== SIG_ERR){
    perror("can't catch the alarm");
  }

  return;
}


/*
 *--------------------------------------------------------------
 *
 * main --
 *
 *	Parses command line, starts decoding and displaying.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

void
main(argc, argv)
     int argc;
     char **argv;
{

  char *name;
  static VidStream *theStream;
  int mark;
  int i,j,k;
  struct timeval interval1,zero;
  struct itimerval timer1;
  struct itimerval otimer1;
  int * shmptr;

  interval1.tv_sec = 1;
  interval1.tv_usec = 0;
  zero.tv_sec = 0;
  zero.tv_usec = 0;
  timer1.it_interval = interval1;
  timer1.it_value = interval1;
  otimer1.it_interval = zero;
  otimer1.it_value = zero;


#ifdef SYSTEM_PLAY
  control1 = CTL_STOP;
#else
  control1 = CTL_PLAY;
#endif

  control2 = CTL_PLAY;  


  init(&rt_info);  /* initialize realtime info structure*/

  if(signal(SIGALRM,alarm_handler) == SIG_ERR){
    perror("could not catch alarm");
  }
  if(signal(SIGUSR1,control_usr1) == SIG_ERR){
    perror("could not catch usr1 signal");
  }
  if(signal(SIGUSR2,control_usr2) == SIG_ERR){
    perror("could not catch usr2 signal");
  }

  if(signal(SIGINT,int_handler) == SIG_ERR){
    perror("could not catch interrupt");
  }


  mark = 1;
  argc--;

  name = "";
  input = stdin;
  ditherType = ORDERED2_DITHER;
  LUM_RANGE = 8;
  CR_RANGE = CB_RANGE = 4;
  noDisplayFlag = 0;

#ifdef SH_MEM
  shmemFlag = 1;
#endif

  while (argc) {
    if (strcmp(argv[mark], "-nop") == 0) {
      TogglePFlag();
      argc--; mark++;
    } else if (strcmp(argv[mark], "-realtime") == 0){
      realtime = REALTIME;
      argc--; mark++;
    } else if (strcmp(argv[mark], "-title") == 0){
      if(argc < 1) {
        perror("must specify title after -title flag");
        usage(argv[0]);      
      }
      argc--; mark++;
      win_title = argv[mark];
      argc--; mark++;      
      sscanf(&win_title[0],"%d",&stream_id);
    } else if (strcmp(argv[mark], "-position") == 0){
      if(argc< 2) {
        perror("must specify position: horizontal vertical");
        usage(argv[0]);
      }
      argc--; mark++;
      x_position = atoi(argv[mark]);
      argc--; mark++;
      y_position = atoi(argv[mark]);
      argc--; mark++;
    } else if (strcmp(argv[mark], "-dimension") == 0){
      if(argc<2) {
         perror("must specify window dimensions: horz. vert");
         usage(argv[0]);
       }
       argc--; mark++;
       win_width = atoi(argv[mark]);
       argc--; mark++;
       win_height = atoi(argv[mark]);
       argc--; mark++;
    } else if (strcmp(argv[mark], "-process") == 0){
      if(argc<2) {
         perror("must specify number of audio then video processes");
         usage(argv[0]);
       }
       argc--; mark++;
       N = atoi(argv[mark]);
       argc--; mark++;
       M = atoi(argv[mark]);
       argc--; mark++;
    } else if (strcmp(argv[mark], "-shm") == 0){
      if(argc<2) {
         perror("must specify shared memory and semaphore id");
         usage(argv[0]);
       }
       argc--; mark++;
       shm_id= atoi(argv[mark]);
       argc--; mark++;
       sem_id = atoi(argv[mark]);
       argc--; mark++;
    } else if (strcmp(argv[mark], "-nob") == 0) {
      ToggleBFlag();
      argc--; mark++;
    } else if (strcmp(argv[mark], "-display") == 0) {
      name = argv[++mark];
      argc -= 2; mark++;
    } else if (strcmp(argv[mark], "-dither") == 0) {
      argc--; mark++;
      if (argc < 1) {
	perror("Must specify dither option after -dither flag");
	usage(argv[0]);
      }
      if (strcmp(argv[mark], "hybrid") == 0) {
	argc--; mark++;
	ditherType = HYBRID_DITHER;
      } else if (strcmp(argv[mark], "hybrid2") == 0) {
	argc--; mark++;
	ditherType = HYBRID2_DITHER;
      } else if (strcmp(argv[mark], "fs4") == 0) {
	argc--; mark++;
	ditherType = FS4_DITHER;
      } else if (strcmp(argv[mark], "fs2") == 0) {
	argc--; mark++;
	ditherType = FS2_DITHER;
      } else if (strcmp(argv[mark], "fs2fast") == 0) {
	argc--; mark++;
	ditherType = FS2FAST_DITHER;
      } else if (strcmp(argv[mark], "hybrid2") == 0) {
	argc--; mark++;
	ditherType = HYBRID2_DITHER;
      } else if (strcmp(argv[mark], "2x2") == 0) {
	argc--; mark++;
	ditherType = Twox2_DITHER;
      } else if (strcmp(argv[mark], "gray") == 0) {
	argc--; mark++;
	ditherType = GRAY_DITHER;
      } else if (strcmp(argv[mark], "color") == 0) {
	argc--; mark++;
	ditherType = FULL_COLOR_DITHER;
      } else if (strcmp(argv[mark], "none") == 0) {
	argc--; mark++;
	ditherType = NO_DITHER;
      } else if (strcmp(argv[mark], "ordered") == 0) {
	argc--; mark++;
	ditherType = ORDERED_DITHER;
      } else if (strcmp(argv[mark], "ordered2") == 0) {
	argc--; mark++;
	ditherType = ORDERED2_DITHER;
      } else if (strcmp(argv[mark], "mbordered") == 0) {
	argc--; mark++;
	ditherType = MBORDERED_DITHER;
      } else if (strcmp(argv[mark], "mono") == 0) {
	argc--; mark++;
	ditherType = MONO_DITHER;
      } else if (strcmp(argv[mark], "threshold") == 0) {
	argc--; mark++;
	ditherType = MONO_THRESHOLD;
      } else {
	perror("Illegal dither option.");
	usage(argv[0]);
      }
    } 
    else if (strcmp(argv[mark], "-eachstat") == 0) {
      argc--; mark++;
#ifdef ANALYSIS
      showEachFlag = 1;
#else
      fprintf(stderr, "To use -eachstat, recompile with -DANALYSIS in CFLAGS\n");
      exit(1);
#endif
    }
    else if (strcmp(argv[mark], "-shmem_off") == 0) {
      argc--; mark++;
      shmemFlag = 0;
    }
    else if (strcmp(argv[mark], "-quiet") == 0) {
      argc--; mark++;
      quietFlag = 1;
    }
    else if (strcmp(argv[mark], "-loop") == 0) {
      argc--; mark++;
      loopFlag = 1;
    }
    else if (strcmp(argv[mark], "-no_display") == 0) {
      argc--; mark++;
      noDisplayFlag = 1;
    }
    else if (strcmp(argv[mark], "-l_range") == 0) {
      argc--; mark++;
      LUM_RANGE = atoi(argv[mark]);
      if (LUM_RANGE < 1) {
	fprintf(stderr, "Illegal luminance range value: %d\n", LUM_RANGE);
	exit(1);
      }
      argc--; mark++;
    }
    else if (strcmp(argv[mark], "-cr_range") == 0) {
      argc--; mark++;
      CR_RANGE = atoi(argv[mark]);
      if (CR_RANGE < 1) {
	fprintf(stderr, "Illegal cr range value: %d\n", CR_RANGE);
	exit(1);
      }
      argc--; mark++;
    }
    else if (strcmp(argv[mark], "-cb_range") == 0) {
      argc--; mark++;
      CB_RANGE = atoi(argv[mark]);
      if (CB_RANGE < 1) {
	fprintf(stderr, "Illegal cb range value: %d\n", CB_RANGE);
	exit(1);
      }
      argc--; mark++;
    }
    else if (argv[mark][0] == '-') {
      fprintf(stderr, "Un-recognized flag %s\n",argv[mark]);
      usage(argv[0]);
    }
    else {
      input = fopen(argv[mark], "r");
      if (input == NULL) {
	fprintf(stderr, "Could not open file %s\n", argv[mark]);
	usage(argv[0]);
      }
      argc--; mark++;
    }
  }

  lum_values = (int *) malloc(LUM_RANGE*sizeof(int));
  cr_values = (int *) malloc(CR_RANGE*sizeof(int));
  cb_values = (int *) malloc(CB_RANGE*sizeof(int));


  init_tables();
  
  switch (ditherType) {
    
  case HYBRID_DITHER:
    
    InitColor();
    InitHybridDither();
    InitDisplay(name);
    break;
    
    case HYBRID2_DITHER:
    InitColor();
    InitHybridErrorDither();
    InitDisplay(name);
    break;
    
  case FS4_DITHER:
    InitColor();
    InitFS4Dither();
      InitDisplay(name);
    break;
    
  case FS2_DITHER:
    InitColor();
    InitFS2Dither();
    InitDisplay(name);
    break;
    
  case FS2FAST_DITHER:
    InitColor();
    InitFS2FastDither();
    InitDisplay(name);
    break;
    
  case Twox2_DITHER:
    InitColor();
    Init2x2Dither();
    InitDisplay(name);
    PostInit2x2Dither();
    break;

  case GRAY_DITHER:
    InitGrayDisplay(name);
    break;

  case FULL_COLOR_DITHER:
    InitColorDither();
    InitColorDisplay(name);
    break;

  case NO_DITHER:
    shmemFlag = 0;
    break;

  case ORDERED_DITHER:
    InitColor();
    InitOrderedDither();
    InitDisplay(name);
    break;

  case MONO_DITHER:
  case MONO_THRESHOLD:
    InitMonoDisplay(name);
    break;

  case ORDERED2_DITHER:
    InitColor();
    InitDisplay(name);
    InitOrdered2Dither();
    break;

  case MBORDERED_DITHER:
    InitColor();
    InitDisplay(name);
    InitMBOrderedDither();
    break;

  }

#ifdef SH_MEM
    if (shmemFlag && (display != NULL)) {
      if (!XShmQueryExtension(display)) {
	shmemFlag = 0;
	if (!quietFlag) {
	  fprintf(stderr, "Shared memory not supported\n");
	  fprintf(stderr, "Reverting to normal Xlib.\n");
	}
      }
    }
#endif

  if (setjmp(env) != 0) {

    DestroyVidStream(theStream);

    rewind(input);

    EOF_flag = 0;
    curBits = 0;
    bitOffset = 0;
    bufLength = 0;
    bitBuffer = NULL;
    totNumFrames = 0;
#ifdef ANALYSIS 
    init_stats();
#endif

  }

  theStream = NewVidStream(BUF_LENGTH);


  mpegVidRsrc(0, theStream);

  if (ditherType == Twox2_DITHER) i = 2;
  else i = 1;  

  ResizeDisplay(curVidStream->h_size*i, curVidStream->v_size*i);

#ifdef SYSTEM_PLAY
  shmptr = (int *) shmat(shm_id,0,0);
  if(*shmptr < 0){
    perror("shmat error");
  }

  time_stamps = (TIME_stamps *)shmptr;

#endif

  /* calibrate system speed for realtime mode*/
  if((rt_info.mode == COUNTING)&&(realtime == REALTIME)){

    if(setitimer(ITIMER_REAL, &timer1,&otimer1)<0){
      perror("interval timer error");
    }

    while(1){  /*count 1 second of delay intervals*/
      rt_info.calibration_count++;
      for(k=0;k<1000;k++){
	j=0;
	j++;
      }
      if(rt_info.mode != COUNTING) break;
      
    }
  } /*end if*/
  
  realTimeStart = ReadSysClock();
  
  while (1) mpegVidRsrc(0, theStream);

}
 

/*
 *--------------------------------------------------------------
 *
 * usage --
 *
 *	Print mpeg_play usage
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	exits with a return value -1
 *
 *--------------------------------------------------------------
 */

void
usage(s)
char *s;	/* program name */
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "mpeg_play\n");
    fprintf(stderr, "          [-realtime]\n");
    fprintf(stderr, "          [-nob]\n");
    fprintf(stderr, "          [-nop]\n");
    fprintf(stderr, "          [-dither {ordered|ordered2|mbordered|fs4|fs2|fs2fast|hybrid|\n");
    fprintf(stderr, "                    hybrid2|2x2|gray|color|none|mono|threshold}]\n");
    fprintf(stderr, "          [-loop]\n");
    fprintf(stderr, "          [-eachstat]\n");
    fprintf(stderr, "          [-no_display]\n");
    fprintf(stderr, "          [-quiet]\n");
    fprintf(stderr, "          [-title {window title}]\n");
    fprintf(stderr, "          [-position x y] initial window\n");
    fprintf(stderr, "          [-dimension x y] initial dimensions\n");
    fprintf(stderr, "          file_name\n");
    exit (-1);
}



/*
 *--------------------------------------------------------------
 *
 * DoDitherImage --
 *
 *	Called when image needs to be dithered. Selects correct
 *      dither routine based on info in ditherType.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

void
DoDitherImage(l, Cr, Cb, disp, h, w) 
unsigned char *l, *Cr, *Cb, *disp;
int h,w;
{

  switch(ditherType) {
  case HYBRID_DITHER:
    HybridDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case HYBRID2_DITHER:
    HybridErrorDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FS2FAST_DITHER:
    FS2FastDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FS2_DITHER:
    FS2DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FS4_DITHER:
    FS4DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case Twox2_DITHER:
    Twox2DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case FULL_COLOR_DITHER:
    ColorDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case GRAY_DITHER:
    GrayDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case NO_DITHER:
    break;

  case ORDERED_DITHER:
    OrderedDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case MONO_DITHER:
    MonoDitherImage(l, Cr, Cb, disp, h, w);
    break;

  case MONO_THRESHOLD:
    MonoThresholdImage(l, Cr, Cb, disp, h, w);
    break;

  case ORDERED2_DITHER:
    Ordered2DitherImage(l, Cr, Cb, disp, h, w);
    break;

  case MBORDERED_DITHER:
    MBOrderedDitherImage(l, Cr, Cb, disp, h, w);
    break;
  }
}
