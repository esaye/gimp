/* LIBGIMP - The GIMP Library 
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpcolorbutton.c
 * Copyright (C) 1999-2001 Sven Neumann
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

#include "libgimpcolor/gimpcolor.h"

#include "gimpwidgetstypes.h"

#include "gimpcolorarea.h"
#include "gimpcolorbutton.h"

#include "libgimp/gimppalette_pdb.h"

#include "libgimp/libgimp-intl.h"


#define TODOUBLE(i) (i / 65535.0)
#define TOUINT16(d) ((guint16) (d * 65535 + 0.5))


typedef enum
{
  GIMP_COLOR_BUTTON_COLOR_FG,
  GIMP_COLOR_BUTTON_COLOR_BG,
  GIMP_COLOR_BUTTON_COLOR_BLACK,
  GIMP_COLOR_BUTTON_COLOR_WHITE,
} GimpColorButtonColor;

enum
{
  COLOR_CHANGED,
  LAST_SIGNAL
};


static void     gimp_color_button_class_init     (GimpColorButtonClass *klass);
static void     gimp_color_button_init           (GimpColorButton      *gcb);
static void     gimp_color_button_destroy        (GtkObject            *object);

static gboolean gimp_color_button_button_press   (GtkWidget      *widget,
                                                  GdkEventButton *bevent);
static void     gimp_color_button_state_changed  (GtkWidget      *widget,
                                                  GtkStateType    prev_state);
static void     gimp_color_button_clicked        (GtkButton      *button);

static void     gimp_color_button_dialog_ok      (GtkWidget      *widget,
                                                  gpointer        data);
static void     gimp_color_button_dialog_cancel  (GtkWidget      *widget,
                                                  gpointer        data);

static void     gimp_color_button_use_color      (gpointer        callback_data,
                                                  guint           callback_action, 
                                                  GtkWidget      *widget);
static gchar  * gimp_color_button_menu_translate (const gchar    *path,
                                                  gpointer        func_data);

static void     gimp_color_button_color_changed  (GtkObject      *object,
                                                  gpointer        data);


static GtkItemFactoryEntry menu_items[] =
{
  { N_("/Foreground Color"), NULL, 
    gimp_color_button_use_color, GIMP_COLOR_BUTTON_COLOR_FG, NULL },
  { N_("/Background Color"), NULL, 
    gimp_color_button_use_color, GIMP_COLOR_BUTTON_COLOR_BG, NULL },
  { "/fg-bg-separator", NULL, NULL, 0, "<Separator>"},
  { N_("/Black"), NULL, 
    gimp_color_button_use_color, GIMP_COLOR_BUTTON_COLOR_BLACK, NULL },
  { N_("/White"), NULL, 
    gimp_color_button_use_color, GIMP_COLOR_BUTTON_COLOR_WHITE, NULL },
};

static guint   gimp_color_button_signals[LAST_SIGNAL] = { 0 };

static GimpButtonClass * parent_class = NULL;


GType
gimp_color_button_get_type (void)
{
  static GType gcb_type = 0;

  if (!gcb_type)
    {
      static const GTypeInfo gcb_info =
      {
        sizeof (GimpColorButtonClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) gimp_color_button_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (GimpColorButton),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) gimp_color_button_init,
      };

      gcb_type = g_type_register_static (GIMP_TYPE_BUTTON,
                                         "GimpColorButton", 
                                         &gcb_info, 0);
    }
  
  return gcb_type;
}

static void
gimp_color_button_class_init (GimpColorButtonClass *klass)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkButtonClass *button_class;

  object_class = GTK_OBJECT_CLASS (klass);
  widget_class = GTK_WIDGET_CLASS (klass);
  button_class = GTK_BUTTON_CLASS (klass);
 
  parent_class = g_type_class_peek_parent (klass);

  gimp_color_button_signals[COLOR_CHANGED] = 
    g_signal_new ("color_changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GimpColorButtonClass, color_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  object_class->destroy            = gimp_color_button_destroy;

  widget_class->button_press_event = gimp_color_button_button_press;
  widget_class->state_changed      = gimp_color_button_state_changed;

  button_class->clicked            = gimp_color_button_clicked;

  klass->color_changed             = NULL;
}

static void
gimp_color_button_init (GimpColorButton *gcb)
{
  GimpRGB color;

  gcb->title  = NULL;
  gcb->dialog = NULL;

  gimp_rgba_set (&color, 0.0, 0.0, 0.0, 1.0);
  gcb->color_area = gimp_color_area_new (&color, FALSE, GDK_BUTTON2_MASK);
  g_signal_connect (G_OBJECT (gcb->color_area), "color_changed",
		    G_CALLBACK (gimp_color_button_color_changed),
		    gcb);

  gtk_container_add (GTK_CONTAINER (gcb), gcb->color_area);
  gtk_widget_show (gcb->color_area);
  
  /* right-click opens a popup */
  gcb->item_factory = gtk_item_factory_new (GTK_TYPE_MENU, "<popup>", NULL);  
  gtk_item_factory_set_translate_func (gcb->item_factory,
				       gimp_color_button_menu_translate,
	  			       NULL, NULL);
  gtk_item_factory_create_items (gcb->item_factory, 
				 G_N_ELEMENTS (menu_items), menu_items, gcb);
}

