/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimpselection_pdb.c
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

/* NOTE: This file is autogenerated by pdbgen.pl */

#include "config.h"

#include "gimp.h"

/**
 * gimp_selection_bounds:
 * @image_ID: The image.
 * @non_empty: True if there is a selection.
 * @x1: x coordinate of upper left corner of selection bounds.
 * @y1: y coordinate of upper left corner of selection bounds.
 * @x2: x coordinate of lower right corner of selection bounds.
 * @y2: y coordinate of lower right corner of selection bounds.
 *
 * Find the bounding box of the current selection.
 *
 * This procedure returns whether there is a selection for the
 * specified image. If there is one, the upper left and lower right
 * corners of the bounding box are returned. These coordinates are
 * relative to the image.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_bounds (gint32    image_ID,
		       gboolean *non_empty,
		       gint     *x1,
		       gint     *y1,
		       gint     *x2,
		       gint     *y2)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_bounds",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  *non_empty = FALSE;
  *x1 = 0;
  *y1 = 0;
  *x2 = 0;
  *y2 = 0;

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  if (success)
    {
      *non_empty = return_vals[1].data.d_int32;
      *x1 = return_vals[2].data.d_int32;
      *y1 = return_vals[3].data.d_int32;
      *x2 = return_vals[4].data.d_int32;
      *y2 = return_vals[5].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_value:
 * @image_ID: The image.
 * @x: x coordinate of value.
 * @y: y coordinate of value.
 *
 * Find the value of the selection at the specified coordinates.
 *
 * This procedure returns the value of the selection at the specified
 * coordinates. If the coordinates lie out of bounds, 0 is returned.
 *
 * Returns: Value of the selection.
 */
gint
gimp_selection_value (gint32 image_ID,
		      gint   x,
		      gint   y)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint value = 0;

  return_vals = gimp_run_procedure ("gimp_selection_value",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, x,
				    GIMP_PDB_INT32, y,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    value = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return value;
}

/**
 * gimp_selection_is_empty:
 * @image_ID: The image.
 *
 * Determine whether the selection is empty.
 *
 * This procedure returns non-zero if the selection for the specified
 * image is not empty.
 *
 * Returns: Is the selection empty?
 */
