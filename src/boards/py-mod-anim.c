#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-gcompris-board.h"
#include "py-mod-anim.h"

#define THROW_INACTIVE_ANIMATION                                        \
{                                                                       \
    PyErr_SetString(PyExc_RuntimeError, "Tried to access an inactive "  \
                                        "AnimCanvas");                  \
    return NULL;                                                        \
}

static int Animation_init(py_GcomprisAnimation *self, PyObject*, PyObject*);
static void Animation_free(py_GcomprisAnimation *self);

static int AnimCanvas_init(py_GcomprisAnimCanvas*, PyObject*, PyObject*);
static void AnimCanvas_free(py_GcomprisAnimCanvas*);
static PyObject *AnimCanvas_getattr(py_GcomprisAnimCanvas*, char*);

/* AnimCanvas methods */
static PyObject *py_gcompris_animcanvas_setstate(PyObject*, PyObject*);
static PyObject *py_gcompris_animcanvas_swapanim(PyObject*, PyObject*);
static PyObject *py_gcompris_animcanvas_destroy(PyObject*, PyObject*);

static PyMethodDef AnimCanvasMethods[] = {
  {"setState", py_gcompris_animcanvas_setstate, METH_VARARGS,
    "gcompris_animcanvas_setstate"},
  {"swapAnimation", py_gcompris_animcanvas_swapanim, METH_VARARGS,
    "gcompris_animcanvas_swapanim"},
  {"destroy", py_gcompris_animcanvas_destroy, METH_VARARGS,
    "gcompris_animcanvas_destroy"},
  {NULL, NULL, 0, NULL}
};

static PyMethodDef AnimationMethods[] = {
  {NULL, NULL, 0, NULL}
};

