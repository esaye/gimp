/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2003 Spencer Kimball and Peter Mattis
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

/* NOTE: This file is autogenerated by pdbgen.pl. */

#include "config.h"

#include <string.h>

#include <glib-object.h>

#include "pdb-types.h"
#include "procedural_db.h"

#include "base/temp-buf.h"
#include "core/gimp.h"
#include "core/gimpbrush.h"
#include "core/gimpcontainer-filter.h"
#include "core/gimpcontext.h"
#include "core/gimpdatafactory.h"
#include "core/gimplist.h"

static ProcRecord brushes_refresh_proc;
static ProcRecord brushes_get_list_proc;
static ProcRecord brushes_get_brush_proc;
static ProcRecord brushes_set_brush_proc;
static ProcRecord brushes_get_spacing_proc;
static ProcRecord brushes_set_spacing_proc;
static ProcRecord brushes_get_brush_data_proc;

void
register_brushes_procs (Gimp *gimp)
{
  procedural_db_register (gimp, &brushes_refresh_proc);
  procedural_db_register (gimp, &brushes_get_list_proc);
  procedural_db_register (gimp, &brushes_get_brush_proc);
  procedural_db_register (gimp, &brushes_set_brush_proc);
  procedural_db_register (gimp, &brushes_get_spacing_proc);
  procedural_db_register (gimp, &brushes_set_spacing_proc);
  procedural_db_register (gimp, &brushes_get_brush_data_proc);
}

static Argument *
brushes_refresh_invoker (Gimp         *gimp,
                         GimpContext  *context,
                         GimpProgress *progress,
                         Argument     *args)
{
  gimp_data_factory_data_save (gimp->brush_factory);
  gimp_data_factory_data_init (gimp->brush_factory, FALSE);
  return procedural_db_return_args (&brushes_refresh_proc, TRUE);
}

static ProcRecord brushes_refresh_proc =
{
  "gimp_brushes_refresh",
  "Refresh current brushes. This function always succeeds.",
  "This procedure retrieves all brushes currently in the user's brush path and updates the brush dialog accordingly.",
  "Seth Burgess",
  "Seth Burgess",
  "1997",
  GIMP_INTERNAL,
  0,
  NULL,
  0,
  NULL,
  { { brushes_refresh_invoker } }
};

static Argument *
brushes_get_list_invoker (Gimp         *gimp,
                          GimpContext  *context,
                          GimpProgress *progress,
                          Argument     *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  gchar *filter;
  gint32 num_brushes;
  gchar **brush_list = NULL;

  filter = (gchar *) args[0].value.pdb_pointer;
  if (filter && !g_utf8_validate (filter, -1, NULL))
    success = FALSE;

  if (success)
    brush_list = gimp_container_get_filtered_name_array (gimp->brush_factory->container, filter, &num_brushes);

  return_args = procedural_db_return_args (&brushes_get_list_proc, success);

  if (success)
    {
      return_args[1].value.pdb_int = num_brushes;
      return_args[2].value.pdb_pointer = brush_list;
    }

  return return_args;
}

static ProcArg brushes_get_list_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "filter",
    "An optional regular expression used to filter the list"
  }
};

static ProcArg brushes_get_list_outargs[] =
{
  {
    GIMP_PDB_INT32,
    "num_brushes",
    "The number of brushes in the brush list"
  },
  {
    GIMP_PDB_STRINGARRAY,
    "brush_list",
    "The list of brush names"
  }
};

static ProcRecord brushes_get_list_proc =
{
  "gimp_brushes_get_list",
  "Retrieve a complete listing of the available brushes.",
  "This procedure returns a complete listing of available GIMP brushes. Each name returned can be used as input to the 'gimp_brushes_set_brush'.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  1,
  brushes_get_list_inargs,
  2,
  brushes_get_list_outargs,
  { { brushes_get_list_invoker } }
};

static Argument *
brushes_get_brush_invoker (Gimp         *gimp,
                           GimpContext  *context,
                           GimpProgress *progress,
                           Argument     *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  GimpBrush *brush;

  success = (brush = gimp_context_get_brush (context)) != NULL;

  return_args = procedural_db_return_args (&brushes_get_brush_proc, success);

  if (success)
    {
      return_args[1].value.pdb_pointer = g_strdup (GIMP_OBJECT (brush)->name);
      return_args[2].value.pdb_int = brush->mask->width;
      return_args[3].value.pdb_int = brush->mask->height;
      return_args[4].value.pdb_int = brush->spacing;
    }

  return return_args;
}

