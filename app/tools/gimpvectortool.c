/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * Vector tool
 * Copyright (C) 2003 Simon Budig  <simon@gimp.org>
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

#include "libgimpmath/gimpmath.h"
#include "libgimpbase/gimpbase.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "tools-types.h"

#include "core/gimp.h"
#include "core/gimpcontext.h"
#include "core/gimpchannel-select.h"
#include "core/gimpimage.h"
#include "core/gimpimage-undo-push.h"
#include "core/gimplist.h"
#include "core/gimptoolinfo.h"
#include "core/gimpundo.h"
#include "core/gimpundostack.h"

#include "vectors/gimpanchor.h"
#include "vectors/gimpvectors.h"
#include "vectors/gimpbezierstroke.h"

#include "widgets/gimphelp-ids.h"

#include "display/gimpdisplay.h"
#include "display/gimpdisplayshell.h"
#include "display/gimpdisplayshell-scale.h"

#include "gimptoolcontrol.h"
#include "gimpvectoroptions.h"
#include "gimpvectortool.h"

#include "dialogs/stroke-dialog.h"

#include "gimp-intl.h"


#define TARGET 9

#define TOGGLE_MASK GDK_SHIFT_MASK
#define MOVE_MASK   GDK_MOD1_MASK
#define INSDEL_MASK GDK_CONTROL_MASK


/*  local function prototypes  */

static void     gimp_vector_tool_class_init      (GimpVectorToolClass *klass);
static void     gimp_vector_tool_init            (GimpVectorTool      *tool);

static void     gimp_vector_tool_control         (GimpTool        *tool,
                                                  GimpToolAction   action,
                                                  GimpDisplay     *gdisp);
static void     gimp_vector_tool_button_press    (GimpTool        *tool,
                                                  GimpCoords      *coords,
                                                  guint32          time,
                                                  GdkModifierType  state,
                                                  GimpDisplay     *gdisp);
static void     gimp_vector_tool_button_release  (GimpTool        *tool,
                                                  GimpCoords      *coords,
                                                  guint32          time,
                                                  GdkModifierType  state,
                                                  GimpDisplay     *gdisp);
static void     gimp_vector_tool_motion          (GimpTool        *tool,
                                                  GimpCoords      *coords,
                                                  guint32          time,
                                                  GdkModifierType  state,
                                                  GimpDisplay     *gdisp);
static gboolean gimp_vector_tool_key_press       (GimpTool        *tool,
                                                  GdkEventKey     *kevent,
                                                  GimpDisplay     *gdisp);
static void     gimp_vector_tool_modifier_key    (GimpTool        *tool,
                                                  GdkModifierType  key,
                                                  gboolean         press,
                                                  GdkModifierType  state,
                                                  GimpDisplay     *gdisp);
static void     gimp_vector_tool_oper_update     (GimpTool        *tool,
                                                  GimpCoords      *coords,
                                                  GdkModifierType  state,
                                                  GimpDisplay     *gdisp);
static void     gimp_vector_tool_status_update   (GimpTool        *tool,
                                                  GimpDisplay     *gdisp);
static void     gimp_vector_tool_status_set      (GimpTool        *tool,
                                                  GimpDisplay     *gdisp,
                                                  const gchar     *message);
static void     gimp_vector_tool_cursor_update   (GimpTool        *tool,
                                                  GimpCoords      *coords,
                                                  GdkModifierType  state,
                                                  GimpDisplay     *gdisp);

static void     gimp_vector_tool_draw            (GimpDrawTool    *draw_tool);

static void     gimp_vector_tool_vectors_changed (GimpImage       *gimage,
                                                  GimpVectorTool  *vector_tool);
static void     gimp_vector_tool_vectors_removed (GimpVectors     *vectors,
                                                  GimpVectorTool  *vector_tool);
static void     gimp_vector_tool_vectors_visible (GimpVectors     *vectors,
                                                  GimpVectorTool  *vector_tool);
static void     gimp_vector_tool_vectors_freeze  (GimpVectors     *vectors,
                                                  GimpVectorTool  *vector_tool);
static void     gimp_vector_tool_vectors_thaw    (GimpVectors     *vectors,
                                                  GimpVectorTool  *vector_tool);

static void     gimp_vector_tool_move_selected_anchors
                                                 (GimpVectorTool  *vector_tool,
                                                  gdouble          x,
                                                  gdouble          y);
static void     gimp_vector_tool_delete_selected_anchors
                                                 (GimpVectorTool  *vector_tool);
static void     gimp_vector_tool_verify_state    (GimpVectorTool  *vector_tool);
static void     gimp_vector_tool_undo_push       (GimpVectorTool  *vector_tool,
                                                  const gchar     *desc);

static void     gimp_vector_tool_to_selection    (GimpVectorTool  *vector_tool);
static void     gimp_vector_tool_to_selection_extended
                                                 (GimpVectorTool  *vector_tool,
                                                  gint             state);
static void     gimp_vector_tool_stroke_vectors  (GimpVectorTool  *vector_tool,
                                                  GtkWidget       *button);


static GimpDrawToolClass *parent_class = NULL;


void
gimp_vector_tool_register (GimpToolRegisterCallback callback,
                           gpointer                 data)
{
  (* callback) (GIMP_TYPE_VECTOR_TOOL,
                GIMP_TYPE_VECTOR_OPTIONS,
                gimp_vector_options_gui,
                0,
                "gimp-vector-tool",
                _("Paths"),
                _("Create and edit paths"),
                N_("_Paths"), "b",
                NULL, GIMP_HELP_TOOL_PATH,
                GIMP_STOCK_TOOL_PATH,
                data);
}

