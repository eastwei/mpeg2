/*************************************************************************
*  File Name:     system.c
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher, John Palmer, and Elisa Rubin
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*  Usage:         functions called from system.c
*
*  Description:   This is the main file to system_play. This file
*  starts by acquiring and opening the input file. It then spawns
*  the required number of audio and video decoding processes.  Next,
*  the user interface/audio mixer process is spawned.  Finally, the
*  parent process turns into the system layer demultiplexer process.
*  Process communicate via pipes, signals, and a semaphore controlled
*  shared memory region.
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
*/



#include "system.h"
#include "sp_wins.h"


/* Global variable */
volatile unsigned long rtc_value = 0;
volatile int quit = FALSE;       /* causes process termination if TRUE*/


/* interrupt handler */
static void sig_alarm(int);    /* used to adjust audio frame clock if*/
                               /* no audio streams active*/ 
static void sig_ctrlC(int);    /* used by demux on terminal interrupt*/
static void sig_ctrlC2(int);   /* used by mixer/interface*/
static void sig_pipe(int);     /* called if writing to a closed pipe*/
static void sig_usr1(int);     /* sent by each video process upon termination*/

/*************************************************************************
*  Function:      sig_alarm()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       signal number
*
*  Effects:      Updates audio frame clock if no audio streams active
*                resets audio frame count
*                
*  Description:  This interrupt service routine is called upon a SIGALRM
*                signal caused every second by the realtime interval timer.                                  
**************************************************************************/

static void sig_alarm(int signo)
{
  TIME_stamps ts;
  int semrtn;
  struct sembuf semoparray[16];
  struct shmid_ds *buff;
  size_t nops;
  int i;
  int audio_started,video_started;
  int shmrtn;
  int semnum;
  int cmd;
  union semun arg;
  long end_time;
  long elapse;
  int stat;
  int rtn;

/* see if any video processes have been cancelled*/

  for(i=N;i<(N+M);i++){
    rtn = waitpid(pid[i],&stat,WNOHANG);
    if(rtn <0){
#ifdef ANALYSYS
      fprintf(stderr,"found video ended\n");
#endif
      adi->pstatus[i] = CANCEL;
    }
  }


/* needs to update the audio frame clock if no audio streams active*/
  if(((sync_state == INTERVAL_SYNC)&&(master_state!=STOP))
     ||(quit==TRUE)){
    /*access shared memory*/
    /*set semaphore*/
    nops = 1;
    semoparray[0].sem_num = 0;      /* semaphore id within set*/
    semoparray[0].sem_op = -1;        /*set the semaphore*/
    semoparray[0].sem_flg = SEM_UNDO;  /* ensure it goes away*/

    semrtn = semop(sem_id, semoparray,nops);
    /* copy all data from the share memory structure into ts*/
    ts.SCR = time_stamps->SCR;
    time_stamps->audio_frame_clk += RTC_INCREMENT;
    ts.audio_frame_clk = time_stamps->audio_frame_clk;
    for(i=0;i<(N+M);i++){
      ts.PTS[i] = time_stamps->PTS[i];
      ts.DTS[i] = time_stamps->DTS[i];
      ts.started[i] = time_stamps->started[i];
    }

    time_stamps->mixer_pid = getpid();
    /* release semaphore*/
    semoparray[0].sem_op = 1;        /*release the semaphore*/
    semrtn = semop(sem_id, semoparray,nops);

    /*evaluate sync state*/
    /* calculate number of currently active audio channels*/
    audio_active = N;
    for(i=0;i<N;i++){
      if(adi->pstatus[i]== CANCEL){
	audio_active--;
      }
    }

    /*video active maintained globally*/

    /*calculate number of audio streams started*/
    audio_started = 0;
    for(i=0;i<N;i++){
      if(ts.started[i] == TRUE){
	audio_started++;
      }
    }

    /*calculate number of video streams started*/
    video_started = 0;
    for(i=N;i<N+M;i++){
      if(ts.started[i] == TRUE){
	video_started++;
      }
    }

    /*calculate next state*/

    if(((audio_active == 0)&&(video_active==0))||(quit==TRUE)){

      /*do end procedure*/
      /* wait for playout to finish and close audio port */
      end_audio(audio_device);
      close(adi->fd_audio_out);

      shmrtn = shmdt((void *)time_stamps);
      if(shmrtn < 0){
	perror("shmdt error");
      }
      cmd = IPC_RMID;
      semnum = 0;
      
      semrtn = semctl(sem_id,semnum,cmd,arg);
      if(semrtn <0){
	perror("could not remove semaphore set");
      }

      shmrtn = shmctl(shm_id,cmd,buff);
      if(shmrtn < 0){
	perror("shmctl error");
      }


#ifdef ANALYSIS
      fprintf(stdout,"audio frame clock: %f\n",(float)ts.audio_frame_clk/
	      (float)90000);
#endif 
      exit(1);
      /* end of termination routine */

    } else if(audio_active >0){
      if(audio_started > (N-audio_active)){
	sync_state = AUDIO_SYNC;
      }else{
	sync_state = INTERVAL_SYNC;
      }

    } else{
      sync_state = INTERVAL_SYNC;
    }
  } /* end if sync_state*/

  if((audio_active > 0)&&(audio_frames == 0)&&
     (master_state != STOP)&&(errno == EINTR)){
#ifdef ANALYSIS
    fprintf(stderr,"audio ended\n");
#endif
    frozen = TRUE;
    errno = 0;
  }
  /* reset frame count to 0 at end of each interval*/
  audio_frames = 0;

  if(signal(SIGALRM,sig_alarm) == SIG_ERR){
    perror("can't catch alarm signal");
  }
  return;
}
/*************************************************************************
*  Function:      sig_pipe()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       signal number
*                
*
*  Effects:      ends program if called by SIGPIPE
*                
*  Description:  If a SIGPIPE is received, end the process. This is
*  not enabled in the demux process since we expect to attempt
*  write to closed pipes                                  
**************************************************************************/

