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
#include "libgimpwidgets/gimpwidgets-private.h"

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
#include "display/gimpprogress.h"

#include "widgets/gimpdevices.h"
#include "widgets/gimpdevicestatus.h"
#include "widgets/gimpdialogfactory.h"
#include "widgets/gimperrorconsole.h"
#include "widgets/gimphelp.h"
#include "widgets/gimphelp-ids.h"
#include "widgets/gimpitemfactory.h"
#include "widgets/gimpmenufactory.h"
#include "widgets/gimpwidgets-utils.h"

#include "brush-select.h"
#include "dialogs.h"
#include "dialogs-commands.h"
#include "font-select.h"
#include "gradient-select.h"
#include "gui.h"
#include "menus.h"
#include "palette-select.h"
#include "pattern-select.h"
#include "plug-in-menus.h"
#include "session.h"
#include "themes.h"

#include "app_procs.h" /* FIXME */

#include "gimp-intl.h"


/*  local function prototypes  */

static void           gui_help_func            (const gchar   *help_id,
                                                gpointer       help_data);
static gboolean       gui_get_background_func  (GimpRGB       *color);
static gboolean       gui_get_foreground_func  (GimpRGB       *color);

static void           gui_threads_enter        (Gimp          *gimp);
static void           gui_threads_leave        (Gimp          *gimp);
static void           gui_set_busy             (Gimp          *gimp);
static void           gui_unset_busy           (Gimp          *gimp);
static void           gui_message              (Gimp          *gimp,
                                                const gchar   *domain,
                                                const gchar   *message);
static GimpObject   * gui_display_new          (GimpImage     *gimage,
                                                guint          scale);
static void           gui_menus_init           (Gimp          *gimp,
                                                GSList        *plug_in_defs,
                                                const gchar   *plugins_domain);
static void           gui_menus_create_entry   (Gimp          *gimp,
                                                PlugInProcDef *proc_def,
                                                const gchar   *locale_domain,
                                                const gchar   *help_domain);
static void           gui_menus_delete_entry   (Gimp          *gimp,
                                                const gchar   *menu_path);
static GimpProgress * gui_start_progress       (Gimp          *gimp,
                                                gint           gdisp_ID,
                                                const gchar   *message,
                                                GCallback      cancel_cb,
                                                gpointer       cancel_data);
static GimpProgress * gui_restart_progress     (Gimp          *gimp,
                                                GimpProgress  *progress,
                                                const gchar   *message,
                                                GCallback      cancel_cb,
                                                gpointer       cancel_data);
static void           gui_update_progress      (Gimp          *gimp,
                                                GimpProgress  *progress,
                                                gdouble        percentage);
static void           gui_end_progress         (Gimp          *gimp,
                                                GimpProgress  *progress);
static void           gui_pdb_dialogs_check    (Gimp          *gimp);

static gboolean       gui_exit_callback        (Gimp          *gimp,
                                                gboolean       kill_it);
static gboolean       gui_exit_finish_callback (Gimp          *gimp,
                                                gboolean       kill_it);
static void           gui_really_quit_callback (GtkWidget     *button,
                                                gboolean       quit,
                                                gpointer       data);
static void           gui_show_tooltips_notify (GObject       *config,
                                                GParamSpec    *param_spec,
                                                Gimp          *gimp);
static void           gui_device_change_notify (Gimp          *gimp);

static void           gui_display_changed      (GimpContext   *context,
                                                GimpDisplay   *display,
                                                Gimp          *gimp);
static void           gui_image_disconnect     (GimpImage     *gimage,
                                                Gimp          *gimp);


/*  private variables  */

static GQuark image_disconnect_handler_id = 0;

static GimpItemFactory *toolbox_item_factory = NULL;
static GimpItemFactory *image_item_factory   = NULL;


/*  public functions  */

