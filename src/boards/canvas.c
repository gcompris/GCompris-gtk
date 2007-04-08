/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>

#if PY_VERSION_HEX < 0x02050000
/* use int based index for python 2.4 and below */
typedef int Py_ssize_t;
#endif

#line 4 "canvas.override"
#include <Python.h>

#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"
#include <libgnomecanvas/libgnomecanvas.h>
#include <libgnomecanvas/gnome-canvas-clipgroup.h>

static int
gnomecanvasaffine_to_value(PyObject *py_affine, double affine[6])
{
    int i;

    if (PySequence_Length(py_affine) != 6) {
	PyErr_SetString(PyExc_TypeError, "argument must be a 6 tuple of floats.");
	return -1;
    }
    for (i = 0; i < 6; i++) {
	PyObject *sitem = PySequence_GetItem(py_affine, i);
	Py_DECREF(sitem);
	sitem = PyNumber_Float(sitem);
	if (sitem)
	    affine[i] = PyFloat_AsDouble(sitem);
	else {
    PyErr_Clear();
	    PyErr_SetString(PyExc_TypeError, "sequence item not a float");
	    return -1;
	}
	Py_DECREF(sitem);
    }
    return 0;
}

static PyObject *
gnomecanvasaffine_from_value(const double affine[6])
{
    return Py_BuildValue("(dddddd)", affine[0], affine[1], affine[2],
			 affine[3], affine[4], affine[5]);
}

#ifndef GNOME_TYPE_CANVAS_PATH_DEF

static GType
gnome_canvas_path_def_get_type(void)
{
    static GType type = 0;

    if (type == 0)
	type = g_boxed_type_register_static
	    ("GnomeCanvasPathDef",
	     (GBoxedCopyFunc) gnome_canvas_path_def_ref,
	     (GBoxedFreeFunc) gnome_canvas_path_def_unref);
    return type;
}

#define GNOME_TYPE_CANVAS_PATH_DEF (gnome_canvas_path_def_get_type())

#endif

#line 67 "canvas.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyGtkLayout_Type;
#define PyGtkLayout_Type (*_PyGtkLayout_Type)
static PyTypeObject *_PyGtkObject_Type;
#define PyGtkObject_Type (*_PyGtkObject_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyGnomeCanvasPathDef_Type;
PyTypeObject PyGnomeCanvas_Type;
PyTypeObject PyGnomeCanvasItem_Type;
PyTypeObject PyGnomeCanvasGroup_Type;
PyTypeObject PyGnomeCanvasClipgroup_Type;
PyTypeObject PyGnomeCanvasLine_Type;
PyTypeObject PyGnomeCanvasPixbuf_Type;
PyTypeObject PyGnomeCanvasRichText_Type;
PyTypeObject PyGnomeCanvasShape_Type;
PyTypeObject PyGnomeCanvasRE_Type;
PyTypeObject PyGnomeCanvasRect_Type;
PyTypeObject PyGnomeCanvasEllipse_Type;
PyTypeObject PyGnomeCanvasPolygon_Type;
PyTypeObject PyGnomeCanvasBpath_Type;
PyTypeObject PyGnomeCanvasText_Type;
PyTypeObject PyGnomeCanvasWidget_Type;


/* ----------- GnomeCanvasPathDef ----------- */

static int
pygobject_no_constructor(PyObject *self, PyObject *args, PyObject *kwargs)
{
    gchar buf[512];

    g_snprintf(buf, sizeof(buf), "%s is an abstract widget", self->ob_type->tp_name);
    PyErr_SetString(PyExc_NotImplementedError, buf);
    return -1;
}

PyTypeObject PyGnomeCanvasPathDef_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasPathDef",			/* tp_name */
    sizeof(PyGBoxed),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)pygobject_no_constructor,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvas ----------- */

#line 262 "canvas.override"
static int
_wrap_gnome_canvas_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    int aa = 0;
    static char *kwlist[] = { "aa", NULL };
    GType obj_type = pyg_type_from_object((PyObject *) self);

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i:GnomeCanvas.__init__",
				     kwlist, &aa))
        return -1;

    self->obj = g_object_new(obj_type, "aa", aa, NULL);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create GnomeCanvas object");
        return -1;
    }
    gtk_object_ref(GTK_OBJECT(self->obj));
    gtk_object_sink(GTK_OBJECT(self->obj));
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}
#line 182 "canvas.c"


static PyObject *
_wrap_gnome_canvas_root(PyGObject *self)
{
    GnomeCanvasGroup *ret;

    ret = gnome_canvas_root(GNOME_CANVAS(self->obj));
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_gnome_canvas_set_pixels_per_unit(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "n", NULL };
    double n;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "d:GnomeCanvas.set_pixels_per_unit", kwlist, &n))
        return NULL;
    gnome_canvas_set_pixels_per_unit(GNOME_CANVAS(self->obj), n);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_set_scroll_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x1", "y1", "x2", "y2", NULL };
    double x1, y1, x2, y2;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dddd:GnomeCanvas.set_scroll_region", kwlist, &x1, &y1, &x2, &y2))
        return NULL;
    gnome_canvas_set_scroll_region(GNOME_CANVAS(self->obj), x1, y1, x2, y2);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 237 "canvas.override"