static void sig_pipe(int signo)
{
  fprintf(stderr,"pipe signal\n ");
  quit = TRUE;
  if(signal(SIGPIPE,sig_pipe) == SIG_ERR){
    perror("can't catch pipe signal");
  }
  return;
}
/*************************************************************************
*  Function:      sig_ctrlC2()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        signal number
*                
*
*  Effects:      removes shared memory and semophore and ends process
*                
*  Description:  The Audio mixer process has to remove the shared
*                memory and semophores upon termination                                  
**************************************************************************/

static void sig_ctrlC2(int signo)
{
  union semun arg;
  int semrtn;
  int shmrtn;

  quit = TRUE;

  semrtn = semctl(sem_id,0,IPC_RMID,arg);
  if(semrtn <0){
    perror("could not remove semaphore set");
  }

  shmrtn = shmctl(shm_id,IPC_RMID,NULL);
  if(shmrtn < 0){
    perror("shmctl error");
  }
}
/*************************************************************************
*  Function:      sig_ctrlC()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       signal number
*                
*
*  Effects:      set quit to TRUE
*                
*  Description:  Ends process.                                  
**************************************************************************/

static void sig_ctrlC(int signo)
{
  quit = TRUE;
}

/*************************************************************************
*  Function:      sig_usr1()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       signal number
*                
*
*  Effects:      video_active
*                
*  Description:  Receives video end signal from each video process
*                and decrements the number of video processes active.
*                This information is used to decide when the
*                player is finished.                  
**************************************************************************/

