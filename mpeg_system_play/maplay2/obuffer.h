/*
 *  @(#) obuffer.h 1.8, last edit: 6/15/94 16:51:56
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

#ifndef OBUFFER_H
#define OBUFFER_H

#include <iostream.h>
#include <unistd.h>
#include <stdlib.h>
#include "all.h"
#include "header.h"
extern "C" {
#include "audio_includes.h"
}


static const uint32 OBUFFERSIZE = 2 * 1152;	// max. 2 * 1152 samples per frame
static const uint32 MAXCHANNELS = 2;		// max. number of channels


// Abstract base class for audio output classes:
class Obuffer
{
public:
  virtual     ~Obuffer (void) {}		// dummy
  virtual void append (uint32 channel, int16 value) = 0;




	       // this function takes a 16 Bit PCM sample
  virtual void write_buffer (int fd) = 0;
	       // this function should write the samples to the filedescriptor
	       // or directly to the audio hardware
};


// An audio output class for raw pcm output
// or if ULAW is defined:
// An audio output class for 8-bit ulaw output at 8 kHz:
class FileObuffer : public Obuffer
{
private:
#ifdef ULAW2
  ulawsample buffer[OBUFFERSIZE];
  ulawsample *bufferp[MAXCHANNELS];
#else
  int16 buffer[OBUFFERSIZE];
  int16 *bufferp[MAXCHANNELS];
#endif
  uint32 channels;

public:
	FileObuffer (uint32 number_of_channels);
       ~FileObuffer (void) {}
  void	append (uint32 channel, int16 value);
  void	write_buffer (int fd);
};


#ifdef Indigo
// a class for direct sound output on SGI machines:
class IndigoObuffer : public Obuffer
{
private:
  int16 buffer[OBUFFERSIZE];
  int16 *bufferp[MAXCHANNELS];
  uint32 channels;
  ALport port;

public:
	IndigoObuffer (uint32 number_of_channels, Header *);
       ~IndigoObuffer (void);
  void	append (uint32 channel, int16 value);
  void	write_buffer (int dummy);
};
#endif	// Indigo


#ifdef SPARC
// a class for direct sound output on SPARC 10 machines (dbri device)
// or if ULAW is defined:
// a class for direct sound output on SPARCs using a 8 kHz
// 8-bit u-law audio device: (audioamd)
class SparcObuffer : public Obuffer
{
private:
#ifdef ULAW2
  ulawsample buffer[OBUFFERSIZE >> 1];		// mono only
  ulawsample *bufferp;
#else
  int16 buffer[OBUFFERSIZE];
  int16 *bufferp[MAXCHANNELS];
  uint32 channels;
#endif
  static int audio_fd;

  static int open_audio_device (void);
#ifdef Solaris
  static void get_device_type (int fd, audio_device *);
#else
  static int get_device_type (int fd);
#endif

public:
#ifdef ULAW2
	SparcObuffer (Header *, bool use_speaker, bool use_headphone, bool use_line_out);
#else
	SparcObuffer (uint32 number_of_channels, Header *,
 		      bool use_speaker, bool use_headphone, bool use_line_out);
#endif
       ~SparcObuffer (void);
  void	append (uint32 channel, int16 value);
  void	write_buffer (int dummy);

#ifdef ULAW2
  static bool class_suitable (uint32 number_of_channels, bool force_amd);
	// returnvalue == False: no u-law output possible (class unsuitable)
#else
  static bool class_suitable (void);
	// returnvalue == False: no 16-bit output possible (class unsuitable)
#endif
};
#endif	// SPARC


#ifdef LINUX
class LinuxObuffer : public Obuffer
{
  int16  buffer[OBUFFERSIZE];
  int16 *bufferp[MAXCHANNELS];
  uint32 channels;
  static int audio_fd;

  static int open_audio_device (void);

public:
	LinuxObuffer (uint32 number_of_channels, Header *);
       ~LinuxObuffer (void);
  void	append (uint32 channel, int16 value);
  void	write_buffer (int dummy);

  static bool class_suitable (uint32 number_of_channels);
};
#endif	// LINUX

#endif
