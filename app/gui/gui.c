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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "libgimpbase/gimpbase.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "gui-types.h"

#include "config/gimpguiconfig.h"

#include "core/gimp.h"
#include "core/gimpcontainer.h"
#include "core/gimpcontext.h"
#include "core/gimpenvirontable.h"
#include "core/gimpimage.h"

#include "display/gimpdisplay.h"
#include "display/gimpdisplay-foreach.h"
#include "display/gimpdisplayshell.h"
#include "display/gimpdisplayshell-render.h"

#include "widgets/gimpdevices.h"
#include "widgets/gimpdialogfactory.h"
#include "widgets/gimpitemfactory.h"
#include "widgets/gimpwidgets-utils.h"

#include "device-status-dialog.h"
#include "dialogs.h"
#include "dialogs-commands.h"
#include "error-console-dialog.h"
#include "file-open-dialog.h"
#include "file-save-dialog.h"
#include "gui.h"
#include "menus.h"
#include "session.h"

#include "libgimp/gimpintl.h"

#include "pixmaps/wilber2.xpm"


/*  local function prototypes  */

static void         gui_main                        (Gimp        *gimp);
static void         gui_main_quit                   (Gimp        *gimp);
static void         gui_set_busy                    (Gimp        *gimp);
static void         gui_unset_busy                  (Gimp        *gimp);
static void         gui_message                     (Gimp        *gimp,
                                                     const gchar *message);
static GimpObject * gui_display_new                 (GimpImage   *gimage,
                                                     guint        scale);

static void         gui_themes_dir_foreach_func     (GimpDatafileData *file_data);
static gint         gui_rotate_the_shield_harmonics (GtkWidget   *widget,
                                                     GdkEvent    *eevent,
                                                     gpointer     data);

static gboolean     gui_exit_callback               (Gimp        *gimp,
                                                     gboolean     kill_it);
static gboolean     gui_exit_finish_callback        (Gimp        *gimp,
                                                     gboolean     kill_it);
static void         gui_really_quit_callback        (GtkWidget   *button,
                                                     gboolean     quit,
                                                     gpointer     data);
static void         gui_show_tooltips_notify        (GObject     *config,
                                                     GParamSpec  *param_spec,
                                                     Gimp        *gimp);
static void         gui_display_changed             (GimpContext *context,
                                                     GimpDisplay *display,
                                                     Gimp        *gimp);
static void         gui_image_disconnect            (GimpImage   *gimage,
                                                     Gimp        *gimp);


/*  private variables  */

static GQuark image_disconnect_handler_id = 0;

static GHashTable *themes_hash = NULL;


/*  public functions  */

void
gui_libs_init (gint    *argc,
	       gchar ***argv)
{
  g_return_if_fail (argc != NULL);
  g_return_if_fail (argv != NULL);

  gtk_init (argc, argv);

  gimp_widgets_init ();

  g_type_class_ref (GIMP_TYPE_COLOR_SELECT);
}

void
gui_environ_init (Gimp *gimp)
{
  gchar *name = NULL;

#if defined (GDK_WINDOWING_X11)
  name = "DISPLAY";
#elif defined (GDK_WINDOWING_DIRECTFB) || defined (GDK_WINDOWING_FB)
  name = "GDK_DISPLAY";
#endif

  /* TODO: Need to care about display migration with GTK+ 2.2 at some point */

  if (name)
    gimp_environ_table_add (gimp->environ_table,
                            name, gdk_get_display (),
			    NULL);
}

void
gui_themes_init (Gimp *gimp)
{
  GimpGuiConfig *config;
  const gchar   *theme_dir;
  gchar         *gtkrc;

  g_return_if_fail (GIMP_IS_GIMP (gimp));

  config = GIMP_GUI_CONFIG (gimp->config);

  themes_hash = g_hash_table_new_full (g_str_hash,
				       g_str_equal,
				       g_free,
				       g_free);

  if (config->theme_path)
    {
      gimp_datafiles_read_directories (config->theme_path,
				       G_FILE_TEST_IS_DIR,
				       gui_themes_dir_foreach_func,
				       gimp);
    }

  theme_dir = gui_themes_get_theme_dir (gimp);

  if (theme_dir)
    {
      gtkrc = g_build_filename (theme_dir, "gtkrc", NULL);
    }
  else
    {
      /*  get the hardcoded default theme gtkrc  */

      gtkrc = g_strdup (gimp_gtkrc ());
    }

  if (gimp->be_verbose)
    g_print (_("Parsing '%s'\n"), gtkrc);

  gtk_rc_parse (gtkrc);

  g_free (gtkrc);

  /*  parse the user gtkrc  */

  gtkrc = gimp_personal_rc_file ("gtkrc");

  if (gimp->be_verbose)
    g_print (_("Parsing '%s'\n"), gtkrc);

  gtk_rc_parse (gtkrc);

  g_free (gtkrc);

  /*  tooltips  */
  gimp_help_init ();

  if (! config->show_tool_tips)
    gimp_help_disable_tooltips ();

  g_signal_connect (G_OBJECT (gimp->config), "notify::show-tool-tips",
                    G_CALLBACK (gui_show_tooltips_notify),
                    gimp);

  gdk_rgb_set_min_colors (CLAMP (gimp->config->min_colors, 27, 256));
  gdk_rgb_set_install (gimp->config->install_cmap);

  gtk_widget_set_default_colormap (gdk_rgb_get_colormap ());
}