static void sig_usr1(int signo)
{
#ifdef ANALYSIS
  fprintf(stderr,"video ended\n ");
#endif

  video_active--;
  if(signal(SIGUSR1,sig_usr1) == SIG_ERR){
    perror("can't catch usr1 signal");
  }
  return;
}
/***********************************************************************/
/* global data buffer used to demultiplex stream*/
volatile char packet_data[BIG_BUFFER_SIZE];
int fd_mux[32][2];       /* file descriptors for pipes into audio mixer*/
int fd_demux[48][2];   /* file descriptors for pipes out of demultiplexer*/
int fdin;           /*file desc. of input data file*/
volatile pid_t pid[48];   /* saved values of child process pid's*/
volatile int N ; /* N audio processes */
volatile int M ; /* M video processes */
volatile AUDIO_info audio_info;
volatile AUDIO_info *adi;      /* pointer to audio info structure*/
volatile long audio_frame_clk; /* master time reference*/
volatile int frame_increment;  /* smallest increment of  audio time available*/
volatile AUDIO_device Audio_Device;
volatile AUDIO_device *audio_device; /* audio device specific structures*/
volatile int master_state;  /* set to either STOP or PLAY */
char filename[256];     /* X passes input filename via*/
char audiosavename[256];  /* X passes audio save filename via*/
long start_time;
volatile pid_t parent_pid; /* process id of main process->interface/mixer*/
volatile int audio_frames; /* count of audio frames written since last intrpt*/
volatile int shm_id,sem_id; /* IPCS identifiers*/
volatile TIME_stamps *time_stamps; /* pointer to shared memory structure*/
volatile int frozen;   /* set TRUE if audio_frames == 0 since last interrupt*/
volatile int sync_state;  /* can be AUDIO_SYNC or INTERVAL_SYSNC*/
volatile int video_active; /* M minus number of video streams ended*/
volatile int audio_active; /* N minus number of audio streams ended*/

volatile unsigned char raw_data_buf[MAX_DATA_BUF_LEN];  /* input demultiplex*/
volatile unsigned char *raw_data_buf_ptr;    /* buffer and pointer*/
volatile int raw_data_buf_len;     /* length of data in  buffer*/

/*
 * Global Widget variable declarations.
 */
Widget AppShell; /* The Main Application Shell */
Widget Shell000;
Widget MasterPlaybackControlWindow;
Widget Shell001;
Widget StreamPlaybackForm;

/* currently selected streams for manipulation by user */
int currentAudioStream;   /* used by X to pass parameters from buttons*/
int currentVideoStream;   /* same*/
Boolean saveAudioFile;    /* Did we want to save audio to a file?*/

/***********************************************************/

