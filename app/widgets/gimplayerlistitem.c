/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * gimplayerlistitem.c
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

#ifdef __GNUC__
#warning GTK_DISABLE_DEPRECATED
#endif
#undef GTK_DISABLE_DEPRECATED

#include <gtk/gtk.h>

#include "libgimpcolor/gimpcolor.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "widgets-types.h"

#include "core/gimpdrawable.h"
#include "core/gimpimage.h"
#include "core/gimplayer.h"
#include "core/gimplayermask.h"

#include "display/gimpdisplay-foreach.h"

#include "gimpdnd.h"
#include "gimplayerlistitem.h"
#include "gimppreview.h"


static void   gimp_layer_list_item_class_init (GimpLayerListItemClass  *klass);
static void   gimp_layer_list_item_init       (GimpLayerListItem       *list_item);

static void      gimp_layer_list_item_set_viewable  (GimpListItem      *list_item,
                                                     GimpViewable      *viewable);
static void      gimp_layer_list_item_set_preview_size (GimpListItem   *list_item);

static gboolean  gimp_layer_list_item_drag_motion   (GtkWidget         *widget,
                                                     GdkDragContext    *context,
                                                     gint               x,
                                                     gint               y,
                                                     guint              time);
static gboolean  gimp_layer_list_item_drag_drop     (GtkWidget         *widget,
                                                     GdkDragContext    *context,
                                                     gint               x,
                                                     gint               y,
                                                     guint              time);
static void      gimp_layer_list_item_state_changed (GtkWidget         *widget,
                                                     GtkStateType       old_state);

static void      gimp_layer_list_item_linked_toggled (GtkWidget        *widget,
						      gpointer          data);
static void      gimp_layer_list_item_linked_changed (GimpLayer        *layer,
						      gpointer          data);

static void      gimp_layer_list_item_mask_changed  (GimpLayer         *layer,
                                                     GimpLayerListItem *layer_item);
static void      gimp_layer_list_item_update_state  (GtkWidget         *widget);

static void      gimp_layer_list_item_layer_clicked (GtkWidget         *widget,
                                                     GimpLayer         *layer);
static void      gimp_layer_list_item_mask_clicked  (GtkWidget         *widget,
                                                     GimpLayerMask     *mask);
static void      gimp_layer_list_item_mask_extended_clicked
                                                    (GtkWidget         *widget,
						     guint              state,
						     GimpLayerMask     *mask);


static GimpDrawableListItemClass *parent_class = NULL;

static GimpRGB  black_color;
static GimpRGB  white_color;
static GimpRGB  green_color;
static GimpRGB  red_color;


GType
gimp_layer_list_item_get_type (void)
{
  static GType list_item_type = 0;

  if (! list_item_type)
    {
      static const GTypeInfo list_item_info =
      {
        sizeof (GimpLayerListItemClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) gimp_layer_list_item_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (GimpLayerListItem),
        0,              /* n_preallocs */
        (GInstanceInitFunc) gimp_layer_list_item_init,
      };

      list_item_type = g_type_register_static (GIMP_TYPE_DRAWABLE_LIST_ITEM,
                                               "GimpLayerListItem",
                                               &list_item_info, 0);
    }

  return list_item_type;
}

static void
gimp_layer_list_item_class_init (GimpLayerListItemClass *klass)
{
  GtkWidgetClass    *widget_class;
  GimpListItemClass *list_item_class;

  widget_class    = GTK_WIDGET_CLASS (klass);
  list_item_class = GIMP_LIST_ITEM_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  widget_class->drag_motion         = gimp_layer_list_item_drag_motion;
  widget_class->drag_drop           = gimp_layer_list_item_drag_drop;
  widget_class->state_changed       = gimp_layer_list_item_state_changed;

  list_item_class->set_viewable     = gimp_layer_list_item_set_viewable;
  list_item_class->set_preview_size = gimp_layer_list_item_set_preview_size;

  gimp_rgba_set (&black_color, 0.0, 0.0, 0.0, 1.0);
  gimp_rgba_set (&white_color, 1.0, 1.0, 1.0, 1.0);
  gimp_rgba_set (&green_color, 0.0, 1.0, 0.0, 1.0);
  gimp_rgba_set (&red_color,   1.0, 0.0, 0.0, 1.0);
}

