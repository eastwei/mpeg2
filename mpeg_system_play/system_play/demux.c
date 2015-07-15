/****************************************************
*  File Name:   demux.c
*  Creation Date: 1330 hrs 04 August 1994
*  Authors:       John D. Palmer and James A. Boucher
*                 email palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*                  Professor T.D.C. Little tdcl@flash.bu.edu
*
*  Description:   This module contains 8 functions which
*         deal with a multiplexed MPEG-I data stream.
*  void get_buf_data(char *buf_ptr, int read_len)
*  int  get_next_start_code(int fd_input)
*  void read_pack_header(PACK_header *pki)
*  void read_system_header(SYSTEM_header *si)
*  void read_packet_header(PACKET_header *pi )
*  void print_pack_header(PACK_header *pki)
*  void print_system_header(SYSTEM_header *si)
*  void print_packet_header(PACKET_header *pi)  
*
*****************************************************/

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
***************************************************************************/

#include "system.h"

/***************************************************/
/* GLOBALS */
extern unsigned char raw_data_buf[MAX_DATA_BUF_LEN];
extern unsigned char *raw_data_buf_ptr;
extern int raw_data_buf_len;

/***************************************************/

/*************************************************************************
*  Function:      get_buf_data
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        read_len is the number of bytes that the calling function
*                          wants to be able to use from the input stream.
*  Effects:       Maintains a buffer of data as read from the input file.
*                 Maintains a pointer to the next unused data in the buffer.
*                 Returns a pointer to the next unused data in the buffer.*
*
*  Description:
*   A global buffer is maintained. 
*   When the function is called, 
*     if there is insufficient data in the buffer, and
*      if there is more data to be read from the input file, 
*       get_buf_data will replenish the buffer.
*   get_buf_data increments a global pointer to the buffer 
*    by the amount of data requested, so that even if too much data
*    is referenced by the calling function, the next call will get a 
*    pointer to the correct location.
*   get_buf_data then passes back a pointer to 
*    the first unused byte in the buffer.
**************************************************************************/

char *get_buf_data(int read_len)
{
  int len_left;
  int len_read;
  int temp1;  
  int i;
  char *buf_ptr;

  /* Calculate how much data is left in the buffer */
  len_left = raw_data_buf_len - (raw_data_buf_ptr - raw_data_buf);
  if (len_left < read_len){   /*Not enough for the calling function's needs */
    if (len_left > 0){   /* There is some data left, so copy to start of buffer  */
      memcpy((unsigned char*)raw_data_buf, (unsigned char*)raw_data_buf_ptr, len_left);
    }
    raw_data_buf_ptr = &raw_data_buf[0];
    temp1 = MAX_DATA_BUF_LEN - len_left;  /* To fill the rest of the buffer */
    len_read = read(fdin,(char *)(raw_data_buf_ptr + len_left), temp1);
    if(len_read == -1){
      perror("Error: reading system file");
      exit(1);
    }

    /* no data left in file - data should never have been requested */
    if (len_read == 0){
      perror("Error: reached EOF");
      exit(1);
    }

    raw_data_buf_len = len_left + len_read;

    /* Read done to end of file but still not enough data to complete request */
    if (raw_data_buf_len < read_len){
      perror("Error: insufficient data for read");
      exit(1);
    }
  }

  buf_ptr = raw_data_buf_ptr;   /* For return to calling function. */
  raw_data_buf_ptr += read_len; /* Global advanced for use at next call. */
  return(buf_ptr);
}  /* end get_buf_data  */
/***************************************************/



/*************************************************************************
*  Function:      calc_time_stamp
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        buf_ptr is a pointer to the global input data buffer.
*                 time_stamp is a pointer to a variable which 
*                     is to be calculated.
*  Effects:       A value for time_stamp is calculated.
*                 buf_ptr is incremented by the number of buffer bytes
*                         used by calc_time_stamp.
*  Description: 
*   Three time stamps are used in an MPEG-I stream: DTS, PTS, and SCR 
*    (Decoding Time Stamp, Presentation Time Stamp, System Clock Reference)
*    Since all are encoded in the same format, one function decodes them all.
*   buf_ptr points to the first data byte to be used.
*    calc_time_stamp parses out marker bits, 
*            adding the components of the time stamp.
*    The time stamp value is passed back by reference. 
*    The buf_ptr is passed back with its new value.
**************************************************************************/