void
main(int argc,char *argv[])
{
  int current_audio; /* used to identify if packet is from a new*/
  int current_video; /* stream */
  int demux_id[48];  /* used to map stream #'s to pipe fd's*/
  int val;
  int status,i;
  char *audio_out;
  int bytes;
  char special[4];
  int j;
  int bytes_read[48];
  int bytes_written[48];
  char command[256];
  struct timeval interval1,zero;  /* interval timer structures*/
  struct itimerval timer1;
  struct itimerval otimer1;
  /* audio related */
  int mix_val;  
  char *out_buffer_ptr;
  int num_written;
  int astat;
  /* Demux declarations*/
  int  nr,x,y,c, c2, u, n, m;
  char next_code;
  /* used to pass info via the command line in exec of mpeg_play*/
  char x_pos[10];
  char y_pos[11];
  char title[256];
  char sem_string[20];
  char shm_string[20];
  char n_string[4];
  char m_string[4];
  
  char comm;
  /* shared Memory and Semaphore related*/
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

  /* X Windows related*/
  Display *display;
  Arg args[256];
  Cardinal argcnt;
  /*SUPPRESS 591*/
  Boolean argok;
  XtAppContext context;
  /* demultiplexer information structures*/
  PACK_header  pack_info;
  PACK_header *pack_info_p;
  SYSTEM_header  sys_info;
  SYSTEM_header *sys_info_p;
  PACKET_header  pkt_info;
  PACKET_header *pkt_info_p;
 
 /*INITIALIZATIONS*/
  frozen = FALSE;

  /*initialize pionters (saves having to malloc)*/
  pack_info_p = &pack_info;
  sys_info_p  = &sys_info;
  pkt_info_p  = &pkt_info;

  /*create 1 semaphore to control shared memory*/
  nsems = 1;                   /*one semaphore for now*/
  flag = (0600);    /*user read and alter*/
  key = IPC_PRIVATE;           /* force creation of a new semaphore*/

  sem_id = semget(key,nsems,flag);
  if(sem_id <0){
    perror("could not get semaphore");
  }

  /* initialize semaphores*/
  cmd = SETVAL;
  semnum = 0;
  arg.val = 1;

  semrtn = semctl(sem_id,semnum,cmd,arg);
  if(semrtn <0){
    perror("could not initialize semaphore ");
  }

  /* key and flag are still ok*/
  size = sizeof(struct Time_Stamps);
  /* create shared memory structure*/
  shm_id = shmget(key,size,flag);
  if(shm_id < 0){
    perror("shmget error");
  }
  /* attach shared memory structure to this process*/
  shmptr = (int *) shmat(shm_id,0,0);
  if(*shmptr < 0){
    perror("shmat error");
  }

  time_stamps = (TIME_stamps *)shmptr;

  raw_data_buf_len = 0;            /* required for demultiplexing*/
  raw_data_buf_ptr = raw_data_buf;

  audio_device = &Audio_Device;  /* saves having to malloc*/
  adi = &audio_info;
  
  /* initialize timer interval values*/
  interval1.tv_sec = 1;
  interval1.tv_usec = 000000;     /* interval = 1.0s */
  zero.tv_sec = 0;
  zero.tv_usec = 0;

  timer1.it_interval = interval1;
  timer1.it_value = interval1;
  otimer1.it_interval = zero;
  otimer1.it_value = zero;

  master_state = STOP;     /* ensure we wait for START ALL to be pressed*/
  currentAudioStream = 0;
  currentVideoStream = 0;
  saveAudioFile = False;

  /* initialize X Windows Application Context and Application Shell */

  XtToolkitInitialize();
  context = XtCreateApplicationContext();
  display = XtOpenDisplay(context, 0, APP_NAME, APP_CLASS, 
#ifndef XtSpecificationRelease
  0, 0, (Cardinal*)&argc, argv);
#else
#if XtSpecificationRelease == 4
  0, 0, (Cardinal*)&argc, argv);
#else
  0, 0, &argc, argv);
#endif
#endif
if(display == NULL)
{
  XtWarning("cannot open display");
  exit(1);
}

XtInitializeWidgetClass(applicationShellWidgetClass);

/*
 * The applicationShell is created as an unrealized
 * parent for multiple topLevelShells.  The topLevelShells
 * are created as popup children of the applicationShell.
 * This is a recommendation of Paul Asente & Ralph Swick in
 * _X_Window_System_Toolkit_ p. 677.
 */
argcnt = 0;
XtSetArg(args[argcnt], XmNx, 6); argcnt++;
XtSetArg(args[argcnt], XmNy, 6); argcnt++;
AppShell = XtAppCreateShell(APP_NAME, APP_CLASS,
			    applicationShellWidgetClass, display,
			    args, argcnt);

if(argc == 1){
  InitFileOpen(context);

  /*  Open the input file for reading of the data stream.  */
  if ((fdin = open (filename, O_RDONLY) ) < 0 )  {
    perror("Could not open input file.");
    exit(1);
  }
  argc = 2;

  /* handle user selected save to audio file */
  PopupSaveAudioQuestion(context);

  if (saveAudioFile == True) {
    InitAudioFileSave(context);

    if((adi->fd_audio_out = open(audiosavename,O_WRONLY|O_CREAT,0644)) < 0){
      perror("could not open audio output file.");
    }

    argc = 3;
  }

}else if((argc==2)||(argc == 3)){
  /*  Open the input file for reading of the data stream.  */
  if ((fdin = open (argv[1], O_RDONLY) ) < 0 )  {
    perror("Could not open input file.");
    exit(1);
  }
  if(argc==3){
    if((adi->fd_audio_out = open(argv[2],O_WRONLY|O_CREAT,0644)) < 0){
      perror("could not open audio output file.");
    }
  }
}else{
  fprintf(stderr,"USAGE: system_play inputfile  [audiofile]\n");
  exit(1);
}  /* end else*/


