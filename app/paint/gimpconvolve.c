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

#include <glib-object.h>

#include "paint-types.h"

#include "base/pixel-region.h"
#include "base/temp-buf.h"

#include "paint-funcs/paint-funcs.h"

#include "core/gimp.h"
#include "core/gimpbrush.h"
#include "core/gimpdrawable.h"
#include "core/gimpimage.h"

#include "gimpconvolve.h"
#include "gimpconvolveoptions.h"

#include "gimp-intl.h"


#define FIELD_COLS    4
#define MIN_BLUR      64         /*  (8/9 original pixel)   */
#define MAX_BLUR      0.25       /*  (1/33 original pixel)  */
#define MIN_SHARPEN   -512
#define MAX_SHARPEN   -64

/* Different clip relationships between a blur-blob and edges:
 * see convolve_motion
 */
typedef enum
{
  CONVOLVE_NCLIP,       /* Left or top edge     */
  CONVOLVE_NOT_CLIPPED, /* No edges             */
  CONVOLVE_PCLIP        /* Right or bottom edge */
} ConvolveClipType;


static void   gimp_convolve_class_init       (GimpConvolveClass   *klass);
static void   gimp_convolve_init             (GimpConvolve        *convolve);

static void   gimp_convolve_paint            (GimpPaintCore       *paint_core,
                                              GimpDrawable        *drawable,
                                              GimpPaintOptions    *paint_options,
                                              GimpPaintCoreState   paint_state);
static void   gimp_convolve_motion           (GimpPaintCore       *paint_core,
                                              GimpDrawable        *drawable,
                                              GimpPaintOptions    *paint_options);

static void   gimp_convolve_calculate_matrix (GimpConvolveType     type,
                                              gdouble              rate);
static void   gimp_convolve_integer_matrix   (gfloat              *source,
                                              gint                *dest,
                                              gint                 size);
static void   gimp_convolve_copy_matrix      (gfloat              *src,
                                              gfloat              *dest,
                                              gint                 size);
static gint   gimp_convolve_sum_matrix       (gint                *matrix,
                                              gint                 size);


static gint   matrix [25];
static gint   matrix_size;
static gint   matrix_divisor;

static gfloat custom_matrix [25] =
{
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 1, 0, 0,
  0, 0, 0, 0, 0,
  0, 0, 0, 0, 0,
};

static gfloat blur_matrix [25] =
{
  0, 0, 0, 0, 0,
  0, 1, 1, 1, 0,
  0, 1, MIN_BLUR, 1, 0,
  0, 1, 1, 1, 0,
  0, 0 ,0, 0, 0,
};

static gfloat sharpen_matrix [25] =
{
  0, 0, 0, 0, 0,
  0, 1, 1, 1, 0,
  0, 1, MIN_SHARPEN, 1, 0,
  0, 1, 1, 1, 0,
  0, 0, 0, 0, 0,
};

static GimpPaintCoreClass *parent_class;


void
gimp_convolve_register (Gimp                      *gimp,
                        GimpPaintRegisterCallback  callback)
{
  (* callback) (gimp,
                GIMP_TYPE_CONVOLVE,
                GIMP_TYPE_CONVOLVE_OPTIONS,
                _("Convolve"));
}

GType
gimp_convolve_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      static const GTypeInfo info =
      {
        sizeof (GimpConvolveClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) gimp_convolve_class_init,
	NULL,           /* class_finalize */
	NULL,           /* class_data     */
	sizeof (GimpConvolve),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) gimp_convolve_init,
      };

      type = g_type_register_static (GIMP_TYPE_PAINT_CORE,
                                     "GimpConvolve",
                                     &info, 0);
    }

  return type;
}

static void
gimp_convolve_class_init (GimpConvolveClass *klass)
{
  GimpPaintCoreClass *paint_core_class;

  paint_core_class = GIMP_PAINT_CORE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  paint_core_class->paint = gimp_convolve_paint;
}

static void
gimp_convolve_init (GimpConvolve *convolve)
{
}

