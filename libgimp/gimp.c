/* LIBGIMP - The GIMP Library
 * Copyright (C) 1995-1997 Peter Mattis and Spencer Kimball
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "config.h"

#include <locale.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef WAIT_ANY
#define WAIT_ANY -1
#endif

#ifndef G_OS_WIN32
#include "gimpsignal.h"
#else
#include <signal.h>
#endif

#include "parasiteP.h"
#include "gimpenv.h"

#ifdef HAVE_IPC_H
#include <sys/ipc.h>
#endif

#ifdef HAVE_SHM_H
#include <sys/shm.h>
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if defined(G_OS_WIN32) || defined(G_WITH_CYGWIN)
#  define STRICT
#  include <windows.h>
#  undef RGB
#endif
#ifdef __EMX__
#  include <fcntl.h>
#endif

#include "gimp.h"
#include "gimpprotocol.h"
#include "gimpwire.h"


#define WRITE_BUFFER_SIZE  1024

void gimp_extension_process (guint timeout);
void gimp_extension_ack     (void);
void gimp_read_expect_msg   (WireMessage *msg, gint type);


#ifndef G_OS_WIN32
static void       gimp_plugin_sigfatal_handler (gint          sig_num);
static void       gimp_plugin_sigchld_handler  (gint          sig_num);
#endif
static gboolean   gimp_plugin_io_error_handler (GIOChannel   *channel,
						GIOCondition  cond,
						gpointer      data);

static gint       gimp_write                   (GIOChannel   *channel,
						guint8       *buf,
						gulong        count);
static gint       gimp_flush                   (GIOChannel   *channel);
static void       gimp_loop                    (void);
static void       gimp_config                  (GPConfig     *config);
static void       gimp_proc_run                (GPProcRun    *proc_run);
static void       gimp_temp_proc_run           (GPProcRun    *proc_run);
static void       gimp_message_func            (gchar        *str);
static void       gimp_process_message         (WireMessage  *msg);
static void       gimp_close                   (void);


GIOChannel *_readchannel  = NULL;
GIOChannel *_writechannel = NULL;

gint    _shm_ID   = -1;
guchar *_shm_addr = NULL;

guint gimp_major_version = GIMP_MAJOR_VERSION;
guint gimp_minor_version = GIMP_MINOR_VERSION;
guint gimp_micro_version = GIMP_MICRO_VERSION;

#if defined(G_OS_WIN32) || defined(G_WITH_CYGWIN)
static HANDLE shm_handle;
#endif

static gdouble  _gamma_val;
static gboolean _install_cmap;
static gboolean _use_xshm;
static guchar   _color_cube[4];
static gint     _min_colors;
static gint     _gdisp_ID = -1;

static gchar   *progname = NULL;
static guint8   write_buffer[WRITE_BUFFER_SIZE];
static guint    write_buffer_index = 0;

static GHashTable *temp_proc_ht = NULL;

#ifdef G_OS_WIN32
static GPlugInInfo *PLUG_IN_INFO_PTR;
#define PLUG_IN_INFO (*PLUG_IN_INFO_PTR)
void
set_gimp_PLUG_IN_INFO_PTR(GPlugInInfo *p)
{
  PLUG_IN_INFO_PTR = p;
}
#else
#ifndef __EMX__
extern GPlugInInfo PLUG_IN_INFO;
#else
static GPlugInInfo PLUG_IN_INFO;
void set_gimp_PLUG_IN_INFO(const GPlugInInfo *p)
{
  PLUG_IN_INFO = *p;
}
#endif
#endif


int
gimp_main (int   argc,
	   char *argv[])
{
#ifdef G_OS_WIN32
  char *peer, *peer_fd;
  guint32 thread;
  int i, j, k;
#endif

  setlocale (LC_NUMERIC, "C");

#ifdef G_OS_WIN32
  /* Check for exe file name with spaces in the path having been split up
   * by buggy NT C runtime, or something. I don't know why this happens
   * on NT (including w2k), but not on w95/98.
   */

  for (i = 1; i < argc; i++)
    {
      k = strlen (argv[i]);
      if (k > 10)
	if (g_strcasecmp (argv[i] + k - 4, ".exe") == 0)
	  {
	    /* Found the end of the executable name, most probably.
	     * Splice the parts of the name back together.
	     */
	    GString *s;

	    s = g_string_new (argv[0]);
	    for (j = 1; j <= i; j++)
	      {
		s = g_string_append_c (s, ' ');
		s = g_string_append (s, argv[j]);
	      }
	    argv[0] = s->str;
	    /* Move rest of argv down */
	    for (j = 1; j < argc - i; j++)
	      argv[j] = argv[j + i];
	    argv[argc - i] = NULL;
	    argc -= i;
	    break;
	  }
    }    
#endif

  if ((argc < 4) || (strcmp (argv[1], "-gimp") != 0))
    {
      g_print ("%s is a gimp plug-in and must be run by the gimp to be used\n", argv[0]);
      return 1;
    }

  progname = argv[0];

  g_set_prgname (g_basename (progname));

