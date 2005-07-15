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

#include "config.h"

#include <glib-object.h>

#include "core-types.h"

#include "base/pixel-region.h"
#include "base/siox.h"
#include "base/tile-manager.h"

#include "gimpchannel.h"
#include "gimpdrawable.h"
#include "gimpdrawable-foreground-extract.h"
#include "gimpimage.h"


/*  public functions  */

void
gimp_drawable_foreground_extract (GimpDrawable *drawable,
                                  GimpDrawable *mask)
{
  GimpImage *gimage;
  gfloat     limits[SIOX_DIMS] = { 0.66, 1.25, 2.5 };

  g_return_if_fail (GIMP_IS_DRAWABLE (drawable));
  g_return_if_fail (gimp_item_is_attached (GIMP_ITEM (drawable)));

  g_return_if_fail (GIMP_IS_DRAWABLE (mask));
  g_return_if_fail (gimp_drawable_bytes (mask) == 1);

  gimage = gimp_item_get_image (GIMP_ITEM (drawable));

  siox_foreground_extract (gimp_drawable_data (drawable),
                           gimp_drawable_data (mask),
                           limits, 3);

  gimp_drawable_update (mask,
                        0, 0,
                        gimp_item_width (GIMP_ITEM (mask)),
                        gimp_item_height (GIMP_ITEM (mask)));
}