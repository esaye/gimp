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


#include <glib-object.h>

#include "libgimpcolor/gimpcolor.h"

#include "pdb-types.h"
#include "procedural_db.h"

#include "core/gimp.h"
#include "core/gimpcontext.h"
#include "plug-in/plug-in-context.h"
#include "plug-in/plug-in.h"

static ProcRecord context_push_proc;
static ProcRecord context_pop_proc;
static ProcRecord context_get_foreground_proc;
static ProcRecord context_set_foreground_proc;
static ProcRecord context_get_background_proc;
static ProcRecord context_set_background_proc;
static ProcRecord context_set_default_colors_proc;
static ProcRecord context_swap_colors_proc;
static ProcRecord context_get_opacity_proc;
static ProcRecord context_set_opacity_proc;
static ProcRecord context_get_paint_mode_proc;
static ProcRecord context_set_paint_mode_proc;

void
register_context_procs (Gimp *gimp)
{
  procedural_db_register (gimp, &context_push_proc);
  procedural_db_register (gimp, &context_pop_proc);
  procedural_db_register (gimp, &context_get_foreground_proc);
  procedural_db_register (gimp, &context_set_foreground_proc);
  procedural_db_register (gimp, &context_get_background_proc);
  procedural_db_register (gimp, &context_set_background_proc);
  procedural_db_register (gimp, &context_set_default_colors_proc);
  procedural_db_register (gimp, &context_swap_colors_proc);
  procedural_db_register (gimp, &context_get_opacity_proc);
  procedural_db_register (gimp, &context_set_opacity_proc);
  procedural_db_register (gimp, &context_get_paint_mode_proc);
  procedural_db_register (gimp, &context_set_paint_mode_proc);
}

static Argument *
context_push_invoker (Gimp         *gimp,
                      GimpContext  *context,
                      GimpProgress *progress,
                      Argument     *args)
{
  gboolean success = TRUE;
  if (gimp->current_plug_in && gimp->current_plug_in->open)
    {
      success = plug_in_context_push (gimp->current_plug_in);
    }
  else
    success = FALSE;
  return procedural_db_return_args (&context_push_proc, success);
}

static ProcRecord context_push_proc =
{
  "gimp_context_push",
  "Pushes a context to the top of the plug-in's context stack.",
  "This procedure creates a new context by copying the current context. This copy becomes the new current context for the calling plug-in until it is popped again.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  0,
  NULL,
  { { context_push_invoker } }
};

static Argument *
context_pop_invoker (Gimp         *gimp,
                     GimpContext  *context,
                     GimpProgress *progress,
                     Argument     *args)
{
  gboolean success = TRUE;
  if (gimp->current_plug_in && gimp->current_plug_in->open)
    {
      success = plug_in_context_push (gimp->current_plug_in);
    }
  else
    success = FALSE;
  return procedural_db_return_args (&context_pop_proc, success);
}

static ProcRecord context_pop_proc =
{
  "gimp_context_pop",
  "Pops the topmost context from the plug-in's context stack.",
  "This procedure creates a new context and makes it the current context for the calling plug-in.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  0,
  NULL,
  { { context_pop_invoker } }
};

static Argument *
context_get_foreground_invoker (Gimp         *gimp,
                                GimpContext  *context,
                                GimpProgress *progress,
                                Argument     *args)
{
  Argument *return_args;
  GimpRGB color;

  gimp_context_get_foreground (context, &color);

  return_args = procedural_db_return_args (&context_get_foreground_proc, TRUE);
  return_args[1].value.pdb_color = color;

  return return_args;
}

static ProcArg context_get_foreground_outargs[] =
{
  {
    GIMP_PDB_COLOR,
    "foreground",
    "The foreground color"
  }
};

static ProcRecord context_get_foreground_proc =
{
  "gimp_context_get_foreground",
  "Get the current GIMP foreground color.",
  "This procedure retrieves the current GIMP foreground color. The foreground color is used in a variety of tools such as paint tools, blending, and bucket fill.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  1,
  context_get_foreground_outargs,
  { { context_get_foreground_invoker } }
};

static Argument *
context_set_foreground_invoker (Gimp         *gimp,
                                GimpContext  *context,
                                GimpProgress *progress,
                                Argument     *args)
{
  GimpRGB color;

  color = args[0].value.pdb_color;

  gimp_rgb_set_alpha (&color, 1.0);
  gimp_context_set_foreground (context, &color);

  return procedural_db_return_args (&context_set_foreground_proc, TRUE);
}

static ProcArg context_set_foreground_inargs[] =
{
  {
    GIMP_PDB_COLOR,
    "foreground",
    "The foreground color"
  }
};