#ifndef G_OS_WIN32
  /* No use catching these on Win32, the user won't get any meaningful
   * stack trace from glib anyhow. It's better to let Windows inform
   * about the program error, and offer debugging if the plug-in
   * has been built with MSVC, and the user has MSVC installed.
   */
  gimp_signal_private (SIGHUP,  gimp_plugin_sigfatal_handler, 0);
  gimp_signal_private (SIGINT,  gimp_plugin_sigfatal_handler, 0);
  gimp_signal_private (SIGQUIT, gimp_plugin_sigfatal_handler, 0);
  gimp_signal_private (SIGBUS,  gimp_plugin_sigfatal_handler, 0);
  gimp_signal_private (SIGSEGV, gimp_plugin_sigfatal_handler, 0);
  gimp_signal_private (SIGTERM, gimp_plugin_sigfatal_handler, 0);
  gimp_signal_private (SIGFPE,  gimp_plugin_sigfatal_handler, 0);

  /* Ignore SIGPIPE from crashing Gimp */
  gimp_signal_private (SIGPIPE, SIG_IGN, 0);

  /* Restart syscalls interrupted by SIGCHLD */
  gimp_signal_private (SIGCHLD, gimp_plugin_sigchld_handler, SA_RESTART);
#endif

#ifndef G_OS_WIN32
  _readchannel = g_io_channel_unix_new (atoi (argv[2]));
  _writechannel = g_io_channel_unix_new (atoi (argv[3]));
#ifdef __EMX__
  setmode(g_io_channel_unix_get_fd(_readchannel), O_BINARY);
  setmode(g_io_channel_unix_get_fd(_writechannel), O_BINARY);
#endif
#else
  g_assert (PLUG_IN_INFO_PTR != NULL);
  _readchannel = g_io_channel_win32_new_pipe (atoi (argv[2]));
  peer = strchr (argv[3], ':') + 1;
  peer_fd = strchr (peer, ':') + 1;
  _writechannel = g_io_channel_win32_new_pipe_with_wakeups
    (atoi (argv[3]), atoi (peer), atoi (peer_fd));
#endif

  gp_init ();
  wire_set_writer (gimp_write);
  wire_set_flusher (gimp_flush);

  if ((argc == 5) && (strcmp (argv[4], "-query") == 0))
    {
      if (PLUG_IN_INFO.query_proc)
	(* PLUG_IN_INFO.query_proc) ();
      gimp_close ();
      return 0;
    }

#ifdef G_OS_WIN32
  /* Tell the GIMP our thread id */
  thread = GetCurrentThreadId ();
  wire_write_int32 (_writechannel, &thread, 1);
  wire_flush (_writechannel);
#endif

  g_set_message_handler ((GPrintFunc) gimp_message_func);

  temp_proc_ht = g_hash_table_new (&g_str_hash, &g_str_equal);

  g_io_add_watch (_readchannel,
		  G_IO_ERR | G_IO_HUP,
		  gimp_plugin_io_error_handler,
		  NULL);

  gimp_loop ();
  return 0;
}

static void
gimp_close (void)
{
  if (PLUG_IN_INFO.quit_proc)
    (* PLUG_IN_INFO.quit_proc) ();

#if defined(G_OS_WIN32) || defined(G_WITH_CYGWIN)
  CloseHandle (shm_handle);
#else
#ifdef HAVE_SHM_H
  if ((_shm_ID != -1) && _shm_addr)
    shmdt ((char*) _shm_addr);
#endif
#endif

  gp_quit_write (_writechannel);
}

void
gimp_quit (void)
{
  gimp_close ();
  exit (0);
}

void
gimp_set_data (gchar    *id,
	       gpointer  data,
	       guint32   length)
{
  GParam *return_vals;
  int nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_procedural_db_set_data",
				    &nreturn_vals,
				    PARAM_STRING, id,
				    PARAM_INT32, length,
				    PARAM_INT8ARRAY, data,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

guint32
gimp_get_data_size (gchar *id)
{
  GParam *return_vals;
  gint nreturn_vals;
  guint32 length;

  return_vals = gimp_run_procedure ("gimp_procedural_db_get_data_size",
                                    &nreturn_vals,
                                    PARAM_STRING, id,
                                    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
      length= return_vals[1].data.d_int32;
  else
      length= 0;

  gimp_destroy_params (return_vals, nreturn_vals);
  return length;
}


void
gimp_get_data (gchar    *id,
	       gpointer  data)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint length;
  gchar *returned_data;

  return_vals = gimp_run_procedure ("gimp_procedural_db_get_data",
				    &nreturn_vals,
				    PARAM_STRING, id,
				    PARAM_END);

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      length = return_vals[1].data.d_int32;
      returned_data = (gchar *) return_vals[2].data.d_int8array;

      memcpy (data, returned_data, length);
    }

  gimp_destroy_params (return_vals, nreturn_vals);
}


