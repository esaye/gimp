/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
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

#include <string.h>

#include <glib-object.h>

#include "libgimpbase/gimputils.h"

#include "core-types.h"

#include "gimpmarshal.h"
#include "gimpobject.h"


enum
{
  DISCONNECT,
  NAME_CHANGED,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_NAME
};


static void    gimp_object_class_init       (GimpObjectClass *klass);
static void    gimp_object_init             (GimpObject      *object);

static void    gimp_object_dispose          (GObject         *object);
static void    gimp_object_finalize         (GObject         *object);
static void    gimp_object_set_property     (GObject         *object,
                                             guint            property_id,
                                             const GValue    *value,
                                             GParamSpec      *pspec);
static void    gimp_object_get_property     (GObject         *object,
                                             guint            property_id,
                                             GValue          *value,
                                             GParamSpec      *pspec);
static gsize   gimp_object_real_get_memsize (GimpObject      *object);


static guint   object_signals[LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;


GType 
gimp_object_get_type (void)
{
  static GType object_type = 0;

  if (! object_type)
    {
      static const GTypeInfo object_info =
      {
        sizeof (GimpObjectClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_object_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data     */
	sizeof (GimpObject),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_object_init,
      };

      object_type = g_type_register_static (G_TYPE_OBJECT,
					    "GimpObject", 
					    &object_info, 0);
    }

  return object_type;
}

static void
gimp_object_class_init (GimpObjectClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_signals[DISCONNECT] =
    g_signal_new ("disconnect",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GimpObjectClass, disconnect),
		  NULL, NULL,
		  gimp_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  object_signals[NAME_CHANGED] =
    g_signal_new ("name_changed",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (GimpObjectClass, name_changed),
		  NULL, NULL,
		  gimp_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  object_class->dispose      = gimp_object_dispose;
  object_class->finalize     = gimp_object_finalize;
  object_class->set_property = gimp_object_set_property;
  object_class->get_property = gimp_object_get_property;

  klass->disconnect          = NULL;
  klass->name_changed        = NULL;
  klass->get_memsize         = gimp_object_real_get_memsize;

  g_object_class_install_property (object_class,
				   PROP_NAME,
				   g_param_spec_string ("name",
							NULL, NULL,
							NULL,
							G_PARAM_READWRITE));
}

static void
gimp_object_init (GimpObject *object)
{
  object->name = NULL;
}

static void
gimp_object_dispose (GObject *object)
{
  gboolean disconnected;

  disconnected = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (object),
						     "disconnected"));

  if (! disconnected)
    {
      g_signal_emit (object, object_signals[DISCONNECT], 0);

      g_object_set_data (G_OBJECT (object), "disconnected",
			 GINT_TO_POINTER (TRUE));
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gimp_object_finalize (GObject *object)
{
  GimpObject *gimp_object;

  gimp_object = GIMP_OBJECT (object);

  if (gimp_object->name)
    {
      g_free (gimp_object->name);
      gimp_object->name = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gimp_object_set_property (GObject      *object,
			  guint         property_id,
			  const GValue *value,
			  GParamSpec   *pspec)
{
  GimpObject *gimp_object;

  gimp_object = GIMP_OBJECT (object);

  switch (property_id)
    {
    case PROP_NAME:
      gimp_object_set_name (gimp_object, g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
gimp_object_get_property (GObject    *object,
			  guint       property_id,
			  GValue     *value,
			  GParamSpec *pspec)
{
  GimpObject *gimp_object;

  gimp_object = GIMP_OBJECT (object);

  switch (property_id)
    {
    case PROP_NAME:
      g_value_set_string (value, gimp_object->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

void
gimp_object_set_name (GimpObject  *object,
		      const gchar *name)
{
  g_return_if_fail (GIMP_IS_OBJECT (object));

  if ((!object->name && !name) ||
      (object->name && name && !strcmp (object->name, name)))
    return;

  g_free (object->name);

  object->name = g_strdup (name);

  gimp_object_name_changed (object);
}

/*  A safe version of gimp_object_set_name() that takes care
 *  of newlines and overly long names.
 */
void
gimp_object_set_name_safe (GimpObject  *object,
                           const gchar *name)
{
  g_return_if_fail (GIMP_IS_OBJECT (object));

  if ((!object->name && !name) ||
      (object->name && name && !strcmp (object->name, name)))
    return;

  g_free (object->name);

  object->name = gimp_utf8_strtrim (name, 30);

  gimp_object_name_changed (object);
}

const gchar *
gimp_object_get_name (const GimpObject *object)
{
  g_return_val_if_fail (GIMP_IS_OBJECT (object), NULL);

  return object->name;
}

void
gimp_object_name_changed (GimpObject *object)
{
  g_return_if_fail (GIMP_IS_OBJECT (object));

  g_signal_emit (object, object_signals[NAME_CHANGED], 0);
}


#define DEBUG_MEMSIZE 1

#ifdef DEBUG_MEMSIZE
gboolean gimp_debug_memsize = FALSE;
#endif

gsize
gimp_object_get_memsize (GimpObject *object)
{
  g_return_val_if_fail (GIMP_IS_OBJECT (object), 0);

#ifdef DEBUG_MEMSIZE
  if (gimp_debug_memsize)
    {
      static gint   indent_level     = 0;
      static GList *aggregation_tree = NULL;
      static gchar  indent_buf[256];

      gsize  memsize;
      gint   i;
      gint   my_indent_level;
      gchar *object_size;

      indent_level++;

      my_indent_level = indent_level;

      memsize = GIMP_OBJECT_GET_CLASS (object)->get_memsize (object);

      indent_level--;

      for (i = 0; i < MIN (my_indent_level * 2, sizeof (indent_buf) - 1); i++)
        indent_buf[i] = ' ';

      indent_buf[i] = '\0';

      /* FIXME: are we going to ever have > 4 GB objects?? */
      object_size = g_strdup_printf ("%s%s \"%s\": %u\n",
                                     indent_buf,
                                     g_type_name (G_TYPE_FROM_INSTANCE (object)),
                                     object->name,
                                     (guint) memsize);

      aggregation_tree = g_list_prepend (aggregation_tree, object_size);

      if (indent_level == 0)
        {
          g_list_foreach (aggregation_tree, (GFunc) g_print, NULL);
          g_list_foreach (aggregation_tree, (GFunc) g_free, NULL);
          g_list_free (aggregation_tree);

          aggregation_tree = NULL;
        }

      return memsize;
    }
#endif /* DEBUG_MEMSIZE */

  return GIMP_OBJECT_GET_CLASS (object)->get_memsize (object);
}

gsize
gimp_g_object_get_memsize (GObject *object)
{
  GTypeQuery type_query;
  gsize      memsize = 0;

  g_return_val_if_fail (G_IS_OBJECT (object), 0);

  g_type_query (G_TYPE_FROM_INSTANCE (object), &type_query);

  memsize += type_query.instance_size;

  return memsize;
}

static gsize
gimp_object_real_get_memsize (GimpObject *object)
{
  gsize memsize = 0;

  if (object->name)
    memsize += strlen (object->name) + 1;

  return memsize + gimp_g_object_get_memsize ((GObject *) object);
}
