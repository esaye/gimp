#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

#include "gimpressionist.h"

#include "libgimp/stdplugins-intl.h"

#ifdef G_OS_WIN32
#include "libgimpbase/gimpwin32-io.h"
#endif

static GtkWidget    *presetnameentry = NULL;
static GtkWidget    *presetlist      = NULL;
static GtkWidget    *presetdesclabel = NULL;
static GtkListStore *store;

static void set_preset_description_text (const gchar *text)
{
  gtk_label_set_text (GTK_LABEL (presetdesclabel), text);
}

static char presetdesc[4096] = "";

static char *factory_defaults = "<Factory defaults>";

static void addfactorydefaults (void)
{
  GtkTreeIter iter;

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, 0, factory_defaults, -1);
}

static void presetsrefresh(void)
{
  gtk_list_store_clear (store);
  addfactorydefaults ();
  readdirintolist ("Presets", presetlist, NULL);
}

#define PRESETMAGIC "Preset"

static int loadoldpreset (const gchar *fname)
{
  FILE *f;
  int len;

  f = fopen(fname, "rb");
  if(!f) {
    fprintf(stderr, "Error opening file \"%s\" for reading!%c\n", fname, 7);
    return -1;
  }
  len = fread(&pcvals, 1, sizeof(pcvals), f);
  fclose(f);

  return 0;
}

static unsigned int hexval (char c)
{
  c = g_ascii_tolower (c);
  if((c >= 'a') && (c <= 'f')) return c - 'a' + 10;
  if((c >= '0') && (c <= '9')) return c - '0';
  return 0;
}

static char *parsergbstring (const gchar *s)
{
  static char col[3];
  col[0] = (hexval(s[0]) << 4) | hexval(s[1]);
  col[1] = (hexval(s[2]) << 4) | hexval(s[3]);
  col[2] = (hexval(s[4]) << 4) | hexval(s[5]);
  return col;
}

static void setorientvector (const gchar *str)
{
  const gchar *tmps = str;
  int n;

  n = atoi(tmps);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.orientvector[n].x = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.orientvector[n].y = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.orientvector[n].dir = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.orientvector[n].dx = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.orientvector[n].dy = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.orientvector[n].str = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.orientvector[n].type = atoi (++tmps);

}

static void setsizevector (const gchar *str)
{
  const gchar *tmps = str;
  int n;

  n = atoi(tmps);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.sizevector[n].x = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.sizevector[n].y = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.sizevector[n].siz = g_ascii_strtod (++tmps, NULL);

  if(!(tmps = strchr(tmps, ','))) return;
  pcvals.sizevector[n].str = g_ascii_strtod (++tmps, NULL);

}

static void parsedesc (const gchar *str, gchar *d, gssize d_len)
{
  gchar *dest = g_strcompress (str);

  g_strlcpy (d, dest, d_len);

  g_free (dest);
}

