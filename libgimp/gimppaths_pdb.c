/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-2000 Peter Mattis and Spencer Kimball
 *
 * gimppaths_pdb.c
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

/* NOTE: This file is autogenerated by pdbgen.pl */

#include "config.h"

#include <string.h>

#include "gimp.h"

/**
 * gimp_path_list:
 * @image_ID: The ID of the image to list the paths from.
 * @num_paths: The number of paths returned.
 *
 * List the paths associated with the passed image.
 *
 * List the paths associated with the passed image.
 *
 * Returns: List of the paths belonging to this image.
 */
gchar **
gimp_path_list (gint32  image_ID,
		gint   *num_paths)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gchar **path_list = NULL;
  gint i;

  return_vals = gimp_run_procedure ("gimp_path_list",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  *num_paths = 0;

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      *num_paths = return_vals[1].data.d_int32;
      path_list = g_new (gchar *, *num_paths);
      for (i = 0; i < *num_paths; i++)
	path_list[i] = g_strdup (return_vals[2].data.d_stringarray[i]);
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return path_list;
}

/**
 * gimp_path_get_points:
 * @image_ID: The ID of the image to list the paths from.
 * @pathname: the name of the path whose points should be listed.
 * @path_closed: Return if the path is closed. {0=path open, 1= path closed}.
 * @num_path_point_details: The number of points returned. Each point is made up of (x,y,pnt_type) of floats.
 * @points_pairs: The points in the path represented as 3 floats. The first is the x pos, next is the y pos, last is the type of the pnt. The type field is dependant on the path type. For beziers (type 1 paths) the type can either be {1.0= BEZIER_ANCHOR, 2.0= BEZIER_CONTROL}. Note all points are returned in pixel resolution.
 *
 * List the points associated with the named path.
 *
 * List the points associated with the named path.
 *
 * Returns: The type of the path. Currently only one type (1 = Bezier) is supported.
 */
gint
gimp_path_get_points (gint32    image_ID,
		      gchar    *pathname,
		      gint     *path_closed,
		      gint     *num_path_point_details,
		      gdouble **points_pairs)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint path_type = 0;

  return_vals = gimp_run_procedure ("gimp_path_get_points",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, pathname,
				    GIMP_PDB_END);

  *num_path_point_details = 0;

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      path_type = return_vals[1].data.d_int32;
      *path_closed = return_vals[2].data.d_int32;
      *num_path_point_details = return_vals[3].data.d_int32;
      *points_pairs = g_new (gdouble, *num_path_point_details);
      memcpy (*points_pairs, return_vals[4].data.d_floatarray,
	      *num_path_point_details * sizeof (gdouble));
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return path_type;
}

/**
 * gimp_path_get_current:
 * @image_ID: The ID of the image to get the current paths from.
 *
 * The name of the current path. Error if no paths.
 *
 * The name of the current path. Error if no paths.
 *
 * Returns: The name of the current path.
 */
