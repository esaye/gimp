/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpprogress_pdb.h
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* NOTE: This file is autogenerated by pdbgen.pl */

#ifndef __GIMP_PROGRESS_PDB_H__
#define __GIMP_PROGRESS_PDB_H__

G_BEGIN_DECLS

/* For information look into the C source or the html documentation */


G_GNUC_INTERNAL gboolean _gimp_progress_init             (const gchar *message,
                                                          gint32       gdisplay_ID);
G_GNUC_INTERNAL gboolean _gimp_progress_update           (gdouble      percentage);
gboolean                 gimp_progress_pulse             (void);
gboolean                 gimp_progress_set_text          (const gchar *message);
gint                     gimp_progress_get_window_handle (void);
G_GNUC_INTERNAL gboolean _gimp_progress_install          (const gchar *progress_callback);
G_GNUC_INTERNAL gboolean _gimp_progress_uninstall        (const gchar *progress_callback);
gboolean                 gimp_progress_cancel            (const gchar *progress_callback);


G_END_DECLS

#endif /* __GIMP_PROGRESS_PDB_H__ */