const gchar *
gui_themes_get_theme_dir (Gimp *gimp)
{
  GimpGuiConfig *config = GIMP_GUI_CONFIG (gimp->config);

  if (config->theme)
    return g_hash_table_lookup (themes_hash, config->theme);

  return g_hash_table_lookup (themes_hash, "Default");
}

void
gui_init (Gimp *gimp)
{
  GimpDisplayConfig *display_config;
  GimpGuiConfig     *gui_config;

  g_return_if_fail (GIMP_IS_GIMP (gimp));

  display_config = GIMP_DISPLAY_CONFIG (gimp->config);
  gui_config     = GIMP_GUI_CONFIG (gimp->config);

  gimp->gui_main_loop_func      = gui_main;
  gimp->gui_main_loop_quit_func = gui_main_quit;
  gimp->gui_set_busy_func       = gui_set_busy;
  gimp->gui_unset_busy_func     = gui_unset_busy;
  gimp->gui_message_func        = gui_message;
  gimp->gui_create_display_func = gui_display_new;

  image_disconnect_handler_id =
    gimp_container_add_handler (gimp->images, "disconnect",
				G_CALLBACK (gui_image_disconnect),
				gimp);

  g_signal_connect (G_OBJECT (gimp_get_user_context (gimp)), "display_changed",
		    G_CALLBACK (gui_display_changed),
		    gimp);

  /* make sure the monitor resolution is valid */
  if (display_config->monitor_res_from_gdk               ||
      display_config->monitor_xres < GIMP_MIN_RESOLUTION ||
      display_config->monitor_yres < GIMP_MIN_RESOLUTION)
    {
      gdouble xres, yres;
      
      gui_get_screen_resolution (&xres, &yres);

      g_object_set (G_OBJECT (gimp->config),
			      "monitor-xresolution",                      xres,
			      "monitor-yresolution",                      yres,
			      "monitor-resolution-from-windowing-system", TRUE,
			      NULL);
    }

  menus_init (gimp);
  render_init (gimp);

  dialogs_init (gimp);

  gimp_devices_init (gimp, device_status_dialog_update_current);
  session_init (gimp);
}

void
gui_restore (Gimp     *gimp,
             gboolean  restore_session)
{
  g_return_if_fail (GIMP_IS_GIMP (gimp));

  gimp->message_handler = GIMP_MESSAGE_BOX;

  file_open_dialog_menu_init (gimp, gimp_item_factory_from_path ("<Load>"));
  file_save_dialog_menu_init (gimp, gimp_item_factory_from_path ("<Save>"));

  menus_restore (gimp);

  gimp_devices_restore (gimp);

  if (GIMP_GUI_CONFIG (gimp->config)->restore_session || restore_session)
    session_restore (gimp);

  dialogs_show_toolbox ();

  g_signal_connect (G_OBJECT (gimp), "exit",
                    G_CALLBACK (gui_exit_callback),
                    NULL);
  g_signal_connect_after (G_OBJECT (gimp), "exit",
                          G_CALLBACK (gui_exit_finish_callback),
                          NULL);
}

void
gui_post_init (Gimp *gimp)
{
  g_return_if_fail (GIMP_IS_GIMP (gimp));

  if (GIMP_GUI_CONFIG (gimp->config)->show_tips)
    {
      gimp_dialog_factory_dialog_new (global_dialog_factory,
                                      "gimp-tips-dialog", -1);
    }
}