static void setval (const gchar *key, const gchar *val)
{
  if(!strcmp(key, "desc"))
    parsedesc(val, presetdesc, sizeof (presetdesc));
  else if(!strcmp(key, "orientnum"))
    pcvals.orientnum = atoi(val);
  else if(!strcmp(key, "orientfirst"))
    pcvals.orientfirst = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "orientlast"))
    pcvals.orientlast = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "orienttype"))
   pcvals.orienttype = atoi(val);

  else if(!strcmp(key, "sizenum"))
    pcvals.sizenum = atoi(val);
  else if(!strcmp(key, "sizefirst"))
    pcvals.sizefirst = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "sizelast"))
    pcvals.sizelast = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "sizetype"))
   pcvals.sizetype = atoi(val);

  else if(!strcmp(key, "brushrelief"))
    pcvals.brushrelief = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "brushscale")) {
    /* For compatibility */
    pcvals.sizenum = 1;
    pcvals.sizefirst = pcvals.sizelast = g_ascii_strtod (val, NULL);
  }
  else if(!strcmp(key, "brushdensity"))
    pcvals.brushdensity = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "brushgamma"))
    pcvals.brushgamma = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "brushaspect"))
    pcvals.brushaspect = g_ascii_strtod (val, NULL);

  else if(!strcmp(key, "generalbgtype"))
    pcvals.generalbgtype = atoi(val);
  else if(!strcmp(key, "generaldarkedge"))
    pcvals.generaldarkedge = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "generalpaintedges"))
    pcvals.generalpaintedges = atoi(val);
  else if(!strcmp(key, "generaltileable"))
    pcvals.generaltileable = atoi(val);
  else if(!strcmp(key, "generaldropshadow"))
    pcvals.generaldropshadow = atoi(val);
  else if(!strcmp(key, "generalshadowdarkness"))
    pcvals.generalshadowdarkness = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "generalshadowdepth"))
    pcvals.generalshadowdepth = atoi(val);
  else if(!strcmp(key, "generalshadowblur"))
    pcvals.generalshadowblur = atoi(val);
  else if(!strcmp(key, "devthresh"))
    pcvals.devthresh = g_ascii_strtod (val, NULL);

  else if(!strcmp(key, "paperrelief"))
    pcvals.paperrelief = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "paperscale"))
    pcvals.paperscale = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "paperinvert"))
    pcvals.paperinvert = atoi(val);
  else if(!strcmp(key, "paperoverlay"))
    pcvals.paperoverlay = atoi(val);

  else if(!strcmp(key, "placetype"))
    pcvals.placetype = atoi(val);
  else if(!strcmp(key, "placecenter"))
    pcvals.placecenter = atoi(val);

  else if(!strcmp(key, "selectedbrush"))
    g_strlcpy (pcvals.selectedbrush, val, sizeof (pcvals.selectedbrush));
  else if(!strcmp(key, "selectedpaper"))
    g_strlcpy (pcvals.selectedpaper, val, sizeof (pcvals.selectedpaper));

  else if(!strcmp(key, "color")){
    char *c = parsergbstring(val);
    gimp_rgba_set_uchar(&pcvals.color, c[0], c[1], c[2], 255);
  }

  else if(!strcmp(key, "numorientvector"))
    pcvals.numorientvector = atoi(val);
  else if(!strcmp(key, "orientvector"))
    setorientvector(val);
  else if(!strcmp(key, "orientangoff"))
   pcvals.orientangoff = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "orientstrexp"))
   pcvals.orientstrexp = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "orientvoronoi"))
   pcvals.orientvoronoi = atoi(val);

  else if(!strcmp(key, "numsizevector"))
    pcvals.numsizevector = atoi(val);
  else if(!strcmp(key, "sizevector"))
    setsizevector(val);
  else if(!strcmp(key, "sizestrexp"))
   pcvals.sizestrexp = g_ascii_strtod (val, NULL);
  else if(!strcmp(key, "sizevoronoi"))
   pcvals.sizevoronoi = atoi(val);

  else if(!strcmp(key, "colortype"))
    pcvals.colortype = atoi(val);
  else if(!strcmp(key, "colornoise"))
    pcvals.colornoise = g_ascii_strtod (val, NULL);
}

static int loadpreset(const gchar *fn)
{
  char line[1024] = "";
  FILE *f;

  f = fopen(fn, "rt");
  if(!f) {
    fprintf(stderr, "Error opening file \"%s\" for reading!\n", fn);
    return -1;
  }
  fgets(line,10,f);
  if(strncmp(line,PRESETMAGIC,4)) {
    fclose(f);
    return loadoldpreset(fn);
  }
  restore_default_values();
  while(!feof(f)) {
    char *tmps;
    if(!fgets(line,1024,f))
      break;
    remove_trailing_whitespace(line);
    tmps = strchr(line, '=');
    if(!tmps)
      continue;
    *tmps = '\0';
    tmps++;
    setval(line, tmps);
  }
  fclose(f);
  return 0;
}

int select_preset(const gchar *preset)
{
    int ret = SELECT_PRESET_OK;
    /* I'm copying this behavior as is. As it seems applying the
     * factory_defaults preset does nothing, which I'm not sure
     * if that was what the author intended.
     *              -- Shlomi Fish
     */
    if (strcmp (preset, factory_defaults))
      {
        gchar *rel = g_build_filename ("Presets", preset, NULL);
        gchar *abs = findfile (rel);

        g_free (rel);

        if (abs)
          {
            if (loadpreset (abs))
              {
                ret = SELECT_PRESET_LOAD_FAILED;
              }
            g_free (abs);
          }
        else
          {
            ret = SELECT_PRESET_FILE_NOT_FOUND;
          }
      }
    if (ret == SELECT_PRESET_OK)
      {
        /* This is so the colorbrushes param (that is not stored in the
         * preset will be set correctly upon the preset loading.
         * */
        set_colorbrushes (pcvals.selectedbrush);
      }

    return ret;
}

static void applypreset(GtkWidget *w, GtkTreeSelection *selection)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gchar *preset;

      gtk_tree_model_get (model, &iter, 0, &preset, -1);

      select_preset(preset);

      restorevals ();

      g_free (preset);
    }
}

