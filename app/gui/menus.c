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
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "libgimpbase/gimpbase.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "tools/tools-types.h"

#include "core/gimp.h"
#include "core/gimplist.h"
#include "core/gimptoolinfo.h"

#include "tools/gimpbrightnesscontrasttool.h"
#include "tools/gimpcolorbalancetool.h"
#include "tools/gimpcurvestool.h"
#include "tools/gimphuesaturationtool.h"
#include "tools/gimplevelstool.h"
#include "tools/gimpposterizetool.h"
#include "tools/gimpthresholdtool.h"
#include "tools/tool_manager.h"

#include "channels-commands.h"
#include "commands.h"
#include "data-commands.h"
#include "dialogs-commands.h"
#include "edit-commands.h"
#include "file-commands.h"
#include "gradients-commands.h"
#include "image-commands.h"
#include "layers-commands.h"
#include "menus.h"
#include "palettes-commands.h"
#include "paths-dialog.h"
#include "select-commands.h"
#include "test-commands.h"
#include "tools-commands.h"
#include "view-commands.h"

#include "app_procs.h"
#include "gdisplay.h"
#include "gimphelp.h"
#include "gimprc.h"

#include "libgimp/gimpintl.h"


/*  local function prototypes  */

static void    menus_create_item          (GtkItemFactory       *item_factory,
					   GimpItemFactoryEntry *entry,
					   gpointer              callback_data,
					   guint                 callback_type,
					   gboolean              create_tearoff,
					   gboolean              static_entry);
static void    menus_create_items         (GtkItemFactory       *item_factory,
					   guint                 n_entries,
					   GimpItemFactoryEntry *entries,
					   gpointer              callback_data,
					   guint                 callback_type,
					   gboolean              create_tearoff,
					   gboolean              static_entries);
static void    menus_create_branches      (GtkItemFactory       *item_factory,
					   GimpItemFactoryEntry *entry);
static void    menus_init                 (void);

#ifdef ENABLE_NLS
static gchar * menus_menu_translate_func  (const gchar          *path,
					   gpointer              data);
#else
#define        menus_menu_translate_func  (NULL)
#endif

static void    menus_tearoff_cmd_callback (GtkWidget            *widget,
					   gpointer              data,
					   guint                 action);

#ifdef ENABLE_DEBUG_ENTRY
static void    menus_debug_recurse_menu   (GtkWidget            *menu,
					   gint                  depth,
					   gchar                *path);
static void    menus_debug_cmd_callback   (GtkWidget            *widget,
					   gpointer              data,
					   guint                 action);
#endif  /*  ENABLE_DEBUG_ENTRY  */


GSList *last_opened_raw_filenames = NULL;


#define SEPARATOR(path) \
        { { (path), NULL, NULL, 0, "<Separator>" }, NULL, NULL, NULL }

#define BRANCH(path) \
        { { (path), NULL, NULL, 0, "<Branch>" }, NULL, NULL, NULL }


/*****  <Toolbox>  *****/

