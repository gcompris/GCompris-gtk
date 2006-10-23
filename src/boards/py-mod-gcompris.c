/* gcompris - py-mod-gcompris.c
 *
 * Time-stamp: <2006/08/21 23:36:37 bruno>
 *
 * Copyright (C) 2003 Olivier Samyn <osamyn@ulb.ac.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-gcompris.h"
#include "py-gcompris-board.h"
#include "py-gcompris-properties.h"
#include "py-gcompris-profile.h"
#include "py-gcompris-wordlist.h"

/* submodules includes */
#include "py-mod-bonus.h"
#include "py-mod-score.h"
#include "py-mod-skin.h"
#include "py-mod-sound.h"
#include "py-mod-timer.h"
#include "py-mod-utils.h"
#include "py-mod-anim.h"
#include "py-mod-admin.h"

void initgnomecanvas (void);

void pair_in_dict(gpointer key,
		  gpointer value,
		  gpointer dict)
{
  PyObject *pyValue;
  PyObject *pyKey;

  pyKey = PyString_FromString((gchar *)key);
  Py_INCREF(pyKey);


  /* key cannot be NULL. But value can */
  if (value==NULL){
    Py_INCREF(Py_None);
    pyValue = Py_None;
  } else {
    pyValue = PyString_FromString((gchar *)value);
    Py_INCREF(pyValue);
  }

  PyDict_SetItem((PyObject *)dict, pyKey, pyValue);

  g_warning("Hash to dict: pass key %s and value %s",(gchar *)key, (gchar *)value );
}


/* Utility */
PyObject* hash_to_dict(GHashTable *table)
{
  PyObject *pydict;

  pydict = PyDict_New();

  g_hash_table_foreach            (table,
				   pair_in_dict,
                                   (gpointer) pydict);

  Py_INCREF(pydict);
  return pydict;
}


/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */


