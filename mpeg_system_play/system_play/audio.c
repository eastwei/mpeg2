/*************************************************************************
*  File Name:     audio.c
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*  Usage:         functions called from system.c
*
*  Description:   This file contains the audio hardware specific
*  parts of the program.  The only execeptions to this are
*  the call to 'exec' maplay in system.c and the audio structure
*  declarations in system.h.  Three functions  provide for
*  the initialization, mixing,feeding, and closure of the audio device.
*  In addition, one function shifts each of three input frequencies 
*  to a common output frequency before mixing input streams.
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
v* ON AN "AS IS" BASIS, AND BOSTON UNIVERSITY HAS NO OBLIGATION TO
* PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include "system.h"
#ifdef SPARC1PX
#include "ulaw.h"
#endif

/*************************************************************************
*  Function:      init_audio()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       Audio device info structure pointer
*                
*
*  Effects:      The audio device and the Audio info structure
*                
*  Description:  This function initializes audio device.                                  
**************************************************************************/
#ifdef INDIGO
void init_audio(volatile AUDIO_device *ad){

  ad->frequency = 48000;  /* we shift data to highest common frequency*/
  ad->channels = 1;       /* forced to use mono due to the low*/
                          /* data rate of pipes*/
  ad->pvbuffer[0] =  AL_OUTPUT_RATE;
  ad->pvbuffer[1] = 0;

  if (!(ad->config = ALnewconfig ()))
    {
      perror("ALnewconfig failed!");
      exit (1);
    }
  ALsetwidth (ad->config, AL_SAMPLE_16);
  if (ad->channels == 1)
    ALsetchannels (ad->config, AL_MONO);
  else
    ALsetchannels (ad->config, AL_STEREO);
  
  ALsetqueuesize(ad->config, 10000);/* small output buffer size aids sync*/

  if (!(ad->port = ALopenport ("MPEG audio player", "w", ad->config)))
    {
      perror("can't allocate an audio port!");
      exit (1);
    }

  /* set sample rate: */
  ad->pvbuffer[1] = ad->frequency;
  ALsetparams (AL_DEFAULT_DEVICE, ad->pvbuffer, 2);
  ALfreeconfig (ad->config);
  return;
}
#endif

#ifdef SPARC1PX
void init_audio(volatile AUDIO_device *ad){

ad->fd_audio = open("/dev/audio", O_WRONLY|O_NDELAY);
if(ad->fd_audio <0){
 perror("couldn't open /dev/audio");
}
  return;
}
#endif

#ifdef LINUX
void init_audio(volatile AUDIO_device *ad){
int flags;
  int play_precision = 16;
  int play_stereo = 0;  /*mono only*/
  int play_sample_rate = 44100;

  ad->fd_audio = open("/dev/dsp", O_WRONLY|O_NDELAY,0);

  if(ad->fd_audio == EBUSY){
    perror("audio device busy");
    exit(1);
  }

  if(ad->fd_audio <0){
    perror("couldn't open /dev/dsp");
    exit(1);
  }

 /* turn NDELAY mode off:*/

  if ((flags = fcntl (ad->fd_audio, F_GETFL, 0)) < 0)
  {
    perror ("fcntl F_GETFL on /dev/dsp failed");
    exit (1);
  }
  flags &= ~O_NDELAY;
  if (fcntl (ad->fd_audio, F_SETFL, flags) < 0)
  {
    perror ("fcntl F_SETFL on /dev/dsp failed");
    exit (1);
  }
  if(
      ioctl(ad->fd_audio, SNDCTL_DSP_SAMPLESIZE, &play_precision) == -1 ||
      ioctl(ad->fd_audio, SNDCTL_DSP_STEREO, &play_stereo) == -1 ||
      ioctl(ad->fd_audio, SNDCTL_DSP_SPEED, &play_sample_rate) == -1
    )
  {
    perror ("configuration of /dev/dsp failed");
    exit (1);
  }
 
  return;
}
#endif

#ifdef NONE
void init_audio(volatile AUDIO_device *ad){

  return;
}
#endif




/*************************************************************************
*  Function:      write_audio()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       Audio device info structure pointer
*                mixed data buffer pointer
*                number of samples to write(16 bit)
*  Effects:      The audio device and its external buffer
*
*  Description:  Writes data to audio port
**************************************************************************/
#ifdef INDIGO
int write_audio(volatile AUDIO_device *ad, short *buffer, long bytes)
{
  int val;

  val =  ALwritesamps (ad->port, buffer, bytes);
  return val;
}
#endif
#ifdef SPARC1PX
int write_audio(volatile AUDIO_device *ad, unsigned char *buffer, long bytes)
{
  int val;

  val =  write(ad->fd_audio, buffer, bytes);
  return val;
}
#endif
#ifdef LINUX
int write_audio(volatile AUDIO_device *ad, short *buffer, long bytes)
{
  int val;

  val =  write(ad->fd_audio, (char *)buffer, 2*bytes);
  return val;
}
#endif




/*************************************************************************
*  Function:      end_audio()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       Audio device info structure pointer
*            
*         
*  Effects:      The audio device and its external buffer
*
*  Description:  finished playing leftover data in audio port
**************************************************************************/