void
gimp_progress_init (char *message)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_progress_init",
				    &nreturn_vals,
				    PARAM_STRING, message,
				    PARAM_INT32, _gdisp_ID,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

void
gimp_progress_update (gdouble percentage)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_progress_update",
				    &nreturn_vals,
				    PARAM_FLOAT, percentage,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

gint32
gimp_default_display (void)
{
  return _gdisp_ID;
}


void
gimp_message (const gchar *message)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_message",
				    &nreturn_vals,
				    PARAM_STRING, message,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

static void
gimp_message_func (gchar *str)
{
  gimp_message (str);
}


void
gimp_query_database (gchar   *name_regexp,
		     gchar   *blurb_regexp,
		     gchar   *help_regexp,
		     gchar   *author_regexp,
		     gchar   *copyright_regexp,
		     gchar   *date_regexp,
		     gchar   *proc_type_regexp,
		     gint    *nprocs,
		     gchar ***proc_names)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint i;

  return_vals = gimp_run_procedure ("gimp_procedural_db_query",
				    &nreturn_vals,
				    PARAM_STRING, name_regexp,
				    PARAM_STRING, blurb_regexp,
				    PARAM_STRING, help_regexp,
				    PARAM_STRING, author_regexp,
				    PARAM_STRING, copyright_regexp,
				    PARAM_STRING, date_regexp,
				    PARAM_STRING, proc_type_regexp,
				    PARAM_END);

  *nprocs = 0;
  *proc_names = NULL;

  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      *nprocs = return_vals[1].data.d_int32;
      *proc_names = g_new (gchar*, *nprocs);

      for (i = 0; i < *nprocs; i++)
	(*proc_names)[i] = g_strdup (return_vals[2].data.d_stringarray[i]);
    }

  gimp_destroy_params (return_vals, nreturn_vals);
}

gboolean
gimp_query_procedure (gchar      *proc_name,
		      gchar     **proc_blurb,
		      gchar     **proc_help,
		      gchar     **proc_author,
		      gchar     **proc_copyright,
		      gchar     **proc_date,
		      gint       *proc_type,
		      gint       *nparams,
		      gint       *nreturn_vals,
		      GParamDef **params,
		      GParamDef **return_vals)
{
  GParam *ret_vals;
  gint nret_vals;
  gint i;
  gboolean success = TRUE;

  ret_vals = gimp_run_procedure ("gimp_procedural_db_proc_info",
				 &nret_vals,
				 PARAM_STRING, proc_name,
				 PARAM_END);

  if (ret_vals[0].data.d_status == STATUS_SUCCESS)
    {
      *proc_blurb = g_strdup (ret_vals[1].data.d_string);
      *proc_help = g_strdup (ret_vals[2].data.d_string);
      *proc_author = g_strdup (ret_vals[3].data.d_string);
      *proc_copyright = g_strdup (ret_vals[4].data.d_string);
      *proc_date = g_strdup (ret_vals[5].data.d_string);
      *proc_type = ret_vals[6].data.d_int32;
      *nparams = ret_vals[7].data.d_int32;
      *nreturn_vals = ret_vals[8].data.d_int32;
      *params = g_new (GParamDef, *nparams);
      *return_vals = g_new (GParamDef, *nreturn_vals);

      for (i = 0; i < *nparams; i++)
	{
	  GParam *rvals;
	  gint nrvals;

	  rvals = gimp_run_procedure ("gimp_procedural_db_proc_arg",
				      &nrvals,
				      PARAM_STRING, proc_name,
				      PARAM_INT32, i,
				      PARAM_END);

	  if (rvals[0].data.d_status == STATUS_SUCCESS)
	    {
	      (*params)[i].type = rvals[1].data.d_int32;
	      (*params)[i].name = g_strdup (rvals[2].data.d_string);
	      (*params)[i].description = g_strdup (rvals[3].data.d_string);
	    }
	  else
	    {
	      g_free (*params);
	      g_free (*return_vals);
	      gimp_destroy_params (rvals, nrvals);
	      return FALSE;
	    }

	  gimp_destroy_params (rvals, nrvals);
	}

      for (i = 0; i < *nreturn_vals; i++)
	{
	  GParam *rvals;
	  int nrvals;

	  rvals = gimp_run_procedure ("gimp_procedural_db_proc_val",
				      &nrvals,
				      PARAM_STRING, proc_name,
				      PARAM_INT32, i,
				      PARAM_END);

	  if (rvals[0].data.d_status == STATUS_SUCCESS)
	    {
	      (*return_vals)[i].type = rvals[1].data.d_int32;
	      (*return_vals)[i].name = g_strdup (rvals[2].data.d_string);
	      (*return_vals)[i].description = g_strdup (rvals[3].data.d_string);
	    }
	  else
	    {
	      g_free (*params);
	      g_free (*return_vals);
	      gimp_destroy_params (rvals, nrvals);
	      return FALSE;
	    }

	  gimp_destroy_params (rvals, nrvals);
	}
    }
  else
    success = FALSE;

  gimp_destroy_params (ret_vals, nret_vals);

  return success;
}

