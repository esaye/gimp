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

#ifndef __GIMP_LAYER_MASK_H__
#define __GIMP_LAYER_MASK_H__


#include "gimpchannel.h"


#define GIMP_TYPE_LAYER_MASK            (gimp_layer_mask_get_type ())
#define GIMP_LAYER_MASK(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_LAYER_MASK, GimpLayerMask))
#define GIMP_LAYER_MASK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_LAYER_MASK, GimpLayerMaskClass))
#define GIMP_IS_LAYER_MASK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_LAYER_MASK))
#define GIMP_IS_LAYER_MASK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_LAYER_MASK))


typedef struct _GimpLayerMaskClass  GimpLayerMaskClass;

struct _GimpLayerMask
{
  GimpChannel  parent_instance;

  GimpLayer   *layer;

  gboolean     apply_mask;    /*  controls mask application  */
  gboolean     edit_mask;     /*  edit mask or layer?        */
  gboolean     show_mask;     /*  show mask or layer?        */
};

struct _GimpLayerMaskClass
{
  GimpChannelClass  parent_class;

  void (* apply_changed) (GimpLayerMask *layer_mask);
  void (* edit_changed)  (GimpLayerMask *layer_mask);
  void (* show_changed)  (GimpLayerMask *layer_mask);
};


/*  Special undo type  */

struct _LayerMaskUndo
{
  GimpLayer     *layer;    /*  the layer             */
  GimpLayerMask *mask;     /*  the layer mask        */
  gint           mode;     /*  the application mode  */
};


/*  function declarations  */

GtkType         gimp_layer_mask_get_type    (void);

GimpLayerMask * gimp_layer_mask_new	    (GimpImage       *gimage,
					     gint             width,
					     gint             height,
					     const gchar     *name,
					     const GimpRGB   *color);
GimpLayerMask * gimp_layer_mask_copy	    (GimpLayerMask       *layer_mask);

void            gimp_layer_mask_set_layer   (GimpLayerMask       *layer_mask, 
				             GimpLayer           *layer);
GimpLayer     * gimp_layer_mask_get_layer   (GimpLayerMask       *layer_mask);

void            gimp_layer_mask_set_apply   (GimpLayerMask       *layer_mask,
                                             gboolean             apply);
gboolean        gimp_layer_mask_get_apply   (GimpLayerMask       *layer_mask);

void            gimp_layer_mask_set_edit    (GimpLayerMask       *layer_mask,
                                             gboolean             apply);
gboolean        gimp_layer_mask_get_edit    (GimpLayerMask       *layer_mask);

void            gimp_layer_mask_set_show    (GimpLayerMask       *layer_mask,
                                             gboolean             show);
gboolean        gimp_layer_mask_get_show    (GimpLayerMask       *layer_mask);


#endif /* __GIMP_LAYER_MASK_H__ */
