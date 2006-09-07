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

#include <string.h>

#include <glib-object.h>

#include "libgimpbase/gimpbase.h"

#include "paint-types.h"

#include "paint-funcs/paint-funcs.h"

#include "base/pixel-region.h"
#include "base/temp-buf.h"
#include "base/tile-manager.h"

#include "core/gimppickable.h"
#include "core/gimpimage.h"
#include "core/gimpdrawable.h"

#include "gimpheal.h"
#include "gimpsourceoptions.h"

#include "gimp-intl.h"


/* NOTES:
 *
 * I had the code working for healing from a pattern, but the results look
 * terrible and I can't see a use for it right now.
 */


static void   gimp_heal_motion (GimpSourceCore   *source_core,
                                GimpDrawable     *drawable,
                                GimpPaintOptions *paint_options,
                                gdouble           opacity,
                                GimpImage        *src_image,
                                GimpPickable     *src_pickable,
                                PixelRegion      *srcPR,
                                TempBuf          *paint_area,
                                gint              paint_area_offset_x,
                                gint              paint_area_offset_y,
                                gint              paint_area_width,
                                gint              paint_area_height);


G_DEFINE_TYPE (GimpHeal, gimp_heal, GIMP_TYPE_SOURCE_CORE)

#define parent_class gimp_heal_parent_class


void
gimp_heal_register (Gimp                      *gimp,
                    GimpPaintRegisterCallback  callback)
{
  (* callback) (gimp,
                GIMP_TYPE_HEAL,
                GIMP_TYPE_SOURCE_OPTIONS,
                "gimp-heal",
                _("Heal"),
                "gimp-tool-heal");
}

static void
gimp_heal_class_init (GimpHealClass *klass)
{
  GimpSourceCoreClass *source_core_class = GIMP_SOURCE_CORE_CLASS (klass);

  source_core_class->motion = gimp_heal_motion;
}

static void
gimp_heal_init (GimpHeal *heal)
{
}

/*
 * Substitute any zeros in the input PixelRegion for ones.  This is needed by
 * the algorithm to avoid division by zero, and to get a more realistic image
 * since multiplying by zero is often incorrect (i.e., healing from a dark to
 * a light region will have incorrect spots of zero)
 */
static void
gimp_heal_substitute_0_for_1 (PixelRegion *pr)
{
  gint     i, j, k;

  gint     height = pr->h;
  gint     width  = pr->w;
  gint     depth  = pr->bytes;

  guchar  *pr_data = pr->data;

  guchar  *p;

  for (i = 0; i < height; i++)
    {
      p = pr_data;

      for (j = 0; j < width; j++)
        {
          for (k = 0; k < depth; k++)
            {
              if (p[k] == 0)
                p[k] = 1;
            }

          p += depth;
        }

      pr_data += pr->rowstride;
    }
}

/*
 * Divide topPR by bottomPR and store the result as a double
 */
static void
gimp_heal_divide (PixelRegion *topPR,
                  PixelRegion *bottomPR,
                  gdouble     *result)
{
  gint     i, j, k;

  gint     height = topPR->h;
  gint     width  = topPR->w;
  gint     depth  = topPR->bytes;

  guchar  *t_data = topPR->data;
  guchar  *b_data = bottomPR->data;

  guchar  *t;
  guchar  *b;
  gdouble *r      = result;

  for (i = 0; i < height; i++)
    {
      t = t_data;
      b = b_data;

      for (j = 0; j < width; j++)
        {
          for (k = 0; k < depth; k++)
            {
              r[k] = (gdouble) (t[k]) / (gdouble) (b[k]);
            }

          t += depth;
          b += depth;
          r += depth;
        }

      t_data += topPR->rowstride;
      b_data += bottomPR->rowstride;
    }
}

/*
 * multiply first by secondPR and store the result as a PixelRegion
 */
static void
gimp_heal_multiply (gdouble     *first,
                    PixelRegion *secondPR,
                    PixelRegion *resultPR)
{
  gint     i, j, k;

  gint     height = secondPR->h;
  gint     width  = secondPR->w;
  gint     depth  = secondPR->bytes;

  guchar  *s_data = secondPR->data;
  guchar  *r_data = resultPR->data;

  gdouble *f      = first;
  guchar  *s;
  guchar  *r;

  for (i = 0; i < height; i++)
    {
      s = s_data;
      r = r_data;

      for (j = 0; j < width; j++)
        {
          for (k = 0; k < depth; k++)
            {
              r[k] = (guchar) CLAMP0255 (ROUND (((gdouble) (f[k])) * ((gdouble) (s[k]))));
            }

          f += depth;
          s += depth;
          r += depth;
        }

      s_data += secondPR->rowstride;
      r_data += resultPR->rowstride;
    }
}

