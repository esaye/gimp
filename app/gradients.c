/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURIGHTE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "apptypes.h"

#include "datafiles.h"
#include "gimpcontext.h"
#include "gimpdatalist.h"
#include "gimpgradient.h"
#include "gimprc.h"
#include "gradients.h"
#include "gradient_select.h"
#include "temp_buf.h"

#include "libgimp/gimpintl.h"


/*  local function prototypes  */
static void   gradients_load_gradient (const gchar *filename,
				       gpointer     loader_data);


/*  global variables  */
GimpContainer *global_gradient_list = NULL;


/*  public functions  */

void
gradients_init (gint no_data)
{
  if (global_gradient_list)
    gradients_free ();
  else
    global_gradient_list =
      GIMP_CONTAINER (gimp_data_list_new (GIMP_TYPE_GRADIENT));

  if (gradient_path != NULL && !no_data)
    {
      gradient_select_freeze_all ();

      datafiles_read_directories (gradient_path, 0,
				  gradients_load_gradient, global_gradient_list);

      gradient_select_thaw_all ();
    }

  gimp_context_refresh_gradients ();
}

void
gradients_free (void)
{
  if (! global_gradient_list)
    return;

  gradient_select_freeze_all ();

  gimp_data_list_save_and_clear (GIMP_DATA_LIST (global_gradient_list),
				 gradient_path,
				 GIMP_GRADIENT_FILE_EXTENSION);

  gradient_select_thaw_all ();
}

GimpGradient *
gradients_get_standard_gradient (void)
{
  static GimpGradient *standard_gradient = NULL;

  if (! standard_gradient)
    {
      standard_gradient = gimp_gradient_new ("Standard");

      gtk_object_ref (GTK_OBJECT (standard_gradient));
      gtk_object_sink (GTK_OBJECT (standard_gradient));
    }

  return standard_gradient;
}


/*  private functions  */

static void
gradients_load_gradient (const gchar *filename,
			 gpointer     loader_data)
{
  GimpGradient *gradient = NULL;

  g_return_if_fail (filename != NULL);

  if (! datafiles_check_extension (filename, GIMP_GRADIENT_FILE_EXTENSION))
    {
      g_warning ("%s(): trying old gradient file format on file with "
		 "unknown extension: %s",
		 G_GNUC_FUNCTION, filename);
    }

  gradient = gimp_gradient_load (filename);

  if (! gradient)
    g_message (_("Warning: Failed to load gradient\n\"%s\""), filename);
  else
    gimp_container_add (GIMP_CONTAINER (loader_data), GIMP_OBJECT (gradient));
}