/* void gc_board_end(void); */
static PyObject*
py_gc_board_end(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_board_end"))
    return NULL;

  /* Call the corresponding C function */
  gc_board_end();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void	gc_bar_start (GnomeCanvas *theCanvas); */
static PyObject*
py_gc_bar_start(PyObject* self, PyObject* args)
{
  PyObject* pyCanvas;
  GnomeCanvas* canvas;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gc_bar_start", &pyCanvas))
    return NULL;
  canvas = (GnomeCanvas*) pygobject_get(pyCanvas);

  /* Call the corresponding C function */
  gc_bar_start(canvas);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* GnomeCanvasItem *gc_set_background(GnomeCanvasGroup *parent, gchar *file); */
static PyObject*
py_gc_set_background(PyObject* self, PyObject* args)
{
  PyObject* pyCanvasGroup;
  GnomeCanvasGroup* canvasGroup;
  gchar* file;
  PyObject* pyResult;
  GnomeCanvasItem* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Os:gc_set_background", &pyCanvasGroup, &file))
    return NULL;
  canvasGroup = (GnomeCanvasGroup*) pygobject_get(pyCanvasGroup);

  /* Call the corresponding C function */
  result = gc_set_background(canvasGroup, file);

  /* Create and return the result */
  pyResult = pygobject_new((GObject*)result);
  return pyResult;
}


/* void gc_bar_set_level (GcomprisBoard *gcomprisBoard); */
static PyObject*
py_gc_bar_set_level(PyObject* self, PyObject* args)
{
  PyObject* pyObject;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gc_bar_set_level", &pyObject))
    return NULL;
  pyGcomprisBoard = (pyGcomprisBoardObject*) pyObject;
  cGcomprisBoard = pyGcomprisBoard->cdata;

  /* Call the corresponding C function */
  gc_bar_set_level(cGcomprisBoard);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_bar_set_repeat_icon (GdkPixbuf *pixmap); */
static PyObject*
py_gc_bar_set_repeat_icon(PyObject* self, PyObject* args)
{
  PyObject* pyObject;
  GdkPixbuf* pixmap;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gc_bar_set_repeat_icon", &pyObject))
    return NULL;
  pixmap = (GdkPixbuf*) pygobject_get(pyObject);

  /* Call the corresponding C function */
  gc_bar_set_repeat_icon(pixmap);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_bar_set (const GComprisBarFlags flags); */
static PyObject*
py_gc_bar_set(PyObject* self, PyObject* args)
{
  gint values;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gc_bar_set", &values))
    return NULL;

  /* Call the corresponding C function */
  gc_bar_set(values);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_bar_hide (gboolean hide); */
static PyObject*
py_gc_bar_hide(PyObject* self, PyObject* args)
{
  gint values;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gc_bar_hide", &values))
    return NULL;

  /* Call the corresponding C function */
  gc_bar_hide(values);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

/* GnomeCanvas *gc_get_canvas(void); */
static PyObject*
py_gc_get_canvas(PyObject* self, PyObject* args)
{
  GnomeCanvas* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_get_canvas"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_get_canvas();

  /* Create and return the result */
  return (PyObject*) pygobject_new((GObject*) result);
}


/* GtkWidget *gc_get_window(void); */
static PyObject*
py_gc_get_window(PyObject* self, PyObject* args)
{
  GtkWidget* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_get_window"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_get_window();

  /* Create and return the result */
  return (PyObject*)pygobject_new((GObject*)result);
}


/* gchar *gc_locale_get(void); */
static PyObject*
py_gc_locale_get(PyObject* self, PyObject* args)
{
  gchar* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_locale_get"))
    return NULL;

  /* Call the corresponding C function */
  result = (gchar*)gc_locale_get();

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* void gc_locale_set(gchar *locale); */
static PyObject*
py_gc_locale_set(PyObject* self, PyObject* args)
{
  gchar* locale;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_locale_set", &locale))
    return NULL;

  /* Call the corresponding C function */
  gc_locale_set(locale);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* char *gc_locale_get_user_default(void) */
static PyObject*
py_gc_locale_get_user_default(PyObject* self, PyObject* args)
{
  char* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_locale_get_user_default"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_locale_get_user_default();

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* void gc_cursor_set(guint gdk_cursor_type); */
static PyObject*
py_gc_cursor_set(PyObject* self, PyObject* args)
{
  guint cursor;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gc_cursor_set",&cursor))
    return NULL;

  /* Call the corresponding C function */
  gc_cursor_set(cursor);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* Some functions and variables needed to get the image selector working */
static PyObject* pyImageSelectorCallBackFunc = NULL;

void pyImageSelectorCallBack(gchar* image){
  PyObject* args;
  PyObject* result;
  if(pyImageSelectorCallBackFunc==NULL) return;

  /* Build arguments */
  args = PyTuple_New(1);
  PyTuple_SetItem(args, 0, Py_BuildValue("s", image));
  result = PyObject_CallObject(pyImageSelectorCallBackFunc, args);
  if(result==NULL){
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }
}


/* void gc_selector_images_start (GcomprisBoard *gcomprisBoard,  */
/* 					gchar *dataset,  */
/* 					ImageSelectorCallBack imscb); */
static PyObject*
py_gc_selector_images_start(PyObject* self, PyObject* args)
{
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  PyObject* pyCallback;
  gchar* dataset;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "OsO:gc_selector_images_start",
		       &pyGcomprisBoard,
		       &dataset,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  pyImageSelectorCallBackFunc = pyCallback;
  gc_selector_images_start(cGcomprisBoard,
				 dataset,
				 pyImageSelectorCallBack);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_log_set_comment (GcomprisBoard *gcomprisBoard, gchar *expected, gchar *got); */
static PyObject*
py_gc_log_set_comment(PyObject* self, PyObject* args)
{
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  gchar* expected;
  gchar* got;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "Os:gc_log_set_comment",
		       &pyGcomprisBoard,
		       &expected,
		       &got))
    return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  gc_log_set_comment(cGcomprisBoard,
			   expected, got);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_log_end (GcomprisBoard *gcomprisBoard, gchar *status); */
static PyObject*
py_gc_log_end(PyObject* self, PyObject* args)
{
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  gchar* status;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "Os:gc_log_end",
		       &pyGcomprisBoard,
		       &status))
    return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  gc_log_end(cGcomprisBoard,
		   status);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_selector_images_stop (void); */
static PyObject*
py_gc_selector_images_stop(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_selector_images_stop"))
    return NULL;

  /* Call the corresponding C function */
  gc_selector_images_stop();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* Some functions and variables needed to get the file selector working */
static PyObject* pyFileSelectorCallBackFunc = NULL;

void pyFileSelectorCallBack(gchar* file, char* file_type){
  PyObject* args;
  PyObject* result;
  if(pyFileSelectorCallBackFunc==NULL) return;

  /* Build arguments */
  args  = PyTuple_New(2);
  PyTuple_SetItem(args, 0, Py_BuildValue("s", file));
  PyTuple_SetItem(args, 1, Py_BuildValue("s", file_type));
  result = PyObject_CallObject(pyFileSelectorCallBackFunc, args);
  if(result==NULL){
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }

}


/* void gc_selector_file_load(GcomprisBoard *gcomprisBoard,
                                    gchar *rootdir,
				    gchar *file_types, (A Comma separated text explaining the different file types)
                                    FileSelectorCallBack fscb);
*/
static PyObject*
py_gc_selector_file_load(PyObject* self, PyObject* args){
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  PyObject* pyCallback;
  gchar* rootdir;
  gchar* file_types;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "OssO:gc_selector_file_load",
		       &pyGcomprisBoard,
		       &rootdir,
		       &file_types,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  pyFileSelectorCallBackFunc = pyCallback;
  gc_selector_file_load(cGcomprisBoard,
                              rootdir,
			      file_types,
                              pyFileSelectorCallBack);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_selector_file_save(GcomprisBoard *gcomprisBoard,
                                    gchar *rootdir,
				    gchar *file_types, (A Comma separated text explaining the different file types)
                                    FileSelectorCallBack fscb);
*/
static PyObject*
py_gc_selector_file_save(PyObject* self, PyObject* args){
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  PyObject* pyCallback;
  gchar* rootdir;
  char* file_types;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "OssO:gc_selector_file_save",
		       &pyGcomprisBoard,
		       &rootdir,
		       &file_types,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  pyFileSelectorCallBackFunc = pyCallback;
  gc_selector_file_save(cGcomprisBoard,
                              rootdir,
			      file_types,
                              pyFileSelectorCallBack);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_selector_file_stop (void); */
static PyObject*
py_gc_selector_file_stop(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_selector_file_stop"))
    return NULL;

  /* Call the corresponding C function */
  gc_selector_file_stop();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

/* gchar *gc_db_get_filename(void); */
static PyObject*
py_gc_db_get_filename(PyObject* self, PyObject* args)
{
  gchar* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_db_get_filename"))
    return NULL;

  /* Call the corresponding C function */
  result = (gchar*)gc_db_get_filename();

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* GcomprisProperties *gc_prop_get(void); */
static PyObject*
py_gc_prop_get(PyObject* self, PyObject* args)
{
  GcomprisProperties* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_prop_get"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_prop_get();

  /* Create and return the result */
  return gcompris_new_pyGcomprisPropertiesObject(result);
}

/* GcomprisBoard *gc_menu_section_get(gchar *section); */
static PyObject*
py_gc_menu_section_get(PyObject* self, PyObject* args)
{
  GcomprisBoard* result;
  gchar *section;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_menu_section_get", &section))
    return NULL;

  /* Call the corresponding C function */
  result = gc_menu_section_get(section);

  /* Create and return the result */
  return gcompris_new_pyGcomprisBoardObject(result);
}


/* Code stolen from PyGTK */
/* This bindings are available only in pygtk 2.6, a little bit too young for us. */


struct _PyGChildData {
    PyObject *func;
    PyObject *data;
};


static void
child_watch_func(GPid pid, gint status, gpointer data)
{
    struct _PyGChildData *child_data = (struct _PyGChildData *) data;
    PyObject *retval;
    PyGILState_STATE gil;

    gil = pyg_gil_state_ensure();
    if (child_data->data)
        retval = PyObject_CallFunction(child_data->func, "iiO", pid, status,
                                       child_data->data);
    else
        retval = PyObject_CallFunction(child_data->func, "ii", pid, status);

    if (retval)
	Py_DECREF(retval);
    else
	PyErr_Print();

    pyg_gil_state_release(gil);
}

static void
child_watch_dnotify(gpointer data)
{
    struct _PyGChildData *child_data = (struct _PyGChildData *) data;
    Py_DECREF(child_data->func);
    Py_XDECREF(child_data->data);
    g_free(child_data);
}


static PyObject *
py_gcompris_child_watch_add(PyObject *unused, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pid", "function", "data", "priority", NULL };
    guint id;
    gint priority = G_PRIORITY_DEFAULT;
    GPid pid;
    PyObject *func, *user_data = NULL;
    struct _PyGChildData *child_data;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "iO|Oi:gcompris.child_watch_add", kwlist,
                                     &pid, &func, &user_data, &priority))
        return NULL;
    if (!PyCallable_Check(func)) {
        PyErr_SetString(PyExc_TypeError,
                        "gobject.child_watch_add: second argument must be callable");
        return NULL;
    }

    child_data = g_new(struct _PyGChildData, 1);
    child_data->func = func;
    child_data->data = user_data;
    Py_INCREF(child_data->func);
    if (child_data->data)
        Py_INCREF(child_data->data);
    id = g_child_watch_add_full(priority, pid, child_watch_func,
                                child_data, child_watch_dnotify);
    return PyInt_FromLong(id);
}

struct _PyGChildSetupData {
    PyObject *func;
    PyObject *data;
};

static void
_pyg_spawn_async_callback(gpointer user_data)
{
    struct _PyGChildSetupData *data;
    PyObject *retval;
    PyGILState_STATE gil;

    data = (struct _PyGChildSetupData *) user_data;
    gil = pyg_gil_state_ensure();
    if (data->data)
        retval = PyObject_CallFunction(data->func, "O", data->data);
    else
        retval = PyObject_CallFunction(data->func, NULL);
    if (retval)
	Py_DECREF(retval);
    else
	PyErr_Print();
    Py_DECREF(data->func);
    Py_XDECREF(data->data);
    g_free(data);
    pyg_gil_state_release(gil);
}

static PyObject *
py_gcompris_spawn_async(PyObject *unused, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "argv", "envp", "working_directory", "flags",
                              "child_setup", "user_data", "standard_input",
                              "standard_output", "standard_error", NULL };
    PyObject *pyargv, *pyenvp = NULL;
    char **argv, **envp = NULL;
    PyObject *func = NULL, *user_data = NULL;
    char *working_directory = NULL;
    int flags = 0, _stdin = -1, _stdout = -1, _stderr = -1;
    PyObject *pystdin = NULL, *pystdout = NULL, *pystderr = NULL;
    gint *standard_input, *standard_output, *standard_error;
    struct _PyGChildSetupData *callback_data = NULL;
    GError *error = NULL;
    GPid child_pid = (GPid) -1;
    int len, i;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OsiOOOOO:gcompris.spawn_async",
                                     kwlist,
                                     &pyargv, &pyenvp, &working_directory, &flags,
                                     &func, &user_data,
                                     &pystdin, &pystdout, &pystderr))
        return NULL;

    if (pystdin && PyObject_IsTrue(pystdin))
        standard_input = &_stdin;
    else
        standard_input = NULL;

    if (pystdout && PyObject_IsTrue(pystdout))
        standard_output = &_stdout;
    else
        standard_output = NULL;

    if (pystderr && PyObject_IsTrue(pystderr))
        standard_error = &_stderr;
    else
        standard_error = NULL;

      /* parse argv */
    if (!PySequence_Check(pyargv)) {
        PyErr_SetString(PyExc_TypeError,
                        "gobject.spawn_async: first argument must be a sequence of strings");
        return NULL;
    }
    len = PySequence_Length(pyargv);
    argv = g_new0(char *, len + 1);
    for (i = 0; i < len; ++i) {
        PyObject *tmp = PySequence_ITEM(pyargv, i);
        if (!PyString_Check(tmp)) {
            PyErr_SetString(PyExc_TypeError,
                            "gobject.spawn_async: first argument must be a sequence of strings");
            g_free(argv);
            Py_XDECREF(tmp);
            return NULL;
        }
        argv[i] = PyString_AsString(tmp);
        Py_DECREF(tmp);
    }

      /* parse envp */
    if (pyenvp) {
        if (!PySequence_Check(pyenvp)) {
            PyErr_SetString(PyExc_TypeError,
                            "gobject.spawn_async: second argument must be a sequence of strings");
            g_free(argv);
            return NULL;
        }
        len = PySequence_Length(pyenvp);
        envp = g_new0(char *, len + 1);
        for (i = 0; i < len; ++i) {
            PyObject *tmp = PySequence_ITEM(pyenvp, i);
            if (!PyString_Check(tmp)) {
                PyErr_SetString(PyExc_TypeError,
                                "gobject.spawn_async: second argument must be a sequence of strings");
                g_free(envp);
                Py_XDECREF(tmp);
                return NULL;
            }
            envp[i] = PyString_AsString(tmp);
            Py_DECREF(tmp);
        }
    }

    if (func) {
        callback_data = g_new(struct _PyGChildSetupData, 1);
        callback_data->func = func;
        callback_data->data = user_data;
        Py_INCREF(callback_data->func);
        if (callback_data->data)
            Py_INCREF(callback_data->data);
    }

    if (!g_spawn_async_with_pipes(working_directory, argv, envp, flags,
                                  func? _pyg_spawn_async_callback : NULL,
                                  callback_data, &child_pid,
                                  standard_input,
                                  standard_output,
                                  standard_error,
                                  &error))
    {
        g_free(argv);
        if (envp) g_free(envp);
        if (callback_data) {
            Py_DECREF(callback_data->func);
            Py_XDECREF(callback_data->data);
            g_free(callback_data);
        }
        pyg_error_check(&error);
        return NULL;
    }
    g_free(argv);
    if (envp) g_free(envp);

    if (standard_input)
        pystdin = PyInt_FromLong(*standard_input);
    else {
        Py_INCREF(Py_None);
        pystdin = Py_None;
    }

    if (standard_output)
        pystdout = PyInt_FromLong(*standard_output);
    else {
        Py_INCREF(Py_None);
        pystdout = Py_None;
    }

    if (standard_error)
        pystderr = PyInt_FromLong(*standard_error);
    else {
        Py_INCREF(Py_None);
        pystderr = Py_None;
    }

    return Py_BuildValue("iNNN", child_pid, pystdin, pystdout, pystderr);
}


/*
  { "",  py_gcompris_, METH_VARARGS, "gcompris_" },
*/


static PyObject*
py_gc_db_get_board_conf(PyObject* self, PyObject* args)
{
  PyObject *pydict;
  GHashTable *table;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_board_conf"))
    return NULL;

  /* Call the corresponding C function */

  table = gc_db_get_board_conf();

  pydict = hash_to_dict(table);

  g_hash_table_destroy(table);

  return pydict;;
}

static PyObject*
py_gc_db_get_conf(PyObject* self, PyObject* args)
{
  PyObject *pydict;
  GHashTable *table;

  PyObject* pyBoard;
  PyObject* pyProfile;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  pyGcomprisProfileObject* pyGcomprisProfile;
  GcomprisProfile* cGcomprisProfile;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "OO:gcompris.get_conf", &pyProfile ,&pyBoard))
    return NULL;

  pyGcomprisBoard = (pyGcomprisBoardObject *) pyBoard;
  pyGcomprisProfile = (pyGcomprisProfileObject *) pyProfile;

  cGcomprisProfile = pyGcomprisProfile->cdata;
  cGcomprisBoard = pyGcomprisBoard->cdata;

  /* Call the corresponding C function */
  table = gc_db_get_conf(cGcomprisProfile, cGcomprisBoard);

  pydict = hash_to_dict(table);

  g_hash_table_destroy(table);

  return pydict;;
}


/* GcomprisProfile *gc_profile_get_current(void); */
static PyObject*
py_gc_profile_get_current(PyObject* self, PyObject* args)
{
  GcomprisProfile* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_profile_get_current"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_profile_get_current();

  /* Create and return the result */
  return gcompris_new_pyGcomprisProfileObject(result);
}


/* GcomprisUser *gc_profile_get_current_user(void); */
static PyObject*
py_gc_profile_get_current_user(PyObject* self, PyObject* args)
{
  GcomprisUser* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_profile_get_current_user"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_profile_get_current_user();

  /* Create and return the result */
  return gcompris_new_pyGcomprisUserObject(result);
}


static PyObject*
py_gc_db_set_board_conf (PyObject* self, PyObject* args)
{
  PyObject* pyBoard;
  PyObject* pyProfile;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  pyGcomprisProfileObject* pyGcomprisProfile;
  GcomprisProfile* cGcomprisProfile;
  char *key;
  char *value;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "OOss:gcompris.set_board_conf",
		       &pyProfile, &pyBoard,
		       &key, &value))
    return NULL;

  pyGcomprisBoard = (pyGcomprisBoardObject *) pyBoard;
  pyGcomprisProfile = (pyGcomprisProfileObject *) pyProfile;

  cGcomprisProfile = pyGcomprisProfile->cdata;
  cGcomprisBoard = pyGcomprisBoard->cdata;

  /* Call the corresponding C function */
  gc_db_set_board_conf(cGcomprisProfile, cGcomprisBoard, key, value);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* Some functions and variables needed to get the file selector working */
static PyObject* pyGcomprisConfCallbackFunc = NULL;

static void pyGcomprisConfCallback(GHashTable* table){
  PyObject* result;

  PyGILState_STATE gil;


  if(pyGcomprisConfCallbackFunc==NULL) return;

  gil = pyg_gil_state_ensure();

  if (table)
    result = PyObject_CallFunction(pyGcomprisConfCallbackFunc, "O", hash_to_dict(table));
  else
    result = PyObject_CallFunction(pyGcomprisConfCallbackFunc, "O", Py_None);

  // This callback can be called multiple time ? not now

  Py_DECREF(pyGcomprisConfCallbackFunc);

  if(result==NULL){
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }

  pyg_gil_state_release(gil);

}


static PyObject*
py_gc_board_config_window_display(PyObject* self, PyObject* args){
  PyObject* pyCallback;
  gchar *label;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "sO:gc_board_config_window_display",
		       &label,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback))
    {
      PyErr_SetString(PyExc_TypeError,
		      "gc_board_config_window_display second argument must be callable");
      return NULL;
    }

  //if (pyGcomprisConfCallbackFunc)
  //  Py_DECREF(pyGcomprisConfCallbackFunc);

  pyGcomprisConfCallbackFunc = pyCallback;

  Py_INCREF(pyGcomprisConfCallbackFunc);


  return (PyObject *) \
             pygobject_new((GObject*) \
			   gc_board_config_window_display( label,
							  (GcomprisConfCallback )pyGcomprisConfCallback));

}


/* GtkCheckButton *gc_board_config_boolean_box (label, key, init);*/
static PyObject*
py_gc_board_config_boolean_box(PyObject* self, PyObject* args)
{
  PyObject *py_bool;
  gchar *label;
  gchar *key;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssO:gc_board_config_boolean_box", &label, &key, &py_bool))
    return NULL;

  /* Call the corresponding C function */
  return (PyObject *)pygobject_new((GObject*) \
				    gc_board_config_boolean_box((const gchar *)label, key, PyObject_IsTrue(py_bool)));

}

/* GtkComboBox *gc_board_config_combo_box(const gchar *label, GList *strings, gchar *key, gint index); */
static PyObject*
py_gc_board_config_combo_box(PyObject* self, PyObject* args)
{
  PyObject *py_list;
  gchar *label;
  gchar *key;
  gchar *init;

  GList *list = NULL;

  int i, size;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "sOss:gc_board_config_combo_box", &label, &py_list, &key, &init))
    return NULL;

  if (!PyList_Check(py_list)){
    PyErr_SetString(PyExc_TypeError,
		      "gc_board_config_combo_box second argument must be a list");
    return NULL;
  }

  size = PyList_Size (py_list);

  for (i=0; i < size; i ++)
    list = g_list_append( list,
			  PyString_AsString( PyList_GetItem( py_list, i)));

  /* Call the corresponding C function */
  return (PyObject *)pygobject_new((GObject*) \
				    gc_board_config_combo_box((const gchar *)label,
						       list,
						       key,
						       init));
}


/* Params: */
/*   - Label */
/*   - key (for hashtable  return) */
/*   - g_hash_table (gchar *values, gchar *label) */
/* Returns */
/*   - g_hash_table (gchar *values, GtkWidget *pointer) */

/* GHashTable *gc_board_config_radio_buttons(const gchar *label, */
/* 				   gchar *key, */
/* 				   GHashTable *buttons_label, */
/* 				   gchar *init);  */

void pair_object_in_dict(gpointer key,
			 gpointer value,
			 gpointer dict)
{
  PyObject *pyValue;
  PyObject *pyKey;

  pyKey = PyString_FromString((gchar *)key);
  Py_INCREF(pyKey);

  pyValue = pygobject_new((GObject*) value);
  Py_INCREF(pyValue);

  PyDict_SetItem((PyObject *)dict, pyKey, pyValue);
}


/* Utility */
PyObject* hash_object_to_dict(GHashTable *table)
{
  PyObject *pydict;

  pydict = PyDict_New();

  g_hash_table_foreach            (table,
				   pair_object_in_dict,
                                   (gpointer) pydict);

  Py_INCREF(pydict);
  return pydict;
}


static PyObject*
py_gc_board_config_radio_buttons(PyObject* self, PyObject* args)
{
  PyObject *py_dict;
  GHashTable *buttons_label, *result;
  gchar *label;
  gchar *key;
  gchar *init;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssOs:gc_board_config_radio_buttons", &label, &key, &py_dict, &init))
    return NULL;

  if (!PyDict_Check(py_dict)){
    PyErr_SetString(PyExc_TypeError,
		      "gc_board_config_radio_buttons second argument must be a dict");
    return NULL;
  }

  PyObject *pykey, *pyvalue;
  int pos = 0;

  buttons_label = g_hash_table_new_full (g_str_hash,
				    g_str_equal,
				    g_free,
				    g_free);

  while (PyDict_Next(py_dict, &pos, &pykey, &pyvalue)) {
    g_hash_table_replace (buttons_label,
			  g_strdup(PyString_AsString(pykey)),
			  g_strdup(PyString_AsString(pyvalue)));
  }

  result = gc_board_config_radio_buttons(label,
				  key,
				  buttons_label,
				  init);

  g_hash_table_destroy(buttons_label);

  return hash_object_to_dict(result);
}

static PyObject*
py_gc_board_config_spin_int(PyObject* self, PyObject* args)
{
  gchar *label;
  gchar *key;
  gint min, max, step, init;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssiiii:gc_board_config_radio_buttons", &label, &key, &min, &max, &step, &init))
    return NULL;

  return (PyObject *)pygobject_new((GObject*) \
				   gc_board_config_spin_int((const gchar *)label,
						     key,
						     min,
						     max,
						     step,
						     init));

}


/* GtkHSeparator *gc_board_conf_separator(void); */
static PyObject*
py_gc_board_conf_separator(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_board_conf_separator"))
    return NULL;

  /* Create and return the result */
  return (PyObject *)pygobject_new((GObject*) gc_board_conf_separator());

}


static PyObject*
py_gc_board_config_combo_locales(PyObject* self, PyObject* args)
{
  gchar *init;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_board_config_combo_locales", &init))
    return NULL;

  return (PyObject *)pygobject_new((GObject*) \
				   gc_board_config_combo_locales( init));
}


static PyObject*
py_gc_locale_gets_list(PyObject* self, PyObject* args)
{
  PyObject *pylist;
  GList *result, *list ;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_locales_list"))
    return NULL;

  /* Call the corresponding C function */
  result = gc_locale_gets_list();

  pylist = PyList_New(0);

  for (list = result; list != NULL; list = list->next){
    PyList_Append( pylist, PyString_FromString(list->data));
  }

  Py_INCREF(pylist);
  return pylist;
}


static PyObject*
py_gc_board_config_combo_locales_asset(PyObject* self, PyObject* args)
{
  gchar *init;
  gchar *label;
  gchar *file;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssz:gc_board_config_combo_locales",
		       &label,
		       &init,
		       &file))
    return NULL;

  return (PyObject *)pygobject_new((GObject*) \
				   gc_board_config_combo_locales_asset( label, init, file ));
}