gint32*
gimp_query_images (gint *nimages)
{
  GParam *return_vals;
  gint nreturn_vals;
  gint32 *images;

  return_vals = gimp_run_procedure ("gimp_image_list",
				    &nreturn_vals,
				    PARAM_END);

  images = NULL;
  if (return_vals[0].data.d_status == STATUS_SUCCESS)
    {
      *nimages = return_vals[1].data.d_int32;
      images = g_new (gint32, *nimages);
      memcpy (images, return_vals[2].data.d_int32array, *nimages * 4);
    }

  gimp_destroy_params (return_vals, nreturn_vals);

  return images;
}

void
gimp_install_procedure (gchar     *name,
			gchar     *blurb,
			gchar     *help,
			gchar     *author,
			gchar     *copyright,
			gchar     *date,
			gchar     *menu_path,
			gchar     *image_types,
			gint       type,
			gint       nparams,
			gint       nreturn_vals,
			GParamDef *params,
			GParamDef *return_vals)
{
  GPProcInstall proc_install;

  proc_install.name = name;
  proc_install.blurb = blurb;
  proc_install.help = help;
  proc_install.author = author;
  proc_install.copyright = copyright;
  proc_install.date = date;
  proc_install.menu_path = menu_path;
  proc_install.image_types = image_types;
  proc_install.type = type;
  proc_install.nparams = nparams;
  proc_install.nreturn_vals = nreturn_vals;
  proc_install.params = (GPParamDef*) params;
  proc_install.return_vals = (GPParamDef*) return_vals;

  if (!gp_proc_install_write (_writechannel, &proc_install))
    gimp_quit ();
}

void
gimp_install_temp_proc (gchar     *name,
			gchar     *blurb,
			gchar     *help,
			gchar     *author,
			gchar     *copyright,
			gchar     *date,
			gchar     *menu_path,
			gchar     *image_types,
			gint       type,
			gint       nparams,
			gint       nreturn_vals,
			GParamDef *params,
			GParamDef *return_vals,
			GRunProc   run_proc)
{
  gimp_install_procedure (name, blurb, help, author, copyright, date,
			  menu_path, image_types, type,
			  nparams, nreturn_vals, params, return_vals);

  /*  Insert the temp proc run function into the hash table  */
  g_hash_table_insert (temp_proc_ht, g_strdup (name), (gpointer) run_proc);
}

void
gimp_uninstall_temp_proc (gchar *name)
{
  GPProcUninstall proc_uninstall;
  gpointer hash_name;
  gboolean found;
  proc_uninstall.name = name;

  if (!gp_proc_uninstall_write (_writechannel, &proc_uninstall))
    gimp_quit ();
  
  found = g_hash_table_lookup_extended (temp_proc_ht, name, &hash_name, NULL);
  if (found)
  {
    g_free (hash_name);
    g_hash_table_remove (temp_proc_ht, (gpointer) name);
  }
}

void
gimp_register_magic_load_handler (gchar *name,
				  gchar *extensions,
				  gchar *prefixes,
				  gchar *magics)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_register_magic_load_handler",
				    &nreturn_vals,
				    PARAM_STRING, name,
				    PARAM_STRING, extensions,
				    PARAM_STRING, prefixes,
				    PARAM_STRING, magics,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

void
gimp_register_load_handler (gchar *name,
			    gchar *extensions,
			    gchar *prefixes)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_register_load_handler",
				    &nreturn_vals,
				    PARAM_STRING, name,
				    PARAM_STRING, extensions,
				    PARAM_STRING, prefixes,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

void
gimp_register_save_handler (gchar *name,
			    gchar *extensions,
			    gchar *prefixes)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_register_save_handler",
				    &nreturn_vals,
				    PARAM_STRING, name,
				    PARAM_STRING, extensions,
				    PARAM_STRING, prefixes,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