adi->argc = argc;

/* read first pack header*/
next_code = get_next_start_code();

if(next_code != (char)PACK_START_CODE){
  fprintf(stderr,"%s: Not an MPEG I System stream!\n",argv[1]);
  exit(1);
}

read_pack_header(pack_info_p);

#ifdef ANALYSIS
print_pack_header(pack_info_p);
#endif

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
/* initlaize shared memory values*/
time_stamps->start_SCR = pack_info_p->SCR;
time_stamps->SCR = pack_info_p->SCR;
time_stamps->audio_frame_clk = 0;
for(i=0;i<48;i++){
  time_stamps->started[i] = FALSE;
}

/* release semaphore*/
semoparray[0].sem_op = 1;        /*release the semaphore*/
semrtn = semop(sem_id, semoparray,nops);
if(semrtn <0){
  perror("could not  release");
}

/* read first system header */ 
next_code = get_next_start_code(fdin);
if(next_code != (char)SYSTEM_START_CODE){
  fprintf(stderr,"%s: No system header present\n",argv[1]);
  exit(1);
}

read_system_header(sys_info_p);

#ifdef ANALYSIS
print_system_header(sys_info_p);
#endif

/* at this point we know how many audio and video streams are coming*/
N = sys_info_p->audio_bound;
M = sys_info_p->video_bound;
adi->N = N;
adi->master_volume = 1;  /* audio mixer volume default*/

#ifdef ANALYSIS  
fprintf(stderr," %d audio %d video\n",N,M);
#endif

video_active = M;
current_audio = 0;
current_video = N;

/*initialize synchronization state*/
if(N==0){
  sync_state = INTERVAL_SYNC;
}else{
  sync_state = AUDIO_SYNC;
}

for(i=0;i<48;i++){  /* -1 means uninitialized*/
  demux_id[i] = -1;
}

/* initialize bytes read */
for(i=0;i<N+M;i++){
  bytes_read[i] = 1;
  bytes_written[i] =1;
}

/*set all audio streams to inactive*/
for(i=0;i<N;i++){
  adi->pstatus[i] = INACTIVE;
}
/*set all video streams to active*/
for(i=N;i<(N+M);i++){
  adi->pstatus[i] = PLAY;
}

/*ensure we are not clipping the 16 bit rate to start*/
for(i=0;i<32;i++){
  adi->audio_weight[i] = -N;
}

/* open pipes from system demultiplexer to audio decoders */

for(i=0;i<N;i++){
  if (pipe(fd_demux[i])<0){
    perror("pipe error");
    exit(1);
  }
}
/* open pipes to funnel stdout into audio mixer*/
for(i=0;i<N;i++){
  if(pipe(fd_mux[i]) < 0){
    perror("pipe error");
    exit(1);
  }
}