static PyObject*
py_gc_locale_gets_asset_list(PyObject* self, PyObject* args)
{
  PyObject *pylist;
  GList *result, *list ;
  gchar *file;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "z:gcompris.get_locales_asset_list",
		       &file))
    return NULL;

  /* Call the corresponding C function */
  result = gc_locale_gets_asset_list(file);

  pylist = PyList_New(0);

  for (list = result; list != NULL; list = list->next){
    PyList_Append( pylist, PyString_FromString(list->data));
  }

  Py_INCREF(pylist);
  return pylist;
}



static PyObject*
py_gcompris_gettext(PyObject* self, PyObject* args)
{
  gchar *text;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_gettext", &text))
    return NULL;

  return PyString_FromString(_(text));
}



/* void gc_locale_change(gchar *locale); */
static PyObject*
py_gc_locale_change(PyObject* self, PyObject* args)
{
  gchar *locale;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_locale_change", &locale))
    return NULL;

  /* Call the corresponding C function */
  gc_locale_set(locale);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_locale_reset(gchar *locale); */
static PyObject*
py_gc_locale_reset(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_locale_reset"))
    return NULL;

  /* Call the corresponding C function */
  gc_locale_reset();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* How can i free that ? */
static GHashTable *text_callbacks = NULL;

static gboolean pyGcomprisTextCallback(gchar *key, gchar *text, GtkLabel *label){
  PyObject* result;
  gboolean validate;

  PyGILState_STATE gil;

  PyObject* pyGcomprisTextCallbackFunc = g_hash_table_lookup( text_callbacks, key);

  if(pyGcomprisTextCallbackFunc==NULL) return FALSE;

  gil = pyg_gil_state_ensure();

  result = PyObject_CallFunction(pyGcomprisTextCallbackFunc, "ssO", key, text,(PyObject *)pygobject_new((GObject*) label ));

  //Py_DECREF(pyGcomprisTextCallbackFunc);

  if (PyObject_IsTrue(result))
    validate = TRUE;
  else
    validate = FALSE;

  if(result==NULL){
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }

  pyg_gil_state_release(gil);

  return validate;

}


static PyObject*
py_gc_board_config_textview(PyObject* self, PyObject* args){
  PyObject* pyCallback;
  gchar *label;
  gchar *key;
  gchar *desc = NULL;
  gchar *init_text = NULL;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "sszzO:gc_board_config_window_display",
		       &label,
		       &key,
		       &desc,
		       &init_text,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback))
    {
      PyErr_SetString(PyExc_TypeError,
		      "gc_board_config_textview 5th argument must be callable");
      return NULL;
    }

  if (!text_callbacks)
    text_callbacks = g_hash_table_new ( g_str_hash, g_str_equal);

  g_hash_table_replace (text_callbacks, key, pyCallback);

  Py_INCREF(pyCallback);

  return (PyObject *) \
             pygobject_new((GObject*) \
			   gc_board_config_textview( label,
					      key,
					      desc,
					      init_text,
					      (GcomprisTextCallback )pyGcomprisTextCallback));

}