GParam*
gimp_run_procedure (gchar *name,
		    gint  *nreturn_vals,
		    ...)
{
  GPProcRun proc_run;
  GPProcReturn *proc_return;
  WireMessage msg;
  GParamType param_type;
  GParam *return_vals;
  va_list args;
  guchar *color;
  gint i;

  proc_run.name = name;
  proc_run.nparams = 0;
  proc_run.params = NULL;

  va_start (args, nreturn_vals);
  param_type = va_arg (args, GParamType);

  while (param_type != PARAM_END)
    {
      switch (param_type)
	{
	case PARAM_INT32:
        case PARAM_DISPLAY:
        case PARAM_IMAGE:
        case PARAM_LAYER:
        case PARAM_CHANNEL:
        case PARAM_DRAWABLE:
        case PARAM_SELECTION:
        case PARAM_BOUNDARY:
        case PARAM_PATH:
        case PARAM_STATUS:
	  (void) va_arg (args, int);
	  break;
	case PARAM_INT16:
	  (void) va_arg (args, int);
	  break;
	case PARAM_INT8:
	  (void) va_arg (args, int);
	  break;
        case PARAM_FLOAT:
          (void) va_arg (args, double);
          break;
        case PARAM_STRING:
          (void) va_arg (args, gchar*);
          break;
        case PARAM_INT32ARRAY:
          (void) va_arg (args, gint32*);
          break;
        case PARAM_INT16ARRAY:
          (void) va_arg (args, gint16*);
          break;
        case PARAM_INT8ARRAY:
          (void) va_arg (args, gint8*);
          break;
        case PARAM_FLOATARRAY:
          (void) va_arg (args, gdouble*);
          break;
        case PARAM_STRINGARRAY:
          (void) va_arg (args, gchar**);
          break;
        case PARAM_COLOR:
          (void) va_arg (args, guchar*);
          break;
        case PARAM_PARASITE:
          (void) va_arg (args, GimpParasite*);
          break;
        case PARAM_REGION:
          break;
	case PARAM_END:
	  break;
	}

      proc_run.nparams += 1;
      param_type = va_arg (args, GParamType);
    }

  va_end (args);

  proc_run.params = g_new (GPParam, proc_run.nparams);

  va_start (args, nreturn_vals);

  for (i = 0; i < proc_run.nparams; i++)
    {
      proc_run.params[i].type = va_arg (args, GParamType);

      switch (proc_run.params[i].type)
	{
	case PARAM_INT32:
	  proc_run.params[i].data.d_int32 = (gint32) va_arg (args, int);
	  break;
	case PARAM_INT16:
	  proc_run.params[i].data.d_int16 = (gint16) va_arg (args, int);
	  break;
	case PARAM_INT8:
	  proc_run.params[i].data.d_int8 = (gint8) va_arg (args, int);
	  break;
        case PARAM_FLOAT:
          proc_run.params[i].data.d_float = (gdouble) va_arg (args, double);
          break;
        case PARAM_STRING:
          proc_run.params[i].data.d_string = va_arg (args, gchar*);
          break;
        case PARAM_INT32ARRAY:
          proc_run.params[i].data.d_int32array = va_arg (args, gint32*);
          break;
        case PARAM_INT16ARRAY:
          proc_run.params[i].data.d_int16array = va_arg (args, gint16*);
          break;
        case PARAM_INT8ARRAY:
          proc_run.params[i].data.d_int8array = va_arg (args, gint8*);
          break;
        case PARAM_FLOATARRAY:
          proc_run.params[i].data.d_floatarray = va_arg (args, gdouble*);
          break;
        case PARAM_STRINGARRAY:
          proc_run.params[i].data.d_stringarray = va_arg (args, gchar**);
          break;
        case PARAM_COLOR:
	  color = va_arg (args, guchar*);
          proc_run.params[i].data.d_color.red = color[0];
          proc_run.params[i].data.d_color.green = color[1];
          proc_run.params[i].data.d_color.blue = color[2];
          break;
        case PARAM_REGION:
          break;
        case PARAM_DISPLAY:
	  proc_run.params[i].data.d_display = va_arg (args, gint32);
          break;
        case PARAM_IMAGE:
	  proc_run.params[i].data.d_image = va_arg (args, gint32);
          break;
        case PARAM_LAYER:
	  proc_run.params[i].data.d_layer = va_arg (args, gint32);
          break;
        case PARAM_CHANNEL:
	  proc_run.params[i].data.d_channel = va_arg (args, gint32);
          break;
        case PARAM_DRAWABLE:
	  proc_run.params[i].data.d_drawable = va_arg (args, gint32);
          break;
        case PARAM_SELECTION:
	  proc_run.params[i].data.d_selection = va_arg (args, gint32);
          break;
        case PARAM_BOUNDARY:
	  proc_run.params[i].data.d_boundary = va_arg (args, gint32);
          break;
        case PARAM_PATH:
	  proc_run.params[i].data.d_path = va_arg (args, gint32);
          break;
        case PARAM_PARASITE:
	  {
	    GimpParasite *p = va_arg (args, GimpParasite*);
	    if (p == NULL)
	      {
		proc_run.params[i].data.d_parasite.name = NULL;
		proc_run.params[i].data.d_parasite.data = NULL;
	      }
	    else
	      {
		proc_run.params[i].data.d_parasite.name  = p->name;
		proc_run.params[i].data.d_parasite.flags = p->flags;
		proc_run.params[i].data.d_parasite.size  = p->size;
		proc_run.params[i].data.d_parasite.data  = p->data;
	      }
	  }
	  break;
        case PARAM_STATUS:
	  proc_run.params[i].data.d_status = va_arg (args, gint32);
          break;
	case PARAM_END:
	  break;
	}
    }

  va_end (args);

  if (!gp_proc_run_write (_writechannel, &proc_run))
    gimp_quit ();

  gimp_read_expect_msg (&msg, GP_PROC_RETURN);

  proc_return = msg.data;
  *nreturn_vals = proc_return->nparams;
  return_vals = (GParam*) proc_return->params;

  switch (return_vals[0].data.d_status)
    {
    case STATUS_EXECUTION_ERROR:
      /*g_warning ("an execution error occured while trying to run: \"%s\"", name);*/
      break;
    case STATUS_CALLING_ERROR:
      g_warning ("a calling error occured while trying to run: \"%s\"", name);
      break;
    default:
      break;
    }

  g_free (proc_run.params);
  g_free (proc_return->name);
  g_free (proc_return);

  return return_vals;
}

