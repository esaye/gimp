/* The GIMP -- an image manipulation program
 * Copyright (C) 1995-1997 Spencer Kimball and Peter Mattis
 *
 * gimplist.h
 * Copyright (C) 2001 Michael Natterer <mitch@gimp.org>
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

#ifndef __GIMP_LIST_H__
#define __GIMP_LIST_H__


#include "gimpcontainer.h"


#define GIMP_TYPE_LIST            (gimp_list_get_type ())
#define GIMP_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIMP_TYPE_LIST, GimpList))
#define GIMP_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GIMP_TYPE_LIST, GimpListClass))
#define GIMP_IS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIMP_TYPE_LIST))
#define GIMP_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GIMP_TYPE_LIST))


typedef struct _GimpListClass GimpListClass;

struct _GimpList
{
  GimpContainer  parent_instance;

  GList         *list;
};

struct _GimpListClass
{
  GimpContainerClass  parent_class;
};


GtkType         gimp_list_get_type (void);
GimpContainer * gimp_list_new      (GtkType              children_type,
				    GimpContainerPolicy  policy);


#endif  /* __GIMP_LIST_H__ */