void
gui_get_screen_resolution (gdouble *xres,
                           gdouble *yres)
{
  gint    width, height;
  gint    width_mm, height_mm;
  gdouble x = 0.0;
  gdouble y = 0.0;

  g_return_if_fail (xres != NULL);
  g_return_if_fail (yres != NULL);

  width  = gdk_screen_width ();
  height = gdk_screen_height ();

  width_mm  = gdk_screen_width_mm ();
  height_mm = gdk_screen_height_mm ();

  /*
   * From xdpyinfo.c:
   *
   * there are 2.54 centimeters to an inch; so there are 25.4 millimeters.
   *
   *     dpi = N pixels / (M millimeters / (25.4 millimeters / 1 inch))
   *         = N pixels / (M inch / 25.4)
   *         = N * 25.4 pixels / M inch
   */

  if (width_mm > 0 && height_mm > 0)
    {
      x = (width  * 25.4) / (gdouble) width_mm;
      y = (height * 25.4) / (gdouble) height_mm;
    }

  if (x < GIMP_MIN_RESOLUTION || x > GIMP_MAX_RESOLUTION ||
      y < GIMP_MIN_RESOLUTION || y > GIMP_MAX_RESOLUTION)
    {
      g_warning ("GDK returned bogus values for the screen resolution, "
                 "using 75 dpi instead.");

      x = 75.0;
      y = 75.0;
    }

  *xres = x;
  *yres = y;
}


/*  private functions  */

static void
gui_main (Gimp *gimp)
{
  gtk_main ();
}

static void
gui_main_quit (Gimp *gimp)
{
  gtk_main_quit ();
}

static void
gui_set_busy (Gimp *gimp)
{
  gimp_displays_set_busy (gimp);
  gimp_dialog_factories_idle ();

  gdk_flush ();
}

static void
gui_unset_busy (Gimp *gimp)
{
  gimp_displays_unset_busy (gimp);
  gimp_dialog_factories_unidle ();

  gdk_flush ();
}

static void
gui_message (Gimp        *gimp,
             const gchar *message)
{
  switch (gimp->message_handler)
    {
    case GIMP_MESSAGE_BOX:
      gimp_message_box (message, NULL, NULL);
      break;

    case GIMP_ERROR_CONSOLE:
      gimp_dialog_factory_dialog_raise (global_dock_factory,
                                        "gimp-error-console", -1);
      error_console_add (gimp, message);
      break;

    default:
      break;
    }
}

gboolean double_speed = FALSE;

static GimpObject *
gui_display_new (GimpImage *gimage,
                 guint      scale)
{
  GimpDisplay      *gdisp;
  GimpDisplayShell *shell;

  gdisp = gimp_display_new (gimage, scale);

  shell = GIMP_DISPLAY_SHELL (gdisp->shell);

  gimp_display_shell_set_menu_sensitivity (shell, gimage->gimp, FALSE);

  gimp_context_set_display (gimp_get_user_context (gimage->gimp), gdisp);

  if (double_speed)
    g_signal_connect_after (G_OBJECT (shell->canvas), "expose_event",
                            G_CALLBACK (gui_rotate_the_shield_harmonics),
                            NULL);

  return GIMP_OBJECT (gdisp);
}

static void
gui_themes_dir_foreach_func (GimpDatafileData *file_data)
{
  Gimp  *gimp;
  gchar *basename;

  gimp = (Gimp *) file_data->user_data;

  basename = g_path_get_basename (file_data->filename);

  if (gimp->be_verbose)
    g_print (_("Adding theme '%s' (%s)\n"), basename, file_data->filename);

  g_hash_table_insert (themes_hash,
		       basename,
		       g_strdup (file_data->filename));
}

static gint
gui_rotate_the_shield_harmonics (GtkWidget *widget,
				 GdkEvent  *eevent,
				 gpointer   data)
{
  GdkPixmap *pixmap = NULL;
  GdkBitmap *mask   = NULL;
  gint       width  = 0;
  gint       height = 0;

  g_signal_handlers_disconnect_by_func (G_OBJECT (widget),
					gui_rotate_the_shield_harmonics,
					data);

  pixmap = gdk_pixmap_create_from_xpm_d (widget->window,
					 &mask,
					 NULL,
					 wilber2_xpm);

  gdk_drawable_get_size (pixmap, &width, &height);

  if (widget->allocation.width  >= width &&
      widget->allocation.height >= height)
    {
      gint x, y;

      x = (widget->allocation.width  - width) / 2;
      y = (widget->allocation.height - height) / 2;

      gdk_gc_set_clip_mask (widget->style->black_gc, mask);
      gdk_gc_set_clip_origin (widget->style->black_gc, x, y);

      gdk_draw_drawable (widget->window,
			 widget->style->black_gc,
			 pixmap, 0, 0,
			 x, y,
			 width, height);

      gdk_gc_set_clip_mask (widget->style->black_gc, NULL);
      gdk_gc_set_clip_origin (widget->style->black_gc, 0, 0);
    }

  g_object_unref (pixmap);
  g_object_unref (mask);

  return FALSE;
}

