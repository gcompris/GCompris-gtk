#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-gcompris.h"
#include "py-gcompris-board.h"

/* submodules includes */
#include "py-mod-bonus.h"
#include "py-mod-score.h"
#include "py-mod-skin.h"
#include "py-mod-sound.h"
#include "py-mod-timer.h"
#include "py-mod-utils.h"

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
  result = gcompris_get_locale();

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


/* void gcompris_log_set_comment (GcomprisBoard *gcomprisBoard, gchar *comment); */
static PyObject*
py_gcompris_log_set_comment(PyObject* self, PyObject* args)
{
  PyObject* pyGcomprisBoard;
  GcomprisBoard* cGcomprisBoard;
  gchar* comment;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args,
		       "Os:gcompris_log_set_comment",
		       &pyGcomprisBoard,
		       &comment))
    return NULL;
  cGcomprisBoard = ((pyGcomprisBoardObject*) pyGcomprisBoard)->cdata;

  /* Call the corresponding C function */
  gcompris_log_set_comment(cGcomprisBoard,
			   comment);

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
  PyModule_AddIntConstant(gcomprisModule, "BAR_LEVEL",  GCOMPRIS_BAR_LEVEL);
  PyModule_AddIntConstant(gcomprisModule, "BAR_OK",     GCOMPRIS_BAR_OK);
  PyModule_AddIntConstant(gcomprisModule, "BAR_REPEAT", GCOMPRIS_BAR_REPEAT);
  PyModule_AddIntConstant(gcomprisModule, "BAR_CONFIG", GCOMPRIS_BAR_CONFIG);
  PyModule_AddIntConstant(gcomprisModule, "BAR_ABOUT",  GCOMPRIS_BAR_ABOUT);

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
  PyModule_AddStringConstant(gcomprisModule, "DATA_DIR", PACKAGE_DATA_DIR);

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