static PyObject *
_wrap_gnome_canvas_get_scroll_region(PyGObject *self, PyObject *args)
{
    double x1, y1, x2, y2;

    gnome_canvas_get_scroll_region(GNOME_CANVAS(self->obj),
				   &x1, &y1, &x2, &y2);

    return Py_BuildValue("(dddd)", x1, y1, x2, y2);
}
#line 232 "canvas.c"


static PyObject *
_wrap_gnome_canvas_get_center_scroll_region(PyGObject *self)
{
    int ret;

    ret = gnome_canvas_get_center_scroll_region(GNOME_CANVAS(self->obj));
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_gnome_canvas_set_center_scroll_region(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "center_scroll_region", NULL };
    int center_scroll_region;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GnomeCanvas.set_center_scroll_region", kwlist, &center_scroll_region))
        return NULL;
    gnome_canvas_set_center_scroll_region(GNOME_CANVAS(self->obj), center_scroll_region);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_scroll_to(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cx", "cy", NULL };
    int cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii:GnomeCanvas.scroll_to", kwlist, &cx, &cy))
        return NULL;
    gnome_canvas_scroll_to(GNOME_CANVAS(self->obj), cx, cy);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 249 "canvas.override"
static PyObject *
_wrap_gnome_canvas_get_scroll_offsets(PyGObject *self, PyObject *args)
{
    int cx, cy;

    gnome_canvas_get_scroll_offsets(GNOME_CANVAS(self->obj), &cx, &cy);

    return Py_BuildValue("(ii)", cx, cy);
}
#line 281 "canvas.c"


static PyObject *
_wrap_gnome_canvas_update_now(PyGObject *self)
{
    gnome_canvas_update_now(GNOME_CANVAS(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_get_item_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    double x, y;
    GnomeCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:GnomeCanvas.get_item_at", kwlist, &x, &y))
        return NULL;
    ret = gnome_canvas_get_item_at(GNOME_CANVAS(self->obj), x, y);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 440 "canvas.override"
static PyObject *
_wrap_gnome_canvas_w2c_affine(PyGObject *self, PyObject *args) {
    PyObject *py_affine;
    double affine[6];

    if (!PyArg_ParseTuple(args, "O!:GnomeCanvas.w2c_affine",
			  &PyTuple_Type, &py_affine))
	return NULL;
    if ((gnomecanvasaffine_to_value(py_affine, affine)) == -1)
	return NULL;
    gnome_canvas_w2c_affine(GNOME_CANVAS(self->obj), affine);

    return gnomecanvasaffine_from_value(affine);
}
#line 321 "canvas.c"


#line 286 "canvas.override"
static PyObject *
_wrap_gnome_canvas_w2c(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "wx", "wy", NULL };
    double wx, wy;
    int cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:GnomeCanvas.w2c",
				     kwlist, &wx, &wy))
        return NULL;
    gnome_canvas_w2c(GNOME_CANVAS(self->obj), wx, wy, &cx, &cy);

    return Py_BuildValue("(ii)", cx, cy);
}
#line 339 "canvas.c"


#line 302 "canvas.override"
static PyObject *
_wrap_gnome_canvas_w2c_d(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "wx", "wy", NULL };
    double wx, wy;
    double cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:GnomeCanvas.w2c_d",
				     kwlist, &wx, &wy))
        return NULL;
    gnome_canvas_w2c_d(GNOME_CANVAS(self->obj), wx, wy, &cx, &cy);

    return Py_BuildValue("(dd)", cx, cy);
}
#line 357 "canvas.c"


#line 318 "canvas.override"
static PyObject *
_wrap_gnome_canvas_c2w(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cx", "cy", NULL };
    int cx, cy;
    double wx, wy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ii:GnomeCanvas.w2c",
				     kwlist, &cx, &cy))
        return NULL;
    gnome_canvas_c2w(GNOME_CANVAS(self->obj), cx, cy, &wx, &wy);

    return Py_BuildValue("(dd)", wx, wy);
}
#line 375 "canvas.c"


#line 334 "canvas.override"
static PyObject *
_wrap_gnome_canvas_window_to_world(PyGObject *self, PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "winx", "winy", NULL };
    double winx, winy, worldx, worldy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dd:GnomeCanvas.window_to_world",
				     kwlist, &winx, &winy))
        return NULL;
    gnome_canvas_window_to_world(GNOME_CANVAS(self->obj), winx, winy,
				 &worldx, &worldy);

    return Py_BuildValue("(dd)", worldx, worldy);
}
#line 395 "canvas.c"


