/*************************************************************************
*  File Name:     test.c 
*  Creation Date: 30 July 1994
*  Author:        John D. Palmer
*                 email palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
                   Professor T.D.C. Little tdcl@flash.bu.edu
*  Usage:         test input_file
*
*  Description:   This program accepts an MPEG-I system stream
*   and separates it into its component elementary streams.
*   Elementary audio & video streams are written to separate files.
*   Reserved, Private, and Stuffing streams are discarded.
*   Printed output allows the user to review the structure and
*   content of the system stream, packs, and packets.
*   Output saved to file should be ready for playback through an MPEG 
*   audio or video player as applicable.
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
**************************************************************************/

#define MAX_DATA_BUF_LEN   100000

#include "system.h"

char packet_data[BIG_BUFFER_SIZE];
char raw_data_buf[MAX_DATA_BUF_LEN];
char *raw_data_buf_ptr;
int raw_data_buf_len;
int fdin;
int M,N;
int fd_demuxa[48];
 
/***********************************************/
void main(int argc, char *argv[])  
/*Requires a single argument: a system-stream file name*/
{
  int  c, c2, i, j, n, m;
  char next_code;
  int current_audio;
  int current_video;
  int demux_id[48];

  char name[48][20];

  PACK_header  pack_info;
  PACK_header *pack_info_p;
  SYSTEM_header  sys_info;
  SYSTEM_header *sys_info_p;
  PACKET_header  pkt_info;
  PACKET_header *pkt_info_p;

  pack_info_p = &pack_info;
  sys_info_p  = &sys_info;
  pkt_info_p  = &pkt_info;

  raw_data_buf_len = 0;
  raw_data_buf_ptr = raw_data_buf;


  /*Verify that the correct number of arguments were provided*/
  if (argc != 2){
    perror("Usage: demux input_file_name.");
    exit(1);
  }
  for (i = 0; i < argc; i++){ 
    fprintf(stderr, "Argument %d is %s \n", i, argv[i]);
  }
   
  /*---------------------------------------*/
  /*  Open the input file for reading of the data stream.  */
  if ((fdin = open (argv[1], O_RDONLY) ) == -1 ) {
    perror("Could not open input file.");
    exit(1);
  }
  fprintf(stderr,"Input file %d opened OK.\n", fdin);

  /*---------------------------------------*/
  /*Read and write data.                   */
  while(1){

    next_code = get_next_start_code();
    fprintf(stderr, "next_code = %x\n", next_code );
    /* next_code identifies whether the stream is at 
     *  the beginning of a system stream, a pack, or a packet, 
     *  or if it is at the end of a system stream. */
    switch(next_code){

    case((char)PACK_START_CODE):      /*0xBA*/
      read_pack_header(pack_info_p);
      print_pack_header(pack_info_p);
      break;

    case((char)SYSTEM_START_CODE):      /*0xBB*/
      read_system_header(sys_info_p);
      print_system_header(sys_info_p);
      N = sys_info_p->audio_bound;
      M = sys_info_p->video_bound;
      fprintf(stderr, " %d audio, %d video\n", N,M);
      current_audio = 0;
      current_video = N;
      for (i=0; i<48; i++){
        demux_id[i] = -1;
      }

      /* Create N+M output files, pointed to by fd_demuxa[i] */
      for(i=0; i<(N+M); i++){
        sprintf(name[i], "out_file_%d", i);
        if((fd_demuxa[i] = creat(name[i], PERMS)) < 0){
          perror("Could not create output file.\n");
          exit(1);
        }
      fprintf(stderr, "Created file %s\n", name[i]);
      }
      break;

    case((char)ISO_11172_END_CODE):      /*0xB9*/
      fprintf(stderr,"end of system stream reached\n");
      close(fdin);
      for(i=0; i<(N+M); i++){
        close(fd_demuxa[i]);
      }
      exit(1);
      break;

    default:   /*PACKET START CODE & stream_id*/
      pkt_info_p->stream_id = next_code;      
      read_packet_header(pkt_info_p);
      print_packet_header(pkt_info_p);

      /*-----------------------------*/
      /* The stream_id identifies the stream by type and number. */
      switch(pkt_info_p->stream_id){
      case((char)0xB8): /* may need more work*/
        /*Following STD_buffer_scale and _size refer to */
        /*all audio streams in the muxed stream*/
        perror("Error handling common audio STD_buffer_scale and _size.");
        exit(1);
        break;
      case((char)0xB9): /*may need more work*/
        /*Following STD_buffer_scale and _size refer to */
        /*all video streams in the muxed stream*/
        perror("Error handling common video STD_buffer_scale and _size.");
        exit(1);
        break;
      case((char)0xBC):
        /*Reserved stream: discard*/
	perror("Received reserved stream");
        exit(1);
        break;
      case((char)0xBD):
        /*private_stream_1: discard*/
        perror("Received private_stream_1");
        exit(1);
        break;
      case((char)0xBE):  /* data bytes all FF */
        /*Padding stream: discard*/
        perror("Received padding stream");
        break;
      case((char)0xBF):
        /*private_stream_2: discard*/
        perror("Received private_stream_2");
        exit(1);
        break;
      default:
        /* The stream is an audio, video, or reserved_data stream. */
        /*Convert stream_id to C0 to 0, EF to 48*/
        i = (((int)pkt_info_p->stream_id) & 0xFF) - 192;

        if(i<0){
          /*Invalid stream: do nothing */
        }
        else if (i<48){  /* Audio or Video stream. */
          if(demux_id[i] == -1){  /* A new stream */
            if(i<32){    /* Audio */
              demux_id[i] = current_audio;
              current_audio++;
            }
            else{
              demux_id[i] = current_video;
              current_video++;
            }
          }
          j = demux_id[i];

          if((c2 = write(fd_demuxa[j], pkt_info_p->packet_ptr,
               pkt_info_p->packetlength)) !=  pkt_info_p->packetlength){
            fprintf(stderr, "Error writing to output file %d\n", j);
            exit(1);
          }
        }
        else if(i>=48){ /* It's a reserved data stream.  
               * Additional functionality will be implementation dependent*/
          fprintf(stderr, "Reserved data stream %d is being ignored.\n", i);
          break;
        } /*end else if pkt_info_p->stream_id...*/

        else{  /* Should never be reached. */
          fprintf(stderr,"Unrecognized stream ID: %d %x %c\n", 
                    pkt_info_p->stream_id, pkt_info_p->stream_id,
                    (char)pkt_info_p->stream_id);
          exit(1);
          break;
        }
        break;
      }  /*  end switch(pkt_info_p->stream_id)*/
    }  /* end switch(next_code) */
  }  /* end while */
}  /* end main */