GType
gimp_vector_tool_get_type (void)
{
  static GType tool_type = 0;

  if (! tool_type)
    {
      static const GTypeInfo tool_info =
      {
        sizeof (GimpVectorToolClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_vector_tool_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data     */
	sizeof (GimpVectorTool),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_vector_tool_init,
      };

      tool_type = g_type_register_static (GIMP_TYPE_DRAW_TOOL,
					  "GimpVectorTool",
                                          &tool_info, 0);
    }

  return tool_type;
}

static void
gimp_vector_tool_class_init (GimpVectorToolClass *klass)
{
  GObjectClass      *object_class;
  GimpToolClass     *tool_class;
  GimpDrawToolClass *draw_tool_class;

  object_class    = G_OBJECT_CLASS (klass);
  tool_class      = GIMP_TOOL_CLASS (klass);
  draw_tool_class = GIMP_DRAW_TOOL_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  tool_class->control        = gimp_vector_tool_control;
  tool_class->button_press   = gimp_vector_tool_button_press;
  tool_class->button_release = gimp_vector_tool_button_release;
  tool_class->motion         = gimp_vector_tool_motion;
  tool_class->key_press      = gimp_vector_tool_key_press;
  tool_class->modifier_key   = gimp_vector_tool_modifier_key;
  tool_class->oper_update    = gimp_vector_tool_oper_update;
  tool_class->cursor_update  = gimp_vector_tool_cursor_update;

  draw_tool_class->draw      = gimp_vector_tool_draw;
}

static void
gimp_vector_tool_init (GimpVectorTool *vector_tool)
{
  GimpTool *tool = GIMP_TOOL (vector_tool);

  gimp_tool_control_set_scroll_lock (tool->control, TRUE);
  gimp_tool_control_set_motion_mode (tool->control, GIMP_MOTION_MODE_COMPRESS);
  gimp_tool_control_set_tool_cursor (tool->control, GIMP_TOOL_CURSOR_PATHS);

  vector_tool->status_msg     = NULL;

  vector_tool->function       = VECTORS_CREATE_VECTOR;
  vector_tool->restriction    = GIMP_ANCHOR_FEATURE_NONE;
  vector_tool->modifier_lock  = FALSE;
  vector_tool->last_x         = 0.0;
  vector_tool->last_y         = 0.0;
  vector_tool->undo_motion    = FALSE;
  vector_tool->have_undo      = FALSE;

  vector_tool->cur_anchor     = NULL;
  vector_tool->cur_anchor2    = NULL;
  vector_tool->cur_stroke     = NULL;
  vector_tool->cur_position   = 0.0;
  vector_tool->cur_vectors    = NULL;
  vector_tool->vectors        = NULL;

  vector_tool->sel_count      = 0;
  vector_tool->sel_anchor     = NULL;
  vector_tool->sel_stroke     = NULL;

  vector_tool->saved_mode     = GIMP_VECTOR_MODE_DESIGN;
}


static void
gimp_vector_tool_control (GimpTool       *tool,
                          GimpToolAction  action,
                          GimpDisplay    *gdisp)
{
  GimpVectorTool *vector_tool = GIMP_VECTOR_TOOL (tool);

  switch (action)
    {
    case PAUSE:
      break;

    case RESUME:
      break;

    case HALT:
      gimp_vector_tool_set_vectors (vector_tool, NULL);
      gimp_vector_tool_status_set (tool, NULL, NULL);
      break;

    default:
      break;
    }

  GIMP_TOOL_CLASS (parent_class)->control (tool, action, gdisp);
}

static void
gimp_vector_tool_button_press (GimpTool        *tool,
                               GimpCoords      *coords,
                               guint32          time,
                               GdkModifierType  state,
                               GimpDisplay     *gdisp)
{
  GimpDrawTool      *draw_tool;
  GimpVectorTool    *vector_tool;
  GimpVectorOptions *options;
  GimpVectors       *vectors;

  draw_tool   = GIMP_DRAW_TOOL (tool);
  vector_tool = GIMP_VECTOR_TOOL (tool);
  options     = GIMP_VECTOR_OPTIONS (tool->tool_info->tool_options);

  /* do nothing if we are an FINISHED state */
  if (vector_tool->function == VECTORS_FINISHED)
    return;

  g_return_if_fail (vector_tool->vectors  != NULL                  ||
                    vector_tool->function == VECTORS_SELECT_VECTOR ||
                    vector_tool->function == VECTORS_CREATE_VECTOR);

  vector_tool->undo_motion = FALSE;

  /* Save the current modifier state */

  vector_tool->saved_state = state;

  gimp_draw_tool_pause (draw_tool);

  if (gimp_draw_tool_is_active (draw_tool) && draw_tool->gdisp != gdisp)
    {
      gimp_draw_tool_stop (draw_tool);
    }

  gimp_tool_control_activate (tool->control);
  tool->gdisp = gdisp;

  /* select a vectors object */

  if (vector_tool->function == VECTORS_SELECT_VECTOR)
    {
      if (gimp_draw_tool_on_vectors (draw_tool, gdisp, coords, TARGET, TARGET,
                                     NULL, NULL, NULL, NULL, NULL, &vectors))
        {
          gimp_vector_tool_set_vectors (vector_tool, vectors);
          gimp_image_set_active_vectors (gdisp->gimage, vectors);
        }
      vector_tool->function = VECTORS_FINISHED;
    }

  /* create a new vector from scratch */

  if (vector_tool->function == VECTORS_CREATE_VECTOR)
    {
      vectors = gimp_vectors_new (gdisp->gimage, _("Unnamed"));

      /* Undo step gets added implicitely */
      vector_tool->have_undo = TRUE;

      vector_tool->undo_motion = TRUE;

      gimp_image_add_vectors (gdisp->gimage, vectors, -1);
      gimp_image_flush (gdisp->gimage);

      gimp_vector_tool_set_vectors (vector_tool, vectors);

      vector_tool->function = VECTORS_CREATE_STROKE;

    }

  gimp_vectors_freeze (vector_tool->vectors);

  /* create a new stroke */

  if (vector_tool->function == VECTORS_CREATE_STROKE)
    {
      g_return_if_fail (vector_tool->vectors != NULL);

      gimp_vector_tool_undo_push (vector_tool, _("Add Stroke"));

      vector_tool->cur_stroke = gimp_bezier_stroke_new ();
      gimp_vectors_stroke_add (vector_tool->vectors, vector_tool->cur_stroke);

      vector_tool->undo_motion = TRUE;

      vector_tool->sel_stroke = vector_tool->cur_stroke;
      vector_tool->cur_anchor = NULL;
      vector_tool->sel_anchor = NULL;
      vector_tool->function = VECTORS_ADD_ANCHOR;
    }


  /* add an anchor to an existing stroke */

  if (vector_tool->function == VECTORS_ADD_ANCHOR)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Add Anchor"));

      vector_tool->undo_motion = TRUE;

      vector_tool->cur_anchor =
                     gimp_bezier_stroke_extend (vector_tool->sel_stroke, coords,
                                                vector_tool->sel_anchor,
                                                EXTEND_EDITABLE);

      vector_tool->restriction = GIMP_ANCHOR_FEATURE_SYMMETRIC;

      if (!options->polygonal)
        vector_tool->function = VECTORS_MOVE_HANDLE;
      else
        vector_tool->function = VECTORS_MOVE_ANCHOR;
      vector_tool->cur_stroke = vector_tool->sel_stroke;
    }


  /* Insertion of an anchor in a curve segment */

  if (vector_tool->function == VECTORS_INSERT_ANCHOR)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Insert Anchor"));

      vector_tool->undo_motion = TRUE;

      vector_tool->cur_anchor =
                         gimp_stroke_anchor_insert (vector_tool->cur_stroke,
                                                    vector_tool->cur_anchor,
                                                    vector_tool->cur_position);
      if (vector_tool->cur_anchor)
        {
          if (options->polygonal)
            {
              gimp_stroke_anchor_convert (vector_tool->cur_stroke,
                                          vector_tool->cur_anchor,
                                          GIMP_ANCHOR_FEATURE_EDGE);
            }
          vector_tool->function = VECTORS_MOVE_ANCHOR;
        }
      else
        {
          vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* move a handle */

  if (vector_tool->function == VECTORS_MOVE_HANDLE)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Drag Handle"));

      if (vector_tool->cur_anchor->type == GIMP_ANCHOR_ANCHOR)
        {
          if (!vector_tool->cur_anchor->selected)
            {
              gimp_vectors_anchor_select (vector_tool->vectors,
                                          vector_tool->cur_stroke,
                                          vector_tool->cur_anchor,
                                          TRUE, TRUE);
              vector_tool->undo_motion = TRUE;
            }

          gimp_draw_tool_on_vectors_handle (GIMP_DRAW_TOOL (tool), gdisp,
                                            vector_tool->vectors, coords,
                                            TARGET, TARGET,
                                            GIMP_ANCHOR_CONTROL, TRUE,
                                            &vector_tool->cur_anchor,
                                            &vector_tool->cur_stroke);
          if (!vector_tool->cur_anchor)
            vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* move an anchor */

  if (vector_tool->function == VECTORS_MOVE_ANCHOR)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Drag Anchor"));

      if (!vector_tool->cur_anchor->selected)
        {
          gimp_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor,
                                      TRUE, TRUE);
          vector_tool->undo_motion = TRUE;
        }
    }


  /* move multiple anchors */

  if (vector_tool->function == VECTORS_MOVE_ANCHORSET)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Drag Anchors"));

      if (state & TOGGLE_MASK)
        {
          gimp_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor,
                                      !vector_tool->cur_anchor->selected,
                                      FALSE);
          vector_tool->undo_motion = TRUE;
          if (vector_tool->cur_anchor->selected == FALSE)
            vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* move a curve segment directly */

  if (vector_tool->function == VECTORS_MOVE_CURVE)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Drag Curve"));

      /* the magic numbers are taken from the "feel good" parameter
       * from gimp_bezier_stroke_point_move_relative in gimpbezierstroke.c. */
      if (vector_tool->cur_position < 5.0 / 6.0)
        {
          gimp_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor, TRUE, TRUE);
          vector_tool->undo_motion = TRUE;
        }

      if (vector_tool->cur_position > 1.0 / 6.0)
        {
          gimp_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor2, TRUE,
                                      (vector_tool->cur_position >= 5.0 / 6.0));
          vector_tool->undo_motion = TRUE;
        }

    }


  /* connect two strokes */

  if (vector_tool->function == VECTORS_CONNECT_STROKES)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Connect Strokes"));

      gimp_stroke_connect_stroke (vector_tool->sel_stroke,
                                  vector_tool->sel_anchor,
                                  vector_tool->cur_stroke,
                                  vector_tool->cur_anchor);
      vector_tool->undo_motion = TRUE;

      if (vector_tool->cur_stroke != vector_tool->sel_stroke &&
          gimp_stroke_is_empty (vector_tool->cur_stroke))
        {
          gimp_vectors_stroke_remove (vector_tool->vectors,
                                      vector_tool->cur_stroke);
        }
      vector_tool->sel_anchor = vector_tool->cur_anchor;
      vector_tool->cur_stroke = vector_tool->sel_stroke;

      gimp_vectors_anchor_select (vector_tool->vectors,
                                  vector_tool->sel_stroke,
                                  vector_tool->sel_anchor, TRUE, TRUE);

      vector_tool->function = VECTORS_FINISHED;
    }


  /* move a stroke or all strokes of a vectors object */

  if (vector_tool->function == VECTORS_MOVE_STROKE ||
      vector_tool->function == VECTORS_MOVE_VECTORS)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Drag Path"));

      /* Work is being done in gimp_vector_tool_motion ()... */
    }


  /* convert an anchor to something that looks like an edge */

  if (vector_tool->function == VECTORS_CONVERT_EDGE)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Convert Edge"));

      gimp_stroke_anchor_convert (vector_tool->cur_stroke,
                                  vector_tool->cur_anchor,
                                  GIMP_ANCHOR_FEATURE_EDGE);
      vector_tool->undo_motion = TRUE;

      if (vector_tool->cur_anchor->type == GIMP_ANCHOR_ANCHOR)
        {
          gimp_vectors_anchor_select (vector_tool->vectors,
                                      vector_tool->cur_stroke,
                                      vector_tool->cur_anchor, TRUE, TRUE);

          vector_tool->function = VECTORS_MOVE_ANCHOR;
        }
      else
        {
          vector_tool->cur_stroke = NULL;
          vector_tool->cur_anchor = NULL;

          /* avoid doing anything stupid */
          vector_tool->function = VECTORS_FINISHED;
        }
    }


  /* removal of a node in a stroke */

  if (vector_tool->function == VECTORS_DELETE_ANCHOR)
    {
      gimp_vector_tool_undo_push (vector_tool, _("Delete Anchor"));

      gimp_stroke_anchor_delete (vector_tool->cur_stroke,
                                 vector_tool->cur_anchor);
      vector_tool->undo_motion = TRUE;

      if (gimp_stroke_is_empty (vector_tool->cur_stroke))
        gimp_vectors_stroke_remove (vector_tool->vectors,
                                    vector_tool->cur_stroke);

      vector_tool->cur_stroke = NULL;
      vector_tool->cur_anchor = NULL;
      vector_tool->function = VECTORS_FINISHED;
    }


  /* deleting a segment (opening up a stroke) */

  if (vector_tool->function == VECTORS_DELETE_SEGMENT)
    {
      GimpStroke *new_stroke;

      gimp_vector_tool_undo_push (vector_tool, _("Delete Segment"));

      new_stroke = gimp_stroke_open (vector_tool->cur_stroke,
                                     vector_tool->cur_anchor);
      if (new_stroke)
        gimp_vectors_stroke_add (vector_tool->vectors, new_stroke);

      vector_tool->undo_motion = TRUE;
      vector_tool->cur_stroke = NULL;
      vector_tool->cur_anchor = NULL;
      vector_tool->function = VECTORS_FINISHED;
    }

  vector_tool->last_x = coords->x;
  vector_tool->last_y = coords->y;

  gimp_vectors_thaw (vector_tool->vectors);

  if (! gimp_draw_tool_is_active (draw_tool))
    {
      gimp_draw_tool_start (draw_tool, gdisp);
    }

  gimp_draw_tool_resume (draw_tool);
}

