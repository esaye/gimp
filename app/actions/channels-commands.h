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

#ifndef __CHANNELS_COMMANDS_H__
#define __CHANNELS_COMMANDS_H__


void   channels_edit_attributes_cmd_callback (GtkAction   *action,
                                              gpointer     data);
void   channels_new_cmd_callback             (GtkAction   *action,
                                              gpointer     data);

void   channels_raise_cmd_callback           (GtkAction   *action,
                                              gpointer     data);
void   channels_raise_to_top_cmd_callback    (GtkAction   *action,
                                              gpointer     data);
void   channels_lower_cmd_callback           (GtkAction   *action,
                                              gpointer     data);
void   channels_lower_to_bottom_cmd_callback (GtkAction   *action,
                                              gpointer     data);

void   channels_duplicate_cmd_callback       (GtkAction   *action,
                                              gpointer     data);
void   channels_delete_cmd_callback          (GtkAction   *action,
                                              gpointer     data);
void   channels_to_selection_cmd_callback    (GtkAction   *action,
                                              gint         value,
                                              gpointer     data);

void   channels_new_channel_query            (GimpImage   *gimage,
                                              GimpContext *context,
                                              GimpChannel *template,
                                              gboolean     interactive,
                                              GtkWidget   *parent);
void   channels_edit_channel_query           (GimpChannel *channel,
                                              GimpContext *context,
                                              GtkWidget   *parent);


#endif /* __CHANNELS_COMMANDS_H__ */