static void
gimp_color_button_destroy (GtkObject *object)
{
  GimpColorButton *gcb;

  gcb = GIMP_COLOR_BUTTON (object);

  if (gcb->title)
    {
      g_free (gcb->title);
      gcb->title = NULL;
    }

  if (gcb->dialog)
    {
      gtk_widget_destroy (gcb->dialog);
      gcb->dialog = NULL;
    }

  if (gcb->color_area)
    {
      gtk_widget_destroy (gcb->color_area);
      gcb->color_area = NULL;
    }
  
  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static gboolean
gimp_color_button_button_press (GtkWidget      *widget,
                                GdkEventButton *bevent)
{
  GimpColorButton *gcb;
  gint             x;
  gint             y;

  gcb = GIMP_COLOR_BUTTON (widget);
  
  if (bevent->button == 3)
    {
      gdk_window_get_origin (GTK_WIDGET (widget)->window, &x, &y);
      x += widget->allocation.x;
      y += widget->allocation.y;

      gtk_item_factory_popup (gcb->item_factory,
                              x + bevent->x, y + bevent->y,
                              bevent->button, bevent->time);
    }

  if (GTK_WIDGET_CLASS (parent_class)->button_press_event)
    return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, bevent);

  return FALSE;
}

static void
gimp_color_button_state_changed (GtkWidget    *widget,
				 GtkStateType  prev_state)
{  
  g_return_if_fail (GIMP_IS_COLOR_BUTTON (widget));

  if (! GTK_WIDGET_IS_SENSITIVE (widget) && GIMP_COLOR_BUTTON (widget)->dialog)
    gtk_widget_hide (GIMP_COLOR_BUTTON (widget)->dialog);

  if (GTK_WIDGET_CLASS (parent_class)->state_changed)
    GTK_WIDGET_CLASS (parent_class)->state_changed (widget, prev_state);
}

/**
 * gimp_color_button_new:
 * @title: String that will be used as title for the color_selector.
 * @width: Width of the colorpreview in pixels.
 * @height: Height of the colorpreview in pixels.
 * @color: A pointer to a #GimpRGB color.
 * @type: 
 * 
 * Creates a new #GimpColorButton widget.
 *
 * This returns a button with a preview showing the color.
 * When the button is clicked a GtkColorSelectionDialog is opened.
 * If the user changes the color the new color is written into the
 * array that was used to pass the initial color and the "color_changed"
 * signal is emitted.
 * 
 * Returns: Pointer to the new #GimpColorButton widget.
 **/
GtkWidget *
gimp_color_button_new (const gchar       *title,
		       gint               width,
		       gint               height,
		       const GimpRGB     *color,
		       GimpColorAreaType  type)
{
  GimpColorButton *gcb;
  
  g_return_val_if_fail (color != NULL, NULL);  

  gcb = g_object_new (GIMP_TYPE_COLOR_BUTTON, NULL);

  gcb->title = g_strdup (title);
  
  gtk_widget_set_size_request (GTK_WIDGET (gcb->color_area), width, height);

  gimp_color_area_set_color (GIMP_COLOR_AREA (gcb->color_area), color);
  gimp_color_area_set_type (GIMP_COLOR_AREA (gcb->color_area), type);

  return GTK_WIDGET (gcb);
}

/**
 * gimp_color_button_set_color:
 * @gcb: Pointer to a #GimpColorButton.
 * @color: Pointer to the new #GimpRGB color.
 * 
 **/
void       
gimp_color_button_set_color (GimpColorButton *gcb,
			     const GimpRGB   *color)
{
  g_return_if_fail (GIMP_IS_COLOR_BUTTON (gcb));
  g_return_if_fail (color != NULL);

  gimp_color_area_set_color (GIMP_COLOR_AREA (gcb->color_area), color);
}

/**
 * gimp_color_button_get_color:
 * @gcb: Pointer to a #GimpColorButton.
 * @color:
 * 
 **/
void
gimp_color_button_get_color (GimpColorButton *gcb,
			     GimpRGB         *color)
{
  g_return_if_fail (GIMP_IS_COLOR_BUTTON (gcb));
  g_return_if_fail (color != NULL);

  gimp_color_area_get_color (GIMP_COLOR_AREA (gcb->color_area), color);
}

/**
 * gimp_color_button_has_alpha:
 * @gcb: Pointer to a #GimpColorButton.
 *
 *
 * Returns:
 **/
gboolean
gimp_color_button_has_alpha (GimpColorButton *gcb)
{
  g_return_val_if_fail (GIMP_IS_COLOR_BUTTON (gcb), FALSE);

  return gimp_color_area_has_alpha (GIMP_COLOR_AREA (gcb->color_area));
}

void
gimp_color_button_set_type (GimpColorButton   *gcb,
			    GimpColorAreaType  type)
{
  g_return_if_fail (GIMP_IS_COLOR_BUTTON (gcb));

  gimp_color_area_set_type (GIMP_COLOR_AREA (gcb->color_area), type);
}

