
/*
 *  @(#) maplay.cc 1.19, last edit: 6/17/94 15:43:38
 *  @(#) Copyright (C) 1993, 1994 Tobias Bading (bading@cs.tu-berlin.de)
 *  @(#) Berlin University of Technology
 *
 *  Many thanks for ideas and implementations to:
 *  -> Jim Boucher (jboucher@flash.bu.edu)
 *     for his idea and first implementation of 8 kHz u-law output
 *  -> Louis P. Kruger (lpkruger@phoenix.princeton.edu)
 *     for his implementation of the LinuxObuffer class
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
 *    - minor changes to create a LinuxObuffer object
 *    - minor changes for a u-law version, which creates 8 kHz u-law output
 *      on an amd device or in stdout mode, if compiled with ULAW defined
 *    - option -amd forces maplay to treat /dev/audio as an amd device
 *      in the u-law version. This is helpful on some SPARC clones.
 *    - iostreams manipulator calls like "cerr << setw (2) << ..." replaced by
 *      "cerr.width (2); ..." due to problems with older GNU C++ releases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream.h>
#include <iomanip.h>
#include "all.h"
#include "crc.h"
#include "header.h"
#include "ibitstream.h"
#include "obuffer.h"
#include "subband.h"
#include "subband_layer_1.h"
#include "subband_layer_2.h"
#include "synthesis_filter.h"
#include <signal.h>

// data extracted from commandline arguments:
static char *filename;
static bool verbose_mode = False, filter_check = False;
static bool stdout_mode = False;
static enum e_channels which_channels = both;
static bool use_speaker = False, use_headphone = False, use_line_out = False;
#ifdef ULAW
static bool force_amd = False;
#endif
static bool use_own_scalefactor = False;
static real scalefactor;

// data extracted from header of first frame:
static uint32 layer;
static e_mode mode;
static e_sample_frequency sample_frequency;

// objects needed for playing a file:
static Ibitstream *stream;
static Header *header;
static Crc16 *crc;
static Subband *subbands[32];
static SynthesisFilter *filter1 = NULL, *filter2 = NULL;
static Obuffer *buffer = NULL;


static void Exit (int returncode)
  // delete some objects and exit
{
  delete buffer;	// delete on NULL-pointers are harmless
  delete filter1;
  delete filter2;
  delete header;
  delete stream;
  exit (returncode);
}


main (int argc, char *argv[])
{
  int i;
  bool read_ready = False, write_ready = False;
  char special[4];
  
  if (argc < 2 || !strncmp (argv[1], "-h", 2))
  {
usage:
    cerr << "\nusage: " << argv[0]
	 << " [-v] [-s] [-l] [-r] "
#ifdef SPARC
	    "[-us] [-uh] "
#ifndef SunOS4_1_1
	    "[-ul] "
#endif
#endif
	    "[-c] [-f ushort] filename\n"
	    "  filename   filename of a MPEG audio stream or - for stdin\n"
	    "  -v         verbose mode\n"
#ifdef ULAW
	    "  -s         write u-law samples to stdout\n"
#else
	    "  -s         write pcm samples to stdout\n"
#endif
	    "  -l         decode only the left channel\n"
	    "  -r         decode only the right channel\n"
#ifdef SPARC
	    "  -us        send audio signal to speaker (default)\n"
	    "  -uh        send audio signal to headphone\n"
#ifndef SunOS4_1_1
	    "  -ul        send audio signal to line out\n"
#endif
#endif
#ifdef ULAW
	    "  -amd       force maplay to treat /dev/audio as an amd device\n"
#endif
	    "  -c         check for filter range violations\n"
	    "  -f ushort  use this scalefactor instead of the default value 32768\n\n"
	    "@(#) MPEG Audio Player maplay 1.2 "
#ifdef ULAW
	    "(8 kHz u-law "
#else
	    "("
#endif
#ifdef Indigo
	    "Indigo IRIX"
#else
#ifdef SunOS4_1_1
	    "SPARC SunOS 4.1.1"
#else
#ifdef SunOS4_1_3
	    "SPARC SunOS 4.1.3"
#else
#ifdef Solaris
	    "SPARC Solaris 2.x"
#else
#ifdef LINUX
	    "Linux"
#else
#ifdef ULTRIX
	    "RISC ULTRIX 4.x"
#else
	    "unknown"
#endif
#endif
#endif
#endif
#endif
#endif
	    " version)\n"
	    "@(#) Copyright (C) 1993, 1994 Tobias Bading (bading@cs.tu-berlin.de)\n"
	    "@(#) Berlin University of Technology\n"
	    "@(#) Created: 6/17/94 15:44:30\n"
	    "@(#) This program is free software. See the GNU General Public License\n"
	    "@(#) in the file COPYING for more details.\n\n";
    exit (0);
  }

  // parse arguments:
  for (i = 1; i < argc; ++i)
    if (argv[i][0] == '-' && argv[i][1])
      switch ((int)argv[i][1])
      {
	case 'v':
	  verbose_mode = True;
	  break;
	case 's':
	  stdout_mode = True;
	  break;
	case 'l':
	  which_channels = left;
	  break;
	case 'r':
	  which_channels = right;
	  break;
#ifdef SPARC
	case 'u':
	  switch (argv[i][2])
	  {
	    case 's':
	      use_speaker = True;
	      break;
	    case 'h':
	      use_headphone = True;
	      break;
#ifndef SunOS4_1_1
	    case 'l':
	      use_line_out = True;
	      break;
#endif
	    default:
	      goto usage;
	  }
	  break;
#endif
#ifdef ULAW
	case 'a':
	  force_amd = True;
	  break;
#endif
	case 'c':
	  filter_check = True;
	  break;
	case 'f':
	  if (++i == argc)
	  {
	    cerr << "please specify a new scalefactor after the -f option!\n";
	    exit (1);
	  }
	  use_own_scalefactor = True;
	  sscanf (argv[i], "%f", &scalefactor);
	  break;
	default:
	  goto usage;
      }
    else if (!filename)
      filename = argv[i];
    else
      goto usage;

  if (!filename)
    goto usage;
  if (!(use_speaker || use_headphone || use_line_out))
    use_speaker = True;

  if (!strcmp (filename, "-"))
    stream = new Ibitstream (0);		// read from stdin
  else
    stream = new Ibitstream (filename);		// read from file

  header = new Header;
  if (!header->read_header (stream, &crc))
  {
    cerr << "no header found!\n";
    Exit (1);
  }

  // get info from header of first frame:
  layer = header->layer ();
  if ((mode = header->mode ()) == single_channel)
    which_channels = left;
  sample_frequency = header->sample_frequency ();

  // create filter(s):
#ifdef ULAW
  if (use_own_scalefactor)
    filter1 = new SynthesisFilter (0, sample_frequency, scalefactor);
  else
    filter1 = new SynthesisFilter (0, sample_frequency);
  if (mode != single_channel && which_channels == both)
    if (use_own_scalefactor)
      filter2 = new SynthesisFilter (1, sample_frequency, scalefactor);
    else
      filter2 = new SynthesisFilter (1, sample_frequency);
#else
  if (use_own_scalefactor)
    filter1 = new SynthesisFilter (0, scalefactor);
  else
    filter1 = new SynthesisFilter (0);
  if (mode != single_channel && which_channels == both)
    if (use_own_scalefactor)
      filter2 = new SynthesisFilter (1, scalefactor);
    else
      filter2 = new SynthesisFilter (1);
#endif	// !ULAW

  // create buffer:
  if (stdout_mode)
    if (mode == single_channel || which_channels != both)
      buffer = new FileObuffer (1);
    else
      buffer = new FileObuffer (2);
  else
#ifdef Indigo
  {
    if (mode == single_channel || which_channels != both)
      buffer = new IndigoObuffer (1, header);
    else
      buffer = new IndigoObuffer (2, header);
  }
#else
#ifdef SPARC
  {
#ifdef ULAW2
    if (SparcObuffer::class_suitable ((mode == single_channel || which_channels != both) ? 1 : 2,
				      force_amd))	// amd device available?
      buffer = new SparcObuffer (header, use_speaker, use_headphone, use_line_out);
#else
    if (SparcObuffer::class_suitable ())		// dbri device available?
      if (mode == single_channel || which_channels != both)
	buffer = new SparcObuffer (1, header, use_speaker, use_headphone, use_line_out);
      else
	buffer = new SparcObuffer (2, header, use_speaker, use_headphone, use_line_out);
#endif
    else
      Exit (0);
  }
#else
#ifdef LINUX
  {
    if (LinuxObuffer::class_suitable (mode == single_channel || which_channels != both) ? 1 : 2)
      if (mode == single_channel || which_channels != both)
	buffer = new LinuxObuffer (1, header);
      else
	buffer = new LinuxObuffer (2, header);
    else
      Exit (0);
  }
#else
//#ifdef your_machine
//  {
//    if (mode == single_channel || which_channels != both)
//      buffer = new your_Obuffer (your_parameters);	// mono
//    else
//      buffer = new your_Obuffer (your_parameters);	// stereo
//  }
//#else
  {
    cerr << "Sorry, I don't know your audio device.\n"
	    "Please use the stdout mode.\n";
    Exit (0);
  }
//#endif	// !your_machine
#endif	// !LINUX
#endif	// !SPARC
#endif	// !Indigo

  if (verbose_mode)
  {
    // print informations about the stream
    char *name = strrchr (filename, '/');
    if (name)
      ++name;
    else
      name = filename;
    cerr << name << " is a layer " << header->layer_string () << ' '
	 << header->mode_string () << " MPEG audio stream with";
    if (!header->checksums ())
      cerr << "out";
    cerr << " checksums.\nThe sample frequency is "
	 << header->sample_frequency_string () << " at a bitrate of "
	 << header->bitrate_string () << ".\n"
	    "This stream is ";
    if (header->original ())
      cerr << "an original";
    else
      cerr << "a copy";
    cerr << " and is ";
    if (!header->copyright ())
      cerr << "not ";
    cerr << "copyright protected.\n";
  }

/* Output data rate to stdout so the audio mixer knows
   what the input frequency is*/
