/*
 *   xmcd - Motif(tm) CD Audio Player
 *
 *   Copyright (C) 1995  Ti Kan
 *   E-mail: ti@amb.org
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#ifndef LINT
static char *_main_c_ident_ = "@(#)main.c	5.2 94/12/28";
#endif

#define _XMINC_

#include "common.d/appenv.h"
#include "common.d/util.h"
#include "xmcd.d/resource.h"
#include "xmcd.d/widget.h"
#include "xmcd.d/cdfunc.h"
#include "libdi.d/libdi.h"


/* Global data */
char			*progname;	/* The path name we are invoked with */
bool_t			exit_flag;	/* Flag indicating end of application */
appdata_t		app_data;	/* Options data */
widgets_t		widgets;	/* Holder of all widgets */
pixmaps_t		pixmaps;	/* Holder of all pixmaps */
FILE			*errfp = stderr;/* Error message stream */

/* Data global to this module only */
STATIC curstat_t	status;		/* Current CD player status */


/***********************
 *   public routines   *
 ***********************/

/*
 * curstat_addr
 *	Return the address of the curstat_t structure.
 *
 * Args:
 *	Nothing.
 *
 * Return:
 *	Nothing.
 */
curstat_t *
curstat_addr(void)
{
	return (&status);
}


/***********************
 *  internal routines  *
 ***********************/


/*
 * usage
 *	Display command line usage syntax
 *
 * Args:
 *	argc, argv
 *
 * Return:
 *	Nothing.
 */
STATIC void
usage(int argc, char **argv)
{
	int	i;

	fprintf(errfp, "%s\n", app_data.str_badopts);
	for (i = 1; i < argc; i++)
		fprintf(errfp, "%s ", argv[i]);

	fprintf(errfp, "\n\n%s %s [-dev device] [-debug]",
		app_data.str_usage, argv[0]);

#if defined(SVR4) && defined(sun)
	/* Solaris 2 volume manager auto-start support */
	fprintf(errfp, " [-c device] [-X] [-o]");
#endif

 	fprintf(errfp,
	    "\n\nStandard Xt Intrinsics and Motif options are supported.\n");
}


/*
 * main
 *	The main function
 */
void
main(int argc, char **argv)
{
	XtAppContext	app;
	XEvent		ev;

	/* Initialize variables */
	progname = argv[0];
	exit_flag = FALSE;

	/* Handle some signals */
	signal(SIGINT, onsig);
	signal(SIGHUP, onsig);
	signal(SIGTERM, onsig);

	/* Initialize X toolkit */
	widgets.toplevel = XtVaAppInitialize(
		&app,
		"XMcd",
		options, XtNumber(options),
		&argc, argv,
		fallbacks,
		XmNmappedWhenManaged, False,
		NULL
	);

	/* Get application options */
	XtVaGetApplicationResources(
		widgets.toplevel,
		(XtPointer) &app_data,
		resources,
		XtNumber(resources),
		NULL
	);
		
	/* Check command line for unknown arguments */
	if (argc > 1) {
		usage(argc, argv);
		exit(1);
	}

	/* Create all widgets */
	create_widgets(&widgets);

	/* Configure resources before realizing widgets */
	pre_realize_config(&widgets);

	/* Display widgets */
	XtRealizeWidget(widgets.toplevel);

	/* Configure resources after realizing widgets */
	post_realize_config(&widgets, &pixmaps);

	/* Register callback routines */
	register_callbacks(&widgets, &status);

	/* Initialize various subsystems */
	cd_init(&status);

	/* Start various subsystems */
	cd_start(&status);

	/* Make the main window visible */
	XtMapWidget(widgets.toplevel);

	/* Event processing loop */
	while (!exit_flag) {
		XtAppNextEvent(app, &ev);
		XtDispatchEvent(&ev);
	}

	exit(0);
}

