/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2003 Peter Mattis and Spencer Kimball
 *
 * gimpbrushes_pdb.c
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

#include <string.h>

#include "gimp.h"

/**
 * gimp_brushes_refresh:
 *
 * Refresh current brushes. This function always succeeds.
 *
 * This procedure retrieves all brushes currently in the user's brush
 * path and updates the brush dialog accordingly.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_brushes_refresh (void)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_brushes_refresh",
				    &nreturn_vals,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_brushes_get_list:
 * @filter: An optional regular expression used to filter the list.
 * @num_brushes: The number of brushes in the brush list.
 *
 * Retrieve a complete listing of the available brushes.
 *
 * This procedure returns a complete listing of available GIMP brushes.
 * Each name returned can be used as input to the
 * 'gimp_brushes_set_brush'.
 *
 * Returns: The list of brush names.
 */
gchar **
gimp_brushes_get_list (const gchar *filter,
		       gint        *num_brushes)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gchar **brush_list = NULL;
  gint i;

  return_vals = gimp_run_procedure ("gimp_brushes_get_list",
				    &nreturn_vals,
				    GIMP_PDB_STRING, filter,
				    GIMP_PDB_END);

  *num_brushes = 0;

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      *num_brushes = return_vals[1].data.d_int32;
      brush_list = g_new (gchar *, *num_brushes);
      for (i = 0; i < *num_brushes; i++)
	brush_list[i] = g_strdup (return_vals[2].data.d_stringarray[i]);
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return brush_list;
}

/**
 * gimp_brushes_get_brush:
 * @width: The brush width.
 * @height: The brush height.
 * @spacing: The brush spacing.
 *
 * Retrieve information about the currently active brush mask.
 *
 * This procedure retrieves information about the currently active
 * brush mask. This includes the brush name, the width and height, and
 * the brush spacing paramter. All paint operations and stroke
 * operations use this mask to control the application of paint to the
 * image.
 *
 * Returns: The brush name.
 */
gchar *
gimp_brushes_get_brush (gint *width,
			gint *height,
			gint *spacing)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gchar *name = NULL;

  return_vals = gimp_run_procedure ("gimp_brushes_get_brush",
				    &nreturn_vals,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      name = g_strdup (return_vals[1].data.d_string);
      *width = return_vals[2].data.d_int32;
      *height = return_vals[3].data.d_int32;
      *spacing = return_vals[4].data.d_int32;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return name;
}

/**
 * gimp_brushes_set_brush:
 * @name: The brush name.
 *
 * Set the specified brush as the active brush.
 *
 * This procedure allows the active brush mask to be set by specifying
 * its name. The name is simply a string which corresponds to one of
 * the names of the installed brushes. If there is no matching brush
 * found, this procedure will return an error. Otherwise, the specified
 * brush becomes active and will be used in all subsequent paint
 * operations.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_brushes_set_brush (const gchar *name)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_brushes_set_brush",
				    &nreturn_vals,
				    GIMP_PDB_STRING, name,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_brushes_get_spacing:
 *
 * Get the brush spacing.
 *
 * This procedure returns the spacing setting for brushes. This value
 * is set per brush and will change if a different brush is selected.
 * The return value is an integer between 0 and 1000 which represents
 * percentage of the maximum of the width and height of the mask.
 *
 * Returns: The brush spacing.
 */
gint
gimp_brushes_get_spacing (void)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint spacing = 0;

  return_vals = gimp_run_procedure ("gimp_brushes_get_spacing",
				    &nreturn_vals,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    spacing = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return spacing;
}

/**
 * gimp_brushes_set_spacing:
 * @spacing: The brush spacing.
 *
 * Set the brush spacing.
 *
 * This procedure modifies the spacing setting for the current brush.
 * This value is set on a per-brush basis and will change if a
 * different brush mask is selected. The value should be a integer
 * between 0 and 1000.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_brushes_set_spacing (gint spacing)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_brushes_set_spacing",
				    &nreturn_vals,
				    GIMP_PDB_INT32, spacing,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_brushes_get_brush_data:
 * @name: The brush name (\"\" means current active brush).
 * @opacity: The brush opacity.
 * @spacing: The brush spacing.
 * @paint_mode: The paint mode.
 * @width: The brush width.
 * @height: The brush height.
 * @length: Length of brush mask data.
 * @mask_data: The brush mask data.
 *
 * Retrieve information about the currently active brush (including
 * data).
 *
 * This procedure retrieves information about the currently active
 * brush. This includes the brush name, and the brush extents (width
 * and height). It also returns the brush data.
 *
 * Returns: The brush name.
 */
gchar *
gimp_brushes_get_brush_data (const gchar           *name,
			     gdouble               *opacity,
			     gint                  *spacing,
			     GimpLayerModeEffects  *paint_mode,
			     gint                  *width,
			     gint                  *height,
			     gint                  *length,
			     guint8               **mask_data)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gchar *ret_name = NULL;

  return_vals = gimp_run_procedure ("gimp_brushes_get_brush_data",
				    &nreturn_vals,
				    GIMP_PDB_STRING, name,
				    GIMP_PDB_END);

  *length = 0;

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      ret_name = g_strdup (return_vals[1].data.d_string);
      *opacity = return_vals[2].data.d_float;
      *spacing = return_vals[3].data.d_int32;
      *paint_mode = return_vals[4].data.d_int32;
      *width = return_vals[5].data.d_int32;
      *height = return_vals[6].data.d_int32;
      *length = return_vals[7].data.d_int32;
      *mask_data = g_new (guint8, *length);
      memcpy (*mask_data, return_vals[8].data.d_int8array,
	      *length * sizeof (guint8));
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return ret_name;
}
