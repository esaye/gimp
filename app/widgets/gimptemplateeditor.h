/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimptemplateeditor.h
 * Copyright (C) 2002 Michael Natterer <mitch@gimp.org>
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

#ifndef __GIMP_TEMPLATE_EDITOR_H__
#define __GIMP_TEMPLATE_EDITOR_H__


#include <gtk/gtkvbox.h>


#define GIMP_TYPE_TEMPLATE_EDITOR            (gimp_template_editor_get_type ())
#define GIMP_TEMPLATE_EDITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_TEMPLATE_EDITOR, GimpTemplateEditor))
#define GIMP_TEMPLATE_EDITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_TEMPLATE_EDITOR, GimpTemplateEditorClass))
#define GIMP_IS_TEMPLATE_EDITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_TEMPLATE_EDITOR))
#define GIMP_IS_TEMPLATE_EDITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_TEMPLATE_EDITOR))
#define GIMP_TEMPLATE_EDITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GIMP_TYPE_TEMPLATE_EDITOR, GimpTemplateEditorClass))


typedef struct _GimpTemplateEditorClass GimpTemplateEditorClass;

struct _GimpTemplateEditor
{
  GtkVBox        parent_instance;

  GimpTemplate  *template;

  GimpContainer *stock_id_container;
  GimpContext   *stock_id_context;

  GtkWidget     *aspect_button;
  gboolean       block_aspect;

  GtkWidget     *expander;
  GtkWidget     *size_se;
  GtkWidget     *memsize_label;
  GtkWidget     *pixel_label;
  GtkWidget     *resolution_se;
};

struct _GimpTemplateEditorClass
{
  GtkVBoxClass   parent_class;
};


GType       gimp_template_editor_get_type      (void) G_GNUC_CONST;

GtkWidget * gimp_template_editor_new           (GimpTemplate       *template,
                                                Gimp               *gimp,
                                                gboolean            edit_template);

void        gimp_template_editor_show_advanced (GimpTemplateEditor *editor,
                                                gboolean            expanded);


#endif  /*  __GIMP_TEMPLATE_EDITOR_H__  */
