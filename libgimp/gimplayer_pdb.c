/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimplayer_pdb.c
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
 * _gimp_layer_new:
 * @image_ID: The image to which to add the layer.
 * @width: The layer width.
 * @height: The layer height.
 * @type: The layer type.
 * @name: The layer name.
 * @opacity: The layer opacity.
 * @mode: The layer combination mode.
 *
 * Create a new layer.
 *
 * This procedure creates a new layer with the specified width, height,
 * and type. Name, opacity, and mode are also supplied parameters. The
 * new layer still needs to be added to the image, as this is not
 * automatic. Add the new layer with the 'gimp_image_add_layer'
 * command. Other attributes such as layer mask modes, and offsets
 * should be set with explicit procedure calls.
 *
 * Returns: The newly created layer.
 */
gint32
_gimp_layer_new (gint32                image_ID,
		 gint                  width,
		 gint                  height,
		 GimpImageType         type,
		 const gchar          *name,
		 gdouble               opacity,
		 GimpLayerModeEffects  mode)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 layer_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_new",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, width,
				    GIMP_PDB_INT32, height,
				    GIMP_PDB_INT32, type,
				    GIMP_PDB_STRING, name,
				    GIMP_PDB_FLOAT, opacity,
				    GIMP_PDB_INT32, mode,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_ID;
}

/**
 * _gimp_layer_copy:
 * @layer_ID: The layer to copy.
 * @add_alpha: Add an alpha channel to the copied layer.
 *
 * Copy a layer.
 *
 * This procedure copies the specified layer and returns the copy. The
 * newly copied layer is for use within the original layer's image. It
 * should not be subsequently added to any other image. The copied
 * layer can optionally have an added alpha channel. This is useful if
 * the background layer in an image is being copied and added to the
 * same image.
 *
 * Returns: The newly copied layer.
 */
gint32
_gimp_layer_copy (gint32   layer_ID,
		  gboolean add_alpha)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 layer_copy_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_copy",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, add_alpha,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer_copy_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_copy_ID;
}

/**
 * gimp_layer_create_mask:
 * @layer_ID: The layer to which to add the mask.
 * @mask_type: The type of mask.
 *
 * Create a layer mask for the specified specified layer.
 *
 * This procedure creates a layer mask for the specified layer. Layer
 * masks serve as an additional alpha channel for a layer. A number of
 * ifferent types of masks are allowed for initialisation: completely
 * white masks (which will leave the layer fully visible), completely
 * black masks (which will give the layer complete transparency, the
 * layer's already existing alpha channel (which will leave the layer
 * fully visible, but which may be more useful than a white mask), the
 * current selection or a grayscale copy of the layer. The layer mask
 * still needs to be added to the layer. This can be done with a call
 * to 'gimp_image_add_layer_mask'.
 *
 * Returns: The newly created mask.
 */
gint32
gimp_layer_create_mask (gint32          layer_ID,
			GimpAddMaskType mask_type)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 mask_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_create_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, mask_type,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    mask_ID = return_vals[1].data.d_layer_mask;

  gimp_destroy_params (return_vals, nreturn_vals);

  return mask_ID;
}

