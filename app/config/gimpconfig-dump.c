/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * GimpConfig object property dumper. 
 * Copyright (C) 2001-2002  Sven Neumann <sven@gimp.org>
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

#include "stdlib.h"
#include "string.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib-object.h>

#include "libgimpbase/gimplimits.h"
#include "libgimpbase/gimpversion.h"
#include "libgimpbase/gimpbasetypes.h"
#include "libgimpbase/gimpversion.h"

#include "config-types.h"

#include "gimpconfig.h"
#include "gimpconfig-params.h"
#include "gimpconfig-serialize.h"
#include "gimpconfig-types.h"
#include "gimprc.h"


static gint    dump_system_gimprc   (gint         fd);
static gint    dump_man_page        (gint         fd);

static gchar * dump_describe_param  (GParamSpec  *param_spec);
static void    dump_with_linebreaks (gint         fd,
				     const gchar *text);


int
main (int   argc,
      char *argv[])
{
  GObject *rc;
  
  g_type_init ();

  if (argc > 1)
    {
      if (strcmp (argv[1], "--system-gimprc") == 0)
	{
	  return dump_system_gimprc (1);
	}
      else if (strcmp (argv[1], "--man-page") == 0)
	{
	  return dump_man_page (1);
	}
      else if (strcmp (argv[1], "--version") == 0)
	{
	  g_printerr ("gimpconfig-dump version %s\n",  GIMP_VERSION);
	  return EXIT_SUCCESS;
	}
      else
	{
	  g_printerr ("gimpconfig-dump version %s\n\n",  GIMP_VERSION);
	  g_printerr ("Usage: %s [option ... ]\n\n", argv[0]);
	  g_printerr ("Options:\n"
		      "  --man-page        create a gimprc manual page\n"
		      "  --system-gimprc   create a commented system gimprc with default values\n"
		      "  --help            output usage information\n"
		      "  --version         output version information\n"
		      "\n");

	  if (strcmp (argv[1], "--help") == 0)
	    return EXIT_SUCCESS;
	  else
	    return EXIT_FAILURE;
	}
    }

  rc = g_object_new (GIMP_TYPE_RC,
                     "module-load-inhibit", "foo",  /* for completeness */
                     NULL);

  g_print ("# Dump of the GIMP default configuration\n\n");
  gimp_config_serialize_properties (rc, 1, 0);
  g_print ("\n");

  g_object_unref (rc);

  return EXIT_SUCCESS;
}


static const gchar *system_gimprc_header =
"# This is the system-wide gimprc file.  Any change made in this file\n"
"# will affect all users of this system, provided that they are not\n"
"# overriding the default values in their personal gimprc file.\n"
"#\n"
"# Lines that start with a '#' are comments. Blank lines are ignored.\n"
"#\n"
"# By default everything in this file is commented out. The file then\n"
"# documents the default values and shows what changes are possible.\n" 
"\n"
"# The variable ${gimp_dir} is set to the value of the environment\n"
"# variable GIMP_DIRECTORY or, if that is not set, the compiled-in\n"
"# default value is used. If GIMP_DIRECTORY is not an absolute path,\n"
"# it is interpreted relative to your home directory.\n"
"\n";

static gint
dump_system_gimprc (gint fd)
{
  GObjectClass  *klass;
  GParamSpec   **property_specs;
  GObject       *rc;
  GString       *str;
  guint          n_property_specs;
  guint          i;

  str = g_string_new (system_gimprc_header);

  write (fd, str->str, str->len);

  rc = g_object_new (GIMP_TYPE_RC,
                     "module-load-inhibit", "foo",  /* for completeness */
                     NULL);

  klass = G_OBJECT_GET_CLASS (rc);

  property_specs = g_object_class_list_properties (klass, &n_property_specs);

  for (i = 0; i < n_property_specs; i++)
    {
      GParamSpec *prop_spec = property_specs[i];
      gchar      *comment;

      if (! (prop_spec->flags & GIMP_PARAM_SERIALIZE))
        continue;

      g_string_assign (str, "");

      comment = dump_describe_param (prop_spec);
      if (comment)
	{
	  gimp_config_serialize_comment (str, comment);
	  g_free (comment);
	  g_string_append (str, "#\n");
	}

      g_string_append (str, "# ");

      if (gimp_config_serialize_property (rc, prop_spec, str, TRUE))
	{
	  g_string_append (str, "\n");

          write (fd, str->str, str->len);
        }
    }

  g_free (property_specs);

  g_object_unref (rc);

  g_string_free (str, TRUE);

  return EXIT_SUCCESS;
}