/*
 * Perform one iteration of the laplace solver for matrix.  Store the result in
 * solution and return the cummulative error of the solution.
 */
gdouble
gimp_heal_laplace_iteration (gdouble *matrix,
                             gint     height,
                             gint     depth,
                             gint     width,
                             gdouble *solution)
{
  gint     rowstride  = width * depth;
  gint     i, j, k;
  gdouble  err        = 0.0;
  gdouble  tmp, diff;

  for (i = 0; i < height; i++)
    {
      for (j = 0; j < width; j++)
        {
          if ((i == 0) || (i == (height - 1)) ||
              (j == 0) || (j == (height - 1)))
            {
              /* do nothing at the boundary */
              for (k = 0; k < depth; k++)
                *(solution + k) = *(matrix + k);
            }
          else
            {
              /* calculate the mean of four neighbours for all color channels */
              /* v[i][j] = 0.45 * (v[i][j-1]+v[i][j+1]+v[i-1][j]+v[i+1][j]) */
              for (k = 0; k < depth; k++)
                {
                  tmp = *(solution + k);

                  *(solution + k) = 0.25 *
                                    ( *(matrix - depth + k)       /* west */
                                    + *(matrix + depth + k)       /* east */
                                    + *(matrix - rowstride + k)   /* north */
                                    + *(matrix + rowstride + k)); /* south */

                  diff = *(solution + k) - tmp;
                  err += diff*diff;
                }
            }

          /* advance pointers to data */
          matrix += depth;
          solution += depth;
        }
    }

  return sqrt (err);
}

/*
 * Solve the laplace equation for matrix and store the result in solution.
 */
void
gimp_heal_laplace_loop (gdouble *matrix,
                        gint     height,
                        gint     depth,
                        gint     width,
                        gdouble *solution)
{
#define EPSILON   0.0001
#define MAX_ITER  500

  gint num_iter = 0;
  gdouble err;

  /* do one iteration and store the amount of error */
  err = gimp_heal_laplace_iteration (matrix, height, depth, width, solution);

  /* copy solution to matrix */
  memcpy (matrix, solution, width * height * depth * sizeof(double));

  /* repeat until convergence or max iterations */
  while (err > EPSILON)
    {
      err = gimp_heal_laplace_iteration (matrix, height, depth, width, solution);
      memcpy (matrix, solution, width * height * depth * sizeof(double));

      num_iter++;

      if (num_iter >= MAX_ITER)
        break;
    }
}

/*
 * Algorithm Design:
 *
 * T. Georgiev, "Image Reconstruction Invariant to Relighting", EUROGRAPHICS
 * 2005, http://www.tgeorgiev.net/
 */
PixelRegion *
gimp_heal_region (PixelRegion *tempPR,
                  PixelRegion *srcPR)
{
  gdouble *i_1 = g_new (gdouble, tempPR->h * tempPR->bytes * tempPR->w);
  gdouble *i_2 = g_new (gdouble, tempPR->h * tempPR->bytes * tempPR->w);

  /* substitute 0's for 1's for the division and multiplication operations that
   * come later
   */
  gimp_heal_substitute_0_for_1 (srcPR);

  /* divide tempPR by srcPR and store the result as a double in i_1 */
  gimp_heal_divide (tempPR, srcPR, i_1);

  /* FIXME: is a faster implementation needed? */
  gimp_heal_laplace_loop (i_1, tempPR->h, tempPR->bytes, tempPR->w, i_2);

  /* multiply a double by srcPR and store in tempPR */
  gimp_heal_multiply (i_2, srcPR, tempPR);

  g_free (i_1);
  g_free (i_2);

  return tempPR;
}

