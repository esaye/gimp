/* CSource - GIMP Plugin to dump image data in RGB(A) format for C source
 * Copyright (C) 1999 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * This plugin is heavily based on the header plugin by Spencer Kimball and
 * Peter Mattis.
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libgimp/gimp.h"
#include "libgimp/gimpui.h"
#include <gtk/gtk.h>


typedef struct {
  gchar   *file_name;
  gchar   *prefixed_name;
  gchar   *comment;
  gboolean use_comment;
  gboolean glib_types;
  gboolean alpha;
  gboolean use_macros;
  gboolean use_rle;
  gdouble  opacity;
} Config;


/* --- prototypes --- */
static void	query		(void);
static void	run		(gchar		*name,
				 gint		 nparams,
				 GParam		*param,
				 gint		*nreturn_vals,
				 GParam	       **return_vals);
static gint	save_image	(Config		*config,
				 gint32		 image_ID,
				 gint32		 drawable_ID);
static gboolean	run_save_dialog	(Config         *config);


/* --- variables --- */
GPlugInInfo PLUG_IN_INFO =
{
  NULL,    /* init_proc */
  NULL,    /* quit_proc */
  query,   /* query_proc */
  run,     /* run_proc */
};

/* --- implement main (), provided by libgimp --- */
MAIN ()

/* --- functions --- */
static void
query (void)
{
  static GParamDef save_args[] =
  {
    { PARAM_INT32, "run_mode", "Interactive" },
    { PARAM_IMAGE, "image", "Input image" },
    { PARAM_DRAWABLE, "drawable", "Drawable to save" },
    { PARAM_STRING, "filename", "The name of the file to save the image in" },
    { PARAM_STRING, "raw_filename", "The name of the file to save the image in" },
  };
  static int nsave_args = sizeof (save_args) / sizeof (save_args[0]);
  
  gimp_install_procedure ("file_csource_save",
                          "Dump image data in RGB(A) format for C source",
                          "FIXME: write help",
                          "Tim Janik",
                          "Tim Janik",
                          "1999",
                          "<Save>/C-Source",
			  "RGB*",
                          PROC_PLUG_IN,
                          nsave_args, 0,
                          save_args, NULL);
  
  gimp_register_save_handler ("file_csource_save", "c", "");
}

static void
run (gchar   *name,
     gint     nparams,
     GParam  *param,
     gint    *nreturn_vals,
     GParam **return_vals)
{
  static GParam values[2];
  GRunModeType run_mode;
  GimpExportReturnType export = EXPORT_CANCEL;
  
  run_mode = param[0].data.d_int32;
  
  *nreturn_vals = 1;
  *return_vals = values;
  values[0].type = PARAM_STATUS;
  values[0].data.d_status = STATUS_CALLING_ERROR;
  
  if (run_mode == RUN_INTERACTIVE &&
      strcmp (name, "file_csource_save") == 0)
    {
      gint32 image_ID = param[1].data.d_int32;
      gint32 drawable_ID = param[2].data.d_int32;
      Parasite *parasite;
      gchar *x;
      GDrawableType drawable_type = gimp_drawable_type (drawable_ID);
      Config config = 
      {
	NULL,			/* file_name */
	"gimp_image",		/* prefixed_name */
	NULL,			/* comment */
	FALSE,			/* use_comment */
	TRUE,			/* glib_types */
	FALSE,			/* alpha */
	FALSE,			/* use_macros */
	FALSE,			/* use_rle */
	100.0,			/* opacity */
      };

      config.file_name = param[3].data.d_string;
      config.alpha = (drawable_type == RGBA_IMAGE ||
		      drawable_type == GRAYA_IMAGE ||
		      drawable_type == INDEXEDA_IMAGE);

      parasite = gimp_image_parasite_find (image_ID, "gimp-comment");
      if (parasite)
	{
	  config.comment = g_strdup (parasite->data);
	  parasite_free (parasite);
	}
      x = config.comment;
      
      if (run_save_dialog (&config))
	{
	  if (x != config.comment &&
	      !(x && config.comment && strcmp (x, config.comment) == 0))
	    {
	      if (!config.comment || !config.comment[0])
		gimp_image_parasite_detach (image_ID, "gimp-comment");
	      else
		{
		  parasite = parasite_new ("gimp-comment",
					   PARASITE_PERSISTENT,
					   strlen (config.comment) + 1,
					   config.comment);
		  gimp_image_parasite_attach (image_ID, parasite);
		  parasite_free (parasite);
		}
	    }

	  *nreturn_vals = 1;

	  

	  export = gimp_export_image (&image_ID, &drawable_ID, "C Source", 
				      (CAN_HANDLE_RGB | CAN_HANDLE_ALPHA));

	  if (export == EXPORT_CANCEL)
	    {
	      values[0].data.d_status = STATUS_EXECUTION_ERROR;
	      return;
	    }

	  if (save_image (&config, image_ID, drawable_ID))
	    values[0].data.d_status = STATUS_SUCCESS;
	  else
	    values[0].data.d_status = STATUS_EXECUTION_ERROR;

	  if (export == EXPORT_EXPORT)
	    gimp_image_delete (image_ID);
	}
    }
}