static void deletepreset(GtkWidget *w, GtkTreeSelection *selection)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gchar *preset;

      gtk_tree_model_get (model, &iter, 0, &preset, -1);

      if (preset)
        {
          gchar *rel = g_build_filename ("Presets", preset, NULL);
          gchar *abs = findfile (rel);

          g_free (rel);

          if (abs)
            {
              unlink (abs);
              g_free (abs);
            }

          presetsrefresh ();

          g_free (preset);
        }
    }
}

static void savepreset(void);

static void presetdesccallback(GtkTextBuffer *buffer, gpointer data)
{
  char *str;
  GtkTextIter start, end;

  gtk_text_buffer_get_bounds (buffer, &start, &end);
  str = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  g_strlcpy (presetdesc, str, sizeof (presetdesc));
  g_free (str);
}

static void
savepresetresponse (GtkWidget *widget,
                    gint       response_id,
                    gpointer   data)
{
  gtk_widget_destroy (widget);

  if (response_id == GTK_RESPONSE_OK)
    savepreset ();
}

static void
create_savepreset (void)
{
  static GtkWidget *window = NULL;
  GtkWidget *box, *label;
  GtkWidget *swin, *text;
  GtkTextBuffer *buffer;

  if (window)
    {
      gtk_window_present (GTK_WINDOW (window));
      return;
    }

  window =
    gimp_dialog_new (_("Save Current"), "gimpressionist",
                     NULL, 0,
                     gimp_standard_help_func, HELP_ID,

                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                     GTK_STOCK_OK,     GTK_RESPONSE_OK,

                     NULL);

  g_signal_connect (window, "response",
                    G_CALLBACK (savepresetresponse),
                    NULL);
  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_widget_destroyed),
                    &window);

  box = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (box), 12);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), box);
  gtk_widget_show (box);

  label = gtk_label_new( _("Description:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  swin = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(swin),
                                       GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER(box), swin);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(swin),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (swin);

  buffer = gtk_text_buffer_new (NULL);
  g_signal_connect (buffer, "changed",
                    G_CALLBACK (presetdesccallback), NULL);
  gtk_text_buffer_set_text (buffer, presetdesc, -1);

  text = gtk_text_view_new_with_buffer (buffer);
  gtk_widget_set_size_request (text, -1, 192);
  gtk_container_add (GTK_CONTAINER(swin), text);
  gtk_widget_show (text);

  gtk_widget_show (window);
}