static const gchar *man_page_header =
".\\\" This man-page is auto-generated by gimpconfig-dump.\n"
"\n"
".TH GIMPRC 5 \"Version @GIMP_VERSION@\" \"GIMP Manual Pages\"\n"
".SH NAME\n"
"gimprc \\- gimp configuration file\n"
".SH DESCRIPTION\n"
"The\n"
".B gimprc\n"
"file is a configuation file read by the GIMP when it starts up.  There\n"
"are two of these: one system-wide one stored in\n"
"@gimpsysconfdir@/gimprc and a per-user \\fB$HOME\\fP/@gimpdir@/gimprc\n"
"which may override system settings.\n"
"\n"
"Comments are introduced by a hash sign (#), and continue until the end\n"
"of the line.  Blank lines are ignored.\n"
"\n"
"The\n"
".B gimprc\n"
"file associates values with properties.  These properties may be set\n"
"by lisp-like assignments of the form:\n"
".IP\n"
"\\f3(\\f2property-name\\ value\\f3)\\f1\n"
".TP\n"
"where:\n"
".TP 10\n"
".I property-name\n"
"is one of the property names described below.\n"
".TP\n"
".I value\n"
"is the value the property is to be set to.\n"
".PP\n"
"\n"
"Either spaces or tabs may be used to separate the name from the value.\n"
".PP\n"
".SH PROPERTIES\n"
"Valid properties and their default values are:\n"
"\n";

static const gchar *man_page_path =
".PP\n"
".SH PATH EXPANSION\n"
"Strings of type PATH are expanded in a manner similar to\n"
".BR bash (1).\n"
"Specifically: tilde (~) is expanded to the user's home directory. Note that\n"
"the bash feature of being able to refer to other user's home directories\n"
"by writing ~userid/ is not valid in this file.\n"
"\n"
"${variable} is expanded to the current value of an environment variable.\n"
"There are a few variables that are pre-defined:\n"
".TP\n"
".I gimp_dir\n"
"The personal gimp directory which is set to the value of the environment\n"
"variable GIMP_DIRECTORY or to ~/@gimpdir@.\n"
".TP\n"
".I gimp_data_dir\n"
"Nase for paths to shareable data, which is set to the value of the\n"
"environment variable GIMP_DATADIR or to the compiled-in default value\n"
"@gimpdatadir@.\n"
".TP\n"
".I gimp_plug_in_dir\n"
"Base to paths for architecture-specific plugins and modules, which is set\n"
"to the value of the environment variable GIMP_PLUGINDIR or to the\n"
"compiled-in default value @gimpplugindir@.\n"
".TP\n"
".I gimp_sysconf_dir\n"
"Path to configuration files, which is set to the value of the environment\n"
"variable GIMP_SYSCONFDIR or to the compiled-in default value \n"
"@gimpsysconfdir@.\n"
"\n";

static const gchar *man_page_footer =
".SH FILES\n"
".TP\n"
".I @gimpsysconfdir@/gimprc\n"
"System-wide configuration file\n"
".TP\n"
".I \\fB$HOME\\fP/@gimpdir@/gimprc\n"
"Per-user configuration file\n"
"\n"
".SH \"SEE ALSO\"\n"
".BR gimp (1),\n"
".BR gimptool (1),\n"
".BR gimp-remote (1)\n";


static gint
dump_man_page (gint fd)
{
  GObjectClass  *klass;
  GParamSpec   **property_specs;
  GObject       *rc;
  GString       *str;
  guint          n_property_specs;
  guint          i;


  write (fd, man_page_header, strlen (man_page_header));

  str = g_string_new (NULL);

  rc = g_object_new (GIMP_TYPE_RC, NULL);
  klass = G_OBJECT_GET_CLASS (rc);

  property_specs = g_object_class_list_properties (klass, &n_property_specs);

  for (i = 0; i < n_property_specs; i++)
    {
      GParamSpec *prop_spec = property_specs[i];
      gchar      *desc;

      if (! (prop_spec->flags & GIMP_PARAM_SERIALIZE))
        continue;

      g_string_assign (str, ".TP\n");

      if (gimp_config_serialize_property (rc, prop_spec, str, TRUE))
	{
	  g_string_append (str, "\n");

          write (fd, str->str, str->len);

	  desc = dump_describe_param (prop_spec);

	  dump_with_linebreaks (fd, desc);
	  write (fd, "\n", 1);

	  g_free (desc);
        }
    }

  g_free (property_specs);

  g_object_unref (rc);

  g_string_free (str, TRUE);

  write (fd, man_page_path,   strlen (man_page_path));
  write (fd, man_page_footer, strlen (man_page_footer));

  return EXIT_SUCCESS;
}