static ProcRecord context_set_foreground_proc =
{
  "gimp_context_set_foreground",
  "Set the current GIMP foreground color.",
  "This procedure sets the current GIMP foreground color. After this is set, operations which use foreground such as paint tools, blending, and bucket fill will use the new value.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  1,
  context_set_foreground_inargs,
  0,
  NULL,
  { { context_set_foreground_invoker } }
};

static Argument *
context_get_background_invoker (Gimp         *gimp,
                                GimpContext  *context,
                                GimpProgress *progress,
                                Argument     *args)
{
  Argument *return_args;
  GimpRGB color;

  gimp_context_get_background (context, &color);

  return_args = procedural_db_return_args (&context_get_background_proc, TRUE);
  return_args[1].value.pdb_color = color;

  return return_args;
}

static ProcArg context_get_background_outargs[] =
{
  {
    GIMP_PDB_COLOR,
    "foreground",
    "The foreground color"
  }
};

static ProcRecord context_get_background_proc =
{
  "gimp_context_get_background",
  "Get the current GIMP background color.",
  "This procedure retrieves the current GIMP background color. The background color is used in a variety of tools such as blending, erasing (with non-alpha images), and image filling.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  1,
  context_get_background_outargs,
  { { context_get_background_invoker } }
};

static Argument *
context_set_background_invoker (Gimp         *gimp,
                                GimpContext  *context,
                                GimpProgress *progress,
                                Argument     *args)
{
  GimpRGB color;

  color = args[0].value.pdb_color;

  gimp_rgb_set_alpha (&color, 1.0);
  gimp_context_set_foreground (context, &color);

  return procedural_db_return_args (&context_set_background_proc, TRUE);
}

static ProcArg context_set_background_inargs[] =
{
  {
    GIMP_PDB_COLOR,
    "background",
    "The background color"
  }
};

static ProcRecord context_set_background_proc =
{
  "gimp_context_set_background",
  "Set the current GIMP background color.",
  "This procedure sets the current GIMP background color. After this is set, operations which use background such as blending, filling images, clearing, and erasing (in non-alpha images) will use the new value.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  1,
  context_set_background_inargs,
  0,
  NULL,
  { { context_set_background_invoker } }
};

static Argument *
context_set_default_colors_invoker (Gimp         *gimp,
                                    GimpContext  *context,
                                    GimpProgress *progress,
                                    Argument     *args)
{
  gimp_context_set_default_colors (context);
  return procedural_db_return_args (&context_set_default_colors_proc, TRUE);
}

static ProcRecord context_set_default_colors_proc =
{
  "gimp_context_set_default_colors",
  "Set the current GIMP foreground and background colors to black and white.",
  "This procedure sets the current GIMP foreground and background colors to their initial default values, black and white.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  0,
  NULL,
  { { context_set_default_colors_invoker } }
};

static Argument *
context_swap_colors_invoker (Gimp         *gimp,
                             GimpContext  *context,
                             GimpProgress *progress,
                             Argument     *args)
{
  gimp_context_swap_colors (context);
  return procedural_db_return_args (&context_swap_colors_proc, TRUE);
}

static ProcRecord context_swap_colors_proc =
{
  "gimp_context_swap_colors",
  "Swap the current GIMP foreground and background colors.",
  "This procedure swaps the current GIMP foreground and background colors, so that the new foreground color becomes the old background color and vice versa.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  0,
  NULL,
  { { context_swap_colors_invoker } }
};

static Argument *
context_get_opacity_invoker (Gimp         *gimp,
                             GimpContext  *context,
                             GimpProgress *progress,
                             Argument     *args)
{
  Argument *return_args;

  return_args = procedural_db_return_args (&context_get_opacity_proc, TRUE);
  return_args[1].value.pdb_float = gimp_context_get_opacity (context) * 100.0;

  return return_args;
}

static ProcArg context_get_opacity_outargs[] =
{
  {
    GIMP_PDB_FLOAT,
    "opacity",
    "The opacity: 0 <= opacity <= 100"
  }
};

static ProcRecord context_get_opacity_proc =
{
  "gimp_context_get_opacity",
  "Get the opacity.",
  "This procedure returns the opacity setting. The return value is a floating point number between 0 and 100.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  1,
  context_get_opacity_outargs,
  { { context_get_opacity_invoker } }
};

static Argument *
context_set_opacity_invoker (Gimp         *gimp,
                             GimpContext  *context,
                             GimpProgress *progress,
                             Argument     *args)
{
  gboolean success = TRUE;
  gdouble opacity;

  opacity = args[0].value.pdb_float;
  if (opacity < 0.0 || opacity > 100.0)
    success = FALSE;

  if (success)
    gimp_context_set_opacity (context, opacity / 100.0);

  return procedural_db_return_args (&context_set_opacity_proc, success);
}

