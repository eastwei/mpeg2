/*
 * Copyright (c) 1994 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA NOR BOSTON UNIVERSITY
 * BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA AND BOSTON UNIVERSITY
 * SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */
/********************************************************************
*                     fps_convert.c  by Jim Boucher
*                          Boston University
*                      Multimedia Communications Lab
*
*     This program can be used to take any numerically ordered
* set of files(images) sampled at a given integer sampling
* rate and convert them to another integer sampling rate
* either faster or slower than the original.
********************************************************************/

#include <stdio.h>

usage (p)
char *p;

{
fprintf (stderr, "Usage: %s inbase outbase extension start \
end oldfps newfps\n", p);
fprintf (stderr, "Example: Given files input20.jpg input30.jpg at 15 fps:\n");
fprintf (stderr, "\n => fps_convert input output jpg 20 30 15 24\n");
fprintf(stderr,"\n  Produces files output0.jpg and so on\n");
fprintf (stderr, "\n Where:\noldfps is the old image sampling rate \
(INTEGER ONLY)\n");
fprintf (stderr, "newfps is desired output sampling rate (INTEGER ONLY)\n");
    exit (1);
}

main (argc, argv)
int argc;
char **argv;

{
    char *inbase;      
    char *outbase;
    char *extension;     
    int start,end;
    int oldfps;
    int newfps;
    char command[256];      /* command line buffer */
    int i,j;
    int in_total,out_total;
    int new_frames;

    if (argc != 8) usage(argv[0]); /* to read input command and interpret */
    inbase = argv[1];
    outbase = argv[2];
    extension  = argv[3];
    start = atoi(argv[4]);
    end = atoi(argv[5]);
    oldfps = atoi(argv[6]);
    newfps = atoi(argv[7]);

    in_total = 0.0;
    out_total = 0.0;


    j = start;
    i = 0;

    while(j<=end){
          
	sprintf(command, "cp %s%d.%s %s%d.%s",inbase,  
	      j,extension,outbase,i,extension); 

     system(command);
     i++;
      out_total = out_total + oldfps;
      while(in_total < out_total){
        in_total = in_total + newfps; 
        j++;
      }
      if(in_total > out_total){
      in_total = in_total - newfps;
      j--;
    }


     }
 fprintf(stderr,"%d frames written[0-%d]\n",i,(i-1));
    
}



