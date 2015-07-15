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
#ifndef __HELP_H__
#define __HELP_H__

#ifndef LINT
static char *_help_h_ident_ = "@(#)help.h	5.2 94/12/28";
#endif

/* Max number of widgets supported by the help system */
#define MAX_HELP_WIDGETS	96

/* Public functions */
extern void	help_setup(widgets_t *);
extern void	help_popup(Widget);

#endif	/* __HELP_H__ */
