/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpfontselectbutton.c
 * Copyright (C) 2003  Sven Neumann  <sven@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "gimp.h"

#include "gimpuitypes.h"
#include "gimpfontselectbutton.h"
#include "gimpuimarshal.h"

#include "libgimp-intl.h"


#define GIMP_FONT_SELECT_BUTTON_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GIMP_TYPE_FONT_SELECT_BUTTON, GimpFontSelectButtonPrivate))

typedef struct _GimpFontSelectButtonPrivate GimpFontSelectButtonPrivate;

struct _GimpFontSelectButtonPrivate
{
  gchar       *title;

  gchar       *font_name;      /* local copy */

  GtkWidget   *inside;
  GtkWidget   *label;
};

enum
{
  FONT_SET,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_TITLE,
  PROP_FONT_NAME
};


/*  local function prototypes  */

static void   gimp_font_select_button_finalize     (GObject      *object);

static void   gimp_font_select_button_set_property (GObject      *object,
                                                    guint         property_id,
                                                    const GValue *value,
                                                    GParamSpec   *pspec);
static void   gimp_font_select_button_get_property (GObject      *object,
                                                    guint         property_id,
                                                    GValue       *value,
                                                    GParamSpec   *pspec);

static void   gimp_font_select_button_clicked  (GimpFontSelectButton *button);

static void   gimp_font_select_button_callback (const gchar *font_name,
                                                gboolean     dialog_closing,
                                                gpointer     user_data);

static void   gimp_font_select_drag_data_received (GimpFontSelectButton *button,
                                                   GdkDragContext       *context,
                                                   gint                  x,
                                                   gint                  y,
                                                   GtkSelectionData     *selection,
                                                   guint                 info,
                                                   guint                 time);

static GtkWidget * gimp_font_select_button_create_inside (GimpFontSelectButton *button);


static const GtkTargetEntry target = { "application/x-gimp-font-name", 0 };

static guint font_button_signals[LAST_SIGNAL] = { 0 };


G_DEFINE_TYPE (GimpFontSelectButton, gimp_font_select_button,
               GIMP_TYPE_SELECT_BUTTON)


static void
gimp_font_select_button_class_init (GimpFontSelectButtonClass *klass)
{
  GObjectClass          *object_class        = G_OBJECT_CLASS (klass);
  GimpSelectButtonClass *select_button_class = GIMP_SELECT_BUTTON_CLASS (klass);

  object_class->finalize     = gimp_font_select_button_finalize;
  object_class->set_property = gimp_font_select_button_set_property;
  object_class->get_property = gimp_font_select_button_get_property;

  select_button_class->select_destroy = gimp_font_select_destroy;

  klass->font_set = NULL;

  /**
   * GimpFontSelectButton:title:
   *
   * The title to be used for the font selection popup dialog.
   *
   * Since: GIMP 2.4
   */
  g_object_class_install_property (object_class, PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "Title",
                                                        "The title to be used for the font selection popup dialog",
                                                        _("Font Selection"),
                                                        GIMP_PARAM_READWRITE |
                                                        G_PARAM_CONSTRUCT_ONLY));

  /**
   * GimpFontSelectButton:font-name:
   *
   * The name of the currently selected font.
   *
   * Since: GIMP 2.4
   */
  g_object_class_install_property (object_class, PROP_FONT_NAME,
                                   g_param_spec_string ("font-name",
                                                        "Font name",
                                                        "The name of the currently selected font",
                                                        _("Sans"),
                                                        GIMP_PARAM_READWRITE));

  /**
   * GimpFontSelectButton::font-set:
   * @widget: the object which received the signal.
   * @font_name: the name of the currently selected font.
   * @dialog_closing: whether the dialog was closed or not.
   *
   * The ::font-set signal is emitted when the user selects a font.
   *
   * Since: GIMP 2.4
   */
  font_button_signals[FONT_SET] =
    g_signal_new ("font-set",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (GimpFontSelectButtonClass, font_set),
                  NULL, NULL,
                  _gimpui_marshal_VOID__STRING_BOOLEAN,
                  G_TYPE_NONE, 2,
                  G_TYPE_STRING,
                  G_TYPE_BOOLEAN);

  g_type_class_add_private (object_class,
                            sizeof (GimpFontSelectButtonPrivate));
}