static void
gimp_layer_list_item_init (GimpLayerListItem *list_item)
{
  GtkWidget *abox;
  GtkWidget *image;

  GIMP_LIST_ITEM (list_item)->convertable = TRUE;

  abox = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_box_pack_start (GTK_BOX (GIMP_LIST_ITEM (list_item)->hbox), abox,
                      FALSE, FALSE, 0);
  gtk_box_reorder_child (GTK_BOX (GIMP_LIST_ITEM (list_item)->hbox), abox, 1);
  gtk_widget_show (abox);

  list_item->linked_button = gtk_toggle_button_new ();
  gtk_button_set_relief (GTK_BUTTON (list_item->linked_button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (abox), list_item->linked_button);
  gtk_widget_show (list_item->linked_button);

  g_signal_connect (G_OBJECT (list_item->linked_button), "realize",
		    G_CALLBACK (gimp_list_item_button_realize),
		    list_item);

  g_signal_connect (G_OBJECT (list_item->linked_button), "state_changed",
		    G_CALLBACK (gimp_list_item_button_state_changed),
		    list_item);

  image = gtk_image_new_from_stock (GIMP_STOCK_LINKED,
				    GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (list_item->linked_button), image);
  gtk_widget_show (image);

  list_item->mask_preview = NULL;
}

static void
gimp_layer_list_item_set_viewable (GimpListItem *list_item,
                                   GimpViewable *viewable)
{
  GimpLayerListItem *layer_item;
  GimpLayer         *layer;
  gboolean           linked;

  if (GIMP_LIST_ITEM_CLASS (parent_class)->set_viewable)
    GIMP_LIST_ITEM_CLASS (parent_class)->set_viewable (list_item, viewable);

  gimp_preview_set_size (GIMP_PREVIEW (list_item->preview),
                         list_item->preview_size, 2);

  layer_item = GIMP_LAYER_LIST_ITEM (list_item);
  layer      = GIMP_LAYER (GIMP_PREVIEW (list_item->preview)->viewable);
  linked     = gimp_layer_get_linked (layer);

  if (! linked)
    {
      GtkRequisition requisition;

      gtk_widget_size_request (layer_item->linked_button, &requisition);

      gtk_widget_set_size_request (layer_item->linked_button,
                                   requisition.width,
                                   requisition.height);
      gtk_widget_hide (GTK_BIN (layer_item->linked_button)->child);
    }

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (layer_item->linked_button),
                                linked);

  g_signal_connect (G_OBJECT (layer_item->linked_button), "toggled",
		    G_CALLBACK (gimp_layer_list_item_linked_toggled),
		    list_item);

  g_signal_connect_object (G_OBJECT (viewable), "linked_changed",
			   G_CALLBACK (gimp_layer_list_item_linked_changed),
			   G_OBJECT (list_item),
			   0);

  g_signal_connect (G_OBJECT (list_item->preview), "clicked",
		    G_CALLBACK (gimp_layer_list_item_layer_clicked),
		    layer);

  if (gimp_layer_get_mask (layer))
    {
      gimp_layer_list_item_mask_changed (layer, layer_item);
    }

  g_signal_connect_object (G_OBJECT (layer), "mask_changed",
			   G_CALLBACK (gimp_layer_list_item_mask_changed),
			   G_OBJECT (list_item),
			   0);
}

static void
gimp_layer_list_item_set_preview_size (GimpListItem *list_item)
{
  GimpLayerListItem *layer_item;

  if (GIMP_LIST_ITEM_CLASS (parent_class)->set_preview_size)
    GIMP_LIST_ITEM_CLASS (parent_class)->set_preview_size (list_item);

  layer_item = GIMP_LAYER_LIST_ITEM (list_item);

  if (layer_item->mask_preview)
    {
      GimpPreview *preview;

      preview = GIMP_PREVIEW (layer_item->mask_preview);

      gimp_preview_set_size (preview,
			     list_item->preview_size, preview->border_width);
    }
}


static gboolean
gimp_layer_list_item_drag_motion (GtkWidget      *widget,
                                  GdkDragContext *context,
                                  gint            x,
                                  gint            y,
                                  guint           time)
{
  GimpListItem  *list_item;
  GimpLayer     *layer;
  GimpViewable  *src_viewable;
  gint           dest_index;
  GdkDragAction  drag_action;
  GimpDropType   drop_type;
  gboolean       return_val;

  list_item = GIMP_LIST_ITEM (widget);
  layer     = GIMP_LAYER (GIMP_PREVIEW (list_item->preview)->viewable);

  return_val = gimp_list_item_check_drag (list_item, context, x, y,
                                          &src_viewable,
                                          &dest_index,
                                          &drag_action,
                                          &drop_type);

  if (! src_viewable                                           ||
      ! gimp_drawable_has_alpha (GIMP_DRAWABLE (src_viewable)) ||
      ! layer                                                  ||
      ! gimp_drawable_has_alpha (GIMP_DRAWABLE (layer)))
    {
      drag_action = GDK_ACTION_DEFAULT;
      drop_type   = GIMP_DROP_NONE;
      return_val  = FALSE;
    }

  gdk_drag_status (context, drag_action, time);

  if (list_item->drop_type != drop_type)
    {
      list_item->drop_type = drop_type;

      gtk_widget_queue_draw (widget);
    }

  return return_val;
}