static PyObject*
py_gc_prop_user_dirname_get (PyObject* self, PyObject* args)
{
  PyObject* pyUser;
  pyGcomprisUserObject* pyGcomprisUser;
  GcomprisUser* cGcomprisUser;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris.get_user_dirname", &pyUser))
    return NULL;

  pyGcomprisUser = (pyGcomprisUserObject *) pyUser;

  cGcomprisUser = pyGcomprisUser->cdata;

  /* Call the corresponding C function */
  return PyString_FromString(gc_prop_user_dirname_get(cGcomprisUser));

}

static PyObject*
py_gc_prop_board_dirname_get (PyObject* self, PyObject* args)
{
  PyObject* pyBoard;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris.get_board_dirname", &pyBoard))
    return NULL;

  pyGcomprisBoard = (pyGcomprisBoardObject *) pyBoard;

  cGcomprisBoard = pyGcomprisBoard->cdata;

  /* Call the corresponding C function */
  return PyString_FromString(gc_prop_board_dirname_get(cGcomprisBoard));

}

static PyObject*
py_gc_prop_current_user_dirname_get (PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_current_user_dirname"))
    return NULL;

  /* Call the corresponding C function */
  return PyString_FromString(gc_prop_current_user_dirname_get());

}

static PyObject*
py_gc_prop_current_board_dirname_get (PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_current_board_dirname"))
    return NULL;

  /* Call the corresponding C function */
  return PyString_FromString(gc_prop_current_board_dirname_get());

}