/* start up N audio processes */ 
for(i=0;i<N;i++){
  if ( ( pid[i] = fork()) < 0) {
    perror("fork error");
    exit(1);
  } else if (pid[i] == 0){     /* child */  
    close(2); 
    close(fd_demux[i][1]);
    close(fd_mux[i][0]);
    /* open pipe from main to maplay */
    if(fd_demux[i][0] != STDIN_FILENO){
      if(dup2(fd_demux[i][0],STDIN_FILENO) != STDIN_FILENO){
	perror("dup2 error");
	exit(1);    
      }
      close(fd_demux[i][0]);
    }
    /* open pipe from maplay to audio mixer */
    if(fd_mux[i][1] != STDOUT_FILENO){
      if(dup2(fd_mux[i][1],STDOUT_FILENO) != STDOUT_FILENO){
	perror("dup2 error");
	exit(1);
      }
      close(fd_mux[i][1]);
    }
/* exec maplay using stdin and stdout modes forcing mono output*/    
    if (execlp("maplay2", "maplay2","-s","-","-l", (char *) 0) < 0){
      perror(" execlp error");
      exit(1);
    }
  } else {                 /* parent */
    close(fd_demux[i][0]);
  
  } /* end start up N audio processes */  
} /* end for loop N */
/* start up M video processes */
x = 100;
y = 200;

for(i=N;i<(N + M);i++){
  if (pipe(fd_demux[i])<0){
    perror("pipe error");
    exit(1);
  }
  if ( ( pid[i] = fork()) < 0) {
    perror("fork error");
    exit(1);
  } else if (pid[i] == 0){                 /* child */
    close(fd_demux[i][1]);
    if(fd_demux[i][0] != STDIN_FILENO){
      if(dup2(fd_demux[i][0],STDIN_FILENO) != STDIN_FILENO){
	perror("dup2 error");
	exit(1);    
      }
      close(fd_demux[i][0]);
    }    
    /* initialize command line parameters*/
    sprintf(x_pos,"%d",x);
    sprintf(y_pos,"%d",y);
    sprintf(title,"%d_Video",(i-N));
    sprintf(shm_string,"%d",shm_id);
    sprintf(sem_string,"%d",sem_id);
    sprintf(n_string,"%d",N);
    sprintf(m_string,"%d",M);

/* exec mpeg_play in stdin mode with realtime switch and some fluff*/
    if (execlp("mpeg_play2", "mpeg_play2","-realtime", "-title",
	       title,"-shm",shm_string,sem_string,"-process",n_string,m_string,
	       "-position", x_pos, y_pos,"-dimension",
	       "160", "112", (char *) 0) < 0){
      perror(" execlp error");
      exit(1);
    }
    x += 100;
    y += 100;

  } else {     /* parent */
    close(fd_demux[i][0]);
  }
} /* end for loop M */

