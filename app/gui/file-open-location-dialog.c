/* The GIMP -- an image manipulation program
 * Copyright (C) 1995, 1996, 1997 Spencer Kimball and Peter Mattis
 * Copyright (C) 1997 Josh MacDonald
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

#include <gtk/gtk.h>

#include "libgimpwidgets/gimpwidgets.h"

#include "gui-types.h"

#include "core/gimp.h"
#include "core/gimpprogress.h"

#include "file/file-open.h"
#include "file/file-utils.h"

#include "widgets/gimpcontainerentry.h"
#include "widgets/gimphelp-ids.h"
#include "widgets/gimpprogressbox.h"
#include "widgets/gimpwidgets-utils.h"

#include "file-open-location-dialog.h"

#include "gimp-intl.h"


static void      file_open_location_response   (GtkDialog          *dialog,
                                                gint                response_id,
                                                Gimp               *gimp);

static gboolean  file_open_location_completion (GtkEntryCompletion *completion,
                                                const gchar        *key,
                                                GtkTreeIter        *iter,
                                                gpointer            data);


/*  public functions  */

void
file_open_location_dialog_show (Gimp      *gimp,
                                GtkWidget *parent)
{
  GtkWidget          *dialog;
  GtkWidget          *hbox;
  GtkWidget          *vbox;
  GtkWidget          *image;
  GtkWidget          *label;
  GtkWidget          *entry;
  GtkEntryCompletion *completion;

  g_return_if_fail (GIMP_IS_GIMP (gimp));
  g_return_if_fail (parent == NULL || GTK_IS_WIDGET (parent));

  dialog = gimp_dialog_new (_("Open Location"),
                            "gimp-file-open-location",
                            parent, 0,
                            gimp_standard_help_func,
                            GIMP_HELP_FILE_OPEN_LOCATION,

                            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                            GTK_STOCK_OPEN,   GTK_RESPONSE_OK,

                            NULL);

  g_signal_connect (dialog, "response",
                    G_CALLBACK (file_open_location_response),
                    gimp);

  hbox = gtk_hbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 12);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
                      hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  image = gtk_image_new_from_stock (GIMP_STOCK_WEB, GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start (GTK_BOX (vbox), image, FALSE, FALSE, 0);
  gtk_widget_show (image);

  vbox = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
  gtk_widget_show (vbox);

  label = gtk_label_new (_("Enter location (URI):"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  entry = gimp_container_entry_new (gimp->documents, NULL,
                                    GIMP_PREVIEW_SIZE_SMALL, 1);

  completion = gtk_entry_get_completion (GTK_ENTRY (entry));
  gtk_entry_completion_set_match_func (completion,
                                       file_open_location_completion,
                                       NULL, NULL);

  gtk_widget_set_size_request (entry, 300, -1);
  gtk_box_pack_start (GTK_BOX (vbox), entry, FALSE, FALSE, 0);
  gtk_widget_show (entry);

  g_object_set_data (G_OBJECT (dialog), "location-entry", entry);

  gtk_widget_show (dialog);
}

static void
file_open_location_response (GtkDialog *dialog,
                             gint       response_id,
                             Gimp      *gimp)
{
  GtkWidget   *entry;
  GtkWidget   *box;
  const gchar *text = NULL;

  if (response_id != GTK_RESPONSE_OK)
    {
      box = g_object_get_data (G_OBJECT (dialog), "progress-box");

      if (box && GIMP_PROGRESS_BOX (box)->active)
        gimp_progress_cancel (GIMP_PROGRESS (box));
      else
        gtk_widget_destroy (GTK_WIDGET (dialog));

      return;
    }

  gimp_dialog_set_sensitive (dialog, FALSE);

  entry = g_object_get_data (G_OBJECT (dialog), "location-entry");

  text = gtk_entry_get_text (GTK_ENTRY (entry));

  if (text && strlen (text))
    {
      GimpImage         *image;
      gchar             *uri;
      gchar             *filename;
      GError            *error = NULL;
      GimpPDBStatusType  status;

      filename = g_filename_from_uri (text, NULL, NULL);

      if (filename)
        {
          uri = g_filename_to_uri (filename, NULL, NULL);
          g_free (filename);
        }
      else
        {
          uri = file_utils_filename_to_uri (gimp->load_procs, text, NULL);
        }

      box = gimp_progress_box_new ();
      gtk_container_set_border_width (GTK_CONTAINER (box), 12);
      gtk_box_pack_end (GTK_BOX (dialog->vbox), box, FALSE, FALSE, 0);
      gtk_widget_show (box);

      g_object_set_data (G_OBJECT (dialog), "progress-box", box);

      image = file_open_with_proc_and_display (gimp,
                                               gimp_get_user_context (gimp),
                                               GIMP_PROGRESS (box),
                                               uri, text, NULL,
                                               &status, &error);

      if (image == NULL && status != GIMP_PDB_CANCEL)
        {
          gchar *filename = file_utils_uri_to_utf8_filename (uri);

          g_message (_("Opening '%s' failed:\n\n%s"),
                     filename, error->message);
          g_clear_error (&error);

          g_free (filename);
        }

      g_free (uri);
    }

  gtk_widget_destroy (GTK_WIDGET (dialog));
}

static gboolean
file_open_location_completion (GtkEntryCompletion *completion,
                               const gchar        *key,
                               GtkTreeIter        *iter,
                               gpointer            data)
{
  GtkTreeModel *model = gtk_entry_completion_get_model (completion);
  gchar        *name;
  gchar        *normalized;
  gchar        *case_normalized;
  gboolean      match;

  gtk_tree_model_get (model, iter,
                      1, &name,
                      -1);

  if (! name)
    return FALSE;

  normalized = g_utf8_normalize (name, -1, G_NORMALIZE_ALL);
  case_normalized = g_utf8_casefold (normalized, -1);

  match = (strncmp (key, case_normalized, strlen (key)) == 0);

  if (! match)
    {
      const gchar *colon = strchr (case_normalized, ':');

      if (colon && strlen (colon) > 2 && colon[1] == '/' && colon[2] == '/')
        match = (strncmp (key, colon + 3, strlen (key)) == 0);
    }

  g_free (normalized);
  g_free (case_normalized);
  g_free (name);

  return match;
}