gboolean
gimp_selection_is_empty (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean is_empty = FALSE;

  return_vals = gimp_run_procedure ("gimp_selection_is_empty",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    is_empty = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return is_empty;
}

/**
 * gimp_selection_translate:
 * @image_ID: The image.
 * @offx: x offset for translation.
 * @offy: y offset for translation.
 *
 * Translate the selection by the specified offsets.
 *
 * This procedure actually translates the selection for the specified
 * image by the specified offsets. Regions that are translated from
 * beyond the bounds of the image are set to empty. Valid regions of
 * the selection which are translated beyond the bounds of the image
 * because of this call are lost.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_translate (gint32 image_ID,
			  gint   offx,
			  gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_translate",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * _gimp_selection_float:
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
_gimp_selection_float (gint32 drawable_ID,
		       gint   offx,
		       gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 layer_ID = -1;

  return_vals = gimp_run_procedure ("gimp_selection_float",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_ID;
}

/**
 * gimp_selection_clear:
 * @image_ID: The image.
 *
 * Set the selection to none, clearing all previous content.
 *
 * This procedure sets the selection mask to empty, assigning the value
 * 0 to every pixel in the selection channel.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_clear (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_clear",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_invert:
 * @image_ID: The image.
 *
 * Invert the selection mask.
 *
 * This procedure inverts the selection mask. For every pixel in the
 * selection channel, its new value is calculated as (255 - old_value).
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_invert (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_invert",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_sharpen:
 * @image_ID: The image.
 *
 * Sharpen the selection mask.
 *
 * This procedure sharpens the selection mask. For every pixel in the
 * selection channel, if the value is > 0, the new pixel is assigned a
 * value of 255. This removes any \"anti-aliasing\" that might exist in
 * the selection mask's boundary.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_sharpen (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_sharpen",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_all:
 * @image_ID: The image.
 *
 * Select all of the image.
 *
 * This procedure sets the selection mask to completely encompass the
 * image. Every pixel in the selection channel is set to 255.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_all (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_all",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_none:
 * @image_ID: The image.
 *
 * Deselect the entire image.
 *
 * This procedure deselects the entire image. Every pixel in the
 * selection channel is set to 0.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_none (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_none",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_feather:
 * @image_ID: The image.
 * @radius: Radius of feather (in pixels).
 *
 * Feather the image's selection
 *
 * This procedure feathers the selection. Feathering is implemented
 * using a gaussian blur.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_feather (gint32  image_ID,
			gdouble radius)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_feather",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_FLOAT, radius,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_border:
 * @image_ID: The image.
 * @radius: Radius of border (in pixels).
 *
 * Border the image's selection
 *
 * This procedure borders the selection. Bordering creates a new
 * selection which is defined along the boundary of the previous
 * selection at every point within the specified radius.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_border (gint32 image_ID,
		       gint   radius)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_border",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, radius,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_grow:
 * @image_ID: The image.
 * @steps: Steps of grow (in pixels).
 *
 * Grow the image's selection
 *
 * This procedure grows the selection. Growing involves expanding the
 * boundary in all directions by the specified pixel amount.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_grow (gint32 image_ID,
		     gint   steps)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_grow",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, steps,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_shrink:
 * @image_ID: The image.
 * @radius: Radius of shrink (in pixels).
 *
 * Shrink the image's selection
 *
 * This procedure shrinks the selection. Shrinking invovles trimming
 * the existing selection boundary on all sides by the specified number
 * of pixels.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_shrink (gint32 image_ID,
		       gint   radius)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_shrink",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, radius,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_layer_alpha:
 * @layer_ID: Layer with alpha.
 *
 * Transfer the specified layer's alpha channel to the selection mask.
 *
 * This procedure requires a layer with an alpha channel. The alpha
 * channel information is used to create a selection mask such that for
 * any pixel in the image defined in the specified layer, that layer
 * pixel's alpha value is transferred to the selection mask. If the
 * layer is undefined at a particular image pixel, the associated
 * selection mask value is set to 0.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_layer_alpha (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_layer_alpha",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_load:
 * @channel_ID: The channel.
 *
 * Transfer the specified channel to the selection mask.
 *
 * This procedure loads the specified channel into the selection mask.
 * This essentially involves a copy of the channel's content in to the
 * selection mask. Therefore, the channel must have the same width and
 * height of the image, or an error is returned.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_load (gint32 channel_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_load",
				    &nreturn_vals,
				    GIMP_PDB_CHANNEL, channel_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_selection_save:
 * @image_ID: The image.
 *
 * Copy the selection mask to a new channel.
 *
 * This procedure copies the selection mask and stores the content in a
 * new channel. The new channel is automatically inserted into the
 * image's list of channels.
 *
 * Returns: The new channel.
 */
gint32
gimp_selection_save (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 channel_ID = -1;

  return_vals = gimp_run_procedure ("gimp_selection_save",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    channel_ID = return_vals[1].data.d_channel;

  gimp_destroy_params (return_vals, nreturn_vals);

  return channel_ID;
}

/**
 * gimp_selection_combine:
 * @channel_ID: The channel.
 * @operation: The selection operation.
 *
 * Combines the specified channel with the selection mask.
 *
 * This procedure combines the specified channel into the selection
 * mask. It essentially involves a transfer of the channel's content
 * into the selection mask. Therefore, the channel must have the same
 * width and height of the image, or an error is returned.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_selection_combine (gint32         channel_ID,
			GimpChannelOps operation)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_selection_combine",
				    &nreturn_vals,
				    GIMP_PDB_CHANNEL, channel_ID,
				    GIMP_PDB_INT32, operation,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
