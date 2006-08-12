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


/* void gcompris_end_board(void); */
static PyObject*
py_gcompris_end_board(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_end_board"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_end_board();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void	gcompris_bar_start (GnomeCanvas *theCanvas); */
static PyObject*
py_gcompris_bar_start(PyObject* self, PyObject* args)
{
  PyObject* pyCanvas;
  GnomeCanvas* canvas;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris_bar_start", &pyCanvas))
    return NULL;
  canvas = (GnomeCanvas*) pygobject_get(pyCanvas);

  /* Call the corresponding C function */
  gcompris_bar_start(canvas);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* GnomeCanvasItem *gcompris_set_background(GnomeCanvasGroup *parent, gchar *file); */
static PyObject*
py_gcompris_set_background(PyObject* self, PyObject* args)
{
  PyObject* pyCanvasGroup;
  GnomeCanvasGroup* canvasGroup;
  gchar* file;
  PyObject* pyResult;
  GnomeCanvasItem* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Os:gcompris_set_background", &pyCanvasGroup, &file))
    return NULL;
  canvasGroup = (GnomeCanvasGroup*) pygobject_get(pyCanvasGroup);

  /* Call the corresponding C function */
  result = gcompris_set_background(canvasGroup, file);

  /* Create and return the result */
  pyResult = pygobject_new((GObject*)result);
  return pyResult;
}


/* void gcompris_bar_set_level (GcomprisBoard *gcomprisBoard); */
static PyObject*
py_gcompris_bar_set_level(PyObject* self, PyObject* args)
{
  PyObject* pyObject;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris_bar_set_level", &pyObject))
    return NULL;
  pyGcomprisBoard = (pyGcomprisBoardObject*) pyObject;
  cGcomprisBoard = pyGcomprisBoard->cdata;

  /* Call the corresponding C function */
  gcompris_bar_set_level(cGcomprisBoard);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_bar_set_repeat_icon (GdkPixbuf *pixmap); */
static PyObject*
py_gcompris_bar_set_repeat_icon(PyObject* self, PyObject* args)
{
  PyObject* pyObject;
  GdkPixbuf* pixmap;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris_bar_set_repeat_icon", &pyObject))
    return NULL;
  pixmap = (GdkPixbuf*) pygobject_get(pyObject);

  /* Call the corresponding C function */
  gcompris_bar_set_repeat_icon(pixmap);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_bar_set (const GComprisBarFlags flags); */
static PyObject*
py_gcompris_bar_set(PyObject* self, PyObject* args)
{
  gint values;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gcompris_bar_set", &values))
    return NULL;

  /* Call the corresponding C function */
  gcompris_bar_set(values);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_bar_hide (gboolean hide); */
static PyObject*
py_gcompris_bar_hide(PyObject* self, PyObject* args)
{
  gint values;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gcompris_bar_hide", &values))
    return NULL;

  /* Call the corresponding C function */
  gcompris_bar_hide(values);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

/* gboolean gcompris_board_has_help (GcomprisBoard *gcomprisBoard); */
static PyObject*
py_gcompris_board_has_help(PyObject* self, PyObject* args)
{
  PyObject* pyObject;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  gboolean result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris_board_has_help", &pyObject))
    return NULL;
  pyGcomprisBoard = (pyGcomprisBoardObject*) pyObject;
  cGcomprisBoard = pyGcomprisBoard -> cdata;

  /* Call the corresponding C function */
  result = gcompris_board_has_help(cGcomprisBoard);

  /* Create and return the result */
  if(result){
    Py_INCREF(Py_True);
    return Py_True;
  } else {
    Py_INCREF(Py_False);
    return Py_False;
  }
}

/* void gcompris_help_start (GcomprisBoard *gcomprisBoard); */
static PyObject*
py_gcompris_help_start(PyObject* self, PyObject* args)
{
  PyObject* pyObject;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:gcompris_help_start", &pyObject))
    return NULL;
  pyGcomprisBoard = (pyGcomprisBoardObject*) pyObject;
  cGcomprisBoard = pyGcomprisBoard->cdata;

  /* Call the corresponding C function */
  gcompris_help_start(cGcomprisBoard);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_help_stop (void); */
static PyObject*
py_gcompris_help_stop(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_help_stop"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_help_stop();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

/* GnomeCanvas *gcompris_get_canvas(void); */
static PyObject*
py_gcompris_get_canvas(PyObject* self, PyObject* args)
{
  GnomeCanvas* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_canvas"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_canvas();

  /* Create and return the result */
  return (PyObject*) pygobject_new((GObject*) result);
}


/* GtkWidget *gcompris_get_window(void); */
static PyObject*
py_gcompris_get_window(PyObject* self, PyObject* args)
{
  GtkWidget* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_window"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_window();

  /* Create and return the result */
  return (PyObject*)pygobject_new((GObject*)result);
}


/* gchar *gcompris_get_locale(void); */
static PyObject*
py_gcompris_get_locale(PyObject* self, PyObject* args)
{
  gchar* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_locale"))
    return NULL;

  /* Call the corresponding C function */
  result = (gchar*)gcompris_get_locale();

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* void gcompris_set_locale(gchar *locale); */
static PyObject*
py_gcompris_set_locale(PyObject* self, PyObject* args)
{
  gchar* locale;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_set_locale", &locale))
    return NULL;

  /* Call the corresponding C function */
  gcompris_set_locale(locale);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* char *gcompris_get_user_default_locale(void) */
static PyObject*
py_gcompris_get_user_default_locale(PyObject* self, PyObject* args)
{
  char* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_user_default_locale"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_user_default_locale();

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* void gcompris_set_cursor(guint gdk_cursor_type); */
static PyObject*
py_gcompris_set_cursor(PyObject* self, PyObject* args)
{
  guint cursor;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "i:gcompris_set_cursor",&cursor))
    return NULL;

  /* Call the corresponding C function */
  gcompris_set_cursor(cursor);

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


/* void gcompris_images_selector_start (GcomprisBoard *gcomprisBoard,  */
/* 					gchar *dataset,  */
/* 					ImageSelectorCallBack imscb); */
static PyObject*
py_gcompris_images_selector_start(PyObject* self, PyObject* args)
{
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  PyObject* pyCallback;
  gchar* dataset;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "OsO:gcompris_images_selector_start",
		       &pyGcomprisBoard,
		       &dataset,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  pyImageSelectorCallBackFunc = pyCallback;
  gcompris_images_selector_start(cGcomprisBoard,
				 dataset,
				 pyImageSelectorCallBack);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_log_set_comment (GcomprisBoard *gcomprisBoard, gchar *expected, gchar *got); */
static PyObject*
py_gcompris_log_set_comment(PyObject* self, PyObject* args)
{
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  gchar* expected;
  gchar* got;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "Os:gcompris_log_set_comment",
		       &pyGcomprisBoard,
		       &expected,
		       &got))
    return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  gcompris_log_set_comment(cGcomprisBoard,
			   expected, got);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_log_end (GcomprisBoard *gcomprisBoard, gchar *status); */
static PyObject*
py_gcompris_log_end(PyObject* self, PyObject* args)
{
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  gchar* status;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "Os:gcompris_log_end",
		       &pyGcomprisBoard,
		       &status))
    return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  gcompris_log_end(cGcomprisBoard,
		   status);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_images_selector_stop (void); */
static PyObject*
py_gcompris_images_selector_stop(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_images_selector_stop"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_images_selector_stop();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_exit(); */
static PyObject*
py_gcompris_exit(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_exit"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_exit();

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


/* void gcompris_file_selector_load(GcomprisBoard *gcomprisBoard,
                                    gchar *rootdir,
				    gchar *file_types, (A Comma separated text explaining the different file types)
                                    FileSelectorCallBack fscb);
*/
static PyObject*
py_gcompris_file_selector_load(PyObject* self, PyObject* args){
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  PyObject* pyCallback;
  gchar* rootdir;
  gchar* file_types;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "OssO:gcompris_file_selector_load",
		       &pyGcomprisBoard,
		       &rootdir,
		       &file_types,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  pyFileSelectorCallBackFunc = pyCallback;
  gcompris_file_selector_load(cGcomprisBoard,
                              rootdir,
			      file_types,
                              pyFileSelectorCallBack);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_file_selector_save(GcomprisBoard *gcomprisBoard,
                                    gchar *rootdir,
				    gchar *file_types, (A Comma separated text explaining the different file types)
                                    FileSelectorCallBack fscb);
*/
static PyObject*
py_gcompris_file_selector_save(PyObject* self, PyObject* args){
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  PyObject* pyCallback;
  gchar* rootdir;
  char* file_types;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "OssO:gcompris_file_selector_save",
		       &pyGcomprisBoard,
		       &rootdir,
		       &file_types,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)) return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  pyFileSelectorCallBackFunc = pyCallback;
  gcompris_file_selector_save(cGcomprisBoard,
                              rootdir,
			      file_types,
                              pyFileSelectorCallBack);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_file_selector_stop (void); */
static PyObject*
py_gcompris_file_selector_stop(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_file_selector_stop"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_file_selector_stop();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

/* gchar *gcompris_get_database(void); */
static PyObject*
py_gcompris_get_database(PyObject* self, PyObject* args)
{
  gchar* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_database"))
    return NULL;

  /* Call the corresponding C function */
  result = (gchar*)gcompris_get_database();

  /* Create and return the result */
  return Py_BuildValue("s", result);
}


/* GcomprisProperties *gcompris_get_properties(void); */
static PyObject*
py_gcompris_get_properties(PyObject* self, PyObject* args)
{
  GcomprisProperties* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_properties"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_properties();

  /* Create and return the result */
  return gcompris_new_pyGcomprisPropertiesObject(result);
}

/* GcomprisBoard *gcompris_get_board_from_section(gchar *section); */
static PyObject*
py_gcompris_get_board_from_section(PyObject* self, PyObject* args)
{
  GcomprisBoard* result;
  gchar *section;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_get_board_from_section", &section))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_board_from_section(section);

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
    int pid;
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
    GPid child_pid = -1;
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
py_gcompris_get_board_conf(PyObject* self, PyObject* args)
{
  PyObject *pydict;
  GHashTable *table;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_board_conf"))
    return NULL;

  /* Call the corresponding C function */

  table = gcompris_get_board_conf();

  pydict = hash_to_dict(table);

  g_hash_table_destroy(table);

  return pydict;;
}

static PyObject*
py_gcompris_get_conf(PyObject* self, PyObject* args)
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
  table = gcompris_get_conf(cGcomprisProfile, cGcomprisBoard);

  pydict = hash_to_dict(table);

  g_hash_table_destroy(table);

  return pydict;;
}


/* GcomprisProfile *gcompris_get_current_profile(void); */
static PyObject*
py_gcompris_get_current_profile(PyObject* self, PyObject* args)
{
  GcomprisProfile* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_current_profile"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_current_profile();

  /* Create and return the result */
  return gcompris_new_pyGcomprisProfileObject(result);
}


/* GcomprisUser *gcompris_get_current_user(void); */
static PyObject*
py_gcompris_get_current_user(PyObject* self, PyObject* args)
{
  GcomprisUser* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_current_user"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_current_user();

  /* Create and return the result */
  return gcompris_new_pyGcomprisUserObject(result);
}


static PyObject*
py_gcompris_set_board_conf (PyObject* self, PyObject* args)
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
  gcompris_set_board_conf(cGcomprisProfile, cGcomprisBoard, key, value);

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
py_gcompris_configuration_window(PyObject* self, PyObject* args){
  PyObject* pyCallback;
  gchar *label;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "sO:gcompris_configuration_window",
		       &label,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback))
    {
      PyErr_SetString(PyExc_TypeError,
		      "gcompris_configuration_window second argument must be callable");
      return NULL;
    }

  //if (pyGcomprisConfCallbackFunc)
  //  Py_DECREF(pyGcomprisConfCallbackFunc);

  pyGcomprisConfCallbackFunc = pyCallback;

  Py_INCREF(pyGcomprisConfCallbackFunc);


  return (PyObject *) \
             pygobject_new((GObject*) \
			   gcompris_configuration_window( label,
							  (GcomprisConfCallback )pyGcomprisConfCallback));

}


/* GtkCheckButton *gcompris_boolean_box (label, key, init);*/
static PyObject*
py_gcompris_boolean_box(PyObject* self, PyObject* args)
{
  PyObject *py_bool;
  gchar *label;
  gchar *key;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssO:gcompris_boolean_box", &label, &key, &py_bool))
    return NULL;

  /* Call the corresponding C function */
  return (PyObject *)pygobject_new((GObject*) \
				    gcompris_boolean_box((const gchar *)label, key, PyObject_IsTrue(py_bool)));

}

/* GtkComboBox *gcompris_combo_box(const gchar *label, GList *strings, gchar *key, gint index); */
static PyObject*
py_gcompris_combo_box(PyObject* self, PyObject* args)
{
  PyObject *py_list;
  gchar *label;
  gchar *key;
  gchar *init;

  GList *list = NULL;

  int i, size;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "sOss:gcompris_combo_box", &label, &py_list, &key, &init))
    return NULL;

  if (!PyList_Check(py_list)){
    PyErr_SetString(PyExc_TypeError,
		      "gcompris_combo_box second argument must be a list");
    return NULL;
  }

  size = PyList_Size (py_list);

  for (i=0; i < size; i ++)
    list = g_list_append( list, 
			  PyString_AsString( PyList_GetItem( py_list, i)));

  /* Call the corresponding C function */
  return (PyObject *)pygobject_new((GObject*) \
				    gcompris_combo_box((const gchar *)label, 
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

/* GHashTable *gcompris_radio_buttons(const gchar *label, */
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
py_gcompris_radio_buttons(PyObject* self, PyObject* args)
{
  PyObject *py_dict;
  GHashTable *buttons_label, *result;
  gchar *label;
  gchar *key;
  gchar *init;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssOs:gcompris_radio_buttons", &label, &key, &py_dict, &init))
    return NULL;

  if (!PyDict_Check(py_dict)){
    PyErr_SetString(PyExc_TypeError,
		      "gcompris_radio_buttons second argument must be a dict");
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

  result = gcompris_radio_buttons(label, 
				  key, 
				  buttons_label,
				  init);

  g_hash_table_destroy(buttons_label);

  return hash_object_to_dict(result);
}

static PyObject*
py_gcompris_spin_int(PyObject* self, PyObject* args)
{
  gchar *label;
  gchar *key;
  gint min, max, step, init;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssiiii:gcompris_radio_buttons", &label, &key, &min, &max, &step, &init))
    return NULL;

  return (PyObject *)pygobject_new((GObject*) \
				   gcompris_spin_int((const gchar *)label, 
						     key,
						     min,
						     max,
						     step,
						     init));

}


/* GtkHSeparator *gcompris_separator(void); */
static PyObject*
py_gcompris_separator(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_separator"))
    return NULL;

  /* Create and return the result */
  return (PyObject *)pygobject_new((GObject*) gcompris_separator());

}


static PyObject*
py_gcompris_combo_locales(PyObject* self, PyObject* args)
{
  gchar *init;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_combo_locales", &init))
    return NULL;

  return (PyObject *)pygobject_new((GObject*) \
				   gcompris_combo_locales( init));
}


static PyObject*
py_gcompris_get_locales_list(PyObject* self, PyObject* args)
{
  PyObject *pylist;
  GList *result, *list ;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_locales_list"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_locales_list();

  pylist = PyList_New(0);

  for (list = result; list != NULL; list = list->next){
    PyList_Append( pylist, PyString_FromString(list->data));
  }

  Py_INCREF(pylist);
  return pylist;
}


static PyObject*
py_gcompris_combo_locales_asset(PyObject* self, PyObject* args)
{
  gchar *init;
  gchar *label;
  gchar *file;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "ssz:gcompris_combo_locales", 
		       &label, 
		       &init,
		       &file))
    return NULL;

  return (PyObject *)pygobject_new((GObject*) \
				   gcompris_combo_locales_asset( label, init, file ));
}


static PyObject*
py_gcompris_get_locales_asset_list(PyObject* self, PyObject* args)
{
  PyObject *pylist;
  GList *result, *list ;
  gchar *file;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "z:gcompris.get_locales_asset_list",
		       &file))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_locales_asset_list(file);

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



/* void gcompris_change_locale(gchar *locale); */
static PyObject*
py_gcompris_change_locale(PyObject* self, PyObject* args)
{
  gchar *locale;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris_change_locale", &locale))
    return NULL;

  /* Call the corresponding C function */
  gcompris_change_locale(locale);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gcompris_reset_locale(gchar *locale); */
static PyObject*
py_gcompris_reset_locale(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_reset_locale"))
    return NULL;

  /* Call the corresponding C function */
  gcompris_reset_locale();

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
py_gcompris_textview(PyObject* self, PyObject* args){
  PyObject* pyCallback;
  gchar *label;
  gchar *key;
  gchar *desc = NULL;
  gchar *init_text = NULL;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "sszzO:gcompris_configuration_window",
		       &label,
		       &key,
		       &desc,
		       &init_text,
		       &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback))
    {
      PyErr_SetString(PyExc_TypeError,
		      "gcompris_textview 5th argument must be callable");
      return NULL;
    }

  if (!text_callbacks)
    text_callbacks = g_hash_table_new ( g_str_hash, g_str_equal);

  g_hash_table_replace (text_callbacks, key, pyCallback);

  Py_INCREF(pyCallback);

  return (PyObject *) \
             pygobject_new((GObject*) \
			   gcompris_textview( label,
					      key,
					      desc,
					      init_text,
					      (GcomprisTextCallback )pyGcomprisTextCallback));

}


static PyObject*
py_gcompris_get_user_dirname (PyObject* self, PyObject* args)
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
  return PyString_FromString(gcompris_get_user_dirname(cGcomprisUser));

}

static PyObject*
py_gcompris_get_board_dirname (PyObject* self, PyObject* args)
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
  return PyString_FromString(gcompris_get_board_dirname(cGcomprisBoard));

}

static PyObject*
py_gcompris_get_current_user_dirname (PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_current_user_dirname"))
    return NULL;

  /* Call the corresponding C function */
  return PyString_FromString(gcompris_get_current_user_dirname());

}

static PyObject*
py_gcompris_get_current_board_dirname (PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_current_board_dirname"))
    return NULL;

  /* Call the corresponding C function */
  return PyString_FromString(gcompris_get_current_board_dirname());

}

static PyObject*
py_gcompris_get_wordlist_from_file (PyObject* self, PyObject* args)
{
  GcomprisWordlist *result;
  gchar *filename;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gcompris.get_wordlist", &filename))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_wordlist_from_file (filename);
  if (result)
    return gcompris_new_pyGcomprisWordlistObject(result);
  else {
    Py_INCREF(Py_None);
    return Py_None;
  }
}

static PyObject*
py_gcompris_im_reset (PyObject* self, PyObject* args)
{

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.im_reset"))
    return NULL;
  
  /* Call the corresponding C function */
  gcompris_im_reset ();
  
  Py_INCREF(Py_None);
  return Py_None;
}



/****************************************************/

static PyMethodDef PythonGcomprisModule[] = {
  { "end_board",  py_gcompris_end_board, METH_VARARGS, "gcompris_end_board" },
  { "bar_start",  py_gcompris_bar_start, METH_VARARGS, "gcompris_bar_start" },
  { "set_background",  py_gcompris_set_background, METH_VARARGS, "gcompris_set_background" },
  { "bar_set_level",  py_gcompris_bar_set_level, METH_VARARGS, "gcompris_bar_set_level" },
  { "bar_set_repeat_icon",  py_gcompris_bar_set_repeat_icon, METH_VARARGS, "gcompris_bar_set_repeat_icon" },
  { "bar_set",  py_gcompris_bar_set, METH_VARARGS, "gcompris_bar_set" },
  { "bar_hide",  py_gcompris_bar_hide, METH_VARARGS, "gcompris_bar_hide" },
  { "board_has_help",  py_gcompris_board_has_help, METH_VARARGS, "gcompris_board_has_help" },
  { "help_start",  py_gcompris_help_start, METH_VARARGS, "gcompris_help_start" },
  { "help_stop",  py_gcompris_help_stop, METH_VARARGS, "gcompris_help_stop" },
  { "get_canvas",  py_gcompris_get_canvas, METH_VARARGS, "gcompris_get_canvas" },
  { "get_window",  py_gcompris_get_window, METH_VARARGS, "gcompris_get_window" },
  { "get_locale",  py_gcompris_get_locale, METH_VARARGS, "gcompris_get_locale" },
  { "get_user_default_locale",  py_gcompris_get_user_default_locale, METH_VARARGS, "gcompris_get_user_default_locale" },
  { "set_locale",  py_gcompris_set_locale, METH_VARARGS, "gcompris_set_locale" },
  { "set_cursor",  py_gcompris_set_cursor, METH_VARARGS, "gcompris_set_cursor" },
  { "images_selector_start",  py_gcompris_images_selector_start,
    METH_VARARGS, "gcompris_images_selector_start" },
  { "images_selector_stop",  py_gcompris_images_selector_stop,
    METH_VARARGS, "gcompris_images_selector_stop" },
  { "exit",  py_gcompris_exit, METH_VARARGS, "gcompris_exit" },
  { "log_set_comment",  py_gcompris_log_set_comment, METH_VARARGS, "gcompris_log_set_comment" },
  { "log_end",  py_gcompris_log_end, METH_VARARGS, "gcompris_log_end" },
  { "file_selector_load",  py_gcompris_file_selector_load,
    METH_VARARGS, "gcompris_file_selector_load" },
  { "file_selector_save",  py_gcompris_file_selector_save,
    METH_VARARGS, "gcompris_file_selector_save" },
  { "file_selector_stop",  py_gcompris_file_selector_stop,
    METH_VARARGS, "gcompris_file_selector_stop" },
  { "get_database",  py_gcompris_get_database, METH_VARARGS, "gcompris_get_database" },
  { "get_properties",  py_gcompris_get_properties, METH_VARARGS, "gcompris_get_properties" },
  { "get_board_from_section",  py_gcompris_get_board_from_section, METH_VARARGS, "gcompris_get_board_from_section" },
  { "spawn_async",  (PyCFunction)py_gcompris_spawn_async, METH_VARARGS|METH_KEYWORDS, "gcompris_spawn_sync" },
  { "child_watch_add",  (PyCFunction)py_gcompris_child_watch_add, METH_VARARGS|METH_KEYWORDS, "gcompris_child_watch_add" },
  { "get_board_conf",  py_gcompris_get_board_conf, METH_VARARGS, "gcompris_get_board_conf" },
  { "get_conf",  py_gcompris_get_conf, METH_VARARGS, "gcompris_get_conf" },
  { "set_board_conf",  py_gcompris_set_board_conf, METH_VARARGS, "gcompris_set_board_conf" },
  { "get_current_profile",  py_gcompris_get_current_profile, METH_VARARGS, "gcompris_get_current_profile" },
  { "get_current_user",  py_gcompris_get_current_user, METH_VARARGS, "gcompris_get_current_user" },
  { "configuration_window",  py_gcompris_configuration_window, METH_VARARGS, "gcompris_configuration_window" },
  { "boolean_box",  py_gcompris_boolean_box, METH_VARARGS, "gcompris_boolean_box" },
  { "combo_box",  py_gcompris_combo_box, METH_VARARGS, "gcompris_combo_box" },
  { "radio_buttons",  py_gcompris_radio_buttons, METH_VARARGS, "gcompris_radio_buttons" },
  { "spin_int",  py_gcompris_spin_int, METH_VARARGS, "gcompris_spin_int" },
  { "separator",  py_gcompris_separator, METH_VARARGS, "gcompris_separator" },
  { "combo_locales",  py_gcompris_combo_locales, METH_VARARGS, "gcompris_combo_locales" },
  { "get_locales_list",  py_gcompris_get_locales_list, METH_VARARGS, "gcompris_get_locales_list" },
  { "gcompris_gettext",  py_gcompris_gettext, METH_VARARGS, "gcompris_gettext" },
  { "change_locale",  py_gcompris_change_locale, METH_VARARGS, "gcompris_change_locale" },
  { "reset_locale",  py_gcompris_reset_locale, METH_VARARGS, "gcompris_reset_locale" },
  { "combo_locales_asset",  py_gcompris_combo_locales_asset, METH_VARARGS, "gcompris_combo_locales_asset" },
  { "get_locales_asset_list",  py_gcompris_get_locales_asset_list, METH_VARARGS, "gcompris_get_locales_asset_list" },
  { "textview",  py_gcompris_textview, METH_VARARGS, "gcompris_textview" },
  { "get_user_dirname",  py_gcompris_get_user_dirname, METH_VARARGS, "gcompris_get_user_dirname" },
  { "get_current_user_dirname",  py_gcompris_get_current_user_dirname, METH_VARARGS, "gcompris_get_current_user_dirname" },
  { "get_board_dirname",  py_gcompris_get_board_dirname, METH_VARARGS, "gcompris_get_board_dirname" },
  { "get_current_board_dirname",  py_gcompris_get_current_board_dirname, METH_VARARGS, "gcompris_get_current_board_dirname" },
  { "get_wordlist",  py_gcompris_get_wordlist_from_file, METH_VARARGS, "gcompris_get_wordlist_from_file" },
  { "im_reset",  py_gcompris_im_reset, METH_VARARGS, "gcompris_im_reset" },
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
  PyModule_AddIntConstant(gcomprisModule, "BAR_LEVEL",       GCOMPRIS_BAR_LEVEL);
  PyModule_AddIntConstant(gcomprisModule, "BAR_OK",          GCOMPRIS_BAR_OK);
  PyModule_AddIntConstant(gcomprisModule, "BAR_REPEAT",      GCOMPRIS_BAR_REPEAT);
  PyModule_AddIntConstant(gcomprisModule, "BAR_REPEAT_ICON", GCOMPRIS_BAR_REPEAT_ICON);
  PyModule_AddIntConstant(gcomprisModule, "BAR_CONFIG",      GCOMPRIS_BAR_CONFIG);
  PyModule_AddIntConstant(gcomprisModule, "BAR_ABOUT",       GCOMPRIS_BAR_ABOUT);

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
  GcomprisProperties	*properties = gcompris_get_properties();
  PyModule_AddStringConstant(gcomprisModule, "DATA_DIR", properties->package_data_dir);

  /* GetText constants. */
  PyModule_AddStringConstant(gcomprisModule, "GETTEXT_PACKAGE", GETTEXT_PACKAGE);
  PyModule_AddStringConstant(gcomprisModule, "PACKAGE_LOCALE_DIR",  PACKAGE_LOCALE_DIR);


  /* Initialize the sub modules */
  python_gcompris_bonus_module_init();
  python_gcompris_score_module_init();
  python_gcompris_skin_module_init();
  python_gcompris_sound_module_init();
  python_gcompris_timer_module_init();
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
