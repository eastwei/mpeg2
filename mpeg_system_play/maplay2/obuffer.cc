/*
 *  @(#) obuffer.cc 1.12, last edit: 6/17/94 15:42:37
 *  @(#) Copyright (C) 1993, 1994 Tobias Bading (bading@cs.tu-berlin.de)
 *  @(#) Berlin University of Technology
 *
 *  Idea and first implementation for u-law output with fast downsampling by
 *  Jim Boucher (jboucher@flash.bu.edu)
 *
 *  LinuxObuffer class written by
 *  Louis P. Kruger (lpkruger@phoenix.princeton.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *  Changes from version 1.1 to 1.2:
 *    - new LinuxObuffer class, which works with 16 bit soundcards
 *      like Gravis Ultrasound, SoundBlaster 16 or Pro Audio Spectrum 16,
 *      if attached to /dev/dsp
 *    - ShortObuffer renamed to FileObuffer
 *    - If ULAW is defined:
 *      - SparcObuffer feeds mono u-law output to an amd device (u-law format).
 *        The required downsampling to 8 kHz in done in the synthesis filter.
 *      - FileObuffer creates u-law output at 8 kHz instead of 16 bit PCM samples.
 *    - O_NDELAY flag is cleared now after a successful open syscall
 *      on /dev/audio (caused problems under Solaris)
 *    - options -us, -uh and -ul should work now
 *    - FileObuffer can handle incomplete write syscalls now
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <iostream.h>
#include "obuffer.h"
#include "header.h"
#ifdef ULAW2
#include "ulaw.h"
#endif

#define AUDIO_ENCODING_LINEAR 3

#ifdef SunOS
extern "C" int ioctl (int, int, void *);
#endif

FileObuffer::FileObuffer (uint32 number_of_channels)
{
#ifdef DEBUG
  if (!number_of_channels || number_of_channels > MAXCHANNELS)
  {
    cerr << "FileObuffer: number of channels has to be in [1, " <<  MAXCHANNELS << "] !\n";
    exit (1);
  }
#endif

#ifdef ULAW2
  if (number_of_channels > 1)
    cerr << "Are you sure you need stereo u-law output?\n";
#endif
  channels = number_of_channels;
  for (int i = 0; i < number_of_channels; ++i)
    bufferp[i] = buffer + i;
}


void FileObuffer::append (uint32 channel, int16 value)
{
#ifdef DEBUG
  if (channel >= channels)
  {
    cerr << "illegal channelnumber in FileObuffer::append()!\n";
    exit (1);
  }
  if (bufferp[channel] - buffer >= OBUFFERSIZE)
  {
    cerr << "FileObuffer: buffer overflow!\n";
    exit (1);
  }
#endif

#ifdef ULAW2
  // convert 16-bit PCM sample to 8-bit ulaw:
  *bufferp[channel] = linear2ulaw[value >> 3];
#else
  *bufferp[channel] = value;
#endif
  bufferp[channel] += channels;
}


void FileObuffer::write_buffer (int fd)
{
  int length = (int)((char *)bufferp[0] - (char *)buffer), writelength;

  if ((writelength = write (fd, (char *)buffer, length)) != length)
  {
    // buffer has not been written completely
    if (writelength < 0)
    {
      perror ("write");
      exit (1);
    }
    length -= writelength;
    char *buffer_pos = (char *)buffer;
    do
    {
      buffer_pos += writelength;
      if ((writelength = write (fd, buffer_pos, length)) < 0)
      {
	perror ("write");
	exit (1);
      }
    }
    while (length -= writelength);
  }

  for (int i = 0; i < channels; ++i)
    bufferp[i] = buffer + i;
}



#ifdef Indigo
IndigoObuffer::IndigoObuffer (uint32 number_of_channels, Header *header)
{
#ifdef DEBUG
  if (!number_of_channels || number_of_channels > MAXCHANNELS)
  {
    cerr << "IndigoObuffer: number of channels has to be in [1, " <<  MAXCHANNELS << "] !\n";
    exit (1);
  }
#endif
  channels = number_of_channels;
  for (int i = 0; i < number_of_channels; ++i)
    bufferp[i] = buffer + i;

  // open an audio port and configure it:
  ALconfig config;
  if (!(config = ALnewconfig ()))
  {
    cerr << "ALnewconfig failed!\n";
    exit (1);
  }
  ALsetwidth (config, AL_SAMPLE_16);
  if (channels == 1)
    ALsetchannels (config, AL_MONO);
  else
    ALsetchannels (config, AL_STEREO);
  if (!(port = ALopenport ("MPEG audio player", "w", config)))
  {
    cerr << "can't allocate an audio port!\n";
    exit (1);
  }

  // set sample rate:
  long pvbuffer[2] = { AL_OUTPUT_RATE, 0 };
  pvbuffer[1] = header->frequency ();
  ALsetparams (AL_DEFAULT_DEVICE, pvbuffer, 2);
  ALfreeconfig (config);
}


IndigoObuffer::~IndigoObuffer (void)
{
  while (ALgetfilled (port) > 0)
    sleep (1);
  ALcloseport (port);
}


void IndigoObuffer::append (uint32 channel, int16 value)
{
#ifdef DEBUG
  if (channel >= channels)
  {
    cerr << "illegal channelnumber in IndigoObuffer::append()!\n";
    exit (1);
  }
  if (bufferp[channel] - buffer >= OBUFFERSIZE)
  {
    cerr << "IndigoObuffer: buffer overflow!\n";
    exit (1);
  }
#endif
  *bufferp[channel] = value;
  bufferp[channel] += channels;
}


void IndigoObuffer::write_buffer (int)
{
  ALwritesamps (port, buffer, (long)(bufferp[0] - buffer));
  for (int i = 0; i < channels; ++i)
    bufferp[i] = buffer + i;
}
#endif // Indigo


#ifdef SPARC
int SparcObuffer::audio_fd = -1;

#ifdef ULAW2
SparcObuffer::SparcObuffer (Header *header, bool use_speaker, bool use_headphone, bool use_line_out)
#else
SparcObuffer::SparcObuffer (uint32 number_of_channels, Header *header,
			    bool use_speaker, bool use_headphone, bool use_line_out)
#endif
{
#ifndef ULAW2
#ifdef DEBUG
  if (!number_of_channels || number_of_channels > MAXCHANNELS)
  {
    cerr << "SparcObuffer: 0 < number of channels < " << MAXCHANNELS << "!\n";
    exit (1);
  }
#endif
#endif	// !ULAW2

  if (audio_fd < 0)
  {
    cerr << "Internal error: SparcObuffer::audio_fd has to be initialized\n"
	    "by SparcObuffer::class_suitable()!\n";
    exit (1);
  }

  audio_info info;
  AUDIO_INITINFO (&info);
#ifdef SunOS4_1_3
  info.output_muted = False;
#endif
  info.play.port = 0;
  if (use_speaker)
    info.play.port |= AUDIO_SPEAKER;
  if (use_headphone)
    info.play.port |= AUDIO_HEADPHONE;
#ifdef SunOS4_1_3
  if (use_line_out)
    info.play.port |= AUDIO_LINE_OUT;
#endif

#ifdef ULAW2
  bufferp = buffer;

  // configure the amd device:
  info.play.encoding = AUDIO_ENCODING_ULAW;
  info.play.precision = 8;
  info.play.channels = 1;
  info.play.sample_rate = 8000;
#else
  channels = number_of_channels;
  for (int i = 0; i < number_of_channels; ++i)
    bufferp[i] = buffer + i;

  // configure the dbri device:
  info.play.encoding = AUDIO_ENCODING_LINEAR;
  info.play.precision = 16;
  info.play.channels = channels;
  info.play.sample_rate = header->frequency ();
#endif	// !ULAW2

  if (ioctl (audio_fd, AUDIO_SETINFO, &info))
  {
    perror ("configuration of /dev/audio failed");
    exit (1);
  }
}


SparcObuffer::~SparcObuffer (void)
{
  ioctl (audio_fd, AUDIO_DRAIN, NULL);
  close (audio_fd);
}


void SparcObuffer::append (uint32 channel, int16 value)
{
#ifdef ULAW2
#ifdef DEBUG
  if (bufferp - buffer >= OBUFFERSIZE >> 1)
  {
    cerr << "SparcObuffer: buffer overflow!\n";
    exit (1);
  }
#endif

  // convert 16-bit PCM sample to 8-bit ulaw:
  *bufferp++ = linear2ulaw[value >> 3];
#else
#ifdef DEBUG
  if (channel >= channels)
  {
    cerr << "illegal channelnumber in SparcObuffer::append()!\n";
    exit (1);
  }
  if (bufferp[channel] - buffer >= OBUFFERSIZE)
  {
    cerr << "SparcObuffer: buffer overflow!\n";
    exit (1);
  }
#endif

  *bufferp[channel] = value;
  bufferp[channel] += channels;
#endif	// !ULAW2
}


void SparcObuffer::write_buffer (int)
{
#ifdef ULAW2
  int length = (int)((char *)bufferp - (char *)buffer);
#else
  int length = (int)((char *)*bufferp - (char *)buffer);
#endif
  if (write (audio_fd, (char *)buffer, length) != length)
  {
    perror ("write to /dev/audio failed");
    exit (1);
  }
#ifdef ULAW2
  bufferp = buffer;
#else
  for (int i = 0; i < channels; ++i)
    bufferp[i] = buffer + i;
#endif
}


int SparcObuffer::open_audio_device (void)
{
  int fd;

  if ((fd = open ("/dev/audio", O_WRONLY | O_NDELAY)) < 0)
    if (errno == EBUSY)
    {
      cerr << "Sorry, the audio device is busy!\n";
      exit (1);
    }
    else
    {
      perror ("can't open /dev/audio for writing");
      exit (1);
    }

  // turn NDELAY mode off:
  int flags;
  if ((flags = fcntl (fd, F_GETFL, 0)) < 0)
  {
    perror ("fcntl F_GETFL on /dev/audio failed");
    exit (1);
  }
  flags &= ~O_NDELAY;
  if (fcntl (fd, F_SETFL, flags) < 0)
  {
    perror ("fcntl F_SETFL on /dev/audio failed");
    exit (1);
  }
  return fd;
}


#ifdef Solaris
void SparcObuffer::get_device_type (int fd, audio_device *devtype)
{
  if (ioctl (fd, AUDIO_GETDEV, devtype))
  {
    perror ("ioctl AUDIO_GETDEV on /dev/audio");
    exit (1);
  }
}
#else
int SparcObuffer::get_device_type (int fd)
{
#ifdef AUDIO_GETDEV
  int devtype;
  if (ioctl (fd, AUDIO_GETDEV, &devtype))
  {
    perror ("ioctl AUDIO_GETDEV on /dev/audio");
    exit (1);
  }
  return devtype;
#else
  cerr << "SparcObuffer::get_device_type(): AUDIO_GETDEV ioctl not available!\n";
  return -1;
#endif
}
#endif	// !Solaris


#ifdef ULAW2
bool SparcObuffer::class_suitable (uint32 number_of_channels, bool force_amd)
#else
bool SparcObuffer::class_suitable (void)
#endif
{
#ifdef ULAW2
  if (number_of_channels > 1)
  {
    cerr << "Your audio hardware cannot handle more than one audio channel.\n"
	    "Please use the option -l or -r for stereo streams.\n";
    return False;
  }
#endif

  // check for the dbri audio device:
  audio_fd = open_audio_device ();

#ifdef ULAW2
  if (force_amd)
    return True;
#endif

#ifdef Solaris
  audio_device devtype;
  get_device_type (audio_fd, &devtype);
# ifdef ULAW2
    if (!strcmp (devtype.name, "SUNW,am79c30"))
      return True;
    else if (!strcmp (devtype.name, "SUNW,dbri"))
    {
      cerr << "Your machine can produce CD-quality audio output,\n"
	      "but this binary was compiled for 8 kHz u-law ouput. (telephone quality)\n"
	      "Please recompile it without the ULAW define in COMPILERFLAGS.\n"
	      "(or use the -amd option to use this binary with low-quality output)\n";
      close (audio_fd);
      return False;
    }
# else
    if (!strcmp (devtype.name, "SUNW,dbri"))
      return True;
    else if (!strcmp (devtype.name, "SUNW,am79c30"))
    {
      cerr << "Your machine can produce 8 kHz u-law audio output only,\n"
	      "but this binary was compiled for CD-quality output.\n"
	      "Please recompile it with ULAW defined in COMPILERFLAGS.\n";
      close (audio_fd);
      return False;
    }
# endif
#else
  // !Solaris
# ifdef SunOS4_1_1
    // no AUDIO_GETDEV under SunOS 4.1.1, so we have to assume that there is
    // an amd device attached to /dev/audio
#   ifdef ULAW2
      return True;
#   else
      cerr << "Your machine can produce 8 kHz u-law audio output only,\n"
	      "but this binary was compiled for CD-quality output.\n"
	      "Please recompile it with ULAW defined in COMPILERFLAGS.\n";
      close (audio_fd);
      return False;
#   endif	// !ULAW2
# else
    // SunOS 4.1.3
    int device_type = get_device_type (audio_fd);
#   ifdef ULAW2
      if (device_type == AUDIO_DEV_AMD)
	return True;
      else if (device_type == AUDIO_DEV_SPEAKERBOX)
      {
	cerr << "Your machine can produce CD-quality audio output,\n"
		"but this binary was compiled for 8 kHz u-law ouput. (telephone quality)\n"
		"Please recompile it without the ULAW define in COMPILERFLAGS.\n"
		"(or use the -amd option to use this binary with low-quality output)\n";
	close (audio_fd);
	return False;
      }
#   else
      if (device_type == AUDIO_DEV_SPEAKERBOX)
	return True;
      else if (device_type == AUDIO_DEV_AMD)
      {
	cerr << "Your machine can produce 8 kHz u-law audio output only,\n"
		"but this binary was compiled for CD-quality output.\n"
		"Please recompile it with ULAW defined in COMPILERFLAGS.\n";
	close (audio_fd);
	return False;
      }
#   endif	// !ULAW2
# endif	// !SunOS4_1_1
#endif	// !Solaris

#ifndef SunOS4_1_1
  close (audio_fd);
  cerr << "Sorry, I don't recognize your audio device.\n"
# ifdef ULAW2
	  "Please try the -amd option or use the stdout mode.\n";
# else
	  "Please use the stdout mode.\n";
# endif
  return False;
#endif
}

#endif	// SPARC


#ifdef LINUX
int LinuxObuffer::audio_fd = -1;

int LinuxObuffer::open_audio_device (void)
{
  int fd;

  if ((fd = open ("/dev/dsp", O_WRONLY | O_NDELAY, 0)) < 0)
    if (errno == EBUSY)
    {
      cerr << "Sorry, the audio device is busy!\n";
      exit (1);
    }
    else
    {
      perror ("can't open /dev/dsp for writing");
      exit (1);
    }

  // turn NDELAY mode off:
  int flags;
  if ((flags = fcntl (fd, F_GETFL, 0)) < 0)
  {
    perror ("fcntl F_GETFL on /dev/audio failed");
    exit (1);
  }
  flags &= ~O_NDELAY;
  if (fcntl (fd, F_SETFL, flags) < 0)
  {
    perror ("fcntl F_SETFL on /dev/audio failed");
    exit (1);
  }
  return fd;
}


LinuxObuffer::LinuxObuffer (uint32 number_of_channels, Header *header)
{
#ifdef DEBUG
  if (!number_of_channels || number_of_channels > MAXCHANNELS)
  {
    cerr << "LinuxObuffer: 0 < number of channels < " << MAXCHANNELS << "!\n";
    exit (1);
  }
#endif
  channels = number_of_channels;
  for (int i = 0; i < number_of_channels; ++i)
    bufferp[i] = buffer + i;

  if (audio_fd < 0)
  {
    cerr << "Internal error, LinuxObuffer::audio_fd has to be initialized\n"
	    "by LinuxObuffer::class_suitable()!\n";
    exit (1);
  }

  // configure the device:
  int play_precision = 16;
  int play_stereo = channels-1;
  int play_sample_rate = header->frequency ();

  if(
      ioctl(audio_fd, SNDCTL_DSP_SAMPLESIZE, &play_precision) == -1 ||
      ioctl(audio_fd, SNDCTL_DSP_STEREO, &play_stereo) == -1 ||
      ioctl(audio_fd, SNDCTL_DSP_SPEED, &play_sample_rate) == -1
    )
  {
    perror ("configuration of /dev/dsp failed");
    exit (1);
  }
}


LinuxObuffer::~LinuxObuffer (void)
{
  sleep (1);
  close (audio_fd);
}


void LinuxObuffer::append (uint32 channel, int16 value)
{
#ifdef DEBUG
  if (channel >= channels)
  {
    cerr << "illegal channelnumber in LinuxObuffer::append()!\n";
    exit (1);
  }
  if (bufferp[channel] - buffer >= OBUFFERSIZE)
  {
    cerr << "buffer overflow!\n";
    exit (1);
  }
#endif
  *bufferp[channel] = value;
  bufferp[channel] += channels;
}


void LinuxObuffer::write_buffer (int)
{
  int length = (int)((char *)bufferp[0] - (char *)buffer);
  if (write (audio_fd, buffer, length) != length)
  {
    perror ("write to /dev/dsp failed");
    exit (1);
  }
  for (int i = 0; i < channels; ++i)
    bufferp[i] = buffer + i;
}


bool LinuxObuffer::class_suitable (uint32 number_of_channels)
{
  if (number_of_channels > 1)
  {
    cerr << "Sorry, stereo output not supported.\n"
	    "Please use the option -l or -r for stereo streams.\n";
    return False;
  }

  // open the dsp audio device:
  audio_fd = open_audio_device ();
  return True;
}

#endif	/* LINUX */
