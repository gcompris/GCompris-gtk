#include <Python.h>
#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-mod-utils.h"
#include "py-gcompris-board.h"

/* All functions provided by this python module
 * wraps a gcompris function. Each "py_*" function wraps the
 * "*" C function.
 */


/* GdkPixbuf *gc_pixmap_load(char *pixmapfile); */
static PyObject*
py_gc_pixmap_load(PyObject* self, PyObject* args)
{
  char* pixmapfile;
  GdkPixbuf* result;
  PyObject* pyresult;

  /* Parse arguments */

  if(!PyArg_ParseTuple(args, "s:gc_pixmap_load", &pixmapfile))
    return NULL;

  /* Call the corresponding C function */
  result = gc_pixmap_load(pixmapfile);

  /* Create and return the result */
  pyresult = (PyObject*) pygobject_new((GObject*) result);

  gdk_pixbuf_unref(result);

  return(pyresult);

}

/* gchar *gc_file_find_absolute(gchar *file); */
static PyObject*
py_gc_file_find_absolute(PyObject* self, PyObject* args)
{
  char* file;
  gchar* result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "s:gc_db_get_filename", &file))
    return NULL;

  /* Call the corresponding C function */
  result = (gchar*)gc_file_find_absolute(file);

  /* Create and return the result */
  return Py_BuildValue("s", result);
}



/* void	gc_item_focus_set(GnomeCanvasItem *item, gboolean focus); */
static PyObject*
py_gc_item_focus_set(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  gint pyfocus;
  gboolean focus;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Oi:gc_item_focus_set", &pyitem, &pyfocus))
    return NULL;

  item = (GnomeCanvasItem*) pygobject_get(pyitem);
  if(pyfocus>0) focus = TRUE;
  else focus = FALSE;

  /* Call the corresponding C function */
  gc_item_focus_set(item, focus);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* gint	gc_item_focus_event(GnomeCanvasItem *item,
                                  GdkEvent *event,
                                  GnomeCanvasItem *dest_item);
*/
static PyObject*
py_gc_item_focus_event(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  PyObject* pyevent;
  GdkEvent* event;
  gint result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "OO:gc_item_focus_event", &pyitem, &pyevent))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);
  event = (GdkEvent*) pygobject_get(pyevent);

  /* Call the corresponding C function */
  result = gc_item_focus_event(item, event, NULL);

  /* Create and return the result */
  return Py_BuildValue("i", result);
}