static void
gimp_font_select_button_init (GimpFontSelectButton *button)
{
  GimpFontSelectButtonPrivate *priv;

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (button);

  priv->font_name = NULL;

  priv->inside = gimp_font_select_button_create_inside (button);
  gtk_container_add (GTK_CONTAINER (button), priv->inside);
}

/**
 * gimp_font_select_button_new:
 * @title:     Title of the dialog to use or %NULL to use the default title.
 * @font_name: Initial font name.
 *
 * Creates a new #GtkWidget that completely controls the selection of
 * a font.  This widget is suitable for placement in a table in a
 * plug-in dialog.
 *
 * Returns: A #GtkWidget that you can use in your UI.
 *
 * Since: GIMP 2.4
 */
GtkWidget *
gimp_font_select_button_new (const gchar *title,
                             const gchar *font_name)
{
  GtkWidget *button;

  if (title)
    button = g_object_new (GIMP_TYPE_FONT_SELECT_BUTTON,
                           "title",     title,
                           "font-name", font_name,
                           NULL);
  else
    button = g_object_new (GIMP_TYPE_FONT_SELECT_BUTTON,
                           "font-name", font_name,
                           NULL);

  return button;
}

/**
 * gimp_font_select_button_get_font:
 * @button: A #GimpFontSelectButton
 *
 * Retrieves the name of currently selected font.
 *
 * Returns: an internal copy of the font name which must not be freed.
 *
 * Since: GIMP 2.4
 */
G_CONST_RETURN gchar *
gimp_font_select_button_get_font (GimpFontSelectButton *button)
{
  GimpFontSelectButtonPrivate *priv;

  g_return_val_if_fail (GIMP_IS_FONT_SELECT_BUTTON (button), NULL);

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (button);
  return priv->font_name;
}

/**
 * gimp_font_select_button_set_font:
 * @button: A #GimpFontSelectButton
 * @font_name: Font name to set; %NULL means no change.
 *
 * Sets the current font for the font select button.
 *
 * Since: GIMP 2.4
 */
void
gimp_font_select_button_set_font (GimpFontSelectButton *button,
                                  const gchar          *font_name)
{
  GimpSelectButton *select_button;

  g_return_if_fail (GIMP_IS_FONT_SELECT_BUTTON (button));

  select_button = GIMP_SELECT_BUTTON (button);

  if (select_button->temp_callback)
    gimp_fonts_set_popup (select_button->temp_callback, font_name);
  else
    gimp_font_select_button_callback (font_name, FALSE, button);
}


/*  private functions  */

static void
gimp_font_select_button_finalize (GObject *object)
{
  GimpFontSelectButtonPrivate *priv;

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (object);

  g_free (priv->font_name);
  priv->font_name = NULL;

  g_free (priv->title);
  priv->title = NULL;

  G_OBJECT_CLASS (gimp_font_select_button_parent_class)->finalize (object);
}