void end_audio(volatile AUDIO_device *ad)
{ 
#ifdef INDIGO
  while (ALgetfilled (ad->port) > 0){
    sleep (1);
  }
  ALcloseport (ad->port);
#endif
#ifdef LINUX
 sleep(1);
 close(ad->fd_audio);
#endif
  return;
}

/*************************************************************************
*  Function:      Resize_Array_Width()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       input and output buffer pointers
*                input and output widths
*                input array height(always 1 here)
*
*  Effects:      shifts input audio frequency up or down as required
*
*  Description:   This function will resize any array width, up
* or down, in size.  The algorithm is based on the
* least common multiple approach. For this funtion, it is used to
* shift audio frequencies.
*=====================================================*/

void 
resize_array_width(short *inarray,int in_x,int in_y,short *outarray,
                  int out_x)
{
  int i,j; 
  int in_total;
  int out_total;
  short *inptr;
  short *outptr;
  short pointA,pointB;

  inptr = &inarray[0];
  outptr = &outarray[0];
  in_total = 0;
  out_total = 0;
  for(j=0;j<out_x;j++){      /* For every output value */
    if(in_total == out_total){  
      *outptr = *inptr;
      outptr++;
      out_total=out_total+in_x;
      while(in_total < out_total){
        in_total = in_total + out_x;
        inptr++;
      }
      if(in_total > out_total){
        in_total = in_total - out_x;
        inptr--;
      }
    } else {  
      pointA = *inptr;
      inptr++;
      pointB = *inptr;
      inptr--;
      *outptr = *inptr;  

      outptr++;
      out_total=out_total+in_x;
      while(in_total < out_total){
        in_total = in_total + out_x;
        inptr++;
      }
      if(in_total > out_total){
        in_total = in_total - out_x;
        inptr--;
      }
    }  /* end if */
  }  /* end for each output value */


}  /* end  */

/*************************************************************************
*  Function:      do_audio()
*  Creation Date: 12 Aug 1994
*  Author:        James Boucher
*                 email  jboucher@flash.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:       Xt Applications Context
*                
*
*  Effects:      reads input pipes, mixes data, and then writes
*                it to the audio device and/or a file.
*
*
*  Description:  This function is a Xt Work Process that is called as
*                often as possible from the audio mixer/user interface
*                process. For this reason, lots of mouse movements and
*                user interaction can degrade audio output quality.
**************************************************************************/
#ifdef INDIGO