/* void gc_item_absolute_move(GnomeCanvasItem *item, int x, int y); */
static PyObject*
py_gc_item_absolute_move(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  int x, y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Oii:gc_item_absolute_move", &pyitem, &x, &y))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_absolute_move(item, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate(GnomeCanvasItem *item, double angle); */
static PyObject*
py_gc_item_rotate(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Od:gc_item_rotate_relative", &pyitem, &angle))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate(item, angle);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate_relative(GnomeCanvasItem *item, double angle); */
static PyObject*
py_gc_item_rotate_relative(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Od:gc_item_rotate_relative", &pyitem, &angle))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate_relative(item, angle);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate_with_center(GnomeCanvasItem *item, double angle, int x, int y); */
static PyObject*
py_gc_item_rotate_with_center(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;
  int x,y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Odii:gc_item_rotate_with_center", &pyitem, &angle, &x, &y))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate_with_center(item, angle, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void gc_item_rotate_relative_with_center(GnomeCanvasItem *item, double angle, int x, int y); */
static PyObject*
py_gc_item_rotate_relative_with_center(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem* item;
  double angle;
  int x,y;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Odii:gc_item_rotate_relative_with_center", &pyitem, &angle, &x, &y))
    return NULL;
  item = (GnomeCanvasItem*) pygobject_get(pyitem);

  /* Call the corresponding C function */
  gc_item_rotate_relative_with_center(item, angle, x, y);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* Dialog callback wrapper */
static PyObject* pyDialogBoxCallBackFunc = NULL;

/* typedef void (*DialogBoxCallBack) (); */
void pyDialogBoxCallBack(){
  PyObject* result;
  if(pyDialogBoxCallBackFunc==NULL) return;

  /* Build arguments */
  result = PyObject_CallObject(pyDialogBoxCallBackFunc, NULL);
  if(result==NULL){
    PyErr_Print();
  } else {
    Py_DECREF(result);
  }
}


/* void	gc_dialog(gchar *str, DialogBoxCallBack dbcb); */
static PyObject*
py_gc_dialog(PyObject* self, PyObject* args)
{
  PyObject* pyCallback;
  gchar* str;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "sO:gc_dialog", &str, &pyCallback))
    return NULL;
  if(!PyCallable_Check(pyCallback)){
    gc_dialog(str,NULL);
  }
  else {
  /* Call the corresponding C function */
  pyDialogBoxCallBackFunc = pyCallback ;
  gc_dialog(str, pyDialogBoxCallBack);
  }
  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void	gc_dialog_close(); */
static PyObject*
py_gc_dialog_close(PyObject* self, PyObject* args)
{
  /* Parse arguments */
  if(!PyArg_ParseTuple(args, ":gc_dialog"))
    return NULL;

  /* Call the corresponding C function */
  gc_dialog_close();

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;
}


/* void filename_pass(PyObject *pyitem, PyObject *pystring) */
static PyObject*
py_gcompris_filename_pass(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem *item;
  char *string;


  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Os:gcompris_filename_pass", &pyitem, &string))
    return NULL;

  /* pass parameter from python */
  item = (GnomeCanvasItem *) pygobject_get(pyitem);

  /* gcompris_filename_pass( item, string); */
  g_object_set_data( G_OBJECT(item), "filename", string);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;

}

/*
 * Set a property in a canvas object
 * ---------------------------------
 */
/* void canvas_set_property(GnomeCanvasItem *item, gchar *property, gchar* value) */
static PyObject*
py_gcompris_canvas_set_property(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem *item;
  char *property;
  char *value;


  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Oss:gcompris_canvas_set_property", &pyitem, &property, &value))
    return NULL;

  /* pass parameter from python */
  item = (GnomeCanvasItem *) pygobject_get(pyitem);

  /* gcompris_filename_pass( item, string); */
  g_object_set_data( G_OBJECT(item), property, value);

  /* Create and return the result */
  Py_INCREF(Py_None);
  return Py_None;

}

/*
 * Get a property in a canvas object
 * ---------------------------------
 */
/* gchar *canvas_get_property(GnomeCanvasItem *item, gchar *property) */
static PyObject*
py_gcompris_canvas_get_property(PyObject* self, PyObject* args)
{
  PyObject* pyitem;
  GnomeCanvasItem *item;
  char  *property;
  gchar *result;

  /* Parse arguments */
  if(!PyArg_ParseTuple(args, "Os:gcompris_canvas_get_property", &pyitem, &property))
    return NULL;

  /* pass parameter from python */
  item = (GnomeCanvasItem *) pygobject_get(pyitem);

  /* gcompris_filename_pass( item, string); */
  result = g_object_get_data( G_OBJECT(item), property);

  /* Create and return the result */
  if(result==NULL) {
    Py_INCREF(Py_None);
    return Py_None;
  } else {
    return Py_BuildValue("s", result);
  }

}


static PyMethodDef PythonGcomprisUtilsModule[] = {
  { "load_pixmap",  py_gc_pixmap_load, METH_VARARGS, "gc_pixmap_load" },
  { "find_file_absolute",  py_gc_file_find_absolute, METH_VARARGS, "gc_file_find_absolute" },
  { "set_image_focus",  py_gc_item_focus_set, METH_VARARGS, "gc_item_focus_set" },
  { "item_event_focus",  py_gc_item_focus_event, METH_VARARGS, "gc_item_focus_event" },
  { "item_absolute_move",  py_gc_item_absolute_move, METH_VARARGS, "gc_item_absolute_move" },
  { "item_rotate",  py_gc_item_rotate, METH_VARARGS, "gc_item_rotate" },
  { "item_rotate_relative",  py_gc_item_rotate_relative, METH_VARARGS, "gc_item_rotate_relative" },
  { "item_rotate__with_center",  py_gc_item_rotate_with_center, METH_VARARGS,
    "item_rotate_with_center" },
  { "item_rotate_relative_with_center",  py_gc_item_rotate_relative_with_center, METH_VARARGS,
    "item_rotate_relative_with_center" },
  { "dialog",  py_gc_dialog, METH_VARARGS, "gc_dialog" },
  { "dialog_close",  py_gc_dialog_close, METH_VARARGS, "gc_dialog_close" },
  { "filename_pass",  py_gcompris_filename_pass, METH_VARARGS, "gcompris_filename_pass" },
  { "canvas_set_property",  py_gcompris_canvas_set_property, METH_VARARGS, "gcompris_canvas_set_property" },
  { "canvas_get_property",  py_gcompris_canvas_get_property, METH_VARARGS, "gcompris_canvas_get_property" },
  { NULL, NULL, 0, NULL}
};


void python_gcompris_utils_module_init(void)
{
  PyObject* module;
  module = Py_InitModule("_gcompris_utils", PythonGcomprisUtilsModule);
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
