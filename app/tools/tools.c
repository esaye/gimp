/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-2001 Spencer Kimball, Peter Mattis and others
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

#include <gtk/gtk.h>

#include "libgimpbase/gimpbase.h"

#include "tools-types.h"

#include "tool_manager.h"

#include "gimpairbrushtool.h"
#include "gimpbezierselecttool.h"
#include "gimpblendtool.h"
#include "gimpbrightnesscontrasttool.h"
#include "gimpbucketfilltool.h"
#include "gimpbycolorselecttool.h"
#include "gimpclonetool.h"
#include "gimpcolorbalancetool.h"
#include "gimpcolorpickertool.h"
#include "gimpconvolvetool.h"
#include "gimpcroptool.h"
#include "gimpcurvestool.h"
#include "gimpdodgeburntool.h"
#include "gimpellipseselecttool.h"
#include "gimperasertool.h"
#include "gimpfliptool.h"
#include "gimpfreeselecttool.h"
#include "gimpfuzzyselecttool.h"
#include "gimphistogramtool.h"
#include "gimphuesaturationtool.h"
#include "gimpinktool.h"
#include "gimpiscissorstool.h"
#include "gimplevelstool.h"
#include "gimpmagnifytool.h"
#include "gimpmeasuretool.h"
#include "gimpmovetool.h"
#include "gimppaintbrushtool.h"
#include "gimppenciltool.h"
#include "gimpperspectivetool.h"
#include "gimpposterizetool.h"
#include "gimprectselecttool.h"
#include "gimpthresholdtool.h"
#include "gimprotatetool.h"
#include "gimpscaletool.h"
#include "gimpsheartool.h"
#include "gimpsmudgetool.h"
#include "gimptexttool.h"
#include "gimpvectortool.h"


void
tools_init (Gimp *gimp)
{
  GimpToolRegisterFunc register_funcs[] =
  {
    /*  register tools in reverse order  */

    /*  color tools  */
    gimp_posterize_tool_register,
    gimp_curves_tool_register,
    gimp_levels_tool_register,
    gimp_threshold_tool_register,
    gimp_brightness_contrast_tool_register,
    gimp_hue_saturation_tool_register,
    gimp_color_balance_tool_register,

    /*  paint tools  */

    gimp_smudge_tool_register,
    gimp_dodgeburn_tool_register,
    gimp_convolve_tool_register,
    gimp_clone_tool_register,
    gimp_ink_tool_register,
    gimp_airbrush_tool_register,
    gimp_eraser_tool_register,
    gimp_paintbrush_tool_register,
    gimp_pencil_tool_register,
    gimp_blend_tool_register,
    gimp_bucket_fill_tool_register,
    gimp_text_tool_register,

    /*  transform tools  */

    gimp_flip_tool_register,
    gimp_perspective_tool_register,
    gimp_shear_tool_register,
    gimp_scale_tool_register,
    gimp_rotate_tool_register,
    gimp_crop_tool_register,
    gimp_move_tool_register,

    /*  non-modifying tools  */

    gimp_vector_tool_register,
    gimp_measure_tool_register,
    gimp_magnify_tool_register,
    gimp_histogram_tool_register,
    gimp_color_picker_tool_register,

    /*  selection tools */

    gimp_bezier_select_tool_register,
    gimp_iscissors_tool_register,
    gimp_by_color_select_tool_register,
    gimp_fuzzy_select_tool_register,
    gimp_free_select_tool_register,
    gimp_ellipse_select_tool_register,
    gimp_rect_select_tool_register
  };

  gint i;

  for (i = 0; i < G_N_ELEMENTS (register_funcs); i++)
    {
      register_funcs[i] (tool_manager_register_tool, gimp);
    }
}

void
tools_exit (Gimp *gimp)
{
}