char *calc_time_stamp(char *buf_ptr, volatile long int *time_stamp)
{
  int i;
  char *temp_ptr;

  /*Determine Time Stamp           */
  /*Byte  0: 4 bits:depend on SCR/PTS/DTS*/
  /*         3 bits:Time32..30, 1 bit marker:1   */
  /*Byte  1: 8 bits Time29..22.   Byte  2: 7 bits Time21..15, 1 bit marker*/
  /*Byte  3: 8 bits Time14..7.    Byte  4: 7 bits Time6..0,   1 bit marker*/
  *time_stamp = ((((long)(*buf_ptr)) & 0x0E) << 29);
  if((*buf_ptr & (char)0x01) != (char)0x01){      /*Marker_bit check*/
    perror("Expecting marker_bit value '1' at calc_time_stamp A.");
    exit(1);
  }
  buf_ptr++;

  *time_stamp |= ((((long)(*buf_ptr)) & 0xFF) << 22);
  buf_ptr++;

  *time_stamp |= ((((long)(*buf_ptr)) & 0xFE) << 14);
  if((*buf_ptr & (char)0x01) != (char)0x01){      /*Marker_bit check*/
    perror("Expecting marker_bit value '1' at calc_time_stamp B.");
    exit(1);
  }
  buf_ptr++;

  *time_stamp |= ((((long)(*buf_ptr)) & 0xFF) <<  7);
  buf_ptr++;

  *time_stamp |= ((((long)(*buf_ptr)) & 0xFE) >>  1);
  if((*buf_ptr & (char)0x01) != (char)0x01){      /*Marker_bit check*/
    perror("Expecting marker_bit value '1' at calc_time_stamp C.");
    exit(1);
  }
  buf_ptr++;
  return(buf_ptr);
}
/***************************************************/



/*************************************************************************
*  Function:      get_next_start_code
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        None.
*  Effects:       Increments the global buf_ptr past the start code bytes.
*                 Returns a char indicating the meaning of the start code.
*  Description:
*   Gets 4 bytes from the buffer.
*     Ensures first 3 are 0,0,1 indicating start_code.
*     Evaluates fourth byte to return char indicating meaning of start_code.
**************************************************************************/

char get_next_start_code()
{
  char *buf_ptr;
  int i;
  char start_code[3];

  start_code[0] = 0x00;
  start_code[1] = 0x00;
  start_code[2] = 0x01;

  /*  Check first 3 bytes: should be hex 00 00 01 for Pack Start Code,*/
  /*  System Start Code, Packet Start Code or ISO_11172_end_code.     */
  buf_ptr = get_buf_data(4);

  for (i = 0; i < 3; i++) {
    if (*buf_ptr != start_code[i]){
      fprintf(stderr, "No Start Code. %d %d %d\n",i,(int)(*buf_ptr),(int)start_code[i]);
       exit(1);
    }
    buf_ptr++;
  }

  #ifdef ANALYSIS
    fprintf(stderr,"*Start Code found.\n");
  #endif

  switch(*buf_ptr){
  case (char)0xBA:
    return((char)PACK_START_CODE);
    break;
  case (char)0xB9:
    return((char)ISO_11172_END_CODE);
    break;
  case (char)0xBB:
    return((char)SYSTEM_START_CODE);
    break;
  default:
    return(*buf_ptr);  /*It must be a stream_id.*/
    break;
  }
  /* This statement should never be reached */
  return(ISO_11172_END_CODE);

} /*end get_next_start_code*/
/*****************************************************/



/*************************************************************************
*  Function:      read_pack_header
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:        pki is a pointer to a pack_header structure,
*                     for which values are to be determined.
*  Effects:       Values for components of the pack_header are calculated.
*                 buf_ptr is incremented by the number of bytes used.
*  Description:
*   Data from the system-stream buffer is parsed, and values are extracted for:
*       SCR (System Clock Reference), and 
*       Mux Rate
*   buf_ptr is incremented by the number of bytes used.
*
**************************************************************************/