/* fork off demux process */
if((pid[N+M] = fork()) < 0) {
  perror("fork error");
  exit(1);
} else if (pid[N+M] > 0) {        /* parent */

/*nothing for now*/

} else {                         /* child */


     /* set control C capture */
     if(signal(SIGINT,sig_ctrlC) == SIG_ERR){
       perror("can't catch control C");
   }
     if(signal(SIGPIPE,SIG_IGN) == SIG_ERR){ /* ignore pipe error and*/
       perror("can't ignore sig pipe");   /* dump data meant for terminated*/
   }                                      /* processes*/

     if(signal(SIGALRM,SIG_IGN) == SIG_ERR){
       perror("can't ignore alarm signal");
   }

     /*Read and write data.                   */
     while((quit == FALSE)&&(next_code != ISO_11172_END_CODE)){
       next_code = get_next_start_code();
     switch(next_code){
       case((char)PACK_START_CODE):      /*0xBA*/
/*initialize the SCR value in ahared memory to zero*/
	 pack_info_p->SCR = 0;
       read_pack_header(pack_info_p);
       /* set semaphore */
       nops = 1;
       semoparray[0].sem_op = -1;        /*set the semaphore*/
       semrtn = semop(sem_id, semoparray,nops);
       if(semrtn <0){
	 perror("could not  set");
       }
       time_stamps->SCR = pack_info_p->SCR;
       semoparray[0].sem_op = 1;        /*release the semaphore*/
       semrtn = semop(sem_id, semoparray,nops);
       if(semrtn <0){
	 perror("could not  release");
       }
       break;
       case((char)SYSTEM_START_CODE):      /*0xBB*/
	 read_system_header(sys_info_p);
       break;
       case((char)ISO_11172_END_CODE):      /*0xB9*/
	 fprintf(stderr,"end of system stream reached\n");
       break;
     default:   /*PACKET START CODE & stream_id*/
       pkt_info_p->stream_id = next_code;      
       read_packet_header(pkt_info_p);

       i = (((int)pkt_info_p->stream_id)&0xFF)  - 192;
       /*convert C0 to 0 and EF to 47*/  

       if((i< 0)||(i>=48)){
         /* do nothing invalid*/
       }else  if(demux_id[i] == -1){        /* a new stream*/
	 if(i<32){        /*audio*/
	   demux_id[i] = current_audio;
	   current_audio++;
	 } else {
	   demux_id[i] = current_video;
	   current_video++;
	 }	     
	 j= demux_id[i];
	 /* set semaphore */
	 nops = 1;
	 semoparray[0].sem_op = -1;        /*set the semaphore*/
	 semrtn = semop(sem_id, semoparray,nops);
	 if(semrtn <0){
	   perror("could not  set");
	 }
	 if(pkt_info_p->PTS != 0){
	   time_stamps->PTS[j] = pkt_info_p->PTS;
	   time_stamps->start_PTS[j] = pkt_info_p->PTS;
	 }
	 if(pkt_info_p->DTS != 0){
	   time_stamps->DTS[j] = pkt_info_p->DTS;
	   time_stamps->start_DTS[j] = pkt_info_p->DTS;
	 }
	 time_stamps->started[j] = TRUE;
	 semoparray[0].sem_op = 1;        /*release the semaphore*/
	 semrtn = semop(sem_id, semoparray,nops);
	 if(semrtn <0){
	   perror("could not  release");
	 }
	             /* write packets to pipes*/
	 c2 = -10;
	 while ((c2 != pkt_info_p->packetlength)&&(quit == FALSE)){
	   c2 = write(fd_demux[j][1], pkt_info_p->packet_ptr, 
		      pkt_info_p->packetlength);
	   if(c2 <0){
	     break;
	   }
	 }
       } else {                     /* a valid active stream*/
	 j= demux_id[i];
	 
	 /* set semaphore */
	 nops = 1;
	 semoparray[0].sem_op = -1;        /*set the semaphore*/
	 semrtn = semop(sem_id, semoparray,nops);
	 if(semrtn <0){
	   perror("could not  set");
	 }

	 if(pkt_info_p->PTS != 0){
	   time_stamps->PTS[j] = pkt_info_p->PTS;
	 }
	 if(pkt_info_p->DTS != 0){
	   time_stamps->DTS[j] = pkt_info_p->DTS;
	 }
	 semoparray[0].sem_op = 1;        /*release the semaphore*/
	 semrtn = semop(sem_id, semoparray,nops);
	 if(semrtn <0){
	   perror("could not  release");
	 }

	 c2 = -10;
         /* write packets to pipes*/
	 while ((c2 != pkt_info_p->packetlength)&&(quit == FALSE)){

	   c2 = write(fd_demux[j][1], pkt_info_p->packet_ptr, 
		      pkt_info_p->packetlength);

	   if(c2 <0){
	     break;
	   }
	   if((c2 != pkt_info_p->packetlength)&&(c2 != -1)){
	     fprintf(stderr," short write: %d %d\n",c2,
		     pkt_info_p->packetlength);
	   }
	 }
       }
     }  /* end switch(next_code) */
   }  /* end while */

#ifdef ANALYSIS
     fprintf(stderr,"detach  shared memory \n");
#endif

     shmrtn = shmdt((void *)time_stamps);
     if(shmrtn < 0){
       perror("shmdt error");
   }

 /* close input file*/
     close(fdin);
/* close demux pipes*/

     for(i=0;i<(N+M);i++){
  close(fd_demux[i][1]);
}
exit(0);

} /*end fork demux process*/