static ProcArg context_set_opacity_inargs[] =
{
  {
    GIMP_PDB_FLOAT,
    "opacity",
    "The opacity: 0 <= opacity <= 100"
  }
};

static ProcRecord context_set_opacity_proc =
{
  "gimp_context_set_opacity",
  "Set the opacity.",
  "This procedure modifies the opacity setting. The value should be a floating point number between 0 and 100.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  1,
  context_set_opacity_inargs,
  0,
  NULL,
  { { context_set_opacity_invoker } }
};

static Argument *
context_get_paint_mode_invoker (Gimp         *gimp,
                                GimpContext  *context,
                                GimpProgress *progress,
                                Argument     *args)
{
  Argument *return_args;

  return_args = procedural_db_return_args (&context_get_paint_mode_proc, TRUE);
  return_args[1].value.pdb_int = gimp_context_get_paint_mode (context);

  return return_args;
}

static ProcArg context_get_paint_mode_outargs[] =
{
  {
    GIMP_PDB_INT32,
    "paint_mode",
    "The paint mode: { GIMP_NORMAL_MODE (0), GIMP_DISSOLVE_MODE (1), GIMP_BEHIND_MODE (2), GIMP_MULTIPLY_MODE (3), GIMP_SCREEN_MODE (4), GIMP_OVERLAY_MODE (5), GIMP_DIFFERENCE_MODE (6), GIMP_ADDITION_MODE (7), GIMP_SUBTRACT_MODE (8), GIMP_DARKEN_ONLY_MODE (9), GIMP_LIGHTEN_ONLY_MODE (10), GIMP_HUE_MODE (11), GIMP_SATURATION_MODE (12), GIMP_COLOR_MODE (13), GIMP_VALUE_MODE (14), GIMP_DIVIDE_MODE (15), GIMP_DODGE_MODE (16), GIMP_BURN_MODE (17), GIMP_HARDLIGHT_MODE (18), GIMP_SOFTLIGHT_MODE (19), GIMP_GRAIN_EXTRACT_MODE (20), GIMP_GRAIN_MERGE_MODE (21), GIMP_COLOR_ERASE_MODE (22) }"
  }
};

static ProcRecord context_get_paint_mode_proc =
{
  "gimp_context_get_paint_mode",
  "Get the paint mode.",
  "This procedure returns the paint-mode setting. The return value is an integer which corresponds to the values listed in the argument description.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  0,
  NULL,
  1,
  context_get_paint_mode_outargs,
  { { context_get_paint_mode_invoker } }
};

static Argument *
context_set_paint_mode_invoker (Gimp         *gimp,
                                GimpContext  *context,
                                GimpProgress *progress,
                                Argument     *args)
{
  gboolean success = TRUE;
  gint32 paint_mode;

  paint_mode = args[0].value.pdb_int;
  if (paint_mode < GIMP_NORMAL_MODE || paint_mode > GIMP_COLOR_ERASE_MODE)
    success = FALSE;

  if (success)
    gimp_context_set_paint_mode (context, paint_mode);

  return procedural_db_return_args (&context_set_paint_mode_proc, success);
}

static ProcArg context_set_paint_mode_inargs[] =
{
  {
    GIMP_PDB_INT32,
    "paint_mode",
    "The paint mode: { GIMP_NORMAL_MODE (0), GIMP_DISSOLVE_MODE (1), GIMP_BEHIND_MODE (2), GIMP_MULTIPLY_MODE (3), GIMP_SCREEN_MODE (4), GIMP_OVERLAY_MODE (5), GIMP_DIFFERENCE_MODE (6), GIMP_ADDITION_MODE (7), GIMP_SUBTRACT_MODE (8), GIMP_DARKEN_ONLY_MODE (9), GIMP_LIGHTEN_ONLY_MODE (10), GIMP_HUE_MODE (11), GIMP_SATURATION_MODE (12), GIMP_COLOR_MODE (13), GIMP_VALUE_MODE (14), GIMP_DIVIDE_MODE (15), GIMP_DODGE_MODE (16), GIMP_BURN_MODE (17), GIMP_HARDLIGHT_MODE (18), GIMP_SOFTLIGHT_MODE (19), GIMP_GRAIN_EXTRACT_MODE (20), GIMP_GRAIN_MERGE_MODE (21), GIMP_COLOR_ERASE_MODE (22) }"
  }
};

static ProcRecord context_set_paint_mode_proc =
{
  "gimp_context_set_paint_mode",
  "Set the paint mode.",
  "This procedure modifies the paint_mode setting.",
  "Michael Natterer <mitch@gimp.org> & Sven Neumann <sven@gimp.org>",
  "Michael Natterer & Sven Neumann",
  "2004",
  GIMP_INTERNAL,
  1,
  context_set_paint_mode_inargs,
  0,
  NULL,
  { { context_set_paint_mode_invoker } }
};