Boolean do_audio(XtAppContext context)
{
  int nr;
  int mix_val;
  int bytes;
  int num_written;
  int astat,end;
  int i,j,val;
  char special[4];
  long end_time;
  long elapse;
  /*shared mem and semaphore varaibles*/
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
  TIME_stamps ts;
  long temp1;
  int audio_started,video_started;
  int statptr;
  int stat_pid;

/* if we are either stopped or no audio streams are active*/
/* or are trying to quit, return without writing audio data*/
  if((master_state == STOP)||(sync_state == INTERVAL_SYNC)||(quit==TRUE)){
    XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

    return(TRUE);  /* call Work Process agian*/
  }


  /*collect time stamps*/
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

  ts.SCR = time_stamps->SCR;
  ts.start_SCR = time_stamps->start_SCR;

  if((sync_state == AUDIO_SYNC)&&(master_state != STOP)){
    time_stamps->audio_frame_clk += frame_increment;
    ts.audio_frame_clk = time_stamps->audio_frame_clk;
  }

  for(i=0;i<N+M;i++){

    ts.PTS[i] = time_stamps->PTS[i];
    ts.DTS[i] = time_stamps->DTS[i];
    ts.start_PTS[i] = time_stamps->start_PTS[i];
    ts.start_DTS[i] = time_stamps->start_DTS[i];
    ts.started[i] = time_stamps->started[i];

  }

  /* release semaphore*/
  semoparray[0].sem_op = 1;        /*release the semaphore*/
  semrtn = semop(sem_id, semoparray,nops);
  if(semrtn <0){
    perror("could not  release");
  }

  /*evaluate sync state*/
  /* calculate number of currently active audio channels*/
  audio_active = N;
  for(i=0;i<N;i++){
    if(adi->pstatus[i]== CANCEL){
      audio_active--;
    }
  }

  /*video active maintained globally by interrupts*/

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

#ifdef ANALYSIS
    fprintf(stderr,"ending audio mixer\n");
#endif

    end_audio(audio_device);

    close(adi->fd_audio_out);

#ifdef ANALYSIS
    fprintf(stderr,"detach and remove shared memory and semaphore\n");
#endif

    shmrtn = shmdt((void *)time_stamps);
    if(shmrtn < 0){
      perror("shmdt error");
    }
    cmd = IPC_RMID;
    semnum = 0;
    
    semrtn = semctl(sem_id,semnum,cmd,arg);  /* remove semophore*/
    if(semrtn <0){
      perror("could not remove semaphore set");
    }

    shmrtn = shmctl(shm_id,cmd,buff); /* remove shared memory*/
    if(shmrtn < 0){
      perror("shmctl error");
    }

#ifdef ANALYSIS
    fprintf(stdout,"audio frame clock: %f\n",(float)ts.audio_frame_clk/
                                           (float)90000);
#endif

    exit(0);

    /* end of termination routine */

  } else if(audio_active >0){    /* if currently playing audio, use */
    if(audio_started > (N-audio_active)){ /* audio frame time*/
      sync_state = AUDIO_SYNC;
    }else{
      sync_state = INTERVAL_SYNC; /* if no audio active, use interval*/
    }                             /* time to adjust audio frame clock*/

  } else{
    sync_state = INTERVAL_SYNC;
  }

  for(i=0;i<adi->N;i++){       /* for each of the audio streams*/



    stat_pid =waitpid(pid[i],&statptr,WNOHANG);
    if(stat_pid < 0){
      adi->pstatus[i] = CANCEL;
    }


    frozen = FALSE;          /* reset frozen*/
    if(adi->pstatus[i] == INACTIVE){   /* audio streams inactive if not*/
      nr= -1;                 /* yet initialized*/
      while((nr == -1)&&(errno != EAGAIN)){   /* do a non-blocking read*/
	nr = read(fd_mux[i][0],special, 4);  /* to see if initialization parameters*/
      }                                 /* have been sent*/
      errno = 0;
      if(nr ==4){   /* we have received initilization info*/


if(N==1){
	/* turn pipe to blocking*/
	if((val = fcntl(fd_mux[i][0],F_GETFL,0))<0){
	  perror("fcntl error");
	  exit(1);
	}
	val ^= O_NONBLOCK;
	if(val = fcntl(fd_mux[i][0],F_SETFL,val)<0){
	  perror("fcntl error");
	  exit(1);
	}
      }
	adi->pstatus[i] = PLAY;  /* set process status to PLAY*/

	if(special[0] == 'A'){    /* store audio data rate */

	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}else if(special[0] =='B'){

	  adi->frequency[i] = 44100;
	  adi->channels[i] = 1;
	}else if(special[0] == 'C'){
	  adi->frequency[i] = 48000;
	  adi->channels[i] = 1;
	}else{
	  fprintf(stderr,"bad frequency\n");
	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}


/* do initial read from pipes*/
	adi->num_read[i] = -10;    /* dummy set to ensure loop done at least once*/

	if((adi->frequency[i] == 32000)&&(adi->channels[i] == 1)){
	  bytes = (4*OUTBUFFSIZE)/3;

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i] < 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->common_buffer,bytes);
	  }
/* shift frequency to 48000 hz*/
	  resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			     (short *)adi->out_buffer[i],
			     OUTBUFFSIZE);

	}else if((adi->frequency[i] == 44100)&&(adi->channels[i] == 1)){
	  bytes = (OUTBUFFSIZE*441)/240;

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i]< 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->common_buffer,bytes);
	  }

	  resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			     (short *)adi->out_buffer[i],
			     OUTBUFFSIZE);

	}else{        

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i] < 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->out_buffer[i],2*OUTBUFFSIZE);
	  } 
	}

      }  /*end if nr*/

    } else { /* PLAY*/

      adi->num_read[i] = -10;

      if((adi->frequency[i] == 32000)&&(adi->channels[i] == 1)){

	bytes = (4*OUTBUFFSIZE)/3;
	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&
	      (adi->pstatus[i] != CANCEL)){


	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->common_buffer,bytes);
         

	}

	resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			   (short *)adi->out_buffer[i],
			   OUTBUFFSIZE);

      }else if((adi->frequency[i] == 44100)&&(adi->channels[i] == 1)){
	bytes = (OUTBUFFSIZE*441)/240;

	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&
	      (adi->pstatus[i] != CANCEL)){

	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->common_buffer,bytes);

	}

	resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			   (short *)adi->out_buffer[i],
			   OUTBUFFSIZE);

      }else{        

	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&(adi->pstatus[i] != CANCEL)){

	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->out_buffer[i],2*OUTBUFFSIZE);

	} 
      }

      if((frozen==TRUE)||(adi->num_read[i]==0)){
	fprintf(stderr,"scr %d pts %d afc %d\n",ts.SCR,ts.PTS[i]
		,ts.audio_frame_clk);
	if(ts.SCR > ts.audio_frame_clk){ /* check to ensure no false call*/
	  adi->pstatus[i] = CANCEL;
	  kill(pid[i],SIGINT);
	}else{   /*set to non-blocking*/

	}
	frozen = FALSE;
      }
      
    } /* end else for INACTIVE/PLAY*/
  }  /* for loop */

  /* Mix data ensuring muted channels skipped*/
     /* stream level audio controls also implemented*/
  for(i=0;i< OUTBUFFSIZE;i++){
    mix_val = 0;
    for(j=0;j<adi->N;j++){
      if(adi->pstatus[j] == PLAY){
	if(adi->audio_weight[j] >0){
	  mix_val += ((int) adi->out_buffer[j][i])*adi->audio_weight[j];
	}else if(adi->audio_weight[j] < 0){
	  mix_val += ((int) adi->out_buffer[j][i])/(-adi->audio_weight[j]);
	}else{
	  mix_val += ((int) adi->out_buffer[j][i]);
	}
      }
    }
    if(adi->master_volume < 0){
      adi->mix_buffer[i] = (short)(mix_val/adi->master_volume);
    }else if(adi->master_volume > 0){
      adi->mix_buffer[i] = (short)(mix_val*adi->master_volume);
    }else{
      adi->mix_buffer[i] = (short)(mix_val);
    }
  }
  /* Write data to audio port  */
  astat = write_audio(audio_device, adi->mix_buffer, (long)OUTBUFFSIZE);
  if(astat < 0){
    perror("audio mixer write error");
    exit(1);
  }
   /* As well, save to file if requested*/
  if(adi->argc ==3){
    num_written = write(adi->fd_audio_out,adi->mix_buffer,
			(long)(2*OUTBUFFSIZE));
    if(num_written < 0){
      perror("mixer write error");
      exit(1);
    }
  }
  /* increment number of audio frames played out*/
  audio_frames++; 
  /* tell to recall Work Process*/
  XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

  return(TRUE);  /* return TRUE for X work process*/
}

