#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-gcompris-board.h"
#include "py-mod-anim.h"

static int Animation_init(py_GcomprisAnimation *self, PyObject*, PyObject*);
static void Animation_free(py_GcomprisAnimation *self);

/* Animation methods */
static PyObject *py_gcompris_activate_animation_m(PyObject *self,
                                                  PyObject *args);

/* global methods */
static PyObject *py_gcompris_activate_animation(PyObject *self,
                                                PyObject *args);
static PyObject *py_gcompris_deactivate_animation(PyObject *self,
                                                  PyObject *args);

static PyMethodDef AnimationMethods[] = {
  {"activate", py_gcompris_activate_animation_m, METH_VARARGS,
    "gcompris_activate_animation_m"},
  {"deactivate", py_gcompris_deactivate_animation, METH_VARARGS,
    "gcompris_deactivate_animation"},
  {NULL}
};

static PyTypeObject py_GcomprisAnimationType = {
  PyObject_HEAD_INIT(NULL)
  0,                            /* ob_size */
  "pyGcomprisAnimation",        /* tp_name */
  sizeof(py_GcomprisAnimation), /* tp_basicsize */
  0,                            /* tp_itemsize */
  (destructor)Animation_free,   /* tp_dealloc */
  0,                            /* tp_print */
  0,                            /* tp_getattr */
  0,                            /* tp_setattr */
  0,                            /* tp_compare */
  0,                            /* tp_repr */
  0,                            /* tp_as_number */
  0,                            /* tp_as_sequence */
  0,                            /* tp_as_mapping */
  0,                            /* tp_hash */
  0,                            /* tp_call */
  0,                            /* tp_str */
  0,                            /* tp_getattro */
  0,                            /* tp_setattro */
  0,                            /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,           /* tp_flags */
  "Animation objects",          /* tp_doc */
  0,                            /* tp_traverse */
  0,                            /* tp_clear */
  0,                            /* tp_richcompare */
  0,                            /* tp_weaklistoffset */
  0,                            /* tp_iter */
  0,                            /* tp_iternext */
  AnimationMethods,             /* tp_methods */
  0,                            /* tp_members */
  0,                            /* tp_getset */
  0,                            /* tp_base */
  0,                            /* tp_dict */
  0,                            /* tp_descr_get */
  0,                            /* tp_descr_set */
  0,                            /* tp_dictoffset */
  (initproc)Animation_init,     /* tp_init */
  0,                            /* tp_alloc */
  0,                            /* tp_new */
};

static PyMethodDef PythonGcomprisAnimModule[] = {
  {"activate_animation", py_gcompris_activate_animation, METH_VARARGS,
    "gcompris_activate_animation"},
  {"deactivate_animation", py_gcompris_deactivate_animation, METH_VARARGS,
    "gcompris_deactivate_animation"},
  {NULL}
};

static int
Animation_init(py_GcomprisAnimation *self, PyObject *args, PyObject *key)
{
  static char *kwlist[] =
  {
    "filename", "dataset", "categories", "mimetype", "name", NULL
  };

  char *file=NULL, *data=NULL, *cat=NULL, *mime=NULL, *name=NULL;

  if(!PyArg_ParseTupleAndKeywords(args, key, "|sssss", kwlist,
                                   &file, &data, &cat, &mime, &name))
    {
      return -1;
    }

  if(file)
    {
      self->a = gcompris_load_animation(file);
    }
  else
    {
      if( !data || !cat || !mime || !name )
          return -1;
      self->a = gcompris_load_animation_asset(data, cat, mime, name);
    }

  if(!self->a)
    {
      return -1;
    }
  return 0;
}

static void Animation_free(py_GcomprisAnimation *self)
{
  gcompris_free_animation(self->a);
}

/*============================================================================*/
/*                            Animation Methods                               */
/*============================================================================*/

static PyObject*
py_gcompris_activate_animation_m(PyObject *s, PyObject *args)
{
  GnomeCanvasItem *item;
  GnomeCanvasGroup *parent;
  PyObject *py_p;
  py_GcomprisAnimation *self = (py_GcomprisAnimation*) s;

  if(!PyArg_ParseTuple(args, "O:gcompris_activate_animation_m", &py_p))
      return NULL;

  parent = (GnomeCanvasGroup*) pygobject_get(py_p);
  item = (GnomeCanvasItem*) gcompris_activate_animation(parent, self->a);
  return (PyObject*) pygobject_new((GObject*) item);
}

static PyObject*
py_gcompris_deactivate_animation(PyObject *s, PyObject *args)
{
  GnomeCanvasItem *item;
  PyObject *pyitem;

  if(!PyArg_ParseTuple(args, "O:gcompris_deactivate_animation", &pyitem) )
      return NULL;

  item = (GnomeCanvasItem*) pygobject_get(pyitem);
  gcompris_deactivate_animation(item);
  Py_INCREF(Py_None);
  return Py_None;
}

/*============================================================================*/
/*                               Global Methods                               */
/*============================================================================*/

static PyObject*
py_gcompris_activate_animation(PyObject *self, PyObject *args)
{
  py_GcomprisAnimation *anim;
  PyObject *py_p;
  GnomeCanvasGroup *parent;

  if(!PyArg_ParseTuple(args, "OO:gcompris_activate_animation", &anim, &py_p))
      return NULL;

  parent = (GnomeCanvasGroup*) pygobject_get(py_p);

  GnomeCanvasItem *item;
  item = (GnomeCanvasItem*) gcompris_activate_animation(parent, anim->a);
  return (PyObject*) pygobject_new((GObject*) item);
}

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
python_gcompris_anim_module_init(void) 
{
  PyObject* m;

  py_GcomprisAnimationType.tp_new = PyType_GenericNew;
  py_GcomprisAnimationType.ob_type = &PyType_Type;
  if (PyType_Ready(&py_GcomprisAnimationType) < 0)
      return;

  m = Py_InitModule("_gcompris_anim", PythonGcomprisAnimModule);

  Py_INCREF(&py_GcomprisAnimationType);
  PyModule_AddObject(m, "Animation", (PyObject *)&py_GcomprisAnimationType);
}