void
gimp_read_expect_msg (WireMessage *msg, 
		      gint         type)
{
  while (TRUE)
    {
      if (!wire_read_msg (_readchannel, msg))
	gimp_quit ();
      
      if (msg->type != type)
	{
	  if (msg->type == GP_TEMP_PROC_RUN || msg->type == GP_QUIT)
	    {
	      gimp_process_message (msg);
	      continue;
	    }
	  else
	    g_error ("unexpected message: %d\n", msg->type);
	}
      else
	break;
    }
}


GParam*
gimp_run_procedure2 (gchar  *name,
		     gint   *nreturn_vals,
		     gint    nparams,
		     GParam *params)
{
  GPProcRun proc_run;
  GPProcReturn *proc_return;
  WireMessage msg;
  GParam *return_vals;

  proc_run.name = name;
  proc_run.nparams = nparams;
  proc_run.params = (GPParam *) params;

  if (!gp_proc_run_write (_writechannel, &proc_run))
    gimp_quit ();

  gimp_read_expect_msg(&msg,GP_PROC_RETURN);
  
  proc_return = msg.data;
  *nreturn_vals = proc_return->nparams;
  return_vals = (GParam*) proc_return->params;

  switch (return_vals[0].data.d_status)
    {
    case STATUS_EXECUTION_ERROR:
      /*g_warning ("an execution error occured while trying to run: \"%s\"", name);*/
      break;
    case STATUS_CALLING_ERROR:
      g_warning ("a calling error occured while trying to run: \"%s\"", name);
      break;
    default:
      break;
    }

  g_free (proc_return->name);
  g_free (proc_return);

  return return_vals;
}

void
gimp_destroy_params (GParam *params,
		     gint    nparams)
{
  extern void _gp_params_destroy (GPParam *params, gint nparams);

  _gp_params_destroy ((GPParam*) params, nparams);
}

void
gimp_destroy_paramdefs (GParamDef *paramdefs,
			gint       nparams)
{
  while (nparams--)
    {
      g_free (paramdefs[nparams].name);
      g_free (paramdefs[nparams].description);
    }
  
  g_free (paramdefs);
}

gdouble
gimp_gamma (void)
{
  return _gamma_val;
}

gboolean
gimp_install_cmap (void)
{
  return _install_cmap;
}

gboolean
gimp_use_xshm (void)
{
  return _use_xshm;
}

guchar *
gimp_color_cube (void)
{
  return _color_cube;
}

gint
gimp_min_colors (void)
{
  return _min_colors;
}

static void
gimp_process_message (WireMessage *msg)
{
  switch (msg->type)
    {
    case GP_QUIT:
      gimp_quit ();
      break;
    case GP_CONFIG:
      gimp_config (msg->data);
      break;
    case GP_TILE_REQ:
    case GP_TILE_ACK:
    case GP_TILE_DATA:
      g_warning ("unexpected tile message received (should not happen)\n");
      break;
    case GP_PROC_RUN:
      g_warning ("unexpected proc run message received (should not happen)\n");
      break;
    case GP_PROC_RETURN:
      g_warning ("unexpected proc return message received (should not happen)\n");
      break;
    case GP_TEMP_PROC_RUN:
      gimp_temp_proc_run (msg->data);
      break;
    case GP_TEMP_PROC_RETURN:
      g_warning ("unexpected temp proc return message received (should not happen)\n");
      break;
    case GP_PROC_INSTALL:
      g_warning ("unexpected proc install message received (should not happen)\n");
      break;
    }
}

static void 
gimp_single_message (void)
{
  WireMessage msg;

  /* Run a temp function */
  if (!wire_read_msg (_readchannel, &msg))
    gimp_quit ();

  gimp_process_message (&msg);
  
  wire_destroy (&msg);
}

void
gimp_extension_process (guint timeout)
{
#ifndef G_OS_WIN32
  fd_set readfds;
  gint select_val;
  struct timeval tv;
  struct timeval *tvp;

  if (timeout)
    {
      tv.tv_sec = timeout / 1000;
      tv.tv_usec = (timeout % 1000) * 1000;
      tvp = &tv;
    }
  else
    tvp = NULL;

  FD_ZERO (&readfds);
  FD_SET (g_io_channel_unix_get_fd (_readchannel), &readfds);

  if ((select_val = select (FD_SETSIZE, &readfds, NULL, NULL, tvp)) > 0)
    {
      gimp_single_message();
    }
  else if (select_val == -1)
    {
      perror ("gimp_process");
      gimp_quit ();
    }
#else
  /* ??? */
  MSG msg;
  UINT timer;
  static UINT message = 0;

  if (message == 0)
    message = RegisterWindowMessage ("g-pipe-readable");
  if (timeout > 0)
    timer = SetTimer (NULL, 0, timeout, NULL);
  WaitMessage ();
  if (timeout > 0)
    KillTimer (NULL, timer);

  if (PeekMessage (&msg, (HWND) -1, message, message, PM_NOREMOVE))
    gimp_single_message ();
#endif
}