/* main turns into mixer/interface process*/

/* set all pipes to non-blocking*/
  for(i=0;i<N;i++){
    close(fd_mux[i][1]);
    /* set all fd_mux pipes to non-blocking*/
    if((val = fcntl(fd_mux[i][0],F_GETFL,0))<0){
      perror("fcntl error");
    }
    val |= O_NONBLOCK;
    if(fcntl(fd_mux[i][0],F_SETFL,val)<0){
      perror("fcntl error");
      exit(1);
    }
  }
  if(signal(SIGPIPE,sig_pipe) == SIG_ERR){
    perror("can't catch pipe  signal");
  }
  if(signal(SIGUSR1,sig_usr1) == SIG_ERR){
    perror("can't catch usr1");
  }
  /* set signal handler for rtc alarm */
  if(signal(SIGALRM,sig_alarm) == SIG_ERR){
    perror("can't catch signal");
  }
  /* set signal handler for terminal interrupt(delete key)*/
  if(signal(SIGINT,sig_ctrlC2) == SIG_ERR){
    perror("can't ignore terminal interrupt");
  }

  audio_frame_clk = 0;

#ifdef INDIGO
  frame_increment = 90000/(48000/OUTBUFFSIZE);
#endif
#ifdef SPARC1PX
  frame_increment = 90000/(8000/OUTBUFFSIZE);
#endif
#ifdef LINUX
  frame_increment = 90000/(44100/OUTBUFFSIZE);
#endif

  /* open an audio port and configure it:*/
  init_audio(audio_device);


  /*** start user interface with control windows ***/

/*
 * Install the tearOffModel resource converter.
 */
#if (XmVersion >= 1002)
XmRepTypeInstallTearOffModelConverter();
#endif
/*
 * Register special BuilderXcessory converters.
 */
RegisterBxConverters(context);

XtInitializeWidgetClass(topLevelShellWidgetClass);
argcnt = 0;
XtSetArg(args[argcnt], XmNtitle, "MASTER PLAYBACK CONTROL"); argcnt++;
XtSetArg(args[argcnt], XmNiconName, "Master Playback Control"); argcnt++;
XtSetArg(args[argcnt], XmNx, 6); argcnt++;
XtSetArg(args[argcnt], XmNy, 6); argcnt++;
Shell000 = XtCreatePopupShell( "masterPlaybackControl", 
			      topLevelShellWidgetClass, AppShell,
			      args, argcnt);
MasterPlaybackControlWindow = CreatemasterPlaybackControlWindow(Shell000);
XtInitializeWidgetClass(topLevelShellWidgetClass);
argcnt = 0;
XtSetArg(args[argcnt], XmNtitle, "AUDIO/VIDEO SINGLE STREAM CONTROL");
argcnt++;
XtSetArg(args[argcnt], XmNiconName, "Stream Playback Control"); argcnt++;
XtSetArg(args[argcnt], XmNx, 660); argcnt++;
XtSetArg(args[argcnt], XmNy, 1); argcnt++;
Shell001 = XtCreatePopupShell( "streamPlaybackControl", 
			      topLevelShellWidgetClass, AppShell,
			      args, argcnt);
StreamPlaybackForm = CreatestreamPlaybackForm(Shell001);
XtManageChild(MasterPlaybackControlWindow);
XtPopup(XtParent(MasterPlaybackControlWindow), XtGrabNone);
XtManageChild(StreamPlaybackForm);
XtPopup(XtParent(StreamPlaybackForm), XtGrabNone);

/* get current time */
time(&start_time);
/* start realtime interrupt mechanism*/
if( setitimer(ITIMER_REAL, &timer1,&otimer1) < 0){
  perror("set interval error");
} 
/* call audio mixer process as many times as possible*/
     XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

/* Let 'er rip*/
     XtAppMainLoop(context);
     

}   /* end main */