static gboolean
gimp_layer_list_item_drag_drop (GtkWidget      *widget,
                                GdkDragContext *context,
                                gint            x,
                                gint            y,
                                guint           time)
{
  GimpListItem  *list_item;
  GimpLayer     *layer;
  GimpViewable  *src_viewable;
  gint           dest_index;
  GdkDragAction  drag_action;
  GimpDropType   drop_type;
  gboolean       return_val;

  list_item = GIMP_LIST_ITEM (widget);
  layer     = GIMP_LAYER (GIMP_PREVIEW (list_item->preview)->viewable);

  return_val = gimp_list_item_check_drag (list_item, context, x, y,
                                          &src_viewable,
                                          &dest_index,
                                          &drag_action,
                                          &drop_type);

  if (! gimp_drawable_has_alpha (GIMP_DRAWABLE (src_viewable)) ||
      ! gimp_drawable_has_alpha (GIMP_DRAWABLE (layer)))
    {
      drag_action = GDK_ACTION_DEFAULT;
      drop_type   = GIMP_DROP_NONE;
      return_val  = FALSE;

      gtk_drag_finish (context, return_val, FALSE, time);

      list_item->drop_type = GIMP_DROP_NONE;

      return return_val;
    }

  return GTK_WIDGET_CLASS (parent_class)->drag_drop (widget, context,
                                                     x, y, time);

  return return_val;
}

static void
gimp_layer_list_item_state_changed (GtkWidget    *widget,
                                    GtkStateType  old_state)
{
  if (GTK_WIDGET_CLASS (parent_class)->state_changed)
    GTK_WIDGET_CLASS (parent_class)->state_changed (widget, old_state);

  gimp_layer_list_item_update_state (widget);
}

static void
gimp_layer_list_item_linked_toggled (GtkWidget *widget,
                                     gpointer   data)
{
  GimpListItem *list_item;
  GimpLayer    *layer;
  gboolean      linked;

  list_item = GIMP_LIST_ITEM (data);
  layer     = GIMP_LAYER (GIMP_PREVIEW (list_item->preview)->viewable);
  linked    = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

  if (linked != gimp_layer_get_linked (layer))
    {
      if (! linked)
        {
          gtk_widget_set_size_request (GTK_WIDGET (widget),
                                       GTK_WIDGET (widget)->allocation.width,
                                       GTK_WIDGET (widget)->allocation.height);
          gtk_widget_hide (GTK_BIN (widget)->child);
        }
      else
        {
          gtk_widget_show (GTK_BIN (widget)->child);
          gtk_widget_set_size_request (GTK_WIDGET (widget), -1, -1);
        }

      g_signal_handlers_block_by_func (G_OBJECT (layer),
				       gimp_layer_list_item_linked_changed,
				       list_item);

      gimp_layer_set_linked (layer, linked);

      g_signal_handlers_unblock_by_func (G_OBJECT (layer),
					 gimp_layer_list_item_linked_changed,
					 list_item);
    }
}

static void
gimp_layer_list_item_linked_changed (GimpLayer *layer,
				     gpointer   data)
{
  GimpListItem    *list_item;
  GtkToggleButton *toggle;
  gboolean         linked;

  list_item = GIMP_LIST_ITEM (data);
  toggle    = GTK_TOGGLE_BUTTON (GIMP_LAYER_LIST_ITEM (data)->linked_button);
  linked    = gimp_layer_get_linked (layer);

  if (linked != toggle->active)
    {
      if (! linked)
        {
          gtk_widget_set_size_request (GTK_WIDGET (toggle),
                                       GTK_WIDGET (toggle)->allocation.width,
                                       GTK_WIDGET (toggle)->allocation.height);
          gtk_widget_hide (GTK_BIN (toggle)->child);
        }
      else
        {
          gtk_widget_show (GTK_BIN (toggle)->child);
          gtk_widget_set_size_request (GTK_WIDGET (toggle), -1, -1);
        }

      g_signal_handlers_block_by_func (G_OBJECT (toggle),
				       gimp_layer_list_item_linked_toggled,
				       list_item);

      gtk_toggle_button_set_active (toggle, linked);

      g_signal_handlers_unblock_by_func (G_OBJECT (toggle),
					 gimp_layer_list_item_linked_toggled,
					 list_item);
    }
}

