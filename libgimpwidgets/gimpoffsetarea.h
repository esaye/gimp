/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * gimpoffsetarea.h
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org> 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GIMP_OFFSET_AREA_H__
#define __GIMP_OFFSET_AREA_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* For information look into the C source or the html documentation */

#define GIMP_TYPE_OFFSET_AREA            (gimp_offset_area_get_type ())
#define GIMP_OFFSET_AREA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_OFFSET_AREA, GimpOffsetArea))
#define GIMP_OFFSET_AREA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_OFFSET_AREA, GimpOffsetAreaClass))
#define GIMP_IS_OFFSET_AREA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_OFFSET_AREA))
#define GIMP_IS_OFFSET_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_OFFSET_AREA))


typedef struct _GimpOffsetAreaClass  GimpOffsetAreaClass;

struct _GimpOffsetArea
{
  GtkDrawingArea  parent_instance;

  gint            orig_width;
  gint            orig_height;
  gint            width;
  gint            height;
  gint            offset_x;
  gint            offset_y;
  gdouble         display_ratio_x;
  gdouble         display_ratio_y;  
};

struct _GimpOffsetAreaClass
{
  GtkDrawingAreaClass  parent_class;

  void (* offsets_changed)  (GimpOffsetArea *offset_area, 
                             gint            offset_x, 
                             gint            offset_y);
};


GtkType     gimp_offset_area_get_type    (void);
GtkWidget * gimp_offset_area_new         (gint            orig_width,
                                          gint            orig_height);
void        gimp_offset_area_set_size    (GimpOffsetArea *offset_area,
                                          gint            width,
                                          gint            height);
void        gimp_offset_area_set_offsets (GimpOffsetArea *offset_area,
                                          gint            offset_x,
                                          gint            offset_y);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GIMP_OFFSET_AREA_H__ */