void read_pack_header(PACK_header *pki)
{
  char *buf_ptr;
  int mux_byte_rate;
  
  pki->SCR = 0;
  buf_ptr = get_buf_data(PACK_HEADER_LENGTH);
  /*Determine System Clock Reference (SCR)                   */
  /*Check marker_bits: '0010' = 0x2     */
  if((*buf_ptr & (char)0xF0) != (char)0x20){
    perror("Expecting '0010' after pack_start_code.");
    exit(1);
  }
  buf_ptr = calc_time_stamp(buf_ptr, &pki->SCR);
  #ifdef ANALYSIS
    fprintf(stderr,"System Clock Reference is %d.\n", pki->SCR);
  #endif

  /*Determine Mux Rate*/
  /*Byte  5: 1 bit marker, 7 bits mux_rate.    Byte 6: 8 bits mux_rate.  */
  /*Byte  7: 7 bits mux_rate, 1 bit marker                                */
  pki->mux_rate = ((((int)(*buf_ptr)) & 0x7F) << 14);
  if((*buf_ptr & (char)0x80) != (char)0x80){      /*Marker_bit check*/
    fprintf(stderr,"byte= %x.\n", *buf_ptr);
    perror("Expecting marker_bit value '1' at read_pack_header A.");
    exit(1);
  }
  buf_ptr++;

  pki->mux_rate |= ((((int)(*buf_ptr)) & 0xFF) <<  7);
  buf_ptr++;

  pki->mux_rate |= ((((int)(*buf_ptr)) & 0xFE) >>  1);
  if((*buf_ptr & (char)0x01) != (char)0x01){      /*Marker_bit check*/
    perror("Expecting marker_bit value '1' at read_pack_header B.");
    exit(1);
  }
  buf_ptr++;

  #ifdef ANALYSIS
    mux_byte_rate = pki->mux_rate * 50;
    fprintf(stderr,"Mux Rate # is %d or %d Bytes per Second.\n",
                   pki->mux_rate, mux_byte_rate);
  #endif

  return;
}  /*end read_pack_header*/
/*************************************************************/



/*************************************************************************
*  Function:      read_system_header
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:    si is a pointer to a SYSTEM_header structure,
*                 for which values are to be determined.
*  Effects:   Values for SYSTEM_header components are calculated and changed.
*             buf_ptr is incremented by the number of bytes used.
*  Description:
*   The system_stream header length is read from the buffer.
*   Data from the system-stream buffer is parsed, and values are extracted
*     for each of the values in the system_header structure, including 
*     STREAM data for each elementary stream in the system_stream.
**************************************************************************/

