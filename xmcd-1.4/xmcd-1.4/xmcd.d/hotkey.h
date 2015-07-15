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
#ifndef __HOTKEY_H__
#define __HOTKEY_H__

#ifndef LINT
static char *_hotkey_h_ident_ = "@(#)hotkey.h	5.2 94/12/28";
#endif

#define MAX_TRANSLATIONS_SZ	4096

/* Public functions */
extern void	hotkey_setup(widgets_t *);
extern void	hotkey_grabkeys(Widget);
extern void	hotkey_ungrabkeys(Widget);
extern void	hotkey(Widget, XEvent *, String *, Cardinal *);

#endif	/* __HOTKEY_H__ */