static void
gimp_color_button_clicked (GtkButton *button)
{
  GimpColorButton *gcb;
  GimpRGB          color;
  GdkColor         gdk_color;
  guint16          alpha;

  g_return_if_fail (GIMP_IS_COLOR_BUTTON (button));

  gcb = GIMP_COLOR_BUTTON (button);

  gimp_color_button_get_color (gcb, &color);

  gdk_color.red   = TOUINT16 (color.r);
  gdk_color.green = TOUINT16 (color.g);
  gdk_color.blue  = TOUINT16 (color.b);
  alpha           = TOUINT16 (color.a);

  if (!gcb->dialog)
    {
      gcb->dialog = gtk_color_selection_dialog_new (gcb->title);

      gtk_color_selection_set_has_opacity_control (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->colorsel), TRUE);

      gtk_widget_destroy (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->help_button);
      gtk_container_set_border_width (GTK_CONTAINER (gcb->dialog), 2);

      g_signal_connect (G_OBJECT (gcb->dialog), "destroy",
			  G_CALLBACK (gtk_widget_destroyed),
			  &gcb->dialog);
      g_signal_connect (G_OBJECT (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->ok_button), 
                        "clicked",
                        G_CALLBACK (gimp_color_button_dialog_ok), 
                        gcb);
      g_signal_connect (G_OBJECT (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->cancel_button), 
                        "clicked",
                        G_CALLBACK (gimp_color_button_dialog_cancel), 
                        gcb);
      gtk_window_set_position (GTK_WINDOW (gcb->dialog), GTK_WIN_POS_MOUSE);  
    }

  gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->colorsel), &gdk_color);
  gtk_color_selection_set_current_alpha (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->colorsel), alpha);

  gtk_widget_show (gcb->dialog);
}

static void  
gimp_color_button_dialog_ok (GtkWidget *widget, 
			     gpointer   data)
{
  GimpColorButton *gcb;
  GimpRGB          color;
  GdkColor         gdk_color;
  guint16          alpha;

  g_return_if_fail (GIMP_IS_COLOR_BUTTON (data));

  gcb = GIMP_COLOR_BUTTON (data);

  gtk_color_selection_get_current_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->colorsel), &gdk_color);
  alpha = gtk_color_selection_get_current_alpha (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->colorsel));

  color.r = TODOUBLE (gdk_color.red);
  color.g = TODOUBLE (gdk_color.green);
  color.b = TODOUBLE (gdk_color.blue);
  color.a = TODOUBLE (alpha);

  gimp_color_button_set_color (gcb, &color);

  gtk_widget_hide (gcb->dialog);  
}

static void  
gimp_color_button_dialog_cancel (GtkWidget *widget, 
				 gpointer   data)
{
  g_return_if_fail (GIMP_IS_COLOR_BUTTON (data));

  gtk_widget_hide (GIMP_COLOR_BUTTON (data)->dialog);
}


static void  
gimp_color_button_use_color (gpointer   callback_data, 
                             guint      callback_action, 
                             GtkWidget *widget)
{
  GimpRGB              color;
  GimpColorButtonColor type;

  type = (GimpColorButtonColor) callback_action;

  gimp_color_button_get_color (GIMP_COLOR_BUTTON (callback_data), &color);

  switch (type)
    {
    case GIMP_COLOR_BUTTON_COLOR_FG:
      gimp_palette_get_foreground (&color);
      break;
    case GIMP_COLOR_BUTTON_COLOR_BG:
      gimp_palette_get_background (&color);
      break;
    case GIMP_COLOR_BUTTON_COLOR_BLACK:
      gimp_rgb_set (&color, 0.0, 0.0, 0.0);
      break;
    case GIMP_COLOR_BUTTON_COLOR_WHITE:
      gimp_rgb_set (&color, 1.0, 1.0, 1.0);
      break;
    }

  gimp_color_button_set_color (GIMP_COLOR_BUTTON (callback_data), &color);
}

static void
gimp_color_button_color_changed (GtkObject *object,
				 gpointer   data)
{
  GimpColorButton *gcb = GIMP_COLOR_BUTTON (data);

  if (gcb->dialog)
    {
      GimpRGB  color;
      GdkColor gdk_color;
      guint16  alpha;

      gimp_color_button_get_color (GIMP_COLOR_BUTTON (data), &color);
      
      gdk_color.red   = TOUINT16 (color.r);
      gdk_color.green = TOUINT16 (color.g);
      gdk_color.blue  = TOUINT16 (color.b);
      alpha           = TOUINT16 (color.a);

      gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->colorsel), &gdk_color);
      gtk_color_selection_set_current_alpha (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (gcb->dialog)->colorsel), alpha);
    }

  g_signal_emit (G_OBJECT (gcb), 
                 gimp_color_button_signals[COLOR_CHANGED], 0);
}

static gchar *
gimp_color_button_menu_translate (const gchar *path, 
				  gpointer     func_data)
{
  return (gettext (path));
}