/**
 * gimp_layer_scale:
 * @layer_ID: The layer.
 * @new_width: New layer width.
 * @new_height: New layer height.
 * @local_origin: Use a local origin (as opposed to the image origin).
 *
 * Scale the layer to the specified extents.
 *
 * This procedure scales the layer so that it's new width and height
 * are equal to the supplied parameters. The \"local_origin\" parameter
 * specifies whether to scale from the center of the layer, or from the
 * image origin. This operation only works if the layer has been added
 * to an image.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_scale (gint32   layer_ID,
		  gint     new_width,
		  gint     new_height,
		  gboolean local_origin)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_scale",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, new_width,
				    GIMP_PDB_INT32, new_height,
				    GIMP_PDB_INT32, local_origin,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_resize:
 * @layer_ID: The layer.
 * @new_width: New layer width.
 * @new_height: New layer height.
 * @offx: x offset between upper left corner of old and new layers: (new - old).
 * @offy: y offset between upper left corner of old and new layers: (new - old).
 *
 * Resize the layer to the specified extents.
 *
 * This procedure resizes the layer so that it's new width and height
 * are equal to the supplied parameters. Offsets are also provided
 * which describe the position of the previous layer's content. This
 * operation only works if the layer has been added to an image.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_resize (gint32 layer_ID,
		   gint   new_width,
		   gint   new_height,
		   gint   offx,
		   gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_resize",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, new_width,
				    GIMP_PDB_INT32, new_height,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_resize_to_image_size:
 * @layer_ID: The layer to resize.
 *
 * Resize a layer to the image size.
 *
 * This procedure resizes the layer so that it's new width and height
 * are equal to the width and height of its image container.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_resize_to_image_size (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_resize_to_image_size",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_translate:
 * @layer_ID: The layer.
 * @offx: Offset in x direction.
 * @offy: Offset in y direction.
 *
 * Translate the layer by the specified offsets.
 *
 * This procedure translates the layer by the amounts specified in the
 * x and y arguments. These can be negative, and are considered offsets
 * from the current position. This command only works if the layer has
 * been added to an image. All additional layers contained in the image
 * which have the linked flag set to TRUE w ill also be translated by
 * the specified offsets.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_translate (gint32 layer_ID,
		      gint   offx,
		      gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_translate",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_add_alpha:
 * @layer_ID: The layer.
 *
 * Add an alpha channel to the layer if it doesn't already have one.
 *
 * This procedure adds an additional component to the specified layer
 * if it does not already possess an alpha channel. An alpha channel
 * makes it possible to move a layer from the bottom of the layer stack
 * and to clear and erase to transparency, instead of the background
 * color. This transforms images of type RGB to RGBA, GRAY to GRAYA,
 * and INDEXED to INDEXEDA.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_add_alpha (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_add_alpha",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_set_offsets:
 * @layer_ID: The layer.
 * @offx: Offset in x direction.
 * @offy: Offset in y direction.
 *
 * Set the layer offsets.
 *
 * This procedure sets the offsets for the specified layer. The offsets
 * are relative to the image origin and can be any values. This
 * operation is valid only on layers which have been added to an image.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_offsets (gint32 layer_ID,
			gint   offx,
			gint   offy)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_offsets",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, offx,
				    GIMP_PDB_INT32, offy,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_mask:
 * @layer_ID: The layer.
 *
 * Get the specified layer's mask if it exists.
 *
 * This procedure returns the specified layer's mask, or -1 if none
 * exists.
 *
 * Returns: The layer mask.
 */
gint32
gimp_layer_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 mask_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    mask_ID = return_vals[1].data.d_channel;

  gimp_destroy_params (return_vals, nreturn_vals);

  return mask_ID;
}

/**
 * gimp_layer_is_floating_sel:
 * @layer_ID: The layer.
 *
 * Is the specified layer a floating selection?
 *
 * This procedure returns whether the layer is a floating selection.
 * Floating selections are special cases of layers which are attached
 * to a specific drawable.
 *
 * Returns: Non-zero if the layer is a floating selection.
 */
gboolean
gimp_layer_is_floating_sel (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean is_floating_sel = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_is_floating_sel",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    is_floating_sel = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return is_floating_sel;
}

/**
 * gimp_layer_new_from_drawable:
 * @drawable_ID: The source drawable from where the new layer is copied.
 * @dest_image_ID: The destination image to which to add the layer.
 *
 * Create a new layer by copying an existing drawable.
 *
 * This procedure creates a new layer as a copy of the specified
 * drawable. The new layer still needs to be added to the image, as
 * this is not automatic. Add the new layer with the
 * 'gimp_image_add_layer' command. Other attributes such as layer mask
 * modes, and offsets should be set with explicit procedure calls.
 *
 * Returns: The newly copied layer.
 */
gint32
gimp_layer_new_from_drawable (gint32 drawable_ID,
			      gint32 dest_image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint32 layer_copy_ID = -1;

  return_vals = gimp_run_procedure ("gimp_layer_new_from_drawable",
				    &nreturn_vals,
				    GIMP_PDB_DRAWABLE, drawable_ID,
				    GIMP_PDB_IMAGE, dest_image_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    layer_copy_ID = return_vals[1].data.d_layer;

  gimp_destroy_params (return_vals, nreturn_vals);

  return layer_copy_ID;
}

/**
 * gimp_layer_get_preserve_trans:
 * @layer_ID: The layer.
 *
 * Get the preserve transperancy setting of the specified layer.
 *
 * This procedure returns the specified layer's preserve transperancy
 * setting.
 *
 * Returns: The layer's preserve transperancy setting.
 */
gboolean
gimp_layer_get_preserve_trans (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean preserve_trans = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_preserve_trans",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    preserve_trans = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return preserve_trans;
}

/**
 * gimp_layer_set_preserve_trans:
 * @layer_ID: The layer.
 * @preserve_trans: The new layer's preserve transperancy setting.
 *
 * Set the preserve transperancy setting of the specified layer.
 *
 * This procedure sets the specified layer's preserve transperancy
 * setting.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_preserve_trans (gint32   layer_ID,
			       gboolean preserve_trans)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_preserve_trans",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, preserve_trans,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_get_apply_mask:
 * @layer_ID: The layer.
 *
 * Get the apply mask of the specified layer.
 *
 * This procedure returns the specified layer's apply mask. If the
 * value is non-zero, then the layer mask for this layer is currently
 * being composited with the layer's alpha channel.
 *
 * Returns: The layer apply mask.
 */
gboolean
gimp_layer_get_apply_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean apply_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_apply_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    apply_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return apply_mask;
}

