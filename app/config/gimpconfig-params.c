/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * ParamSpecs for config objects
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org>
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

#include "gimpconfig-params.h"
#include "gimpconfig-types.h"


static void  gimp_param_memsize_class_init (GParamSpecClass *class);


GType
gimp_param_memsize_get_type (void)
{
  static GType spec_type = 0;

  if (!spec_type)
    {
      static const GTypeInfo type_info = 
      {
        sizeof (GParamSpecClass),
        NULL, NULL, 
        (GClassInitFunc) gimp_param_memsize_class_init, 
        NULL, NULL,
        sizeof (GParamSpecUInt),
        0, NULL, NULL
      };

      spec_type = g_type_register_static (G_TYPE_PARAM_UINT, 
                                          "GimpParamMemsize", 
                                          &type_info, 0);
    }
  
  return spec_type;
}

static void
gimp_param_memsize_class_init (GParamSpecClass *class)
{
  class->value_type = GIMP_TYPE_MEMSIZE;
}

GParamSpec *
gimp_param_spec_memsize (const gchar    *name,
                         const gchar    *nick,
                         const gchar    *blurb,
                         guint           minimum,
                         guint           maximum,
                         guint           default_value,
                         GParamFlags     flags)
{
  GParamSpecUInt *mspec;

  mspec = g_param_spec_internal (GIMP_TYPE_PARAM_MEMSIZE,
                                 name, nick, blurb, flags);
  
  mspec->minimum = minimum;
  mspec->maximum = maximum;
  mspec->default_value = default_value;
  
  return G_PARAM_SPEC (mspec);
}