#line 352 "canvas.override"
static PyObject *
_wrap_gnome_canvas_world_to_window(PyGObject *self, PyObject *args,
				   PyObject *kwargs)
{
    static char *kwlist[] = { "worldx", "worldy", NULL };
    double winx, winy, worldx, worldy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "dd:GnomeCanvas.window_to_world",
				     kwlist, &worldx, &worldy))
        return NULL;
    gnome_canvas_world_to_window(GNOME_CANVAS(self->obj), worldx, worldy,
				 &winx, &winy);

    return Py_BuildValue("(dd)", winx, winy);
}
#line 415 "canvas.c"


static PyObject *
_wrap_gnome_canvas_get_color(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "spec", "color", NULL };
    char *spec;
    PyObject *py_color;
    int ret;
    GdkColor *color = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "sO:GnomeCanvas.get_color", kwlist, &spec, &py_color))
        return NULL;
    if (pyg_boxed_check(py_color, GDK_TYPE_COLOR))
        color = pyg_boxed_get(py_color, GdkColor);
    else {
        PyErr_SetString(PyExc_TypeError, "color should be a GdkColor");
        return NULL;
    }
    ret = gnome_canvas_get_color(GNOME_CANVAS(self->obj), spec, color);
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_gnome_canvas_get_color_pixel(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "rgba", NULL };
    gulong ret;
    guint rgba;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I:GnomeCanvas.get_color_pixel", kwlist, &rgba))
        return NULL;
    ret = gnome_canvas_get_color_pixel(GNOME_CANVAS(self->obj), rgba);
    return PyLong_FromUnsignedLong(ret);
}

static PyObject *
_wrap_gnome_canvas_set_dither(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dither", NULL };
    PyObject *py_dither = NULL;
    GdkRgbDither dither;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GnomeCanvas.set_dither", kwlist, &py_dither))
        return NULL;
    if (pyg_enum_get_value(GDK_TYPE_RGB_DITHER, py_dither, (gint *)&dither))
        return NULL;
    gnome_canvas_set_dither(GNOME_CANVAS(self->obj), dither);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_get_dither(PyGObject *self)
{
    gint ret;

    ret = gnome_canvas_get_dither(GNOME_CANVAS(self->obj));
    return pyg_enum_from_gtype(GDK_TYPE_RGB_DITHER, ret);
}

static PyMethodDef _PyGnomeCanvas_methods[] = {
    { "root", (PyCFunction)_wrap_gnome_canvas_root, METH_NOARGS },
    { "set_pixels_per_unit", (PyCFunction)_wrap_gnome_canvas_set_pixels_per_unit, METH_VARARGS|METH_KEYWORDS },
    { "set_scroll_region", (PyCFunction)_wrap_gnome_canvas_set_scroll_region, METH_VARARGS|METH_KEYWORDS },
    { "get_scroll_region", (PyCFunction)_wrap_gnome_canvas_get_scroll_region, METH_NOARGS },
    { "get_center_scroll_region", (PyCFunction)_wrap_gnome_canvas_get_center_scroll_region, METH_NOARGS },
    { "set_center_scroll_region", (PyCFunction)_wrap_gnome_canvas_set_center_scroll_region, METH_VARARGS|METH_KEYWORDS },
    { "scroll_to", (PyCFunction)_wrap_gnome_canvas_scroll_to, METH_VARARGS|METH_KEYWORDS },
    { "get_scroll_offsets", (PyCFunction)_wrap_gnome_canvas_get_scroll_offsets, METH_NOARGS },
    { "update_now", (PyCFunction)_wrap_gnome_canvas_update_now, METH_NOARGS },
    { "get_item_at", (PyCFunction)_wrap_gnome_canvas_get_item_at, METH_VARARGS|METH_KEYWORDS },
    { "w2c_affine", (PyCFunction)_wrap_gnome_canvas_w2c_affine, METH_VARARGS },
    { "w2c", (PyCFunction)_wrap_gnome_canvas_w2c, METH_VARARGS|METH_KEYWORDS },
    { "w2c_d", (PyCFunction)_wrap_gnome_canvas_w2c_d, METH_VARARGS|METH_KEYWORDS },
    { "c2w", (PyCFunction)_wrap_gnome_canvas_c2w, METH_VARARGS|METH_KEYWORDS },
    { "window_to_world", (PyCFunction)_wrap_gnome_canvas_window_to_world, METH_VARARGS|METH_KEYWORDS },
    { "world_to_window", (PyCFunction)_wrap_gnome_canvas_world_to_window, METH_VARARGS|METH_KEYWORDS },
    { "get_color", (PyCFunction)_wrap_gnome_canvas_get_color, METH_VARARGS|METH_KEYWORDS },
    { "get_color_pixel", (PyCFunction)_wrap_gnome_canvas_get_color_pixel, METH_VARARGS|METH_KEYWORDS },
    { "set_dither", (PyCFunction)_wrap_gnome_canvas_set_dither, METH_VARARGS|METH_KEYWORDS },
    { "get_dither", (PyCFunction)_wrap_gnome_canvas_get_dither, METH_NOARGS },
    { NULL, NULL, 0 }
};

PyTypeObject PyGnomeCanvas_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.Canvas",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyGnomeCanvas_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_gnome_canvas_new,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasItem ----------- */

#line 140 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_set (PyGObject *self, PyObject *args,
			     PyObject *kwargs)
{
    GType              type;
    GnomeCanvasItem   *item;
    GObjectClass      *class;
    Py_ssize_t         pos = 0;
    PyObject          *value;
    PyObject          *key;

    item = GNOME_CANVAS_ITEM(self->obj);
    type = G_OBJECT_TYPE(item);
    class = G_OBJECT_GET_CLASS(item);

    g_object_freeze_notify (G_OBJECT(item));

    while (kwargs && PyDict_Next (kwargs, &pos, &key, &value)) {
	gchar *key_str = PyString_AsString (key);
	GParamSpec *pspec;
	GValue gvalue ={ 0, };

	pspec = g_object_class_find_property (class, key_str);
	if (!pspec) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "canvas item `%s' doesn't support property `%s'",
		       g_type_name(type), key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    g_object_thaw_notify (G_OBJECT(item));
	    return NULL;
	}

	g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
	if (pyg_value_from_pyobject(&gvalue, value)) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "could not convert value for property `%s'", key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    g_object_thaw_notify (G_OBJECT(item));
	    return NULL;
	}
	g_object_set_property(G_OBJECT(item), key_str, &gvalue);
	gnome_canvas_item_request_update(item);
	g_value_unset(&gvalue);
    }
    g_object_thaw_notify (G_OBJECT(item));

    Py_INCREF(Py_None);
    return Py_None;
}
#line 604 "canvas.c"