static gchar *
dump_describe_param (GParamSpec *param_spec)
{
  GType        type;
  const gchar *blurb;
  const gchar *values = NULL;

  blurb = g_param_spec_get_blurb (param_spec);

  if (!blurb)
    {
      g_warning ("FIXME: Property '%s' has no blurb.", param_spec->name);

      blurb = g_strdup_printf ("The %s property has no description.",
			       param_spec->name);
    }

  type = param_spec->value_type;

  if (g_type_is_a (type, GIMP_TYPE_COLOR))
    {
      values =
	"The color is specified in the form (color-rgba red green blue alpha) "
	"with channel values as floats between 0.0 and 1.0.";
    }
  else if (g_type_is_a (type, GIMP_TYPE_MEMSIZE))
    {
      values =
	"The integer size can contain a suffix of 'B', 'K', 'M' or 'G' which "
	"makes GIMP interpret the size as being specified in bytes, kilobytes, "
	"megabytes or gigabytes. If no suffix is specified the size defaults "
	"to being specified in kilobytes.";
    }
  else if (g_type_is_a (type, GIMP_TYPE_PATH))
    {
      switch (gimp_param_spec_path_type (param_spec))
	{
	case GIMP_PARAM_PATH_FILE:
	  values = "This is a single filename.";
	  break;

	case GIMP_PARAM_PATH_FILE_LIST:
	  switch (G_SEARCHPATH_SEPARATOR)
	    {
	    case ':':
	      values = "This is a colon-separated list of files.";
	      break;
	    case ';':
	      values = "This is a semicolon-separated list of files.";
	      break;
	    default:
	      g_warning ("unhandled G_SEARCHPATH_SEPARATOR value");
	      break;
	    }
	  break;

	case GIMP_PARAM_PATH_DIR:
	  values = "This is a single folder.";
	  break;
	  
	case GIMP_PARAM_PATH_DIR_LIST:
	  switch (G_SEARCHPATH_SEPARATOR)
	    {
	    case ':':
	      values = "This is a colon-separated list of folders to search.";
	      break;
	    case ';':
	      values = "This is a semicolon-separated list of folders to search.";
	      break;
	    default:
	      g_warning ("unhandled G_SEARCHPATH_SEPARATOR value");
	      break;
	    }
	  break;
	}
    }
  else if (g_type_is_a (type, GIMP_TYPE_UNIT))
    {
      values =
	"The unit can be one inches, millimeters, points or picas plus "
	"those in your user units database.";
    }
  else
    {
      switch (G_TYPE_FUNDAMENTAL (type))
	{
	case G_TYPE_BOOLEAN:
	  values = "Possible values are yes and no.";
	  break;
	case G_TYPE_INT:
	case G_TYPE_UINT:
	case G_TYPE_LONG:
	case G_TYPE_ULONG:
	  values = "This is an integer value.";
	  break;
	case G_TYPE_FLOAT:
	case G_TYPE_DOUBLE:
	  values = "This is a float value.";
	  break;
	case G_TYPE_STRING:
	  values = "This is a string value.";
	  break;
	case G_TYPE_ENUM:
	  {
	    GEnumClass *enum_class;
	    GEnumValue *enum_value;
	    GString    *str;
	    gint        i;
	    
	    enum_class = g_type_class_peek (type);
	    
	    str = g_string_new (blurb);

	    g_string_append (str, "  Possible values are ");

	    for (i = 0, enum_value = enum_class->values;
		 i < enum_class->n_values;
		 i++, enum_value++)
	      {
		g_string_append (str, enum_value->value_nick);

		switch (enum_class->n_values - i)
		  {
		  case 1:
		    g_string_append_c (str, '.');
		    break;
		  case 2:
		    g_string_append (str, " and ");
		    break;
		  default:
		    g_string_append (str, ", ");
		    break;
		  }
	      }

	    return g_string_free (str, FALSE);
	  }
	  break;
	default:
	  break;
	}
    }

  if (!values)
    g_warning ("FIXME: Can't tell anything about a %s.", g_type_name (type));

  return g_strdup_printf ("%s  %s", blurb, values);
}


#define LINE_LENGTH 78

static void
dump_with_linebreaks (gint         fd,
		      const gchar *text)
{
  const gchar *t;
  gint         i, len, space;

  len = strlen (text);

  while (len > 0)
    {
      for (t = text, i = 0, space = 0;
           *t != '\n' && (i <= LINE_LENGTH || space == 0) && i < len;
           t++, i++)
        {
          if (g_ascii_isspace (*t))
            space = i;
        }

      if (i > LINE_LENGTH && space && *t != '\n')
        i = space;

      write (fd, text, i);
      write (fd, "\n", 1);

      i++;

      text += i;
      len  -= i;
    }
}


/*
 * some dummy funcs so we can properly link this beast
 */

const gchar *
gimp_unit_get_identifier (GimpUnit unit)
{
  switch (unit)
    {
    case GIMP_UNIT_PIXEL:
      return "pixels";
    case GIMP_UNIT_INCH:
      return "inches";
    case GIMP_UNIT_MM:
      return "millimeters";
    case GIMP_UNIT_POINT:
      return "points";
    case GIMP_UNIT_PICA:
      return "picas";
    default:
      return NULL;
    }
}

gint
gimp_unit_get_number_of_units (void)
{
  return GIMP_UNIT_END;
}