static void
gimp_font_select_button_set_property (GObject      *object,
                                      guint         property_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  GimpFontSelectButton        *button = GIMP_FONT_SELECT_BUTTON (object);
  GimpFontSelectButtonPrivate *priv;

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (button);

  switch (property_id)
    {
    case PROP_TITLE:
      priv->title = g_value_dup_string (value);
      break;
    case PROP_FONT_NAME:
      gimp_font_select_button_set_font (button,
                                        g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_font_select_button_get_property (GObject    *object,
                                      guint       property_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  GimpFontSelectButton        *button = GIMP_FONT_SELECT_BUTTON (object);
  GimpFontSelectButtonPrivate *priv;

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (button);

  switch (property_id)
    {
    case PROP_TITLE:
      g_value_set_string (value, priv->title);
      break;
    case PROP_FONT_NAME:
      g_value_set_string (value, priv->font_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_font_select_button_callback (const gchar *font_name,
                                  gboolean     dialog_closing,
                                  gpointer     user_data)
{
  GimpFontSelectButton        *button;
  GimpFontSelectButtonPrivate *priv;
  GimpSelectButton            *select_button;

  button = GIMP_FONT_SELECT_BUTTON (user_data);

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (button);
  select_button = GIMP_SELECT_BUTTON (button);

  g_free (priv->font_name);
  priv->font_name = g_strdup (font_name);

  gtk_label_set_text (GTK_LABEL (priv->label), font_name);

  if (dialog_closing)
    select_button->temp_callback = NULL;

  g_signal_emit (button, font_button_signals[FONT_SET], 0,
                 font_name, dialog_closing);
  g_object_notify (G_OBJECT (button), "font-name");
}

static void
gimp_font_select_button_clicked (GimpFontSelectButton *button)
{
  GimpFontSelectButtonPrivate *priv;
  GimpSelectButton            *select_button;

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (button);
  select_button = GIMP_SELECT_BUTTON (button);

  if (select_button->temp_callback)
    {
      /*  calling gimp_fonts_set_popup() raises the dialog  */
      gimp_fonts_set_popup (select_button->temp_callback,
                            priv->font_name);
    }
  else
    {
      select_button->temp_callback =
        gimp_font_select_new (priv->title, priv->font_name,
                              gimp_font_select_button_callback,
                              button);
    }
}

static void
gimp_font_select_drag_data_received (GimpFontSelectButton *button,
                                     GdkDragContext       *context,
                                     gint                  x,
                                     gint                  y,
                                     GtkSelectionData     *selection,
                                     guint                 info,
                                     guint                 time)
{
  gchar *str;

  if ((selection->format != 8) || (selection->length < 1))
    {
      g_warning ("Received invalid font data!");
      return;
    }

  str = g_strndup ((const gchar *) selection->data, selection->length);

  if (g_utf8_validate (str, -1, NULL))
    {
      gint     pid;
      gpointer unused;
      gint     name_offset = 0;

      if (sscanf (str, "%i:%p:%n", &pid, &unused, &name_offset) >= 2 &&
          pid == gimp_getpid () && name_offset > 0)
        {
          gchar *name = str + name_offset;

          gimp_font_select_button_set_font (button, name);
        }
    }

  g_free (str);
}

static GtkWidget *
gimp_font_select_button_create_inside (GimpFontSelectButton *font_button)
{
  GtkWidget                   *button;
  GtkWidget                   *hbox;
  GtkWidget                   *image;
  GimpFontSelectButtonPrivate *priv;

  priv = GIMP_FONT_SELECT_BUTTON_GET_PRIVATE (font_button);

  gtk_widget_push_composite_child ();

  button = gtk_button_new ();

  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (button), hbox);

  image = gtk_image_new_from_stock (GIMP_STOCK_FONT, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  priv->label = gtk_label_new (priv->font_name);
  gtk_box_pack_start (GTK_BOX (hbox), priv->label, TRUE, TRUE, 4);

  gtk_widget_show_all (button);

  g_signal_connect_swapped (button, "clicked",
                            G_CALLBACK (gimp_font_select_button_clicked),
                            font_button);

  gtk_drag_dest_set (GTK_WIDGET (button),
                     GTK_DEST_DEFAULT_HIGHLIGHT |
                     GTK_DEST_DEFAULT_MOTION |
                     GTK_DEST_DEFAULT_DROP,
                     &target, 1,
                     GDK_ACTION_COPY);

  g_signal_connect_swapped (button, "drag-data-received",
                            G_CALLBACK (gimp_font_select_drag_data_received),
                            font_button);

  gtk_widget_pop_composite_child ();

  return button;
}