gboolean
gui_libs_init (gint    *argc,
	       gchar ***argv)
{
  GimpWidgetsVTable vtable;

  g_return_val_if_fail (argc != NULL, FALSE);
  g_return_val_if_fail (argv != NULL, FALSE);

  if (!gtk_init_check (argc, argv))
    return FALSE;

  /*  Initialize the eeky vtable needed by libgimpwidgets  */
  vtable.unit_get_number_of_units          = gimp_unit_get_number_of_units;
  vtable.unit_get_number_of_built_in_units = gimp_unit_get_number_of_built_in_units;
  vtable.unit_get_factor          = gimp_unit_get_factor;
  vtable.unit_get_digits          = gimp_unit_get_digits;
  vtable.unit_get_identifier      = gimp_unit_get_identifier;
  vtable.unit_get_symbol          = gimp_unit_get_symbol;
  vtable.unit_get_abbreviation    = gimp_unit_get_abbreviation;
  vtable.unit_get_singular        = gimp_unit_get_singular;
  vtable.unit_get_plural          = gimp_unit_get_plural;

  gimp_widgets_init (&vtable,
                     gui_help_func,
                     gui_get_foreground_func,
                     gui_get_background_func);

  g_type_class_ref (GIMP_TYPE_COLOR_SELECT);

  return TRUE;
}

void
gui_environ_init (Gimp *gimp)
{
  const gchar *name = NULL;

  g_return_if_fail (GIMP_IS_GIMP (gimp));

#if defined (GDK_WINDOWING_X11)
  name = "DISPLAY";
#elif defined (GDK_WINDOWING_DIRECTFB) || defined (GDK_WINDOWING_FB)
  name = "GDK_DISPLAY";
#endif

  /* TODO: Need to care about display migration with GTK+ 2.2 at some point */

  if (name)
    {
      gchar *display;

      display = gdk_get_display ();
      gimp_environ_table_add (gimp->environ_table, name, display, NULL);
      g_free (display);
    }
}

void
gui_themes_init (Gimp *gimp)
{
  g_return_if_fail (GIMP_IS_GIMP (gimp));

  themes_init (gimp);

  if (! GIMP_GUI_CONFIG (gimp->config)->show_tool_tips)
    gimp_help_disable_tooltips ();

  g_signal_connect (gimp->config, "notify::show-tool-tips",
                    G_CALLBACK (gui_show_tooltips_notify),
                    gimp);

  gdk_rgb_set_min_colors (CLAMP (gimp->config->min_colors, 27, 256));
  gdk_rgb_set_install (gimp->config->install_cmap);

  gtk_widget_set_default_colormap (gdk_rgb_get_colormap ());
}

void
gui_init (Gimp *gimp)
{
  GimpDisplayConfig *display_config;
  GimpGuiConfig     *gui_config;

  g_return_if_fail (GIMP_IS_GIMP (gimp));

  display_config = GIMP_DISPLAY_CONFIG (gimp->config);
  gui_config     = GIMP_GUI_CONFIG (gimp->config);

  gimp->gui_threads_enter_func     = gui_threads_enter;
  gimp->gui_threads_leave_func     = gui_threads_leave;
  gimp->gui_set_busy_func          = gui_set_busy;
  gimp->gui_unset_busy_func        = gui_unset_busy;
  gimp->gui_message_func           = gui_message;
  gimp->gui_create_display_func    = gui_display_new;
  gimp->gui_menus_init_func        = gui_menus_init;
  gimp->gui_menus_create_func      = gui_menus_create_entry;
  gimp->gui_menus_delete_func      = gui_menus_delete_entry;
  gimp->gui_progress_start_func    = gui_start_progress;
  gimp->gui_progress_restart_func  = gui_restart_progress;
  gimp->gui_progress_update_func   = gui_update_progress;
  gimp->gui_progress_end_func      = gui_end_progress;
  gimp->gui_pdb_dialogs_check_func = gui_pdb_dialogs_check;

  image_disconnect_handler_id =
    gimp_container_add_handler (gimp->images, "disconnect",
				G_CALLBACK (gui_image_disconnect),
				gimp);

  g_signal_connect (gimp_get_user_context (gimp), "display_changed",
		    G_CALLBACK (gui_display_changed),
		    gimp);

  /* make sure the monitor resolution is valid */
  if (display_config->monitor_res_from_gdk               ||
      display_config->monitor_xres < GIMP_MIN_RESOLUTION ||
      display_config->monitor_yres < GIMP_MIN_RESOLUTION)
    {
      gdouble xres, yres;

      gimp_get_screen_resolution (NULL, &xres, &yres);

      g_object_set (gimp->config,
		    "monitor-xresolution",                      xres,
		    "monitor-yresolution",                      yres,
		    "monitor-resolution-from-windowing-system", TRUE,
		    NULL);
    }

  menus_init (gimp);
  render_init (gimp);

  dialogs_init (gimp);

  gimp_devices_init (gimp, gui_device_change_notify);
  session_init (gimp);
}