#endif  /*INDIGO*/

#ifdef LINUX

Boolean do_audio(XtAppContext context)
{
  int nr;
  int mix_val;
  int bytes;
  int num_written;
  int astat,end;
  int i,j,val;
  char special[4];
  long end_time;
  long elapse;
  /*shared mem and semaphore varaibles*/
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
  TIME_stamps ts;
  long temp1;
  int audio_started,video_started;
  int statptr;
  int stat_pid;

/* if we are either stopped or no audio streams are active*/
/* or are trying to quit, return without writing audio data*/
  if((master_state == STOP)||(sync_state == INTERVAL_SYNC)||(quit==TRUE)){
    XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

    return(TRUE);  /* call Work Process agian*/
  }


  /*collect time stamps*/
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

  ts.SCR = time_stamps->SCR;
  ts.start_SCR = time_stamps->start_SCR;

  if((sync_state == AUDIO_SYNC)&&(master_state != STOP)){
    time_stamps->audio_frame_clk += frame_increment;
    ts.audio_frame_clk = time_stamps->audio_frame_clk;
  }

  for(i=0;i<N+M;i++){

    ts.PTS[i] = time_stamps->PTS[i];
    ts.DTS[i] = time_stamps->DTS[i];
    ts.start_PTS[i] = time_stamps->start_PTS[i];
    ts.start_DTS[i] = time_stamps->start_DTS[i];
    ts.started[i] = time_stamps->started[i];

  }

  /* release semaphore*/
  semoparray[0].sem_op = 1;        /*release the semaphore*/
  semrtn = semop(sem_id, semoparray,nops);
  if(semrtn <0){
    perror("could not  release");
  }

  /*evaluate sync state*/
  /* calculate number of currently active audio channels*/
  audio_active = N;
  for(i=0;i<N;i++){
    if(adi->pstatus[i]== CANCEL){
      audio_active--;
    }
  }

  /*video active maintained globally by interrupts*/

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

#ifdef ANALYSIS
    fprintf(stderr,"ending audio mixer\n");
#endif

    end_audio(audio_device);

    close(adi->fd_audio_out);

#ifdef ANALYSIS
    fprintf(stderr,"detach and remove shared memory and semaphore\n");
#endif

    shmrtn = shmdt((void *)time_stamps);
    if(shmrtn < 0){
      perror("shmdt error");
    }
    cmd = IPC_RMID;
    semnum = 0;
    
    semrtn = semctl(sem_id,semnum,cmd,arg);  /* remove semophore*/
    if(semrtn <0){
      perror("could not remove semaphore set");
    }

    shmrtn = shmctl(shm_id,cmd,buff); /* remove shared memory*/
    if(shmrtn < 0){
      perror("shmctl error");
    }

#ifdef ANALYSIS
    fprintf(stdout,"audio frame clock: %f\n",(float)ts.audio_frame_clk/
                                           (float)90000);
#endif

    exit(0);

    /* end of termination routine */

  } else if(audio_active >0){    /* if currently playing audio, use */
    if(audio_started > (N-audio_active)){ /* audio frame time*/
      sync_state = AUDIO_SYNC;
    }else{
      sync_state = INTERVAL_SYNC; /* if no audio active, use interval*/
    }                             /* time to adjust audio frame clock*/

  } else{
    sync_state = INTERVAL_SYNC;
  }

  for(i=0;i<adi->N;i++){       /* for each of the audio streams*/



    stat_pid =waitpid(pid[i],&statptr,WNOHANG);
    if(stat_pid < 0){
      adi->pstatus[i] = CANCEL;
    }


    frozen = FALSE;          /* reset frozen*/
    if(adi->pstatus[i] == INACTIVE){   /* audio streams inactive if not*/
      nr= -1;                 /* yet initialized*/
      while((nr == -1)&&(errno != EAGAIN)){   /* do a non-blocking read*/
	nr = read(fd_mux[i][0],special, 4);  /* to see if initialization parameters*/
      }                                 /* have been sent*/
      errno = 0;
      if(nr ==4){   /* we have received initilization info*/


if(N==1){
	/* turn pipe to blocking*/
	if((val = fcntl(fd_mux[i][0],F_GETFL,0))<0){
	  perror("fcntl error");
	  exit(1);
	}
	val ^= O_NONBLOCK;
	if(val = fcntl(fd_mux[i][0],F_SETFL,val)<0){
	  perror("fcntl error");
	  exit(1);
	}
      }
	adi->pstatus[i] = PLAY;  /* set process status to PLAY*/

	if(special[0] == 'A'){    /* store audio data rate */

	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}else if(special[0] =='B'){

	  adi->frequency[i] = 44100;
	  adi->channels[i] = 1;
	}else if(special[0] == 'C'){
	  adi->frequency[i] = 48000;
	  adi->channels[i] = 1;
	}else{
	  fprintf(stderr,"bad frequency\n");
	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}


/* do initial read from pipes*/
	adi->num_read[i] = -10;    /* dummy set to ensure loop done at least once*/

	if((adi->frequency[i] == 32000)&&(adi->channels[i] == 1)){
	  bytes = (640*OUTBUFFSIZE)/441;

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i] < 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->common_buffer,bytes);
	  }
/* shift frequency to 44100 hz*/
	  resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			     (short *)adi->out_buffer[i],
			     OUTBUFFSIZE);

	}else if((adi->frequency[i] == 48000)&&(adi->channels[i] == 1)){
	  bytes = (OUTBUFFSIZE*960)/441;

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i]< 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->common_buffer,bytes);
	  }

	  resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			     (short *)adi->out_buffer[i],
			     OUTBUFFSIZE);

	}else{ /*we are already at 44100 Hz*/       

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i] < 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->out_buffer[i],2*OUTBUFFSIZE);
	  } 
	}

      }  /*end if nr*/

    } else { /* PLAY*/

      adi->num_read[i] = -10;

      if((adi->frequency[i] == 32000)&&(adi->channels[i] == 1)){

	bytes = (640*OUTBUFFSIZE)/441;
	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&
	      (adi->pstatus[i] != CANCEL)){


	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->common_buffer,bytes);
         

	}

	resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			   (short *)adi->out_buffer[i],
			   OUTBUFFSIZE);

      }else if((adi->frequency[i] == 48000)&&(adi->channels[i] == 1)){
	bytes = (OUTBUFFSIZE*960)/441;

	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&
	      (adi->pstatus[i] != CANCEL)){

	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->common_buffer,bytes);

	}

	resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			   (short *)adi->out_buffer[i],
			   OUTBUFFSIZE);

      }else{ /* already at 44100 Hz*/       

	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&(adi->pstatus[i] != CANCEL)){

	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->out_buffer[i],2*OUTBUFFSIZE);

	} 
      }

      if((frozen==TRUE)||(adi->num_read[i]==0)){
	fprintf(stderr,"scr %d pts %d afc %d\n",ts.SCR,ts.PTS[i]
		,ts.audio_frame_clk);
	if(ts.SCR > ts.audio_frame_clk){ /* check to ensure no false call*/
	  adi->pstatus[i] = CANCEL;
	  kill(pid[i],SIGINT);
	}else{   /*set to non-blocking*/

	}
	frozen = FALSE;
      }
      
    } /* end else for INACTIVE/PLAY*/
  }  /* for loop */

  /* Mix data ensuring muted channels skipped*/
     /* stream level audio controls also implemented*/
  for(i=0;i< OUTBUFFSIZE;i++){
    mix_val = 0;
    for(j=0;j<adi->N;j++){
      if(adi->pstatus[j] == PLAY){
	if(adi->audio_weight[j] >0){
	  mix_val += ((int) adi->out_buffer[j][i])*adi->audio_weight[j];
	}else if(adi->audio_weight[j] < 0){
	  mix_val += ((int) adi->out_buffer[j][i])/(-adi->audio_weight[j]);
	}else{
	  mix_val += ((int) adi->out_buffer[j][i]);
	}
      }
    }
    if(adi->master_volume < 0){
      adi->mix_buffer[i] = (short)(mix_val/adi->master_volume);
    }else if(adi->master_volume > 0){
      adi->mix_buffer[i] = (short)(mix_val*adi->master_volume);
    }else{
      adi->mix_buffer[i] = (short)(mix_val);
    }
  }
  /* Write data to audio port  */
  astat = write_audio(audio_device, adi->mix_buffer, (long)OUTBUFFSIZE);
  if(astat < 0){
    perror("audio mixer write error");
    exit(1);
  }
   /* As well, save to file if requested*/
  if(adi->argc ==3){
    num_written = write(adi->fd_audio_out,adi->mix_buffer,
			(long)(2*OUTBUFFSIZE));
    if(num_written < 0){
      perror("mixer write error");
      exit(1);
    }
  }
  /* increment number of audio frames played out*/
  audio_frames++; 
  /* tell to recall Work Process*/
  XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

  return(TRUE);  /* return TRUE for X work process*/
}