static gboolean
diff2_rgb (guint8 *ip)
{
  return ip[0] != ip[3] || ip[1] != ip[4] || ip[2] != ip[5];
}

static gboolean
diff2_rgba (guint8 *ip)
{
  return ip[0] != ip[4] || ip[1] != ip[5] || ip[2] != ip[6] || ip[3] != ip[7];
}

static guint8*
rl_encode_rgbx (guint8 *bp,
		guint8 *ip,
		guint8 *limit,
		guint   n_ch)
{
  gboolean (*diff2_pix) (guint8 *) = n_ch > 3 ? diff2_rgba : diff2_rgb;
  guint8 *ilimit = limit - n_ch;

  while (ip < limit)
    {
      g_assert (ip < ilimit); /* paranoid */

      if (diff2_pix (ip))
	{
	  guint8 *s_ip = ip;
	  guint l = 1;

	  ip += n_ch;
	  while (l < 127 && ip < ilimit && diff2_pix (ip))
	    { ip += n_ch; l += 1; }
	  if (ip == ilimit && l < 127)
            { ip += n_ch; l += 1; }
	  *(bp++) = l;
	  memcpy (bp, s_ip, l * n_ch);
	  bp += l * n_ch;
	}
      else
	{
	  guint l = 2;

	  ip += n_ch;
	  while (l < 127 && ip < ilimit && !diff2_pix (ip))
            { ip += n_ch; l += 1; }
	  *(bp++) = l | 128;
	  memcpy (bp, ip, n_ch);
	  ip += n_ch;
	  bp += n_ch;
	}
      if (ip == ilimit)
	{
	  *(bp++) = 1;
	  memcpy (bp, ip, n_ch);
	  ip += n_ch;
	  bp += n_ch;
	}
    }

  return bp;
}

static inline void
save_rle_decoder (FILE        *fp,
		  const gchar *macro_name,
		  const gchar *s_uint,
		  const gchar *s_uint_8,
		  guint        n_ch)
{
  fprintf (fp, "#define %s_RUN_LENGTH_DECODE(image_buf, rle_data, size, bpp) do \\\n",
	   macro_name);
  fprintf (fp, "{ %s __bpp; %s *__ip; const %s *__il, *__rd; \\\n", s_uint, s_uint_8, s_uint_8);
  fprintf (fp, "  __bpp = (bpp); __ip = (image_buf); __il = __ip + (size) * __bpp; \\\n");

  fprintf (fp, "  __rd = (rle_data); if (__bpp > 3) { /* RGBA */ \\\n");

  fprintf (fp, "    while (__ip < __il) { %s __l = *(__rd++); \\\n", s_uint);
  fprintf (fp, "      if (__l & 128) { __l = __l - 128; \\\n");
  fprintf (fp, "        do { memcpy (__ip, __rd, 4); __ip += 4; } while (--__l); __rd += 4; \\\n");
  fprintf (fp, "      } else { __l *= 4; memcpy (__ip, __rd, __l); \\\n");
  fprintf (fp, "               __ip += __l; __rd += __l; } } \\\n");

  fprintf (fp, "  } else { /* RGB */ \\\n");

  fprintf (fp, "    while (__ip < __il) { %s __l = *(__rd++); \\\n", s_uint);
  fprintf (fp, "      if (__l & 128) { __l = __l - 128; \\\n");
  fprintf (fp, "        do { memcpy (__ip, __rd, 3); __ip += 3; } while (--__l); __rd += 3; \\\n");
  fprintf (fp, "      } else { __l *= 3; memcpy (__ip, __rd, __l); \\\n");
  fprintf (fp, "               __ip += __l; __rd += __l; } } \\\n");

  fprintf (fp, "  } } while (0)\n");
}