/**
 * gimp_layer_set_apply_mask:
 * @layer_ID: The layer.
 * @apply_mask: The new layer apply mask.
 *
 * Set the apply mask of the specified layer.
 *
 * This procedure sets the specified layer's apply mask. This controls
 * whether the layer's mask is currently affecting the alpha channel.
 * If there is no layer mask, this function will return an error.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_apply_mask (gint32   layer_ID,
			   gboolean apply_mask)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_apply_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, apply_mask,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_get_show_mask:
 * @layer_ID: The layer.
 *
 * Get the show mask of the specified layer.
 *
 * This procedure returns the specified layer's show mask. If the value
 * is non-zero, then the layer mask for this layer is currently being
 * shown instead of the layer.
 *
 * Returns: The layer show mask.
 */
gboolean
gimp_layer_get_show_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean show_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_show_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    show_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return show_mask;
}

/**
 * gimp_layer_set_show_mask:
 * @layer_ID: The layer.
 * @show_mask: The new layer show mask.
 *
 * Set the show mask of the specified layer.
 *
 * This procedure sets the specified layer's show mask. This controls
 * whether the layer or it's mask is visible. Non-zero values indicate
 * that the mask should be visible. If the layer has no mask, then this
 * function returns an error.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_show_mask (gint32   layer_ID,
			  gboolean show_mask)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_show_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, show_mask,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_get_edit_mask:
 * @layer_ID: The layer.
 *
 * Get the show mask of the specified layer.
 *
 * This procedure returns the specified layer's show mask. If the value
 * is non-zero, then the layer mask for this layer is currently active,
 * and not the layer.
 *
 * Returns: The layer show mask.
 */
gboolean
gimp_layer_get_edit_mask (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean edit_mask = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_edit_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    edit_mask = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return edit_mask;
}

/**
 * gimp_layer_set_edit_mask:
 * @layer_ID: The layer.
 * @edit_mask: The new layer show mask.
 *
 * Set the show mask of the specified layer.
 *
 * This procedure sets the specified layer's show mask. This controls
 * whether the layer or it's mask is currently active for editing. If
 * the specified layer has no layer mask, then this procedure will
 * return an error.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_edit_mask (gint32   layer_ID,
			  gboolean edit_mask)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_edit_mask",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, edit_mask,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_get_opacity:
 * @layer_ID: The layer.
 *
 * Get the opacity of the specified layer.
 *
 * This procedure returns the specified layer's opacity.
 *
 * Returns: The layer opacity.
 */
gdouble
gimp_layer_get_opacity (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gdouble opacity = 0;

  return_vals = gimp_run_procedure ("gimp_layer_get_opacity",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    opacity = return_vals[1].data.d_float;

  gimp_destroy_params (return_vals, nreturn_vals);

  return opacity;
}

/**
 * gimp_layer_set_opacity:
 * @layer_ID: The layer.
 * @opacity: The new layer opacity.
 *
 * Set the opacity of the specified layer.
 *
 * This procedure sets the specified layer's opacity.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_opacity (gint32  layer_ID,
			gdouble opacity)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_opacity",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_FLOAT, opacity,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_get_mode:
 * @layer_ID: The layer.
 *
 * Get the combination mode of the specified layer.
 *
 * This procedure returns the specified layer's combination mode.
 *
 * Returns: The layer combination mode.
 */
GimpLayerModeEffects
gimp_layer_get_mode (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  GimpLayerModeEffects mode = 0;

  return_vals = gimp_run_procedure ("gimp_layer_get_mode",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    mode = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return mode;
}

/**
 * gimp_layer_set_mode:
 * @layer_ID: The layer.
 * @mode: The new layer combination mode.
 *
 * Set the combination mode of the specified layer.
 *
 * This procedure sets the specified layer's combination mode.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_mode (gint32               layer_ID,
		     GimpLayerModeEffects mode)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_mode",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, mode,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_layer_get_linked:
 * @layer_ID: The layer.
 *
 * Get the linked state of the specified layer.
 *
 * This procedure returns the specified layer's linked state.
 *
 * Returns: The layer linked state (for moves).
 */
gboolean
gimp_layer_get_linked (gint32 layer_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean linked = FALSE;

  return_vals = gimp_run_procedure ("gimp_layer_get_linked",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    linked = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return linked;
}

/**
 * gimp_layer_set_linked:
 * @layer_ID: The layer.
 * @linked: The new layer linked state.
 *
 * Set the linked state of the specified layer.
 *
 * This procedure sets the specified layer's linked state.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_layer_set_linked (gint32   layer_ID,
		       gboolean linked)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_layer_set_linked",
				    &nreturn_vals,
				    GIMP_PDB_LAYER, layer_ID,
				    GIMP_PDB_INT32, linked,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
