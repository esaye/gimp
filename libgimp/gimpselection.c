/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpselection.c
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include "gimp.h"
#undef GIMP_DISABLE_DEPRECATED
#undef __GIMP_SELECTION_H__
#include "gimpselection.h"

/**
 * gimp_selection_float:
 * @image_ID: ignored
 * @drawable_ID: The drawable from which to float selection.
 * @offx: x offset for translation.
 * @offy: y offset for translation.
 *
 * Float the selection from the specified drawable with initial offsets
 * as specified.
 *
 * This procedure determines the region of the specified drawable that
 * lies beneath the current selection. The region is then cut from the
 * drawable and the resulting data is made into a new layer which is
 * instantiated as a floating selection. The offsets allow initial
 * positioning of the new floating selection.
 *
 * Returns: The floated layer.
 */
gint32
gimp_selection_float (gint32 image_ID,
                      gint32 drawable_ID,
                      gint   offx,
                      gint   offy)
{
  return _gimp_selection_float (drawable_ID,
                                offx,
                                offy);
}

/**
 * gimp_selection_clear:
 * @image_ID: The image.
 *
 * This procedure is deprecated! Use gimp_selection_none() instead.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_clear (gint32 image_ID)
{
  return gimp_selection_none (image_ID);
}
