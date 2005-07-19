/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __GIMP_DISPLAY_SHELL_RENDER_H__
#define __GIMP_DISPLAY_SHELL_RENDER_H__


void   gimp_display_shell_render_init (Gimp             *gimp);
void   gimp_display_shell_render_exit (Gimp             *gimp);

void   gimp_display_shell_render      (GimpDisplayShell *shell,
                                       gint              x,
                                       gint              y,
                                       gint              w,
                                       gint              h,
                                       GdkRectangle     *highlight);


#endif  /*  __GIMP_DISPLAY_SHELL_RENDER_H__  */