static PyObject *
_wrap_gnome_canvas_item_move(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dx", "dy", NULL };
    double dx, dy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:GnomeCanvasItem.move", kwlist, &dx, &dy))
        return NULL;
    gnome_canvas_item_move(GNOME_CANVAS_ITEM(self->obj), dx, dy);
    Py_INCREF(Py_None);
    return Py_None;
}

#line 389 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_affine_relative(PyGObject *self, PyObject *args) {
    PyObject *py_affine;
    double affine[6];

    if (!PyArg_ParseTuple(args, "O!:GnomeCanvasItem.affine_relative",
			  &PyTuple_Type, &py_affine))
	return NULL;

    if ((gnomecanvasaffine_to_value(py_affine, affine)) == -1)
	return NULL;

    gnome_canvas_item_affine_relative(GNOME_CANVAS_ITEM(self->obj), affine);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 638 "canvas.c"


#line 370 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_affine_absolute(PyGObject *self, PyObject *args) {
    PyObject *py_affine;
    double affine[6];

    if (!PyArg_ParseTuple(args, "O!:GnomeCanvasItem.affine_absolute",
			  &PyTuple_Type, &py_affine))
	return NULL;

    if ((gnomecanvasaffine_to_value(py_affine, affine)) == -1)
	return NULL;

    gnome_canvas_item_affine_absolute(GNOME_CANVAS_ITEM(self->obj), affine);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 659 "canvas.c"