#ifdef SYSTEM_PLAY
  switch(sample_frequency){
  case thirtytwo:
    special[0] = 'A';
    break;
  case fourtyfour_point_one:
    special[0] = 'B';
    break;
  case fourtyeight:
    special[0] = 'C';
    break;
  default:
    cerr<<"not valid frequency\n";
    exit(1);
  }
  special[1] = '1';
  special[2] = '2';
  special[3] = '3';
  (void) write(1,special,4);
#endif
  
  do
  {
    // is there a change in important parameters?
    // (bitrate switching is allowed)
    if (header->layer () != layer)
    {
      // layer switching is allowed
      if (verbose_mode)
	cerr << "switching to layer " << header->layer_string () << ".\n";
      layer = header->layer ();
    }
    if ((mode == single_channel && header->mode () != single_channel) ||
	(mode != single_channel && header->mode () == single_channel))
    {
      // switching from single channel to stereo or vice versa is not allowed
      cerr << "illegal switch from single channel to stereo or vice versa!\n";
      Exit (1);
    }
    if (header->sample_frequency () != sample_frequency)
    {
      // switching the sample frequency is not allowed
      cerr << "sorry, can't switch the sample frequency in the middle of the stream!\n";
      Exit (1);
    }

    // create subband objects:
    if (header->layer () == 1)
    {
      if (header->mode () == single_channel)
	for (i = 0; i < header->number_of_subbands (); ++i)
	  subbands[i] = new SubbandLayer1 (i);
      else if (header->mode () == joint_stereo)
      {
	for (i = 0; i < header->intensity_stereo_bound (); ++i)
	  subbands[i] = new SubbandLayer1Stereo (i);
	for (; i < header->number_of_subbands (); ++i)
	  subbands[i] = new SubbandLayer1IntensityStereo (i);
      }
      else
	for (i = 0; i < header->number_of_subbands (); ++i)
	  subbands[i] = new SubbandLayer1Stereo (i);
    }
    else if (header->layer () == 2)
    {
      if (header->mode () == single_channel)
	for (i = 0; i < header->number_of_subbands (); ++i)
	  subbands[i] = new SubbandLayer2 (i);
      else if (header->mode () == joint_stereo)
      {
	for (i = 0; i < header->intensity_stereo_bound (); ++i)
	  subbands[i] = new SubbandLayer2Stereo (i);
	for (; i < header->number_of_subbands (); ++i)
	  subbands[i] = new SubbandLayer2IntensityStereo (i);
      }
      else
	for (i = 0; i < header->number_of_subbands (); ++i)
	  subbands[i] = new SubbandLayer2Stereo (i);
    }
    else
    {
      cerr << "sorry, layer 3 not implemented!\n";
      Exit (0);
    }

    
    // start to read audio data:
    for (i = 0; i < header->number_of_subbands (); ++i)
      subbands[i]->read_allocation (stream, header, crc);

    if (header->layer () == 2)
      for (i = 0; i < header->number_of_subbands (); ++i)
	((SubbandLayer2 *)subbands[i])->read_scalefactor_selection (stream, crc);

    if (!crc || header->checksum_ok ())
    {
      // no checksums or checksum ok, continue reading from stream:
      for (i = 0; i < header->number_of_subbands (); ++i)
	subbands[i]->read_scalefactor (stream, header);

      do
      {
	for (i = 0; i < header->number_of_subbands (); ++i)
	  read_ready = subbands[i]->read_sampledata (stream);

	do
	{
	  for (i = 0; i < header->number_of_subbands (); ++i)
	    write_ready = subbands[i]->put_next_sample (which_channels, filter1, filter2);

	  filter1->calculate_pcm_samples (buffer);
	  if (which_channels == both && header->mode () != single_channel)
	    filter2->calculate_pcm_samples (buffer);
	}
	while (!write_ready);
      }
      while (!read_ready);

      buffer->write_buffer (1);		// write to stdout
    }
    else
      // Sh*t! Wrong crc checksum in frame!
      cerr << "WARNING: frame contains wrong crc checksum! (throwing frame away)\n";

    for (i = 0; i < header->number_of_subbands (); ++i)
      delete subbands[i];
  }
  while (header->read_header (stream, &crc));

  delete buffer;

  uint32 range_violations = filter1->violations ();
  if (mode != single_channel && which_channels == both)
   range_violations += filter2->violations ();

  if (filter_check)
  {
    // check whether (one of) the filter(s) produced values not in [-1.0, 1.0]:
    if (range_violations)
    {
      cerr << range_violations << " range violations have occured!\n";
      if (stdout_mode)
	cerr << "If you notice these violations,\n";
      else
	cerr << "If you have noticed these violations,\n";
      cerr << "please use the -f option with the value ";
      if (mode != single_channel && which_channels == both &&
	  filter2->hardest_violation () > filter1->hardest_violation ())
	cerr << filter2->recommended_scalefactor ();
      else
	cerr << filter1->recommended_scalefactor ();
      cerr << "\nor a greater value up to 32768 and try again.\n";
    }
  }
  if (verbose_mode)
  {
    // print playtime of stream:
    real playtime = filter1->seconds_played (Header::frequency (sample_frequency));
    uint32 minutes = (uint32)(playtime / 60.0);
    uint32 seconds = (uint32)playtime - minutes * 60;
    uint32 centiseconds = (uint32)((playtime - (real)(minutes * 60) - (real)seconds) * 100.0);
    cerr << "end of stream, playtime: " << minutes << ':';
    cerr.width (2);
    cerr.fill ('0');
    cerr << seconds << '.';
    cerr.width (2);
    cerr.fill ('0');
    cerr << centiseconds << '\n';
  }

  return 0;
}