static void
gimp_convolve_paint (GimpPaintCore      *paint_core,
                     GimpDrawable       *drawable,
                     GimpPaintOptions   *paint_options,
                     GimpPaintCoreState  paint_state)
{
  switch (paint_state)
    {
    case MOTION_PAINT:
      gimp_convolve_motion (paint_core, drawable, paint_options);
      break;

    default:
      break;
    }
}

static void
gimp_convolve_motion (GimpPaintCore    *paint_core,
                      GimpDrawable     *drawable,
                      GimpPaintOptions *paint_options)
{
  GimpConvolveOptions *options;
  GimpPressureOptions *pressure_options;
  GimpContext         *context;
  GimpImage           *gimage;
  TempBuf             *area;
  guchar              *temp_data;
  PixelRegion          srcPR;
  PixelRegion          destPR;
  gdouble              opacity;
  gdouble              scale;
  gdouble              rate;
  ConvolveClipType     area_hclip = CONVOLVE_NOT_CLIPPED;
  ConvolveClipType     area_vclip = CONVOLVE_NOT_CLIPPED;
  gint                 marginx    = 0;
  gint                 marginy    = 0;

  options = GIMP_CONVOLVE_OPTIONS (paint_options);
  context = GIMP_CONTEXT (paint_options);

  pressure_options = paint_options->pressure_options;

  if (! (gimage = gimp_item_get_image (GIMP_ITEM (drawable))))
    return;

  if (gimp_drawable_is_indexed (drawable))
    return;

  /* If the brush is smaller than the convolution matrix, don't convolve */
  if (paint_core->brush->mask->width  < matrix_size ||
      paint_core->brush->mask->height < matrix_size)
    return;

  opacity = gimp_paint_options_get_fade (paint_options, gimage,
                                         paint_core->pixel_dist);

  if (opacity == 0.0)
    return;

  if (pressure_options->size)
    scale = paint_core->cur_coords.pressure;
  else
    scale = 1.0;

  if (! (area = gimp_paint_core_get_paint_area (paint_core, drawable, scale)))
    return;

  /*  configure the source pixel region  */
  pixel_region_init (&srcPR, gimp_drawable_data (drawable),
		     area->x, area->y, area->width, area->height, FALSE);

  /*  configure the destination pixel region  */
  destPR.bytes     = area->bytes;
  destPR.tiles     = NULL;
  destPR.x         = 0;
  destPR.y         = 0;
  destPR.w         = area->width;
  destPR.h         = area->height;
  destPR.rowstride = area->width * destPR.bytes;
  destPR.data      = temp_buf_data (area);

  rate = options->rate;

  if (pressure_options->rate)
    rate *= 2.0 * paint_core->cur_coords.pressure;

  gimp_convolve_calculate_matrix (options->type, rate);

  /*  Image region near edges? If so, paint area will be clipped   */
  /*  with respect to brush mask + 1 pixel border (# 19285)        */

  if ((marginx = (gint) paint_core->cur_coords.x - paint_core->brush->mask->width / 2 - 1) != area->x)
    area_hclip = CONVOLVE_NCLIP;
  else if ((marginx = area->width - paint_core->brush->mask->width - 2) != 0)
    area_hclip = CONVOLVE_PCLIP;

  if ((marginy = (gint) paint_core->cur_coords.y - paint_core->brush->mask->height / 2 - 1) != area->y)
    area_vclip = CONVOLVE_NCLIP;
  else if ((marginy = area->height - paint_core->brush->mask->height - 2) != 0)
    area_vclip = CONVOLVE_PCLIP;

  /*  Has the TempBuf been clipped by a canvas edge or two?  */
  if (area_hclip == CONVOLVE_NOT_CLIPPED &&
      area_vclip == CONVOLVE_NOT_CLIPPED)
    {
      /* No clipping...                                              */
      /* Standard case: copy src to temp. convolve temp to dest.     */
      /* Brush defines pipe size and no edge adjustments are needed. */

      /*  If the source has no alpha, then add alpha pixels          */
      /*  Because paint_core.c is alpha-only code. See below.        */

      PixelRegion  tempPR;

      tempPR.x     = 0;
      tempPR.y     = 0;
      tempPR.w     = area->width;
      tempPR.h     = area->height;
      tempPR.tiles = NULL;

      if (! gimp_drawable_has_alpha (drawable))
	{
	  /* note: this particular approach needlessly convolves the totally-
	     opaque alpha channel. A faster approach would be to keep
	     tempPR the same number of bytes as srcPR, and extend the
	     paint_core_replace_canvas API to handle non-alpha images. */

	  tempPR.bytes     = srcPR.bytes + 1;
	  tempPR.rowstride = tempPR.bytes * tempPR.w;
	  temp_data        = g_malloc (tempPR.h * tempPR.rowstride);
	  tempPR.data      = temp_data;
	  add_alpha_region (&srcPR, &tempPR);
	}
      else
	{
	  tempPR.bytes     = srcPR.bytes;
	  tempPR.rowstride = tempPR.bytes * tempPR.w;
	  temp_data        = g_malloc (tempPR.h * tempPR.rowstride);
	  tempPR.data      = temp_data;
	  copy_region (&srcPR, &tempPR);
	}

      /*  Convolve the region  */

      tempPR.x    = 0;
      tempPR.y    = 0;
      tempPR.w    = area->width;
      tempPR.h    = area->height;
      tempPR.data = temp_data;

      convolve_region (&tempPR, &destPR, matrix, matrix_size,
		       matrix_divisor, GIMP_NORMAL_CONVOL);

      /*  Free the allocated temp space  */
      g_free (temp_data);
    }
  else
    {
      /* TempBuf clipping has occured on at least one edge...
       * Edge case: expand area under brush margin px on near edge(s), convolve
       * expanded buffers. copy src -> ovrsz1 convolve ovrsz1 -> ovrsz2
       * copy-with-crop ovrsz2 -> dest
       */
      PixelRegion  ovrsz1PR;
      PixelRegion  ovrsz2PR;
      guchar      *ovrsz1_data = NULL;
      guchar      *ovrsz2_data = NULL;
      guchar      *fillcolor;

      fillcolor = gimp_drawable_get_color_at
	(drawable,
	 CLAMP ((gint) paint_core->cur_coords.x,
                0, gimp_item_width  (GIMP_ITEM (drawable)) - 1),
	 CLAMP ((gint) paint_core->cur_coords.y,
                0, gimp_item_height (GIMP_ITEM (drawable)) - 1));

      marginx *= (marginx < 0) ? -1 : 0;
      marginy *= (marginy < 0) ? -1 : 0;

      ovrsz2PR.x         = 0;
      ovrsz2PR.y         = 0;
      ovrsz2PR.w         = area->width  + marginx;
      ovrsz2PR.h         = area->height + marginy;
      ovrsz2PR.bytes     = (gimp_drawable_has_alpha (drawable) ?
                            srcPR.bytes : srcPR.bytes + 1);
      ovrsz2PR.offx      = 0;
      ovrsz2PR.offy      = 0;
      ovrsz2PR.rowstride = ovrsz2PR.bytes * ovrsz2PR.w;
      ovrsz2PR.tiles     = NULL;
      ovrsz2_data        = g_malloc (ovrsz2PR.h * ovrsz2PR.rowstride);
      ovrsz2PR.data      = ovrsz2_data;

      ovrsz1PR.x         = 0;
      ovrsz1PR.y         = 0;
      ovrsz1PR.w         = area->width  + marginx;
      ovrsz1PR.h         = area->height + marginy;
      ovrsz1PR.bytes     = (gimp_drawable_has_alpha (drawable) ?
                            srcPR.bytes : srcPR.bytes + 1);
      ovrsz1PR.offx      = 0;
      ovrsz1PR.offy      = 0;
      ovrsz1PR.rowstride = ovrsz2PR.bytes * ovrsz2PR.w;
      ovrsz1PR.tiles     = NULL;
      ovrsz1_data        = g_malloc (ovrsz1PR.h * ovrsz1PR.rowstride);
      ovrsz1PR.data      = ovrsz1_data;

      color_region (&ovrsz1PR, fillcolor);

      ovrsz1PR.x         = (area_hclip == CONVOLVE_NCLIP)? marginx : 0;
      ovrsz1PR.y         = (area_vclip == CONVOLVE_NCLIP)? marginy : 0;
      ovrsz1PR.w         = area->width;
      ovrsz1PR.h         = area->height;
      ovrsz1PR.data      = (ovrsz1_data +
                            (ovrsz1PR.rowstride * ovrsz1PR.y) +
                            (ovrsz1PR.bytes * ovrsz1PR.x));

      if (! gimp_drawable_has_alpha (drawable))
	add_alpha_region (&srcPR, &ovrsz1PR);
      else
	copy_region (&srcPR, &ovrsz1PR);

      /*  Convolve the region  */

      ovrsz1PR.x    = 0;
      ovrsz1PR.y    = 0;
      ovrsz1PR.w    = area->width  + marginx;
      ovrsz1PR.h    = area->height + marginy;
      ovrsz1PR.data = ovrsz1_data;

      convolve_region (&ovrsz1PR, &ovrsz2PR, matrix, matrix_size,
		       matrix_divisor, GIMP_NORMAL_CONVOL);

      /* Crop and copy to destination */

      ovrsz2PR.x    = (area_hclip == CONVOLVE_NCLIP)? marginx : 0;
      ovrsz2PR.y    = (area_vclip == CONVOLVE_NCLIP)? marginy : 0;
      ovrsz2PR.w    = area->width;
      ovrsz2PR.h    = area->height;
      ovrsz2PR.data = (ovrsz2_data +
                       (ovrsz2PR.rowstride * ovrsz2PR.y) +
                       (ovrsz2PR.bytes * ovrsz2PR.x));

      copy_region (&ovrsz2PR, &destPR);

      g_free (ovrsz1_data);
      g_free (ovrsz2_data);
      g_free (fillcolor);
    }

  /*  paste the newly painted canvas to the gimage which is being worked on  */
  gimp_paint_core_replace_canvas (paint_core, drawable,
                                  MIN (opacity, GIMP_OPACITY_OPAQUE),
				  gimp_context_get_opacity (context),
				  gimp_paint_options_get_brush_mode (paint_options),
                                  scale,
                                  GIMP_PAINT_INCREMENTAL);
}