static void
gimp_layer_list_item_mask_changed (GimpLayer         *layer,
                                   GimpLayerListItem *layer_item)
{
  GimpListItem  *list_item;
  GimpLayerMask *mask;

  list_item = GIMP_LIST_ITEM (layer_item);
  mask      = gimp_layer_get_mask (layer);

  if (mask && ! layer_item->mask_preview)
    {
      layer_item->mask_preview = gimp_preview_new (GIMP_VIEWABLE (mask),
                                                   list_item->preview_size,
                                                   2, FALSE);

      GIMP_PREVIEW (layer_item->mask_preview)->clickable = TRUE;

      gtk_box_pack_start (GTK_BOX (list_item->hbox), layer_item->mask_preview,
                          FALSE, FALSE, 0);
      gtk_box_reorder_child (GTK_BOX (list_item->hbox),
                             layer_item->mask_preview, 3);
      gtk_widget_show (layer_item->mask_preview);

      g_signal_connect (G_OBJECT (layer_item->mask_preview), "clicked",
			G_CALLBACK (gimp_layer_list_item_mask_clicked),
			mask);
      g_signal_connect (G_OBJECT (layer_item->mask_preview), "extended_clicked",
			G_CALLBACK (gimp_layer_list_item_mask_extended_clicked),
			mask);

      g_signal_connect_object (G_OBJECT (mask), "apply_changed",
			       G_CALLBACK (gimp_layer_list_item_update_state),
			       G_OBJECT (layer_item),
			       G_CONNECT_SWAPPED);

      g_signal_connect_object (G_OBJECT (mask), "edit_changed",
			       G_CALLBACK (gimp_layer_list_item_update_state),
			       G_OBJECT (layer_item),
			       G_CONNECT_SWAPPED);

      g_signal_connect_object (G_OBJECT (mask), "show_changed",
			       G_CALLBACK (gimp_layer_list_item_update_state),
			       G_OBJECT (layer_item),
			       G_CONNECT_SWAPPED);
    }
  else if (! mask && layer_item->mask_preview)
    {
      gtk_widget_destroy (layer_item->mask_preview);
      layer_item->mask_preview = NULL;
    }

  gimp_layer_list_item_update_state (GTK_WIDGET (layer_item));
}

static void
gimp_layer_list_item_update_state (GtkWidget *widget)
{
  GimpLayerListItem *layer_item;
  GimpListItem      *list_item;
  GimpLayer         *layer;
  GimpLayerMask     *mask;
  GimpPreview       *preview;
  GimpRGB           *layer_color;
  GimpRGB           *mask_color;
  GimpRGB            bg_color;

  gimp_rgb_set_uchar (&bg_color,
		      widget->style->base[widget->state].red   >> 8,
		      widget->style->base[widget->state].green >> 8,
		      widget->style->base[widget->state].blue  >> 8);

  layer_color = &bg_color;
  mask_color  = &bg_color;

  layer_item = GIMP_LAYER_LIST_ITEM (widget);
  list_item  = GIMP_LIST_ITEM (widget);
  layer      = GIMP_LAYER (GIMP_PREVIEW (list_item->preview)->viewable);
  mask       = gimp_layer_get_mask (layer);

  if (! mask || (mask && ! gimp_layer_mask_get_edit (mask)))
    {
      if (widget->state == GTK_STATE_SELECTED)
	layer_color = &white_color;
      else
	layer_color = &black_color;
    }

  if (mask)
    {
      if (gimp_layer_mask_get_show (mask))
	{
	  mask_color = &green_color;
	}
      else if (! gimp_layer_mask_get_apply (mask))
	{
	  mask_color = &red_color;
	}
      else if (gimp_layer_mask_get_edit (mask))
	{
	  if (widget->state == GTK_STATE_SELECTED)
	    mask_color = &white_color;
	  else
	    mask_color = &black_color;
	}
    }

  preview = GIMP_PREVIEW (list_item->preview);

  gimp_preview_set_border_color (preview, layer_color);

  if (mask)
    {
      preview = GIMP_PREVIEW (layer_item->mask_preview);

      gimp_preview_set_border_color (preview, mask_color);
    }
}

static void
gimp_layer_list_item_layer_clicked (GtkWidget *widget,
                                    GimpLayer *layer)
{
  GimpLayerMask *mask;

  mask = gimp_layer_get_mask (layer);

  if (mask)
    {
      if (gimp_layer_mask_get_edit (mask))
        gimp_layer_mask_set_edit (mask, FALSE);
    }
}

static void
gimp_layer_list_item_mask_clicked (GtkWidget     *widget,
                                   GimpLayerMask *mask)
{
  if (! gimp_layer_mask_get_edit (mask))
    gimp_layer_mask_set_edit (mask, TRUE);
}

static void
gimp_layer_list_item_mask_extended_clicked (GtkWidget     *widget,
					    guint          state,
					    GimpLayerMask *mask)
{
  gboolean flush = FALSE;

  if (state & GDK_MOD1_MASK)
    {
      gimp_layer_mask_set_show (mask, ! gimp_layer_mask_get_show (mask));

      flush = TRUE;
    }
  else if (state & GDK_CONTROL_MASK)
    {
      gimp_layer_mask_set_apply (mask, ! gimp_layer_mask_get_apply (mask));

      flush = TRUE;
    }

  if (flush)
    gdisplays_flush ();
}