static gboolean
gui_exit_callback (Gimp     *gimp,
                   gboolean  kill_it)
{
  g_print ("EXIT: gui_exit_callback(%s)\n",
           kill_it ? "TRUE" : "FALSE");

  if (! kill_it && gimp_displays_dirty (gimp))
    {
      GtkWidget *dialog;

      gimp_item_factories_set_sensitive ("<Toolbox>", "/File/Quit", FALSE);
      gimp_item_factories_set_sensitive ("<Image>",   "/File/Quit", FALSE);

      dialog = gimp_query_boolean_box (_("Quit The GIMP?"),
                                       gimp_standard_help_func,
                                       "dialogs/really_quit.html",
                                       GIMP_STOCK_WILBER_EEK,
                                       _("Some files are unsaved.\n"
                                         "\nReally quit The GIMP?"),
                                       GTK_STOCK_QUIT, GTK_STOCK_CANCEL,
                                       NULL, NULL,
                                       gui_really_quit_callback,
                                       gimp);

      gtk_widget_show (dialog);

      return TRUE; /* stop exit for now */
    }

  gimp->message_handler = GIMP_CONSOLE;

  session_save (gimp);

  if (GIMP_GUI_CONFIG (gimp->config)->save_device_status)
    gimp_devices_save (gimp);

  gimp_displays_delete (gimp);

  return FALSE; /* continue exiting */
}

static gboolean
gui_exit_finish_callback (Gimp     *gimp,
                          gboolean  kill_it)
{
  g_print ("EXIT: gui_exit_finish_callback(%s)\n",
           kill_it ? "TRUE" : "FALSE");

  menus_exit (gimp);
  render_exit (gimp);

  dialogs_exit (gimp);
  gimp_devices_exit (gimp);

  gimp_help_free ();

  g_signal_handlers_disconnect_by_func (G_OBJECT (gimp->config),
                                        gui_show_tooltips_notify,
                                        gimp);

  gimp_container_remove_handler (gimp->images, image_disconnect_handler_id);
  image_disconnect_handler_id = 0;

  if (themes_hash)
    {
      g_hash_table_destroy (themes_hash);
      themes_hash = NULL;
    }

  g_type_class_unref (g_type_class_peek (GIMP_TYPE_COLOR_SELECT));

  return FALSE; /* continue exiting */
}

static void
gui_really_quit_callback (GtkWidget *button,
			  gboolean   quit,
			  gpointer   data)
{
  Gimp *gimp;

  gimp = GIMP (data);

  if (quit)
    {
      gimp_exit (gimp, TRUE);
    }
  else
    {
      gimp_item_factories_set_sensitive ("<Toolbox>", "/File/Quit", TRUE);
      gimp_item_factories_set_sensitive ("<Image>",   "/File/Quit", TRUE);
    }
}

static void
gui_show_tooltips_notify (GObject    *config,
                          GParamSpec *param_spec,
                          Gimp       *gimp)
{
  gboolean show_tool_tips;

  g_object_get (config,
                "show-tool-tips", &show_tool_tips,
                NULL);

  if (show_tool_tips)
    gimp_help_enable_tooltips ();
  else
    gimp_help_disable_tooltips ();
}


#ifdef __GNUC__
#warning FIXME: this junk should mostly go to the display subsystem
#endif

static void
gui_display_changed (GimpContext *context,
		     GimpDisplay *display,
		     Gimp        *gimp)
{
  GimpDisplayShell *shell = NULL;

  if (display)
    shell = GIMP_DISPLAY_SHELL (display->shell);

  gimp_display_shell_set_menu_sensitivity (shell, gimp, TRUE);
}

static void
gui_image_disconnect (GimpImage *gimage,
		      Gimp      *gimp)
{
  /*  check if this is the last image and if it had a display  */
  if (gimp_container_num_children (gimp->images) == 1 &&
      gimage->instance_count                      > 0)
    {
      dialogs_show_toolbox ();
    }
}
