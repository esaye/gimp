/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimptooldialog.c
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

#include "config.h"

#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "widgets-types.h"

#include "core/gimpobject.h"
#include "core/gimptoolinfo.h"

#include "gimpdialogfactory.h"
#include "gimptooldialog.h"


GType
gimp_tool_dialog_get_type (void)
{
  static GType dialog_type = 0;

  if (! dialog_type)
    {
      static const GTypeInfo dialog_info =
      {
        sizeof (GimpToolDialogClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    NULL,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (GimpToolDialog),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) NULL
      };

      dialog_type = g_type_register_static (GIMP_TYPE_VIEWABLE_DIALOG,
					    "GimpToolDialog",
					    &dialog_info, 0);
    }

  return dialog_type;
}


/**
 * gimp_tool_dialog_new:
 * @tool_info: a #GimpToolInfo
 * @parent:    the parent widget of this dialog or %NULL
 * @desc:      a string to use in the dialog header or %NULL to use the help
 *             field from #GimpToolInfo
 * @...:       a %NULL-terminated valist of button parameters as described in
 *             gtk_dialog_new_with_buttons().
 *
 * This function conveniently creates a #GimpViewableDialog using the
 * information stored in @tool_info. It also registers the tool with
 * the "toplevel" dialog factory.
 *
 * Return value: a new #GimpViewableDialog
 **/
GtkWidget *
gimp_tool_dialog_new (GimpToolInfo *tool_info,
                      GtkWidget    *parent,
                      const gchar  *desc,
                      ...)
{
  GtkWidget   *dialog;
  const gchar *stock_id;
  gchar       *identifier;
  va_list      args;

  g_return_val_if_fail (GIMP_IS_TOOL_INFO (tool_info), NULL);
  g_return_val_if_fail (parent == NULL || GTK_IS_WIDGET (parent), NULL);

  stock_id = gimp_viewable_get_stock_id (GIMP_VIEWABLE (tool_info));

  dialog = g_object_new (GIMP_TYPE_TOOL_DIALOG,
                         "title",       tool_info->blurb,
                         "role",        GIMP_OBJECT (tool_info)->name,
                         "stock_id",    stock_id,
                         "description", desc ? desc : tool_info->help,
                         "parent",      parent,
                         NULL);

  gimp_help_connect (GTK_WIDGET (dialog),
                     gimp_standard_help_func, tool_info->help_id, dialog);

  va_start (args, desc);
  gimp_dialog_add_buttons_valist (GIMP_DIALOG (dialog), args);
  va_end (args);

  identifier = g_strconcat (GIMP_OBJECT (tool_info)->name, "-dialog", NULL);

  gimp_dialog_factory_add_foreign (gimp_dialog_factory_from_name ("toplevel"),
                                   identifier,
                                   dialog);

  g_free (identifier);

  return dialog;
}