static inline guint
save_uchar (FILE   *fp,
	    guint   c,
	    guint8  d,
	    Config *config)
{
  static guint8 pad = 0;

  if (c > 74)
    {
      if (!config->use_macros)
	{
	  fprintf (fp, "\"\n  \"");
	  c = 3;
	}
      else
	{
	  fprintf (fp, "\"\n \"");
	  c = 2;
	}
    }
  if (d < 33 || d > 126)
    {
      fprintf (fp, "\\%o", d);
      c += 1 + 1 + (d > 7) + (d > 63);
      pad = d < 64;

      return c;
    }
  
  if (d == '\\')
    {
      fputs ("\\\\", fp);
      c += 2;
    }
  else if (d == '"')
    {
      fputs ("\\\"", fp);
      c += 2;
    }
  else if (pad && d >= '0' && d <= '9')
    {
      fputs ("\"\"", fp);
      fputc (d, fp);
      c += 3;
    }
  else
    {
      fputc (d, fp);
      c += 1;
    }
  pad = 0;
  
  return c;
}

static gint
save_image (Config *config,
	    gint32  image_ID,
	    gint32  drawable_ID)
{
  GDrawable *drawable = gimp_drawable_get (drawable_ID);
  GDrawableType drawable_type = gimp_drawable_type (drawable_ID);
  GPixelRgn pixel_rgn;
  gchar *s_uint_8, *s_uint_32, *s_uint, *s_char, *s_null;
  FILE *fp;
  guint c;
  gchar *macro_name;
  guint8 *img_buffer, *img_buffer_end;
  
  fp = fopen (config->file_name, "w");
  if (!fp)
    return FALSE;
  
  gimp_pixel_rgn_init (&pixel_rgn, drawable, 0, 0, drawable->width, drawable->height, FALSE, FALSE);

  if (1)
    {
      guint8 *data, *p;
      gint x, y, pad, n_bytes, bpp;

      bpp = config->alpha ? 4 : 3;
      n_bytes = drawable->width * drawable->height * bpp;
      pad = drawable->width * drawable->bpp;
      if (config->use_rle)
	pad = MAX (pad, 130 + n_bytes / 127);
      data = g_new (guint8, pad + n_bytes);
      p = data + pad;
      for (y = 0; y < drawable->height; y++)
	{
	  gimp_pixel_rgn_get_row (&pixel_rgn, data, 0, y, drawable->width);
	  if (config->alpha)
	    for (x = 0; x < drawable->width; x++)
	      {
		guint8 *d = data + x * drawable->bpp;
		gdouble alpha = drawable_type == RGBA_IMAGE ? d[3] : 0xff;
		
		alpha *= config->opacity / 100.0;
		*(p++) = d[0];
		*(p++) = d[1];
		*(p++) = d[2];
		*(p++) = alpha + 0.5;
	      }
	  else
	    for (x = 0; x < drawable->width; x++)
	      {
		guint8 *d = data + x * drawable->bpp;
		gdouble alpha = drawable_type == RGBA_IMAGE ? d[3] : 0xff;
		
		alpha *= config->opacity / 25600.0;
		*(p++) = 0.5 + alpha * (gdouble) d[0];
		*(p++) = 0.5 + alpha * (gdouble) d[1];
		*(p++) = 0.5 + alpha * (gdouble) d[2];
	      }
	}
      img_buffer = data + pad;
      if (config->use_rle)
	{
	  img_buffer_end = rl_encode_rgbx (data, img_buffer, img_buffer + n_bytes, bpp);
	  img_buffer = data;
	}
      else
	img_buffer_end = img_buffer + n_bytes;
    }

  if (!config->use_macros && config->glib_types)
    {
      s_uint_8 =  "guint8 ";
      s_uint_32 = "guint32";
      s_uint  =   "guint  ";
      s_char =    "gchar  ";
      s_null =    "NULL";
    }
  else if (!config->use_macros)
    {
      s_uint_8 =  "unsigned char";
      s_uint_32 = "unsigned int ";
      s_uint =    "unsigned int ";
      s_char =    "char         ";
      s_null =    "(char*) 0";
    }
  else if (config->use_macros && config->glib_types)
    {
      s_uint_8 =  "guint8";
      s_uint_32 = "guint32";
      s_uint  =   "guint";
      s_char =    "gchar";
      s_null =    "NULL";
    }
  else /* config->use_macros && !config->glib_types */
    {
      s_uint_8 =  "unsigned char";
      s_uint_32 = "unsigned int";
      s_uint =    "unsigned int";
      s_char =    "char";
      s_null =    "(char*) 0";
    }
  macro_name = g_strdup (config->prefixed_name);
  g_strup (macro_name);
  
  fprintf (fp, "/* GIMP %s C-Source image dump %s(%s) */\n\n",
	   config->alpha ? "RGBA" : "RGB",
	   config->use_rle ? "1-byte-run-length-encoded " : "",
	   g_basename (config->file_name));

  if (config->use_rle && !config->use_macros)
    save_rle_decoder (fp,
		      macro_name,
		      config->glib_types ? "guint" : "unsigned int",
		      config->glib_types ? "guint8" : "unsigned char",
		      config->alpha ? 4 : 3);

  if (!config->use_macros)
    {
      fprintf (fp, "static const struct {\n");
      fprintf (fp, "  %s\t width;\n", s_uint);
      fprintf (fp, "  %s\t height;\n", s_uint);
      fprintf (fp, "  %s\t bytes_per_pixel; /* 3:RGB, 4:RGBA */ \n", s_uint);
      if (config->use_comment)
	fprintf (fp, "  %s\t*comment;\n", s_char);
      fprintf (fp, "  %s\t %spixel_data[",
	       s_uint_8,
	       config->use_rle ? "rle_" : "");
      if (config->use_rle)
	fprintf (fp, "%u];\n", img_buffer_end - img_buffer);
      else
	fprintf (fp, "%u * %u * %u];\n",
		 drawable->width,
		 drawable->height,
		 config->alpha ? 4 : 3);
      fprintf (fp, "} %s = {\n", config->prefixed_name);
      fprintf (fp, "  %u, %u, %u,\n",
	       drawable->width,
	       drawable->height,
	       config->alpha ? 4 : 3);
    }
  else /* use macros */
    {
      fprintf (fp, "#define %s_WIDTH (%u)\n",
	       macro_name, drawable->width);
      fprintf (fp, "#define %s_HEIGHT (%u)\n",
	       macro_name, drawable->height);
      fprintf (fp, "#define %s_BYTES_PER_PIXEL (%u) /* 3:RGB, 4:RGBA */\n",
	       macro_name, config->alpha ? 4 : 3);
    }
  if (config->use_comment && !config->comment)
    {
      if (!config->use_macros)
	fprintf (fp, "  %s,\n", s_null);
      else /* use macros */
	fprintf (fp, "#define %s_COMMENT (%s)\n", macro_name, s_null);
    }
  else if (config->use_comment)
    {
      gchar *p = config->comment - 1;
      
      if (config->use_macros)
	fprintf (fp, "#define %s_COMMENT \\\n", macro_name);
      fprintf (fp, "  \"");
      while (*(++p))
	if (*p == '\\')
	  fprintf (fp, "\\\\");
	else if (*p == '"')
	  fprintf (fp, "\\\"");
	else if (*p == '\n' && p[1])
	  fprintf (fp, "\\n\"%s\n  \"",
		   config->use_macros ? " \\" : "");
	else if (*p == '\n')
	  fprintf (fp, "\\n");
	else if (*p == '\r')
	  fprintf (fp, "\\r");
	else if (*p == '\b')
	  fprintf (fp, "\\b");
	else if (*p == '\f')
	  fprintf (fp, "\\f");
	else if (*p >= 32 && *p <= 126)
	  fprintf (fp, "%c", *p);
	else
	  fprintf (fp, "\\%03o", *p);
      if (!config->use_macros)
	fprintf (fp, "\",\n");
      else /* use macros */
	fprintf (fp, "\"\n");
    }
  if (config->use_macros)
    {
      fprintf (fp, "#define %s_%sPIXEL_DATA ((%s*) %s_%spixel_data)\n",
	       macro_name,
	       config->use_rle ? "RLE_" : "",
	       s_uint_8,
	       macro_name,
	       config->use_rle ? "rle_" : "");
      if (config->use_rle)
	save_rle_decoder (fp,
			  macro_name,
			  s_uint,
			  s_uint_8,
			  config->alpha ? 4 : 3);
      fprintf (fp, "static const %s %s_%spixel_data[",
	       s_uint_8,
	       macro_name,
	       config->use_rle ? "rle_" : "");
      if (config->use_rle)
	fprintf (fp, "%u] =\n", img_buffer_end - img_buffer);
      else
	fprintf (fp, "%u * %u * %u] =\n",
		 drawable->width,
		 drawable->height,
		 config->alpha ? 4 : 3);
      fprintf (fp, "(\"");
      c = 2;
    }
  else
    {
      fprintf (fp, "  \"");
      c = 3;
    }
  switch (drawable_type)
    {
    case RGB_IMAGE:
    case RGBA_IMAGE:
      do
	c = save_uchar (fp, c, *(img_buffer++), config);
      while (img_buffer < img_buffer_end);
      break;
    default:
      g_warning ("unhandled drawable type (%d)", drawable_type);
      return FALSE;
    }
  if (!config->use_macros)
    fprintf (fp, "\",\n};\n\n");
  else /* use macros */
    fprintf (fp, "\");\n\n");
  
  fclose (fp);
  
  gimp_drawable_detach (drawable);
  
  return TRUE;
}