static GimpItemFactoryEntry toolbox_entries[] =
{
  /*  <Toolbox>/File  */

  BRANCH (N_("/_File")),

  { { N_("/File/New..."), "<control>N",
      file_new_cmd_callback, 0 },
    NULL,
    "file/dialogs/file_new.html", NULL },
  { { N_("/File/Open..."), "<control>O",
      file_open_cmd_callback, 0 },
    NULL,
    "file/dialogs/file_open.html", NULL },

  /*  <Toolbox>/File/Acquire  */

  SEPARATOR ("/File/---"),

  BRANCH (N_("/File/Acquire")),

  SEPARATOR ("/File/---"),

  { { N_("/File/Preferences..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:preferences-dialog",
    "file/dialogs/preferences/preferences.html", NULL },

  /*  <Toolbox>/File/Dialogs  */

  SEPARATOR ("/File/---"),

  { { N_("/File/Dialogs/Layers, Channels & Paths..."), "<control>L",
      dialogs_create_lc_cmd_callback, 0 },
    NULL,
    "file/dialogs/layers_and_channels.html", NULL },
  { { N_("/File/Dialogs/Tool Options..."), "<control><shift>T",
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:tool-options-dialog",
    "file/dialogs/tool_options.html", NULL },

  SEPARATOR ("/File/Dialogs/---"),

  { { "/File/Dialogs/Old + Testing/Brushes...", NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:brush-select-dialog",
    "file/dialogs/brush_selection.html", NULL },
  { { "/File/Dialogs/Old + Testing/Patterns...", NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:pattern-select-dialog",
    "file/dialogs/pattern_selection.html", NULL },
  { { "/File/Dialogs/Old + Testing/Gradients...", NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:gradient-select-dialog",
    "file/dialogs/gradient_selection.html", NULL },
  { { "/File/Dialogs/Old + Testing/Palette...", NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:palette-select-dialog",
    "file/dialogs/palette_selection.html", NULL },

  SEPARATOR ("/File/Dialogs/Old + Testing/---"),

  { { "/File/Dialogs/Old + Testing/Multi List...", NULL,
      test_multi_container_list_view_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { "/File/Dialogs/Old + Testing/Multi Grid...", NULL,
      test_multi_container_grid_view_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  { { N_("/File/Dialogs/Brushes..."), "<control><shift>B",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:brush-grid",
    "file/dialogs/brush_selection.html", NULL },
  { { N_("/File/Dialogs/Patterns..."), "<control><shift>P",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:pattern-grid",
    "file/dialogs/pattern_selection.html", NULL },
  { { N_("/File/Dialogs/Gradients..."), "<control>G",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:gradient-list",
    "file/dialogs/gradient_selection.html", NULL },
  { { N_("/File/Dialogs/Palettes..."), "<control>P",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:palette-list",
    "file/dialogs/palette_selection.html", NULL },
  { { N_("/File/Dialogs/Indexed Palette..."), NULL,
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:indexed-palette",
    "file/dialogs/indexed_palette.html", NULL },
  { { N_("/File/Dialogs/Buffers..."), NULL,
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:buffer-list",
    NULL, NULL },

  SEPARATOR ("/File/Dialogs/---"),

  { { N_("/File/Dialogs/Input Devices..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:input-devices-dialog",
    "file/dialogs/input_devices.html", NULL },
  { { N_("/File/Dialogs/Device Status..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:device-status-dialog",
    "file/dialogs/device_status.html", NULL },

  SEPARATOR ("/File/Dialogs/---"),

  { { N_("/File/Dialogs/Document Index..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:document-index-dialog",
    "file/dialogs/document_index.html", NULL },
  { { N_("/File/Dialogs/Error Console..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:error-console-dialog",
    "file/dialogs/error_console.html", NULL },
#ifdef DISPLAY_FILTERS
  { { N_("/File/Dialogs/Display Filters..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:display-filters-dialog",
    "file/dialogs/display_filters/display_filters.html", NULL },
#endif /* DISPLAY_FILTERS */

  SEPARATOR ("/File/---"),

  /*  MRU entries are inserted here  */

  SEPARATOR ("/File/---MRU"),

  { { N_("/File/Quit"), "<control>Q",
      file_quit_cmd_callback, 0 },
    NULL,
    "file/quit.html", NULL },

  /*  <Toolbox>/Xtns  */

  BRANCH (N_("/_Xtns")),

  { { N_("/Xtns/Module Browser..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:module-browser-dialog",
    "dialogs/module_browser.html", NULL },

  SEPARATOR ("/Xtns/---"),

  /*  <Toolbox>/Help  */

  BRANCH (N_("/_Help")),

  { { N_("/Help/Help..."), "F1",
      help_help_cmd_callback, 0 },
    NULL,
    "help/dialogs/help.html", NULL },
  { { N_("/Help/Context Help..."), "<shift>F1",
      help_context_help_cmd_callback, 0 },
    NULL,
    "help/context_help.html", NULL },
  { { N_("/Help/Tip of the Day..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:tips-dialog",
    "help/dialogs/tip_of_the_day.html", NULL },
  { { N_("/Help/About..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:about-dialog",
    "help/dialogs/about.html", NULL },

  SEPARATOR ("/Help/---"),

  { { N_("/Help/Mem Profile"), NULL,
      mem_profile_cmd_callback, 0 },
    NULL,
    NULL, NULL },

#ifdef ENABLE_DEBUG_ENTRY
  { { "/Help/Dump Items (Debug)", NULL,
      menus_debug_cmd_callback, 0 },
    NULL,
    NULL, NULL }
#endif
};
static guint n_toolbox_entries = (sizeof (toolbox_entries) /
				  sizeof (toolbox_entries[0]));
static GtkItemFactory *toolbox_factory = NULL;


/*****  <Image>  *****/

static GimpItemFactoryEntry image_entries[] =
{
  { { "/tearoff1", NULL, menus_tearoff_cmd_callback, 0, "<Tearoff>" },
    NULL,
    NULL, NULL },

  /*  <Image>/File  */

  { { N_("/File/New..."), "<control>N",
      file_new_cmd_callback, 1 },
    NULL,
    "file/dialogs/file_new.html", NULL },
  { { N_("/File/Open..."), "<control>O",
      file_open_cmd_callback, 0 },
    NULL,
    "file/dialogs/file_open.html", NULL },
  { { N_("/File/Save"), "<control>S",
      file_save_cmd_callback, 0 },
    NULL,
    "file/dialogs/file_save.html", NULL },
  { { N_("/File/Save as..."), NULL,
      file_save_as_cmd_callback, 0 },
    NULL,
    "file/dialogs/file_save.html", NULL },
  { { N_("/File/Save a Copy as..."), NULL,
      file_save_a_copy_as_cmd_callback, 0 },
    NULL,
    "file/dialogs/file_save.html", NULL },
  { { N_("/File/Revert..."), NULL,
      file_revert_cmd_callback, 0 },
    NULL,
    "file/revert.html", NULL },

  SEPARATOR ("/File/---"),

  { { N_( "/File/Close"), "<control>W",
      file_close_cmd_callback, 0 },
    NULL,
    "file/close.html", NULL },
  { { N_("/File/Quit"), "<control>Q",
      file_quit_cmd_callback, 0 },
    NULL,
    "file/quit.html", NULL },

  SEPARATOR ("/File/---moved"),

  /*  <Image>/Edit  */

  { { N_("/Edit/Undo"), "<control>Z",
      edit_undo_cmd_callback, 0 },
    NULL,
    "edit/undo.html", NULL },
  { { N_("/Edit/Redo"), "<control>R",
      edit_redo_cmd_callback, 0 },
    NULL,
    "edit/redo.html", NULL },

  SEPARATOR ("/Edit/---"),

  { { N_("/Edit/Cut"), "<control>X",
      edit_cut_cmd_callback, 0 },
    NULL,
    "edit/cut.html", NULL },
  { { N_("/Edit/Copy"), "<control>C",
      edit_copy_cmd_callback, 0 },
    NULL,
    "edit/copy.html", NULL },
  { { N_("/Edit/Paste"), "<control>V",
      edit_paste_cmd_callback, 0 },
    NULL,
    "edit/paste.html", NULL },
  { { N_("/Edit/Paste Into"), NULL,
      edit_paste_into_cmd_callback, 0 },
    NULL,
    "edit/paste_into.html", NULL },
  { { N_("/Edit/Paste as New"), NULL,
      edit_paste_as_new_cmd_callback, 0 },
    NULL,
    "edit/paste_as_new.html", NULL },

  /*  <Image>/Edit/Buffer  */

  { { N_("/Edit/Buffer/Cut Named..."), "<control><shift>X",
      edit_named_cut_cmd_callback, 0 },
    NULL,
    "edit/dialogs/cut_named.html", NULL },
  { { N_("/Edit/Buffer/Copy Named..."), "<control><shift>C",
      edit_named_copy_cmd_callback, 0 },
    NULL,
    "edit/dialogs/copy_named.html", NULL },
  { { N_("/Edit/Buffer/Paste Named..."), "<control><shift>V",
      edit_named_paste_cmd_callback, 0 },
    NULL,
    "edit/dialogs/paste_named.html", NULL },

  SEPARATOR ("/Edit/---"),

  { { N_("/Edit/Clear"), "<control>K",
      edit_clear_cmd_callback, 0 },
    NULL,
    "edit/clear.html", NULL },
  { { N_("/Edit/Fill with FG Color"), "<control>comma",
      edit_fill_cmd_callback, (guint) FOREGROUND_FILL },
    NULL,
    "edit/fill.html", NULL },
  { { N_("/Edit/Fill with BG Color"), "<control>period",
      edit_fill_cmd_callback, (guint) BACKGROUND_FILL },
    NULL,
    "edit/fill.html", NULL },
  { { N_("/Edit/Stroke"), NULL,
      edit_stroke_cmd_callback, 0 },
    NULL,
    "edit/stroke.html", NULL },

  SEPARATOR ("/Edit/---"),

  /*  <Image>/Select  */
  
  { { N_("/Select/Invert"), "<control>I",
      select_invert_cmd_callback, 0 },
    NULL,
    "select/invert.html", NULL },
  { { N_("/Select/All"), "<control>A",
      select_all_cmd_callback, 0 },
    NULL,
    "select/all.html", NULL },
  { { N_("/Select/None"), "<control><shift>A",
      select_none_cmd_callback, 0 },
    NULL,
    "select/none.html", NULL },
  { { N_("/Select/Float"), "<control><shift>L",
      select_float_cmd_callback, 0 },
    NULL,
    "select/float.html", NULL },

  SEPARATOR ("/Select/---"),

  { { N_("/Select/Feather..."), "<control><shift>F",
      select_feather_cmd_callback, 0 },
    NULL,
    "select/dialogs/feather_selection.html", NULL },
  { { N_("/Select/Sharpen"), "<control><shift>H",
      select_sharpen_cmd_callback, 0 },
    NULL,
    "select/sharpen.html", NULL },
  { { N_("/Select/Shrink..."), NULL,
      select_shrink_cmd_callback, 0 },
    NULL,
    "select/dialogs/shrink_selection.html", NULL },
  { { N_("/Select/Grow..."), NULL,
      select_grow_cmd_callback, 0 },
    NULL,
    "select/dialogs/grow_selection.html", NULL },
  { { N_("/Select/Border..."), "<control><shift>B",
      select_border_cmd_callback, 0 },
    NULL,
    "select/dialogs/border_selection.html", NULL },

  SEPARATOR ("/Select/---"),

  { { N_("/Select/Save to Channel"), NULL,
      select_save_cmd_callback, 0 },
    NULL,
    "select/save_to_channel.html", NULL },

  /*  <Image>/View  */

  { { N_("/View/Zoom In"), "equal", view_zoomin_cmd_callback, 0 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom Out"), "minus", view_zoomout_cmd_callback, 0 },
    NULL,
    "view/zoom.html", NULL },

  /*  <Image>/View/Zoom  */

  { { N_("/View/Zoom/16:1"), NULL, view_zoom_cmd_callback, 1601 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/8:1"), NULL, view_zoom_cmd_callback, 801 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/4:1"), NULL, view_zoom_cmd_callback, 401 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/2:1"), NULL, view_zoom_cmd_callback, 201 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/1:1"), "1", view_zoom_cmd_callback, 101 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/1:2"), NULL, view_zoom_cmd_callback, 102 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/1:4"), NULL, view_zoom_cmd_callback, 104 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/1:8"), NULL, view_zoom_cmd_callback, 108 },
    NULL,
    "view/zoom.html", NULL },
  { { N_("/View/Zoom/1:16"), NULL, view_zoom_cmd_callback, 116 },
    NULL,
    "view/zoom.html", NULL },

  { { N_("/View/Dot for Dot"), NULL,
      view_dot_for_dot_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    "view/dot_for_dot.html", NULL },

  SEPARATOR ("/View/---"),

  { { N_("/View/Info Window..."), "<control><shift>I",
      view_info_window_cmd_callback, 0 },
    NULL,
    "view/dialogs/info_window.html", NULL },
  { { N_("/View/Nav. Window..."), "<control><shift>N",
      view_nav_window_cmd_callback, 0 },
    NULL,
    "view/dialogs/navigation_window.html", NULL },

  SEPARATOR ("/View/---"),

  { { N_("/View/Toggle Selection"), "<control>T",
      view_toggle_selection_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    "view/toggle_selection.html", NULL },
  { { N_("/View/Toggle Rulers"), "<control><shift>R",
      view_toggle_rulers_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    "view/toggle_rulers.html", NULL },
  { { N_("/View/Toggle Statusbar"), "<control><shift>S",
      view_toggle_statusbar_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    "view/toggle_statusbar.html", NULL },
  { { N_("/View/Toggle Guides"), "<control><shift>T",
      view_toggle_guides_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    "view/toggle_guides.html", NULL },
  { { N_("/View/Snap to Guides"), NULL,
      view_snap_to_guides_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    "view/snap_to_guides.html", NULL },

  SEPARATOR ("/View/---"),

  { { N_("/View/New View"), NULL,
      view_new_view_cmd_callback, 0 },
    NULL,
    "view/new_view.html", NULL },
  { { N_("/View/Shrink Wrap"), "<control>E",
      view_shrink_wrap_cmd_callback, 0 },
    NULL,
    "view/shrink_wrap.html", NULL },

  /*  <Image>/Image/Mode  */

  { { N_("/Image/Mode/RGB"), "<alt>R",
      image_convert_rgb_cmd_callback, 0 },
    NULL,
    "image/mode/convert_to_rgb.html", NULL },
  { { N_("/Image/Mode/Grayscale"), "<alt>G",
      image_convert_grayscale_cmd_callback, 0 },
    NULL,
    "image/mode/convert_to_grayscale.html", NULL },
  { { N_("/Image/Mode/Indexed..."), "<alt>I",
      image_convert_indexed_cmd_callback, 0 },
    NULL,
    "image/mode/dialogs/convert_to_indexed.html", NULL },

  SEPARATOR ("/Image/Mode/---"),

  /*  <Image>/Image/Colors  */

  { { N_("/Image/Colors/Desaturate"), NULL,
      image_desaturate_cmd_callback, 0 },
    NULL,
    "image/colors/desaturate.html", NULL },
  { { N_("/Image/Colors/Invert"), NULL,
      image_invert_cmd_callback, 0 },
    NULL,
    "image/colors/invert.html", NULL },

  SEPARATOR ("/Image/Colors/---"),

  /*  <Image>/Image/Colors/Auto  */

  { { N_("/Image/Colors/Auto/Equalize"), NULL,
      image_equalize_cmd_callback, 0 },
    NULL,
    "image/colors/auto/equalize.html", NULL },

  SEPARATOR ("/Image/Colors/---"),

  /*  <Image>/Image/Alpha  */

  { { N_("/Image/Alpha/Add Alpha Channel"), NULL,
      layers_add_alpha_channel_cmd_callback, 0 },
    NULL,
    "layers/add_alpha_channel.html", NULL },

  /*  <Image>/Image/Transforms  */

  { { N_("/Image/Transforms/Offset..."), "<control><shift>O",
      image_offset_cmd_callback, 0 },
    NULL,
    "image/transforms/dialogs/offset.html", NULL },

  BRANCH (N_("/Image/Transforms/Rotate")),

  SEPARATOR ("/Image/Transforms/---"),

  SEPARATOR ("/Image/---"),

  { { N_("/Image/Canvas Size..."), NULL,
      image_resize_cmd_callback, 0 },
    NULL,
    "image/dialogs/set_canvas_size.html", NULL },
  { { N_("/Image/Scale Image..."), NULL,
      image_scale_cmd_callback, 0 },
    NULL,
    "image/dialogs/scale_image.html", NULL },
  { { N_("/Image/Duplicate"), "<control>D",
      image_duplicate_cmd_callback, 0 },
    NULL,
    "image/duplicate.html", NULL },

  SEPARATOR ("/Image/---"),

  /*  <Image>/Layers  */

  { { N_("/Layers/Layers, Channels & Paths..."), "<control>L",
      dialogs_create_lc_cmd_callback, 0 },
    NULL,
    "dialogs/layers_and_channels.html", NULL },

  SEPARATOR ("/Layers/---"),

  { { N_("/Layers/Layer to Imagesize"), NULL,
      layers_resize_to_image_cmd_callback, 0 },
    NULL,
    "layers/layer_to_image_size.html", NULL },

  /*  <Image>/Layers/Stack  */

  { { N_("/Layers/Stack/Previous Layer"), "Prior",
      layers_previous_cmd_callback, 0 },
    NULL,
    "layers/stack/stack.html#previous_layer", NULL },
  { { N_("/Layers/Stack/Next Layer"), "Next",
      layers_next_cmd_callback, 0 },
    NULL,
    "layers/stack/stack.html#next_layer", NULL },
  { { N_("/Layers/Stack/Raise Layer"), "<shift>Prior",
      layers_raise_cmd_callback, 0 },
    NULL,
    "layers/stack/stack.html#raise_layer", NULL },
  { { N_("/Layers/Stack/Lower Layer"), "<shift>Next",
      layers_lower_cmd_callback, 0 },
    NULL,
    "layers/stack/stack.html#lower_layer", NULL },
  { { N_("/Layers/Stack/Layer to Top"), "<control>Prior",
      layers_raise_to_top_cmd_callback, 0 },
    NULL,
    "layers/stack/stack.html#layer_to_top", NULL },
  { { N_("/Layers/Stack/Layer to Bottom"), "<control>Next",
      layers_lower_to_bottom_cmd_callback, 0 },
    NULL,
    "layers/stack/stack.html#layer_to_bottom", NULL },

  SEPARATOR ("/Layers/Stack/---"),

  /*  <Image>/Layers/Rotate  */

  BRANCH (N_("/Layers/Rotate")),

  SEPARATOR ("/Layers/---"),

  { { N_("/Layers/Anchor Layer"), "<control>H",
      layers_anchor_cmd_callback, 0 },
    NULL,
    "layers/anchor_layer.html", NULL },
  { { N_("/Layers/Merge Visible Layers..."), "<control>M",
      layers_merge_layers_cmd_callback, 0 },
    NULL,
    "layers/dialogs/merge_visible_layers.html", NULL },
  { { N_("/Layers/Flatten Image"), NULL,
      layers_flatten_image_cmd_callback, 0 },
    NULL,
    "layers/flatten_image.html", NULL },

  SEPARATOR ("/Layers/---"),

  { { N_("/Layers/Mask to Selection"), NULL,
      layers_mask_select_cmd_callback, 0 },
    NULL,
    "layers/mask_to_selection.html", NULL },

  SEPARATOR ("/Layers/---"),

  { { N_("/Layers/Add Alpha Channel"), NULL,
      layers_add_alpha_channel_cmd_callback, 0 },
    NULL,
    "layers/add_alpha_channel.html", NULL },
  { { N_("/Layers/Alpha to Selection"), NULL,
      layers_alpha_select_cmd_callback, 0 },
    NULL,
    "layers/alpha_to_selection.html", NULL },

  SEPARATOR ("/Layers/---"),

  /*  <Image>/Tools  */

  { { N_("/Tools/Toolbox"), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:toolbox",
    "toolbox/toolbox.html", NULL },
  { { N_("/Tools/Default Colors"), "D",
      tools_default_colors_cmd_callback, 0 },
    NULL,
    "toolbox/toolbox.html#default_colors", NULL },
  { { N_("/Tools/Swap Colors"), "X",
      tools_swap_colors_cmd_callback, 0 },
    NULL,
    "toolbox/toolbox.html#swap_colors", NULL },
  { { N_("/Tools/Swap Contexts"), "<shift>X",
      tools_swap_contexts_cmd_callback, 0 },
    NULL,
    "toolbox/toolbox.html#swap_colors", NULL },

  SEPARATOR ("/Tools/---"),

  /*  <Image>/Dialogs  */

  { { N_("/Dialogs/Layers, Channels & Paths..."), "<control>L",
      dialogs_create_lc_cmd_callback, 0 },
    NULL,
    "dialogs/layers_and_channels.html", NULL },
  { { N_("/Dialogs/Tool Options..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:tool-options-dialog",
    "dialogs/tool_options.html", NULL },

  SEPARATOR ("/Dialogs/---"),

  { { N_("/Dialogs/Brushes..."), "<control><shift>B",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:brush-grid",
    "dialogs/brush_selection.html", NULL },
  { { N_("/Dialogs/Patterns..."), "<control><shift>P",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:pattern-grid",
    "dialogs/pattern_selection.html", NULL },
  { { N_("/Dialogs/Gradients..."), "<control>G",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:gradient-list",
    "dialogs/gradient_selection.html", NULL },
  { { N_("/Dialogs/Palettes..."), "<control>P",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:palette-list",
    "dialogs/palette_selection.html", NULL },
  { { N_("/Dialogs/Indexed Palette..."), NULL,
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:indexed-palette",
    "dialogs/indexed_palette.html", NULL },
  { { N_("/Dialogs/Buffers..."), "<control>P",
      dialogs_create_dockable_cmd_callback, 0 },
    "gimp:buffer-list",
    NULL, NULL },

  SEPARATOR ("/Dialogs/---"),

  { { N_("/Dialogs/Input Devices..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:input-devices-dialog",
    "dialogs/input_devices.html", NULL },
  { { N_("/Dialogs/Device Status..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:device-status-dialog",
    "dialogs/device_status.html", NULL },

  SEPARATOR ("/Dialogs/---"),

  { { N_("/Dialogs/Document Index..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:document-index-dialog",
    "dialogs/document_index.html", NULL },
  { { N_("/Dialogs/Error Console..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:error-console-dialog",
    "dialogs/error_console.html", NULL },
#ifdef DISPLAY_FILTERS
  { { N_("/Dialogs/Display Filters..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:display-filters-dialogs",
    "dialogs/display_filters/display_filters.html", NULL },
#endif /* DISPLAY_FILTERS */
  { { N_("/Dialogs/Undo History..."), NULL,
      dialogs_create_toplevel_cmd_callback, 0 },
    "gimp:undo-history-dialog",
    "dialogs/undo_history.html", NULL },

  SEPARATOR ("/---"),

  /*  <Image>/Filters  */

  { { N_("/Filters/Repeat Last"), "<alt>F",
      filters_repeat_cmd_callback, (guint) FALSE },
    NULL,
    "filters/repeat_last.html", NULL },
  { { N_("/Filters/Re-Show Last"), "<alt><shift>F",
      filters_repeat_cmd_callback, (guint) TRUE },
    NULL,
    "filters/reshow_last.html", NULL },

  SEPARATOR ("/Filters/---"),

  BRANCH (N_("/Filters/Blur")),
  BRANCH (N_("/Filters/Colors")),
  BRANCH (N_("/Filters/Noise")),
  BRANCH (N_("/Filters/Edge-Detect")),
  BRANCH (N_("/Filters/Enhance")),
  BRANCH (N_("/Filters/Generic")),

  SEPARATOR ("/Filters/---"),

  BRANCH (N_("/Filters/Glass Effects")),
  BRANCH (N_("/Filters/Light Effects")),
  BRANCH (N_("/Filters/Distorts")),
  BRANCH (N_("/Filters/Artistic")),
  BRANCH (N_("/Filters/Map")),
  BRANCH (N_("/Filters/Render")),
  BRANCH (N_("/Filters/Text")),
  BRANCH (N_("/Filters/Web")),

  SEPARATOR ("/Filters/---INSERT"),

  BRANCH (N_("/Filters/Animation")),
  BRANCH (N_("/Filters/Combine")),

  SEPARATOR ("/Filters/---"),

  BRANCH (N_("/Filters/Toys"))
};
static guint n_image_entries = (sizeof (image_entries) /
				sizeof (image_entries[0]));
static GtkItemFactory *image_factory = NULL;


/*****  <Load>  *****/

static GimpItemFactoryEntry load_entries[] =
{
  { { N_("/Automatic"), NULL,
      file_open_by_extension_cmd_callback, 0 },
    NULL,
    "open_by_extension.html", NULL },

  SEPARATOR ("/---")
};
static guint n_load_entries = (sizeof (load_entries) /
			       sizeof (load_entries[0]));
static GtkItemFactory *load_factory = NULL;

  
/*****  <Save>  *****/

static GimpItemFactoryEntry save_entries[] =
{
  { { N_("/By Extension"), NULL,
      file_save_by_extension_cmd_callback, 0 },
    NULL,
    "save_by_extension.html", NULL },

  SEPARATOR ("/---")
};
static guint n_save_entries = (sizeof (save_entries) /
			       sizeof (save_entries[0]));
static GtkItemFactory *save_factory = NULL;


/*****  <Layers>  *****/

static GimpItemFactoryEntry layers_entries[] =
{
  { { N_("/New Layer..."), "<control>N",
      layers_new_cmd_callback, 0 },
    NULL,
    "dialogs/new_layer.html", NULL },

  /*  <Layers>/Stack  */

  { { N_("/Stack/Raise Layer"), "<control>F",
      layers_raise_cmd_callback, 0 },
    NULL,
    "stack/stack.html#raise_layer", NULL },
  { { N_("/Stack/Lower Layer"), "<control>B",
      layers_lower_cmd_callback, 0 },
    NULL,
    "stack/stack.html#lower_layer", NULL },
  { { N_("/Stack/Layer to Top"), "<shift><control>F",
      layers_raise_to_top_cmd_callback, 0 },
    NULL,
    "stack/stack.html#later_to_top", NULL },
  { { N_("/Stack/Layer to Bottom"), "<shift><control>B",
      layers_lower_to_bottom_cmd_callback, 0 },
    NULL,
    "stack/stack.html#layer_to_bottom", NULL },

  { { N_("/Duplicate Layer"), "<control>C",
      layers_duplicate_cmd_callback, 0 },
    NULL,
    "duplicate_layer.html", NULL },
  { { N_("/Anchor Layer"), "<control>H",
      layers_anchor_cmd_callback, 0 },
    NULL,
    "anchor_layer.html", NULL },
  { { N_("/Delete Layer"), "<control>X",
      layers_delete_cmd_callback, 0 },
    NULL,
    "delete_layer.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Layer Boundary Size..."), "<control>R",
      layers_resize_cmd_callback, 0 },
    NULL,
    "dialogs/layer_boundary_size.html", NULL },
  { { N_("/Layer to Imagesize"), NULL,
      layers_resize_to_image_cmd_callback, 0 },
    NULL,
    "layer_to_image_size.html", NULL },
  { { N_("/Scale Layer..."), "<control>S",
      layers_scale_cmd_callback, 0 },
    NULL,
    "dialogs/scale_layer.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Merge Visible Layers..."), "<control>M",
      layers_merge_layers_cmd_callback, 0 },
    NULL,
    "dialogs/merge_visible_layers.html", NULL },
  { { N_("/Merge Down"), "<control><shift>M",
      layers_merge_down_cmd_callback, 0 },
    NULL,
    "merge_down.html", NULL },
  { { N_("/Flatten Image"), NULL,
      layers_flatten_image_cmd_callback, 0 },
    NULL,
    "flatten_image.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Add Layer Mask..."), NULL,
      layers_add_layer_mask_cmd_callback, 0 },
    NULL,
    "dialogs/add_layer_mask.html", NULL },
  { { N_("/Apply Layer Mask"), NULL,
      layers_apply_layer_mask_cmd_callback, 0 },
    NULL,
    "apply_mask.html", NULL },
  { { N_("/Delete Layer Mask"), NULL,
      layers_delete_layer_mask_cmd_callback, 0 },
    NULL,
    "delete_mask.html", NULL },
  { { N_("/Mask to Selection"), NULL,
      layers_mask_select_cmd_callback, 0 },
    NULL,
    "mask_to_selection.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Add Alpha Channel"), NULL,
      layers_add_alpha_channel_cmd_callback, 0 },
    NULL,
    "add_alpha_channel.html", NULL },
  { { N_("/Alpha to Selection"), NULL,
      layers_alpha_select_cmd_callback, 0 },
    NULL,
    "alpha_to_selection.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Edit Layer Attributes..."), NULL,
      layers_edit_attributes_cmd_callback, 0 },
    NULL,
    "dialogs/edit_layer_attributes.html", NULL }
};
static guint n_layers_entries = (sizeof (layers_entries) /
				 sizeof (layers_entries[0]));
static GtkItemFactory *layers_factory = NULL;


/*****  <Channels>  *****/

static GimpItemFactoryEntry channels_entries[] =
{
  { { N_("/New Channel..."), "<control>N",
      channels_new_channel_cmd_callback, 0 },
    NULL,
    "dialogs/new_channel.html", NULL },
  { { N_("/Raise Channel"), "<control>F",
      channels_raise_channel_cmd_callback, 0 },
    NULL,
    "raise_channel.html", NULL },
  { { N_("/Lower Channel"), "<control>B",
      channels_lower_channel_cmd_callback, 0 },
    NULL,
    "lower_channel.html", NULL },
  { { N_("/Duplicate Channel"), "<control>C",
      channels_duplicate_channel_cmd_callback, 0 },
    NULL,
    "duplicate_channel.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Channel to Selection"), "<control>S",
      channels_channel_to_sel_cmd_callback, 0 },
    NULL,
    "channel_to_selection.html", NULL },
  { { N_("/Add to Selection"), NULL,
      channels_add_channel_to_sel_cmd_callback, 0 },
    NULL,
    "channel_to_selection.html#add", NULL },
  { { N_("/Subtract from Selection"), NULL,
      channels_sub_channel_from_sel_cmd_callback, 0 },
    NULL,
    "channel_to_selection.html#subtract", NULL },
  { { N_("/Intersect with Selection"), NULL,
      channels_intersect_channel_with_sel_cmd_callback, 0 },
    NULL,
    "channel_to_selection.html#intersect", NULL },

  SEPARATOR ("/---"),

  { { N_("/Delete Channel"), "<control>X",
      channels_delete_channel_cmd_callback, 0 },
    NULL,
    "delete_channel.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Edit Channel Attributes..."), NULL,
      channels_edit_channel_attributes_cmd_callback, 0 },
    NULL,
    "dialogs/edit_channel_attributes.html", NULL }
};
static guint n_channels_entries = (sizeof (channels_entries) /
				   sizeof (channels_entries[0]));
static GtkItemFactory *channels_factory = NULL;


/*****  <Paths>  *****/

static GimpItemFactoryEntry paths_entries[] =
{
  { { N_("/New Path"), "<control>N",
      paths_dialog_new_path_callback, 0 },
    NULL,
    "new_path.html", NULL },
  { { N_("/Duplicate Path"), "<control>U",
      paths_dialog_dup_path_callback, 0 },
    NULL,
    "duplicate_path.html", NULL },
  { { N_("/Path to Selection"), "<control>S",
      paths_dialog_path_to_sel_callback, 0 },
    NULL,
    "path_to_selection.html", NULL },
  { { N_("/Selection to Path"), "<control>P",
      paths_dialog_sel_to_path_callback, 0 },
    NULL,
    "filters/sel2path.html", NULL },
  { { N_("/Stroke Path"), "<control>T",
      paths_dialog_stroke_path_callback, 0 },
    NULL,
    "stroke_path.html", NULL },
  { { N_("/Delete Path"), "<control>X",
      paths_dialog_delete_path_callback, 0 },
    NULL,
    "delete_path.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Copy Path"), "<control>C",
      paths_dialog_copy_path_callback, 0 },
    NULL,
    "copy_path.html", NULL },
  { { N_("/Paste Path"), "<control>V",
      paths_dialog_paste_path_callback, 0 },
    NULL,
    "paste_path.html", NULL },
  { { N_("/Import Path..."), "<control>I",
      paths_dialog_import_path_callback, 0 },
    NULL,
    "dialogs/import_path.html", NULL },
  { { N_("/Export Path..."), "<control>E",
      paths_dialog_export_path_callback, 0 },
    NULL,
    "dialogs/export_path.html", NULL },

  SEPARATOR ("/---"),

  { { N_("/Edit Path Attributes..."), NULL,
      paths_dialog_edit_path_attributes_callback, 0 },
    NULL,
    "dialogs/edit_path_attributes.html", NULL }
};
static guint n_paths_entries = (sizeof (paths_entries) /
				sizeof (paths_entries[0]));
static GtkItemFactory *paths_factory = NULL;


/*****  <Dialogs>  *****/

static GimpItemFactoryEntry dialogs_entries[] =
{
  { { N_("/Select Tab"), NULL, NULL, 0 },
    NULL,
    NULL, NULL },

  { { N_("/Add Tab/Layer List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:layer-list",
    NULL, NULL },
  { { N_("/Add Tab/Channel List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:channel-list",
    NULL, NULL },
  { { N_("/Add Tab/Path List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:path-list",
    NULL, NULL },
  { { N_("/Add Tab/Indexed Palette..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:indexed-palette",
    NULL, NULL },

  SEPARATOR ("/Add Tab/---"),

  { { N_("/Add Tab/Brush List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:brush-list",
    NULL, NULL },
  { { N_("/Add Tab/Brush Grid..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:brush-grid",
    NULL, NULL },

  SEPARATOR ("/Add Tab/---"),

  { { N_("/Add Tab/Pattern List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:pattern-list",
    NULL, NULL },
  { { N_("/Add Tab/Pattern Grid..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:pattern-grid",
    NULL, NULL },

  SEPARATOR ("/Add Tab/---"),

  { { N_("/Add Tab/Gradient List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:gradient-list",
    NULL, NULL },
  { { N_("/Add Tab/Gradient Grid..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:gradient-grid",
    NULL, NULL },

  SEPARATOR ("/Add Tab/---"),

  { { N_("/Add Tab/Palette List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:palette-list",
    NULL, NULL },
  { { N_("/Add Tab/Palette Grid..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:palette-grid",
    NULL, NULL },

  SEPARATOR ("/Add Tab/---"),

  { { N_("/Add Tab/Tool List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:tool-list",
    NULL, NULL },
  { { N_("/Add Tab/Tool Grid..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:tool-grid",
    NULL, NULL },

  SEPARATOR ("/Add Tab/---"),

  { { N_("/Add Tab/Image List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:image-list",
    NULL, NULL },
  { { N_("/Add Tab/Image Grid..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:image-grid",
    NULL, NULL },

  SEPARATOR ("/Add Tab/---"),

  { { N_("/Add Tab/Buffer List..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:buffer-list",
    NULL, NULL },
  { { N_("/Add Tab/Buffer Grid..."), NULL,
      dialogs_add_tab_cmd_callback, 0 },
    "gimp:buffer-grid",
    NULL, NULL },

  { { N_("/Remove Tab"), NULL,
      dialogs_remove_tab_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  SEPARATOR ("/---"),

  { { N_("/Show Image Menu"), NULL,
      dialogs_toggle_image_menu_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    NULL, NULL },
  { { N_("/Auto Follow Active Image"), NULL,
      dialogs_toggle_auto_cmd_callback, 0, "<ToggleItem>" },
    NULL,
    NULL, NULL }

};
static guint n_dialogs_entries = (sizeof (dialogs_entries) /
				  sizeof (dialogs_entries[0]));
static GtkItemFactory *dialogs_factory = NULL;


/*****  <Brushes>  *****/

static GimpItemFactoryEntry brushes_entries[] =
{
  { { N_("/New Brush"), NULL,
      data_new_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Duplicate Brush"), NULL,
      data_duplicate_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Edit Brush..."), NULL,
      data_edit_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Delete Brush..."), NULL,
      data_delete_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  SEPARATOR ("/---"),

  { { N_("/Refresh Brushes"), NULL,
      data_refresh_data_cmd_callback, 0 },
    NULL,
    NULL, NULL }
};
static guint n_brushes_entries = (sizeof (brushes_entries) /
				  sizeof (brushes_entries[0]));
static GtkItemFactory *brushes_factory = NULL;


/*****  <Patterns>  *****/

static GimpItemFactoryEntry patterns_entries[] =
{
  { { N_("/New Pattern"), NULL,
      data_new_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Duplicate Pattern"), NULL,
      data_duplicate_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Edit Pattern..."), NULL,
      data_edit_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Delete Pattern..."), NULL,
      data_delete_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  SEPARATOR ("/---"),

  { { N_("/Refresh Patterns"), NULL,
      data_refresh_data_cmd_callback, 0 },
    NULL,
    NULL, NULL }
};
static guint n_patterns_entries = (sizeof (patterns_entries) /
				   sizeof (patterns_entries[0]));
static GtkItemFactory *patterns_factory = NULL;


/*****  <Gradients>  *****/

static GimpItemFactoryEntry gradients_entries[] =
{
  { { N_("/New Gradient"), NULL,
      data_new_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Duplicate Gradient"), NULL,
      data_duplicate_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Edit Gradient..."), NULL,
      data_edit_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Delete Gradient..."), NULL,
      data_delete_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  SEPARATOR ("/---"),

  { { N_("/Refresh Gradients"), NULL,
      data_refresh_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  SEPARATOR ("/---"),

  { { N_("/Save as POV-Ray..."), NULL,
      gradients_save_as_pov_ray_cmd_callback, 0 },
    NULL,
    NULL, NULL }
};
static guint n_gradients_entries = (sizeof (gradients_entries) /
				    sizeof (gradients_entries[0]));
static GtkItemFactory *gradients_factory = NULL;


/*****  <Palettes>  *****/

static GimpItemFactoryEntry palettes_entries[] =
{
  { { N_("/New Palette"), NULL,
      data_new_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Duplicate Palette"), NULL,
      data_duplicate_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Edit Palette..."), NULL,
      data_edit_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Delete Palette..."), NULL,
      data_delete_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  SEPARATOR ("/---"),

  { { N_("/Refresh Palettes"), NULL,
      data_refresh_data_cmd_callback, 0 },
    NULL,
    NULL, NULL },

  SEPARATOR ("/---"),

  { { N_("/Import Palette..."), NULL,
      palettes_import_palette_cmd_callback, 0 },
    NULL,
    NULL, NULL },
  { { N_("/Merge Palettes..."), NULL,
      palettes_merge_palettes_cmd_callback, 0 },
    NULL,
    NULL, NULL }
};
static guint n_palettes_entries = (sizeof (palettes_entries) /
				   sizeof (palettes_entries[0]));
static GtkItemFactory *palettes_factory = NULL;


static gboolean menus_initialized = FALSE;


GtkItemFactory *
menus_get_toolbox_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return toolbox_factory;
}

GtkItemFactory *
menus_get_image_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return image_factory;
}

GtkItemFactory *
menus_get_load_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return load_factory;
}

GtkItemFactory *
menus_get_save_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return save_factory;
}

GtkItemFactory *
menus_get_layers_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return layers_factory;
}

GtkItemFactory *
menus_get_channels_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return channels_factory;
}

GtkItemFactory *
menus_get_paths_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return paths_factory;
}

GtkItemFactory *
menus_get_dialogs_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return dialogs_factory;
}

GtkItemFactory *
menus_get_brushes_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return brushes_factory;
}

GtkItemFactory *
menus_get_patterns_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return patterns_factory;
}

GtkItemFactory *
menus_get_gradients_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return gradients_factory;
}

GtkItemFactory *
menus_get_palettes_factory (void)
{
  if (! menus_initialized)
    menus_init ();

  return palettes_factory;
}


void
menus_create_item_from_full_path (GimpItemFactoryEntry *entry,
				  gchar                *domain_name,
				  gpointer              callback_data)
{
  GtkItemFactory *item_factory;
  gchar          *path;

  g_return_if_fail (entry != NULL);

  if (!menus_initialized)
    menus_init ();

  path = entry->entry.path;

  if (!path)
    return;

  item_factory = gtk_item_factory_from_path (path);

  if (!item_factory)
    {
      g_warning ("entry refers to unknown item factory: \"%s\"", path);
      return;
    }

  g_object_set_data (G_OBJECT (item_factory), "textdomain", domain_name);

  while (*path != '>')
    path++;
  path++;

  entry->entry.path = path;

  menus_create_item (item_factory, entry, callback_data, 2, TRUE, FALSE);
}

static void
menus_create_branches (GtkItemFactory       *item_factory,
		       GimpItemFactoryEntry *entry)
{
  GString *tearoff_path;
  gint     factory_length;
  gchar   *p;
  gchar   *path;

  if (! entry->entry.path)
    return;

  tearoff_path = g_string_new ("");

  path = entry->entry.path;
  p = strchr (path, '/');
  factory_length = p - path;

  /*  skip the first slash  */
  if (p)
    p = strchr (p + 1, '/');

  while (p)
    {
      g_string_assign (tearoff_path, path + factory_length);
      g_string_truncate (tearoff_path, p - path - factory_length);

      if (!gtk_item_factory_get_widget (item_factory, tearoff_path->str))
	{
	  GimpItemFactoryEntry branch_entry =
	  {
	    { NULL, NULL, NULL, 0, "<Branch>" },
	    NULL,
	    NULL
	  };

	  branch_entry.entry.path = tearoff_path->str;
	  g_object_set_data (G_OBJECT (item_factory), "complete", path);
	  menus_create_item (item_factory, &branch_entry, NULL, 2, TRUE, FALSE);
	  g_object_set_data (G_OBJECT (item_factory), "complete", NULL);
	}

      g_string_append (tearoff_path, "/tearoff1");

      if (! gtk_item_factory_get_widget (item_factory, tearoff_path->str))
	{
	  GimpItemFactoryEntry tearoff_entry =
	  {
	    { NULL, NULL, menus_tearoff_cmd_callback, 0, "<Tearoff>" },
	    NULL,
	    NULL, NULL
	  };

	  tearoff_entry.entry.path = tearoff_path->str;

	  menus_create_item (item_factory, &tearoff_entry, NULL, 2, TRUE, FALSE);
	}

      p = strchr (p + 1, '/');
    }

  g_string_free (tearoff_path, TRUE);
}

static void
menus_filters_subdirs_to_top (GtkMenu *menu)
{
  GtkMenuItem *menu_item;
  GList       *list;
  gboolean     submenus_passed = FALSE;
  gint         pos;
  gint         items;

  pos   = 1;
  items = 0;

  for (list = GTK_MENU_SHELL (menu)->children; list; list = g_list_next (list))
    {
      menu_item = GTK_MENU_ITEM (list->data);
      items++;
      
      if (menu_item->submenu)
	{
	  if (submenus_passed)
	    {
	      menus_filters_subdirs_to_top (GTK_MENU (menu_item->submenu));
	      gtk_menu_reorder_child (menu, GTK_WIDGET (menu_item), pos++);
	    }
	}
      else
	{
	  submenus_passed = TRUE;
	}
    }

  if (pos > 1 && items > pos)
    {
      GtkWidget *separator;

      separator = gtk_menu_item_new ();
      gtk_menu_shell_insert (GTK_MENU_SHELL (menu), separator, pos);
      gtk_widget_show (separator);
    }
}

void
menus_reorder_plugins (void)
{
  static gchar *rotate_plugins[] = { "90 degrees",
				     "180 degrees",
                                     "270 degrees" };
  static gint n_rotate_plugins = (sizeof (rotate_plugins) /
				  sizeof (rotate_plugins[0]));

  static gchar *image_file_entries[] = { "---moved",
					 "Close",
					 "Quit" };
  static gint n_image_file_entries = (sizeof (image_file_entries) /
				      sizeof (image_file_entries[0]));

  static gchar *reorder_submenus[] = { "<Image>/Video",
				       "<Image>/Script-Fu" };
  static gint n_reorder_submenus = (sizeof (reorder_submenus) /
				    sizeof (reorder_submenus[0]));

  static gchar *reorder_subsubmenus[] = { "<Image>/Filters",
					  "<Toolbox>/Xtns" };
  static gint n_reorder_subsubmenus = (sizeof (reorder_subsubmenus) /
				       sizeof (reorder_subsubmenus[0]));

  GtkItemFactory *item_factory;
  GtkWidget *menu_item;
  GtkWidget *menu;
  GList     *list;
  gchar     *path;
  gint      i, pos;

  /*  Move all menu items under "<Toolbox>/Xtns" which are not submenus or
   *  separators to the top of the menu
   */
  pos = 1;
  menu_item = gtk_item_factory_get_widget (toolbox_factory,
					   "/Xtns/Module Browser...");
  if (menu_item && menu_item->parent && GTK_IS_MENU (menu_item->parent))
    {
      menu = menu_item->parent;

      for (list = g_list_nth (GTK_MENU_SHELL (menu)->children, pos); list;
	   list = g_list_next (list))
	{
	  menu_item = GTK_WIDGET (list->data);

	  if (! GTK_MENU_ITEM (menu_item)->submenu &&
	      GTK_IS_LABEL (GTK_BIN (menu_item)->child))
	    {
	      gtk_menu_reorder_child (GTK_MENU (menu_item->parent),
				      menu_item, pos);
	      list = g_list_nth (GTK_MENU_SHELL (menu)->children, pos);
	      pos++;
	    }
	}
    }

  /*  Move all menu items under "<Image>/Filters" which are not submenus or
   *  separators to the top of the menu
   */
  pos = 3;
  menu_item = gtk_item_factory_get_widget (image_factory,
					   "/Filters/Filter all Layers...");
  if (menu_item && menu_item->parent && GTK_IS_MENU (menu_item->parent))
    {
      menu = menu_item->parent;

      for (list = g_list_nth (GTK_MENU_SHELL (menu)->children, pos); list;
	   list = g_list_next (list))
	{
	  menu_item = GTK_WIDGET (list->data);

	  if (! GTK_MENU_ITEM (menu_item)->submenu &&
	      GTK_IS_LABEL (GTK_BIN (menu_item)->child))
	    {
	      gtk_menu_reorder_child (GTK_MENU (menu_item->parent),
				      menu_item, pos);
	      list = g_list_nth (GTK_MENU_SHELL (menu)->children, pos);
	      pos++;
	    }
	}
    }

  /*  Reorder Rotate plugin menu entries */
  pos = 2;
  for (i = 0; i < n_rotate_plugins; i++)
    {
      path = g_strconcat ("/Image/Transforms/Rotate/", rotate_plugins[i], NULL);
      menu_item = gtk_item_factory_get_widget (image_factory, path);
      g_free (path);

      if (menu_item && menu_item->parent)
        {
          gtk_menu_reorder_child (GTK_MENU (menu_item->parent), menu_item, pos);
          pos++;
        }
    }

  pos = 2;
  for (i = 0; i < n_rotate_plugins; i++)
    {
      path = g_strconcat ("/Layers/Rotate/", rotate_plugins[i], NULL);
      menu_item = gtk_item_factory_get_widget (image_factory, path);
      g_free (path);

      if (menu_item && menu_item->parent)
        {
          gtk_menu_reorder_child (GTK_MENU (menu_item->parent), menu_item, pos);
          pos++;
        }
    }

  /*  Reorder "<Image>/File"  */
  for (i = 0; i < n_image_file_entries; i++)
    {
      path = g_strconcat ("/File/", image_file_entries[i], NULL);
      menu_item = gtk_item_factory_get_widget (image_factory, path);
      g_free (path);

      if (menu_item && menu_item->parent)
	gtk_menu_reorder_child (GTK_MENU (menu_item->parent), menu_item, -1);
    }

  /*  Reorder menus where plugins registered submenus  */
  for (i = 0; i < n_reorder_submenus; i++)
    {
      item_factory = gtk_item_factory_from_path (reorder_submenus[i]);
      menu = gtk_item_factory_get_widget (item_factory,
					  reorder_submenus[i]);

      if (menu && GTK_IS_MENU (menu))
	{
	  menus_filters_subdirs_to_top (GTK_MENU (menu));
	}
    }

  for (i = 0; i < n_reorder_subsubmenus; i++)
    {
      item_factory = gtk_item_factory_from_path (reorder_subsubmenus[i]);
      menu = gtk_item_factory_get_widget (item_factory,
					  reorder_subsubmenus[i]);

      if (menu && GTK_IS_MENU (menu))
	{
	  for (list = GTK_MENU_SHELL (menu)->children; list;
	       list = g_list_next (list))
	    {
	      GtkMenuItem *menu_item;

	      menu_item = GTK_MENU_ITEM (list->data);

	      if (menu_item->submenu)
		menus_filters_subdirs_to_top (GTK_MENU (menu_item->submenu));
	    }
	}
    }

  /*  Move all submenus which registered after "<Image>/Filters/Toys"
   *  before the separator after "<Image>/Filters/Web"
   */
  menu_item = gtk_item_factory_get_widget (image_factory,
					   "/Filters/---INSERT");

  if (menu_item && menu_item->parent && GTK_IS_MENU (menu_item->parent))
    {
      menu = menu_item->parent;
      pos = g_list_index (GTK_MENU_SHELL (menu)->children, menu_item);

      menu_item = gtk_item_factory_get_widget (image_factory,
					       "/Filters/Toys");

      if (menu_item && GTK_IS_MENU (menu_item))
	{
	  GList *list;
	  gint index = 1;

	  for (list = GTK_MENU_SHELL (menu)->children; list;
	       list = g_list_next (list))
	    {
	      if (GTK_MENU_ITEM (list->data)->submenu == menu_item)
		break;

	      index++;
	    }

	  while ((menu_item = g_list_nth_data (GTK_MENU_SHELL (menu)->children,
					       index)))
	    {
	      gtk_menu_reorder_child (GTK_MENU (menu), menu_item, pos);

	      pos++;
	      index++;
	    }
	}
    }
}

static void
menus_tools_create (GimpToolInfo *tool_info)
{
  GimpItemFactoryEntry entry;

  if (tool_info->menu_path == NULL)
    return;

  entry.entry.path            = tool_info->menu_path;
  entry.entry.accelerator     = tool_info->menu_accel;
  entry.entry.callback        = tools_select_cmd_callback;
  entry.entry.callback_action = tool_info->tool_type;
  entry.entry.item_type       = NULL;
  entry.quark_string          = NULL;
  entry.help_page             = tool_info->help_data;
  entry.description           = NULL;

  menus_create_item (image_factory, &entry, (gpointer) tool_info, 2, TRUE, FALSE);
}

void
menus_set_sensitive (gchar    *path,
		     gboolean  sensitive)
{
  GtkItemFactory *ifactory;
  GtkWidget      *widget = NULL;

  if (! path)
    return;

  if (!menus_initialized)
    menus_init ();

  ifactory = gtk_item_factory_from_path (path);

  if (ifactory)
    {
      widget = gtk_item_factory_get_widget (ifactory, path);

      if (widget)
	gtk_widget_set_sensitive (widget, sensitive);
    }

  if ((!ifactory || !widget) && ! strstr (path, "Script-Fu"))
    g_warning ("Unable to set sensitivity for menu which doesn't exist:\n%s",
	       path);
}

void
menus_set_state (gchar    *path,
		 gboolean  state)
{
  GtkItemFactory *ifactory;
  GtkWidget      *widget = NULL;

  if (!menus_initialized)
    menus_init ();

  ifactory = gtk_item_factory_from_path (path);

  if (ifactory)
    {
      widget = gtk_item_factory_get_widget (ifactory, path);

      if (widget && GTK_IS_CHECK_MENU_ITEM (widget))
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), state);
      else
	widget = NULL;
    }

  if ((!ifactory || !widget) && ! strstr (path, "Script-Fu"))
    g_warning ("Unable to set state for menu which doesn't exist:\n%s",
	       path);
}

void
menus_destroy (gchar *path)
{
  if (!menus_initialized)
    menus_init ();

  gtk_item_factories_path_delete (NULL, path);
}

void
menus_quit (void)
{
  gchar *filename;

  filename = gimp_personal_rc_file ("menurc");
  gtk_item_factory_dump_rc (filename, NULL, TRUE);
  g_free (filename);

  if (menus_initialized)
    {
      g_object_unref (G_OBJECT (toolbox_factory));
      g_object_unref (G_OBJECT (image_factory));
      g_object_unref (G_OBJECT (load_factory));
      g_object_unref (G_OBJECT (save_factory));
      g_object_unref (G_OBJECT (layers_factory));
      g_object_unref (G_OBJECT (channels_factory));
      g_object_unref (G_OBJECT (paths_factory));
      g_object_unref (G_OBJECT (dialogs_factory));
      g_object_unref (G_OBJECT (brushes_factory));
      g_object_unref (G_OBJECT (patterns_factory));
      g_object_unref (G_OBJECT (gradients_factory));
      g_object_unref (G_OBJECT (palettes_factory));
    }
}

static void
menus_last_opened_update_labels (void)
{
  GSList    *filename_slist;
  GString   *entry_filename;
  GString   *path;
  GtkWidget *widget;
  gint	     i;
  guint      num_entries;

  entry_filename = g_string_new ("");
  path = g_string_new ("");

  filename_slist = last_opened_raw_filenames;
  num_entries = g_slist_length (last_opened_raw_filenames);

  for (i = 1; i <= num_entries; i++)
    {
      g_string_printf (entry_filename, "%d. %s", i,
                       g_basename (((GString *) filename_slist->data)->str));

      g_string_printf (path, "/File/MRU%02d", i);

      widget = gtk_item_factory_get_widget (toolbox_factory, path->str);
      if (widget)
	{
	  gtk_widget_show (widget);

	  gtk_label_set_text (GTK_LABEL (GTK_BIN (widget)->child),
			      entry_filename->str);
	  gimp_help_set_help_data (widget,
				   ((GString *) filename_slist->data)->str,
				   NULL);
	}

      filename_slist = filename_slist->next;
    }

  g_string_free (entry_filename, TRUE);
  g_string_free (path, TRUE);
}

void
menus_last_opened_add (const gchar *filename)
{
  GString   *raw_filename;
  GSList    *list;
  GtkWidget *menu_item;
  guint      num_entries;

  g_return_if_fail (filename != NULL);

  /*  see if we've already got the filename on the list  */
  for (list = last_opened_raw_filenames; list; list = g_slist_next (list))
    {
      raw_filename = (GString *) list->data;

      if (strcmp (raw_filename->str, filename) == 0)
	{
	  /*  move the entry to the top  */

	  last_opened_raw_filenames =
	    g_slist_remove_link (last_opened_raw_filenames, list);

	  last_opened_raw_filenames =
	    g_slist_concat (list, last_opened_raw_filenames);

	  menus_last_opened_update_labels ();

	  return;
	}
    }

  num_entries = g_slist_length (last_opened_raw_filenames);

  if (num_entries == gimprc.last_opened_size)
    {
      GSList *oldest;

      oldest = g_slist_last (last_opened_raw_filenames);

      if (oldest)
	{
	  g_string_free ((GString *) oldest->data, TRUE);

	  last_opened_raw_filenames = g_slist_remove (last_opened_raw_filenames,
						      oldest);
	}
    }

  raw_filename = g_string_new (filename);
  last_opened_raw_filenames = g_slist_prepend (last_opened_raw_filenames,
					       raw_filename);

  if (num_entries == 0)
    {
      menu_item = gtk_item_factory_get_widget (toolbox_factory, 
					       "/File/---MRU");
      gtk_widget_show (menu_item);
    }

  menus_last_opened_update_labels ();
}

static void
menus_init_mru (void)
{
  GimpItemFactoryEntry *last_opened_entries;
  GtkWidget	       *menu_item;
  gint                  i;

  last_opened_entries = g_new (GimpItemFactoryEntry, gimprc.last_opened_size);

  for (i = 0; i < gimprc.last_opened_size; i++)
    {
      last_opened_entries[i].entry.path = 
	g_strdup_printf ("/File/MRU%02d", i + 1);

      if (i < 9)
	last_opened_entries[i].entry.accelerator =
	  g_strdup_printf ("<control>%d", i + 1);
      else
	last_opened_entries[i].entry.accelerator = NULL;

      last_opened_entries[i].entry.callback        = file_last_opened_cmd_callback;
      last_opened_entries[i].entry.callback_action = i;
      last_opened_entries[i].entry.item_type       = NULL;
      last_opened_entries[i].quark_string          = NULL;
      last_opened_entries[i].help_page             = "file/last_opened.html";
      last_opened_entries[i].description           = NULL;
    }

  menus_create_items (toolbox_factory, gimprc.last_opened_size,
		      last_opened_entries, NULL, 2, TRUE, FALSE);

  for (i = 0; i < gimprc.last_opened_size; i++)
    {
      menu_item =
	gtk_item_factory_get_widget (toolbox_factory,
				     last_opened_entries[i].entry.path);
      gtk_widget_hide (menu_item);
    }

  menu_item = gtk_item_factory_get_widget (toolbox_factory, "/File/---MRU");
  if (menu_item && menu_item->parent)
    gtk_menu_reorder_child (GTK_MENU (menu_item->parent), menu_item, -1);
  gtk_widget_hide (menu_item);

  menu_item = gtk_item_factory_get_widget (toolbox_factory, "/File/Quit");
  if (menu_item && menu_item->parent)
    gtk_menu_reorder_child (GTK_MENU (menu_item->parent), menu_item, -1);

  for (i = 0; i < gimprc.last_opened_size; i++)
    {
      g_free (last_opened_entries[i].entry.path);
      g_free (last_opened_entries[i].entry.accelerator);
    }

  g_free (last_opened_entries);
}

/*  This function gets called while browsing a menu created
 *  by a GtkItemFactory
 */
static gint
menus_item_key_press (GtkWidget   *widget,
		      GdkEventKey *kevent,
		      gpointer     data)
{
  GtkItemFactory *item_factory     = NULL;
  GtkWidget      *active_menu_item = NULL;
  gchar          *factory_path     = NULL;
  gchar          *help_path        = NULL;
  gchar          *help_page        = NULL;

  item_factory     = (GtkItemFactory *) data;
  active_menu_item = GTK_MENU_SHELL (widget)->active_menu_item;

  /*  first, get the help page from the item
   */
  if (active_menu_item)
    {
      help_page = (gchar *) g_object_get_data (G_OBJECT (active_menu_item),
                                               "help_page");
    }

  /*  For any key except F1, continue with the standard
   *  GtkItemFactory callback and assign a new shortcut, but don't
   *  assign a shortcut to the help menu entries...
   */
  if (kevent->keyval != GDK_F1)
    {
      if (help_page &&
	  *help_page &&
	  item_factory == toolbox_factory &&
	  (strcmp (help_page, "help/dialogs/help.html") == 0 ||
	   strcmp (help_page, "help/context_help.html") == 0))
	{
	  gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), 
					"key_press_event");
	  return TRUE;
	}
      else
	{
	  return FALSE;
	}
    }

  /*  ...finally, if F1 was pressed over any menu, show it's help page...
   */
  gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");

  factory_path = (gchar *) g_object_get_data (G_OBJECT (item_factory),
                                              "factory_path");

  if (! help_page ||
      ! *help_page)
    help_page = "index.html";

  if (factory_path && help_page)
    {
      gchar *help_string;
      gchar *at;

      help_page = g_strdup (help_page);

      at = strchr (help_page, '@');  /* HACK: locale subdir */

      if (at)
	{
	  *at = '\0';
	  help_path   = g_strdup (help_page);
	  help_string = g_strdup (at + 1);
	}
      else
	{
	  help_string = g_strdup_printf ("%s/%s", factory_path, help_page);
	}

      gimp_help (help_path, help_string);

      g_free (help_string);
      g_free (help_page);
    }
  else
    {
      gimp_standard_help_func (NULL);
    }

  return TRUE;
}

/*  set up the callback to catch the "F1" key  */
static void
menus_item_realize (GtkWidget *widget,
		    gpointer   data)
{
  if (GTK_IS_MENU_SHELL (widget->parent))
    {
      if (! g_object_get_data (G_OBJECT (widget->parent),
                               "menus_key_press_connected"))
	{
	  g_signal_connect (G_OBJECT (widget->parent), "key_press_event",
                            GTK_SIGNAL_FUNC (menus_item_key_press),
                            data);

	  g_object_set_data (G_OBJECT (widget->parent),
                             "menus_key_press_connected",
                             (gpointer) TRUE);
	}
    }
}

static void
menus_create_item (GtkItemFactory       *item_factory,
		   GimpItemFactoryEntry *entry,
		   gpointer              callback_data,
		   guint                 callback_type,
		   gboolean              create_tearoff,
		   gboolean              static_entry)
{
  GtkWidget *menu_item;

  if (! (strstr (entry->entry.path, "tearoff1")))
    {
      if (! gimprc.disable_tearoff_menus && create_tearoff)
	{
	  menus_create_branches (item_factory, entry);
	}
    }
  else if (gimprc.disable_tearoff_menus || ! create_tearoff)
    {
      return;
    }

  if (entry->quark_string)
    {
      GQuark quark;

      if (static_entry)
	quark = g_quark_from_static_string (entry->quark_string);
      else
	quark = g_quark_from_string (entry->quark_string);

      entry->entry.callback_action = (guint) quark;
    }

  gtk_item_factory_create_item (item_factory,
				(GtkItemFactoryEntry *) entry,
				callback_data,
				callback_type);

  menu_item = gtk_item_factory_get_item (item_factory,
					 ((GtkItemFactoryEntry *) entry)->path);

  if (menu_item)
    {
      gtk_signal_connect_after (GTK_OBJECT (menu_item), "realize",
				GTK_SIGNAL_FUNC (menus_item_realize),
				(gpointer) item_factory);

      g_object_set_data (G_OBJECT (menu_item), "help_page", entry->help_page);
    }
}

static void
menus_create_items (GtkItemFactory       *item_factory,
		    guint                 n_entries,
		    GimpItemFactoryEntry *entries,
		    gpointer              callback_data,
		    guint                 callback_type,
		    gboolean              create_tearoff,
		    gboolean              static_entries)
{
  gint i;

  for (i = 0; i < n_entries; i++)
    {
      menus_create_item (item_factory,
			 entries + i,
			 callback_data,
			 callback_type,
			 create_tearoff,
			 static_entries);
    }
}

static GtkItemFactory *
menus_item_factory_new (GtkType               container_type,
			const gchar          *path,
			const gchar          *factory_path,
			guint                 n_entries,
			GimpItemFactoryEntry *entries,
			gpointer              callback_data,
			gboolean              create_tearoff)
{
  GtkItemFactory *item_factory;

  item_factory = gtk_item_factory_new (container_type, path, NULL);

  gtk_item_factory_set_translate_func (item_factory,
				       menus_menu_translate_func,
				       (gpointer) path,
				       NULL);

  g_object_set_data (G_OBJECT (item_factory), "factory_path", factory_path);

  menus_create_items (item_factory,
		      n_entries,
		      entries,
		      callback_data,
		      2,
		      create_tearoff,
		      TRUE);

  return item_factory;
}

static void
menus_init (void)
{
  GtkWidget    *menu_item;
  gchar        *filename;
  GList        *list;
  GimpToolInfo *tool_info;

  if (menus_initialized)
    return;

  menus_initialized = TRUE;

  toolbox_factory = menus_item_factory_new (GTK_TYPE_MENU_BAR,
					    "<Toolbox>", "toolbox",
					    n_toolbox_entries,
					    toolbox_entries,
					    NULL,
					    TRUE);

  menus_init_mru ();

  image_factory = menus_item_factory_new (GTK_TYPE_MENU,
					  "<Image>", "image",
					  n_image_entries,
					  image_entries,
					  NULL,
					  TRUE);

  load_factory = menus_item_factory_new (GTK_TYPE_MENU,
					 "<Load>", "open",
					 n_load_entries,
					 load_entries,
					 NULL,
					 FALSE);

  save_factory = menus_item_factory_new (GTK_TYPE_MENU,
					 "<Save>", "save",
					 n_save_entries,
					 save_entries,
					 NULL,
					 FALSE);

  layers_factory = menus_item_factory_new (GTK_TYPE_MENU,
					   "<Layers>", "layers",
					   n_layers_entries,
					   layers_entries,
					   NULL,
					   FALSE);

  channels_factory = menus_item_factory_new (GTK_TYPE_MENU,
					     "<Channels>", "channels",
					     n_channels_entries,
					     channels_entries,
					     NULL,
					     FALSE);

  paths_factory = menus_item_factory_new (GTK_TYPE_MENU,
					  "<Paths>", "paths",
					  n_paths_entries,
					  paths_entries,
					  NULL,
					  FALSE);

  dialogs_factory = menus_item_factory_new (GTK_TYPE_MENU,
					    "<Dialogs>", "dialogs",
					    n_dialogs_entries,
					    dialogs_entries,
					    NULL,
					    FALSE);

  brushes_factory = menus_item_factory_new (GTK_TYPE_MENU,
					    "<Brushes>", "brushes",
					    n_brushes_entries,
					    brushes_entries,
					    NULL,
					    FALSE);

  patterns_factory = menus_item_factory_new (GTK_TYPE_MENU,
					     "<Patterns>", "patterns",
					     n_patterns_entries,
					     patterns_entries,
					     NULL,
					     FALSE);

  gradients_factory = menus_item_factory_new (GTK_TYPE_MENU,
					      "<Gradients>", "gradients",
					      n_gradients_entries,
					      gradients_entries,
					      NULL,
					      FALSE);

  palettes_factory = menus_item_factory_new (GTK_TYPE_MENU,
					     "<Palettes>", "palettes",
					     n_palettes_entries,
					     palettes_entries,
					     NULL,
					     FALSE);


  for (list = GIMP_LIST (the_gimp->tool_info_list)->list;
       list;
       list = g_list_next (list))
    {
      menus_tools_create (GIMP_TOOL_INFO (list->data));
    }

  /*  reorder <Image>/Image/Colors  */
  tool_info = tool_manager_get_info_by_type (the_gimp, GIMP_TYPE_POSTERIZE_TOOL);

  menu_item = gtk_item_factory_get_widget (image_factory,
					   tool_info->menu_path);
  if (menu_item && menu_item->parent)
    gtk_menu_reorder_child (GTK_MENU (menu_item->parent), menu_item, 3);

  {
    GtkType color_tools[] = { GIMP_TYPE_COLOR_BALANCE_TOOL,
			      GIMP_TYPE_HUE_SATURATION_TOOL,
			      GIMP_TYPE_BRIGHTNESS_CONTRAST_TOOL,
			      GIMP_TYPE_THRESHOLD_TOOL,
			      GIMP_TYPE_LEVELS_TOOL,
			      GIMP_TYPE_CURVES_TOOL };
    static gint n_color_tools = (sizeof (color_tools) /
				 sizeof (color_tools[0]));
    GtkWidget *separator;
    gint       i, pos;

    pos = 1;

    for (i = 0; i < n_color_tools; i++)
      {
	tool_info = tool_manager_get_info_by_type (the_gimp, color_tools[i]);

	menu_item = gtk_item_factory_get_widget (image_factory,
						 tool_info->menu_path);
	if (menu_item && menu_item->parent)
	  {
	    gtk_menu_reorder_child (GTK_MENU (menu_item->parent),
				    menu_item, pos);
	    pos++;
	  }
      }

    if (menu_item && menu_item->parent)
      {
	separator = gtk_menu_item_new ();
	gtk_menu_shell_insert (GTK_MENU_SHELL (menu_item->parent), separator, pos);
	gtk_widget_show (separator);
      }
  }

  filename = gimp_personal_rc_file ("menurc");
  gtk_item_factory_parse_rc (filename);
  g_free (filename);
}

#ifdef ENABLE_NLS

static gchar *
menus_menu_translate_func (const gchar *path,
			   gpointer     data)
{
  static gchar   *menupath = NULL;

  GtkItemFactory *item_factory = NULL;
  gchar          *retval;
  gchar          *factory;
  gchar          *translation;
  gchar          *domain = NULL;
  gchar          *complete = NULL;
  gchar          *p, *t;

  factory = (gchar *) data;

  if (menupath)
    g_free (menupath);

  retval = menupath = g_strdup (path);

  if ((strstr (path, "/tearoff1") != NULL) ||
      (strstr (path, "/---") != NULL) ||
      (strstr (path, "/MRU") != NULL))
    return retval;

  if (factory)
    item_factory = gtk_item_factory_from_path (factory);
  if (item_factory)
    {
      domain   = g_object_get_data (G_OBJECT (item_factory), "textdomain");
      complete = g_object_get_data (G_OBJECT (item_factory), "complete");
    }
  
  if (domain)   /*  use the plugin textdomain  */
    {
      g_free (menupath);
      menupath = g_strconcat (factory, path, NULL);

      if (complete)
	{
	  /*  
           *  This is a branch, use the complete path for translation, 
	   *  then strip off entries from the end until it matches. 
	   */
	  complete = g_strconcat (factory, complete, NULL);
	  translation = g_strdup (dgettext (domain, complete));

	  while (complete && *complete && 
		 translation && *translation && 
		 strcmp (complete, menupath))
	    {
	      p = strrchr (complete, '/');
	      t = strrchr (translation, '/');
	      if (p && t)
		{
		  *p = '\0';
		  *t = '\0';
		}
	      else
		break;
	    }

	  g_free (complete);
	}
      else
	{
	  translation = dgettext (domain, menupath);
	}

      /* 
       * Work around a bug in GTK+ prior to 1.2.7 (similar workaround below)
       */
      if (strncmp (factory, translation, strlen (factory)) == 0)
	{
	  retval = translation + strlen (factory);
	  if (complete)
	    {
	      g_free (menupath);
	      menupath = translation;
	    }
	}
      else
	{
	  g_warning ("bad translation for menupath: %s", menupath);
	  retval = menupath + strlen (factory);
	  if (complete)
	    g_free (translation);
	}
    }
  else   /*  use the gimp textdomain  */
    {
      if (complete)
	{
	  /*  
           *  This is a branch, use the complete path for translation, 
	   *  then strip off entries from the end until it matches. 
	   */
	  complete = g_strdup (complete);
	  translation = g_strdup (gettext (complete));
	  
	  while (*complete && *translation && strcmp (complete, menupath))
	    {
	      p = strrchr (complete, '/');
	      t = strrchr (translation, '/');
	      if (p && t)
		{
		  *p = '\0';
		  *t = '\0';
		}
	      else
		break;
	    }
	  g_free (complete);
	}
      else
	translation = gettext (menupath);

      if (*translation == '/')
	{
	  retval = translation;
	  if (complete)
	    {
	      g_free (menupath);
	      menupath = translation;
	    }
	}
      else
	{
	  g_warning ("bad translation for menupath: %s", menupath);
	  if (complete)
	    g_free (translation);
	}
    }
  
  return retval;
}

#endif  /*  ENABLE_NLS  */

static void   
menus_tearoff_cmd_callback (GtkWidget *widget,
			    gpointer   data,
			    guint      action)
{
  if (GTK_IS_TEAROFF_MENU_ITEM (widget))
    {
      GtkTearoffMenuItem *tomi = (GtkTearoffMenuItem *) widget;

      if (tomi->torn_off)
	{
	  GtkWidget *toplevel;

	  toplevel = gtk_widget_get_toplevel (widget);

	  if (! GTK_IS_WINDOW (toplevel))
	    {
	      g_warning ("menus_tearoff_cmd_callback(): tearoff menu not "
			 "in top level window");
	    }
	  else
	    {
#ifdef __GNUC__
#warning FIXME: register tearoffs
#endif
	      g_object_set_data (G_OBJECT (widget), "tearoff-menu-toplevel",
                                 toplevel);

	      gimp_dialog_set_icon (GTK_WINDOW (toplevel));
	    }
	}
      else
	{
	  GtkWidget *toplevel;

	  toplevel = (GtkWidget *) g_object_get_data (G_OBJECT (widget),
                                                      "tearoff-menu-toplevel");

	  if (! toplevel)
	    {
	      g_warning ("menus_tearoff_cmd_callback(): can't unregister "
			 "tearoff menu top level window");
	    }
	  else
	    {
#ifdef __GNUC__
#warning FIXME: unregister tearoffs
#endif
	    }
	}
    }
}

#ifdef ENABLE_DEBUG_ENTRY

#include <unistd.h>

static void
menus_debug_recurse_menu (GtkWidget *menu,
			  gint       depth,
			  gchar     *path)
{
  GtkItemFactory      *item_factory;
  GtkItemFactoryItem  *item;
  GtkItemFactoryClass *class;
  GtkWidget           *menu_item;
  GList   *list;
  gchar   *label;
  gchar   *help_page;
  gchar   *help_path;
  gchar   *factory_path;
  gchar   *hash;
  gchar   *full_path;
  gchar   *accel;
  gchar   *format_str;

  for (list = GTK_MENU_SHELL (menu)->children; list; list = g_list_next (list))
    {
      menu_item = GTK_WIDGET (list->data);
      
      if (GTK_IS_LABEL (GTK_BIN (menu_item)->child))
	{
	  gtk_label_get (GTK_LABEL (GTK_BIN (menu_item)->child), &label);
	  full_path = g_strconcat (path, "/", label, NULL);
	  class = gtk_type_class (GTK_TYPE_ITEM_FACTORY);
	  item = g_hash_table_lookup (class->item_ht, full_path);
	  if (item)
	    {
	      accel = gtk_accelerator_name (item->accelerator_key, 
					    item->accelerator_mods);
	    }
	  else
	    {
	      accel = NULL;
	    }

	  item_factory = gtk_item_factory_from_path (path);
	  if (item_factory)
	    {
	      factory_path = 
                (gchar *) g_object_get_data (G_OBJECT (item_factory),
                                             "factory_path");
	      help_page = 
                g_strconcat (factory_path ? factory_path : "",
                             factory_path ? G_DIR_SEPARATOR_S : "",
                             (gchar*) g_object_get_data (G_OBJECT (menu_item), 
                                                         "help_page"),
                             NULL);
	    }
	  else
	    {
	      help_page = 
                g_strdup ((gchar *) g_object_get_data (G_OBJECT (menu_item), 
                                                                 "help_page"));
	    } 

	  if (help_page)
	    {
	      help_path = 
                g_strconcat (gimp_data_directory (), G_DIR_SEPARATOR_S, 
                             "help", G_DIR_SEPARATOR_S, 
                             "C", G_DIR_SEPARATOR_S,
                             help_page, NULL);

	      if ((hash = strchr (help_path, '#')) != NULL)
		*hash = '\0';

	      if (access (help_path, R_OK))
		{
		  g_free (help_path);
		  help_path = g_strconcat ("! ", help_page, NULL);
		  g_free (help_page);
		  help_page = help_path;
		}
	      else
		{
		  g_free (help_path);
		}
	    }

	  format_str = g_strdup_printf ("%%%ds%%%ds %%-20s %%s\n", 
					depth * 2, depth * 2 - 40);
	  g_print (format_str, 
		   "", label, accel ? accel : "", help_page ? help_page : "");
	  g_free (format_str);
	  g_free (help_page);

	  if (GTK_MENU_ITEM (menu_item)->submenu)
	    menus_debug_recurse_menu (GTK_MENU_ITEM (menu_item)->submenu, 
				      depth + 1, full_path);

	  g_free (full_path);
	}			      
    }
}

static void
menus_debug_cmd_callback (GtkWidget *widget,
			  gpointer   data,
			  guint      action)
{
  gint                  n_factories = 7;
  GtkItemFactory       *factories[7];
  GimpItemFactoryEntry *entries[7];

  GtkWidget *menu_item;
  gint       i;

  factories[0] = toolbox_factory;
  factories[1] = image_factory;
  factories[2] = layers_factory;
  factories[3] = channels_factory;
  factories[4] = paths_factory;
  factories[5] = load_factory;
  factories[6] = save_factory;

  entries[0] = toolbox_entries;
  entries[1] = image_entries;
  entries[2] = layers_entries;
  entries[3] = channels_entries;
  entries[4] = paths_entries;
  entries[5] = load_entries;
  entries[6] = save_entries;
  
  /*  toolbox needs special treatment  */
  g_print ("%s\n", factories[0]->path);

  menu_item = gtk_item_factory_get_item (factories[0], "/File");
  if (menu_item && menu_item->parent && GTK_IS_MENU_BAR (menu_item->parent))
    menus_debug_recurse_menu (menu_item->parent, 1, factories[0]->path);

  g_print ("\n");

  for (i = 1; i < n_factories; i++)
    {
      g_print ("%s\n", factories[i]->path);

      menu_item = gtk_item_factory_get_item (factories[i], entries[i][0].entry.path);
      if (menu_item && menu_item->parent && GTK_IS_MENU (menu_item->parent))
	menus_debug_recurse_menu (menu_item->parent, 1, factories[i]->path);

      g_print ("\n");
    }
}

#endif  /*  ENABLE_DEBUG_ENTRY  */