static void
gimp_vector_tool_button_release (GimpTool        *tool,
                                 GimpCoords      *coords,
                                 guint32          time,
                                 GdkModifierType  state,
                                 GimpDisplay     *gdisp)
{
  GimpVectorTool *vector_tool = GIMP_VECTOR_TOOL (tool);

  vector_tool->function = VECTORS_FINISHED;

  if (vector_tool->have_undo &&
      (! vector_tool->undo_motion || (state & GDK_BUTTON3_MASK)))
    {
      GimpUndo            *undo;
      GimpUndoAccumulator  accum = { 0, };

      undo = gimp_undo_stack_pop_undo (gdisp->gimage->undo_stack,
                                       GIMP_UNDO_MODE_UNDO, &accum);

      gimp_image_undo_event (gdisp->gimage, GIMP_UNDO_EVENT_UNDO_EXPIRED, undo);

      gimp_undo_free (undo, GIMP_UNDO_MODE_UNDO);
      g_object_unref (undo);
    }

  vector_tool->have_undo = FALSE;
  vector_tool->undo_motion = FALSE;

  gimp_tool_control_halt (tool->control);
  gimp_image_flush (gdisp->gimage);
}

static void
gimp_vector_tool_motion (GimpTool        *tool,
                         GimpCoords      *coords,
                         guint32          time,
                         GdkModifierType  state,
                         GimpDisplay     *gdisp)
{
  GimpVectorTool    *vector_tool;
  GimpVectorOptions *options;
  GimpAnchor        *anchor;

  vector_tool = GIMP_VECTOR_TOOL (tool);
  options     = GIMP_VECTOR_OPTIONS (tool->tool_info->tool_options);

  if (vector_tool->function == VECTORS_FINISHED)
    return;

  gimp_vectors_freeze (vector_tool->vectors);

  if ((vector_tool->saved_state & TOGGLE_MASK) != (state & TOGGLE_MASK))
    vector_tool->modifier_lock = FALSE;

  if (!vector_tool->modifier_lock)
    {
      if (state & TOGGLE_MASK)
        {
          vector_tool->restriction = GIMP_ANCHOR_FEATURE_SYMMETRIC;
        }
      else
        {
          vector_tool->restriction = GIMP_ANCHOR_FEATURE_NONE;
        }
    }

  switch (vector_tool->function)
    {
    case VECTORS_MOVE_ANCHOR:
    case VECTORS_MOVE_HANDLE:
      anchor = vector_tool->cur_anchor;

      if (anchor)
        {
          gimp_stroke_anchor_move_absolute (vector_tool->cur_stroke,
                                            vector_tool->cur_anchor,
                                            coords, vector_tool->restriction);
          vector_tool->undo_motion = TRUE;
        }
      break;

    case VECTORS_MOVE_CURVE:
      if (options->polygonal)
        {
          gimp_vector_tool_move_selected_anchors (vector_tool,
                                               coords->x - vector_tool->last_x,
                                               coords->y - vector_tool->last_y);
          vector_tool->undo_motion = TRUE;
        }
      else
        {
          gimp_stroke_point_move_absolute (vector_tool->cur_stroke,
                                           vector_tool->cur_anchor,
                                           vector_tool->cur_position,
                                           coords, vector_tool->restriction);
          vector_tool->undo_motion = TRUE;
        }
      break;

    case VECTORS_MOVE_ANCHORSET:
      gimp_vector_tool_move_selected_anchors (vector_tool,
                                              coords->x - vector_tool->last_x,
                                              coords->y - vector_tool->last_y);
      vector_tool->undo_motion = TRUE;
      break;

    case VECTORS_MOVE_STROKE:
      if (vector_tool->cur_stroke)
        {
          gimp_stroke_translate (vector_tool->cur_stroke,
                                 coords->x - vector_tool->last_x,
                                 coords->y - vector_tool->last_y);
          vector_tool->undo_motion = TRUE;
        }
      else if (vector_tool->sel_stroke)
        {
          gimp_stroke_translate (vector_tool->sel_stroke,
                                 coords->x - vector_tool->last_x,
                                 coords->y - vector_tool->last_y);
          vector_tool->undo_motion = TRUE;
        }
      break;

    case VECTORS_MOVE_VECTORS:
      gimp_item_translate (GIMP_ITEM (vector_tool->vectors),
                           coords->x - vector_tool->last_x,
                           coords->y - vector_tool->last_y, FALSE);
      vector_tool->undo_motion = TRUE;
      break;

    default:
      break;
    }

  vector_tool->last_x = coords->x;
  vector_tool->last_y = coords->y;

  gimp_vectors_thaw (vector_tool->vectors);
}