static ProcArg brushes_get_brush_outargs[] =
{
  {
    GIMP_PDB_STRING,
    "name",
    "The brush name"
  },
  {
    GIMP_PDB_INT32,
    "width",
    "The brush width"
  },
  {
    GIMP_PDB_INT32,
    "height",
    "The brush height"
  },
  {
    GIMP_PDB_INT32,
    "spacing",
    "The brush spacing: 0 <= spacing <= 1000"
  }
};

static ProcRecord brushes_get_brush_proc =
{
  "gimp_brushes_get_brush",
  "Retrieve information about the currently active brush mask.",
  "This procedure retrieves information about the currently active brush mask. This includes the brush name, the width and height, and the brush spacing paramter. All paint operations and stroke operations use this mask to control the application of paint to the image.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  0,
  NULL,
  4,
  brushes_get_brush_outargs,
  { { brushes_get_brush_invoker } }
};

static Argument *
brushes_set_brush_invoker (Gimp         *gimp,
                           GimpContext  *context,
                           GimpProgress *progress,
                           Argument     *args)
{
  gboolean success = TRUE;
  gchar *name;
  GimpBrush *brush;

  name = (gchar *) args[0].value.pdb_pointer;
  if (name == NULL || !g_utf8_validate (name, -1, NULL))
    success = FALSE;

  if (success)
    {
      brush = (GimpBrush *)
        gimp_container_get_child_by_name (gimp->brush_factory->container, name);

      if (brush)
        gimp_context_set_brush (context, brush);
      else
        success = FALSE;
    }

  return procedural_db_return_args (&brushes_set_brush_proc, success);
}

static ProcArg brushes_set_brush_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "name",
    "The brush name"
  }
};

static ProcRecord brushes_set_brush_proc =
{
  "gimp_brushes_set_brush",
  "Set the specified brush as the active brush.",
  "This procedure allows the active brush mask to be set by specifying its name. The name is simply a string which corresponds to one of the names of the installed brushes. If there is no matching brush found, this procedure will return an error. Otherwise, the specified brush becomes active and will be used in all subsequent paint operations.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  1,
  brushes_set_brush_inargs,
  0,
  NULL,
  { { brushes_set_brush_invoker } }
};

static Argument *
brushes_get_spacing_invoker (Gimp         *gimp,
                             GimpContext  *context,
                             GimpProgress *progress,
                             Argument     *args)
{
  Argument *return_args;

  return_args = procedural_db_return_args (&brushes_get_spacing_proc, TRUE);
  return_args[1].value.pdb_int = gimp_brush_get_spacing (gimp_context_get_brush (context));

  return return_args;
}

static ProcArg brushes_get_spacing_outargs[] =
{
  {
    GIMP_PDB_INT32,
    "spacing",
    "The brush spacing: 0 <= spacing <= 1000"
  }
};

static ProcRecord brushes_get_spacing_proc =
{
  "gimp_brushes_get_spacing",
  "Get the brush spacing.",
  "This procedure returns the spacing setting for brushes. This value is set per brush and will change if a different brush is selected. The return value is an integer between 0 and 1000 which represents percentage of the maximum of the width and height of the mask.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  0,
  NULL,
  1,
  brushes_get_spacing_outargs,
  { { brushes_get_spacing_invoker } }
};

static Argument *
brushes_set_spacing_invoker (Gimp         *gimp,
                             GimpContext  *context,
                             GimpProgress *progress,
                             Argument     *args)
{
  gboolean success = TRUE;
  gint32 spacing;

  spacing = args[0].value.pdb_int;
  if (spacing < 0 || spacing > 1000)
    success = FALSE;

  if (success)
    gimp_brush_set_spacing (gimp_context_get_brush (context), spacing);

  return procedural_db_return_args (&brushes_set_spacing_proc, success);
}

static ProcArg brushes_set_spacing_inargs[] =
{
  {
    GIMP_PDB_INT32,
    "spacing",
    "The brush spacing: 0 <= spacing <= 1000"
  }
};

static ProcRecord brushes_set_spacing_proc =
{
  "gimp_brushes_set_spacing",
  "Set the brush spacing.",
  "This procedure modifies the spacing setting for the current brush. This value is set on a per-brush basis and will change if a different brush mask is selected. The value should be a integer between 0 and 1000.",
  "Spencer Kimball & Peter Mattis",
  "Spencer Kimball & Peter Mattis",
  "1995-1996",
  GIMP_INTERNAL,
  1,
  brushes_set_spacing_inargs,
  0,
  NULL,
  { { brushes_set_spacing_invoker } }
};

