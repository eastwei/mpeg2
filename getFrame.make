# Generated automatically from Makefile.in by configure.
CC            = gcc
OPT           = -O6
DEFS          = -DHAVE_CONFIG_H
INCLUDE       = -I..
CFLAGS        = $(OPT) $(DEFS) $(INCLUDE)
LDFLAGS       =
SHELL         = /bin/sh

LIBRARY       = ../libmpeg.a

getFrame:	$(LIBRARY) getFrame.o ParseArgv.o
		$(CC) $(LDFLAGS) getFrame.o ParseArgv.o -o $@ -L.. -lmpeg
easympeg:	$(LIBRARY) easympeg.o
		$(CC) $(LDFLAGS) easympeg.o -o $@ $(LIBRARY) -lgl_s

getFrame.o easympeg.o: Makefile

#clean:
#rm -f mpegtest easympeg
#rm -f mpegtest.o ParseArgv.o easympeg.o mpegtest easympeg