void read_system_header(SYSTEM_header *si)
{
  char *buf_ptr;
  int i, j;
  char test;
  int buf_byte_size_bound;

  /*Read System Header Length: 2 Bytes after start code. */
    /*Length of header after header length bytes.        */
    /*Bytes  0 & 1: 8 bits header_length--high & low     */
  buf_ptr = get_buf_data(2);
  si->header_length  = (unsigned int)(((int)(*buf_ptr))<<8);
  buf_ptr++;
  si->header_length |= (unsigned int)(((int)(*buf_ptr)) & 0xFF);
  buf_ptr++;
  #ifdef ANALYSIS
    fprintf(stderr,"System Header Length is %d after length bytes.\n", si->header_length);
  #endif

  /*Read the rest of the header*/
  buf_ptr = get_buf_data(si->header_length);

  /*Byte  0: 1 bit marker, 7 bits rate_bound--high  */
  si->rate_bound  = (unsigned int)((((int)*buf_ptr) & 0x7F) << 15);
  if((*buf_ptr & (char)0x80) != (char)0x80){   /*Marker bit check*/
    perror("Expecting marker_bit value '1' before rate_bound--high.");
    exit(1);
  }
  buf_ptr++;

  /*Byte  1: 8 bits rate_bound--mid                 */
  si->rate_bound += (unsigned int)((((int)*buf_ptr) & 0xFF) <<  7);
  buf_ptr++;

  /*Byte  2: 7 bits rate_bound-low, 1 bit marker    */
  si->rate_bound += (unsigned int)((((int)*buf_ptr) & 0xFE) >>  1);
  if((*buf_ptr & (char)0x01) != (char)0x01) {	/*Marker bit check*/
    perror("Expecting marker_bit value '1' after rate_bound.");
    exit(1);
  }
  buf_ptr++;

  #ifdef ANALYSIS
    fprintf(stderr,"Rate_Bound is %d.\n", si->rate_bound);
  #endif

  /*Byte  3: 6 bits audio_bound, 1 bit fixed_flag, 1bit CSPS_flag*/
  si->audio_bound = (unsigned int)((((int)*buf_ptr) & 0xFF) >> 2);
  si->fixed_flag  = (unsigned int)((((int)*buf_ptr) & 0x02) >> 1);
  si->CSPS_flag   = (unsigned int) (((int)*buf_ptr) & 0x01);
  buf_ptr++;
  #ifdef ANALYSIS
    fprintf(stderr,"Audio_Bound=%d. Fixed_Flag=%d.  CSPS_Flag=%d.\n",
                         si->audio_bound, si->fixed_flag, si->CSPS_flag);
  #endif

  /*Byte 4: 1b system_audio_lock_flag, 1b system_video_lock_flag, 1b marker, 5b video_bound*/
  si->system_audio_lock_flag =(unsigned int)((((int)*buf_ptr) & 0x80) >> 7);
  si->system_video_lock_flag =(unsigned int)((((int)*buf_ptr) & 0x40) >> 6);
  si->video_bound            =(unsigned int) (((int)*buf_ptr) & 0x1F);
  if((*buf_ptr & (char)0x20) != (char)0x20){    /*Marker bit check*/
    perror("Expecting marker_bit value '1' after system_video_lock_flag.");
    exit(1);
  }
  buf_ptr++;
  #ifdef ANALYSIS
    fprintf(stderr,"salf=%d. svlf=%d. video_bound=%d.\n",
            si->system_audio_lock_flag, 
            si->system_video_lock_flag, si->video_bound);
  #endif

  if(*buf_ptr != (char)0xFF){
    perror("Reserved byte not set to default");
  exit(1);
  }
  buf_ptr++;

  if(si->header_length == 6) return;
  j=0;

  while((int)buf_ptr < (int)raw_data_buf_ptr){ /*All lmntry stream_id's ...*/
    test = (*buf_ptr &(char)0x80);
    if(test == 0){
      perror("system header length incorrect");
      exit(1);
    }
    si->STD_buffer_info[j].stream_id = *buf_ptr &(char)0xFF;
    i++;
    buf_ptr++;

    /*Check marker_bits*/
    if((*buf_ptr & (char)0xC0) != (char)0xC0){
      perror("Expecting placeholder bit values '11' after stream_id.");
      exit(1);
    }
    si->STD_buffer_info[j].STD_buffer_bound_scale = (*buf_ptr &(char)0x20)>>5;
    si->STD_buffer_info[j].STD_buffer_size_bound  = (*buf_ptr &(char)0x1F)<<8;
    i++;
    buf_ptr++;

    si->STD_buffer_info[j].STD_buffer_size_bound  |= *buf_ptr &(char)0xFF;
    i++;
    buf_ptr++;

    #ifdef ANALYSIS
    if(si->STD_buffer_info[i].STD_buffer_bound_scale == 0){
      buf_byte_size_bound = si->STD_buffer_info[i].STD_buffer_size_bound * 128;
    }
    else{
      buf_byte_size_bound= si->STD_buffer_info[i].STD_buffer_size_bound * 1024;
    }

      fprintf(stderr,"System: stream_id %d\n",((int)si->STD_buffer_info[j].stream_id &0x00FF));
      fprintf(stderr,"System: STD_buffer_bound_scale %d\n",si->STD_buffer_info[j].STD_buffer_bound_scale);
      fprintf(stderr,"System: STD_buffer_size_bound %d or %d bytes.\n",si->STD_buffer_info[j].STD_buffer_size_bound, buf_byte_size_bound);
    #endif
    j++;
  } /*end while*/

  return;

}  /*end read_system_header*/
/*************************************************************/



/*************************************************************************
*  Function:      read_packet_header
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:  pi is a pointer to a PACKET_header structure,
*               for which values are to be determined.
*  Effects: Values for PACKET_header components are calculated and changed.
*           buf_ptr is incremented by the number of bytes used.
*  Description:
*   The PACKET_stream header length is read from the buffer.
*   Data from the system-stream buffer is parsed, and values are extracted
*     for each of the values in the PACKET_header structure, if provided.
*     If PTS & DTS are not provided, their values are set to 0.
**************************************************************************/

