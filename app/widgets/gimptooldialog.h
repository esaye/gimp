/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimptooldialog.h
 * Copyright (C) 2003  Sven Neumann <sven@gimp.org>
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

#ifndef __GIMP_TOOL_DIALOG_H__
#define __GIMP_TOOL_DIALOG_H__

#include "widgets/gimpviewabledialog.h"


#define GIMP_TYPE_TOOL_DIALOG            (gimp_tool_dialog_get_type ())
#define GIMP_TOOL_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_TOOL_DIALOG, GimpToolDialog))
#define GIMP_TOOL_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_TOOL_DIALOG, GimpToolDialogClass))
#define GIMP_IS_TOOL_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_TOOL_DIALOG))
#define GIMP_IS_TOOL_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_TOOL_DIALOG))
#define GIMP_TOOL_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_TOOL_DIALOG, GimpToolDialogClass))


typedef struct _GimpViewableDialogClass  GimpToolDialogClass;
typedef struct _GimpViewableDialog       GimpToolDialog;


GType       gimp_tool_dialog_get_type (void) G_GNUC_CONST;

GtkWidget * gimp_tool_dialog_new      (GimpToolInfo *tool_info,
                                       GtkWidget    *parent,
                                       const gchar  *desc,
                                       ...);


#endif /* __GIMP_TOOL_DIALOG_H__ */
