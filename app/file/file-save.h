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

#ifndef __FILE_SAVE_H__
#define __FILE_SAVE_H__


extern GSList *save_procs;


void   file_save_menu_init             (void);

void   file_save_callback              (GtkWidget   *widget,
                                        gpointer     data);
void   file_save_as_callback           (GtkWidget   *widget,
                                        gpointer     data);
void   file_save_a_copy_as_callback    (GtkWidget   *widget,
                                        gpointer     data);

void   file_save_by_extension_callback (GtkWidget   *widget,
                                        gpointer     data);


#endif /* __FILE_SAVE_H__ */