static void
cb_set_true (gboolean *bool_p)
{
  *bool_p = TRUE;
}

static gboolean
run_save_dialog	(Config *config)
{
  GtkWidget *dialog, *vbox, *hbox, *button;
  GtkWidget *prefixed_name, *centry, *alpha, *use_macros, *use_rle, *gtype, *use_comment, *any;
  GtkObject *opacity;
  gboolean do_save = FALSE;
  
  gtk_init (NULL, NULL);
  g_set_prgname ("Save");
  gtk_rc_parse (gimp_gtkrc ());
  
  dialog = gimp_dialog_new ("Save as C-Source", "csource",
			    gimp_plugin_help_func, "filters/csource.html",
			    GTK_WIN_POS_MOUSE,
			    FALSE, TRUE, FALSE,

			    "OK", cb_set_true,
			    NULL, &do_save, &button, TRUE, FALSE,
			    "Cancel", gtk_widget_destroy,
			    NULL, 1, NULL, FALSE, TRUE,

			    NULL);

  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      GTK_SIGNAL_FUNC (gtk_main_quit),
		      NULL);

  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (gtk_widget_destroy),
			     GTK_OBJECT (dialog));

  vbox = GTK_DIALOG (dialog)->vbox;
  gtk_widget_set (vbox,
		  "border_width", 5,
		  "spacing", 5,
		  NULL);

  /* Prefixed Name
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  gtk_widget_new (GTK_TYPE_LABEL,
		  "label", "Prefixed Name: ",
		  "xalign", 0.0,
		  "visible", TRUE,
		  "parent", hbox,
		  NULL);
  prefixed_name = gtk_widget_new (GTK_TYPE_ENTRY,
				  "visible", TRUE,
				  "parent", hbox,
				  NULL);
  gtk_entry_set_text (GTK_ENTRY (prefixed_name),
		      config->prefixed_name ? config->prefixed_name : "");
  gtk_widget_ref (prefixed_name);
  
  /* Comment Entry
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  gtk_widget_new (GTK_TYPE_LABEL,
		  "label", "Comment: ",
		  "xalign", 0.0,
		  "visible", TRUE,
		  "parent", hbox,
		  NULL);
  centry = gtk_widget_new (GTK_TYPE_ENTRY,
			   "visible", TRUE,
			   "parent", hbox,
			   NULL);
  gtk_entry_set_text (GTK_ENTRY (centry),
		      config->comment ? config->comment : "");
  gtk_widget_ref (centry);

  /* Use Comment
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  use_comment = gtk_widget_new (GTK_TYPE_CHECK_BUTTON,
				"label", "Save comment to file?",
				"visible", TRUE,
				"parent", hbox,
				NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_comment), config->use_comment);
  gtk_widget_ref (use_comment);

  /* GLib types
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  gtype = gtk_widget_new (GTK_TYPE_CHECK_BUTTON,
			  "label", "Use GLib types (guint8*)",
			  "visible", TRUE,
			  "parent", hbox,
			  NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gtype), config->glib_types);
  gtk_widget_ref (gtype);

  /* Use Macros
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  use_macros = gtk_widget_new (GTK_TYPE_CHECK_BUTTON,
			       "label", "Use macros instead of struct",
			       "visible", TRUE,
			       "parent", hbox,
			       NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_macros), config->use_macros);
  gtk_widget_ref (use_macros);

  /* Use RLE
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  use_rle = gtk_widget_new (GTK_TYPE_CHECK_BUTTON,
			    "label", "Use 1 Byte Run-Length-Encoding",
			    "visible", TRUE,
			    "parent", hbox,
			    NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (use_rle), config->use_rle);
  gtk_widget_ref (use_rle);

  /* Alpha
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  alpha = gtk_widget_new (GTK_TYPE_CHECK_BUTTON,
			  "label", "Save Alpha channel (RGBA/RGB)",
			  "visible", TRUE,
			  "parent", hbox,
			  NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (alpha), config->alpha);
  gtk_widget_ref (alpha);
  
  /* Max Alpha Value
   */
  hbox = gtk_widget_new (GTK_TYPE_HBOX,
			 "visible", TRUE,
			 "parent", vbox,
			 NULL);
  gtk_widget_new (GTK_TYPE_LABEL,
		  "label", "Opacity: ",
		  "xalign", 0.0,
		  "visible", TRUE,
		  "parent", hbox,
		  NULL);
  opacity = gtk_adjustment_new (config->opacity,
				0,
				100,
				0.1,
				5.0,
				0);
  any = gtk_spin_button_new (GTK_ADJUSTMENT (opacity), 0, 1);
  gtk_widget_set (any,
		  "visible", TRUE,
		  "parent", hbox,
		  NULL);
  gtk_object_ref (opacity);
  
  gtk_widget_show (dialog);
  
  gtk_main ();
  
  config->prefixed_name = g_strdup (gtk_entry_get_text (GTK_ENTRY (prefixed_name)));
  config->use_macros = GTK_TOGGLE_BUTTON (use_macros)->active;
  config->use_rle = GTK_TOGGLE_BUTTON (use_rle)->active;
  config->alpha = GTK_TOGGLE_BUTTON (alpha)->active;
  config->glib_types = GTK_TOGGLE_BUTTON (gtype)->active;
  config->comment = g_strdup (gtk_entry_get_text (GTK_ENTRY (centry)));
  config->use_comment = GTK_TOGGLE_BUTTON (use_comment)->active;
  config->opacity = GTK_ADJUSTMENT (opacity)->value;
  
  gtk_widget_unref (gtype);
  gtk_widget_unref (centry);
  gtk_widget_unref (alpha);
  gtk_widget_unref (use_macros);
  gtk_widget_unref (use_rle);
  gtk_widget_unref (use_comment);
  gtk_widget_unref (prefixed_name);
  gtk_object_unref (opacity);

  if (!config->prefixed_name || !config->prefixed_name[0])
    config->prefixed_name = "tmp";
  if (config->comment && !config->comment[0])
    config->comment = NULL;

  return do_save;
}
