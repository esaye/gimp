/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimpeditor.c
 * Copyright (C) 2001 Michael Natterer <mitch@gimp.org>
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

#include "gimpeditor.h"
#include "gimpdnd.h"
#include "gimpenummenu.h"
#include "gimpmenufactory.h"


#define DEFAULT_CONTENT_SPACING  2
#define DEFAULT_BUTTON_SPACING   2
#define DEFAULT_BUTTON_ICON_SIZE GTK_ICON_SIZE_MENU


static void        gimp_editor_class_init        (GimpEditorClass *klass);
static void        gimp_editor_init              (GimpEditor      *panel);

static void        gimp_editor_destroy           (GtkObject       *object);
static void        gimp_editor_style_set         (GtkWidget       *widget,
                                                  GtkStyle        *prev_style);
static GtkIconSize gimp_editor_ensure_button_box (GimpEditor      *editor);


static GtkVBoxClass *parent_class = NULL;


GType
gimp_editor_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      static const GTypeInfo info =
      {
        sizeof (GimpEditorClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gimp_editor_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GimpEditor),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gimp_editor_init,
      };

      type = g_type_register_static (GTK_TYPE_VBOX,
                                     "GimpEditor",
                                     &info, 0);
    }

  return type;
}

static void
gimp_editor_class_init (GimpEditorClass *klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = GTK_OBJECT_CLASS (klass);
  widget_class = GTK_WIDGET_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->destroy   = gimp_editor_destroy;

  widget_class->style_set = gimp_editor_style_set;

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("content_spacing",
                                                             NULL, NULL,
                                                             0,
                                                             G_MAXINT,
                                                             DEFAULT_CONTENT_SPACING,
                                                             G_PARAM_READABLE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_int ("button_spacing",
                                                             NULL, NULL,
                                                             0,
                                                             G_MAXINT,
                                                             DEFAULT_BUTTON_SPACING,
                                                             G_PARAM_READABLE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_enum ("button_icon_size",
                                                              NULL, NULL,
                                                              GTK_TYPE_ICON_SIZE,
                                                              DEFAULT_BUTTON_ICON_SIZE,
                                                              G_PARAM_READABLE));
}

static void
gimp_editor_init (GimpEditor *editor)
{
  editor->menu_factory = NULL;
  editor->item_factory = NULL;
  editor->button_box   = NULL;
}

static void
gimp_editor_destroy (GtkObject *object)
{
  GimpEditor *editor;

  editor = GIMP_EDITOR (object);

  if (editor->item_factory)
    {
      g_object_unref (editor->item_factory);
      editor->item_factory = NULL;
    }

  GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
gimp_editor_style_set (GtkWidget *widget,
                       GtkStyle  *prev_style)
{
  GimpEditor  *editor;
  GtkIconSize  button_icon_size;
  gint         button_spacing;
  gint         content_spacing;

  editor = GIMP_EDITOR (widget);

  gtk_widget_style_get (widget,
                        "button_icon_size", &button_icon_size,
			"button_spacing",   &button_spacing,
                        "content_spacing",  &content_spacing,
			NULL);

  gtk_box_set_spacing (GTK_BOX (widget), content_spacing);

  if (editor->button_box)
    {
      GList  *children;
      GList  *list;

      gtk_box_set_spacing (GTK_BOX (editor->button_box), button_spacing);

      children = gtk_container_get_children (GTK_CONTAINER (editor->button_box));

      for (list = children; list; list = g_list_next (list))
        {
          GtkBin *bin;
          gchar  *stock_id;

          bin = GTK_BIN (list->data);

          gtk_image_get_stock (GTK_IMAGE (bin->child), &stock_id, NULL);
          gtk_image_set_from_stock (GTK_IMAGE (bin->child),
                                    stock_id,
                                    button_icon_size);
        }

      g_list_free (children);
    }

  if (GTK_WIDGET_CLASS (parent_class)->style_set)
    GTK_WIDGET_CLASS (parent_class)->style_set (widget, prev_style);
}

GtkWidget *
gimp_editor_new (void)
{
  GimpEditor *editor;

  editor = g_object_new (GIMP_TYPE_EDITOR, NULL);

  return GTK_WIDGET (editor);
}

void
gimp_editor_create_menu (GimpEditor      *editor,
                         GimpMenuFactory *menu_factory,
                         const gchar     *menu_identifier,
                         gpointer         callback_data)
{
  g_return_if_fail (GIMP_IS_EDITOR (editor));
  g_return_if_fail (GIMP_IS_MENU_FACTORY (menu_factory));
  g_return_if_fail (menu_identifier != NULL);

  if (editor->item_factory)
    g_object_unref (editor->item_factory);

  editor->menu_factory = menu_factory;
  editor->item_factory = gimp_menu_factory_menu_new (menu_factory,
                                                     menu_identifier,
                                                     GTK_TYPE_MENU,
                                                     callback_data,
                                                     FALSE);
}

GtkWidget *
gimp_editor_add_button (GimpEditor  *editor,
                        const gchar *stock_id,
                        const gchar *tooltip,
                        const gchar *help_data,
                        GCallback    callback,
                        GCallback    extended_callback,
                        gpointer     callback_data)
{
  GtkWidget   *button;
  GtkWidget   *image;
  GtkIconSize  button_icon_size;

  g_return_val_if_fail (GIMP_IS_EDITOR (editor), NULL);
  g_return_val_if_fail (stock_id != NULL, NULL);

  button_icon_size = gimp_editor_ensure_button_box (editor);

  button = gimp_button_new ();
  gtk_box_pack_start (GTK_BOX (editor->button_box), button, TRUE, TRUE, 0);
  gtk_widget_show (button);

  if (tooltip || help_data)
    gimp_help_set_help_data (button, tooltip, help_data);

  if (callback)
    g_signal_connect (button, "clicked",
		      callback,
		      callback_data);

  if (extended_callback)
    g_signal_connect (button, "extended_clicked",
		      extended_callback,
		      callback_data);

  image = gtk_image_new_from_stock (stock_id, button_icon_size);
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_widget_show (image);

  return button;
}

GtkWidget *
gimp_editor_add_stock_box (GimpEditor  *editor,
                           GType        enum_type,
                           const gchar *stock_prefix,
                           GCallback    callback,
                           gpointer     callback_data)
{
  GtkWidget   *hbox;
  GtkWidget   *first_button;
  GtkIconSize  button_icon_size;
  GList       *children;
  GList       *list;

  g_return_val_if_fail (GIMP_IS_EDITOR (editor), NULL);
  g_return_val_if_fail (g_type_is_a (enum_type, G_TYPE_ENUM), NULL);
  g_return_val_if_fail (stock_prefix != NULL, NULL);

  button_icon_size = gimp_editor_ensure_button_box (editor);

  hbox = gimp_enum_stock_box_new (enum_type, stock_prefix, button_icon_size,
                                  callback, callback_data,
                                  &first_button);

  children = gtk_container_get_children (GTK_CONTAINER (hbox));

  for (list = children; list; list = g_list_next (list))
    {
      GtkWidget *button = list->data;

      g_object_ref (button);

      gtk_container_remove (GTK_CONTAINER (hbox), button);
      gtk_box_pack_start (GTK_BOX (editor->button_box), button,
                          TRUE, TRUE, 0);

      g_object_unref (button);
    }

  g_list_free (children);
  gtk_widget_destroy (hbox);

  return first_button;
}

static GtkIconSize
gimp_editor_ensure_button_box (GimpEditor *editor)
{
  GtkIconSize  button_icon_size;
  gint         button_spacing;

  gtk_widget_style_get (GTK_WIDGET (editor),
                        "button_icon_size", &button_icon_size,
                        "button_spacing",   &button_spacing,
                        NULL);

  if (! editor->button_box)
    {
      editor->button_box = gtk_hbox_new (TRUE, button_spacing);
      gtk_box_pack_end (GTK_BOX (editor), editor->button_box, FALSE, FALSE, 0);
      gtk_widget_show (editor->button_box);
    }

  return button_icon_size;
}