static void
gimp_heal_motion (GimpSourceCore   *source_core,
                  GimpDrawable     *drawable,
                  GimpPaintOptions *paint_options,
                  gdouble           opacity,
                  GimpImage        *src_image,
                  GimpPickable     *src_pickable,
                  PixelRegion      *srcPR,
                  TempBuf          *paint_area,
                  gint              paint_area_offset_x,
                  gint              paint_area_offset_y,
                  gint              paint_area_width,
                  gint              paint_area_height)
{
  GimpPaintCore *paint_core = GIMP_PAINT_CORE (source_core);
  GimpContext   *context    = GIMP_CONTEXT (paint_options);
  TempBuf       *src;
  TempBuf       *temp;
  PixelRegion    origPR;
  PixelRegion    tempPR;
  PixelRegion    destPR;
  GimpImageType  src_type;

  if (gimp_drawable_is_indexed (drawable))
    {
      g_message (_("Indexed images are not currently supported."));
      return;
    }

  src_type = gimp_pickable_get_image_type (src_pickable);

  /* we need the source area with alpha and we modify it, so make a copy */
  src = temp_buf_new (srcPR->w, srcPR->h,
                      GIMP_IMAGE_TYPE_BYTES (GIMP_IMAGE_TYPE_WITH_ALPHA (src_type)),
                      0, 0, NULL);
  pixel_region_init_temp_buf (&tempPR, src, 0, 0, src->width, src->height);

  if (GIMP_IMAGE_TYPE_HAS_ALPHA (src_type))
    copy_region (srcPR, &tempPR);
  else
    add_alpha_region (srcPR, &tempPR);

  /* reinitialize srcPR */
  pixel_region_init_temp_buf (srcPR, src, 0, 0, src->width, src->height);

  /* FIXME: the area under the cursor and the source area should be x% larger
   * than the brush size.  Otherwise the brush must be a lot bigger than the
   * area to heal to get good results.  Having the user pick such a large brush
   * is perhaps counter-intutitive?
   */

  /* Get the area underneath the cursor */
  {
    TempBuf *orig;
    gint     x1, x2, y1, y2;

    x1 = paint_area->x + paint_area_offset_x;
    y1 = paint_area->y + paint_area_offset_y;
    x2 = x1 + paint_area_width;
    y2 = y1 + paint_area_height;

    /* get the original image data at the cursor location */
    orig = gimp_paint_core_get_orig_image (paint_core, drawable,
                                           x1, y1, x2, y2);

    pixel_region_init_temp_buf (&origPR, orig, 0, 0,
                                paint_area_width, paint_area_height);
  }

  temp = temp_buf_new (origPR.w, origPR.h,
                       gimp_drawable_bytes_with_alpha (drawable),
                       0, 0, NULL);
  pixel_region_init_temp_buf (&tempPR, temp, 0, 0, temp->width, temp->height);

  if (gimp_drawable_has_alpha (drawable))
    copy_region (&origPR, &tempPR);
  else
    add_alpha_region (&origPR, &tempPR);

  if (tempPR.bytes != srcPR->bytes)
    {
      GimpImage *image = gimp_item_get_image (GIMP_ITEM (drawable));
      TempBuf   *temp2;
      gboolean   new_buf;

      temp2 = gimp_image_transform_temp_buf (image, drawable,
                                             temp, &new_buf);

      if (new_buf)
        temp_buf_free (temp);

      temp = temp2;
    }

  /* reinitialize tempPR */
  pixel_region_init_temp_buf (&tempPR, temp, 0, 0, temp->width, temp->height);

  /* now tempPR holds the data under the cursor and
   * srcPR holds the area to sample from
   */

  /* get the destination to paint to */
  pixel_region_init_temp_buf (&destPR, paint_area,
                              paint_area_offset_x, paint_area_offset_y,
                              paint_area_width, paint_area_height);

  /* check that srcPR, tempPR, and destPR are the same size */
  if ((srcPR->w != tempPR.w) || (srcPR->w != destPR.w) ||
      (srcPR->h != tempPR.h) || (srcPR->h != destPR.h))
    {
      g_printerr ("%s: Error: Source and destination are not the same size.",
                  G_STRFUNC);
      temp_buf_free (src);
      temp_buf_free (temp);
      return;
    }

  /* heal tempPR using srcPR */
  gimp_heal_region (&tempPR, srcPR);

  /* reinitialize tempPR */
  pixel_region_init_temp_buf (&tempPR, temp, 0, 0, temp->width, temp->height);

  copy_region (&tempPR, &destPR);

  /* check the brush pressure */
  if (paint_options->pressure_options->opacity)
    opacity *= PRESSURE_SCALE * paint_core->cur_coords.pressure;

  /* replace the canvas with our healed data */
  gimp_brush_core_replace_canvas (GIMP_BRUSH_CORE (paint_core),
                                  drawable,
                                  MIN (opacity, GIMP_OPACITY_OPAQUE),
                                  gimp_context_get_opacity (context),
                                  gimp_paint_options_get_brush_mode (paint_options),
                                  GIMP_PAINT_CONSTANT);

  temp_buf_free (src);
  temp_buf_free (temp);
}