gchar *
gimp_path_get_current (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gchar *current_path_name = NULL;

  return_vals = gimp_run_procedure ("gimp_path_get_current",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    current_path_name = g_strdup (return_vals[1].data.d_string);

  gimp_destroy_params (return_vals, nreturn_vals);

  return current_path_name;
}

/**
 * gimp_path_set_current:
 * @image_ID: The ID of the image to list set the paths in.
 * @set_current_path_name: The name of the path to set the current path to.
 *
 * List the paths associated with the passed image.
 *
 * List the paths associated with the passed image.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_path_set_current (gint32  image_ID,
		       gchar  *set_current_path_name)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_path_set_current",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, set_current_path_name,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_path_set_points:
 * @image_ID: The ID of the image to set the paths in.
 * @pathname: The name of the path to create (if it exists then a unique name will be created - query the list of paths if you want to make sure that the name of the path you create is unique. This will be set as the current path.
 * @ptype: The type of the path. Currently only one type (1 = Bezier) is supported.
 * @num_path_points: The number of points in the path. Each point is made up of (x,y,type) of floats. Currently only the creation of bezier curves is allowed. The type parameter must be set to (1) to indicate a BEZIER type curve. For BEZIERS. Note the that points must be given in the following order... ACCACCAC ... If the path is not closed the last control point is missed off. Points consist of three control points (control/anchor/control) so for a curve that is not closed there must be at least two points passed (2 x,y pairs). If num_path_pnts % 3 = 0 then the path is assumed to be closed and the points are ACCACCACCACC.
 * @points_pairs: The points in the path represented as 3 floats. The first is the x pos, next is the y pos, last is the type of the pnt. The type field is dependant on the path type. For beziers (type 1 paths) the type can either be {1.0= BEZIER_ANCHOR, 2.0= BEZIER_CONTROL}. Note all points are returned in pixel resolution.
 *
 * Set the points associated with the named path.
 *
 * Set the points associated with the named path.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_path_set_points (gint32   image_ID,
		      gchar   *pathname,
		      gint     ptype,
		      gint     num_path_points,
		      gdouble *points_pairs)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_path_set_points",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, pathname,
				    GIMP_PDB_INT32, ptype,
				    GIMP_PDB_INT32, num_path_points,
				    GIMP_PDB_FLOATARRAY, points_pairs,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_path_stroke_current:
 * @image_ID: The ID of the image which contains the path to stroke.
 *
 * Stroke the current path in the passed image.
 *
 * Stroke the current path in the passed image.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_path_stroke_current (gint32 image_ID)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_path_stroke_current",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_path_get_point_at_dist:
 * @image_ID: The ID of the image the paths belongs to.
 * @distance: The distance along the path.
 * @y_point: The y position of the point.
 * @gradient: The gradient at the specified point.
 *
 * Get point on a path at a specified distance along the path.
 *
 * This will return the x,y position of a point at a given distance
 * along the bezier curve. The distance will the obtained by first
 * digitizing the curve internally an then walking along the curve. For
 * a closed curve the start of the path is the first point on the path
 * that was created. This might not be obvious. Note the current path
 * is used.
 *
 * Returns: The x position of the point.
 */
gint
gimp_path_get_point_at_dist (gint32   image_ID,
			     gdouble  distance,
			     gint    *y_point,
			     gdouble *gradient)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint x_point = 0;

  return_vals = gimp_run_procedure ("gimp_path_get_point_at_dist",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_FLOAT, distance,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    {
      x_point = return_vals[1].data.d_int32;
      *y_point = return_vals[2].data.d_int32;
      *gradient = return_vals[3].data.d_float;
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return x_point;
}

/**
 * gimp_path_get_tattoo:
 * @image_ID: The image.
 * @pathname: the name of the path whose tattoo should be obtained.
 *
 * Returns the tattoo associated with the name path.
 *
 * This procedure returns the tattoo associated with the specified
 * path. A tattoo is a unique and permanent identifier attached to a
 * path that can be used to uniquely identify a path within an image
 * even between sessions.
 *
 * Returns: The tattoo associated with the name path.
 */
gint
gimp_path_get_tattoo (gint32  image_ID,
		      gchar  *pathname)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint tattoo = 0;

  return_vals = gimp_run_procedure ("gimp_path_get_tattoo",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, pathname,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    tattoo = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return tattoo;
}

/**
 * gimp_get_path_by_tattoo:
 * @image_ID: The image.
 * @tattoo: The tattoo of the required path.
 *
 * Return the name of the path with the given tattoo.
 *
 * The procedure returns the name of the path in the specified image
 * which has the passed tattoo. The tattoos are unique within the image
 * and will be preserved across sessions and through renaming of the
 * path. An error is returned if no path woth the specified tattoo can
 * be found.
 *
 * Returns: The name of the path with the specified tattoo.
 */
gchar *
gimp_get_path_by_tattoo (gint32 image_ID,
			 gint   tattoo)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gchar *path_name = NULL;

  return_vals = gimp_run_procedure ("gimp_get_path_by_tattoo",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_INT32, tattoo,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    path_name = g_strdup (return_vals[1].data.d_string);

  gimp_destroy_params (return_vals, nreturn_vals);

  return path_name;
}

/**
 * gimp_path_delete:
 * @image_ID: The ID of the image to list delete the paths from.
 * @path_name_to_del: The name of the path to delete.
 *
 * Delete the named paths associated with the passed image.
 *
 * Delete the named path.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_path_delete (gint32  image_ID,
		  gchar  *path_name_to_del)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_path_delete",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, path_name_to_del,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_path_get_locked:
 * @image_ID: The image.
 * @pathname: the name of the path whose locked status should be obtained.
 *
 * Returns the locked status associated with the name path.
 *
 * This procedure returns the lock status associated with the specified
 * path. A path can be \"locked\" which means that the transformation
 * tool operations will also apply to the path.
 *
 * Returns: The lock status associated with the name path. 0 returned if the path is not locked. 1 is returned if the path is locked.
 */
gint
gimp_path_get_locked (gint32  image_ID,
		      gchar  *pathname)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gint lockstatus = 0;

  return_vals = gimp_run_procedure ("gimp_path_get_locked",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, pathname,
				    GIMP_PDB_END);

  if (return_vals[0].data.d_status == GIMP_PDB_SUCCESS)
    lockstatus = return_vals[1].data.d_int32;

  gimp_destroy_params (return_vals, nreturn_vals);

  return lockstatus;
}

/**
 * gimp_path_set_locked:
 * @image_ID: The image.
 * @pathname: the name of the path whose locked status should be set.
 * @lockstatus: The lock status associated with the name path. 0 if the path is not locked. 1 if the path is to be locked.
 *
 * Set the locked status associated with the name path.
 *
 * This procedure sets the lock status associated with the specified
 * path. A path can be \"locked\" which means that the transformation
 * tool operations will also apply to the path.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_path_set_locked (gint32  image_ID,
		      gchar  *pathname,
		      gint    lockstatus)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_path_set_locked",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, pathname,
				    GIMP_PDB_INT32, lockstatus,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}

/**
 * gimp_path_set_tattoo:
 * @image_ID: The image.
 * @pathname: the name of the path whose tattoo should be set.
 * @tattovalue: The tattoo associated with the name path. Only values returned from 'path_get_tattoo' should be used here.
 *
 * Sets the tattoo associated with the name path.
 *
 * This procedure sets the tattoo associated with the specified path. A
 * tattoo is a unique and permenant identifier attached to a path that
 * can be used to uniquely identify a path within an image even between
 * sessions. Note that the value passed to this function must have been
 * obtained from a previous call to path_get_tattoo.
 *
 * Returns: TRUE on success.
 */
gboolean
gimp_path_set_tattoo (gint32  image_ID,
		      gchar  *pathname,
		      gint    tattovalue)
{
  GimpParam *return_vals;
  gint nreturn_vals;
  gboolean success = TRUE;

  return_vals = gimp_run_procedure ("gimp_path_set_tattoo",
				    &nreturn_vals,
				    GIMP_PDB_IMAGE, image_ID,
				    GIMP_PDB_STRING, pathname,
				    GIMP_PDB_INT32, tattovalue,
				    GIMP_PDB_END);

  success = return_vals[0].data.d_status == GIMP_PDB_SUCCESS;

  gimp_destroy_params (return_vals, nreturn_vals);

  return success;
}