static Argument *
brushes_get_brush_data_invoker (Gimp         *gimp,
                                GimpContext  *context,
                                GimpProgress *progress,
                                Argument     *args)
{
  gboolean success = TRUE;
  Argument *return_args;
  gchar *name;
  gint32 length = 0;
  guint8 *mask_data = NULL;
  GimpBrush *brush = NULL;

  name = (gchar *) args[0].value.pdb_pointer;
  if (name == NULL || !g_utf8_validate (name, -1, NULL))
    success = FALSE;

  if (success)
    {
      if (strlen (name))
        {
          brush = (GimpBrush *)
            gimp_container_get_child_by_name (gimp->brush_factory->container, name);
        }
      else
        {
          brush = gimp_context_get_brush (context);
        }

      if (brush)
        {
          length    = brush->mask->height * brush->mask->width;
          mask_data = g_memdup (temp_buf_data (brush->mask), length);
        }
      else
        success = FALSE;
    }

  return_args = procedural_db_return_args (&brushes_get_brush_data_proc, success);

  if (success)
    {
      return_args[1].value.pdb_pointer = g_strdup (GIMP_OBJECT (brush)->name);
      return_args[2].value.pdb_float = 1.0;
      return_args[3].value.pdb_int = brush->spacing;
      return_args[4].value.pdb_int = 0;
      return_args[5].value.pdb_int = brush->mask->width;
      return_args[6].value.pdb_int = brush->mask->height;
      return_args[7].value.pdb_int = length;
      return_args[8].value.pdb_pointer = mask_data;
    }

  return return_args;
}

static ProcArg brushes_get_brush_data_inargs[] =
{
  {
    GIMP_PDB_STRING,
    "name",
    "The brush name (\"\" means current active brush)"
  }
};

static ProcArg brushes_get_brush_data_outargs[] =
{
  {
    GIMP_PDB_STRING,
    "name",
    "The brush name"
  },
  {
    GIMP_PDB_FLOAT,
    "opacity",
    "The brush opacity: 0 <= opacity <= 100"
  },
  {
    GIMP_PDB_INT32,
    "spacing",
    "The brush spacing: 0 <= spacing <= 1000"
  },
  {
    GIMP_PDB_INT32,
    "paint_mode",
    "The paint mode: { GIMP_NORMAL_MODE (0), GIMP_DISSOLVE_MODE (1), GIMP_BEHIND_MODE (2), GIMP_MULTIPLY_MODE (3), GIMP_SCREEN_MODE (4), GIMP_OVERLAY_MODE (5), GIMP_DIFFERENCE_MODE (6), GIMP_ADDITION_MODE (7), GIMP_SUBTRACT_MODE (8), GIMP_DARKEN_ONLY_MODE (9), GIMP_LIGHTEN_ONLY_MODE (10), GIMP_HUE_MODE (11), GIMP_SATURATION_MODE (12), GIMP_COLOR_MODE (13), GIMP_VALUE_MODE (14), GIMP_DIVIDE_MODE (15), GIMP_DODGE_MODE (16), GIMP_BURN_MODE (17), GIMP_HARDLIGHT_MODE (18), GIMP_SOFTLIGHT_MODE (19), GIMP_GRAIN_EXTRACT_MODE (20), GIMP_GRAIN_MERGE_MODE (21), GIMP_COLOR_ERASE_MODE (22) }"
  },
  {
    GIMP_PDB_INT32,
    "width",
    "The brush width"
  },
  {
    GIMP_PDB_INT32,
    "height",
    "The brush height"
  },
  {
    GIMP_PDB_INT32,
    "length",
    "Length of brush mask data"
  },
  {
    GIMP_PDB_INT8ARRAY,
    "mask_data",
    "The brush mask data"
  }
};

static ProcRecord brushes_get_brush_data_proc =
{
  "gimp_brushes_get_brush_data",
  "Retrieve information about the currently active brush (including data).",
  "This procedure retrieves information about the currently active brush. This includes the brush name, and the brush extents (width and height). It also returns the brush data.",
  "Andy Thomas",
  "Andy Thomas",
  "1998",
  GIMP_INTERNAL,
  1,
  brushes_get_brush_data_inargs,
  8,
  brushes_get_brush_data_outargs,
  { { brushes_get_brush_data_invoker } }
};