static void savepreset(void)
{
  const gchar *l;
  gchar *fname, *presets_dir_path;
  FILE  *f;
  GList *thispath;
  gchar  buf[G_ASCII_DTOSTR_BUF_SIZE];
  gchar  vbuf[6][G_ASCII_DTOSTR_BUF_SIZE];
  guchar color[3];
  gint   i;
  gchar *desc_escaped;

  l = gtk_entry_get_text (GTK_ENTRY (presetnameentry));
  thispath = parsepath ();
  storevals ();

  if (!thispath)
    {
      g_printerr ("Internal error: (savepreset) thispath == NULL");
      return;
    }

  /* Create the ~/.gimp-$VER/gimpressionist/Presets directory if
   * it doesn't already exists.
   * */
  presets_dir_path = g_build_filename ((char *)thispath->data, "Presets", NULL);

  if (!g_file_test (presets_dir_path, G_FILE_TEST_IS_DIR))
    {
      if (mkdir (presets_dir_path,
                 S_IRUSR | S_IWUSR | S_IXUSR |
                 S_IRGRP | S_IXGRP |
                 S_IROTH | S_IXOTH) == -1)
        {
          g_printerr ("Error creating folder \"%s\"!\n", presets_dir_path);
          g_free (presets_dir_path);
          return;
        }
    }

  fname = g_build_filename (presets_dir_path, l, NULL);
  g_free (presets_dir_path);

  f = fopen (fname, "wt");
  if (!f)
    {
      g_printerr ("Error opening file \"%s\" for writing!%c\n", fname, 7);
      g_free (fname);
      return;
    }

  fprintf(f, "%s\n", PRESETMAGIC);
  desc_escaped = g_strescape (presetdesc, NULL);
  fprintf(f, "desc=%s\n", desc_escaped);
  g_free (desc_escaped);
  fprintf(f, "orientnum=%d\n", pcvals.orientnum);
  fprintf(f, "orientfirst=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientfirst));
  fprintf(f, "orientlast=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientlast));
  fprintf(f, "orienttype=%d\n", pcvals.orienttype);

  fprintf(f, "sizenum=%d\n", pcvals.sizenum);
  fprintf(f, "sizefirst=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.sizefirst));
  fprintf(f, "sizelast=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.sizelast));
  fprintf(f, "sizetype=%d\n", pcvals.sizetype);

  fprintf(f, "brushrelief=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.brushrelief));
  fprintf(f, "brushdensity=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.brushdensity));
  fprintf(f, "brushgamma=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.brushgamma));
  fprintf(f, "brushaspect=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.brushaspect));

  fprintf(f, "generalbgtype=%d\n", pcvals.generalbgtype);
  fprintf(f, "generaldarkedge=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.generaldarkedge));
  fprintf(f, "generalpaintedges=%d\n", pcvals.generalpaintedges);
  fprintf(f, "generaltileable=%d\n", pcvals.generaltileable);
  fprintf(f, "generaldropshadow=%d\n", pcvals.generaldropshadow);
  fprintf(f, "generalshadowdarkness=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.generalshadowdarkness));
  fprintf(f, "generalshadowdepth=%d\n", pcvals.generalshadowdepth);
  fprintf(f, "generalshadowblur=%d\n", pcvals.generalshadowblur);
  fprintf(f, "devthresh=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.devthresh));

  fprintf(f, "paperrelief=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.paperrelief));
  fprintf(f, "paperscale=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.paperscale));
  fprintf(f, "paperinvert=%d\n", pcvals.paperinvert);
  fprintf(f, "paperoverlay=%d\n", pcvals.paperoverlay);

  fprintf(f, "selectedbrush=%s\n", pcvals.selectedbrush);
  fprintf(f, "selectedpaper=%s\n", pcvals.selectedpaper);

  gimp_rgb_get_uchar(&pcvals.color, &color[0], &color[1], &color[2]);
  fprintf(f, "color=%02x%02x%02x\n", color[0], color[1], color[2]);

  fprintf(f, "placetype=%d\n", pcvals.placetype);
  fprintf(f, "placecenter=%d\n", pcvals.placecenter);

  fprintf(f, "numorientvector=%d\n", pcvals.numorientvector);
  for(i = 0; i < pcvals.numorientvector; i++)
    {
      g_ascii_formatd (vbuf[0], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientvector[i].x);
      g_ascii_formatd (vbuf[1], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientvector[i].y);
      g_ascii_formatd (vbuf[2], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientvector[i].dir);
      g_ascii_formatd (vbuf[3], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientvector[i].dx);
      g_ascii_formatd (vbuf[4], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientvector[i].dy);
      g_ascii_formatd (vbuf[5], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientvector[i].str);

      fprintf (f, "orientvector=%d,%s,%s,%s,%s,%s,%s,%d\n", i,
               vbuf[0], vbuf[1], vbuf[2], vbuf[3], vbuf[4], vbuf[5],
               pcvals.orientvector[i].type);
    }

  fprintf(f, "orientangoff=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientangoff));
  fprintf(f, "orientstrexp=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.orientstrexp));
  fprintf(f, "orientvoronoi=%d\n", pcvals.orientvoronoi);

  fprintf(f, "numsizevector=%d\n", pcvals.numsizevector);
  for (i = 0; i < pcvals.numsizevector; i++)
    {
      g_ascii_formatd (vbuf[0], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.sizevector[i].x);
      g_ascii_formatd (vbuf[1], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.sizevector[i].y);
      g_ascii_formatd (vbuf[2], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.sizevector[i].siz);
      g_ascii_formatd (vbuf[3], G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.sizevector[i].str);
      fprintf (f, "sizevector=%d,%s,%s,%s,%s\n", i,
               vbuf[0], vbuf[1], vbuf[2], vbuf[3]);
    }
  fprintf(f, "sizestrexp=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.sizestrexp));
  fprintf(f, "sizevoronoi=%d\n", pcvals.sizevoronoi);

  fprintf(f, "colortype=%d\n", pcvals.colortype);
  fprintf(f, "colornoise=%s\n",
          g_ascii_formatd (buf, G_ASCII_DTOSTR_BUF_SIZE, "%f", pcvals.colornoise));

  fclose (f);
  presetsrefresh ();
  reselect (presetlist, fname);

  g_free (fname);
}

static void readdesc(const char *fn)
{
  char *rel_fname;
  char *fname;
  FILE *f;

  rel_fname = g_build_filename ("Presets", fn, NULL);
  fname   = findfile (rel_fname);
  g_free (rel_fname);

  if (!fname)
    {
      set_preset_description_text("");
      return;
    }

  f = fopen(fname, "rt");
  g_free(fname);
  if (f)
    {
      char line[4096];
      char tmplabel[4096];
      while(!feof(f))
        {
          fgets(line, 4095, f);
          if (!strncmp (line, "desc=", 5))
            {
              parsedesc (line + 5, tmplabel, sizeof (tmplabel));
              set_preset_description_text (tmplabel);
              fclose (f);
              return;
            }
        }
      fclose (f);
    }

  set_preset_description_text ("");
}

static void selectpreset(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  if (gtk_tree_selection_get_selected (selection, &model, &iter))
    {
      gchar *preset;

      gtk_tree_model_get (model, &iter, 0, &preset, -1);
      if(strcmp(preset, factory_defaults))
        gtk_entry_set_text (GTK_ENTRY(presetnameentry), preset);
      readdesc (preset);
      g_free (preset);
    }
}

void create_presetpage(GtkNotebook *notebook)
{
  GtkWidget *vbox, *hbox, *box1, *box2, *thispage;
  GtkWidget *view;
  GtkWidget *tmpw;
  GtkWidget *label;
  GtkTreeSelection *selection;

  label = gtk_label_new_with_mnemonic (_("_Presets"));

  thispage = gtk_vbox_new (FALSE, 12);
  gtk_container_set_border_width (GTK_CONTAINER (thispage), 12);
  gtk_widget_show (thispage);

  box1 = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start(GTK_BOX (thispage), box1, FALSE, FALSE, 0);
  gtk_widget_show (box1);

  presetnameentry = tmpw = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (box1), tmpw, FALSE, FALSE, 0);
  gtk_widget_set_size_request(tmpw, 150, -1);
  gtk_widget_show(tmpw);

  presetsavebutton = tmpw = gtk_button_new_with_label( _("Save current..."));
  gtk_box_pack_start(GTK_BOX(box1), tmpw, FALSE, FALSE, 0);
  gtk_widget_show (tmpw);
  g_signal_connect (tmpw, "clicked", G_CALLBACK(create_savepreset), NULL);
  gimp_help_set_help_data
    (tmpw, _("Save the current settings to the specified file"), NULL);

  box1 = gtk_hbox_new (FALSE, 12);
  gtk_box_pack_start(GTK_BOX (thispage), box1, TRUE, TRUE, 0);
  gtk_widget_show (box1);

  presetlist = view = createonecolumnlist (box1, selectpreset);
  store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (view)));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
  addfactorydefaults ();

  vbox = gtk_vbox_new (FALSE, 12);
  gtk_box_pack_start (GTK_BOX (box1), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  box2 = gtk_vbox_new (FALSE, 6);
  gtk_box_pack_start(GTK_BOX (hbox), box2, FALSE, FALSE, 0);
  gtk_widget_show (box2);

  tmpw = gtk_button_new_from_stock (GTK_STOCK_APPLY);
  gtk_box_pack_start(GTK_BOX(box2), tmpw, FALSE, FALSE, 0);
  gtk_widget_show (tmpw);
  g_signal_connect (tmpw, "clicked", G_CALLBACK(applypreset), selection);
  gimp_help_set_help_data
    (tmpw, _("Reads the selected Preset into memory"), NULL);

  tmpw = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_box_pack_start(GTK_BOX(box2), tmpw, FALSE, FALSE,0);
  gtk_widget_show (tmpw);
  g_signal_connect (tmpw, "clicked", G_CALLBACK(deletepreset), selection);
  gimp_help_set_help_data (tmpw, _("Deletes the selected Preset"), NULL);

  tmpw = gtk_button_new_from_stock (GTK_STOCK_REFRESH);
  gtk_box_pack_start(GTK_BOX(box2), tmpw, FALSE, FALSE,0);
  gtk_widget_show (tmpw);
  g_signal_connect (tmpw, "clicked", G_CALLBACK(presetsrefresh), NULL);
  gimp_help_set_help_data (tmpw, _("Reread the folder of Presets"), NULL);

  presetdesclabel = tmpw = gtk_label_new (NULL);
  gtk_label_set_line_wrap (GTK_LABEL (tmpw), TRUE);
  /*
   * Make sure the label's width is reasonable and it won't stretch
   * the dialog more than its width.
   * */
  gtk_widget_set_size_request (tmpw, 200, -1);

  gtk_misc_set_alignment (GTK_MISC (tmpw), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX (vbox), tmpw, TRUE, TRUE, 0);
  gtk_widget_show(tmpw);

  readdirintolist("Presets", view, NULL);

  gtk_notebook_append_page_menu (notebook, thispage, label, NULL);
}