#endif  /*LINUX*/



#ifdef SPARC1PX

Boolean do_audio(XtAppContext context)
{
  int nr;
  int mix_val;
  int bytes;
  int num_written;
  int astat,end;
  int i,j,val;
  char special[4];
  long end_time;
  long elapse;
  /*shared mem and semaphore varaibles*/
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
  TIME_stamps ts;
  long temp1;
  int audio_started,video_started;
  short value;

/* if we are either stopped or no audio streams are active*/
/* or are trying to quit, return without writing audio data*/
  if((master_state == STOP)||(sync_state == INTERVAL_SYNC)||(quit==TRUE)){
    XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

    return(TRUE);  /* call Work Process agian*/
  }


  /*collect time stamps*/
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

  ts.SCR = time_stamps->SCR;
  ts.start_SCR = time_stamps->start_SCR;

  if((sync_state == AUDIO_SYNC)&&(master_state != STOP)){
    time_stamps->audio_frame_clk += frame_increment;
    ts.audio_frame_clk = time_stamps->audio_frame_clk;
  }

  for(i=0;i<N+M;i++){

    ts.PTS[i] = time_stamps->PTS[i];
    ts.DTS[i] = time_stamps->DTS[i];
    ts.start_PTS[i] = time_stamps->start_PTS[i];
    ts.start_DTS[i] = time_stamps->start_DTS[i];
    ts.started[i] = time_stamps->started[i];

  }

  /* release semaphore*/
  semoparray[0].sem_op = 1;        /*release the semaphore*/
  semrtn = semop(sem_id, semoparray,nops);
  if(semrtn <0){
    perror("could not  release");
  }

  /*evaluate sync state*/
  /* calculate number of currently active audio channels*/
  audio_active = N;
  for(i=0;i<N;i++){
    if(adi->pstatus[i]== CANCEL){
      audio_active--;
    }
  }

  /*video active maintained globally by interrupts*/

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

#ifdef ANALYSIS
    fprintf(stderr,"ending audio mixer\n");
#endif

    end_audio(audio_device);

    close(adi->fd_audio_out);

#ifdef ANALYSIS
    fprintf(stderr,"detach and remove shared memory and semaphore\n");
#endif

    shmrtn = shmdt((void *)time_stamps);
    if(shmrtn < 0){
      perror("shmdt error");
    }
    cmd = IPC_RMID;
    semnum = 0;
    
    semrtn = semctl(sem_id,semnum,cmd,arg);  /* remove semophore*/
    if(semrtn <0){
      perror("could not remove semaphore set");
    }

    shmrtn = shmctl(shm_id,cmd,buff); /* remove shared memory*/
    if(shmrtn < 0){
      perror("shmctl error");
    }

#ifdef ANALYSIS
    fprintf(stdout,"audio frame clock: %f\n",(float)ts.audio_frame_clk/
                                           (float)90000);
#endif

    exit(1);

    /* end of termination routine */

  } else if(audio_active >0){    /* if currently playing audio, use */
    if(audio_started > (N-audio_active)){ /* audio frame time*/
      sync_state = AUDIO_SYNC;
    }else{
      sync_state = INTERVAL_SYNC; /* if no audio active, use interval*/
    }                             /* time to adjust audio frame clock*/

  } else{
    sync_state = INTERVAL_SYNC;
  }

  for(i=0;i<adi->N;i++){       /* for each of the audio streams*/

    frozen = FALSE;          /* reset frozen*/
    if(adi->pstatus[i] == INACTIVE){   /* audio streams inactive if not*/
      nr= -1;                 /* yet initialized*/
      while((nr == -1)&&(errno != EAGAIN)){   /* do a non-blocking read*/
	nr = read(fd_mux[i][0],special, 4);  /* to see if initialization parameters*/
      }                                 /* have been sent*/
      errno = 0;
      if(nr ==4){   /* we have received initilization info*/

if(N==1){
	/* turn pipe to blocking*/
	if((val = fcntl(fd_mux[i][0],F_GETFL,0))<0){
	  perror("fcntl error");
	  exit(1);
	}
	val ^= O_NONBLOCK;
	if(val = fcntl(fd_mux[i][0],F_SETFL,val)<0){
	  perror("fcntl error");
	  exit(1);
	}
      }

	adi->pstatus[i] = PLAY;  /* set process status to PLAY*/

	if(special[0] == 'A'){    /* store audio data rate */

	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}else if(special[0] =='B'){

	  adi->frequency[i] = 44100;
	  adi->channels[i] = 1;
	}else if(special[0] == 'C'){
	  adi->frequency[i] = 48000;
	  adi->channels[i] = 1;
	}else{
	  fprintf(stderr,"bad frequency\n");
	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}

/* for SPARC1PX the pipes provide 8KHz 16 bit PCM*/
/* do initial read from pipes*/
	adi->num_read[i] = -10;    /* dummy set to ensure loop done at least once*/
       bytes = OUTBUFFSIZE;

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i] < 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->out_buffer[i],bytes);
	  }


      }  /*end if nr*/

    } else { /* PLAY*/

      adi->num_read[i] = -10;

	bytes = OUTBUFFSIZE;
	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&
	      (adi->pstatus[i] != CANCEL)){
	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->out_buffer[i],bytes);
	}


      if((frozen==TRUE)||(adi->num_read[i]==0)){
	adi->pstatus[i] = CANCEL;
	frozen = FALSE;
      }

    } /* end else for INACTIVE/PLAY*/
  }  /* for loop */

  /* Mix data ensuring muted channels skipped*/
     /* stream level audio controls also implemented*/
  for(i=0;i< OUTBUFFSIZE;i++){
    mix_val = 0;
    for(j=0;j<adi->N;j++){
      if(adi->pstatus[j] == PLAY){
	if(adi->audio_weight[j] >0){
	  mix_val += ((int) adi->out_buffer[j][i])*adi->audio_weight[j];
	}else if(adi->audio_weight[j] < 0){
	  mix_val += ((int) adi->out_buffer[j][i])/(-adi->audio_weight[j]);
	}else{
	  mix_val += ((int) adi->out_buffer[j][i]);
	}
      }
    }
    if(adi->master_volume < 0){
      value = (short)(mix_val/adi->master_volume);
    }else if(adi->master_volume > 0){
      value = (short)(mix_val*adi->master_volume);
    }else{
      value = (short)(mix_val);
    }
      adi->mix_buffer[i] = linear2ulaw[value>>3];
  }
  /* Write data to audio port  */
  astat = write_audio(audio_device, adi->mix_buffer, (long)OUTBUFFSIZE);
  if(astat < 0){
    perror("audio mixer write error");
    exit(1);
  }
   /* As well, save to file if requested*/
  if(adi->argc ==3){
    num_written = write(adi->fd_audio_out,adi->mix_buffer,
			(long)(OUTBUFFSIZE));
    if(num_written < 0){
      perror("mixer write error");
      exit(1);
    }
  }
  /* increment number of audio frames played out*/
  audio_frames++; 
  /* tell to recall Work Process*/
  XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

  return(TRUE);  /* return TRUE for X work process*/
}

