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

#include <glib-object.h>

#include "paint-types.h"

#include "core/gimp.h"
#include "core/gimplist.h"
#include "core/gimppaintinfo.h"

#include "gimp-paint.h"
#include "gimpairbrush.h"
#include "gimpclone.h"
#include "gimpconvolve.h"
#include "gimpdodgeburn.h"
#include "gimperaser.h"
#include "gimpink.h"
#include "gimppaintoptions.h"
#include "gimppaintbrush.h"
#include "gimppencil.h"
#include "gimpsmudge.h"


/*  local function prototypes  */

static void   gimp_paint_register (Gimp        *gimp,
                                   GType        paint_type,
                                   GType        paint_options_type,
                                   const gchar *blurb);


/*  public functions  */

void
gimp_paint_init (Gimp *gimp)
{
  GimpPaintRegisterFunc register_funcs[] =
  {
    gimp_dodge_burn_register,
    gimp_smudge_register,
    gimp_convolve_register,
    gimp_clone_register,
    gimp_ink_register,
    gimp_airbrush_register,
    gimp_eraser_register,
    gimp_paintbrush_register,
    gimp_pencil_register
  };

  gint i;

  g_return_if_fail (GIMP_IS_GIMP (gimp));

  gimp->paint_info_list = gimp_list_new (GIMP_TYPE_PAINT_INFO, FALSE);
  gimp_object_set_name (GIMP_OBJECT (gimp->paint_info_list), "paint infos");

  for (i = 0; i < G_N_ELEMENTS (register_funcs); i++)
    {
      register_funcs[i] (gimp, gimp_paint_register);
    }
}

void
gimp_paint_exit (Gimp *gimp)
{
  g_return_if_fail (GIMP_IS_GIMP (gimp));

  if (gimp->paint_info_list)
    {
      g_object_unref (gimp->paint_info_list);
      gimp->paint_info_list = NULL;
    }
}


/*  private functions  */

static void
gimp_paint_register (Gimp        *gimp,
                     GType        paint_type,
                     GType        paint_options_type,
                     const gchar *blurb)
{
  GimpPaintInfo *paint_info;

  g_return_if_fail (GIMP_IS_GIMP (gimp));
  g_return_if_fail (g_type_is_a (paint_type, GIMP_TYPE_PAINT_CORE));
  g_return_if_fail (g_type_is_a (paint_options_type, GIMP_TYPE_PAINT_OPTIONS));
  g_return_if_fail (blurb != NULL);

  paint_info = gimp_paint_info_new (gimp,
                                    paint_type,
                                    paint_options_type,
                                    blurb);

  gimp_container_add (gimp->paint_info_list, GIMP_OBJECT (paint_info));
  g_object_unref (paint_info);
}