void
gimp_extension_ack (void)
{
  /*  Send an extension initialization acknowledgement  */
  if (! gp_extension_ack_write (_writechannel))
    gimp_quit ();
}

void
gimp_run_temp (void)
{
  gimp_single_message();
}

void
gimp_request_wakeups (void)
{
  if (!gp_request_wakeups_write (_writechannel))
    gimp_quit ();
}

gchar *
gimp_get_progname (void)
{
  return progname;
}

#ifndef G_OS_WIN32
static void
gimp_plugin_sigfatal_handler (gint sig_num)
{
  switch (sig_num)
    {
    case SIGHUP:
    case SIGINT:
    case SIGQUIT:
    case SIGABRT:
    case SIGTERM:
      g_print ("%s terminated: %s\n", progname, g_strsignal (sig_num));
      break;

    case SIGBUS:
    case SIGSEGV:
    case SIGFPE:
    case SIGPIPE:
    default:
      g_print ("%s: fatal error: %s\n", progname, g_strsignal (sig_num));
      if (TRUE)
	{
	  sigset_t sigset;

	  sigemptyset (&sigset);
	  sigprocmask (SIG_SETMASK, &sigset, NULL);
	  g_on_error_query (progname);
	}
      break;
    }

  gimp_quit ();
}

static void
gimp_plugin_sigchld_handler (gint sig_num)
{
  gint pid;
  gint status;

  while (TRUE)
    {
      pid = waitpid (WAIT_ANY, &status, WNOHANG);

      if (pid <= 0)
        break;
    }
}
#endif

static gboolean
gimp_plugin_io_error_handler (GIOChannel   *channel,
			      GIOCondition  cond,
			      gpointer      data)
{
  g_print ("%s: fatal error: GIMP crashed\n", progname);
  gimp_quit ();

  /* never reached */
  return TRUE;
}

static int
gimp_write (GIOChannel *channel, 
	    guint8     *buf, 
	    gulong      count)
{
  gulong bytes;

  while (count > 0)
    {
      if ((write_buffer_index + count) >= WRITE_BUFFER_SIZE)
	{
	  bytes = WRITE_BUFFER_SIZE - write_buffer_index;
	  memcpy (&write_buffer[write_buffer_index], buf, bytes);
	  write_buffer_index += bytes;
	  if (!wire_flush (channel))
	    return FALSE;
	}
      else
	{
	  bytes = count;
	  memcpy (&write_buffer[write_buffer_index], buf, bytes);
	  write_buffer_index += bytes;
	}

      buf += bytes;
      count -= bytes;
    }

  return TRUE;
}

static int
gimp_flush (GIOChannel *channel)
{
  GIOError error;
  guint count;
  guint bytes;

  if (write_buffer_index > 0)
    {
      count = 0;
      while (count != write_buffer_index)
        {
	  do {
	    bytes = 0;
	    error = g_io_channel_write (channel, &write_buffer[count],
					(write_buffer_index - count),
					&bytes);
	  } while (error == G_IO_ERROR_AGAIN);

	  if (error != G_IO_ERROR_NONE)
	    return FALSE;

          count += bytes;
        }

      write_buffer_index = 0;
    }

  return TRUE;
}

static void
gimp_loop (void)
{
  WireMessage msg;

  while (TRUE)
    {
      if (!wire_read_msg (_readchannel, &msg))
        {
	  gimp_close ();
          return;
        }

      switch (msg.type)
	{
	case GP_QUIT:
	  gimp_close ();
	  return;
	case GP_CONFIG:
	  gimp_config (msg.data);
	  break;
	case GP_TILE_REQ:
	case GP_TILE_ACK:
	case GP_TILE_DATA:
	  g_warning ("unexpected tile message received (should not happen)\n");
	  break;
	case GP_PROC_RUN:
	  gimp_proc_run (msg.data);
	  gimp_close ();
          return;
	case GP_PROC_RETURN:
	  g_warning ("unexpected proc return message received (should not happen)\n");
	  break;
	case GP_TEMP_PROC_RUN:
	  g_warning ("unexpected temp proc run message received (should not happen\n");
	  break;
	case GP_TEMP_PROC_RETURN:
	  g_warning ("unexpected temp proc return message received (should not happen\n");
	  break;
	case GP_PROC_INSTALL:
	  g_warning ("unexpected proc install message received (should not happen)\n");
	  break;
	}

      wire_destroy (&msg);
    }
}