void read_packet_header(PACKET_header *pi )
{
  char *buf_ptr;
  char *current_ptr;
  char test, test1;
  int i;
  int input_buffer_size;

  pi->PTS = 0;
  pi->DTS = 0;
  /* read packet length */ 
  buf_ptr = get_buf_data(2);

  /*Bytes 0 & 1: 8 bits packet_length--high & low    */
  pi->packetlength  = (unsigned int)(((int)*buf_ptr)<<8);
  buf_ptr++;
  pi->packetlength |= (unsigned int)(((int)*buf_ptr) & 0xFF);
  buf_ptr++;
  #ifdef ANALYSIS
    fprintf(stderr," Packet Length is %d.\n", pi->packetlength);
  #endif

  /*Get the remainder of the packet.*/
  buf_ptr = get_buf_data(pi->packetlength);

  if(pi->stream_id <= (char)0xBF){  /*private stream 2*/
    pi->packet_ptr = buf_ptr;
    return;
  }

  /*Skip over any stuffing Bytes 1111 1111 (16 max)     */
     /*Count to 17 to see if there are more than allowed in the spec.  */
  i=0;
  while((*buf_ptr == (char)0xFF)&&(i<17)){
    #ifdef ANALYSIS
      fprintf(stderr,"Stuffing Byte: %d\n",i);
    #endif
    buf_ptr++;
    pi->packetlength--;
    i++;
  }
  if(i == 17){
    perror("Too many stuffing bytes after packet_length");
    exit(1);
  }

  /*If nextbits =='01' read STD_buffer_scale & _size */
    /*Byte: 2b '01', 1b STD_buffer_scale, 5b STD_buffer_size-hi */
    /*Byte: 8 bits STD_buffer_size--lo                          */
  test1 = *buf_ptr & (char)0xC0;
  if(test1 == (char)0x40){      /*read STD values*/
    pi->STD_buffer_scale = (*buf_ptr & (char)0x20)>>5;
    pi->STD_buffer_size  = (*buf_ptr & (char)0x1F)<<8;
    buf_ptr++;
    pi->packetlength--;
    pi->STD_buffer_size |= (*buf_ptr & (char)0xFF);
    buf_ptr++;
    pi->packetlength--;
  }

  if(pi->STD_buffer_scale == 0){
    input_buffer_size = pi->STD_buffer_size * 128;
  }
  else{
    input_buffer_size = pi->STD_buffer_size * 1024;
  }
    #ifdef ANALYSIS
      fprintf(stderr,"Buffer size = %d Bytes.\n", input_buffer_size);
    #endif

  /*Determine whether to read in PTS, PTS & DTS, or neither before data.*/
  test = (*buf_ptr & (char)0xF0);

  switch(test){
  case (char)0x20:   /*Get PTS only*/
    buf_ptr = calc_time_stamp(buf_ptr, &pi->PTS);
    pi->packetlength -= 5;
    #ifdef ANALYSIS
      fprintf(stderr,"PTS is %d.\n", pi->PTS);
    #endif
    break;

  case (char)0x30:   /*Get PTS & DTS. Both same format as PTS above.*/
    /*   pts and data */
    buf_ptr = calc_time_stamp(buf_ptr, &pi->PTS);
    #ifdef ANALYSIS
      fprintf(stderr,"PTS is %d.\n", pi->PTS);
    #endif

    /*Get DTS*/
    if ((*buf_ptr & (char)0xF0) != (char)0x10){
      perror("Expecting bits '0001' before DTS in read_packet_header");
      exit(1);
    }
    buf_ptr = calc_time_stamp(buf_ptr, &pi->DTS);
    pi->packetlength -= 10;
    #ifdef ANALYSIS
      fprintf(stderr,"DTS is %d.\n", pi->DTS);
    #endif    
    break;

  case (char)0x00:
    if(*buf_ptr != (char)0x0F){
      perror("not a valid packet time sequence");
      exit(1);
    }
    buf_ptr++;
    pi->packetlength--;
    break;

  default:
    perror("invalid time code in packet");
    exit(1);
  } /*end switch(test) */

  /*packetlength should have been properly set by the above*/

  pi->packet_ptr = buf_ptr;

  return;
} /*end read_packet_header */
/*************************************************************/