void
gui_restore (Gimp     *gimp,
             gboolean  restore_session)
{
  g_return_if_fail (GIMP_IS_GIMP (gimp));

  gimp->message_handler = GIMP_MESSAGE_BOX;

  menus_restore (gimp);

  toolbox_item_factory = gimp_menu_factory_menu_new (global_menu_factory,
                                                     "<Toolbox>",
                                                     GTK_TYPE_MENU_BAR,
                                                     gimp,
                                                     TRUE);

  image_item_factory = gimp_menu_factory_menu_new (global_menu_factory,
                                                   "<Image>",
                                                   GTK_TYPE_MENU,
                                                   gimp,
                                                   TRUE);

  gimp_devices_restore (gimp);

  if (GIMP_GUI_CONFIG (gimp->config)->restore_session || restore_session)
    session_restore (gimp);

  dialogs_show_toolbox ();

  g_signal_connect (gimp, "exit",
                    G_CALLBACK (gui_exit_callback),
                    NULL);
  g_signal_connect_after (gimp, "exit",
                          G_CALLBACK (gui_exit_finish_callback),
                          NULL);

#ifdef __GNUC__
#warning FIXME: remove this as soon as we depend on GTK+ >= 2.2.2
#endif
  if (gtk_check_version (2, 2, 2) != NULL)
    gimp_message_box (GIMP_STOCK_WILBER_EEK, NULL,
                      "Please upgrade your GTK+ installation!\n\n"
                      "The GTK+ version you are using is too old.\n"
                      "Please upgrade to GTK+ version 2.2.2 or better\n"
                      "or your extended input devices (tablets) will\n"
                      "not work at all!",
                      NULL, NULL);
}

void
gui_post_init (Gimp *gimp)
{
  g_return_if_fail (GIMP_IS_GIMP (gimp));

  if (GIMP_GUI_CONFIG (gimp->config)->show_tips)
    gimp_dialog_factory_dialog_new (global_dialog_factory,
                                    "gimp-tips-dialog", -1);
}


/*  private functions  */

static void
gui_help_func (const gchar *help_id,
               gpointer     help_data)
{
  gimp_help (the_gimp, NULL, help_id);
}

static gboolean
gui_get_foreground_func (GimpRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);

  gimp_context_get_foreground (gimp_get_user_context (the_gimp), color);

  return TRUE;
}

static gboolean
gui_get_background_func (GimpRGB *color)
{
  g_return_val_if_fail (color != NULL, FALSE);

  gimp_context_get_background (gimp_get_user_context (the_gimp), color);

  return TRUE;
}

static void
gui_threads_enter (Gimp *gimp)
{
  GDK_THREADS_ENTER ();
}

static void
gui_threads_leave (Gimp *gimp)
{
  GDK_THREADS_LEAVE ();
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
             const gchar *domain,
             const gchar *message)
{
  if (gimp->message_handler == GIMP_ERROR_CONSOLE)
    {
      GtkWidget *dockable;

      dockable = gimp_dialog_factory_dialog_raise (global_dock_factory,
                                                   "gimp-error-console", -1);

      if (dockable)
        {
          GimpErrorConsole *console;

          console = GIMP_ERROR_CONSOLE (GTK_BIN (dockable)->child);

          gimp_error_console_add (console, GIMP_STOCK_WARNING, domain, message);

          return;
        }

      gimp->message_handler = GIMP_MESSAGE_BOX;
    }

  gimp_message_box (GIMP_STOCK_WARNING, domain, message, NULL, NULL);
}

static GimpObject *
gui_display_new (GimpImage *gimage,
                 guint      scale)
{
  GimpDisplayShell *shell;
  GimpDisplay      *gdisp;

  gdisp = gimp_display_new (gimage, scale,
                            global_menu_factory,
                            image_item_factory);

  shell = GIMP_DISPLAY_SHELL (gdisp->shell);

  gimp_context_set_display (gimp_get_user_context (gimage->gimp), gdisp);

  gimp_item_factory_update (shell->menubar_factory, shell);

  return GIMP_OBJECT (gdisp);
}

static void
gui_menus_init (Gimp        *gimp,
                GSList      *plug_in_defs,
                const gchar *std_plugins_domain)
{
  plug_in_menus_init (gimp, plug_in_defs, std_plugins_domain);
}