static PyObject *
_wrap_gnome_canvas_item_raise(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "positions", NULL };
    int positions;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GnomeCanvasItem.raise", kwlist, &positions))
        return NULL;
    gnome_canvas_item_raise(GNOME_CANVAS_ITEM(self->obj), positions);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_item_lower(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "positions", NULL };
    int positions;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i:GnomeCanvasItem.lower", kwlist, &positions))
        return NULL;
    gnome_canvas_item_lower(GNOME_CANVAS_ITEM(self->obj), positions);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_item_raise_to_top(PyGObject *self)
{
    gnome_canvas_item_raise_to_top(GNOME_CANVAS_ITEM(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_item_lower_to_bottom(PyGObject *self)
{
    gnome_canvas_item_lower_to_bottom(GNOME_CANVAS_ITEM(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_item_show(PyGObject *self)
{
    gnome_canvas_item_show(GNOME_CANVAS_ITEM(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_gnome_canvas_item_hide(PyGObject *self)
{
    gnome_canvas_item_hide(GNOME_CANVAS_ITEM(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

#line 195 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_w2i(PyGObject *self, PyObject *args,
			    PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    double x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:GnomeCanvasItem.w2i", kwlist, &x, &y))
        return NULL;
    gnome_canvas_item_w2i(GNOME_CANVAS_ITEM(self->obj), &x, &y);

    return Py_BuildValue("(dd)", x, y);
}
#line 734 "canvas.c"


#line 210 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_i2w(PyGObject *self, PyObject *args,
			    PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    double x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "dd:GnomeCanvasItem.i2w", kwlist, &x, &y))
        return NULL;
    gnome_canvas_item_i2w(GNOME_CANVAS_ITEM(self->obj), &x, &y);

    return Py_BuildValue("(dd)", x, y);
}
#line 751 "canvas.c"


#line 424 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_i2w_affine(PyGObject *self, PyObject *args) {
    PyObject *py_affine;
    double affine[6];

    if (!PyArg_ParseTuple(args, "O!:GnomeCanvasItem.i2w_affine",
			  &PyTuple_Type, &py_affine))
	return NULL;
    if ((gnomecanvasaffine_to_value(py_affine, affine)) == -1)
	return NULL;
    gnome_canvas_item_i2w_affine(GNOME_CANVAS_ITEM(self->obj), affine);

    return gnomecanvasaffine_from_value(affine);
}
#line 769 "canvas.c"


#line 408 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_i2c_affine(PyGObject *self, PyObject *args) {
    PyObject *py_affine;
    double affine[6];

    if (!PyArg_ParseTuple(args, "O!:GnomeCanvasItem.i2c_affine",
			  &PyTuple_Type, &py_affine))
	return NULL;
    if ((gnomecanvasaffine_to_value(py_affine, affine)) == -1)
	return NULL;
    gnome_canvas_item_i2c_affine(GNOME_CANVAS_ITEM(self->obj), affine);

    return gnomecanvasaffine_from_value(affine);
}
#line 787 "canvas.c"


static PyObject *
_wrap_gnome_canvas_item_reparent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "new_group", NULL };
    PyGObject *new_group;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:GnomeCanvasItem.reparent", kwlist, &PyGnomeCanvasGroup_Type, &new_group))
        return NULL;
    gnome_canvas_item_reparent(GNOME_CANVAS_ITEM(self->obj), GNOME_CANVAS_GROUP(new_group->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

#line 581 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_grab(PyGObject *self, PyObject *args,
			     PyObject *kwargs)
{
    static char *kwlist[] = { "event_mask", "cursor", "etime", NULL };
    PyObject *py_event_mask = NULL, *py_cursor = Py_None, *py_time = NULL;
    GdkCursor *cursor = NULL;
    unsigned int event_mask = 0;
    guint32 etime = GDK_CURRENT_TIME;
    int retval;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "|OOO!:GnomeCanvasItem.grab", kwlist,
				     &py_event_mask,
				     &py_cursor,
				     &PyLong_Type, &py_time))
        return NULL;

    if (py_event_mask && pyg_flags_get_value(GDK_TYPE_EVENT_MASK,
					     py_event_mask,
					     (gint *)&event_mask))
        return NULL;

    if (pyg_boxed_check(py_cursor, GDK_TYPE_CURSOR))
        cursor = pyg_boxed_get(py_cursor, GdkCursor);
    else if (py_cursor != Py_None) {
        PyErr_SetString(PyExc_TypeError, "cursor should be a GdkCursor or None");
        return NULL;
    }

    if (py_time)
        etime = PyLong_AsUnsignedLong(py_time);

    retval = gnome_canvas_item_grab(GNOME_CANVAS_ITEM(self->obj),
				   event_mask, cursor, etime);

    return PyInt_FromLong(retval);
}
#line 842 "canvas.c"


static PyObject *
_wrap_gnome_canvas_item_grab_focus(PyGObject *self)
{
    gnome_canvas_item_grab_focus(GNOME_CANVAS_ITEM(self->obj));
    Py_INCREF(Py_None);
    return Py_None;
}

#line 225 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_get_bounds(PyGObject *self, PyObject *args)
{
    double x1, y1, x2, y2;

    gnome_canvas_item_get_bounds(GNOME_CANVAS_ITEM(self->obj),
				 &x1, &y1, &x2, &y2);

    return Py_BuildValue("(dddd)", x1, y1, x2, y2);
}
#line 864 "canvas.c"


static PyMethodDef _PyGnomeCanvasItem_methods[] = {
    { "set", (PyCFunction)_wrap_gnome_canvas_item_set, METH_VARARGS|METH_KEYWORDS },
    { "move", (PyCFunction)_wrap_gnome_canvas_item_move, METH_VARARGS|METH_KEYWORDS },
    { "affine_relative", (PyCFunction)_wrap_gnome_canvas_item_affine_relative, METH_VARARGS },
    { "affine_absolute", (PyCFunction)_wrap_gnome_canvas_item_affine_absolute, METH_VARARGS },
    { "raise_", (PyCFunction)_wrap_gnome_canvas_item_raise, METH_VARARGS|METH_KEYWORDS },
    { "lower", (PyCFunction)_wrap_gnome_canvas_item_lower, METH_VARARGS|METH_KEYWORDS },
    { "raise_to_top", (PyCFunction)_wrap_gnome_canvas_item_raise_to_top, METH_NOARGS },
    { "lower_to_bottom", (PyCFunction)_wrap_gnome_canvas_item_lower_to_bottom, METH_NOARGS },
    { "show", (PyCFunction)_wrap_gnome_canvas_item_show, METH_NOARGS },
    { "hide", (PyCFunction)_wrap_gnome_canvas_item_hide, METH_NOARGS },
    { "w2i", (PyCFunction)_wrap_gnome_canvas_item_w2i, METH_VARARGS|METH_KEYWORDS },
    { "i2w", (PyCFunction)_wrap_gnome_canvas_item_i2w, METH_VARARGS|METH_KEYWORDS },
    { "i2w_affine", (PyCFunction)_wrap_gnome_canvas_item_i2w_affine, METH_VARARGS },
    { "i2c_affine", (PyCFunction)_wrap_gnome_canvas_item_i2c_affine, METH_VARARGS },
    { "reparent", (PyCFunction)_wrap_gnome_canvas_item_reparent, METH_VARARGS|METH_KEYWORDS },
    { "grab", (PyCFunction)_wrap_gnome_canvas_item_grab, METH_VARARGS|METH_KEYWORDS },
    { "grab_focus", (PyCFunction)_wrap_gnome_canvas_item_grab_focus, METH_NOARGS },
    { "get_bounds", (PyCFunction)_wrap_gnome_canvas_item_get_bounds, METH_NOARGS },
    { NULL, NULL, 0 }
};

PyTypeObject PyGnomeCanvasItem_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasItem",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyGnomeCanvasItem_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasGroup ----------- */

#line 73 "canvas.override"
static PyObject *
_wrap_gnome_canvas_item_new (PyGObject *self, PyObject *args,
			     PyObject *kwargs)

{
    PyObject          *pytype;
    GType              type;
    GnomeCanvasItem   *item;
    GObjectClass      *class;
    Py_ssize_t         pos;
    PyObject          *value;
    PyObject          *key;

    if (!PyArg_ParseTuple (args, "O:gnome.canvas.CanvasGroup.add_item",
			   &pytype)) {
	return NULL;
    }

    type = pyg_type_from_object (pytype);
    item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (self->obj), type, NULL);
    if (!item) {
	PyErr_SetString (PyExc_RuntimeError,
			 "could not create canvas item object");
	return NULL;
    }

    class = G_OBJECT_GET_CLASS(item);
    g_object_freeze_notify (G_OBJECT(item));
    pos = 0;
    /* For each keyword ... */
    while (kwargs && PyDict_Next (kwargs, &pos, &key, &value)) {
	gchar *key_str = PyString_AsString (key);
	GParamSpec *pspec;
	GValue gvalue ={ 0, };

	pspec = g_object_class_find_property (class, key_str);
	if (!pspec) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "canvas item `%s' doesn't support property `%s'",
		       g_type_name(type), key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    gtk_object_destroy(GTK_OBJECT(item));
	    return NULL;
	}

	g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
	if (pyg_value_from_pyobject(&gvalue, value)) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "could not convert value for property `%s'", key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    gtk_object_destroy(GTK_OBJECT(item));
	    return NULL;
	}
	g_object_set_property(G_OBJECT(item), key_str, &gvalue);
	g_value_unset(&gvalue);
    }

    g_object_thaw_notify (G_OBJECT(item));

    return pygobject_new ((GObject *)item);
}
#line 1004 "canvas.c"


static PyMethodDef _PyGnomeCanvasGroup_methods[] = {
    { "add", (PyCFunction)_wrap_gnome_canvas_item_new, METH_VARARGS|METH_KEYWORDS },
    { NULL, NULL, 0 }
};

#line 456 "canvas.override"
static PyObject *
_wrap_gnome_canvas_group__get_item_list(PyGObject *self, void *closure)
{
    GnomeCanvasGroup *parent = GNOME_CANVAS_GROUP(self->obj);
    PyObject         *list, *item;
    GList            *l;

    list = PyList_New(0);
    for (l = parent->item_list; l != NULL; l = l->next) {
	item = pygobject_new(G_OBJECT(l->data));
	PyList_Append(list, item);
	Py_DECREF(item);
    }
    return list;
}

#line 1029 "canvas.c"


static PyGetSetDef gnome_canvas_group_getsets[] = {
    { "item_list", (getter)_wrap_gnome_canvas_group__get_item_list, (setter)0 },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject PyGnomeCanvasGroup_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasGroup",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyGnomeCanvasGroup_methods,			/* tp_methods */
    0,					/* tp_members */
    gnome_canvas_group_getsets,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasClipgroup ----------- */

PyTypeObject PyGnomeCanvasClipgroup_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasClipgroup",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasLine ----------- */

PyTypeObject PyGnomeCanvasLine_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasLine",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasPixbuf ----------- */

PyTypeObject PyGnomeCanvasPixbuf_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasPixbuf",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasRichText ----------- */

PyTypeObject PyGnomeCanvasRichText_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasRichText",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasShape ----------- */

PyTypeObject PyGnomeCanvasShape_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasShape",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasRE ----------- */

PyTypeObject PyGnomeCanvasRE_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasRE",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasRect ----------- */

PyTypeObject PyGnomeCanvasRect_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasRect",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasEllipse ----------- */

PyTypeObject PyGnomeCanvasEllipse_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasEllipse",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasPolygon ----------- */

PyTypeObject PyGnomeCanvasPolygon_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasPolygon",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasBpath ----------- */

#line 560 "canvas.override"
static PyObject *
_wrap_gnome_canvas_bpath_set_bpath(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bpath", NULL };
    PyObject *py_bpath;
    GnomeCanvasPathDef *bpath = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:GnomeCanvasBpath.set_bpath", kwlist, &py_bpath))
        return NULL;
    if (pyg_boxed_check(py_bpath, GNOME_TYPE_CANVAS_PATH_DEF))
        bpath = pyg_boxed_get(py_bpath, GnomeCanvasPathDef);
    else {
        PyErr_SetString(PyExc_TypeError, "bpath should be a GnomeCanvasPathDef");
        return NULL;
    }
    g_object_set(self->obj, "bpath", bpath, NULL);
    Py_INCREF(Py_None);
    return Py_None;
}
#line 1547 "canvas.c"


static PyMethodDef _PyGnomeCanvasBpath_methods[] = {
    { "set_bpath", (PyCFunction)_wrap_gnome_canvas_bpath_set_bpath, METH_VARARGS|METH_KEYWORDS },
    { NULL, NULL, 0 }
};

PyTypeObject PyGnomeCanvasBpath_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasBpath",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyGnomeCanvasBpath_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasText ----------- */

PyTypeObject PyGnomeCanvasText_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasText",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GnomeCanvasWidget ----------- */

PyTypeObject PyGnomeCanvasWidget_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "gnome.canvas.CanvasWidget",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    NULL,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

#line 474 "canvas.override"
static PyObject *
_wrap_gnome_canvas_path_def_new(PyObject *self, PyObject *args)
{
    GnomeCanvasPathDef *path;
    PyObject *list, *arg;
    int i, len, arg_len, code;
    double x1, y1, x2, y2, x3, y3;


    if (!PyArg_ParseTuple(args, "O!:gnome.canvas.path_def_new",
			  &PyList_Type, &list))
	return NULL;

    len = PyList_Size(list);
    path = gnome_canvas_path_def_new();
    for (i = 0; i < len; ++i) {
	arg = PyList_GET_ITEM(list, i);

	if (!PyTuple_Check(arg)) {
	    PyErr_SetString(PyExc_TypeError,
			    "list items must be (code, x1, x2, ...) tuples");
	    gnome_canvas_path_def_unref(path);
	    return NULL;
	}
	arg_len = PyTuple_Size(arg);
	if (arg_len < 1) {
	    PyErr_SetString(PyExc_ValueError,
			    "list items must be (code, x1, x2, ...) tuples");
	    gnome_canvas_path_def_unref(path);
	    return NULL;
	}
	if (!PyInt_Check(PyTuple_GET_ITEM(arg, 0))) {
	    PyErr_SetString(PyExc_TypeError,
			    "list items must be (code, x1, x2, ...) tuples");
	    gnome_canvas_path_def_unref(path);
	    return NULL;
	}
	code = PyInt_AsLong(PyTuple_GET_ITEM(arg, 0));

	switch (code)
	{

	case ART_MOVETO:
	case ART_MOVETO_OPEN:
	    if (!PyArg_ParseTuple(arg, "idd:gnome.canvas.path_def_new (list item)",
				  &code, &x1, &y1)) {
		gnome_canvas_path_def_unref(path);
		return NULL;
	    }
	    gnome_canvas_path_def_moveto(path, x1, y1);
	    break;

	case ART_LINETO:
	    if (!PyArg_ParseTuple(arg, "idd:gnome.canvas.path_def_new (list item)",
				  &code, &x1, &y1)) {
		gnome_canvas_path_def_unref(path);
		return NULL;
	    }
	    gnome_canvas_path_def_lineto(path, x1, y1);
	    break;

	case ART_CURVETO:
	    if (!PyArg_ParseTuple(arg, "idddddd:gnome.canvas.path_def_new (list item)",
				  &code,
				  &x1, &y1,
				  &x2, &y2,
				  &x3, &y3)) {
		gnome_canvas_path_def_unref(path);
		return NULL;
	    }
	    gnome_canvas_path_def_curveto(path, x1, y1, x2, y2, x3, y3);
	    break;

	case ART_END:
	default:
	    gnome_canvas_path_def_unref(path);
	    return PyErr_Format(PyExc_ValueError, "invalid path code %i",
				code);
	}

    }
    return pyg_boxed_new(GNOME_TYPE_CANVAS_PATH_DEF, path, TRUE, TRUE);
}

#line 1787 "canvas.c"


PyMethodDef pycanvas_functions[] = {
    { "path_def_new", (PyCFunction)_wrap_gnome_canvas_path_def_new, METH_VARARGS },
    { NULL, NULL, 0 }
};

/* initialise stuff extension classes */
void
pycanvas_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gobject")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGObject_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "GObject");
        if (_PyGObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name GObject from gobject");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return;
    }
    if ((module = PyImport_ImportModule("gtk._gtk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGtkLayout_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Layout");
        if (_PyGtkLayout_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Layout from gtk._gtk");
            return;
        }
        _PyGtkObject_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Object");
        if (_PyGtkObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Object from gtk._gtk");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk._gtk");
        return;
    }


#line 1837 "canvas.c"
    pyg_register_boxed(d, "CanvasPathDef", GNOME_TYPE_CANVAS_PATH_DEF, &PyGnomeCanvasPathDef_Type);
    pygobject_register_class(d, "GnomeCanvas", GNOME_TYPE_CANVAS, &PyGnomeCanvas_Type, Py_BuildValue("(O)", &PyGtkLayout_Type));
    pygobject_register_class(d, "GnomeCanvasItem", GNOME_TYPE_CANVAS_ITEM, &PyGnomeCanvasItem_Type, Py_BuildValue("(O)", &PyGtkObject_Type));
    pygobject_register_class(d, "GnomeCanvasGroup", GNOME_TYPE_CANVAS_GROUP, &PyGnomeCanvasGroup_Type, Py_BuildValue("(O)", &PyGnomeCanvasItem_Type));
    pygobject_register_class(d, "GnomeCanvasClipgroup", GNOME_TYPE_CANVAS_CLIPGROUP, &PyGnomeCanvasClipgroup_Type, Py_BuildValue("(O)", &PyGnomeCanvasGroup_Type));
    pygobject_register_class(d, "GnomeCanvasLine", GNOME_TYPE_CANVAS_LINE, &PyGnomeCanvasLine_Type, Py_BuildValue("(O)", &PyGnomeCanvasItem_Type));
    pygobject_register_class(d, "GnomeCanvasPixbuf", GNOME_TYPE_CANVAS_PIXBUF, &PyGnomeCanvasPixbuf_Type, Py_BuildValue("(O)", &PyGnomeCanvasItem_Type));
    pygobject_register_class(d, "GnomeCanvasRichText", GNOME_TYPE_CANVAS_RICH_TEXT, &PyGnomeCanvasRichText_Type, Py_BuildValue("(O)", &PyGnomeCanvasItem_Type));
    pygobject_register_class(d, "GnomeCanvasShape", GNOME_TYPE_CANVAS_SHAPE, &PyGnomeCanvasShape_Type, Py_BuildValue("(O)", &PyGnomeCanvasItem_Type));
    pygobject_register_class(d, "GnomeCanvasRE", GNOME_TYPE_CANVAS_RE, &PyGnomeCanvasRE_Type, Py_BuildValue("(O)", &PyGnomeCanvasShape_Type));
    pygobject_register_class(d, "GnomeCanvasRect", GNOME_TYPE_CANVAS_RECT, &PyGnomeCanvasRect_Type, Py_BuildValue("(O)", &PyGnomeCanvasRE_Type));
    pygobject_register_class(d, "GnomeCanvasEllipse", GNOME_TYPE_CANVAS_ELLIPSE, &PyGnomeCanvasEllipse_Type, Py_BuildValue("(O)", &PyGnomeCanvasRE_Type));
    pygobject_register_class(d, "GnomeCanvasPolygon", GNOME_TYPE_CANVAS_POLYGON, &PyGnomeCanvasPolygon_Type, Py_BuildValue("(O)", &PyGnomeCanvasShape_Type));
    pygobject_register_class(d, "GnomeCanvasBpath", GNOME_TYPE_CANVAS_BPATH, &PyGnomeCanvasBpath_Type, Py_BuildValue("(O)", &PyGnomeCanvasShape_Type));
    pygobject_register_class(d, "GnomeCanvasText", GNOME_TYPE_CANVAS_TEXT, &PyGnomeCanvasText_Type, Py_BuildValue("(O)", &PyGnomeCanvasItem_Type));
    pygobject_register_class(d, "GnomeCanvasWidget", GNOME_TYPE_CANVAS_WIDGET, &PyGnomeCanvasWidget_Type, Py_BuildValue("(O)", &PyGnomeCanvasItem_Type));
}