static void
gimp_convolve_calculate_matrix (GimpConvolveType type,
                                gdouble          rate)
{
  gfloat percent;

  /*  find percent of tool pressure  */
  percent = MIN (rate / 100.0, 1.0);

  /*  get the appropriate convolution matrix and size and divisor  */
  switch (type)
    {
    case GIMP_BLUR_CONVOLVE:
      matrix_size = 5;
      blur_matrix [12] = MIN_BLUR + percent * (MAX_BLUR - MIN_BLUR);
      gimp_convolve_copy_matrix (blur_matrix, custom_matrix, matrix_size);
      break;

    case GIMP_SHARPEN_CONVOLVE:
      matrix_size = 5;
      sharpen_matrix [12] = MIN_SHARPEN + percent * (MAX_SHARPEN - MIN_SHARPEN);
      gimp_convolve_copy_matrix (sharpen_matrix, custom_matrix, matrix_size);
      break;

    case GIMP_CUSTOM_CONVOLVE:
      matrix_size = 5;
      break;
    }

  gimp_convolve_integer_matrix (custom_matrix, matrix, matrix_size);
  matrix_divisor = gimp_convolve_sum_matrix (matrix, matrix_size);

  if (!matrix_divisor)
    matrix_divisor = 1;
}

static void
gimp_convolve_integer_matrix (gfloat *source,
                              gint   *dest,
                              gint    size)
{
  gint i;

#define PRECISION  10000

  for (i = 0; i < size*size; i++)
    *dest++ = (gint) (*source ++ * PRECISION);
}

static void
gimp_convolve_copy_matrix (gfloat *src,
                           gfloat *dest,
                           gint    size)
{
  gint i;

  for (i = 0; i < size*size; i++)
    *dest++ = *src++;
}

static gint
gimp_convolve_sum_matrix (gint *matrix,
                          gint  size)
{
  gint sum = 0;

  size *= size;

  while (size --)
    sum += *matrix++;

  return sum;
}