static PyObject*
py_gcompris_wordlist_get_from_file (PyObject* self, PyObject* args)
{
  GcomprisWordlist *result;
  gchar *filename;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris.get_wordlist", &filename))
    return NULL;

  /* Call the corresponding C function */
  result = gc_wordlist_get_from_file (filename);
  if (result)
    return gcompris_new_pyGcomprisWordlistObject(result);
  else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

static PyObject*
py_gc_im_reset (PyObject* self, PyObject* args)
{

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.im_reset"))
    return NULL;

  /* Call the corresponding C function */
  gc_im_reset ();

  Py_INCREF(Py_None);
  return Py_None;
}



/****************************************************/

static PyMethodDef PythonGcomprisModule[] = {
  { "end_board",  py_gc_board_end, METH_VARARGS, "gc_board_end" },
  { "bar_start",  py_gc_bar_start, METH_VARARGS, "gc_bar_start" },
  { "set_background",  py_gc_set_background, METH_VARARGS, "gc_set_background" },
  { "bar_set_level",  py_gc_bar_set_level, METH_VARARGS, "gc_bar_set_level" },
  { "bar_set_repeat_icon",  py_gc_bar_set_repeat_icon, METH_VARARGS, "gc_bar_set_repeat_icon" },
  { "bar_set",  py_gc_bar_set, METH_VARARGS, "gc_bar_set" },
  { "bar_hide",  py_gc_bar_hide, METH_VARARGS, "gc_bar_hide" },
  { "get_canvas",  py_gc_get_canvas, METH_VARARGS, "gc_get_canvas" },
  { "get_window",  py_gc_get_window, METH_VARARGS, "gc_get_window" },
  { "get_locale",  py_gc_locale_get, METH_VARARGS, "gc_locale_get" },
  { "get_user_default_locale",  py_gc_locale_get_user_default, METH_VARARGS, "gc_locale_get_user_default" },
  { "set_locale",  py_gc_locale_set, METH_VARARGS, "gc_locale_set" },
  { "set_cursor",  py_gc_cursor_set, METH_VARARGS, "gc_cursor_set" },
  { "images_selector_start",  py_gc_selector_images_start,
    METH_VARARGS, "gc_selector_images_start" },
  { "images_selector_stop",  py_gc_selector_images_stop,
    METH_VARARGS, "gc_selector_images_stop" },
  { "log_set_comment",  py_gc_log_set_comment, METH_VARARGS, "gc_log_set_comment" },
  { "log_end",  py_gc_log_end, METH_VARARGS, "gc_log_end" },
  { "file_selector_load",  py_gc_selector_file_load,
    METH_VARARGS, "gc_selector_file_load" },
  { "file_selector_save",  py_gc_selector_file_save,
    METH_VARARGS, "gc_selector_file_save" },
  { "file_selector_stop",  py_gc_selector_file_stop,
    METH_VARARGS, "gc_selector_file_stop" },
  { "get_database",  py_gc_db_get_filename, METH_VARARGS, "gc_db_get_filename" },
  { "get_properties",  py_gc_prop_get, METH_VARARGS, "gc_prop_get" },
  { "get_board_from_section",  py_gc_menu_section_get, METH_VARARGS, "gc_menu_section_get" },
  { "spawn_async",  (PyCFunction)py_gcompris_spawn_async, METH_VARARGS|METH_KEYWORDS, "gcompris_spawn_sync" },
  { "child_watch_add",  (PyCFunction)py_gcompris_child_watch_add, METH_VARARGS|METH_KEYWORDS, "gcompris_child_watch_add" },
  { "get_board_conf",  py_gc_db_get_board_conf, METH_VARARGS, "gc_db_get_board_conf" },
  { "get_conf",  py_gc_db_get_conf, METH_VARARGS, "gc_db_get_conf" },
  { "set_board_conf",  py_gc_db_set_board_conf, METH_VARARGS, "gc_db_set_board_conf" },
  { "get_current_profile",  py_gc_profile_get_current, METH_VARARGS, "gc_profile_get_current" },
  { "get_current_user",  py_gc_profile_get_current_user, METH_VARARGS, "gc_profile_get_current_user" },
  { "configuration_window",  py_gc_board_config_window_display, METH_VARARGS, "gc_board_config_window_display" },
  { "boolean_box",  py_gc_board_config_boolean_box, METH_VARARGS, "gc_board_config_boolean_box" },
  { "combo_box",  py_gc_board_config_combo_box, METH_VARARGS, "gc_board_config_combo_box" },
  { "radio_buttons",  py_gc_board_config_radio_buttons, METH_VARARGS, "gc_board_config_radio_buttons" },
  { "spin_int",  py_gc_board_config_spin_int, METH_VARARGS, "gc_board_config_spin_int" },
  { "separator",  py_gc_board_conf_separator, METH_VARARGS, "gc_board_conf_separator" },
  { "combo_locales",  py_gc_board_config_combo_locales, METH_VARARGS, "gc_board_config_combo_locales" },
  { "get_locales_list",  py_gc_locale_gets_list, METH_VARARGS, "gc_locale_gets_list" },
  { "gcompris_gettext",  py_gcompris_gettext, METH_VARARGS, "gcompris_gettext" },
  { "change_locale",  py_gc_locale_change, METH_VARARGS, "gc_locale_change" },
  { "reset_locale",  py_gc_locale_reset, METH_VARARGS, "gc_locale_reset" },
  { "combo_locales_asset",  py_gc_board_config_combo_locales_asset, METH_VARARGS, "gc_board_config_combo_locales_asset" },
  { "get_locales_asset_list",  py_gc_locale_gets_asset_list, METH_VARARGS, "gc_locale_gets_asset_list" },
  { "textview",  py_gc_board_config_textview, METH_VARARGS, "gc_board_config_textview" },
  { "get_user_dirname",  py_gc_prop_user_dirname_get, METH_VARARGS, "gc_prop_user_dirname_get" },
  { "get_current_user_dirname",  py_gc_prop_current_user_dirname_get, METH_VARARGS, "gc_prop_current_user_dirname_get" },
  { "get_board_dirname",  py_gc_prop_board_dirname_get, METH_VARARGS, "gc_prop_board_dirname_get" },
  { "get_current_board_dirname",  py_gc_prop_current_board_dirname_get, METH_VARARGS, "gc_prop_current_board_dirname_get" },
  { "get_wordlist",  py_gcompris_wordlist_get_from_file, METH_VARARGS, "gcompris_wordlist_get_from_file" },
  { "im_reset",  py_gc_im_reset, METH_VARARGS, "gc_im_reset" },
  { NULL, NULL, 0, NULL}
};

void python_gcompris_module_init(void)
{
  PyObject* gcomprisModule;

  gcomprisModule = Py_InitModule("_gcompris", PythonGcomprisModule);

  /* Misc constants */
  PyModule_AddIntConstant(gcomprisModule, "BOARD_HEIGHT", BOARDHEIGHT );
  PyModule_AddIntConstant(gcomprisModule, "BOARD_WIDTH",  BOARDWIDTH);
  PyModule_AddIntConstant(gcomprisModule, "BAR_HEIGHT",   BARHEIGHT);
  PyModule_AddStringConstant(gcomprisModule, "DEFAULT_SKIN", DEFAULT_SKIN);

  /* The GComprisBarFlags enum constants */
  PyModule_AddIntConstant(gcomprisModule, "BAR_LEVEL",       GC_BAR_LEVEL);
  PyModule_AddIntConstant(gcomprisModule, "BAR_OK",          GC_BAR_OK);
  PyModule_AddIntConstant(gcomprisModule, "BAR_REPEAT",      GC_BAR_REPEAT);
  PyModule_AddIntConstant(gcomprisModule, "BAR_REPEAT_ICON", GC_BAR_REPEAT_ICON);
  PyModule_AddIntConstant(gcomprisModule, "BAR_CONFIG",      GC_BAR_CONFIG);
  PyModule_AddIntConstant(gcomprisModule, "BAR_ABOUT",       GC_BAR_ABOUT);

  /* Colors constants */
  PyModule_AddIntConstant(gcomprisModule, "COLOR_TITLE",       COLOR_TITLE);
  PyModule_AddIntConstant(gcomprisModule, "COLOR_TEXT_BUTTON", COLOR_TEXT_BUTTON);
  PyModule_AddIntConstant(gcomprisModule, "COLOR_CONTENT",     COLOR_CONTENT);
  PyModule_AddIntConstant(gcomprisModule, "COLOR_SUBTITLE",    COLOR_SUBTITLE);

  /* Fonts constants */
  PyModule_AddStringConstant(gcomprisModule, "FONT_TITLE",             FONT_TITLE);
  PyModule_AddStringConstant(gcomprisModule, "FONT_TITLE_FALLBACK",    FONT_TITLE_FALLBACK);
  PyModule_AddStringConstant(gcomprisModule, "FONT_SUBTITLE",          FONT_SUBTITLE);
  PyModule_AddStringConstant(gcomprisModule, "FONT_SUBTITLE_FALLBACK", FONT_SUBTITLE_FALLBACK);
  PyModule_AddStringConstant(gcomprisModule, "FONT_CONTENT",           FONT_CONTENT);
  PyModule_AddStringConstant(gcomprisModule, "FONT_CONTENT_FALLBACK",  FONT_CONTENT_FALLBACK);

  /* Fonts board constants */
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_TINY",       FONT_BOARD_TINY);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_SMALL",      FONT_BOARD_SMALL);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_MEDIUM",     FONT_BOARD_MEDIUM);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_BIG",        FONT_BOARD_BIG);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_BIG_BOLD",   FONT_BOARD_BIG_BOLD);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_FIXED",      FONT_BOARD_FIXED);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_TITLE",      FONT_BOARD_TITLE);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_TITLE_BOLD", FONT_BOARD_TITLE_BOLD);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_HUGE",       FONT_BOARD_HUGE);
  PyModule_AddStringConstant(gcomprisModule, "FONT_BOARD_HUGE_BOLD",  FONT_BOARD_HUGE_BOLD);

  /* Cursors constants */
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_FIRST_CUSTOM",  GCOMPRIS_FIRST_CUSTOM_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_BIG_RED_ARROW", GCOMPRIS_BIG_RED_ARROW_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_BIRD",          GCOMPRIS_BIRD_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_LINE",          GCOMPRIS_LINE_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_FILLRECT",      GCOMPRIS_FILLRECT_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_RECT",          GCOMPRIS_RECT_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_FILLCIRCLE",    GCOMPRIS_FILLCIRCLE_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_CIRCLE",        GCOMPRIS_CIRCLE_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_DEL",           GCOMPRIS_DEL_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_FILL",          GCOMPRIS_FILL_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_SELECT",        GCOMPRIS_SELECT_CURSOR);
  PyModule_AddIntConstant(gcomprisModule, "CURSOR_DEFAULT",       GCOMPRIS_DEFAULT_CURSOR);

  /* Some non gcompris.h constants. */
  GcomprisProperties	*properties = gc_prop_get();
  PyModule_AddStringConstant(gcomprisModule, "DATA_DIR", properties->package_data_dir);
  PyModule_AddStringConstant(gcomprisModule, "PYTHON_PLUGIN_DIR", properties->package_python_plugin_dir);

  /* GetText constants. */
  PyModule_AddStringConstant(gcomprisModule, "GETTEXT_PACKAGE", GETTEXT_PACKAGE);
  PyModule_AddStringConstant(gcomprisModule, "PACKAGE_LOCALE_DIR",  PACKAGE_LOCALE_DIR);


  /* Initialize the sub modules */
  initgnomecanvas();
  python_gcompris_bonus_module_init();
  python_gc_score_module_init();
  python_gc_skin_module_init();
  python_gcompris_sound_module_init();
  python_gc_timer_module_init();
  python_gcompris_utils_module_init();
  python_gcompris_anim_module_init();
  python_gcompris_admin_module_init();
}

/* Some usefull code parts ... */
/*
static PyObject*
py_gcompris_(PyObject* self, PyObject* args)
{
*/  /* Parse arguments */
/*  if(!PyArg_ParseTuple(args, ":gcompris_"))
    return NULL;
*/
  /* Call the corresponding C function */
/*  gcompris_();
*/
  /* Create and return the result */
/*  Py_INCREF(Py_None);
  return Py_None;
}
*/
/*
  { "",  py_gcompris_, METH_VARARGS, "gcompris_" },
*/