#endif  /*SPARC1PX*/

#ifdef NONE

Boolean do_audio(XtAppContext context)
{
  int nr;
  int mix_val;
  int bytes;
  int num_written;
  int astat,end;
  int i,j,val;
  char special[4];
  long end_time;
  long elapse;
  /*shared mem and semaphore varaibles*/
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
  TIME_stamps ts;
  long temp1;
  int audio_started,video_started;
  int statptr;
  int stat_pid;

/* if we are either stopped or no audio streams are active*/
/* or are trying to quit, return without writing audio data*/
  if((master_state == STOP)||(sync_state == INTERVAL_SYNC)||(quit==TRUE)){
    XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

    return(TRUE);  /* call Work Process agian*/
  }


  /*collect time stamps*/
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

  ts.SCR = time_stamps->SCR;
  ts.start_SCR = time_stamps->start_SCR;

  if((sync_state == AUDIO_SYNC)&&(master_state != STOP)){
    time_stamps->audio_frame_clk += frame_increment;
    ts.audio_frame_clk = time_stamps->audio_frame_clk;
  }

  for(i=0;i<N+M;i++){

    ts.PTS[i] = time_stamps->PTS[i];
    ts.DTS[i] = time_stamps->DTS[i];
    ts.start_PTS[i] = time_stamps->start_PTS[i];
    ts.start_DTS[i] = time_stamps->start_DTS[i];
    ts.started[i] = time_stamps->started[i];

  }

  /* release semaphore*/
  semoparray[0].sem_op = 1;        /*release the semaphore*/
  semrtn = semop(sem_id, semoparray,nops);
  if(semrtn <0){
    perror("could not  release");
  }

  /*evaluate sync state*/
  /* calculate number of currently active audio channels*/
  audio_active = N;
  for(i=0;i<N;i++){
    if(adi->pstatus[i]== CANCEL){
      audio_active--;
    }
  }

  /*video active maintained globally by interrupts*/

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

#ifdef ANALYSIS
    fprintf(stderr,"ending audio mixer\n");
#endif

    end_audio(audio_device);

    close(adi->fd_audio_out);

#ifdef ANALYSIS
    fprintf(stderr,"detach and remove shared memory and semaphore\n");
#endif

    shmrtn = shmdt((void *)time_stamps);
    if(shmrtn < 0){
      perror("shmdt error");
    }
    cmd = IPC_RMID;
    semnum = 0;
    
    semrtn = semctl(sem_id,semnum,cmd,arg);  /* remove semophore*/
    if(semrtn <0){
      perror("could not remove semaphore set");
    }

    shmrtn = shmctl(shm_id,cmd,buff); /* remove shared memory*/
    if(shmrtn < 0){
      perror("shmctl error");
    }

#ifdef ANALYSIS
    fprintf(stdout,"audio frame clock: %f\n",(float)ts.audio_frame_clk/
                                           (float)90000);
#endif

    exit(0);

    /* end of termination routine */

  } else if(audio_active >0){    /* if currently playing audio, use */
    if(audio_started > (N-audio_active)){ /* audio frame time*/
      sync_state = AUDIO_SYNC;
    }else{
      sync_state = INTERVAL_SYNC; /* if no audio active, use interval*/
    }                             /* time to adjust audio frame clock*/

  } else{
    sync_state = INTERVAL_SYNC;
  }

  for(i=0;i<adi->N;i++){       /* for each of the audio streams*/



    stat_pid =waitpid(pid[i],&statptr,WNOHANG);
    if(stat_pid < 0){
      adi->pstatus[i] = CANCEL;
    }


    frozen = FALSE;          /* reset frozen*/
    if(adi->pstatus[i] == INACTIVE){   /* audio streams inactive if not*/
      nr= -1;                 /* yet initialized*/
      while((nr == -1)&&(errno != EAGAIN)){   /* do a non-blocking read*/
	nr = read(fd_mux[i][0],special, 4);  /* to see if initialization parameters*/
      }                                 /* have been sent*/
      errno = 0;
      if(nr ==4){   /* we have received initilization info*/


if(N==1){
	/* turn pipe to blocking*/
	if((val = fcntl(fd_mux[i][0],F_GETFL,0))<0){
	  perror("fcntl error");
	  exit(1);
	}
	val ^= O_NONBLOCK;
	if(val = fcntl(fd_mux[i][0],F_SETFL,val)<0){
	  perror("fcntl error");
	  exit(1);
	}
      }
	adi->pstatus[i] = PLAY;  /* set process status to PLAY*/

	if(special[0] == 'A'){    /* store audio data rate */

	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}else if(special[0] =='B'){

	  adi->frequency[i] = 44100;
	  adi->channels[i] = 1;
	}else if(special[0] == 'C'){
	  adi->frequency[i] = 48000;
	  adi->channels[i] = 1;
	}else{
	  fprintf(stderr,"bad frequency\n");
	  adi->frequency[i] = 32000;
	  adi->channels[i] = 1;
	}


/* do initial read from pipes*/
	adi->num_read[i] = -10;    /* dummy set to ensure loop done at least once*/

	if((adi->frequency[i] == 32000)&&(adi->channels[i] == 1)){
	  bytes = (4*OUTBUFFSIZE)/3;

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i] < 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->common_buffer,bytes);
	  }
/* shift frequency to 48000 hz*/
	  resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			     (short *)adi->out_buffer[i],
			     OUTBUFFSIZE);

	}else if((adi->frequency[i] == 44100)&&(adi->channels[i] == 1)){
	  bytes = (OUTBUFFSIZE*441)/240;

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i]< 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->common_buffer,bytes);
	  }

	  resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			     (short *)adi->out_buffer[i],
			     OUTBUFFSIZE);

	}else{        

	  while((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
		( adi->num_read[i] < 0)&&(quit==FALSE)&&
		(adi->pstatus[i] != CANCEL)){
	    adi->num_read[i] = read(fd_mux[i][0],
				    (char *)adi->out_buffer[i],2*OUTBUFFSIZE);
	  } 
	}

      }  /*end if nr*/

    } else { /* PLAY*/

      adi->num_read[i] = -10;

      if((adi->frequency[i] == 32000)&&(adi->channels[i] == 1)){

	bytes = (4*OUTBUFFSIZE)/3;
	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&
	      (adi->pstatus[i] != CANCEL)){


	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->common_buffer,bytes);
         

	}

	resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			   (short *)adi->out_buffer[i],
			   OUTBUFFSIZE);

      }else if((adi->frequency[i] == 44100)&&(adi->channels[i] == 1)){
	bytes = (OUTBUFFSIZE*441)/240;

	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&
	      (adi->pstatus[i] != CANCEL)){

	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->common_buffer,bytes);

	}

	resize_array_width((short *)adi->common_buffer,(bytes/2),1,
			   (short *)adi->out_buffer[i],
			   OUTBUFFSIZE);

      }else{        

	if((frozen==FALSE)&&(ts.start_PTS[i]<=ts.audio_frame_clk)&&
	      (quit==FALSE)&&(adi->pstatus[i] != CANCEL)){

	  adi->num_read[i] = read(fd_mux[i][0],
				  (char *)adi->out_buffer[i],2*OUTBUFFSIZE);

	} 
      }

      if((frozen==TRUE)||(adi->num_read[i]==0)){
	fprintf(stderr,"scr %d pts %d afc %d\n",ts.SCR,ts.PTS[i]
		,ts.audio_frame_clk);
	if(ts.SCR > ts.audio_frame_clk){ /* check to ensure no false call*/
	  adi->pstatus[i] = CANCEL;
	  kill(pid[i],SIGINT);
	}else{   /*set to non-blocking*/

	}
	frozen = FALSE;
      }
      
    } /* end else for INACTIVE/PLAY*/
  }  /* for loop */

  /* Mix data ensuring muted channels skipped*/
     /* stream level audio controls also implemented*/
  for(i=0;i< OUTBUFFSIZE;i++){
    mix_val = 0;
    for(j=0;j<adi->N;j++){
      if(adi->pstatus[j] == PLAY){
	if(adi->audio_weight[j] >0){
	  mix_val += ((int) adi->out_buffer[j][i])*adi->audio_weight[j];
	}else if(adi->audio_weight[j] < 0){
	  mix_val += ((int) adi->out_buffer[j][i])/(-adi->audio_weight[j]);
	}else{
	  mix_val += ((int) adi->out_buffer[j][i]);
	}
      }
    }
    if(adi->master_volume < 0){
      adi->mix_buffer[i] = (short)(mix_val/adi->master_volume);
    }else if(adi->master_volume > 0){
      adi->mix_buffer[i] = (short)(mix_val*adi->master_volume);
    }else{
      adi->mix_buffer[i] = (short)(mix_val);
    }
  }

   /*  save to file if requested*/
  if(adi->argc ==3){
    num_written = write(adi->fd_audio_out,adi->mix_buffer,
			(long)(2*OUTBUFFSIZE));
    if(num_written < 0){
      perror("mixer write error");
      exit(1);
    }
  }
  /* increment number of audio frames played out*/
  audio_frames++; 
  /* tell to recall Work Process*/
  XtAppAddWorkProc(context, (XtWorkProc)do_audio, context);

  return(TRUE);  /* return TRUE for X work process*/
}

#endif  /*NONE*/