static gboolean
gimp_vector_tool_key_press (GimpTool     *tool,
                            GdkEventKey  *kevent,
                            GimpDisplay  *gdisp)
{
  GimpVectorTool   *vector_tool = GIMP_VECTOR_TOOL (tool);
  GimpDrawTool     *draw_tool   = GIMP_DRAW_TOOL (tool);
  GimpDisplayShell *shell;
  gdouble           xdist, ydist;
  gdouble           pixels = 1.0;

  if (! vector_tool->vectors)
    return TRUE;

  shell = GIMP_DISPLAY_SHELL (draw_tool->gdisp->shell);

  if (kevent->state & GDK_SHIFT_MASK)
    pixels = 10.0;

  if (kevent->state & GDK_CONTROL_MASK)
    pixels = 50.0;

  if (gdisp == draw_tool->gdisp)
    {
      switch (kevent->keyval)
        {
        case GDK_KP_Enter:
        case GDK_Return:
          gimp_vector_tool_to_selection_extended (vector_tool, kevent->state);
          break;

        case GDK_BackSpace:
        case GDK_Delete:
          gimp_vector_tool_delete_selected_anchors (vector_tool);
          break;

        case GDK_Left:
        case GDK_Right:
        case GDK_Up:
        case GDK_Down:
          xdist = FUNSCALEX (shell, pixels);
          ydist = FUNSCALEY (shell, pixels);

          gimp_vector_tool_undo_push (vector_tool, _("Move Anchors"));

          gimp_vectors_freeze (vector_tool->vectors);

          switch (kevent->keyval)
            {
            case GDK_Left:
              gimp_vector_tool_move_selected_anchors (vector_tool, -xdist, 0);
              break;

            case GDK_Right:
              gimp_vector_tool_move_selected_anchors (vector_tool, xdist, 0);
              break;

            case GDK_Up:
              gimp_vector_tool_move_selected_anchors (vector_tool, 0, -ydist);
              break;

            case GDK_Down:
              gimp_vector_tool_move_selected_anchors (vector_tool, 0, ydist);
              break;

            default:
              break;
            }

          gimp_vectors_thaw (vector_tool->vectors);
          vector_tool->have_undo = FALSE;
          break;

        default:
          break;
        }

      gimp_image_flush (gdisp->gimage);
    }

  return TRUE;
}

static void
gimp_vector_tool_modifier_key (GimpTool        *tool,
                               GdkModifierType  key,
                               gboolean         press,
                               GdkModifierType  state,
                               GimpDisplay     *gdisp)
{
  GimpVectorTool    *vector_tool = GIMP_VECTOR_TOOL (tool);
  GimpVectorOptions *options;

  options = GIMP_VECTOR_OPTIONS (tool->tool_info->tool_options);

  if (key == TOGGLE_MASK)
    return;

  if (key == INSDEL_MASK || key == MOVE_MASK)
    {
      GimpVectorMode button_mode;

      button_mode = options->edit_mode;

      if (press)
        {
          if (key == (state & (INSDEL_MASK | MOVE_MASK)))
            {
              /*  first modifier pressed  */

              vector_tool->saved_mode = options->edit_mode;
            }
        }
      else
        {
          if (! (state & (INSDEL_MASK | MOVE_MASK)))
            {
              /*  last modifier released  */

              button_mode = vector_tool->saved_mode;
            }
        }

      if (state & MOVE_MASK)
        {
          button_mode = GIMP_VECTOR_MODE_MOVE;
        }
      else if (state & INSDEL_MASK)
        {
          button_mode = GIMP_VECTOR_MODE_EDIT;
        }

      if (button_mode != options->edit_mode)
        {
          g_object_set (options, "vectors-edit-mode", button_mode, NULL);
        }
    }
}