static void
gimp_config (GPConfig *config)
{
  extern gint _gimp_tile_width;
  extern gint _gimp_tile_height;

  if (config->version < GP_VERSION)
    {
      g_message ("Could not execute Plug-In \"%s\"\n(%s)\n\n"
		 "The GIMP is using an older version of the "
		 "Plug-In protocol than this Plug-In.",
		 g_basename (progname), progname);
      gimp_quit ();
    }
  else if (config->version > GP_VERSION)
    {
      g_message ("Could not execute Plug-In \"%s\"\n(%s)\n\n"
		 "The GIMP is using an older version of the "
		 "Plug-In protocol than this Plug-In.",
		 g_basename (progname), progname);
      gimp_quit ();
    }

  _gimp_tile_width  = config->tile_width;
  _gimp_tile_height = config->tile_height;
  _shm_ID           = config->shm_ID;
  _gamma_val        = config->gamma;
  _install_cmap     = config->install_cmap;
  _color_cube[0]    = 6;  /*  These are the former default values  */
  _color_cube[1]    = 6;  /*  (for backward compatibility only)    */
  _color_cube[2]    = 4;
  _color_cube[3]    = 24;
  _use_xshm         = config->use_xshm;
  _min_colors       = config->min_colors;
  _gdisp_ID         = config->gdisp_ID;

  if (_shm_ID != -1)
    {
#if defined(G_OS_WIN32) || defined(G_WITH_CYGWIN)
      /*
       * Use Win32 shared memory mechanisms for
       * transfering tile data
       */
      gchar fileMapName[128];
      gint tileByteSize = _gimp_tile_width * _gimp_tile_height * 4;

      /* From the id, derive the file map name */
      g_snprintf (fileMapName, sizeof (fileMapName), "GIMP%d.SHM", _shm_ID);

      /* Open the file mapping */
      shm_handle = OpenFileMapping (FILE_MAP_ALL_ACCESS,
				    0, fileMapName);
      if (shm_handle)
	{
	  /* Map the shared memory into our address space for use */
	  _shm_addr = (guchar *) MapViewOfFile (shm_handle,
						FILE_MAP_ALL_ACCESS,
						0, 0, tileByteSize);

	  /* Verify that we mapped our view */
	  if (!_shm_addr)
	    {
	      g_warning ("MapViewOfFile error: %d... disabling shared memory transport",
			 GetLastError());
	    }
	}
      else
	{
	  g_warning ("OpenFileMapping error: %d... disabling shared memory transport",
		     GetLastError());
	}
#else
#ifdef HAVE_SHM_H
      _shm_addr = (guchar*) shmat (_shm_ID, 0, 0);

      if (_shm_addr == (guchar*) -1)
	g_error ("could not attach to gimp shared memory segment\n");
#endif
#endif
    }
}

static void
gimp_proc_run (GPProcRun *proc_run)
{
  GPProcReturn proc_return;
  GParam *return_vals;
  gint nreturn_vals;

  if (PLUG_IN_INFO.run_proc)
    {
      (* PLUG_IN_INFO.run_proc) (proc_run->name,
				 proc_run->nparams,
				 (GParam*) proc_run->params,
				 &nreturn_vals,
				 &return_vals);

      proc_return.name = proc_run->name;
      proc_return.nparams = nreturn_vals;
      proc_return.params = (GPParam*) return_vals;

      if (!gp_proc_return_write (_writechannel, &proc_return))
	gimp_quit ();
    }
}

static void
gimp_temp_proc_run (GPProcRun *proc_run)
{
  GParam *return_vals;
  gint nreturn_vals;
  GRunProc run_proc;

  run_proc = (GRunProc)g_hash_table_lookup (temp_proc_ht, (gpointer) proc_run->name);

  if (run_proc)
    {
      (* run_proc) (proc_run->name,
		    proc_run->nparams,
		    (GParam*) proc_run->params,
		    &nreturn_vals,
		    &return_vals);

      /* No longer a return message */
/*       proc_return.name = proc_run->name; */
/*       proc_return.nparams = nreturn_vals; */
/*       proc_return.params = (GPParam*) return_vals; */

/*       if (!gp_temp_proc_return_write (_writechannel, &proc_return)) */
/* 	gimp_quit (); */
    }
}

void
gimp_plugin_domain_add (gchar *domain_name)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_plugin_domain_add",
				    &nreturn_vals,
				    PARAM_STRING, domain_name,
				    PARAM_STRING, NULL,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}

void
gimp_plugin_domain_add_with_path (gchar *domain_name,
				  gchar *domain_path)
{
  GParam *return_vals;
  gint nreturn_vals;

  return_vals = gimp_run_procedure ("gimp_plugin_domain_add",
				    &nreturn_vals,
				    PARAM_STRING, domain_name,
				    PARAM_STRING, domain_path,
				    PARAM_END);

  gimp_destroy_params (return_vals, nreturn_vals);
}
