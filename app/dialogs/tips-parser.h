/* The GIMP -- an image manipulation program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * tips-parser.h -- Parse the gimp-tips.xml file.
 * Copyright (C) 2002  Sven Neumann <sven@gimp.org>
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

#ifndef __TIPS_PARSER_H__
#define __TIPS_PARSER_H__


typedef struct _GimpTip GimpTip;

struct _GimpTip
{
  gchar *welcome;
  gchar *thetip;
};


GimpTip * gimp_tip_new        (const gchar  *welcome,
                               const gchar  *thetip);
void      gimp_tip_free       (GimpTip      *tip);

GList   * gimp_tips_from_file (const gchar  *filename,
                               const gchar  *locale,
                               GError      **error);
void      gimp_tips_free      (GList        *tips);


#endif /* __TIPS_PARSER_H__ */