static void
gimp_vector_tool_oper_update (GimpTool        *tool,
                              GimpCoords      *coords,
                              GdkModifierType  state,
                              GimpDisplay     *gdisp)
{
  GimpVectorTool    *vector_tool;
  GimpDrawTool      *draw_tool;
  GimpVectorOptions *options;
  GimpAnchor        *anchor     = NULL;
  GimpAnchor        *anchor2    = NULL;
  GimpStroke        *stroke     = NULL;
  gdouble            position   = -1;
  gboolean           on_handle  = FALSE;
  gboolean           on_curve   = FALSE;
  gboolean           on_vectors = FALSE;

  vector_tool = GIMP_VECTOR_TOOL (tool);
  options     = GIMP_VECTOR_OPTIONS (tool->tool_info->tool_options);

  draw_tool = GIMP_DRAW_TOOL (tool);

  vector_tool->modifier_lock = FALSE;

  /* are we hovering the current vectors on the current display? */
  if (vector_tool->vectors && GIMP_DRAW_TOOL (tool)->gdisp == gdisp)
    {
      on_handle = gimp_draw_tool_on_vectors_handle (GIMP_DRAW_TOOL (tool),
                                                    gdisp,
                                                    vector_tool->vectors,
                                                    coords,
                                                    TARGET, TARGET,
                                                    GIMP_ANCHOR_ANCHOR,
                                                    vector_tool->sel_count > 2,
                                                    &anchor, &stroke);

      if (! on_handle)
        on_curve = gimp_draw_tool_on_vectors_curve (GIMP_DRAW_TOOL (tool),
                                                    gdisp,
                                                    vector_tool->vectors,
                                                    coords,
                                                    TARGET, TARGET,
                                                    NULL,
                                                    &position, &anchor,
                                                    &anchor2, &stroke);
    }

  if (on_handle || on_curve)
    {
      vector_tool->cur_vectors = NULL;
    }
  else
    {
      on_vectors = gimp_draw_tool_on_vectors (draw_tool, gdisp, coords,
                                              TARGET, TARGET,
                                              NULL, NULL, NULL, NULL, NULL,
                                              &(vector_tool->cur_vectors));
    }

  vector_tool->cur_position   = position;
  vector_tool->cur_anchor     = anchor;
  vector_tool->cur_anchor2    = anchor2;
  vector_tool->cur_stroke     = stroke;

  switch (options->edit_mode)
    {
    case GIMP_VECTOR_MODE_DESIGN:
      if (! vector_tool->vectors || GIMP_DRAW_TOOL (tool)->gdisp != gdisp)
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_CREATE_VECTOR;
              vector_tool->restriction = GIMP_ANCHOR_FEATURE_SYMMETRIC;
              vector_tool->modifier_lock = TRUE;
            }
        }
      else if (on_handle)
        {
          if (anchor->type == GIMP_ANCHOR_ANCHOR)
            {
              if (state & TOGGLE_MASK)
                {
                  vector_tool->function = VECTORS_MOVE_ANCHORSET;
                }
              else
                {
                  if (vector_tool->sel_count >= 2 && anchor->selected)
                    vector_tool->function = VECTORS_MOVE_ANCHORSET;
                  else
                    vector_tool->function = VECTORS_MOVE_ANCHOR;
                }
            }
          else
            {
              vector_tool->function = VECTORS_MOVE_HANDLE;
              if (state & TOGGLE_MASK)
                vector_tool->restriction = GIMP_ANCHOR_FEATURE_SYMMETRIC;
              else
                vector_tool->restriction = GIMP_ANCHOR_FEATURE_NONE;
            }
        }
      else if (on_curve)
        {
          if (gimp_stroke_point_is_movable (stroke, anchor, position))
            {
              vector_tool->function = VECTORS_MOVE_CURVE;
              if (state & TOGGLE_MASK)
                vector_tool->restriction = GIMP_ANCHOR_FEATURE_SYMMETRIC;
              else
                vector_tool->restriction = GIMP_ANCHOR_FEATURE_NONE;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else
        {
          if (vector_tool->sel_stroke && vector_tool->sel_anchor &&
              gimp_stroke_is_extendable (vector_tool->sel_stroke,
                                         vector_tool->sel_anchor) &&
              !(state & TOGGLE_MASK))
            vector_tool->function = VECTORS_ADD_ANCHOR;
          else
            vector_tool->function = VECTORS_CREATE_STROKE;

          vector_tool->restriction = GIMP_ANCHOR_FEATURE_SYMMETRIC;
          vector_tool->modifier_lock = TRUE;
        }

      break;

    case GIMP_VECTOR_MODE_EDIT:
      if (! vector_tool->vectors || GIMP_DRAW_TOOL (tool)->gdisp != gdisp)
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else if (on_handle)
        {
          if (anchor->type == GIMP_ANCHOR_ANCHOR)
            {
              if (!(state & TOGGLE_MASK) && vector_tool->sel_anchor &&
                  vector_tool->sel_anchor != anchor &&
                  gimp_stroke_is_extendable (vector_tool->sel_stroke,
                                             vector_tool->sel_anchor) &&
                  gimp_stroke_is_extendable (stroke, anchor))
                {
                  vector_tool->function = VECTORS_CONNECT_STROKES;
                }
              else
                {
                  if (state & TOGGLE_MASK)
                    {
                      vector_tool->function = VECTORS_DELETE_ANCHOR;
                    }
                  else
                    {
                      if (options->polygonal)
                        vector_tool->function = VECTORS_MOVE_ANCHOR;
                      else
                        vector_tool->function = VECTORS_MOVE_HANDLE;
                    }
                }
            }
          else
            {
              if (state & TOGGLE_MASK)
                vector_tool->function = VECTORS_CONVERT_EDGE;
              else
                vector_tool->function = VECTORS_MOVE_HANDLE;
            }
        }
      else if (on_curve)
        {
          if (state & TOGGLE_MASK)
            {
              vector_tool->function = VECTORS_DELETE_SEGMENT;
            }
          else if (gimp_stroke_anchor_is_insertable (stroke, anchor, position))
            {
              vector_tool->function = VECTORS_INSERT_ANCHOR;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else
        {
          vector_tool->function = VECTORS_FINISHED;
        }

      break;

    case GIMP_VECTOR_MODE_MOVE:
      if (! vector_tool->vectors || GIMP_DRAW_TOOL (tool)->gdisp != gdisp)
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_FINISHED;
            }
        }
      else if (on_handle || on_curve)
        {
          if (state & TOGGLE_MASK)
            {
              vector_tool->function = VECTORS_MOVE_VECTORS;
            }
          else
            {
              vector_tool->function = VECTORS_MOVE_STROKE;
            }
        }
      else
        {
          if (on_vectors)
            {
              vector_tool->function = VECTORS_SELECT_VECTOR;
            }
          else
            {
              vector_tool->function = VECTORS_MOVE_VECTORS;
            }
        }
      break;
    }

  gimp_vector_tool_status_update (tool, gdisp);
}


static void
gimp_vector_tool_status_update (GimpTool    *tool,
                                GimpDisplay *gdisp)
{
  GimpVectorTool   *vector_tool = GIMP_VECTOR_TOOL (tool);
  GimpDisplayShell *shell;
  const gchar      *new_status  = NULL;

  shell = tool->gdisp ? GIMP_DISPLAY_SHELL (tool->gdisp->shell) : NULL;

  if (shell && shell->proximity)
    {
      switch (vector_tool->function)
      {
        case VECTORS_SELECT_VECTOR:
          new_status = _("Click to pick path to edit.");
          break;
        case VECTORS_CREATE_VECTOR:
          new_status = _("Click to create a new path.");
          break;
        case VECTORS_CREATE_STROKE:
          new_status = _("Click to create a new component of the path.");
          break;
        case VECTORS_ADD_ANCHOR:
          new_status = _("Click to create a new anchor. (try SHIFT)");
          break;
        case VECTORS_MOVE_ANCHOR:
          new_status = _("Click-Drag to move the anchor around.");
          break;
        case VECTORS_MOVE_ANCHORSET:
          new_status = _("Click-Drag to move the anchors around.");
          break;
        case VECTORS_MOVE_HANDLE:
          new_status = _("Click-Drag to move the handle around. (try SHIFT)");
          break;
        case VECTORS_MOVE_CURVE:
          new_status = _("Click-Drag to change the shape of the curve. "
                         "(SHIFT: symmetrical)");
          break;
        case VECTORS_MOVE_STROKE:
          new_status = _("Click-Drag to move the component around. "
                         "(try SHIFT)");
          break;
        case VECTORS_MOVE_VECTORS:
          new_status = _("Click-Drag to move the path around.");
          break;
        case VECTORS_INSERT_ANCHOR:
          new_status = _("Click to insert an anchor on the path. (try SHIFT)");
          break;
        case VECTORS_DELETE_ANCHOR:
          new_status = _("Click to delete this anchor.");
          break;
        case VECTORS_CONNECT_STROKES:
          new_status = _("Click to connect this anchor "
                          "with the selected endpoint.");
          break;
        case VECTORS_DELETE_SEGMENT:
          new_status = _("Click to open up the path.");
          break;
        case VECTORS_CONVERT_EDGE:
          new_status = _("Click to make this node angular.");
          break;
        case VECTORS_FINISHED:
          new_status = " ";
          break;
      }
    }

  gimp_vector_tool_status_set (tool, gdisp, new_status);
}

