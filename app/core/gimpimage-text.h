/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattisbvf
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

#ifndef __GIMP_IMAGE_TEXT_H__
#define __GIMP_IMAGE_TEXT_H__

GimpLayer * text_render      (GimpImage    *gimage,
			      GimpDrawable *drawable,
			      gint          text_x,
			      gint          text_y,
			      const gchar  *fontname,
			      const gchar  *text,
			      gint          border,
			      gint          antialias);
gboolean    text_get_extents (const gchar  *fontname,
                              const gchar  *text,
                              gint         *width,
                              gint         *height,
                              gint         *ascent,
                              gint         *descent);

#endif /* __GIMP_IMAGE_TEXT_H__ */
