#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../mpeg.h"

int main(int argc, char *argv[])
{
    FILE *fp_mpeg,*fp_out;
    ImageDesc img;
    unsigned char *pixels;
    int frameID, frameCounter,moreFrame;

    if(argc!=4) 
    { printf("Usage:  %s  MPEG_file  frameID  output_file\007\n",argv[0]);
      exit(1);
     }

    fp_mpeg=fopen(argv[1], "r");
    if(fp_mpeg==NULL) 
    {
    	perror("MPEG source file.");
    	exit(-1);
     }

    frameID= atoi(argv[2]);
    if(frameID<=0) { printf("Error: Invalid frameID: %d\007\n",frameID);
                     exit(3);
                   }
    pixels=(unsigned char *)malloc(img.Size);
    if(pixels==NULL)
    {
    	perror("Memory allocation");
    	exit(-1);
    }

    SetMPEGOption(MPEG_DITHER, FULL_COLOR_DITHER);
    OpenMPEG(fp_mpeg,& img);
    printf("MPEG frame Width=%d, Height=%d\n", img.Width, img.Height);
    frameCounter=0;
    moreFrame=TRUE;
    while((frameCounter!=frameID)  && (moreFrame==TRUE))
    {
      moreFrame=GetMPEGFrame(pixels);
      frameCounter++;
    }
    if(frameCounter==frameID)
    {  fp_out=fopen(argv[3],"w");
       if(fp_out==NULL) { perror("output file."); fclose(fp_mpeg);exit(2); }
       fwrite(pixels,img.Size,1,fp_out);
       fclose(fp_out);
       printf("Get Frame OK.\n");
    }
    else
    { printf("Total %d  frames.\007\n",frameCounter);
      printf("No more frame available.\n");
    }
    exit(0);
/*    CloseMPEG();*/
/*    fclose(fp_mpeg);*/
/*    free(pixels);*/
}