static void
gimp_vector_tool_status_set (GimpTool    *tool,
                             GimpDisplay *gdisp,
                             const gchar *message)
{
  GimpVectorTool *vector_tool = GIMP_VECTOR_TOOL (tool);

  if (tool->gdisp &&
      vector_tool->status_msg != message)
    {
      if (vector_tool->status_msg)
        {
          gimp_tool_pop_status (tool);
          vector_tool->status_msg = NULL;
        }

      if (message)
        {
          gimp_tool_push_status (tool, message);
          vector_tool->status_msg = message;
        }
    }
}

static void
gimp_vector_tool_cursor_update (GimpTool        *tool,
                                GimpCoords      *coords,
                                GdkModifierType  state,
                                GimpDisplay     *gdisp)
{
  GimpVectorTool     *vector_tool;
  GimpCursorType      cursor;
  GimpToolCursorType  tool_cursor;
  GimpCursorModifier  cmodifier;

  vector_tool = GIMP_VECTOR_TOOL (tool);

  cursor      = GIMP_CURSOR_MOUSE;
  tool_cursor = GIMP_TOOL_CURSOR_PATHS;
  cmodifier   = GIMP_CURSOR_MODIFIER_NONE;

  switch (vector_tool->function)
    {
    case VECTORS_SELECT_VECTOR:
      cursor      = GDK_HAND2;
      tool_cursor = GIMP_TOOL_CURSOR_HAND;
      break;
    case VECTORS_CREATE_VECTOR:
    case VECTORS_CREATE_STROKE:
      cmodifier = GIMP_CURSOR_MODIFIER_CONTROL;
      break;
    case VECTORS_ADD_ANCHOR:
    case VECTORS_INSERT_ANCHOR:
      cmodifier = GIMP_CURSOR_MODIFIER_PLUS;
      break;
    case VECTORS_DELETE_ANCHOR:
    case VECTORS_DELETE_SEGMENT:
      cmodifier = GIMP_CURSOR_MODIFIER_MINUS;
      break;
    case VECTORS_MOVE_HANDLE:
    case VECTORS_CONVERT_EDGE:
      cursor      = GDK_HAND2;
      tool_cursor = GIMP_TOOL_CURSOR_HAND;
      cmodifier   = GIMP_CURSOR_MODIFIER_CONTROL;
      break;
    case VECTORS_MOVE_ANCHOR:
    case VECTORS_MOVE_CURVE:
    case VECTORS_MOVE_STROKE:
    case VECTORS_MOVE_VECTORS:
    case VECTORS_MOVE_ANCHORSET:
      cmodifier = GIMP_CURSOR_MODIFIER_MOVE;
      break;
    case VECTORS_CONNECT_STROKES:
      cmodifier = GIMP_CURSOR_MODIFIER_INTERSECT;
      break;
    default:
      cursor = GIMP_CURSOR_BAD;
      break;
    }

  gimp_tool_control_set_cursor          (tool->control, cursor);
  gimp_tool_control_set_tool_cursor     (tool->control, tool_cursor);
  gimp_tool_control_set_cursor_modifier (tool->control, cmodifier);

  GIMP_TOOL_CLASS (parent_class)->cursor_update (tool, coords, state, gdisp);
}

static void
gimp_vector_tool_draw (GimpDrawTool *draw_tool)
{
  GimpVectorTool  *vector_tool = GIMP_VECTOR_TOOL (draw_tool);
  GimpAnchor      *cur_anchor  = NULL;
  GimpStroke      *cur_stroke  = NULL;
  GimpVectors     *vectors;
  GArray          *coords;
  gboolean         closed;
  GList           *draw_anchors;
  GList           *list;

  vectors = vector_tool->vectors;

  if (!vectors)
    return;

  while ((cur_stroke = gimp_vectors_stroke_get_next (vectors, cur_stroke)))
    {
      /* anchor handles */
      draw_anchors = gimp_stroke_get_draw_anchors (cur_stroke);

      for (list = draw_anchors; list; list = g_list_next (list))
        {
          cur_anchor = GIMP_ANCHOR (list->data);

          if (cur_anchor->type == GIMP_ANCHOR_ANCHOR)
            {
              gimp_draw_tool_draw_handle (draw_tool,
                                          cur_anchor->selected ?
                                          GIMP_HANDLE_CIRCLE :
                                          GIMP_HANDLE_FILLED_CIRCLE,
                                          cur_anchor->position.x,
                                          cur_anchor->position.y,
                                          TARGET,
                                          TARGET,
                                          GTK_ANCHOR_CENTER,
                                          FALSE);
            }
        }

      g_list_free (draw_anchors);

      if (vector_tool->sel_count <= 2)
        {
          /* control handles */
          draw_anchors = gimp_stroke_get_draw_controls (cur_stroke);

          for (list = draw_anchors; list; list = g_list_next (list))
            {
              cur_anchor = GIMP_ANCHOR (list->data);

              gimp_draw_tool_draw_handle (draw_tool,
                                          GIMP_HANDLE_SQUARE,
                                          cur_anchor->position.x,
                                          cur_anchor->position.y,
                                          TARGET - 3,
                                          TARGET - 3,
                                          GTK_ANCHOR_CENTER,
                                          FALSE);
            }

          g_list_free (draw_anchors);

          /* the lines to the control handles */
          coords = gimp_stroke_get_draw_lines (cur_stroke);

          if (coords)
            {
              if (coords->len % 2 == 0)
                {
                  gint i;

                  for (i = 0; i < coords->len; i += 2)
                    {
                      gimp_draw_tool_draw_dashed_line (draw_tool,
                                    g_array_index (coords, GimpCoords, i).x,
                                    g_array_index (coords, GimpCoords, i).y,
                                    g_array_index (coords, GimpCoords, i + 1).x,
                                    g_array_index (coords, GimpCoords, i + 1).y,
                                    FALSE);
                    }
                }

              g_array_free (coords, TRUE);
            }
        }

      /* the stroke itself */
      if (! gimp_item_get_visible (GIMP_ITEM (vectors)))
        {
          coords = gimp_stroke_interpolate (cur_stroke, 1.0, &closed);

          if (coords)
            {
              if (coords->len)
                gimp_draw_tool_draw_strokes (draw_tool,
                                             &g_array_index (coords,
                                                             GimpCoords, 0),
                                             coords->len, FALSE, FALSE);

              g_array_free (coords, TRUE);
            }
        }
    }
}

