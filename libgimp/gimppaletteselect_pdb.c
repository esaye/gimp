/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimppaletteselect_pdb.c
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
 * gimp_palettes_popup:
 * @palette_callback: The callback PDB proc to call when palette selection is made.
 * @popup_title: Title to give the palette popup window.
 * @initial_palette: The name of the palette to set as the first selected.
 *
 * Invokes the Gimp palette selection.
 *
 * This procedure popups the palette selection dialog.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_palettes_popup (gchar *palette_callback,
		     gchar *popup_title,
		     gchar *initial_palette)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_palettes_popup",
				    &nreturn_vals,
				    GIMP_PDB_STRING, palette_callback,
				    GIMP_PDB_STRING, popup_title,
				    GIMP_PDB_STRING, initial_palette,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_palettes_close_popup:
 * @palette_callback: The name of the callback registered for this popup.
 *
 * Popdown the Gimp palette selection.
 *
 * This procedure closes an opened palette selection dialog.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_palettes_close_popup (gchar *palette_callback)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_palettes_close_popup",
				    &nreturn_vals,
				    GIMP_PDB_STRING, palette_callback,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_palettes_set_popup:
 * @palette_callback: The name of the callback registered for this popup.
 * @palette_name: The name of the palette to set as selected.
 *
 * Sets the current palette selection in a popup.
 *
 * Sets the current palette selection in a popup.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_palettes_set_popup (gchar *palette_callback,
			 gchar *palette_name)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_palettes_set_popup",
				    &nreturn_vals,
				    GIMP_PDB_STRING, palette_callback,
				    GIMP_PDB_STRING, palette_name,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