/*************************************************************************
*  Function:      print_pack_header
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:   pki is a pointer to a PACK_header.
*  Effects:  None.
*  Description:
*   Values of the PACK_header components are printed for stream diagnostic 
*     purposes, with supporting text.
**************************************************************************/

void print_pack_header(PACK_header *pki)
{
  int mux_byte_rate;
  mux_byte_rate = pki->mux_rate * 50;
  printf("PACK HEADER: SCR = %d, Mux Rate = %d, or %d Bytes per second\n",
            pki->SCR, pki->mux_rate, mux_byte_rate);
  return;
}
/*************************************************************/



/*************************************************************************
*  Function:      print_system_header
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:   si is a pointer to a SYSTEM_header.
*  Effects:  None.
*  Description:
*   Values of the SYSTEM_header components are printed for stream diagnostic 
*     purposes, with supporting text.
**************************************************************************/

void print_system_header(SYSTEM_header *si)
{
  int i = 0;
  char stream_type;
  int stream_num;
  int buf_byte_size_bound;

  printf("SYSTEM HEADER: Hdr Len = %d, Rate bound = %d, Audio bound = %d,\n",
          si->header_length, si->rate_bound, si->audio_bound); 
  printf("Fixed flag = %d, CSPS flag = %d, Sys audio lock flag = %d,\n",
           si->fixed_flag, 
          si->CSPS_flag, si->system_audio_lock_flag); 
   printf("Sys video lock flag = %d, Video bound = %d\n",
          si->system_video_lock_flag, si->video_bound);

  while(si->STD_buffer_info[i].stream_id != 0){
    if(((si->STD_buffer_info[i].stream_id>>5) & 0x6) == 0x6){
      stream_type = 'A';
      stream_num = (int)((si->STD_buffer_info[i].stream_id) & 0x1F);
    }
    if(((si->STD_buffer_info[i].stream_id>>4) & 0xE) == 0xE){
      stream_type = 'V';
      stream_num = (int)((si->STD_buffer_info[i].stream_id) & 0x0F);
    }
    if(si->STD_buffer_info[i].STD_buffer_bound_scale == 0){
      buf_byte_size_bound = si->STD_buffer_info[i].STD_buffer_size_bound * 128;
    }
    else{
      buf_byte_size_bound= si->STD_buffer_info[i].STD_buffer_size_bound * 1024;
    }
    printf("STD buf %d: Stream %c%d, Buf bound scale %d, Buf size bound %d or %d bytes\n",
           i, stream_type, stream_num,
           si->STD_buffer_info[i].STD_buffer_bound_scale,
           si->STD_buffer_info[i].STD_buffer_size_bound,
           buf_byte_size_bound);
  i++;
  }
  return;
}
/*************************************************************/



/*************************************************************************
*  Function:      print_packet_header
*  Creation Date: 12 August 1994
*  Author:        John D. Palmer
*                 email  palmerj@acs.bu.edu
*                 Multimedia Communications Lab, Boston University
*  Inputs:   pi is a pointer to a PACKET_header.
*  Effects:  None.
*  Description:
*   Values of the PACKET_header components are printed for stream diagnostic 
*     purposes, with supporting text.
**************************************************************************/

void print_packet_header(PACKET_header *pi)
{
  int i = 0;
  char stream_type;
  int stream_num;
  if(((pi->stream_id>>5) & 0x6) == 0x6){
    stream_type = 'A';
    stream_num = (int)((pi->stream_id) & 0x1F);
  }
  if(((pi->stream_id>>4) & 0xE) == 0xE){
    stream_type = 'V';
    stream_num = (int)((pi->stream_id) & 0x0F);
  }

  printf("PACKET HDR %c%d: %dB, STD buf scale %d, ..size %d, PTS %d, DTS %d\n",
          stream_type, stream_num, pi->packetlength, pi->STD_buffer_scale,
          pi->STD_buffer_size, pi->PTS, pi->DTS);

  return;
}