static void
gimp_vector_tool_vectors_changed (GimpImage      *gimage,
                                  GimpVectorTool *vector_tool)
{
  gimp_vector_tool_set_vectors (vector_tool,
                                gimp_image_get_active_vectors (gimage));
}

static void
gimp_vector_tool_vectors_removed (GimpVectors    *vectors,
                                  GimpVectorTool *vector_tool)
{
  gimp_vector_tool_set_vectors (vector_tool, NULL);
}

static void
gimp_vector_tool_vectors_visible (GimpVectors    *vectors,
                                  GimpVectorTool *vector_tool)
{
  GimpDrawTool *draw_tool = GIMP_DRAW_TOOL (vector_tool);

  if (gimp_draw_tool_is_active (draw_tool) && draw_tool->paused_count == 0)
    {
      GimpStroke *stroke = NULL;

      while ((stroke = gimp_vectors_stroke_get_next (vectors, stroke)))
        {
          GArray   *coords;
          gboolean  closed;

          coords = gimp_stroke_interpolate (stroke, 1.0, &closed);

          if (coords)
            {
              if (coords->len)
                gimp_draw_tool_draw_strokes (draw_tool,
                                             &g_array_index (coords,
                                                             GimpCoords, 0),
                                             coords->len, FALSE, FALSE);

              g_array_free (coords, TRUE);
            }
        }
    }
}

static void
gimp_vector_tool_vectors_freeze (GimpVectors    *vectors,
                                 GimpVectorTool *vector_tool)
{
  gimp_draw_tool_pause (GIMP_DRAW_TOOL (vector_tool));
}

static void
gimp_vector_tool_vectors_thaw (GimpVectors    *vectors,
                               GimpVectorTool *vector_tool)
{
  /* Ok, the vector might have changed externally (e.g. Undo)
   * we need to validate our internal state. */
  gimp_vector_tool_verify_state (vector_tool);

  gimp_draw_tool_resume (GIMP_DRAW_TOOL (vector_tool));
}

void
gimp_vector_tool_set_vectors (GimpVectorTool *vector_tool,
                              GimpVectors    *vectors)
{
  GimpDrawTool      *draw_tool;
  GimpTool          *tool;
  GimpItem          *item = NULL;
  GtkWidget         *stroke_button;
  GtkWidget         *sel_button;
  GimpVectorOptions *options;

  g_return_if_fail (GIMP_IS_VECTOR_TOOL (vector_tool));
  g_return_if_fail (vectors == NULL || GIMP_IS_VECTORS (vectors));

  draw_tool = GIMP_DRAW_TOOL (vector_tool);
  tool      = GIMP_TOOL (vector_tool);
  options   = GIMP_VECTOR_OPTIONS (tool->tool_info->tool_options);

  if (vectors)
    item = GIMP_ITEM (vectors);

  if (vectors == vector_tool->vectors)
    return;

  gimp_draw_tool_pause (draw_tool);

  if (gimp_draw_tool_is_active (draw_tool) &&
      (! vectors || draw_tool->gdisp->gimage != item->gimage))
    {
      gimp_draw_tool_stop (draw_tool);
    }

  stroke_button = g_object_get_data (G_OBJECT (options),
                                     "gimp-stroke-vectors");
  sel_button = g_object_get_data (G_OBJECT (options),
                                  "gimp-vectors-to-selection");

  if (vector_tool->vectors)
    {
      GimpImage *old_gimage;

      old_gimage = gimp_item_get_image (GIMP_ITEM (vector_tool->vectors));

      g_signal_handlers_disconnect_by_func (old_gimage,
                                            gimp_vector_tool_vectors_changed,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            gimp_vector_tool_vectors_removed,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            gimp_vector_tool_vectors_visible,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            gimp_vector_tool_vectors_freeze,
                                            vector_tool);
      g_signal_handlers_disconnect_by_func (vector_tool->vectors,
                                            gimp_vector_tool_vectors_thaw,
                                            vector_tool);
      g_object_unref (vector_tool->vectors);

      if (sel_button)
        {
          gtk_widget_set_sensitive (sel_button, FALSE);
          g_signal_handlers_disconnect_by_func (sel_button,
                                                gimp_vector_tool_to_selection,
                                                tool);
          g_signal_handlers_disconnect_by_func (sel_button,
                                                gimp_vector_tool_to_selection_extended,
                                                tool);
        }

      if (stroke_button)
        {
          gtk_widget_set_sensitive (stroke_button, FALSE);
          g_signal_handlers_disconnect_by_func (stroke_button,
                                                gimp_vector_tool_stroke_vectors,
                                                tool);
        }
    }

  vector_tool->vectors    = vectors;
  vector_tool->function   = VECTORS_FINISHED;
  gimp_vector_tool_verify_state (vector_tool);

  if (! vector_tool->vectors)
    {
      tool->gdisp = NULL;

      /* leave draw_tool->paused_count in a consistent state */
      gimp_draw_tool_resume (draw_tool);

      vector_tool->function = VECTORS_CREATE_VECTOR;

      return;
    }

  g_object_ref (vectors);

  g_signal_connect_object (item->gimage, "active_vectors_changed",
                           G_CALLBACK (gimp_vector_tool_vectors_changed),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "removed",
                           G_CALLBACK (gimp_vector_tool_vectors_removed),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "visibility_changed",
                           G_CALLBACK (gimp_vector_tool_vectors_visible),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "freeze",
                           G_CALLBACK (gimp_vector_tool_vectors_freeze),
                           vector_tool, 0);
  g_signal_connect_object (vectors, "thaw",
                           G_CALLBACK (gimp_vector_tool_vectors_thaw),
                           vector_tool, 0);

  if (sel_button)
    {
      g_signal_connect_swapped (sel_button, "clicked",
                                G_CALLBACK (gimp_vector_tool_to_selection),
                                tool);
      g_signal_connect_swapped (sel_button, "extended_clicked",
                                G_CALLBACK (gimp_vector_tool_to_selection_extended),
                                tool);
      gtk_widget_set_sensitive (sel_button, TRUE);
    }

  if (stroke_button)
    {
      g_signal_connect_swapped (stroke_button, "clicked",
                                G_CALLBACK (gimp_vector_tool_stroke_vectors),
                                tool);
      gtk_widget_set_sensitive (stroke_button, TRUE);
    }

  if (! gimp_draw_tool_is_active (draw_tool))
    {
      if (tool->gdisp && tool->gdisp->gimage == item->gimage)
        {
          gimp_draw_tool_start (draw_tool, tool->gdisp);
        }
      else
        {
          GimpContext *context;
          GimpDisplay *gdisp;

          context = gimp_get_user_context (tool->tool_info->gimp);
          gdisp   = gimp_context_get_display (context);

          if (! gdisp || gdisp->gimage != item->gimage)
            {
              GList *list;

              gdisp = NULL;

              for (list = GIMP_LIST (item->gimage->gimp->displays)->list;
                   list;
                   list = g_list_next (list))
                {
                  if (((GimpDisplay *) list->data)->gimage == item->gimage)
                    {
                      gimp_context_set_display (context,
                                                (GimpDisplay *) list->data);

                      gdisp = gimp_context_get_display (context);
                      break;
                    }
                }
            }

          tool->gdisp = gdisp;

          if (tool->gdisp)
            {
              gimp_draw_tool_start (draw_tool, tool->gdisp);
            }
        }
    }

  gimp_draw_tool_resume (draw_tool);
}

