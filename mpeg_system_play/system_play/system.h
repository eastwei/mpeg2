/*************************************************************************
*  File Name:     system.h
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*  Usage:         header file
*
*  Description:   This header file declares the function prototypes
*  and external varible declarations used in the system_play modules.
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
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <Xm/Xm.h>

/*#define ANALYSIS */

#ifdef INDIGO
#include <audio.h>
#endif
#ifdef SPARC1PX        /* indicates the 8 bit U Law AMD device*/
#include <sun/audioio.h>
#endif
#ifdef LINUX        /* indicates the 8 bit U Law AMD device*/
#include <linux/soundcard.h>
#endif

#define MAX_DATA_BUF_LEN 100000   /* size of input buffer array*/
#define OUTBUFFSIZE 500         /* number of audio samples read at a time*/
#define RTC_INCREMENT 90000   /*90000 Hz * 1 s interval */

#define FALSE 0
#define TRUE 1

/* stream state variables*/
#define ACTIVE 0
#define INACTIVE 1
#define PLAY 2
#define PAUSE 3   /* video only. use mute for audio*/
#define CANCEL 4
#define MUTE 3  /* same as pause, but for audio*/
#define STOP 6

/*synchronization state variable*/
#define INTERVAL_SYNC 1
#define AUDIO_SYNC    2

#define START_CODE_LENGTH         4  /*Bytes*/
#define PACK_HEADER_LENGTH        8  /*Bytes*/
#define SYSTEM_HEADER_LENGTH     14  /*Bytes*/
#define PACKET_LENGTH_LENGTH      2  /*Bytes*/
#define BIG_BUFFER_SIZE       10000  /*Bytes*/
#define READ_SIZE              2286  /*Bytes in group to write to file in each iteration  */
#define READ_QTY                  1  /*Loop iterations of read & write  */
#define PERMS                  0666  /*File permissions */
#define PACK_START_CODE    1         /* ISO stream codes seen*/
#define SYSTEM_START_CODE  2
#define ISO_11172_END_CODE 3

/* System Layer Time stamp structure*/
typedef struct Time_Stamps{
  volatile long SCR;               /*SYSTEM CLOCK REFERENCE*/
   volatile long DTS[48];           /* Decoded Time Stamps - one for each stream seen*/
   volatile long PTS[48];           /* Presentation Time Stamps - one for each*/
   volatile long start_SCR;         /* first SCR seen*/
   volatile long start_PTS[48];     /* first PTS seen - needed for offset operations*/
   volatile long start_DTS[48];
   volatile long audio_frame_clk;   /* audio frame time used for master sync*/
   volatile int  started[48];       /* boolean states if a stream has been seen in demux*/
   volatile int  mixer_pid;         /* video processes need pid of mixer for ipc*/
}TIME_stamps;

/* Stream level information on required buffer size*/
typedef struct Stream{
   volatile char stream_id;
   volatile int STD_buffer_bound_scale;      /*defined in ISO 11172*/
   volatile int STD_buffer_size_bound;
}STREAM;

/* Pack header information structure*/
typedef struct Pack_header{
   volatile long SCR;
   volatile int mux_rate;                     /*defined in ISO 11172*/
}PACK_header;

/* System header information structure*/
typedef struct System_header{
   volatile int header_length;
   volatile int rate_bound;                 /*defined in ISO 11172*/
   volatile int audio_bound;
   volatile int fixed_flag;
   volatile int CSPS_flag;
   volatile int system_audio_lock_flag;
  volatile  int system_video_lock_flag;
  volatile  int video_bound;
  volatile  STREAM STD_buffer_info[48];
}SYSTEM_header;

/* Packet header information structure*/
typedef struct Packet_header{
   volatile char stream_id;
   volatile int packetlength;
   volatile int STD_buffer_scale;    /*defined in ISO 11172*/
   volatile int STD_buffer_size;
   volatile long PTS;
   volatile long DTS;
   char *packet_ptr;
}PACKET_header;

/* Audio info structure*/
typedef struct Audio_info{
  volatile  int frequency[32];
  volatile  int channels[32];
  volatile  int N;
  volatile  int argc;
  volatile  int pstatus[48];    /* both audio and video status*/
  volatile  short out_buffer[32][OUTBUFFSIZE];
   short common_buffer[OUTBUFFSIZE];
#ifdef INDIGO
   short mix_buffer[OUTBUFFSIZE];
#endif
#ifdef LINUX
   short mix_buffer[OUTBUFFSIZE];
#endif
#ifdef SPARC1PX
   unsigned char mix_buffer[OUTBUFFSIZE];
#endif
#ifdef NONE
   short mix_buffer[OUTBUFFSIZE];
#endif
  volatile int audio_weight[32];
  volatile int master_volume;
  volatile int num_read[32] ;
   volatile int fd_audio_out;  
}AUDIO_info;

/* Audio device specific structure*/
typedef struct Audio_device{
  volatile  int      frequency;
  volatile  int      channels;
#ifdef INDIGO
  volatile  ALport   port;
  volatile  ALconfig config;
  long pvbuffer[2];
#endif
#ifdef SPARC1PX
   volatile int fd_audio;
#endif
#ifdef LINUX
   volatile int fd_audio;
#endif
}AUDIO_device;

/* Function Prototypes*/
/* audio.c */
extern void init_audio(volatile AUDIO_device *);
#ifdef INDIGO
extern int write_audio(volatile AUDIO_device *,short *, long);
#endif
#ifdef LINUX
extern int write_audio(volatile AUDIO_device *,short *, long);
#endif
#ifdef SPARC1PX
extern int write_audio(volatile AUDIO_device *, unsigned char *,long);
#endif
extern void end_audio(volatile AUDIO_device *);
extern void resize_array_width(short *,int,int,short *,int);
extern Boolean  do_audio(XtAppContext);
/* demux.c */
extern char get_next_start_code();
extern void read_pack_header( PACK_header *);
extern void read_system_header(SYSTEM_header *);
extern void read_packet_header( PACKET_header *);
extern void print_pack_header(PACK_header *);
extern void print_system_header(SYSTEM_header *);
extern void print_packet_header(PACKET_header *);

/* Global variables (explained in system.c)*/
 volatile extern char packet_data[BIG_BUFFER_SIZE];
 extern long start_time;
 volatile extern unsigned long rtc_value;
 volatile extern int quit;
 extern int fd_mux[32][2];
 extern int fd_demux[48][2];
 extern int fdin;
 volatile extern int N;
 volatile extern pid_t pid[48];
 volatile extern int M;
 volatile extern AUDIO_info audio_info;
 volatile extern AUDIO_info *adi;
 volatile extern int frame_increment;
 volatile extern AUDIO_device Audio_Device;
 volatile extern AUDIO_device *audio_device;
 volatile extern int master_state;
 extern char filename[256];
 extern char audiosavename[256];
 volatile extern pid_t parent_pid;
 volatile extern int audio_frames;
 volatile extern int sem_id;
 volatile extern int shm_id;
 volatile extern TIME_stamps *time_stamps;
 volatile extern int frozen;
 volatile extern int sync_state;
 volatile extern int video_active;
 volatile extern int audio_active;