static PyTypeObject py_GcomprisAnimationType = {
#if defined(WIN32)
  PyObject_HEAD_INIT(NULL)
#else /* ! WIN32 */
  PyObject_HEAD_INIT(&PyType_Type)
#endif
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

static PyTypeObject py_GcomprisAnimCanvasType = {
#if defined(WIN32)
  PyObject_HEAD_INIT(NULL)
#else /* ! WIN32 */
  PyObject_HEAD_INIT(&PyType_Type)
#endif
  0,                                /* ob_size */
  "pyGcomprisAnimCanvas",           /* tp_name */
  sizeof(py_GcomprisAnimCanvas),    /* tp_basicsize */
  0,                                /* tp_itemsize */
  (destructor)AnimCanvas_free,      /* tp_dealloc */
  0,                                /* tp_print */
  (getattrfunc)AnimCanvas_getattr,  /* tp_getattr */
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
  "Animated canvas objects",    /* tp_doc */
  0,                            /* tp_traverse */
  0,                            /* tp_clear */
  0,                            /* tp_richcompare */
  0,                            /* tp_weaklistoffset */
  0,                            /* tp_iter */
  0,                            /* tp_iternext */
  AnimCanvasMethods,            /* tp_methods */
  0,                            /* tp_members */
  0,                            /* tp_getset */
  0,                            /* tp_base */
  0,                            /* tp_dict */
  0,                            /* tp_descr_get */
  0,                            /* tp_descr_set */
  0,                            /* tp_dictoffset */
  (initproc)AnimCanvas_init,    /* tp_init */
  0,                            /* tp_alloc */
  0,                            /* tp_new */
};

static PyMethodDef PythonGcomprisAnimModule[] = {
  {NULL, NULL, 0, NULL}
};

/*============================================================================*/
/*                      GcomprisAnimation functions                           */
/*============================================================================*/
static int
Animation_init(py_GcomprisAnimation *self, PyObject *args, PyObject *key)
{
  static char *kwlist[] =
  {
    "filename", NULL
  };

  char *file=NULL;

  if(!PyArg_ParseTupleAndKeywords(args, key, "|s", kwlist,
                                   &file))
    {
      PyErr_SetString(PyExc_RuntimeError, "Invalid arguments to Animation()");
      return -1;
    }

  if(file)
    {
      self->a = gc_anim_load(file);
    }

  if(!self->a)
    {
      PyErr_SetString(PyExc_RuntimeError, "Failed to load Animation");
      return -1;
    }
  return 0;
}

static void Animation_free(py_GcomprisAnimation *self)
{
  g_warning("*** Garbage collecting Animation ***\n");
  if( self->a)
      gc_anim_free(self->a);
  PyObject_DEL(self);
}

/*============================================================================*/
/*                            Animation Methods                               */
/*============================================================================*/

static int
AnimCanvas_init(py_GcomprisAnimCanvas *self, PyObject *args, PyObject *key)
{
  GcomprisAnimCanvasItem *item;
  GcomprisAnimation *anim;
  GnomeCanvasGroup *parent;
  PyObject *py_p, *py_a;

  if(!PyArg_ParseTuple(args, "OO:AnimCanvas_init", &py_a, &py_p)) {
      PyErr_SetString(PyExc_RuntimeError, "Invalid arguments to AnimCanvas()");
      return -1;
  }
  if(!PyObject_TypeCheck(py_a, &py_GcomprisAnimationType) ||
     !PyObject_TypeCheck(py_p,pygobject_lookup_class(GNOME_TYPE_CANVAS_GROUP))){

      PyErr_SetString(PyExc_TypeError, "AnimCanvas() needs an Animation");
      return -1;
  }

  parent = (GnomeCanvasGroup*) pygobject_get(py_p);
  anim = ( (py_GcomprisAnimation*)py_a )->a;
  item = (GcomprisAnimCanvasItem*) gc_anim_activate(parent, anim);
  self->item = item;
  self->anim = py_a;

  Py_INCREF(self->anim);
  return 0;
}

static void
AnimCanvas_free(py_GcomprisAnimCanvas *self)
{
  g_warning("*** garbage collecting AnimCanvas ***\n");
  if(self->item)
    {
      g_warning("You should really call destroy() on an AnimCanvas "
                "instead of relying on the refcounter\n");
      gc_anim_deactivate(self->item);
      Py_DECREF(self->anim);
    }
  PyObject_DEL(self);
}

static PyObject *AnimCanvas_getattr(py_GcomprisAnimCanvas *self, char *name)
{
  if(!strcmp(name, "gnomecanvas"))
    return (PyObject*) pygobject_new( (GObject*) self->item->canvas );
  else if(!strcmp(name, "num_states"))
    return Py_BuildValue("i", self->item->anim->numstates);

  return Py_FindMethod(AnimCanvasMethods, (PyObject *)self, name);
}

static PyObject*
py_gcompris_animcanvas_setstate(PyObject *self, PyObject *args)
{
  int state;
  GcomprisAnimCanvasItem *item = ( (py_GcomprisAnimCanvas*)self )->item;

  if(!item) THROW_INACTIVE_ANIMATION;

  if(!PyArg_ParseTuple(args, "i:gcompris_animcanvas_setstate", &state))
    return NULL;

  gc_anim_set_state( item, state );

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
py_gcompris_animcanvas_swapanim(PyObject *self, PyObject *args)
{
  py_GcomprisAnimCanvas *s = (py_GcomprisAnimCanvas*)self;
  py_GcomprisAnimation *new_anim;
  py_GcomprisAnimation *old_anim = (py_GcomprisAnimation*)s->anim;
  GcomprisAnimCanvasItem *item = s->item;

  if(!item) THROW_INACTIVE_ANIMATION;

  if(!PyArg_ParseTuple(args, "O:AnimCanvas_swapAnim", (PyObject**)&new_anim))
    return NULL;

  gc_anim_swap(item, new_anim->a);
  Py_INCREF(new_anim);
  s->anim = (PyObject*)new_anim;
  Py_DECREF(old_anim);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
py_gcompris_animcanvas_destroy(PyObject *self, PyObject *args)
{
  py_GcomprisAnimCanvas *s = (py_GcomprisAnimCanvas*)self;

  if(!s->item) THROW_INACTIVE_ANIMATION;

  gc_anim_deactivate(s->item);
  Py_DECREF(s->anim);
  s->item = NULL;
  s->anim = NULL;

  Py_INCREF(Py_None);
  return Py_None;
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
  py_GcomprisAnimCanvasType.tp_new = PyType_GenericNew;
  py_GcomprisAnimCanvasType.ob_type = &PyType_Type;
  if (PyType_Ready(&py_GcomprisAnimationType) < 0)
      return;
  if (PyType_Ready(&py_GcomprisAnimCanvasType) < 0)
      return;

  m = Py_InitModule("_gcompris_anim", PythonGcomprisAnimModule);

  Py_INCREF(&py_GcomprisAnimationType);
  Py_INCREF(&py_GcomprisAnimCanvasType);
  PyModule_AddObject(m, "Animation", (PyObject *)&py_GcomprisAnimationType);
  PyModule_AddObject(m, "CanvasItem", (PyObject *)&py_GcomprisAnimCanvasType);
}