static void
gimp_vector_tool_move_selected_anchors (GimpVectorTool *vector_tool,
                                        gdouble         x,
                                        gdouble         y)
{
  GimpAnchor *cur_anchor;
  GimpStroke *cur_stroke = NULL;
  GList *anchors;
  GList *list;
  GimpCoords offset = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

  offset.x = x;
  offset.y = y;

  while ((cur_stroke = gimp_vectors_stroke_get_next (vector_tool->vectors,
                                                     cur_stroke)))
    {
      /* anchors */
      anchors = gimp_stroke_get_draw_anchors (cur_stroke);

      for (list = anchors; list; list = g_list_next (list))
        {
          cur_anchor = GIMP_ANCHOR (list->data);

          if (cur_anchor->selected)
            gimp_stroke_anchor_move_relative (cur_stroke,
                                              cur_anchor,
                                              &offset,
                                              GIMP_ANCHOR_FEATURE_NONE);
        }

      g_list_free (anchors);
    }
}

static void
gimp_vector_tool_delete_selected_anchors (GimpVectorTool *vector_tool)
{
  GimpAnchor *cur_anchor;
  GimpStroke *cur_stroke = NULL;
  GList      *anchors;
  GList      *list;
  gboolean    have_undo = FALSE;

  gimp_draw_tool_pause (GIMP_DRAW_TOOL (vector_tool));
  gimp_vectors_freeze (vector_tool->vectors);

  while ((cur_stroke = gimp_vectors_stroke_get_next (vector_tool->vectors,
                                                     cur_stroke)))
    {
      /* anchors */
      anchors = gimp_stroke_get_draw_anchors (cur_stroke);

      for (list = anchors; list; list = g_list_next (list))
        {
          cur_anchor = GIMP_ANCHOR (list->data);

          if (cur_anchor->selected)
            {
              if (! have_undo)
                {
                  gimp_vector_tool_undo_push (vector_tool, _("Delete Anchors"));
                  have_undo = TRUE;
                }

              gimp_stroke_anchor_delete (cur_stroke, cur_anchor);

              if (gimp_stroke_is_empty (cur_stroke))
                gimp_vectors_stroke_remove (vector_tool->vectors, cur_stroke);
            }
        }

      g_list_free (anchors);
    }

  gimp_vectors_thaw (vector_tool->vectors);
  gimp_draw_tool_resume (GIMP_DRAW_TOOL (vector_tool));
}

static void
gimp_vector_tool_verify_state (GimpVectorTool *vector_tool)
{
  GimpStroke *cur_stroke = NULL;
  GimpAnchor *cur_anchor;
  GList      *anchors;
  GList      *list;
  gboolean    cur_anchor_valid;
  gboolean    cur_stroke_valid;

  cur_anchor_valid = FALSE;
  cur_stroke_valid = FALSE;

  vector_tool->sel_count  = 0;
  vector_tool->sel_anchor = NULL;
  vector_tool->sel_stroke = NULL;

  if (!vector_tool->vectors)
    {
      vector_tool->cur_position = -1;
      vector_tool->cur_anchor   = NULL;
      vector_tool->cur_stroke   = NULL;
      return;
    }

  while ((cur_stroke = gimp_vectors_stroke_get_next (vector_tool->vectors,
                                                     cur_stroke)))
    {
      /* anchor handles */
      anchors = gimp_stroke_get_draw_anchors (cur_stroke);

      if (cur_stroke == vector_tool->cur_stroke)
        cur_stroke_valid = TRUE;

      for (list = anchors; list; list = g_list_next (list))
        {
          cur_anchor = GIMP_ANCHOR (list->data);

          if (cur_anchor == vector_tool->cur_anchor)
            cur_anchor_valid = TRUE;

          if (cur_anchor->type == GIMP_ANCHOR_ANCHOR &&
              cur_anchor->selected)
            {
              vector_tool->sel_count++;
              if (vector_tool->sel_count == 1)
                {
                  vector_tool->sel_anchor = cur_anchor;
                  vector_tool->sel_stroke = cur_stroke;
                }
              else
                {
                  vector_tool->sel_anchor = NULL;
                  vector_tool->sel_stroke = NULL;
                }
            }
        }

      anchors = gimp_stroke_get_draw_controls (cur_stroke);

      for (list = anchors; list; list = g_list_next (list))
        {
          cur_anchor = GIMP_ANCHOR (list->data);

          if (cur_anchor == vector_tool->cur_anchor)
            cur_anchor_valid = TRUE;
        }
    }

  if (!cur_stroke_valid)
    vector_tool->cur_stroke = NULL;

  if (!cur_anchor_valid)
    vector_tool->cur_anchor = NULL;

}

static void
gimp_vector_tool_undo_push (GimpVectorTool *vector_tool, const gchar *desc)
{
  g_return_if_fail (vector_tool->vectors != NULL);

  /* don't push two undos */
  if (vector_tool->have_undo)
    return;

  gimp_image_undo_push_vectors_mod (GIMP_ITEM (vector_tool->vectors)->gimage,
                                    desc, vector_tool->vectors);
  vector_tool->have_undo = TRUE;
}


static void
gimp_vector_tool_to_selection (GimpVectorTool *vector_tool)
{
  gimp_vector_tool_to_selection_extended (vector_tool, 0);
}


static void
gimp_vector_tool_to_selection_extended (GimpVectorTool *vector_tool,
                                        gint            state)
{
  GimpImage    *gimage;
  GimpChannelOps operation = GIMP_CHANNEL_OP_REPLACE;

  if (! vector_tool->vectors)
    return;

  gimage = gimp_item_get_image (GIMP_ITEM (vector_tool->vectors));

  if (state & GDK_SHIFT_MASK)
    {
      if (state & GDK_CONTROL_MASK)
        operation = GIMP_CHANNEL_OP_INTERSECT;
      else
        operation = GIMP_CHANNEL_OP_ADD;
    }
  else if (state & GDK_CONTROL_MASK)
    {
      operation = GIMP_CHANNEL_OP_SUBTRACT;
    }

  gimp_channel_select_vectors (gimp_image_get_mask (gimage),
                               _("Path to selection"),
                               vector_tool->vectors,
                               operation,
                               TRUE, FALSE, 0, 0);
  gimp_image_flush (gimage);
}


static void
gimp_vector_tool_stroke_vectors (GimpVectorTool *vector_tool,
                                 GtkWidget      *button)
{
  GimpImage    *gimage;
  GimpDrawable *active_drawable;
  GtkWidget    *dialog;

  if (! vector_tool->vectors)
    return;

  gimage = gimp_item_get_image (GIMP_ITEM (vector_tool->vectors));

  active_drawable = gimp_image_active_drawable (gimage);

  if (! active_drawable)
    {
      g_message (_("There is no active layer or channel to stroke to"));
      return;
    }

  dialog = stroke_dialog_new (GIMP_ITEM (vector_tool->vectors),
                              _("Stroke Path"),
                              GIMP_STOCK_PATH_STROKE,
                              GIMP_HELP_PATH_STROKE,
                              button);
  gtk_widget_show (dialog);
}

