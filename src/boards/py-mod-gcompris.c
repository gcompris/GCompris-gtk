#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-gcompris.h"
#include "py-gcompris-board.h"
#include "py-gcompris-properties.h"
#include "py-gcompris-profile.h"

/* submodules includes */
#include "py-mod-bonus.h"
#include "py-mod-score.h"
#include "py-mod-skin.h"
#include "py-mod-sound.h"
#include "py-mod-timer.h"
#include "py-mod-utils.h"
#include "py-mod-anim.h"

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


/* GcomprisProfile *gcompris_get_profile(void); */
static PyObject*
py_gcompris_get_profile(PyObject* self, PyObject* args)
{
  GcomprisProfile* result;
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris_get_profile"))
    return NULL;

  /* Call the corresponding C function */
  result = gcompris_get_profile();

  /* Create and return the result */
  return gcompris_new_pyGcomprisProfileObject(result);
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


static PyObject*
py_board_run_next(PyObject* self, PyObject* args)
{
  PyObject* pyObject;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "O:board_run_next", &pyObject))
    return NULL;
  pyGcomprisBoard = (pyGcomprisBoardObject*) pyObject;
  cGcomprisBoard = pyGcomprisBoard->cdata;

  /* Call the corresponding C function */
  board_run_next(cGcomprisBoard);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;

}

static PyObject*
py_board_config_start(PyObject* self, PyObject* args)
{
  PyObject* pyBoard;
  PyObject* pyCanvasGroup;
  pyGcomprisBoardObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  GnomeCanvasGroup *canvasgroup;
  int x, y, width, height;
  

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "OOiiii:board_config_start", &pyBoard,
		       &pyCanvasGroup, &x, &y, &width, &height))
    return NULL;
  pyGcomprisBoard = (pyGcomprisBoardObject*) pyBoard;
  cGcomprisBoard = pyGcomprisBoard->cdata;
  canvasgroup = (GnomeCanvasGroup*) pygobject_get(pyCanvasGroup);

  /* Call the corresponding C function */
  board_config_start(cGcomprisBoard, canvasgroup, x, y, width, height);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
py_board_config_stop(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.board_config_stop"))
    return NULL;

  /* Call the corresponding C function */
  board_config_stop();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
py_gcompris_get_boards_list(PyObject* self, PyObject* args)
{
  GList *boards_list;
  GList *list;
  PyObject *pylist;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gcompris.get_boards_list"))
    return NULL;

  /* Call the corresponding C function */
  boards_list = gcompris_get_boards_list();

  pylist = PyList_New(0);
  for (list = boards_list; list != NULL; list = list->next){
    PyList_Append(pylist, gcompris_new_pyGcomprisBoardObject((GcomprisBoard*) list->data));
  }
  /* Create and return the result */
  return pylist;;
}

/*
  { "",  py_gcompris_, METH_VARARGS, "gcompris_" },
*/



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
  { "spawn_async",  py_gcompris_spawn_async, METH_VARARGS|METH_KEYWORDS, "gcompris_spawn_sync" },
  { "child_watch_add",  py_gcompris_child_watch_add, METH_VARARGS|METH_KEYWORDS, "gcompris_child_watch_add" },
  { "board_run_next",  py_board_run_next, METH_VARARGS, "board_run_next" },
  { "board_config_start",  py_board_config_start, METH_VARARGS, "board_config_start" },
  { "board_config_stop",  py_board_config_stop, METH_VARARGS, "board_config_stop" },
  { "get_boards_list",  py_gcompris_get_boards_list, METH_VARARGS, "gcompris_get_boards_list" },
  { "get_profile",  py_gcompris_get_profile, METH_VARARGS, "gcompris_get_profile" },
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