static void
gui_menus_create_entry (Gimp          *gimp,
                        PlugInProcDef *proc_def,
                        const gchar   *locale_domain,
                        const gchar   *help_domain)
{
  plug_in_menus_create_entry (NULL, proc_def, locale_domain, help_domain);
}

static void
gui_menus_delete_entry (Gimp        *gimp,
                        const gchar *menu_path)
{
  plug_in_menus_delete_entry (menu_path);
}

static GimpProgress *
gui_start_progress (Gimp        *gimp,
                    gint         gdisp_ID,
                    const gchar *message,
                    GCallback    cancel_cb,
                    gpointer     cancel_data)
{
  GimpDisplay *gdisp = NULL;

  if (gdisp_ID > 0)
    gdisp = gimp_display_get_by_ID (gimp, gdisp_ID);

  return gimp_progress_start (gdisp, message, TRUE, cancel_cb, cancel_data);
}

static GimpProgress *
gui_restart_progress (Gimp         *gimp,
                      GimpProgress *progress,
                      const gchar  *message,
                      GCallback     cancel_cb,
                      gpointer      cancel_data)
{
  return gimp_progress_restart (progress, message, cancel_cb, cancel_data);
}

static void
gui_update_progress (Gimp         *gimp,
                     GimpProgress *progress,
                     gdouble       percentage)
{
  gimp_progress_update (progress, percentage);
}

static void
gui_end_progress (Gimp         *gimp,
                  GimpProgress *progress)
{
  gimp_progress_end (progress);
}

static void
gui_pdb_dialogs_check (Gimp *gimp)
{
  brush_select_dialogs_check ();
  font_select_dialogs_check ();
  gradient_select_dialogs_check ();
  palette_select_dialogs_check ();
  pattern_select_dialogs_check ();
}

static gboolean
gui_exit_callback (Gimp     *gimp,
                   gboolean  kill_it)
{
  if (! kill_it && gimp_displays_dirty (gimp))
    {
      GtkWidget *dialog;

      gimp_item_factories_set_sensitive ("<Toolbox>", "/File/Quit", FALSE);
      gimp_item_factories_set_sensitive ("<Image>",   "/File/Quit", FALSE);

      dialog = gimp_query_boolean_box (_("Quit The GIMP?"),
                                       gimp_standard_help_func,
                                       GIMP_HELP_FILE_QUIT_CONFIRM,
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
  menus_save (gimp);

  if (GIMP_GUI_CONFIG (gimp->config)->save_device_status)
    gimp_devices_save (gimp);

  gimp_displays_delete (gimp);

  return FALSE; /* continue exiting */
}

static gboolean
gui_exit_finish_callback (Gimp     *gimp,
                          gboolean  kill_it)
{
  g_signal_handlers_disconnect_by_func (gimp->config,
                                        gui_show_tooltips_notify,
                                        gimp);

  gimp_container_remove_handler (gimp->images, image_disconnect_handler_id);
  image_disconnect_handler_id = 0;

  g_object_unref (toolbox_item_factory);
  toolbox_item_factory = NULL;

  g_object_unref (image_item_factory);
  image_item_factory = NULL;

  menus_exit (gimp);
  render_exit (gimp);

  dialogs_exit (gimp);
  gimp_devices_exit (gimp);

  themes_exit (gimp);

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

static void
gui_device_change_notify (Gimp *gimp)
{
  GimpSessionInfo *session_info;

  session_info = gimp_dialog_factory_find_session_info (global_dock_factory,
                                                        "gimp-device-status");

  if (session_info && session_info->widget)
    {
      GtkWidget *device_status;

      device_status = GTK_BIN (session_info->widget)->child;

      gimp_device_status_update (GIMP_DEVICE_STATUS (device_status));
    }
}


#ifdef __GNUC__
#warning FIXME: this junk should mostly go to the display subsystem
#endif

static void
gui_display_changed (GimpContext *context,
		     GimpDisplay *display,
		     Gimp        *gimp)
{
  GimpItemFactory  *item_factory;
  GimpDisplayShell *shell = NULL;

  item_factory = gimp_item_factory_from_path ("<Image>");

  if (display)
    shell = GIMP_DISPLAY_SHELL (display->shell);

  gimp_item_factory_update (item_factory, shell);
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
