/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 4 "goocanvas.override"
#define NO_IMPORT_PYGOBJECT
#include "pygobject.h"
#include <goocanvas.h>
#include <pygtk/pygtk.h>

#include <pycairo.h>

#include "config.h"

#if PY_VERSION_HEX < 0x02050000
typedef int Py_ssize_t;
#endif

extern Pycairo_CAPI_t *Pycairo_CAPI;
extern PyTypeObject PyGooCanvasBounds_Type;

static PyObject *
_glist_to_pylist_objs (GList *source)
{
    GList *iter;
    PyObject *dest = PyList_New (0);
    for (iter = source; iter != NULL; iter = iter->next)
    {
        PyObject *item = pygobject_new ((GObject *)iter->data);
        PyList_Append (dest, item);
        Py_DECREF (item);
    }
    return dest;
}

#if 0

static PyObject *
_gslist_to_pylist_objs (GSList *source)
{
    GSList *iter;
    PyObject *dest = PyList_New (0);
    for (iter = source; iter != NULL; iter = iter->next)
    {
        PyObject *item = pygobject_new ((GObject *)iter->data);
        PyList_Append (dest, item);
        Py_DECREF (item);
    }
    return dest;
}

static PyObject *
_gslist_to_pylist_strs (GSList *source)
{
    GSList *iter;
    PyObject *dest = PyList_New (0);
    for (iter = source; iter != NULL; iter = iter->next)
    {
        PyObject *item = PyString_FromString ((char *)iter->data);
        PyList_Append (dest, item);
        Py_DECREF (item);
    }
    return dest;
}

#endif

typedef struct {
    PyObject_HEAD
    GooCanvasBounds bounds;
} PyGooCanvasBounds;

#define bounds_getter(field)                                                    \
static PyObject *                                                               \
_pygoo_canvas_bounds_get_##field(PyGooCanvasBounds *self, void *closure)        \
{                                                                               \
    return PyFloat_FromDouble(self->bounds.field);                              \
}

#define bounds_setter(field)                                                                    \
static int                                                                                      \
_pygoo_canvas_bounds_set_##field(PyGooCanvasBounds *self, PyObject *value, void *closure)       \
{                                                                                               \
    if (!PyFloat_Check(value)) {                                                                \
        PyErr_SetString(PyExc_TypeError, #field" must be a float");                             \
        return -1;                                                                              \
    }                                                                                           \
    self->bounds.field = PyFloat_AsDouble(value);                                               \
    return 0;                                                                                   \
}

bounds_getter(x1);
bounds_setter(x1);
bounds_getter(y1);
bounds_setter(y1);
bounds_getter(x2);
bounds_setter(x2);
bounds_getter(y2);
bounds_setter(y2);


static PyGetSetDef _PyGooCanvasBounds_getsets[] = {
    { "x1", (getter)_pygoo_canvas_bounds_get_x1, (setter)_pygoo_canvas_bounds_set_x1 },
    { "y1", (getter)_pygoo_canvas_bounds_get_y1, (setter)_pygoo_canvas_bounds_set_y1 },
    { "x2", (getter)_pygoo_canvas_bounds_get_x2, (setter)_pygoo_canvas_bounds_set_x2 },
    { "y2", (getter)_pygoo_canvas_bounds_get_y2, (setter)_pygoo_canvas_bounds_set_y2 },
    { NULL, (getter)0, (setter)0 }
};

static PyObject *
pygoo_canvas_bounds_tp_new(PyTypeObject *type, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x1", "y1", "x2", "y2", NULL };
    PyGooCanvasBounds *self;
    GooCanvasBounds bounds = {0,};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "|dddd:goocanvas.Bounds.__init__", kwlist,
                                     &bounds.x1, &bounds.y1, &bounds.x2, &bounds.y2))
        return NULL;

    self = (PyGooCanvasBounds *) PyObject_NEW(PyGooCanvasBounds,
                                              &PyGooCanvasBounds_Type);
    self->bounds = bounds;
    return (PyObject *) self;
}

static void
pygoo_canvas_bounds_dealloc(PyGooCanvasBounds *self)
{
    PyObject_DEL(self);
}

static PyMethodDef _PyGooCanvasBounds_methods[] = {
    { NULL,  0, 0 }
};

PyTypeObject PyGooCanvasBounds_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                  /* ob_size */
    "goocanvas.Bounds",                 /* tp_name */
    sizeof(PyGooCanvasBounds),		/* tp_basicsize */
    0,                                  /* tp_itemsize */
    /* methods */
    (destructor)pygoo_canvas_bounds_dealloc, /* tp_dealloc */
    (printfunc)0,                       /* tp_print */
    (getattrfunc)0,                     /* tp_getattr */
    (setattrfunc)0,                     /* tp_setattr */
    (cmpfunc)0,                         /* tp_compare */
    (reprfunc)0,                        /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    (hashfunc)0,                        /* tp_hash */
    (ternaryfunc)0,                     /* tp_call */
    (reprfunc)0,                        /* tp_str */
    (getattrofunc)0,                    /* tp_getattro */
    (setattrofunc)0,                    /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,   		/* tp_flags */
    "Holds bounding box coordinates",   /* Documentation string */
    (traverseproc)0,                    /* tp_traverse */
    (inquiry)0,                         /* tp_clear */
    (richcmpfunc)0,                     /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    (getiterfunc)0,                     /* tp_iter */
    (iternextfunc)0,                    /* tp_iternext */
    _PyGooCanvasBounds_methods,         /* tp_methods */
    0,                                  /* tp_members */
    _PyGooCanvasBounds_getsets,         /* tp_getset */
    (PyTypeObject *)0,                  /* tp_base */
    (PyObject *)0,                      /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)0,                        /* tp_init */
    0,                			/* tp_alloc */
    pygoo_canvas_bounds_tp_new,         /* tp_new */
    0,                                  /* tp_free */
    (inquiry)0,                         /* tp_is_gc */
    (PyObject *)0,                      /* tp_bases */
};

PyObject*
pygoo_canvas_bounds_new(const GooCanvasBounds *bounds)
{
    PyGooCanvasBounds *self;

    self = (PyGooCanvasBounds *)PyObject_NEW(PyGooCanvasBounds,
                                             &PyGooCanvasBounds_Type);
    if (G_UNLIKELY(self == NULL))
	return NULL;
    if (bounds)
        self->bounds = *bounds;
    return (PyObject *)self;
}

#line 201 "goocanvas.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGObject_Type;
#define PyGObject_Type (*_PyGObject_Type)
static PyTypeObject *_PyGtkContainer_Type;
#define PyGtkContainer_Type (*_PyGtkContainer_Type)
static PyTypeObject *_PyGtkAdjustment_Type;
#define PyGtkAdjustment_Type (*_PyGtkAdjustment_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject G_GNUC_INTERNAL PyGooCanvasPoints_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasLineDash_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvas_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasItemModelSimple_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasImageModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasGroupModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasEllipseModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasItemSimple_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasImage_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasSvg_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasGroup_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasEllipse_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasPath_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasPathModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasPolyline_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasPolylineModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasRect_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasRectModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasStyle_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasTable_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasTableModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasText_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasTextModel_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasWidget_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasItem_Type;
PyTypeObject G_GNUC_INTERNAL PyGooCanvasItemModel_Type;


#line 664 "goocanvas.override"

static PyObject *
_py_canvas_style_get_property(GooCanvasStyle *style,
                              const char *name)
{
    const GValue *value;

    value = goo_canvas_style_get_property(style, g_quark_from_string(name));
    if (!value) {
        PyErr_Format(PyExc_KeyError, "goocanvas.Style object has no property named '%s'",
                     name);
        return NULL;
    }
    return pyg_value_as_pyobject(value, TRUE);
}

static int
_py_goo_canvas_style_set_property(GooCanvasStyle *style,
                                  const char *name,
                                  PyObject *py_value)
{
    GValue value = {0,};

    if (py_value == Py_None) {
        goo_canvas_style_set_property(style, g_quark_from_string(name), NULL);
    } else {
        GType gtype = pyg_type_from_object((PyObject *) py_value->ob_type);
        if (gtype == 0) {
              /* check to see if it is one of our specially supported types */
            if (PyObject_IsInstance(py_value, (PyObject *) &PycairoMatrix_Type)) {
                PyErr_Clear();
                gtype = GOO_TYPE_CAIRO_MATRIX;
            } else if (PyObject_IsInstance(py_value, (PyObject *) &PycairoPattern_Type)) {
                PyErr_Clear();
                gtype = GOO_TYPE_CAIRO_PATTERN;
            } else /* give up; we really don't support this type */
                return -1;
        }
        g_value_init(&value, gtype);
        if (pyg_value_from_pyobject(&value, py_value)) {
            PyErr_SetString(PyExc_TypeError, "unable to convert value");
            return -1;
        }
        goo_canvas_style_set_property(style, g_quark_from_string(name), &value);
        g_value_unset(&value);
    }
    return 0;
}


#line 293 "goocanvas.c"



/* ----------- GooCanvasPoints ----------- */

#line 280 "goocanvas.override"
static int
_wrap_goo_canvas_points_new(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "data", NULL };
    PyObject *py_data;
    gint i, n_data;
    GooCanvasPoints *points;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                "O!:GooCanvasPoints.__init__",
                                kwlist, &PyList_Type, &py_data))
        return -1;
    n_data = PyList_Size(py_data);
    points = goo_canvas_points_new(n_data);
    for (i = 0; i < n_data; i ++) {
        PyObject *item = PyList_GetItem(py_data, i);
        if (!PyArg_ParseTuple(item, "dd", &points->coords[2*i], &points->coords[2*i + 1])) {
            goo_canvas_points_unref(points);
            return -1;
        }
    }
    self->boxed = points;
    self->free_on_dealloc = TRUE;
    self->gtype = GOO_TYPE_CANVAS_POINTS;
    return 0;
}
#line 326 "goocanvas.c"


PyTypeObject G_GNUC_INTERNAL PyGooCanvasPoints_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Points",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_goo_canvas_points_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasLineDash ----------- */

#line 445 "goocanvas.override"
static int
_wrap_goo_canvas_line_dash_newv(PyGBoxed *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "dashes", NULL };
    int num_dashes, i;
    double *dashes;
    PyObject *py_dashes;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasLineDash.__init__", kwlist,
                                     &PyList_Type, &py_dashes))
        return -1;

    num_dashes = PyList_Size(py_dashes);
    dashes = g_new(double, num_dashes);
    for (i = 0; i < num_dashes; ++i) {
        PyObject *item = PyList_GET_ITEM(py_dashes, i);
        if (!PyFloat_Check(item)) {
            g_free(dashes);
            PyErr_SetString(PyExc_TypeError, "parameter must be a list of floats");
            return -1;
        }
        dashes[i] = PyFloat_AsDouble(item);
    }

    self->gtype = GOO_TYPE_CANVAS_LINE_DASH;
    self->free_on_dealloc = FALSE;
    self->boxed = goo_canvas_line_dash_newv(num_dashes, dashes);

    if (!self->boxed) {
        g_free(dashes);
        PyErr_SetString(PyExc_RuntimeError, "could not create GooCanvasLineDash object");
        return -1;
    }
    self->free_on_dealloc = TRUE;
    return 0;
}
#line 415 "goocanvas.c"


PyTypeObject G_GNUC_INTERNAL PyGooCanvasLineDash_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.LineDash",                   /* tp_name */
    sizeof(PyGBoxed),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)_wrap_goo_canvas_line_dash_newv,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvas ----------- */

static int
_wrap_goo_canvas_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char* kwlist[] = { NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     ":goocanvas.Canvas.__init__",
                                     kwlist))
        return -1;

    pygobject_constructv(self, 0, NULL);
    if (!self->obj) {
        PyErr_SetString(
            PyExc_RuntimeError, 
            "could not create goocanvas.Canvas object");
        return -1;
    }
    return 0;
}

static PyObject *
_wrap_goo_canvas_get_root_item(PyGObject *self)
{
    GooCanvasItem *ret;

    
    ret = goo_canvas_get_root_item(GOO_CANVAS(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_set_root_item(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", NULL };
    PyGObject *item;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.set_root_item", kwlist, &PyGooCanvasItem_Type, &item))
        return NULL;
    
    goo_canvas_set_root_item(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_get_root_item_model(PyGObject *self)
{
    GooCanvasItemModel *ret;

    
    ret = goo_canvas_get_root_item_model(GOO_CANVAS(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_set_root_item_model(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "model", NULL };
    PyGObject *model;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.set_root_item_model", kwlist, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    
    goo_canvas_set_root_item_model(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_get_item(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "model", NULL };
    PyGObject *model;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.get_item", kwlist, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    
    ret = goo_canvas_get_item(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_get_item_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "is_pointer_event", NULL };
    int is_pointer_event;
    double x, y;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddi:GooCanvas.get_item_at", kwlist, &x, &y, &is_pointer_event))
        return NULL;
    
    ret = goo_canvas_get_item_at(GOO_CANVAS(self->obj), x, y, is_pointer_event);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

#line 1437 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_get_items_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char     *kwlist[] = { "x", "y", "is_pointer_event", NULL };
    PyObject        *point_event, *ret;
    gdouble         x, y;
    GList           *item_list;
    gboolean        is_pointer_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "ddO:get_items_at",
                                     kwlist,
                                     &x, &y, &point_event))
        return NULL;

    is_pointer_event = (PyObject_IsTrue(point_event)) ? TRUE : FALSE;

    item_list = goo_canvas_get_items_at(GOO_CANVAS(self->obj),
                                        x, y, is_pointer_event);

    if (!item_list) {
        Py_INCREF (Py_None);
        return Py_None;
    }

    ret = _glist_to_pylist_objs(item_list);
    g_list_free(item_list);
    return ret;
}

#line 605 "goocanvas.c"


#line 1469 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_get_items_in_area(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char     *kwlist[] = { "area", "inside_area", "include_overlaps",
                                  "include_containers",  NULL };
    PyObject        *inside_area, *include_containers, *include_overlaps, *ret;
    PyObject        *area;
    GList           *item_list;
    gboolean        inside, include_cont, include_over;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "O!OOO:get_items_at",
                                     kwlist,
                                     &PyGooCanvasBounds_Type, &area, &inside_area,
                                     &include_overlaps, &include_containers))
        return NULL;

    inside = (PyObject_IsTrue(inside_area)) ? TRUE : FALSE;
    include_cont = (PyObject_IsTrue(include_containers)) ? TRUE : FALSE;
    include_over = (PyObject_IsTrue(include_overlaps)) ? TRUE : FALSE;

    item_list = goo_canvas_get_items_in_area (GOO_CANVAS(self->obj),
                                              &((PyGooCanvasBounds *) area)->bounds,
                                              inside,
                                              include_over,
                                              include_cont);
    if (!item_list) {
        Py_INCREF (Py_None);
        return Py_None;
    }


    ret = _glist_to_pylist_objs(item_list);
    g_list_free(item_list);
    return ret;
}

#line 646 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_get_scale(PyGObject *self)
{
    double ret;

    
    ret = goo_canvas_get_scale(GOO_CANVAS(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_goo_canvas_set_scale(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "pixels_per_unit", NULL };
    double pixels_per_unit;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"d:GooCanvas.set_scale", kwlist, &pixels_per_unit))
        return NULL;
    
    goo_canvas_set_scale(GOO_CANVAS(self->obj), pixels_per_unit);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 513 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_get_bounds(PyGObject *self)
{
    gdouble left, right, top, bottom;

    goo_canvas_get_bounds(GOO_CANVAS(self->obj), &left, &right, &top, &bottom);

    return Py_BuildValue("dddd", left, right, top, bottom);
}

#line 686 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_set_bounds(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "left", "top", "right", "bottom", NULL };
    double left, top, right, bottom;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dddd:GooCanvas.set_bounds", kwlist, &left, &top, &right, &bottom))
        return NULL;
    
    goo_canvas_set_bounds(GOO_CANVAS(self->obj), left, top, right, bottom);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_scroll_to(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "left", "top", NULL };
    double left, top;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:GooCanvas.scroll_to", kwlist, &left, &top))
        return NULL;
    
    goo_canvas_scroll_to(GOO_CANVAS(self->obj), left, top);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_grab_focus(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", NULL };
    PyGObject *item;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.grab_focus", kwlist, &PyGooCanvasItem_Type, &item))
        return NULL;
    
    goo_canvas_grab_focus(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_render(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "bounds", "scale", NULL };
    PyObject *py_bounds = NULL;
    double scale = 1.0;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|Od:GooCanvas.render", kwlist, &PycairoContext_Type, &cr, &py_bounds, &scale))
        return NULL;
    if (!(py_bounds == NULL || py_bounds == Py_None || 
        PyObject_IsInstance(py_bounds, (PyObject *) &PyGooCanvasBounds_Type))) {
        PyErr_SetString(PyExc_TypeError, "parameter bounds must be goocanvas.Bounds or None");
        return NULL;
    }
    
    goo_canvas_render(GOO_CANVAS(self->obj), cr->ctx, (py_bounds == NULL || py_bounds == Py_None)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds, scale);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 540 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_convert_to_pixels(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    gdouble x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:GooCanvas.convert_to_pixels", kwlist, &x, &y))
        return NULL;

    goo_canvas_convert_to_pixels(GOO_CANVAS(self->obj), &x, &y);

    return Py_BuildValue("dd", x, y);
}

#line 771 "goocanvas.c"


#line 556 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_convert_from_pixels(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", NULL };
    gdouble x, y;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:GooCanvas.convert_from_pixels", kwlist, &x, &y))
        return NULL;

    goo_canvas_convert_from_pixels(GOO_CANVAS(self->obj), &x, &y);

    return Py_BuildValue("dd", x, y);
}

#line 789 "goocanvas.c"


#line 572 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_convert_to_item_space(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", "x", "y", NULL };
    gdouble x, y;
    PyGObject *item;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!dd:GooCanvas.convert_to_item_space",
                                     kwlist, &PyGooCanvasItem_Type, &item, &x, &y))
        return NULL;

    goo_canvas_convert_to_item_space(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj), &x, &y);

    return Py_BuildValue("dd", x, y);
}
#line 808 "goocanvas.c"


#line 589 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_convert_from_item_space(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", "x", "y", NULL };
    gdouble x, y;
    PyGObject *item;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!dd:GooCanvas.convert_from_item_space",
                                     kwlist, &PyGooCanvasItem_Type, &item, &x, &y))
        return NULL;

    goo_canvas_convert_from_item_space(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj), &x, &y);

    return Py_BuildValue("dd", x, y);
}
#line 827 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_pointer_grab(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", "event_mask", "cursor", "time", NULL };
    PyObject *py_event_mask = NULL, *py_cursor = Py_None;
    unsigned long time;
    GdkCursor *cursor = NULL;
    PyGObject *item;
    GdkEventMask event_mask;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!OOk:GooCanvas.pointer_grab", kwlist, &PyGooCanvasItem_Type, &item, &py_event_mask, &py_cursor, &time))
        return NULL;
    if (pyg_flags_get_value(GDK_TYPE_EVENT_MASK, py_event_mask, (gpointer)&event_mask))
        return NULL;
    if (pyg_boxed_check(py_cursor, GDK_TYPE_CURSOR))
        cursor = pyg_boxed_get(py_cursor, GdkCursor);
    else if (py_cursor != Py_None) {
        PyErr_SetString(PyExc_TypeError, "cursor should be a GdkCursor or None");
        return NULL;
    }
    
    ret = goo_canvas_pointer_grab(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj), event_mask, cursor, time);
    
    return pyg_enum_from_gtype(GDK_TYPE_GRAB_STATUS, ret);
}

static PyObject *
_wrap_goo_canvas_pointer_ungrab(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", "time", NULL };
    PyGObject *item;
    unsigned long time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:GooCanvas.pointer_ungrab", kwlist, &PyGooCanvasItem_Type, &item, &time))
        return NULL;
    
    goo_canvas_pointer_ungrab(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj), time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_keyboard_grab(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", "owner_events", "time", NULL };
    PyGObject *item;
    int owner_events;
    unsigned long time;
    gint ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ik:GooCanvas.keyboard_grab", kwlist, &PyGooCanvasItem_Type, &item, &owner_events, &time))
        return NULL;
    
    ret = goo_canvas_keyboard_grab(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj), owner_events, time);
    
    return pyg_enum_from_gtype(GDK_TYPE_GRAB_STATUS, ret);
}

static PyObject *
_wrap_goo_canvas_keyboard_ungrab(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "item", "time", NULL };
    PyGObject *item;
    unsigned long time;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!k:GooCanvas.keyboard_ungrab", kwlist, &PyGooCanvasItem_Type, &item, &time))
        return NULL;
    
    goo_canvas_keyboard_ungrab(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj), time);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_create_cairo_context(PyGObject *self)
{
    cairo_t *ret;

    
    ret = goo_canvas_create_cairo_context(GOO_CANVAS(self->obj));
    
    cairo_reference(ret);
    return PycairoContext_FromContext(ret, NULL, NULL);
}

static PyObject *
_wrap_goo_canvas_create_item(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "model", NULL };
    PyGObject *model;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.create_item", kwlist, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    
    ret = goo_canvas_create_item(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_unregister_item(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "model", NULL };
    PyGObject *model;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.unregister_item", kwlist, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    
    goo_canvas_unregister_item(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_update(PyGObject *self)
{
    
    goo_canvas_update(GOO_CANVAS(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_request_update(PyGObject *self)
{
    
    goo_canvas_request_update(GOO_CANVAS(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_request_redraw(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "bounds", NULL };
    PyObject *py_bounds;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.request_redraw", kwlist, &PyGooCanvasBounds_Type, &py_bounds))
        return NULL;
    
    goo_canvas_request_redraw(GOO_CANVAS(self->obj), (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_get_default_line_width(PyGObject *self)
{
    double ret;

    
    ret = goo_canvas_get_default_line_width(GOO_CANVAS(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_goo_canvas_register_widget_item(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "witem", NULL };
    PyGObject *witem;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.register_widget_item", kwlist, &PyGooCanvasWidget_Type, &witem))
        return NULL;
    
    goo_canvas_register_widget_item(GOO_CANVAS(self->obj), GOO_CANVAS_WIDGET(witem->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_unregister_widget_item(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "witem", NULL };
    PyGObject *witem;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvas.unregister_widget_item", kwlist, &PyGooCanvasWidget_Type, &witem))
        return NULL;
    
    goo_canvas_unregister_widget_item(GOO_CANVAS(self->obj), GOO_CANVAS_WIDGET(witem->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvas__do_set_scroll_adjustments(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "hadjustment", "vadjustment", NULL };
    PyGObject *self, *hadjustment, *vadjustment;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!:GooCanvas.set_scroll_adjustments", kwlist, &PyGooCanvas_Type, &self, &PyGtkAdjustment_Type, &hadjustment, &PyGtkAdjustment_Type, &vadjustment))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GOO_CANVAS_CLASS(klass)->set_scroll_adjustments)
        GOO_CANVAS_CLASS(klass)->set_scroll_adjustments(GOO_CANVAS(self->obj), GTK_ADJUSTMENT(hadjustment->obj), GTK_ADJUSTMENT(vadjustment->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GooCanvas.set_scroll_adjustments not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvas__do_create_item(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "model", NULL };
    PyGObject *self, *model;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvas.create_item", kwlist, &PyGooCanvas_Type, &self, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GOO_CANVAS_CLASS(klass)->create_item)
        ret = GOO_CANVAS_CLASS(klass)->create_item(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GooCanvas.create_item not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvas__do_item_created(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "item", "model", NULL };
    PyGObject *self, *item, *model;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!:GooCanvas.item_created", kwlist, &PyGooCanvas_Type, &self, &PyGooCanvasItem_Type, &item, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GOO_CANVAS_CLASS(klass)->item_created)
        GOO_CANVAS_CLASS(klass)->item_created(GOO_CANVAS(self->obj), GOO_CANVAS_ITEM(item->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GooCanvas.item_created not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGooCanvas_methods[] = {
    { "get_root_item", (PyCFunction)_wrap_goo_canvas_get_root_item, METH_NOARGS,
      NULL },
    { "set_root_item", (PyCFunction)_wrap_goo_canvas_set_root_item, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_root_item_model", (PyCFunction)_wrap_goo_canvas_get_root_item_model, METH_NOARGS,
      NULL },
    { "set_root_item_model", (PyCFunction)_wrap_goo_canvas_set_root_item_model, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_item", (PyCFunction)_wrap_goo_canvas_get_item, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_item_at", (PyCFunction)_wrap_goo_canvas_get_item_at, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_items_at", (PyCFunction)_wrap_goo_canvas_get_items_at, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_items_in_area", (PyCFunction)_wrap_goo_canvas_get_items_in_area, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_scale", (PyCFunction)_wrap_goo_canvas_get_scale, METH_NOARGS,
      NULL },
    { "set_scale", (PyCFunction)_wrap_goo_canvas_set_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_bounds", (PyCFunction)_wrap_goo_canvas_get_bounds, METH_NOARGS,
      NULL },
    { "set_bounds", (PyCFunction)_wrap_goo_canvas_set_bounds, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "scroll_to", (PyCFunction)_wrap_goo_canvas_scroll_to, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "grab_focus", (PyCFunction)_wrap_goo_canvas_grab_focus, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "render", (PyCFunction)_wrap_goo_canvas_render, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "convert_to_pixels", (PyCFunction)_wrap_goo_canvas_convert_to_pixels, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "convert_from_pixels", (PyCFunction)_wrap_goo_canvas_convert_from_pixels, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "convert_to_item_space", (PyCFunction)_wrap_goo_canvas_convert_to_item_space, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "convert_from_item_space", (PyCFunction)_wrap_goo_canvas_convert_from_item_space, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pointer_grab", (PyCFunction)_wrap_goo_canvas_pointer_grab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "pointer_ungrab", (PyCFunction)_wrap_goo_canvas_pointer_ungrab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyboard_grab", (PyCFunction)_wrap_goo_canvas_keyboard_grab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "keyboard_ungrab", (PyCFunction)_wrap_goo_canvas_keyboard_ungrab, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "create_cairo_context", (PyCFunction)_wrap_goo_canvas_create_cairo_context, METH_NOARGS,
      NULL },
    { "create_item", (PyCFunction)_wrap_goo_canvas_create_item, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unregister_item", (PyCFunction)_wrap_goo_canvas_unregister_item, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "update", (PyCFunction)_wrap_goo_canvas_update, METH_NOARGS,
      NULL },
    { "request_update", (PyCFunction)_wrap_goo_canvas_request_update, METH_NOARGS,
      NULL },
    { "request_redraw", (PyCFunction)_wrap_goo_canvas_request_redraw, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_default_line_width", (PyCFunction)_wrap_goo_canvas_get_default_line_width, METH_NOARGS,
      NULL },
    { "register_widget_item", (PyCFunction)_wrap_goo_canvas_register_widget_item, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "unregister_widget_item", (PyCFunction)_wrap_goo_canvas_unregister_widget_item, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_set_scroll_adjustments", (PyCFunction)_wrap_GooCanvas__do_set_scroll_adjustments, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_create_item", (PyCFunction)_wrap_GooCanvas__do_create_item, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_item_created", (PyCFunction)_wrap_GooCanvas__do_item_created, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGooCanvas_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Canvas",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGooCanvas_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_goo_canvas_new,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GooCanvas__proxy_do_set_scroll_adjustments(GooCanvas *self, GtkAdjustment*hadjustment, GtkAdjustment*vadjustment)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_hadjustment = NULL;
    PyObject *py_vadjustment = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (hadjustment)
        py_hadjustment = pygobject_new((GObject *) hadjustment);
    else {
        Py_INCREF(Py_None);
        py_hadjustment = Py_None;
    }
    if (vadjustment)
        py_vadjustment = pygobject_new((GObject *) vadjustment);
    else {
        Py_INCREF(Py_None);
        py_vadjustment = Py_None;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_hadjustment);
    PyTuple_SET_ITEM(py_args, 1, py_vadjustment);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_scroll_adjustments");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GooCanvasItem*
_wrap_GooCanvas__proxy_do_create_item(GooCanvas *self, GooCanvasItemModel*model)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_model = NULL;
    GooCanvasItem* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (model)
        py_model = pygobject_new((GObject *) model);
    else {
        Py_INCREF(Py_None);
        py_model = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_model);
    
    py_method = PyObject_GetAttrString(py_self, "do_create_item");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasItem*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvas__proxy_do_item_created(GooCanvas *self, GooCanvasItem*item, GooCanvasItemModel*model)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_item = NULL;
    PyObject *py_model = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (item)
        py_item = pygobject_new((GObject *) item);
    else {
        Py_INCREF(Py_None);
        py_item = Py_None;
    }
    if (model)
        py_model = pygobject_new((GObject *) model);
    else {
        Py_INCREF(Py_None);
        py_model = Py_None;
    }
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_item);
    PyTuple_SET_ITEM(py_args, 1, py_model);
    
    py_method = PyObject_GetAttrString(py_self, "do_item_created");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static int
__GooCanvas_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GooCanvasClass *klass = GOO_CANVAS_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_set_scroll_adjustments");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "set_scroll_adjustments")))
            klass->set_scroll_adjustments = _wrap_GooCanvas__proxy_do_set_scroll_adjustments;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_create_item");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "create_item")))
            klass->create_item = _wrap_GooCanvas__proxy_do_create_item;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_item_created");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "item_created")))
            klass->item_created = _wrap_GooCanvas__proxy_do_item_created;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GooCanvasItemModelSimple ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasItemModelSimple_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.ItemModelSimple",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasImageModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasImageModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.ImageModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasGroupModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasGroupModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.GroupModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasEllipseModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasEllipseModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.EllipseModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasItemSimple ----------- */

#line 257 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_simple_get_path_bounds(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", NULL };
    PyObject *py_bounds;
    PycairoContext *cr;
    GooCanvasBounds bounds = {0,};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!:get_path_bounds",
        kwlist, &PycairoContext_Type, &cr))
        return NULL;

    GooCanvasItemSimple *simple = (GooCanvasItemSimple*) self->obj;

    goo_canvas_item_simple_get_path_bounds (simple, cr->ctx, &bounds);

    py_bounds = pygoo_canvas_bounds_new(&bounds);

    return py_bounds;
}

#line 1701 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_simple_user_bounds_to_device(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "bounds", NULL };
    PyObject *py_bounds;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItemSimple.user_bounds_to_device", kwlist, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_bounds))
        return NULL;
    
    goo_canvas_item_simple_user_bounds_to_device(GOO_CANVAS_ITEM_SIMPLE(self->obj), cr->ctx, (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_simple_user_bounds_to_parent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "bounds", NULL };
    PyObject *py_bounds;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItemSimple.user_bounds_to_parent", kwlist, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_bounds))
        return NULL;
    
    goo_canvas_item_simple_user_bounds_to_parent(GOO_CANVAS_ITEM_SIMPLE(self->obj), cr->ctx, (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_simple_check_in_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "cr", "pointer_events", NULL };
    PyObject *py_pointer_events = NULL;
    double x, y;
    int ret;
    PycairoContext *cr;
    GooCanvasPointerEvents pointer_events;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddO!O:GooCanvasItemSimple.check_in_path", kwlist, &x, &y, &PycairoContext_Type, &cr, &py_pointer_events))
        return NULL;
    if (pyg_flags_get_value(GOO_TYPE_CANVAS_POINTER_EVENTS, py_pointer_events, (gpointer)&pointer_events))
        return NULL;
    
    ret = goo_canvas_item_simple_check_in_path(GOO_CANVAS_ITEM_SIMPLE(self->obj), x, y, cr->ctx, pointer_events);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_goo_canvas_item_simple_paint_path(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", NULL };
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemSimple.paint_path", kwlist, &PycairoContext_Type, &cr))
        return NULL;
    
    goo_canvas_item_simple_paint_path(GOO_CANVAS_ITEM_SIMPLE(self->obj), cr->ctx);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_simple_changed(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "recompute_bounds", NULL };
    int recompute_bounds;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GooCanvasItemSimple.changed", kwlist, &recompute_bounds))
        return NULL;
    
    goo_canvas_item_simple_changed(GOO_CANVAS_ITEM_SIMPLE(self->obj), recompute_bounds);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_simple_check_style(PyGObject *self)
{
    
    goo_canvas_item_simple_check_style(GOO_CANVAS_ITEM_SIMPLE(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_simple_get_line_width(PyGObject *self)
{
    double ret;

    
    ret = goo_canvas_item_simple_get_line_width(GOO_CANVAS_ITEM_SIMPLE(self->obj));
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_goo_canvas_item_simple_set_model(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "model", NULL };
    PyGObject *model;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemSimple.set_model", kwlist, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    
    goo_canvas_item_simple_set_model(GOO_CANVAS_ITEM_SIMPLE(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemSimple__do_simple_create_path(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "cr", NULL };
    PyGObject *self;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItemSimple.simple_create_path", kwlist, &PyGooCanvasItemSimple_Type, &self, &PycairoContext_Type, &cr))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_create_path)
        GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_create_path(GOO_CANVAS_ITEM_SIMPLE(self->obj), cr->ctx);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GooCanvasItemSimple.simple_create_path not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemSimple__do_simple_update(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "cr", NULL };
    PyGObject *self;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItemSimple.simple_update", kwlist, &PyGooCanvasItemSimple_Type, &self, &PycairoContext_Type, &cr))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_update)
        GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_update(GOO_CANVAS_ITEM_SIMPLE(self->obj), cr->ctx);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GooCanvasItemSimple.simple_update not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemSimple__do_simple_paint(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "cr", "bounds", NULL };
    PyGObject *self;
    PyObject *py_bounds;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!:GooCanvasItemSimple.simple_paint", kwlist, &PyGooCanvasItemSimple_Type, &self, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_bounds))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_paint)
        GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_paint(GOO_CANVAS_ITEM_SIMPLE(self->obj), cr->ctx, (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GooCanvasItemSimple.simple_paint not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemSimple__do_simple_is_item_at(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    gpointer klass;
    static char *kwlist[] = { "self", "x", "y", "cr", "is_pointer_event", NULL };
    PyGObject *self;
    double x, y;
    int is_pointer_event, ret;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ddO!i:GooCanvasItemSimple.simple_is_item_at", kwlist, &PyGooCanvasItemSimple_Type, &self, &x, &y, &PycairoContext_Type, &cr, &is_pointer_event))
        return NULL;
    klass = g_type_class_ref(pyg_type_from_object(cls));
    if (GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_is_item_at)
        ret = GOO_CANVAS_ITEM_SIMPLE_CLASS(klass)->simple_is_item_at(GOO_CANVAS_ITEM_SIMPLE(self->obj), x, y, cr->ctx, is_pointer_event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "virtual method GooCanvasItemSimple.simple_is_item_at not implemented");
        g_type_class_unref(klass);
        return NULL;
    }
    g_type_class_unref(klass);
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGooCanvasItemSimple_methods[] = {
    { "get_path_bounds", (PyCFunction)_wrap_goo_canvas_item_simple_get_path_bounds, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "user_bounds_to_device", (PyCFunction)_wrap_goo_canvas_item_simple_user_bounds_to_device, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "user_bounds_to_parent", (PyCFunction)_wrap_goo_canvas_item_simple_user_bounds_to_parent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "check_in_path", (PyCFunction)_wrap_goo_canvas_item_simple_check_in_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "paint_path", (PyCFunction)_wrap_goo_canvas_item_simple_paint_path, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "changed", (PyCFunction)_wrap_goo_canvas_item_simple_changed, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "check_style", (PyCFunction)_wrap_goo_canvas_item_simple_check_style, METH_NOARGS,
      NULL },
    { "get_line_width", (PyCFunction)_wrap_goo_canvas_item_simple_get_line_width, METH_NOARGS,
      NULL },
    { "set_model", (PyCFunction)_wrap_goo_canvas_item_simple_set_model, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "do_simple_create_path", (PyCFunction)_wrap_GooCanvasItemSimple__do_simple_create_path, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_simple_update", (PyCFunction)_wrap_GooCanvasItemSimple__do_simple_update, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_simple_paint", (PyCFunction)_wrap_GooCanvasItemSimple__do_simple_paint, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_simple_is_item_at", (PyCFunction)_wrap_GooCanvasItemSimple__do_simple_is_item_at, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 783 "goocanvas.override"

static int
_wrap_goo_canvas_item_simple__set_bounds_x1(PyGObject *self, PyObject *py_value, void *closure)
{
    if (!PyFloat_Check(py_value)) {
        PyErr_SetString(PyExc_TypeError, "value must be a float");
        return -1;
    }
    GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.x1 = PyFloat_AsDouble(py_value);
    return 0;
}

#line 1961 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_simple__get_bounds_x1(PyObject *self, void *closure)
{
    double ret;

    ret = GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.x1;
    return PyFloat_FromDouble(ret);
}

#line 797 "goocanvas.override"

static int
_wrap_goo_canvas_item_simple__set_bounds_x2(PyGObject *self, PyObject *py_value, void *closure)
{
    if (!PyFloat_Check(py_value)) {
        PyErr_SetString(PyExc_TypeError, "value must be a float");
        return -1;
    }
    GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.x2 = PyFloat_AsDouble(py_value);
    return 0;
}

#line 1986 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_simple__get_bounds_x2(PyObject *self, void *closure)
{
    double ret;

    ret = GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.x2;
    return PyFloat_FromDouble(ret);
}

#line 811 "goocanvas.override"

static int
_wrap_goo_canvas_item_simple__set_bounds_y1(PyGObject *self, PyObject *py_value, void *closure)
{
    if (!PyFloat_Check(py_value)) {
        PyErr_SetString(PyExc_TypeError, "value must be a float");
        return -1;
    }
    GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.y1 = PyFloat_AsDouble(py_value);
    return 0;
}

#line 2011 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_simple__get_bounds_y1(PyObject *self, void *closure)
{
    double ret;

    ret = GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.y1;
    return PyFloat_FromDouble(ret);
}

#line 825 "goocanvas.override"

static int
_wrap_goo_canvas_item_simple__set_bounds_y2(PyGObject *self, PyObject *py_value, void *closure)
{
    if (!PyFloat_Check(py_value)) {
        PyErr_SetString(PyExc_TypeError, "value must be a float");
        return -1;
    }
    GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.y2 = PyFloat_AsDouble(py_value);
    return 0;
}

#line 2036 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_simple__get_bounds_y2(PyObject *self, void *closure)
{
    double ret;

    ret = GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds.y2;
    return PyFloat_FromDouble(ret);
}

#line 1587 "goocanvas.override"

static int
_wrap_goo_canvas_item_simple__set_bounds(PyGObject *self, PyObject *py_value, void *closure)
{
    if (!PyObject_IsInstance(py_value, (PyObject*) &PyGooCanvasBounds_Type)) {
        PyErr_SetString(PyExc_TypeError, "value must be goocanvas.Bounds");
        return -1;
    }
    GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds = ((PyGooCanvasBounds*) py_value)->bounds;
    return 0;
}

static PyObject*
_wrap_goo_canvas_item_simple__get_bounds(PyGObject *self, void *closure)
{
    return pygoo_canvas_bounds_new(&GOO_CANVAS_ITEM_SIMPLE(pygobject_get(self))->bounds);
}
#line 2066 "goocanvas.c"


static const PyGetSetDef goo_canvas_item_simple_getsets[] = {
    { "bounds_x1", (getter)_wrap_goo_canvas_item_simple__get_bounds_x1, (setter)_wrap_goo_canvas_item_simple__set_bounds_x1 },
    { "bounds_x2", (getter)_wrap_goo_canvas_item_simple__get_bounds_x2, (setter)_wrap_goo_canvas_item_simple__set_bounds_x2 },
    { "bounds_y1", (getter)_wrap_goo_canvas_item_simple__get_bounds_y1, (setter)_wrap_goo_canvas_item_simple__set_bounds_y1 },
    { "bounds_y2", (getter)_wrap_goo_canvas_item_simple__get_bounds_y2, (setter)_wrap_goo_canvas_item_simple__set_bounds_y2 },
    { "bounds", (getter)_wrap_goo_canvas_item_simple__get_bounds, (setter)_wrap_goo_canvas_item_simple__set_bounds },
    { NULL, (getter)0, (setter)0 },
};

PyTypeObject G_GNUC_INTERNAL PyGooCanvasItemSimple_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.ItemSimple",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGooCanvasItemSimple_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)goo_canvas_item_simple_getsets,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static void
_wrap_GooCanvasItemSimple__proxy_do_simple_create_path(GooCanvasItemSimple *self, cairo_t*cr)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cr;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_cr);
    
    py_method = PyObject_GetAttrString(py_self, "do_simple_create_path");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItemSimple__proxy_do_simple_update(GooCanvasItemSimple *self, cairo_t*cr)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cr;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_cr);
    
    py_method = PyObject_GetAttrString(py_self, "do_simple_update");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItemSimple__proxy_do_simple_paint(GooCanvasItemSimple *self, cairo_t*cr, const GooCanvasBounds*bounds)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cr;
    PyObject *py_bounds;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    py_bounds = pygoo_canvas_bounds_new(bounds);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_cr);
    PyTuple_SET_ITEM(py_args, 1, py_bounds);
    
    py_method = PyObject_GetAttrString(py_self, "do_simple_paint");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gboolean
_wrap_GooCanvasItemSimple__proxy_do_simple_is_item_at(GooCanvasItemSimple *self, gdouble x, gdouble y, cairo_t*cr, gboolean is_pointer_event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_x;
    PyObject *py_y;
    PyObject *py_cr;
    PyObject *py_is_pointer_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_x = PyFloat_FromDouble(x);
    py_y = PyFloat_FromDouble(y);
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    py_is_pointer_event = is_pointer_event? Py_True : Py_False;
    
    py_args = PyTuple_New(4);
    PyTuple_SET_ITEM(py_args, 0, py_x);
    PyTuple_SET_ITEM(py_args, 1, py_y);
    PyTuple_SET_ITEM(py_args, 2, py_cr);
    Py_INCREF(py_is_pointer_event);
    PyTuple_SET_ITEM(py_args, 3, py_is_pointer_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_simple_is_item_at");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static int
__GooCanvasItemSimple_class_init(gpointer gclass, PyTypeObject *pyclass)
{
    PyObject *o;
    GooCanvasItemSimpleClass *klass = GOO_CANVAS_ITEM_SIMPLE_CLASS(gclass);
    PyObject *gsignals = PyDict_GetItemString(pyclass->tp_dict, "__gsignals__");

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_simple_create_path");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "simple_create_path")))
            klass->simple_create_path = _wrap_GooCanvasItemSimple__proxy_do_simple_create_path;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_simple_update");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "simple_update")))
            klass->simple_update = _wrap_GooCanvasItemSimple__proxy_do_simple_update;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_simple_paint");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "simple_paint")))
            klass->simple_paint = _wrap_GooCanvasItemSimple__proxy_do_simple_paint;
        Py_DECREF(o);
    }

    o = PyObject_GetAttrString((PyObject *) pyclass, "do_simple_is_item_at");
    if (o == NULL)
        PyErr_Clear();
    else {
        if (!PyObject_TypeCheck(o, &PyCFunction_Type)
            && !(gsignals && PyDict_GetItemString(gsignals, "simple_is_item_at")))
            klass->simple_is_item_at = _wrap_GooCanvasItemSimple__proxy_do_simple_is_item_at;
        Py_DECREF(o);
    }
    return 0;
}


/* ----------- GooCanvasImage ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasImage_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Image",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasSvg ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasSvg_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Svg",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasGroup ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasGroup_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Group",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasEllipse ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasEllipse_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Ellipse",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasPath ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasPath_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Path",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasPathModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasPathModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.PathModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasPolyline ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasPolyline_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Polyline",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasPolylineModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasPolylineModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.PolylineModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasRect ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasRect_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Rect",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasRectModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasRectModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.RectModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasStyle ----------- */

static PyObject *
_wrap_goo_canvas_style_copy(PyGObject *self)
{
    GooCanvasStyle *ret;

    
    ret = goo_canvas_style_copy(GOO_CANVAS_STYLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_style_get_parent(PyGObject *self)
{
    GooCanvasStyle *ret;

    
    ret = goo_canvas_style_get_parent(GOO_CANVAS_STYLE(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_style_set_parent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parent", NULL };
    PyGObject *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasStyle.set_parent", kwlist, &PyGooCanvasStyle_Type, &parent))
        return NULL;
    
    goo_canvas_style_set_parent(GOO_CANVAS_STYLE(self->obj), GOO_CANVAS_STYLE(parent->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 716 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_style_get_property(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"s:goocanvas.Style.get_property",
    				     kwlist, &name))
        return NULL;
    return _py_canvas_style_get_property(GOO_CANVAS_STYLE(self->obj), name);
}

#line 2981 "goocanvas.c"


#line 730 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_style_set_property(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", "value", NULL };
    char *name;
    PyObject *py_value;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"sO:goocanvas.Style.set_property",
    				     kwlist, &name, &py_value))
        return NULL;
    if (_py_goo_canvas_style_set_property(GOO_CANVAS_STYLE(self->obj), name, py_value))
        return NULL;
    Py_INCREF(Py_None);
    return Py_None;
}

#line 3001 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_style_set_stroke_options(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", NULL };
    int ret;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasStyle.set_stroke_options", kwlist, &PycairoContext_Type, &cr))
        return NULL;
    
    ret = goo_canvas_style_set_stroke_options(GOO_CANVAS_STYLE(self->obj), cr->ctx);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_goo_canvas_style_set_fill_options(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", NULL };
    int ret;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasStyle.set_fill_options", kwlist, &PycairoContext_Type, &cr))
        return NULL;
    
    ret = goo_canvas_style_set_fill_options(GOO_CANVAS_STYLE(self->obj), cr->ctx);
    
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGooCanvasStyle_methods[] = {
    { "copy", (PyCFunction)_wrap_goo_canvas_style_copy, METH_NOARGS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_goo_canvas_style_get_parent, METH_NOARGS,
      NULL },
    { "set_parent", (PyCFunction)_wrap_goo_canvas_style_set_parent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_style_property", (PyCFunction)_wrap_goo_canvas_style_get_property, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_style_property", (PyCFunction)_wrap_goo_canvas_style_set_property, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_stroke_options", (PyCFunction)_wrap_goo_canvas_style_set_stroke_options, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_fill_options", (PyCFunction)_wrap_goo_canvas_style_set_fill_options, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};

#line 748 "goocanvas.override"

static PyObject *
goo_canvas_style_subscript(PyGObject *self, PyObject *arg)
{
    const char *name;
    if (!PyString_Check(arg)) {
        PyErr_SetString(PyExc_TypeError, "key must be a string");
        return NULL;
    }
    name = PyString_AsString(arg);
    return _py_canvas_style_get_property(GOO_CANVAS_STYLE(self->obj), name);
}

static int
_wrap_goo_canvas_style_ass_subscript(PyGObject *self, PyObject *key, PyObject *py_value)
{
    char *name;

    if (!PyString_Check(key)) {
        PyErr_SetString(PyExc_TypeError, "key must be a string");
        return -1;
    }
    name = PyString_AsString(key);
    return _py_goo_canvas_style_set_property(GOO_CANVAS_STYLE(self->obj), name, py_value);
}


static PyMappingMethods _wrap_goo_canvas_style_tp_as_mapping = {
    NULL, /* inquiry mp_length; */
    (binaryfunc) goo_canvas_style_subscript, /* mp_subscript; */
    (objobjargproc) _wrap_goo_canvas_style_ass_subscript, /* objobjargproc mp_ass_subscript; */
};

#line 3088 "goocanvas.c"


PyTypeObject G_GNUC_INTERNAL PyGooCanvasStyle_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Style",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)&_wrap_goo_canvas_style_tp_as_mapping,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGooCanvasStyle_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasTable ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasTable_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Table",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasTableModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasTableModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.TableModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasText ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasText_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Text",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasTextModel ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasTextModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.TextModel",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasWidget ----------- */

PyTypeObject G_GNUC_INTERNAL PyGooCanvasWidget_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Widget",                   /* tp_name */
    sizeof(PyGObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)NULL, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- GooCanvasItem ----------- */

static PyObject *
_wrap_goo_canvas_item_get_n_children(PyGObject *self)
{
    int ret;

    
    ret = goo_canvas_item_get_n_children(GOO_CANVAS_ITEM(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_goo_canvas_item_get_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child_num", NULL };
    int child_num;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GooCanvasItem.get_child", kwlist, &child_num))
        return NULL;
    
    ret = goo_canvas_item_get_child(GOO_CANVAS_ITEM(self->obj), child_num);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_find_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child", NULL };
    PyGObject *child;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.find_child", kwlist, &PyGooCanvasItem_Type, &child))
        return NULL;
    
    ret = goo_canvas_item_find_child(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(child->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_goo_canvas_item_add_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child", "position", NULL };
    PyGObject *child;
    int position = -1;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|i:GooCanvasItem.add_child", kwlist, &PyGooCanvasItem_Type, &child, &position))
        return NULL;
    
    goo_canvas_item_add_child(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(child->obj), position);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_move_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "old_position", "new_position", NULL };
    int old_position, new_position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GooCanvasItem.move_child", kwlist, &old_position, &new_position))
        return NULL;
    
    goo_canvas_item_move_child(GOO_CANVAS_ITEM(self->obj), old_position, new_position);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 483 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_remove_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child", NULL };
    int child_num;
    PyObject *py_child;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GooCanvasItem.remove_child", kwlist, &py_child))
        return NULL;

    if (PyInt_Check(py_child))
        child_num = PyInt_AsLong(py_child);
    else {
        if (!PyObject_IsInstance(py_child, (PyObject *) &PyGooCanvasItem_Type)) {
            PyErr_SetString(PyExc_TypeError, "argument must be integer or goocanvas.Item");
            return NULL;
        }
        child_num = goo_canvas_item_find_child(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(pygobject_get(py_child)));
        if (child_num == -1) {
            PyErr_SetString(PyExc_ValueError, "child not found");
            return NULL;
        }
    }
    goo_canvas_item_remove_child(GOO_CANVAS_ITEM(self->obj), child_num);

    Py_INCREF(Py_None);
    return Py_None;
}
#line 3487 "goocanvas.c"


#line 921 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_get_child_properties(PyGObject *self, PyObject *args)
{
    GObjectClass    *class;
    GooCanvasItem   *parent, *child;
    PyGObject       *pychild;
    int             i, len;
    gint            c_pos;
    PyObject        *tuple;

    if ((len = PyTuple_Size(args)) < 1) {
        PyErr_SetString(PyExc_TypeError, "requires at least one argument");
        return NULL;
    }

    pychild = (PyGObject*)PyTuple_GetItem(args, 0);

    if (!pygobject_check(pychild, &PyGooCanvasItem_Type)) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument should be a GooCanvasItem");
        return NULL;
    }

    parent = GOO_CANVAS_ITEM(self->obj);
    child = GOO_CANVAS_ITEM(pychild->obj);

    c_pos = goo_canvas_item_find_child(parent, child);
    if (c_pos == -1) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument must be a child");
        return NULL;
    }

    tuple = PyTuple_New(len-1);

    class = G_OBJECT_GET_CLASS(self->obj);

    for (i = 1; i < len; i++) {
        PyObject *py_property = PyTuple_GetItem(args, i);
        gchar *property_name;
        GParamSpec *pspec;
        GValue value = { 0 };
        PyObject *item;

        if (!PyString_Check(py_property)) {
            PyErr_SetString(PyExc_TypeError,
                            "Expected string argument for property.");
            return NULL;
        }

        property_name = PyString_AsString(py_property);

        pspec = goo_canvas_item_class_find_child_property (class, property_name);
        if (!pspec) {
	    PyErr_Format(PyExc_TypeError,
		    "object of type `%s' does not have child property `%s'",
		    g_type_name(G_OBJECT_TYPE(self->obj)), property_name);
    	    return NULL;
        }
        if (!(pspec->flags & G_PARAM_READABLE)) {
            PyErr_Format(PyExc_TypeError, "property %s is not readable", property_name);
            return NULL;
        }
        g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
        GooCanvasItemIface *iface;

	    iface = g_type_interface_peek (class, GOO_TYPE_CANVAS_ITEM);
	    iface->get_child_property ((GooCanvasItem*) parent,
		                           (GooCanvasItem*) child,
                                    pspec->param_id, &value, pspec);

        item = pyg_value_as_pyobject(&value, TRUE);
        PyTuple_SetItem(tuple, i-1, item);

        g_value_unset(&value);
    }

    return tuple;
}

#line 3571 "goocanvas.c"


#line 839 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_set_child_properties(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    PyGObject       *pychild;
    GObjectClass    *class;
    GooCanvasItem   *parent, *child;
    Py_ssize_t      pos;
    int             len;
    gint            c_pos;
    PyObject        *value;
    PyObject        *key;

    if ((len = PyTuple_Size(args)) < 1) {
        PyErr_SetString(PyExc_TypeError, "requires at least one argument");
        return NULL;
    }
    pychild = (PyGObject*)PyTuple_GetItem(args, 0);

    if (!pygobject_check(pychild, &PyGooCanvasItem_Type)) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument should be a GooCanvasItem");
        return NULL;
    }

    parent = GOO_CANVAS_ITEM(self->obj);
    child = GOO_CANVAS_ITEM(pychild->obj);

    c_pos = goo_canvas_item_find_child(parent, child);
    if (c_pos == -1) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument must be a child");
        return NULL;
    }

    class = G_OBJECT_GET_CLASS(self->obj);

    g_object_freeze_notify (G_OBJECT(self->obj));
    pos = 0;

    while (kwargs && PyDict_Next (kwargs, &pos, &key, &value)) {
    gchar *key_str = PyString_AsString (key);
    GParamSpec *pspec;
    GValue gvalue ={ 0, };

    pspec = goo_canvas_item_class_find_child_property (class, key_str);
    if (!pspec) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "object `%s' doesn't support child property `%s'",
		       g_type_name(G_OBJECT_TYPE(pychild->obj)), key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    return NULL;
	}

	g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
	if (pyg_value_from_pyobject(&gvalue, value)) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "could not convert value for property `%s'", key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    return NULL;
	}
	GooCanvasItemIface *iface;

	iface = g_type_interface_peek (class, GOO_TYPE_CANVAS_ITEM);
	iface->set_child_property ((GooCanvasItem*) parent,
	                           (GooCanvasItem*) child,
	                           pspec->param_id, &gvalue, pspec);

	g_value_unset(&gvalue);
    }

    g_object_thaw_notify (G_OBJECT(self->obj));

    Py_INCREF(Py_None);
    return Py_None;
}

#line 3655 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_get_transform_for_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child", "transform", NULL };
    PyGObject *child;
    PyObject *py_transform;
    int ret;
    cairo_matrix_t *transform;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GooCanvasItem.get_transform_for_child", kwlist, &PyGooCanvasItem_Type, &child, &py_transform))
        return NULL;
    transform = &((PycairoMatrix*)(py_transform))->matrix;
    
    ret = goo_canvas_item_get_transform_for_child(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(child->obj), transform);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_goo_canvas_item_get_canvas(PyGObject *self)
{
    GooCanvas *ret;

    
    ret = goo_canvas_item_get_canvas(GOO_CANVAS_ITEM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_set_canvas(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "canvas", NULL };
    PyGObject *canvas;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.set_canvas", kwlist, &PyGooCanvas_Type, &canvas))
        return NULL;
    
    goo_canvas_item_set_canvas(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS(canvas->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_get_parent(PyGObject *self)
{
    GooCanvasItem *ret;

    
    ret = goo_canvas_item_get_parent(GOO_CANVAS_ITEM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_set_parent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parent", NULL };
    PyGObject *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.set_parent", kwlist, &PyGooCanvasItem_Type, &parent))
        return NULL;
    
    goo_canvas_item_set_parent(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(parent->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_remove(PyGObject *self)
{
    
    goo_canvas_item_remove(GOO_CANVAS_ITEM(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_is_container(PyGObject *self)
{
    int ret;

    
    ret = goo_canvas_item_is_container(GOO_CANVAS_ITEM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_goo_canvas_item_raise(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "above", NULL };
    PyGObject *py_above;
    GooCanvasItem *above = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GooCanvasItem.raise", kwlist, &py_above))
        return NULL;
    if (py_above && pygobject_check(py_above, &PyGooCanvasItem_Type))
        above = GOO_CANVAS_ITEM(py_above->obj);
    else if ((PyObject *)py_above != Py_None) {
        PyErr_SetString(PyExc_TypeError, "above should be a GooCanvasItem or None");
        return NULL;
    }
    
    goo_canvas_item_raise(GOO_CANVAS_ITEM(self->obj), (GooCanvasItem *) above);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_lower(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "below", NULL };
    PyGObject *py_below;
    GooCanvasItem *below = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GooCanvasItem.lower", kwlist, &py_below))
        return NULL;
    if (py_below && pygobject_check(py_below, &PyGooCanvasItem_Type))
        below = GOO_CANVAS_ITEM(py_below->obj);
    else if ((PyObject *)py_below != Py_None) {
        PyErr_SetString(PyExc_TypeError, "below should be a GooCanvasItem or None");
        return NULL;
    }
    
    goo_canvas_item_lower(GOO_CANVAS_ITEM(self->obj), (GooCanvasItem *) below);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1546 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_get_transform(PyGObject *self)
{
    PyObject        *matrix;
    cairo_matrix_t  transform = {0,};
    gboolean        res;

    res = goo_canvas_item_get_transform(GOO_CANVAS_ITEM(self->obj), &transform);

    if (!res) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    matrix = PycairoMatrix_FromMatrix(&transform);
    return matrix;
}

#line 3816 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_set_transform(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "matrix", NULL };
    PyObject *py_matrix;
    cairo_matrix_t *matrix;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GooCanvasItem.set_transform", kwlist, &py_matrix))
        return NULL;
    matrix = &((PycairoMatrix*)(py_matrix))->matrix;
    
    goo_canvas_item_set_transform(GOO_CANVAS_ITEM(self->obj), matrix);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_set_simple_transform(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "scale", "rotation", NULL };
    double x, y, scale, rotation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dddd:GooCanvasItem.set_simple_transform", kwlist, &x, &y, &scale, &rotation))
        return NULL;
    
    goo_canvas_item_set_simple_transform(GOO_CANVAS_ITEM(self->obj), x, y, scale, rotation);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_translate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tx", "ty", NULL };
    double tx, ty;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:GooCanvasItem.translate", kwlist, &tx, &ty))
        return NULL;
    
    goo_canvas_item_translate(GOO_CANVAS_ITEM(self->obj), tx, ty);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_scale(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sx", "sy", NULL };
    double sx, sy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:GooCanvasItem.scale", kwlist, &sx, &sy))
        return NULL;
    
    goo_canvas_item_scale(GOO_CANVAS_ITEM(self->obj), sx, sy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_rotate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "degrees", "cx", "cy", NULL };
    double degrees, cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GooCanvasItem.rotate", kwlist, &degrees, &cx, &cy))
        return NULL;
    
    goo_canvas_item_rotate(GOO_CANVAS_ITEM(self->obj), degrees, cx, cy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_skew_x(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "degrees", "cx", "cy", NULL };
    double degrees, cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GooCanvasItem.skew_x", kwlist, &degrees, &cx, &cy))
        return NULL;
    
    goo_canvas_item_skew_x(GOO_CANVAS_ITEM(self->obj), degrees, cx, cy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_skew_y(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "degrees", "cx", "cy", NULL };
    double degrees, cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GooCanvasItem.skew_y", kwlist, &degrees, &cx, &cy))
        return NULL;
    
    goo_canvas_item_skew_y(GOO_CANVAS_ITEM(self->obj), degrees, cx, cy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_get_style(PyGObject *self)
{
    GooCanvasStyle *ret;

    
    ret = goo_canvas_item_get_style(GOO_CANVAS_ITEM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_set_style(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "style", NULL };
    PyGObject *style;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.set_style", kwlist, &PyGooCanvasStyle_Type, &style))
        return NULL;
    
    goo_canvas_item_set_style(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_STYLE(style->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_animate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "scale", "degrees", "absolute", "duration", "step_time", "type", NULL };
    int absolute, duration, step_time;
    double x, y, scale, degrees;
    GooCanvasAnimateType type;
    PyObject *py_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddddiiiO:GooCanvasItem.animate", kwlist, &x, &y, &scale, &degrees, &absolute, &duration, &step_time, &py_type))
        return NULL;
    if (pyg_enum_get_value(GOO_TYPE_CANVAS_ANIMATE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    goo_canvas_item_animate(GOO_CANVAS_ITEM(self->obj), x, y, scale, degrees, absolute, duration, step_time, type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_stop_animation(PyGObject *self)
{
    
    goo_canvas_item_stop_animation(GOO_CANVAS_ITEM(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 525 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_get_bounds(PyGObject *self)
{
    PyObject        *py_bounds;
    GooCanvasBounds bounds = {0,};

    goo_canvas_item_get_bounds(GOO_CANVAS_ITEM(self->obj), &bounds);

    py_bounds = pygoo_canvas_bounds_new(&bounds);

    return py_bounds;
}

#line 3997 "goocanvas.c"


#line 1508 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_get_items_at(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char     *kwlist[] = { "x", "y", "cr", "is_pointer_event",
                                  "parent_is_visible", NULL };
    PyObject        *point_event, *ret, *parent_visible;
    gdouble         x, y;
    GList           *item_list;
    gboolean        is_pointer_event;
    gboolean        parent_is_visible;
    PycairoContext  *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "ddOOO:get_items_at",
                                     kwlist,
                                     &x, &y, &cr, &point_event,
                                     &parent_visible))
        return NULL;

    is_pointer_event = (PyObject_IsTrue(point_event)) ? TRUE : FALSE;
    parent_is_visible = (PyObject_IsTrue(parent_visible)) ? TRUE : FALSE;

    item_list = goo_canvas_item_get_items_at(GOO_CANVAS_ITEM(self->obj),
                                             x, y, cr->ctx, is_pointer_event,
                                             parent_is_visible, NULL);

    if (!item_list) {
        Py_INCREF (Py_None);
        return Py_None;
    }

    ret = _glist_to_pylist_objs(item_list);
    g_list_free(item_list);
    return ret;
}

#line 4037 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_is_visible(PyGObject *self)
{
    int ret;

    
    ret = goo_canvas_item_is_visible(GOO_CANVAS_ITEM(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_goo_canvas_item_get_model(PyGObject *self)
{
    GooCanvasItemModel *ret;

    
    ret = goo_canvas_item_get_model(GOO_CANVAS_ITEM(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_set_model(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "model", NULL };
    PyGObject *model;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.set_model", kwlist, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    
    goo_canvas_item_set_model(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_request_update(PyGObject *self)
{
    
    goo_canvas_item_request_update(GOO_CANVAS_ITEM(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_ensure_updated(PyGObject *self)
{
    
    goo_canvas_item_ensure_updated(GOO_CANVAS_ITEM(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_update(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "entire_tree", "cr", "bounds", NULL };
    int entire_tree;
    PyObject *py_bounds;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"iO!O!:GooCanvasItem.update", kwlist, &entire_tree, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_bounds))
        return NULL;
    
    goo_canvas_item_update(GOO_CANVAS_ITEM(self->obj), entire_tree, cr->ctx, (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_paint(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "bounds", "scale", NULL };
    PyObject *py_bounds = NULL;
    double scale = 1.0;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!|Od:GooCanvasItem.paint", kwlist, &PycairoContext_Type, &cr, &py_bounds, &scale))
        return NULL;
    if (!(py_bounds == NULL || py_bounds == Py_None || 
        PyObject_IsInstance(py_bounds, (PyObject *) &PyGooCanvasBounds_Type))) {
        PyErr_SetString(PyExc_TypeError, "parameter bounds must be goocanvas.Bounds or None");
        return NULL;
    }
    
    goo_canvas_item_paint(GOO_CANVAS_ITEM(self->obj), cr->ctx, (py_bounds == NULL || py_bounds == Py_None)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds, scale);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_get_requested_area(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "requested_area", NULL };
    PyObject *py_requested_area;
    int ret;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItem.get_requested_area", kwlist, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_requested_area))
        return NULL;
    
    ret = goo_canvas_item_get_requested_area(GOO_CANVAS_ITEM(self->obj), cr->ctx, (py_requested_area == NULL)? NULL : &((PyGooCanvasBounds *) py_requested_area)->bounds);
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_goo_canvas_item_get_requested_height(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "width", NULL };
    double width, ret;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!d:GooCanvasItem.get_requested_height", kwlist, &PycairoContext_Type, &cr, &width))
        return NULL;
    
    ret = goo_canvas_item_get_requested_height(GOO_CANVAS_ITEM(self->obj), cr->ctx, width);
    
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_goo_canvas_item_allocate_area(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "cr", "requested_area", "allocated_area", "x_offset", "y_offset", NULL };
    PyObject *py_requested_area, *py_allocated_area;
    double x_offset, y_offset;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!dd:GooCanvasItem.allocate_area", kwlist, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_requested_area, &PyGooCanvasBounds_Type, &py_allocated_area, &x_offset, &y_offset))
        return NULL;
    
    goo_canvas_item_allocate_area(GOO_CANVAS_ITEM(self->obj), cr->ctx, (py_requested_area == NULL)? NULL : &((PyGooCanvasBounds *) py_requested_area)->bounds, (py_allocated_area == NULL)? NULL : &((PyGooCanvasBounds *) py_allocated_area)->bounds, x_offset, y_offset);
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1208 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_find_child_property (PyObject *cls,
                                           PyObject *args,
                                           PyObject *kwargs)
{
    static char         *kwlist[] = { "property", NULL };
    GObjectClass        *klass;
    GType               itype;
    const gchar         *prop_name;
    GParamSpec          *pspec;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "s:item_class_find_child_property",
                                     kwlist,
                                     &prop_name))
        return NULL;

    if ((itype = pyg_type_from_object(cls)) == 0)
	return NULL;

    klass = g_type_class_ref(itype);

    if (!klass) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not get a reference to type class");
	return NULL;
    }

    pspec = goo_canvas_item_class_find_child_property (klass, prop_name);

    if(!pspec){
        PyErr_Format(PyExc_KeyError,
                        "object %s does not support property %s",
                        g_type_name(itype), prop_name);
	return NULL;
    }
    return pyg_param_spec_new(pspec);
}

#line 4227 "goocanvas.c"


#line 1291 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_list_child_properties (PyObject *cls,
                                             PyObject *args,
                                             PyObject *kwargs)
{
    GParamSpec      **specs;
    PyObject        *list;
    GType           itype;
    GObjectClass    *klass;
    guint           nprops;
    guint           i;

    if ((itype = pyg_type_from_object(cls)) == 0)
	return NULL;

    klass = g_type_class_ref(itype);
    if (!klass) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not get a reference to type class");
	return NULL;
    }

    specs = goo_canvas_item_class_list_child_properties(klass, &nprops);
    list = PyList_New(nprops);
    if (list == NULL) {
	g_free(specs);
	g_type_class_unref(klass);
	return NULL;
    }

    for (i = 0; i < nprops; i++) {
	PyList_SetItem(list, i, pyg_param_spec_new(specs[i]));
    }

    g_free(specs);
    g_type_class_unref(klass);

    return list;
}

#line 4271 "goocanvas.c"


#line 1385 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_install_child_property (PyObject *cls,
                                              PyObject *args,
                                              PyObject* kwargs)
{
    static char         *kwlist[] = { "property_id", "pspec", NULL };
    PyObject            *property;
    GType               itype;
    GObjectClass        *klass;
    guint               property_id;
    GParamSpec          *pspec;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:item_class_install_child_property",
				     kwlist,
				     &property_id, &property))
	return NULL;


    if ((itype = pyg_type_from_object(cls)) == 0)
	return NULL;

    klass = g_type_class_ref(itype);
    if (!klass) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not get a reference to type class");
	return NULL;
    }

    pspec = pyg_param_spec_from_object(property);
    if(!pspec) {
	g_type_class_unref(klass);
	return NULL;
    }

    if (goo_canvas_item_class_find_child_property(G_OBJECT_CLASS(klass), pspec->name)) {
	PyErr_Format(PyExc_TypeError,
		     "there is already a '%s' property installed", pspec->name);
	g_type_class_unref(klass);
	return NULL;
    }

    goo_canvas_item_class_install_child_property(klass, property_id, pspec);

    g_type_class_unref(klass);

    Py_INCREF(Py_None);
    return Py_None;
}

#line 4325 "goocanvas.c"


static PyObject *
_wrap_GooCanvasItem__do_get_canvas(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GooCanvas *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.get_canvas", kwlist, &PyGooCanvasItem_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_canvas)
        ret = iface->get_canvas(GOO_CANVAS_ITEM(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_canvas not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItem__do_set_canvas(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "canvas", NULL };
    PyGObject *self, *canvas;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItem.set_canvas", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvas_Type, &canvas))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->set_canvas)
        iface->set_canvas(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS(canvas->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.set_canvas not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_get_n_children(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.get_n_children", kwlist, &PyGooCanvasItem_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_n_children)
        ret = iface->get_n_children(GOO_CANVAS_ITEM(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_n_children not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_GooCanvasItem__do_get_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "child_num", NULL };
    PyGObject *self;
    int child_num;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItem.get_child", kwlist, &PyGooCanvasItem_Type, &self, &child_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_child)
        ret = iface->get_child(GOO_CANVAS_ITEM(self->obj), child_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_child not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItem__do_request_update(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.request_update", kwlist, &PyGooCanvasItem_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->request_update)
        iface->request_update(GOO_CANVAS_ITEM(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.request_update not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_add_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "child", "position", NULL };
    PyGObject *self, *child;
    int position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!i:GooCanvasItem.add_child", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &child, &position))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->add_child)
        iface->add_child(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(child->obj), position);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.add_child not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_move_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "old_position", "new_position", NULL };
    PyGObject *self;
    int old_position, new_position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:GooCanvasItem.move_child", kwlist, &PyGooCanvasItem_Type, &self, &old_position, &new_position))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->move_child)
        iface->move_child(GOO_CANVAS_ITEM(self->obj), old_position, new_position);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.move_child not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_remove_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "child_num", NULL };
    PyGObject *self;
    int child_num;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItem.remove_child", kwlist, &PyGooCanvasItem_Type, &self, &child_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->remove_child)
        iface->remove_child(GOO_CANVAS_ITEM(self->obj), child_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.remove_child not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_get_transform_for_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "child", "transform", NULL };
    PyGObject *self, *child;
    PyObject *py_transform;
    int ret;
    cairo_matrix_t *transform;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.get_transform_for_child", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &child, &py_transform))
        return NULL;
    transform = &((PycairoMatrix*)(py_transform))->matrix;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_transform_for_child)
        ret = iface->get_transform_for_child(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(child->obj), transform);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_transform_for_child not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_get_parent(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.get_parent", kwlist, &PyGooCanvasItem_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_parent)
        ret = iface->get_parent(GOO_CANVAS_ITEM(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_parent not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItem__do_set_parent(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "parent", NULL };
    PyGObject *self, *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItem.set_parent", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &parent))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->set_parent)
        iface->set_parent(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(parent->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.set_parent not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_get_bounds(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "bounds", NULL };
    PyGObject *self;
    PyObject *py_bounds;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItem.get_bounds", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasBounds_Type, &py_bounds))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_bounds)
        iface->get_bounds(GOO_CANVAS_ITEM(self->obj), (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_bounds not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_update(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "entire_tree", "cr", "bounds", NULL };
    PyGObject *self;
    int entire_tree;
    PyObject *py_bounds;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!iO!O!:GooCanvasItem.update", kwlist, &PyGooCanvasItem_Type, &self, &entire_tree, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_bounds))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->update)
        iface->update(GOO_CANVAS_ITEM(self->obj), entire_tree, cr->ctx, (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.update not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_paint(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "cr", "bounds", "scale", NULL };
    PyGObject *self;
    PyObject *py_bounds;
    PycairoContext *cr;
    double scale;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!d:GooCanvasItem.paint", kwlist, &PyGooCanvasItem_Type, &self, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_bounds, &scale))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->paint)
        iface->paint(GOO_CANVAS_ITEM(self->obj), cr->ctx, (py_bounds == NULL)? NULL : &((PyGooCanvasBounds *) py_bounds)->bounds, scale);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.paint not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_get_requested_area(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "cr", "requested_area", NULL };
    PyGObject *self;
    PyObject *py_requested_area;
    int ret;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!:GooCanvasItem.get_requested_area", kwlist, &PyGooCanvasItem_Type, &self, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_requested_area))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_requested_area)
        ret = iface->get_requested_area(GOO_CANVAS_ITEM(self->obj), cr->ctx, (py_requested_area == NULL)? NULL : &((PyGooCanvasBounds *) py_requested_area)->bounds);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_requested_area not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_allocate_area(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "cr", "requested_area", "allocated_area", "x_offset", "y_offset", NULL };
    PyGObject *self;
    PyObject *py_requested_area, *py_allocated_area;
    PycairoContext *cr;
    double x_offset, y_offset;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O!O!dd:GooCanvasItem.allocate_area", kwlist, &PyGooCanvasItem_Type, &self, &PycairoContext_Type, &cr, &PyGooCanvasBounds_Type, &py_requested_area, &PyGooCanvasBounds_Type, &py_allocated_area, &x_offset, &y_offset))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->allocate_area)
        iface->allocate_area(GOO_CANVAS_ITEM(self->obj), cr->ctx, (py_requested_area == NULL)? NULL : &((PyGooCanvasBounds *) py_requested_area)->bounds, (py_allocated_area == NULL)? NULL : &((PyGooCanvasBounds *) py_allocated_area)->bounds, x_offset, y_offset);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.allocate_area not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_get_transform(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "transform", NULL };
    PyGObject *self;
    PyObject *py_transform;
    int ret;
    cairo_matrix_t *transform;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GooCanvasItem.get_transform", kwlist, &PyGooCanvasItem_Type, &self, &py_transform))
        return NULL;
    transform = &((PycairoMatrix*)(py_transform))->matrix;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_transform)
        ret = iface->get_transform(GOO_CANVAS_ITEM(self->obj), transform);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_transform not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_set_transform(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "matrix", NULL };
    PyGObject *self;
    PyObject *py_matrix;
    cairo_matrix_t *matrix;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GooCanvasItem.set_transform", kwlist, &PyGooCanvasItem_Type, &self, &py_matrix))
        return NULL;
    matrix = &((PycairoMatrix*)(py_matrix))->matrix;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->set_transform)
        iface->set_transform(GOO_CANVAS_ITEM(self->obj), matrix);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.set_transform not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_get_style(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GooCanvasStyle *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.get_style", kwlist, &PyGooCanvasItem_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_style)
        ret = iface->get_style(GOO_CANVAS_ITEM(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_style not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItem__do_set_style(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "style", NULL };
    PyGObject *self, *style;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItem.set_style", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasStyle_Type, &style))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->set_style)
        iface->set_style(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_STYLE(style->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.set_style not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_is_visible(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.is_visible", kwlist, &PyGooCanvasItem_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->is_visible)
        ret = iface->is_visible(GOO_CANVAS_ITEM(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.is_visible not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_get_requested_height(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "cr", "width", NULL };
    PyGObject *self;
    double width, ret;
    PycairoContext *cr;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!d:GooCanvasItem.get_requested_height", kwlist, &PyGooCanvasItem_Type, &self, &PycairoContext_Type, &cr, &width))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_requested_height)
        ret = iface->get_requested_height(GOO_CANVAS_ITEM(self->obj), cr->ctx, width);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_requested_height not implemented");
        return NULL;
    }
    return PyFloat_FromDouble(ret);
}

static PyObject *
_wrap_GooCanvasItem__do_get_model(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GooCanvasItemModel *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItem.get_model", kwlist, &PyGooCanvasItem_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->get_model)
        ret = iface->get_model(GOO_CANVAS_ITEM(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.get_model not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItem__do_set_model(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "model", NULL };
    PyGObject *self, *model;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItem.set_model", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItemModel_Type, &model))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->set_model)
        iface->set_model(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM_MODEL(model->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.set_model not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItem__do_enter_notify_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.enter_notify_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->enter_notify_event)
        ret = iface->enter_notify_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventCrossing *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.enter_notify_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_leave_notify_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.leave_notify_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->leave_notify_event)
        ret = iface->leave_notify_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventCrossing *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.leave_notify_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_motion_notify_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.motion_notify_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->motion_notify_event)
        ret = iface->motion_notify_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventMotion *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.motion_notify_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_button_press_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.button_press_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->button_press_event)
        ret = iface->button_press_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventButton *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.button_press_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_button_release_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.button_release_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->button_release_event)
        ret = iface->button_release_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventButton *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.button_release_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_focus_in_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.focus_in_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->focus_in_event)
        ret = iface->focus_in_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventFocus *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.focus_in_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_focus_out_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.focus_out_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->focus_out_event)
        ret = iface->focus_out_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventFocus *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.focus_out_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_key_press_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.key_press_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->key_press_event)
        ret = iface->key_press_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventKey *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.key_press_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_key_release_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.key_release_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->key_release_event)
        ret = iface->key_release_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventKey *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.key_release_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItem__do_grab_broken_event(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemIface *iface;
    static char *kwlist[] = { "self", "target", "event", NULL };
    PyGObject *self, *target;
    GdkEvent *event = NULL;
    int ret;
    PyObject *py_event;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!O:GooCanvasItem.grab_broken_event", kwlist, &PyGooCanvasItem_Type, &self, &PyGooCanvasItem_Type, &target, &py_event))
        return NULL;
    if (pyg_boxed_check(py_event, GDK_TYPE_EVENT))
        event = pyg_boxed_get(py_event, GdkEvent);
    else {
        PyErr_SetString(PyExc_TypeError, "event should be a GdkEvent");
        return NULL;
    }
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM);
    if (iface->grab_broken_event)
        ret = iface->grab_broken_event(GOO_CANVAS_ITEM(self->obj), GOO_CANVAS_ITEM(target->obj), (GdkEventGrabBroken *)event);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItem.grab_broken_event not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static const PyMethodDef _PyGooCanvasItem_methods[] = {
    { "get_n_children", (PyCFunction)_wrap_goo_canvas_item_get_n_children, METH_NOARGS,
      NULL },
    { "get_child", (PyCFunction)_wrap_goo_canvas_item_get_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_child", (PyCFunction)_wrap_goo_canvas_item_find_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_child", (PyCFunction)_wrap_goo_canvas_item_add_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "move_child", (PyCFunction)_wrap_goo_canvas_item_move_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_child", (PyCFunction)_wrap_goo_canvas_item_remove_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_child_properties", (PyCFunction)_wrap_goo_canvas_item_get_child_properties, METH_VARARGS,
      NULL },
    { "set_child_properties", (PyCFunction)_wrap_goo_canvas_item_set_child_properties, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_transform_for_child", (PyCFunction)_wrap_goo_canvas_item_get_transform_for_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_canvas", (PyCFunction)_wrap_goo_canvas_item_get_canvas, METH_NOARGS,
      NULL },
    { "set_canvas", (PyCFunction)_wrap_goo_canvas_item_set_canvas, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_goo_canvas_item_get_parent, METH_NOARGS,
      NULL },
    { "set_parent", (PyCFunction)_wrap_goo_canvas_item_set_parent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove", (PyCFunction)_wrap_goo_canvas_item_remove, METH_NOARGS,
      NULL },
    { "is_container", (PyCFunction)_wrap_goo_canvas_item_is_container, METH_NOARGS,
      NULL },
    { "raise_", (PyCFunction)_wrap_goo_canvas_item_raise, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lower", (PyCFunction)_wrap_goo_canvas_item_lower, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_transform", (PyCFunction)_wrap_goo_canvas_item_get_transform, METH_NOARGS,
      NULL },
    { "set_transform", (PyCFunction)_wrap_goo_canvas_item_set_transform, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_simple_transform", (PyCFunction)_wrap_goo_canvas_item_set_simple_transform, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "translate", (PyCFunction)_wrap_goo_canvas_item_translate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "scale", (PyCFunction)_wrap_goo_canvas_item_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rotate", (PyCFunction)_wrap_goo_canvas_item_rotate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "skew_x", (PyCFunction)_wrap_goo_canvas_item_skew_x, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "skew_y", (PyCFunction)_wrap_goo_canvas_item_skew_y, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_style", (PyCFunction)_wrap_goo_canvas_item_get_style, METH_NOARGS,
      NULL },
    { "set_style", (PyCFunction)_wrap_goo_canvas_item_set_style, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "animate", (PyCFunction)_wrap_goo_canvas_item_animate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stop_animation", (PyCFunction)_wrap_goo_canvas_item_stop_animation, METH_NOARGS,
      NULL },
    { "get_bounds", (PyCFunction)_wrap_goo_canvas_item_get_bounds, METH_NOARGS,
      NULL },
    { "get_items_at", (PyCFunction)_wrap_goo_canvas_item_get_items_at, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "is_visible", (PyCFunction)_wrap_goo_canvas_item_is_visible, METH_NOARGS,
      NULL },
    { "get_model", (PyCFunction)_wrap_goo_canvas_item_get_model, METH_NOARGS,
      NULL },
    { "set_model", (PyCFunction)_wrap_goo_canvas_item_set_model, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "request_update", (PyCFunction)_wrap_goo_canvas_item_request_update, METH_NOARGS,
      NULL },
    { "ensure_updated", (PyCFunction)_wrap_goo_canvas_item_ensure_updated, METH_NOARGS,
      NULL },
    { "update", (PyCFunction)_wrap_goo_canvas_item_update, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "paint", (PyCFunction)_wrap_goo_canvas_item_paint, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_requested_area", (PyCFunction)_wrap_goo_canvas_item_get_requested_area, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_requested_height", (PyCFunction)_wrap_goo_canvas_item_get_requested_height, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "allocate_area", (PyCFunction)_wrap_goo_canvas_item_allocate_area, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_child_property", (PyCFunction)_wrap_goo_canvas_item_find_child_property, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "list_child_properties", (PyCFunction)_wrap_goo_canvas_item_list_child_properties, METH_NOARGS|METH_CLASS,
      NULL },
    { "install_child_property", (PyCFunction)_wrap_goo_canvas_item_install_child_property, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_canvas", (PyCFunction)_wrap_GooCanvasItem__do_get_canvas, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_canvas", (PyCFunction)_wrap_GooCanvasItem__do_set_canvas, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_children", (PyCFunction)_wrap_GooCanvasItem__do_get_n_children, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_child", (PyCFunction)_wrap_GooCanvasItem__do_get_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_request_update", (PyCFunction)_wrap_GooCanvasItem__do_request_update, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_add_child", (PyCFunction)_wrap_GooCanvasItem__do_add_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_move_child", (PyCFunction)_wrap_GooCanvasItem__do_move_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_child", (PyCFunction)_wrap_GooCanvasItem__do_remove_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_transform_for_child", (PyCFunction)_wrap_GooCanvasItem__do_get_transform_for_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_parent", (PyCFunction)_wrap_GooCanvasItem__do_get_parent, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_parent", (PyCFunction)_wrap_GooCanvasItem__do_set_parent, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_bounds", (PyCFunction)_wrap_GooCanvasItem__do_get_bounds, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_update", (PyCFunction)_wrap_GooCanvasItem__do_update, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_paint", (PyCFunction)_wrap_GooCanvasItem__do_paint, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_requested_area", (PyCFunction)_wrap_GooCanvasItem__do_get_requested_area, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_allocate_area", (PyCFunction)_wrap_GooCanvasItem__do_allocate_area, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_transform", (PyCFunction)_wrap_GooCanvasItem__do_get_transform, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_transform", (PyCFunction)_wrap_GooCanvasItem__do_set_transform, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_style", (PyCFunction)_wrap_GooCanvasItem__do_get_style, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_style", (PyCFunction)_wrap_GooCanvasItem__do_set_style, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_is_visible", (PyCFunction)_wrap_GooCanvasItem__do_is_visible, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_requested_height", (PyCFunction)_wrap_GooCanvasItem__do_get_requested_height, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_model", (PyCFunction)_wrap_GooCanvasItem__do_get_model, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_model", (PyCFunction)_wrap_GooCanvasItem__do_set_model, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_enter_notify_event", (PyCFunction)_wrap_GooCanvasItem__do_enter_notify_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_leave_notify_event", (PyCFunction)_wrap_GooCanvasItem__do_leave_notify_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_motion_notify_event", (PyCFunction)_wrap_GooCanvasItem__do_motion_notify_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_button_press_event", (PyCFunction)_wrap_GooCanvasItem__do_button_press_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_button_release_event", (PyCFunction)_wrap_GooCanvasItem__do_button_release_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_focus_in_event", (PyCFunction)_wrap_GooCanvasItem__do_focus_in_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_focus_out_event", (PyCFunction)_wrap_GooCanvasItem__do_focus_out_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_key_press_event", (PyCFunction)_wrap_GooCanvasItem__do_key_press_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_key_release_event", (PyCFunction)_wrap_GooCanvasItem__do_key_release_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_grab_broken_event", (PyCFunction)_wrap_GooCanvasItem__do_grab_broken_event, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGooCanvasItem_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.Item",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGooCanvasItem_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static GooCanvas*
_wrap_GooCanvasItem__proxy_do_get_canvas(GooCanvasItem *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GooCanvas* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_canvas");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvas*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItem__proxy_do_set_canvas(GooCanvasItem *self, GooCanvas*canvas)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_canvas = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (canvas)
        py_canvas = pygobject_new((GObject *) canvas);
    else {
        Py_INCREF(Py_None);
        py_canvas = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_canvas);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_canvas");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gint
_wrap_GooCanvasItem__proxy_do_get_n_children(GooCanvasItem *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gint retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_children");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GooCanvasItem*
_wrap_GooCanvasItem__proxy_do_get_child(GooCanvasItem *self, gint child_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child_num;
    GooCanvasItem* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_child_num = PyInt_FromLong(child_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_child_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasItem*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItem__proxy_do_request_update(GooCanvasItem *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_request_update");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItem__proxy_do_add_child(GooCanvasItem *self, GooCanvasItem*child, gint position)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child = NULL;
    PyObject *py_position;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (child)
        py_child = pygobject_new((GObject *) child);
    else {
        Py_INCREF(Py_None);
        py_child = Py_None;
    }
    py_position = PyInt_FromLong(position);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_child);
    PyTuple_SET_ITEM(py_args, 1, py_position);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItem__proxy_do_move_child(GooCanvasItem *self, gint old_position, gint new_position)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_old_position;
    PyObject *py_new_position;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_old_position = PyInt_FromLong(old_position);
    py_new_position = PyInt_FromLong(new_position);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_old_position);
    PyTuple_SET_ITEM(py_args, 1, py_new_position);
    
    py_method = PyObject_GetAttrString(py_self, "do_move_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItem__proxy_do_remove_child(GooCanvasItem *self, gint child_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child_num;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_child_num = PyInt_FromLong(child_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_child_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GooCanvasItem*
_wrap_GooCanvasItem__proxy_do_get_parent(GooCanvasItem *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GooCanvasItem* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_parent");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasItem*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItem__proxy_do_set_parent(GooCanvasItem *self, GooCanvasItem*parent)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_parent = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (parent)
        py_parent = pygobject_new((GObject *) parent);
    else {
        Py_INCREF(Py_None);
        py_parent = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_parent);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_parent");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
#line 606 "goocanvas.override"
static void
_wrap_GooCanvasItem__proxy_do_get_bounds(GooCanvasItem *self, GooCanvasBounds *bounds)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_bounds;

    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }

    py_bounds = PyObject_CallMethod(py_self, "do_get_bounds", "");
    if (py_bounds)
        *bounds = ((PyGooCanvasBounds *) py_bounds)->bounds;
    else
        PyErr_Print();
    Py_DECREF(py_self);
    Py_XDECREF(py_bounds);
    pyg_gil_state_release(__py_state);
}
#line 6006 "goocanvas.c"


#line 633 "goocanvas.override"
static void
_wrap_GooCanvasItem__proxy_do_update(GooCanvasItem *self, gboolean entire_tree,
                                     cairo_t *cr, GooCanvasBounds *bounds)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_bounds;

    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }

    py_bounds = PyObject_CallMethod(py_self, "do_update", "iN",
                                    entire_tree,
                                    PycairoContext_FromContext(cairo_reference(cr), NULL, NULL));
    if (py_bounds)
        *bounds = ((PyGooCanvasBounds *) py_bounds)->bounds;
    else
        PyErr_Print();
    Py_DECREF(py_self);
    Py_XDECREF(py_bounds);
    pyg_gil_state_release(__py_state);
}

#line 6039 "goocanvas.c"


static void
_wrap_GooCanvasItem__proxy_do_paint(GooCanvasItem *self, cairo_t*cr, const GooCanvasBounds*bounds, gdouble scale)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cr;
    PyObject *py_bounds;
    PyObject *py_scale;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    py_bounds = pygoo_canvas_bounds_new(bounds);
    py_scale = PyFloat_FromDouble(scale);
    
    py_args = PyTuple_New(3);
    PyTuple_SET_ITEM(py_args, 0, py_cr);
    PyTuple_SET_ITEM(py_args, 1, py_bounds);
    PyTuple_SET_ITEM(py_args, 2, py_scale);
    
    py_method = PyObject_GetAttrString(py_self, "do_paint");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gboolean
_wrap_GooCanvasItem__proxy_do_get_requested_area(GooCanvasItem *self, cairo_t*cr, GooCanvasBounds*requested_area)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cr;
    PyObject *py_requested_area;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    py_requested_area = pygoo_canvas_bounds_new(requested_area);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_cr);
    PyTuple_SET_ITEM(py_args, 1, py_requested_area);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_requested_area");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItem__proxy_do_allocate_area(GooCanvasItem *self, cairo_t*cr, const GooCanvasBounds*requested_area, const GooCanvasBounds*allocated_area, gdouble x_offset, gdouble y_offset)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cr;
    PyObject *py_requested_area;
    PyObject *py_allocated_area;
    PyObject *py_x_offset;
    PyObject *py_y_offset;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    py_requested_area = pygoo_canvas_bounds_new(requested_area);
    py_allocated_area = pygoo_canvas_bounds_new(allocated_area);
    py_x_offset = PyFloat_FromDouble(x_offset);
    py_y_offset = PyFloat_FromDouble(y_offset);
    
    py_args = PyTuple_New(5);
    PyTuple_SET_ITEM(py_args, 0, py_cr);
    PyTuple_SET_ITEM(py_args, 1, py_requested_area);
    PyTuple_SET_ITEM(py_args, 2, py_allocated_area);
    PyTuple_SET_ITEM(py_args, 3, py_x_offset);
    PyTuple_SET_ITEM(py_args, 4, py_y_offset);
    
    py_method = PyObject_GetAttrString(py_self, "do_allocate_area");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GooCanvasStyle*
_wrap_GooCanvasItem__proxy_do_get_style(GooCanvasItem *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GooCanvasStyle* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_style");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasStyle*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItem__proxy_do_set_style(GooCanvasItem *self, GooCanvasStyle*style)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_style = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (style)
        py_style = pygobject_new((GObject *) style);
    else {
        Py_INCREF(Py_None);
        py_style = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_style);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_style");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gboolean
_wrap_GooCanvasItem__proxy_do_is_visible(GooCanvasItem *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_is_visible");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gdouble
_wrap_GooCanvasItem__proxy_do_get_requested_height(GooCanvasItem *self, cairo_t*cr, gdouble width)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_cr;
    PyObject *py_width;
    gdouble retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    py_cr = PycairoContext_FromContext(cairo_reference(cr), NULL, NULL);
    py_width = PyFloat_FromDouble(width);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_cr);
    PyTuple_SET_ITEM(py_args, 1, py_width);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_requested_height");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "d", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXFLOAT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GooCanvasItemModel*
_wrap_GooCanvasItem__proxy_do_get_model(GooCanvasItem *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GooCanvasItemModel* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_model");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasItemModel*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItem__proxy_do_set_model(GooCanvasItem *self, GooCanvasItemModel*model)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_model = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (model)
        py_model = pygobject_new((GObject *) model);
    else {
        Py_INCREF(Py_None);
        py_model = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_model);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_model");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static gboolean
_wrap_GooCanvasItem__proxy_do_enter_notify_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventCrossing*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_enter_notify_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_leave_notify_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventCrossing*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_leave_notify_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_motion_notify_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventMotion*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_motion_notify_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_button_press_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventButton*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_button_press_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_button_release_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventButton*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_button_release_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_focus_in_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventFocus*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_focus_in_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_focus_out_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventFocus*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_focus_out_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_key_press_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventKey*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_key_press_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_key_release_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventKey*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_key_release_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static gboolean
_wrap_GooCanvasItem__proxy_do_grab_broken_event(GooCanvasItem *self, GooCanvasItem*target, GdkEventGrabBroken*event)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_target = NULL;
    PyObject *py_event;
    gboolean retval;
    PyObject *py_main_retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    if (target)
        py_target = pygobject_new((GObject *) target);
    else {
        Py_INCREF(Py_None);
        py_target = Py_None;
    }
    py_event = pyg_boxed_new(GDK_TYPE_EVENT, event, FALSE, FALSE);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_target);
    PyTuple_SET_ITEM(py_args, 1, py_event);
    
    py_method = PyObject_GetAttrString(py_self, "do_grab_broken_event");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "O", &py_main_retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return FALSE;
    }
    
    retval = PyObject_IsTrue(py_main_retval)? TRUE : FALSE;
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}

static void
__GooCanvasItem__interface_init(GooCanvasItemIface *iface, PyTypeObject *pytype)
{
    GooCanvasItemIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_canvas") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_canvas = _wrap_GooCanvasItem__proxy_do_get_canvas;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_canvas = parent_iface->get_canvas;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_canvas") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_canvas = _wrap_GooCanvasItem__proxy_do_set_canvas;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_canvas = parent_iface->set_canvas;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_children") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_children = _wrap_GooCanvasItem__proxy_do_get_n_children;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_children = parent_iface->get_n_children;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_child = _wrap_GooCanvasItem__proxy_do_get_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_child = parent_iface->get_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_request_update") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->request_update = _wrap_GooCanvasItem__proxy_do_request_update;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->request_update = parent_iface->request_update;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_add_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->add_child = _wrap_GooCanvasItem__proxy_do_add_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->add_child = parent_iface->add_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_move_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->move_child = _wrap_GooCanvasItem__proxy_do_move_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->move_child = parent_iface->move_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_remove_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->remove_child = _wrap_GooCanvasItem__proxy_do_remove_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->remove_child = parent_iface->remove_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_parent") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_parent = _wrap_GooCanvasItem__proxy_do_get_parent;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_parent = parent_iface->get_parent;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_parent") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_parent = _wrap_GooCanvasItem__proxy_do_set_parent;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_parent = parent_iface->set_parent;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_bounds") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_bounds = _wrap_GooCanvasItem__proxy_do_get_bounds;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_bounds = parent_iface->get_bounds;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_update") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->update = _wrap_GooCanvasItem__proxy_do_update;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->update = parent_iface->update;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_paint") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->paint = _wrap_GooCanvasItem__proxy_do_paint;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->paint = parent_iface->paint;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_requested_area") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_requested_area = _wrap_GooCanvasItem__proxy_do_get_requested_area;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_requested_area = parent_iface->get_requested_area;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_allocate_area") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->allocate_area = _wrap_GooCanvasItem__proxy_do_allocate_area;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->allocate_area = parent_iface->allocate_area;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_style") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_style = _wrap_GooCanvasItem__proxy_do_get_style;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_style = parent_iface->get_style;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_style") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_style = _wrap_GooCanvasItem__proxy_do_set_style;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_style = parent_iface->set_style;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_is_visible") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->is_visible = _wrap_GooCanvasItem__proxy_do_is_visible;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->is_visible = parent_iface->is_visible;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_requested_height") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_requested_height = _wrap_GooCanvasItem__proxy_do_get_requested_height;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_requested_height = parent_iface->get_requested_height;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_model") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_model = _wrap_GooCanvasItem__proxy_do_get_model;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_model = parent_iface->get_model;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_model") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_model = _wrap_GooCanvasItem__proxy_do_set_model;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_model = parent_iface->set_model;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_enter_notify_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->enter_notify_event = _wrap_GooCanvasItem__proxy_do_enter_notify_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->enter_notify_event = parent_iface->enter_notify_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_leave_notify_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->leave_notify_event = _wrap_GooCanvasItem__proxy_do_leave_notify_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->leave_notify_event = parent_iface->leave_notify_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_motion_notify_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->motion_notify_event = _wrap_GooCanvasItem__proxy_do_motion_notify_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->motion_notify_event = parent_iface->motion_notify_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_button_press_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->button_press_event = _wrap_GooCanvasItem__proxy_do_button_press_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->button_press_event = parent_iface->button_press_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_button_release_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->button_release_event = _wrap_GooCanvasItem__proxy_do_button_release_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->button_release_event = parent_iface->button_release_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_focus_in_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->focus_in_event = _wrap_GooCanvasItem__proxy_do_focus_in_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->focus_in_event = parent_iface->focus_in_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_focus_out_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->focus_out_event = _wrap_GooCanvasItem__proxy_do_focus_out_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->focus_out_event = parent_iface->focus_out_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_key_press_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->key_press_event = _wrap_GooCanvasItem__proxy_do_key_press_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->key_press_event = parent_iface->key_press_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_key_release_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->key_release_event = _wrap_GooCanvasItem__proxy_do_key_release_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->key_release_event = parent_iface->key_release_event;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_grab_broken_event") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->grab_broken_event = _wrap_GooCanvasItem__proxy_do_grab_broken_event;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->grab_broken_event = parent_iface->grab_broken_event;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GooCanvasItem__iinfo = {
    (GInterfaceInitFunc) __GooCanvasItem__interface_init,
    NULL,
    NULL
};


/* ----------- GooCanvasItemModel ----------- */

static PyObject *
_wrap_goo_canvas_item_model_get_n_children(PyGObject *self)
{
    int ret;

    
    ret = goo_canvas_item_model_get_n_children(GOO_CANVAS_ITEM_MODEL(self->obj));
    
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_goo_canvas_item_model_get_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child_num", NULL };
    int child_num;
    GooCanvasItemModel *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GooCanvasItemModel.get_child", kwlist, &child_num))
        return NULL;
    
    ret = goo_canvas_item_model_get_child(GOO_CANVAS_ITEM_MODEL(self->obj), child_num);
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_model_add_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child", "position", NULL };
    PyGObject *child;
    int position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItemModel.add_child", kwlist, &PyGooCanvasItemModel_Type, &child, &position))
        return NULL;
    
    goo_canvas_item_model_add_child(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_ITEM_MODEL(child->obj), position);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_move_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "old_position", "new_position", NULL };
    int old_position, new_position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ii:GooCanvasItemModel.move_child", kwlist, &old_position, &new_position))
        return NULL;
    
    goo_canvas_item_model_move_child(GOO_CANVAS_ITEM_MODEL(self->obj), old_position, new_position);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_remove_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child_num", NULL };
    int child_num;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"i:GooCanvasItemModel.remove_child", kwlist, &child_num))
        return NULL;
    
    goo_canvas_item_model_remove_child(GOO_CANVAS_ITEM_MODEL(self->obj), child_num);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_find_child(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "child", NULL };
    PyGObject *child;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.find_child", kwlist, &PyGooCanvasItemModel_Type, &child))
        return NULL;
    
    ret = goo_canvas_item_model_find_child(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_ITEM_MODEL(child->obj));
    
    return PyInt_FromLong(ret);
}

#line 1084 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_model_get_child_properties(PyGObject *self, PyObject *args)
{
    GObjectClass        *class;
    GooCanvasItemModel  *parent, *child;
    PyGObject           *pychild;
    int                 i, len;
    gint                c_pos;
    PyObject            *tuple;

    if ((len = PyTuple_Size(args)) < 1) {
        PyErr_SetString(PyExc_TypeError, "requires at least one argument");
        return NULL;
    }

    pychild = (PyGObject*)PyTuple_GetItem(args, 0);

    if (!pygobject_check(pychild, &PyGooCanvasItemModel_Type)) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument should be a GooCanvasItemModel");
        return NULL;
    }

    parent = GOO_CANVAS_ITEM_MODEL(self->obj);
    child = GOO_CANVAS_ITEM_MODEL(pychild->obj);

    c_pos = goo_canvas_item_model_find_child(parent, child);
    if (c_pos == -1) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument must be a child");
        return NULL;
    }

    tuple = PyTuple_New(len-1);

    class = G_OBJECT_GET_CLASS(self->obj);

    for (i = 1; i < len; i++) {
        PyObject *py_property = PyTuple_GetItem(args, i);
        gchar *property_name;
        GParamSpec *pspec;
        GValue value = { 0 };
        PyObject *item;

        if (!PyString_Check(py_property)) {
            PyErr_SetString(PyExc_TypeError,
                            "Expected string argument for property.");
            return NULL;
        }

        property_name = PyString_AsString(py_property);
        pspec = goo_canvas_item_model_class_find_child_property (class, property_name);

        if (!pspec) {
	    PyErr_Format(PyExc_TypeError,
		         "object of type `%s' does not have child property `%s'",
		         g_type_name(G_OBJECT_TYPE(self->obj)), property_name);
    	return NULL;
        }
        if (!(pspec->flags & G_PARAM_READABLE)) {
	    PyErr_Format(PyExc_TypeError, "property %s is not readable",
		        property_name);
	    return NULL;
        }
        g_value_init(&value, G_PARAM_SPEC_VALUE_TYPE(pspec));
        GooCanvasItemModelIface *iface;

	    iface = g_type_interface_peek (class, GOO_TYPE_CANVAS_ITEM_MODEL);
	    iface->get_child_property ((GooCanvasItemModel*) parent,
		                           (GooCanvasItemModel*) child,
                                    pspec->param_id, &value, pspec);

        item = pyg_value_as_pyobject(&value, TRUE);
        PyTuple_SetItem(tuple, i-1, item);

        g_value_unset(&value);
    }

    return tuple;
}

#line 7891 "goocanvas.c"


#line 1003 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_model_set_child_properties(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    PyGObject           *pychild;
    GObjectClass        *class;
    GooCanvasItemModel  *parent, *child;
    Py_ssize_t          pos;
    int                 len;
    gint                c_pos;
    PyObject            *value;
    PyObject            *key;

    if ((len = PyTuple_Size(args)) < 1) {
        PyErr_SetString(PyExc_TypeError, "requires at least one argument");
        return NULL;
    }
    pychild = (PyGObject*)PyTuple_GetItem(args, 0);

    if (!pygobject_check(pychild, &PyGooCanvasItemModel_Type)) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument should be a GooCanvasItemModel");
        return NULL;
    }

    parent = GOO_CANVAS_ITEM_MODEL(self->obj);
    child = GOO_CANVAS_ITEM_MODEL(pychild->obj);

    c_pos = goo_canvas_item_model_find_child(parent, child);
    if (c_pos == -1) {
        PyErr_SetString(PyExc_TypeError,
                        "first argument must be a child");
        return NULL;
    }

    class = G_OBJECT_GET_CLASS(self->obj);

    g_object_freeze_notify (G_OBJECT(self->obj));
    pos = 0;

    while (kwargs && PyDict_Next (kwargs, &pos, &key, &value)) {
    gchar *key_str = PyString_AsString (key);
    GParamSpec *pspec;
    GValue gvalue ={ 0, };

    pspec = goo_canvas_item_model_class_find_child_property (class, key_str);
    if (!pspec) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "object `%s' doesn't support child property `%s'",
		       g_type_name(G_OBJECT_TYPE(pychild->obj)), key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    return NULL;
	}

	g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
	if (pyg_value_from_pyobject(&gvalue, value)) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "could not convert value for property `%s'", key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    return NULL;
	}
	GooCanvasItemModelIface *iface;

	iface = g_type_interface_peek (class, GOO_TYPE_CANVAS_ITEM_MODEL);
	iface->set_child_property ((GooCanvasItemModel*) parent,
	                           (GooCanvasItemModel*) child,
	                           pspec->param_id, &gvalue, pspec);
	g_value_unset(&gvalue);
    }

    g_object_thaw_notify (G_OBJECT(self->obj));

    Py_INCREF(Py_None);
    return Py_None;
}

#line 7974 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_model_get_parent(PyGObject *self)
{
    GooCanvasItemModel *ret;

    
    ret = goo_canvas_item_model_get_parent(GOO_CANVAS_ITEM_MODEL(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_model_set_parent(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "parent", NULL };
    PyGObject *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.set_parent", kwlist, &PyGooCanvasItemModel_Type, &parent))
        return NULL;
    
    goo_canvas_item_model_set_parent(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_ITEM_MODEL(parent->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_remove(PyGObject *self)
{
    
    goo_canvas_item_model_remove(GOO_CANVAS_ITEM_MODEL(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_is_container(PyGObject *self)
{
    int ret;

    
    ret = goo_canvas_item_model_is_container(GOO_CANVAS_ITEM_MODEL(self->obj));
    
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_goo_canvas_item_model_raise(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "above", NULL };
    PyGObject *above;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.raise", kwlist, &PyGooCanvasItemModel_Type, &above))
        return NULL;
    
    goo_canvas_item_model_raise(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_ITEM_MODEL(above->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_lower(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "below", NULL };
    PyGObject *below;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.lower", kwlist, &PyGooCanvasItemModel_Type, &below))
        return NULL;
    
    goo_canvas_item_model_lower(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_ITEM_MODEL(below->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1566 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_model_get_transform(PyGObject *self)
{
    PyObject        *matrix;
    cairo_matrix_t  transform = {0,};
    gboolean        res;

    res = goo_canvas_item_model_get_transform(GOO_CANVAS_ITEM_MODEL(self->obj),
                                              &transform);

    if (!res) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    matrix = PycairoMatrix_FromMatrix(&transform);
    return matrix;
}

#line 8076 "goocanvas.c"


static PyObject *
_wrap_goo_canvas_item_model_set_transform(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "matrix", NULL };
    PyObject *py_matrix;
    cairo_matrix_t *matrix;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:GooCanvasItemModel.set_transform", kwlist, &py_matrix))
        return NULL;
    matrix = &((PycairoMatrix*)(py_matrix))->matrix;
    
    goo_canvas_item_model_set_transform(GOO_CANVAS_ITEM_MODEL(self->obj), matrix);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_set_simple_transform(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "scale", "rotation", NULL };
    double x, y, scale, rotation;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dddd:GooCanvasItemModel.set_simple_transform", kwlist, &x, &y, &scale, &rotation))
        return NULL;
    
    goo_canvas_item_model_set_simple_transform(GOO_CANVAS_ITEM_MODEL(self->obj), x, y, scale, rotation);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_translate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "tx", "ty", NULL };
    double tx, ty;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:GooCanvasItemModel.translate", kwlist, &tx, &ty))
        return NULL;
    
    goo_canvas_item_model_translate(GOO_CANVAS_ITEM_MODEL(self->obj), tx, ty);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_scale(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "sx", "sy", NULL };
    double sx, sy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"dd:GooCanvasItemModel.scale", kwlist, &sx, &sy))
        return NULL;
    
    goo_canvas_item_model_scale(GOO_CANVAS_ITEM_MODEL(self->obj), sx, sy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_rotate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "degrees", "cx", "cy", NULL };
    double degrees, cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GooCanvasItemModel.rotate", kwlist, &degrees, &cx, &cy))
        return NULL;
    
    goo_canvas_item_model_rotate(GOO_CANVAS_ITEM_MODEL(self->obj), degrees, cx, cy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_skew_x(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "degrees", "cx", "cy", NULL };
    double degrees, cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GooCanvasItemModel.skew_x", kwlist, &degrees, &cx, &cy))
        return NULL;
    
    goo_canvas_item_model_skew_x(GOO_CANVAS_ITEM_MODEL(self->obj), degrees, cx, cy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_skew_y(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "degrees", "cx", "cy", NULL };
    double degrees, cx, cy;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddd:GooCanvasItemModel.skew_y", kwlist, &degrees, &cx, &cy))
        return NULL;
    
    goo_canvas_item_model_skew_y(GOO_CANVAS_ITEM_MODEL(self->obj), degrees, cx, cy);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_get_style(PyGObject *self)
{
    GooCanvasStyle *ret;

    
    ret = goo_canvas_item_model_get_style(GOO_CANVAS_ITEM_MODEL(self->obj));
    
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_goo_canvas_item_model_set_style(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "style", NULL };
    PyGObject *style;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.set_style", kwlist, &PyGooCanvasStyle_Type, &style))
        return NULL;
    
    goo_canvas_item_model_set_style(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_STYLE(style->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_animate(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "x", "y", "scale", "degrees", "absolute", "duration", "step_time", "type", NULL };
    int absolute, duration, step_time;
    double x, y, scale, degrees;
    GooCanvasAnimateType type;
    PyObject *py_type = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"ddddiiiO:GooCanvasItemModel.animate", kwlist, &x, &y, &scale, &degrees, &absolute, &duration, &step_time, &py_type))
        return NULL;
    if (pyg_enum_get_value(GOO_TYPE_CANVAS_ANIMATE_TYPE, py_type, (gpointer)&type))
        return NULL;
    
    goo_canvas_item_model_animate(GOO_CANVAS_ITEM_MODEL(self->obj), x, y, scale, degrees, absolute, duration, step_time, type);
    
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_goo_canvas_item_model_stop_animation(PyGObject *self)
{
    
    goo_canvas_item_model_stop_animation(GOO_CANVAS_ITEM_MODEL(self->obj));
    
    Py_INCREF(Py_None);
    return Py_None;
}

#line 1208 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_model_find_child_property (PyObject *cls,
                                                 PyObject *args,
                                                 PyObject *kwargs)
{
    static char         *kwlist[] = { "property", NULL };
    GObjectClass        *klass;
    GType               itype;
    const gchar         *prop_name;
    GParamSpec          *pspec;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
                                     "s:item_model_class_find_child_property",
                                     kwlist,
                                     &prop_name))
        return NULL;

    if ((itype = pyg_type_from_object(cls)) == 0)
	return NULL;

    klass = g_type_class_ref(itype);

    if (!klass) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not get a reference to type class");
	return NULL;
    }

    pspec = goo_canvas_item_model_class_find_child_property (klass, prop_name);

    if(!pspec){
        PyErr_Format(PyExc_KeyError,
                        "object %s does not support property %s",
                        g_type_name(itype), prop_name);
	return NULL;
    }
    return pyg_param_spec_new(pspec);
}

#line 8283 "goocanvas.c"


#line 1291 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_model_list_child_properties (PyObject *cls,
                                                   PyObject *args,
                                                   PyObject *kwargs)
{
    GParamSpec      **specs;
    PyObject        *list;
    GType           itype;
    GObjectClass    *klass;
    guint           nprops;
    guint           i;

    if ((itype = pyg_type_from_object(cls)) == 0)
	return NULL;

    klass = g_type_class_ref(itype);
    if (!klass) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not get a reference to type class");
	return NULL;
    }

    specs = goo_canvas_item_model_class_list_child_properties(klass, &nprops);
    list = PyList_New(nprops);
    if (list == NULL) {
	g_free(specs);
	g_type_class_unref(klass);
	return NULL;
    }

    for (i = 0; i < nprops; i++) {
	PyList_SetItem(list, i, pyg_param_spec_new(specs[i]));
    }

    g_free(specs);
    g_type_class_unref(klass);

    return list;
}

#line 8327 "goocanvas.c"


#line 1385 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_item_model_install_child_property (PyObject *cls,
                                                    PyObject *args,
                                                    PyObject* kwargs)
{
    static char         *kwlist[] = { "property_id", "pspec", NULL };
    PyObject            *property;
    GType               itype;
    GObjectClass        *klass;
    guint               property_id;
    GParamSpec          *pspec;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,
				     "iO:item_model_class_install_child_property",
				     kwlist,
				     &property_id, &property))
	return NULL;


    if ((itype = pyg_type_from_object(cls)) == 0)
	return NULL;

    klass = g_type_class_ref(itype);
    if (!klass) {
	PyErr_SetString(PyExc_RuntimeError,
			"could not get a reference to type class");
	return NULL;
    }

    pspec = pyg_param_spec_from_object(property);
    if(!pspec) {
	g_type_class_unref(klass);
	return NULL;
    }

    if (goo_canvas_item_model_class_find_child_property(G_OBJECT_CLASS(klass), pspec->name)) {
	PyErr_Format(PyExc_TypeError,
		     "there is already a '%s' property installed", pspec->name);
	g_type_class_unref(klass);
	return NULL;
    }

    goo_canvas_item_model_class_install_child_property(klass, property_id, pspec);

    g_type_class_unref(klass);

    Py_INCREF(Py_None);
    return Py_None;
}

#line 8381 "goocanvas.c"


static PyObject *
_wrap_GooCanvasItemModel__do_get_n_children(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    int ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.get_n_children", kwlist, &PyGooCanvasItemModel_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->get_n_children)
        ret = iface->get_n_children(GOO_CANVAS_ITEM_MODEL(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.get_n_children not implemented");
        return NULL;
    }
    return PyInt_FromLong(ret);
}

static PyObject *
_wrap_GooCanvasItemModel__do_get_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "child_num", NULL };
    PyGObject *self;
    int child_num;
    GooCanvasItemModel *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItemModel.get_child", kwlist, &PyGooCanvasItemModel_Type, &self, &child_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->get_child)
        ret = iface->get_child(GOO_CANVAS_ITEM_MODEL(self->obj), child_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.get_child not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItemModel__do_add_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "child", "position", NULL };
    PyGObject *self, *child;
    int position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!i:GooCanvasItemModel.add_child", kwlist, &PyGooCanvasItemModel_Type, &self, &PyGooCanvasItemModel_Type, &child, &position))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->add_child)
        iface->add_child(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_ITEM_MODEL(child->obj), position);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.add_child not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_move_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "old_position", "new_position", NULL };
    PyGObject *self;
    int old_position, new_position;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:GooCanvasItemModel.move_child", kwlist, &PyGooCanvasItemModel_Type, &self, &old_position, &new_position))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->move_child)
        iface->move_child(GOO_CANVAS_ITEM_MODEL(self->obj), old_position, new_position);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.move_child not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_remove_child(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "child_num", NULL };
    PyGObject *self;
    int child_num;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItemModel.remove_child", kwlist, &PyGooCanvasItemModel_Type, &self, &child_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->remove_child)
        iface->remove_child(GOO_CANVAS_ITEM_MODEL(self->obj), child_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.remove_child not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_get_parent(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GooCanvasItemModel *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.get_parent", kwlist, &PyGooCanvasItemModel_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->get_parent)
        ret = iface->get_parent(GOO_CANVAS_ITEM_MODEL(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.get_parent not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItemModel__do_set_parent(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "parent", NULL };
    PyGObject *self, *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItemModel.set_parent", kwlist, &PyGooCanvasItemModel_Type, &self, &PyGooCanvasItemModel_Type, &parent))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->set_parent)
        iface->set_parent(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_ITEM_MODEL(parent->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.set_parent not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_get_transform(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "transform", NULL };
    PyGObject *self;
    PyObject *py_transform;
    int ret;
    cairo_matrix_t *transform;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GooCanvasItemModel.get_transform", kwlist, &PyGooCanvasItemModel_Type, &self, &py_transform))
        return NULL;
    transform = &((PycairoMatrix*)(py_transform))->matrix;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->get_transform)
        ret = iface->get_transform(GOO_CANVAS_ITEM_MODEL(self->obj), transform);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.get_transform not implemented");
        return NULL;
    }
    return PyBool_FromLong(ret);

}

static PyObject *
_wrap_GooCanvasItemModel__do_set_transform(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "matrix", NULL };
    PyGObject *self;
    PyObject *py_matrix;
    cairo_matrix_t *matrix;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O:GooCanvasItemModel.set_transform", kwlist, &PyGooCanvasItemModel_Type, &self, &py_matrix))
        return NULL;
    matrix = &((PycairoMatrix*)(py_matrix))->matrix;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->set_transform)
        iface->set_transform(GOO_CANVAS_ITEM_MODEL(self->obj), matrix);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.set_transform not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_get_style(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", NULL };
    PyGObject *self;
    GooCanvasStyle *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!:GooCanvasItemModel.get_style", kwlist, &PyGooCanvasItemModel_Type, &self))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->get_style)
        ret = iface->get_style(GOO_CANVAS_ITEM_MODEL(self->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.get_style not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItemModel__do_set_style(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "style", NULL };
    PyGObject *self, *style;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItemModel.set_style", kwlist, &PyGooCanvasItemModel_Type, &self, &PyGooCanvasStyle_Type, &style))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->set_style)
        iface->set_style(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS_STYLE(style->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.set_style not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_create_item(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "canvas", NULL };
    PyGObject *self, *canvas;
    GooCanvasItem *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!O!:GooCanvasItemModel.create_item", kwlist, &PyGooCanvasItemModel_Type, &self, &PyGooCanvas_Type, &canvas))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->create_item)
        ret = iface->create_item(GOO_CANVAS_ITEM_MODEL(self->obj), GOO_CANVAS(canvas->obj));
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.create_item not implemented");
        return NULL;
    }
    /* pygobject_new handles NULL checking */
    return pygobject_new((GObject *)ret);
}

static PyObject *
_wrap_GooCanvasItemModel__do_child_added(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "child_num", NULL };
    PyGObject *self;
    int child_num;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItemModel.child_added", kwlist, &PyGooCanvasItemModel_Type, &self, &child_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->child_added)
        iface->child_added(GOO_CANVAS_ITEM_MODEL(self->obj), child_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.child_added not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_child_moved(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "old_child_num", "new_child_num", NULL };
    PyGObject *self;
    int old_child_num, new_child_num;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!ii:GooCanvasItemModel.child_moved", kwlist, &PyGooCanvasItemModel_Type, &self, &old_child_num, &new_child_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->child_moved)
        iface->child_moved(GOO_CANVAS_ITEM_MODEL(self->obj), old_child_num, new_child_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.child_moved not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_child_removed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "child_num", NULL };
    PyGObject *self;
    int child_num;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItemModel.child_removed", kwlist, &PyGooCanvasItemModel_Type, &self, &child_num))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->child_removed)
        iface->child_removed(GOO_CANVAS_ITEM_MODEL(self->obj), child_num);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.child_removed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
_wrap_GooCanvasItemModel__do_changed(PyObject *cls, PyObject *args, PyObject *kwargs)
{
    GooCanvasItemModelIface *iface;
    static char *kwlist[] = { "self", "recompute_bounds", NULL };
    PyGObject *self;
    int recompute_bounds;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O!i:GooCanvasItemModel.changed", kwlist, &PyGooCanvasItemModel_Type, &self, &recompute_bounds))
        return NULL;
    iface = g_type_interface_peek(g_type_class_peek(pyg_type_from_object(cls)), GOO_TYPE_CANVAS_ITEM_MODEL);
    if (iface->changed)
        iface->changed(GOO_CANVAS_ITEM_MODEL(self->obj), recompute_bounds);
    else {
        PyErr_SetString(PyExc_NotImplementedError, "interface method GooCanvasItemModel.changed not implemented");
        return NULL;
    }
    Py_INCREF(Py_None);
    return Py_None;
}

static const PyMethodDef _PyGooCanvasItemModel_methods[] = {
    { "get_n_children", (PyCFunction)_wrap_goo_canvas_item_model_get_n_children, METH_NOARGS,
      NULL },
    { "get_child", (PyCFunction)_wrap_goo_canvas_item_model_get_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "add_child", (PyCFunction)_wrap_goo_canvas_item_model_add_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "move_child", (PyCFunction)_wrap_goo_canvas_item_model_move_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove_child", (PyCFunction)_wrap_goo_canvas_item_model_remove_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "find_child", (PyCFunction)_wrap_goo_canvas_item_model_find_child, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_child_properties", (PyCFunction)_wrap_goo_canvas_item_model_get_child_properties, METH_VARARGS,
      NULL },
    { "set_child_properties", (PyCFunction)_wrap_goo_canvas_item_model_set_child_properties, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_parent", (PyCFunction)_wrap_goo_canvas_item_model_get_parent, METH_NOARGS,
      NULL },
    { "set_parent", (PyCFunction)_wrap_goo_canvas_item_model_set_parent, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "remove", (PyCFunction)_wrap_goo_canvas_item_model_remove, METH_NOARGS,
      NULL },
    { "is_container", (PyCFunction)_wrap_goo_canvas_item_model_is_container, METH_NOARGS,
      NULL },
    { "raise_", (PyCFunction)_wrap_goo_canvas_item_model_raise, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "lower", (PyCFunction)_wrap_goo_canvas_item_model_lower, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_transform", (PyCFunction)_wrap_goo_canvas_item_model_get_transform, METH_NOARGS,
      NULL },
    { "set_transform", (PyCFunction)_wrap_goo_canvas_item_model_set_transform, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "set_simple_transform", (PyCFunction)_wrap_goo_canvas_item_model_set_simple_transform, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "translate", (PyCFunction)_wrap_goo_canvas_item_model_translate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "scale", (PyCFunction)_wrap_goo_canvas_item_model_scale, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "rotate", (PyCFunction)_wrap_goo_canvas_item_model_rotate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "skew_x", (PyCFunction)_wrap_goo_canvas_item_model_skew_x, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "skew_y", (PyCFunction)_wrap_goo_canvas_item_model_skew_y, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "get_style", (PyCFunction)_wrap_goo_canvas_item_model_get_style, METH_NOARGS,
      NULL },
    { "set_style", (PyCFunction)_wrap_goo_canvas_item_model_set_style, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "animate", (PyCFunction)_wrap_goo_canvas_item_model_animate, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "stop_animation", (PyCFunction)_wrap_goo_canvas_item_model_stop_animation, METH_NOARGS,
      NULL },
    { "find_child_property", (PyCFunction)_wrap_goo_canvas_item_model_find_child_property, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "list_child_properties", (PyCFunction)_wrap_goo_canvas_item_model_list_child_properties, METH_NOARGS|METH_CLASS,
      NULL },
    { "install_child_property", (PyCFunction)_wrap_goo_canvas_item_model_install_child_property, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_n_children", (PyCFunction)_wrap_GooCanvasItemModel__do_get_n_children, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_child", (PyCFunction)_wrap_GooCanvasItemModel__do_get_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_add_child", (PyCFunction)_wrap_GooCanvasItemModel__do_add_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_move_child", (PyCFunction)_wrap_GooCanvasItemModel__do_move_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_remove_child", (PyCFunction)_wrap_GooCanvasItemModel__do_remove_child, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_parent", (PyCFunction)_wrap_GooCanvasItemModel__do_get_parent, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_parent", (PyCFunction)_wrap_GooCanvasItemModel__do_set_parent, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_transform", (PyCFunction)_wrap_GooCanvasItemModel__do_get_transform, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_transform", (PyCFunction)_wrap_GooCanvasItemModel__do_set_transform, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_get_style", (PyCFunction)_wrap_GooCanvasItemModel__do_get_style, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_set_style", (PyCFunction)_wrap_GooCanvasItemModel__do_set_style, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_create_item", (PyCFunction)_wrap_GooCanvasItemModel__do_create_item, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_child_added", (PyCFunction)_wrap_GooCanvasItemModel__do_child_added, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_child_moved", (PyCFunction)_wrap_GooCanvasItemModel__do_child_moved, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_child_removed", (PyCFunction)_wrap_GooCanvasItemModel__do_child_removed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { "do_changed", (PyCFunction)_wrap_GooCanvasItemModel__do_changed, METH_VARARGS|METH_KEYWORDS|METH_CLASS,
      NULL },
    { NULL, NULL, 0, NULL }
};

PyTypeObject G_GNUC_INTERNAL PyGooCanvasItemModel_Type = {
    PyObject_HEAD_INIT(NULL)
    0,                                 /* ob_size */
    "goocanvas.ItemModel",                   /* tp_name */
    sizeof(PyObject),          /* tp_basicsize */
    0,                                 /* tp_itemsize */
    /* methods */
    (destructor)0,        /* tp_dealloc */
    (printfunc)0,                      /* tp_print */
    (getattrfunc)0,       /* tp_getattr */
    (setattrfunc)0,       /* tp_setattr */
    (cmpfunc)0,           /* tp_compare */
    (reprfunc)0,             /* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,             /* tp_hash */
    (ternaryfunc)0,          /* tp_call */
    (reprfunc)0,              /* tp_str */
    (getattrofunc)0,     /* tp_getattro */
    (setattrofunc)0,     /* tp_setattro */
    (PyBufferProcs*)0,  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL,                        /* Documentation string */
    (traverseproc)0,     /* tp_traverse */
    (inquiry)0,             /* tp_clear */
    (richcmpfunc)0,   /* tp_richcompare */
    0,             /* tp_weaklistoffset */
    (getiterfunc)0,          /* tp_iter */
    (iternextfunc)0,     /* tp_iternext */
    (struct PyMethodDef*)_PyGooCanvasItemModel_methods, /* tp_methods */
    (struct PyMemberDef*)0,              /* tp_members */
    (struct PyGetSetDef*)0,  /* tp_getset */
    NULL,                              /* tp_base */
    NULL,                              /* tp_dict */
    (descrgetfunc)0,    /* tp_descr_get */
    (descrsetfunc)0,    /* tp_descr_set */
    0,                 /* tp_dictoffset */
    (initproc)0,             /* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};

static gint
_wrap_GooCanvasItemModel__proxy_do_get_n_children(GooCanvasItemModel *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    gint retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_n_children");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    py_retval = Py_BuildValue("(N)", py_retval);
    if (!PyArg_ParseTuple(py_retval, "i", &retval)) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return -G_MAXINT;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static GooCanvasItemModel*
_wrap_GooCanvasItemModel__proxy_do_get_child(GooCanvasItemModel *self, gint child_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child_num;
    GooCanvasItemModel* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_child_num = PyInt_FromLong(child_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_child_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_get_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasItemModel*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItemModel__proxy_do_add_child(GooCanvasItemModel *self, GooCanvasItemModel*child, gint position)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child = NULL;
    PyObject *py_position;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (child)
        py_child = pygobject_new((GObject *) child);
    else {
        Py_INCREF(Py_None);
        py_child = Py_None;
    }
    py_position = PyInt_FromLong(position);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_child);
    PyTuple_SET_ITEM(py_args, 1, py_position);
    
    py_method = PyObject_GetAttrString(py_self, "do_add_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItemModel__proxy_do_move_child(GooCanvasItemModel *self, gint old_position, gint new_position)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_old_position;
    PyObject *py_new_position;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_old_position = PyInt_FromLong(old_position);
    py_new_position = PyInt_FromLong(new_position);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_old_position);
    PyTuple_SET_ITEM(py_args, 1, py_new_position);
    
    py_method = PyObject_GetAttrString(py_self, "do_move_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItemModel__proxy_do_remove_child(GooCanvasItemModel *self, gint child_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child_num;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_child_num = PyInt_FromLong(child_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_child_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_remove_child");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GooCanvasItemModel*
_wrap_GooCanvasItemModel__proxy_do_get_parent(GooCanvasItemModel *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GooCanvasItemModel* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_parent");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasItemModel*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItemModel__proxy_do_set_parent(GooCanvasItemModel *self, GooCanvasItemModel*parent)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_parent = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (parent)
        py_parent = pygobject_new((GObject *) parent);
    else {
        Py_INCREF(Py_None);
        py_parent = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_parent);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_parent");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GooCanvasStyle*
_wrap_GooCanvasItemModel__proxy_do_get_style(GooCanvasItemModel *self)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    GooCanvasStyle* retval;
    PyObject *py_retval;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    
    
    py_method = PyObject_GetAttrString(py_self, "do_get_style");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, NULL);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasStyle*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItemModel__proxy_do_set_style(GooCanvasItemModel *self, GooCanvasStyle*style)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_style = NULL;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    if (style)
        py_style = pygobject_new((GObject *) style);
    else {
        Py_INCREF(Py_None);
        py_style = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_style);
    
    py_method = PyObject_GetAttrString(py_self, "do_set_style");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static GooCanvasItem*
_wrap_GooCanvasItemModel__proxy_do_create_item(GooCanvasItemModel *self, GooCanvas*canvas)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_canvas = NULL;
    GooCanvasItem* retval;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (canvas)
        py_canvas = pygobject_new((GObject *) canvas);
    else {
        Py_INCREF(Py_None);
        py_canvas = Py_None;
    }
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_canvas);
    
    py_method = PyObject_GetAttrString(py_self, "do_create_item");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (py_retval == Py_None) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    if (!PyObject_TypeCheck(py_retval, &PyGObject_Type)) {
        PyErr_SetString(PyExc_TypeError, "retval should be a GObject");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return NULL;
    }
    retval = (GooCanvasItem*) pygobject_get(py_retval);
    g_object_ref((GObject *) retval);
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
    
    return retval;
}
static void
_wrap_GooCanvasItemModel__proxy_do_child_added(GooCanvasItemModel *self, gint child_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child_num;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_child_num = PyInt_FromLong(child_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_child_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_child_added");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItemModel__proxy_do_child_moved(GooCanvasItemModel *self, gint old_child_num, gint new_child_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_old_child_num;
    PyObject *py_new_child_num;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_old_child_num = PyInt_FromLong(old_child_num);
    py_new_child_num = PyInt_FromLong(new_child_num);
    
    py_args = PyTuple_New(2);
    PyTuple_SET_ITEM(py_args, 0, py_old_child_num);
    PyTuple_SET_ITEM(py_args, 1, py_new_child_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_child_moved");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItemModel__proxy_do_child_removed(GooCanvasItemModel *self, gint child_num)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_child_num;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_child_num = PyInt_FromLong(child_num);
    
    py_args = PyTuple_New(1);
    PyTuple_SET_ITEM(py_args, 0, py_child_num);
    
    py_method = PyObject_GetAttrString(py_self, "do_child_removed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}
static void
_wrap_GooCanvasItemModel__proxy_do_changed(GooCanvasItemModel *self, gboolean recompute_bounds)
{
    PyGILState_STATE __py_state;
    PyObject *py_self;
    PyObject *py_recompute_bounds;
    PyObject *py_retval;
    PyObject *py_args;
    PyObject *py_method;
    
    __py_state = pyg_gil_state_ensure();
    py_self = pygobject_new((GObject *) self);
    if (!py_self) {
        if (PyErr_Occurred())
            PyErr_Print();
        pyg_gil_state_release(__py_state);
        return;
    }
    py_recompute_bounds = recompute_bounds? Py_True : Py_False;
    
    py_args = PyTuple_New(1);
    Py_INCREF(py_recompute_bounds);
    PyTuple_SET_ITEM(py_args, 0, py_recompute_bounds);
    
    py_method = PyObject_GetAttrString(py_self, "do_changed");
    if (!py_method) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    py_retval = PyObject_CallObject(py_method, py_args);
    if (!py_retval) {
        if (PyErr_Occurred())
            PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    if (py_retval != Py_None) {
        PyErr_SetString(PyExc_TypeError, "virtual method should return None");
        PyErr_Print();
        Py_XDECREF(py_retval);
        Py_DECREF(py_method);
        Py_DECREF(py_args);
        Py_DECREF(py_self);
        pyg_gil_state_release(__py_state);
        return;
    }
    
    
    Py_XDECREF(py_retval);
    Py_DECREF(py_method);
    Py_DECREF(py_args);
    Py_DECREF(py_self);
    pyg_gil_state_release(__py_state);
}

static void
__GooCanvasItemModel__interface_init(GooCanvasItemModelIface *iface, PyTypeObject *pytype)
{
    GooCanvasItemModelIface *parent_iface = g_type_interface_peek_parent(iface);
    PyObject *py_method;

    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_n_children") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_n_children = _wrap_GooCanvasItemModel__proxy_do_get_n_children;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_n_children = parent_iface->get_n_children;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_child = _wrap_GooCanvasItemModel__proxy_do_get_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_child = parent_iface->get_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_add_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->add_child = _wrap_GooCanvasItemModel__proxy_do_add_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->add_child = parent_iface->add_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_move_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->move_child = _wrap_GooCanvasItemModel__proxy_do_move_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->move_child = parent_iface->move_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_remove_child") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->remove_child = _wrap_GooCanvasItemModel__proxy_do_remove_child;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->remove_child = parent_iface->remove_child;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_parent") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_parent = _wrap_GooCanvasItemModel__proxy_do_get_parent;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_parent = parent_iface->get_parent;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_parent") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_parent = _wrap_GooCanvasItemModel__proxy_do_set_parent;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_parent = parent_iface->set_parent;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_get_style") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->get_style = _wrap_GooCanvasItemModel__proxy_do_get_style;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->get_style = parent_iface->get_style;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_set_style") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->set_style = _wrap_GooCanvasItemModel__proxy_do_set_style;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->set_style = parent_iface->set_style;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_create_item") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->create_item = _wrap_GooCanvasItemModel__proxy_do_create_item;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->create_item = parent_iface->create_item;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_child_added") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->child_added = _wrap_GooCanvasItemModel__proxy_do_child_added;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->child_added = parent_iface->child_added;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_child_moved") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->child_moved = _wrap_GooCanvasItemModel__proxy_do_child_moved;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->child_moved = parent_iface->child_moved;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_child_removed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->child_removed = _wrap_GooCanvasItemModel__proxy_do_child_removed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->child_removed = parent_iface->child_removed;
        }
    Py_XDECREF(py_method);
    }
    py_method = pytype? PyObject_GetAttrString((PyObject *) pytype, "do_changed") : NULL;
    if (py_method && !PyObject_TypeCheck(py_method, &PyCFunction_Type)) {
        iface->changed = _wrap_GooCanvasItemModel__proxy_do_changed;
    } else {
        PyErr_Clear();
        if (parent_iface) {
            iface->changed = parent_iface->changed;
        }
    Py_XDECREF(py_method);
    }
}


static const GInterfaceInfo __GooCanvasItemModel__iinfo = {
    (GInterfaceInitFunc) __GooCanvasItemModel__interface_init,
    NULL,
    NULL
};


/* ----------- functions ----------- */

#line 308 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_polyline_new_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    PyGObject       *parent;
    double          x1, y1, x2, y2;
    GooCanvasItem   *ret;
    GObjectClass    *class;
    Py_ssize_t      pos;
    PyObject        *value;
    PyObject        *key;
    GType           type;

    if (!PyArg_ParseTuple(args, "O!dddd:polyline_new_line",
                          &PyGooCanvasItem_Type, &parent, &x1,
                          &y1, &x2, &y2))
        return NULL;

    ret = goo_canvas_polyline_new_line(GOO_CANVAS_ITEM(parent->obj), x1,
                                       y1, x2, y2, NULL);

    type = goo_canvas_polyline_get_type();

    if (!ret) {
    PyErr_SetString (PyExc_RuntimeError, "could not create canvas item object");
    return NULL;
    }

    class = G_OBJECT_GET_CLASS(ret);
    g_object_freeze_notify (G_OBJECT(ret));
    pos = 0;

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
	    gtk_object_destroy(GTK_OBJECT(ret));
	    return NULL;
	}

	g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
	if (pyg_value_from_pyobject(&gvalue, value)) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "could not convert value for property `%s'", key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    gtk_object_destroy(GTK_OBJECT(ret));
	    return NULL;
	}
	g_object_set_property(G_OBJECT(ret), key_str, &gvalue);
	g_value_unset(&gvalue);
    }

    g_object_thaw_notify (G_OBJECT(ret));

    return pygobject_new((GObject *)ret);
}
#line 10007 "goocanvas.c"


#line 376 "goocanvas.override"
static PyObject *
_wrap_goo_canvas_polyline_model_new_line(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    PyGObject           *parent;
    double              x1, y1, x2, y2;
    GooCanvasItemModel  *ret;
    GObjectClass        *class;
    Py_ssize_t          pos;
    PyObject            *value;
    PyObject            *key;
    GType               type;

    if (!PyArg_ParseTuple(args, "O!dddd:polyline_model_new_line",
                          &PyGooCanvasItemModel_Type, &parent, &x1,
                          &y1, &x2, &y2))
        return NULL;

    ret = goo_canvas_polyline_model_new_line(GOO_CANVAS_ITEM_MODEL(parent->obj), x1,
                                       y1, x2, y2, NULL);

    type = goo_canvas_polyline_model_get_type();

    if (!ret) {
    PyErr_SetString (PyExc_RuntimeError, "could not create canvas item model object");
    return NULL;
    }

    class = G_OBJECT_GET_CLASS(ret);
    g_object_freeze_notify (G_OBJECT(ret));
    pos = 0;

    while (kwargs && PyDict_Next (kwargs, &pos, &key, &value)) {
    gchar *key_str = PyString_AsString (key);
    GParamSpec *pspec;
    GValue gvalue ={ 0, };

    pspec = g_object_class_find_property (class, key_str);
    if (!pspec) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "canvas item model `%s' doesn't support property `%s'",
		       g_type_name(type), key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    gtk_object_destroy(GTK_OBJECT(ret));
	    return NULL;
	}

	g_value_init(&gvalue, G_PARAM_SPEC_VALUE_TYPE(pspec));
	if (pyg_value_from_pyobject(&gvalue, value)) {
	    gchar buf[512];

	    g_snprintf(buf, sizeof(buf),
		       "could not convert value for property `%s'", key_str);
	    PyErr_SetString(PyExc_TypeError, buf);
	    gtk_object_destroy(GTK_OBJECT(ret));
	    return NULL;
	}
	g_object_set_property(G_OBJECT(ret), key_str, &gvalue);
	g_value_unset(&gvalue);
    }

    g_object_thaw_notify (G_OBJECT(ret));

    return pygobject_new((GObject *)ret);
}

#line 10078 "goocanvas.c"


static PyObject *
_wrap_goo_cairo_matrix_copy(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "matrix", NULL };
    PyObject *py_matrix;
    cairo_matrix_t *matrix, *ret;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:goo_cairo_matrix_copy", kwlist, &py_matrix))
        return NULL;
    matrix = &((PycairoMatrix*)(py_matrix))->matrix;
    
    ret = goo_cairo_matrix_copy(matrix);
    
    if (ret)
        return PycairoMatrix_FromMatrix(ret);
    else {
        Py_INCREF(Py_None);
        return Py_None;
    }
}

static PyObject *
_wrap_goo_cairo_matrix_free(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "matrix", NULL };
    PyObject *py_matrix;
    cairo_matrix_t *matrix;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs,"O:goo_cairo_matrix_free", kwlist, &py_matrix))
        return NULL;
    matrix = &((PycairoMatrix*)(py_matrix))->matrix;
    
    goo_cairo_matrix_free(matrix);
    
    Py_INCREF(Py_None);
    return Py_None;
}

const PyMethodDef pygoocanvas_functions[] = {
    { "polyline_new_line", (PyCFunction)_wrap_goo_canvas_polyline_new_line, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "polyline_model_new_line", (PyCFunction)_wrap_goo_canvas_polyline_model_new_line, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "goo_cairo_matrix_copy", (PyCFunction)_wrap_goo_cairo_matrix_copy, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { "goo_cairo_matrix_free", (PyCFunction)_wrap_goo_cairo_matrix_free, METH_VARARGS|METH_KEYWORDS,
      NULL },
    { NULL, NULL, 0, NULL }
};


/* ----------- enums and flags ----------- */

void
pygoocanvas_add_constants(PyObject *module, const gchar *strip_prefix)
{
#ifdef VERSION
    PyModule_AddStringConstant(module, "__version__", VERSION);
#endif
  pyg_enum_add(module, "AnimateType", strip_prefix, GOO_TYPE_CANVAS_ANIMATE_TYPE);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_MOVE_TO", strip_prefix), GOO_CANVAS_PATH_MOVE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_CLOSE_PATH", strip_prefix), GOO_CANVAS_PATH_CLOSE_PATH);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_LINE_TO", strip_prefix), GOO_CANVAS_PATH_LINE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_HORIZONTAL_LINE_TO", strip_prefix), GOO_CANVAS_PATH_HORIZONTAL_LINE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_VERTICAL_LINE_TO", strip_prefix), GOO_CANVAS_PATH_VERTICAL_LINE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_CURVE_TO", strip_prefix), GOO_CANVAS_PATH_CURVE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_SMOOTH_CURVE_TO", strip_prefix), GOO_CANVAS_PATH_SMOOTH_CURVE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_QUADRATIC_CURVE_TO", strip_prefix), GOO_CANVAS_PATH_QUADRATIC_CURVE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_SMOOTH_QUADRATIC_CURVE_TO", strip_prefix), GOO_CANVAS_PATH_SMOOTH_QUADRATIC_CURVE_TO);
    PyModule_AddIntConstant(module, (char *) pyg_constant_strip_prefix("GOO_CANVAS_PATH_ELLIPTICAL_ARC", strip_prefix), GOO_CANVAS_PATH_ELLIPTICAL_ARC);
  pyg_flags_add(module, "PointerEvents", strip_prefix, GOO_TYPE_CANVAS_POINTER_EVENTS);
  pyg_enum_add(module, "ItemVisibility", strip_prefix, GOO_TYPE_CANVAS_ITEM_VISIBILITY);

  if (PyErr_Occurred())
    PyErr_Print();
}

/* initialise stuff extension classes */
void
pygoocanvas_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gobject")) != NULL) {
        _PyGObject_Type = (PyTypeObject *)PyObject_GetAttrString(module, "GObject");
        if (_PyGObject_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name GObject from gobject");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gobject");
        return ;
    }
    if ((module = PyImport_ImportModule("gtk")) != NULL) {
        _PyGtkContainer_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Container");
        if (_PyGtkContainer_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Container from gtk");
            return ;
        }
        _PyGtkAdjustment_Type = (PyTypeObject *)PyObject_GetAttrString(module, "Adjustment");
        if (_PyGtkAdjustment_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Adjustment from gtk");
            return ;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk");
        return ;
    }


#line 198 "goocanvas.override"
    if (PyType_Ready(&PyGooCanvasBounds_Type) < 0) {
        g_return_if_reached();
    }
    if (PyDict_SetItemString(d, "Bounds", (PyObject *)&PyGooCanvasBounds_Type) < 0) {
        g_return_if_reached();
    }



#line 10206 "goocanvas.c"
    pyg_register_boxed(d, "Points", GOO_TYPE_CANVAS_POINTS, &PyGooCanvasPoints_Type);
    pyg_register_boxed(d, "LineDash", GOO_TYPE_CANVAS_LINE_DASH, &PyGooCanvasLineDash_Type);
    pyg_register_interface(d, "Item", GOO_TYPE_CANVAS_ITEM, &PyGooCanvasItem_Type);
    pyg_register_interface_info(GOO_TYPE_CANVAS_ITEM, &__GooCanvasItem__iinfo);
    pyg_register_interface(d, "ItemModel", GOO_TYPE_CANVAS_ITEM_MODEL, &PyGooCanvasItemModel_Type);
    pyg_register_interface_info(GOO_TYPE_CANVAS_ITEM_MODEL, &__GooCanvasItemModel__iinfo);
    pygobject_register_class(d, "GooCanvas", GOO_TYPE_CANVAS, &PyGooCanvas_Type, Py_BuildValue("(O)", &PyGtkContainer_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS);
    pyg_register_class_init(GOO_TYPE_CANVAS, __GooCanvas_class_init);
    pygobject_register_class(d, "GooCanvasItemModelSimple", GOO_TYPE_CANVAS_ITEM_MODEL_SIMPLE, &PyGooCanvasItemModelSimple_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_ITEM_MODEL_SIMPLE);
    pygobject_register_class(d, "GooCanvasImageModel", GOO_TYPE_CANVAS_IMAGE_MODEL, &PyGooCanvasImageModel_Type, Py_BuildValue("(O)", &PyGooCanvasItemModelSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_IMAGE_MODEL);
    pygobject_register_class(d, "GooCanvasGroupModel", GOO_TYPE_CANVAS_GROUP_MODEL, &PyGooCanvasGroupModel_Type, Py_BuildValue("(O)", &PyGooCanvasItemModelSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_GROUP_MODEL);
    pygobject_register_class(d, "GooCanvasEllipseModel", GOO_TYPE_CANVAS_ELLIPSE_MODEL, &PyGooCanvasEllipseModel_Type, Py_BuildValue("(O)", &PyGooCanvasItemModelSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_ELLIPSE_MODEL);
    pygobject_register_class(d, "GooCanvasItemSimple", GOO_TYPE_CANVAS_ITEM_SIMPLE, &PyGooCanvasItemSimple_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_ITEM_SIMPLE);
    pyg_register_class_init(GOO_TYPE_CANVAS_ITEM_SIMPLE, __GooCanvasItemSimple_class_init);
    pygobject_register_class(d, "GooCanvasImage", GOO_TYPE_CANVAS_IMAGE, &PyGooCanvasImage_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_IMAGE);
    pygobject_register_class(d, "GooCanvasSvg", GOO_TYPE_CANVAS_SVG, &PyGooCanvasSvg_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_SVG);
    pygobject_register_class(d, "GooCanvasGroup", GOO_TYPE_CANVAS_GROUP, &PyGooCanvasGroup_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_GROUP);
    pygobject_register_class(d, "GooCanvasEllipse", GOO_TYPE_CANVAS_ELLIPSE, &PyGooCanvasEllipse_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_ELLIPSE);
    pygobject_register_class(d, "GooCanvasPath", GOO_TYPE_CANVAS_PATH, &PyGooCanvasPath_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_PATH);
    pygobject_register_class(d, "GooCanvasPathModel", GOO_TYPE_CANVAS_PATH_MODEL, &PyGooCanvasPathModel_Type, Py_BuildValue("(O)", &PyGooCanvasItemModelSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_PATH_MODEL);
    pygobject_register_class(d, "GooCanvasPolyline", GOO_TYPE_CANVAS_POLYLINE, &PyGooCanvasPolyline_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_POLYLINE);
    pygobject_register_class(d, "GooCanvasPolylineModel", GOO_TYPE_CANVAS_POLYLINE_MODEL, &PyGooCanvasPolylineModel_Type, Py_BuildValue("(O)", &PyGooCanvasItemModelSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_POLYLINE_MODEL);
    pygobject_register_class(d, "GooCanvasRect", GOO_TYPE_CANVAS_RECT, &PyGooCanvasRect_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_RECT);
    pygobject_register_class(d, "GooCanvasRectModel", GOO_TYPE_CANVAS_RECT_MODEL, &PyGooCanvasRectModel_Type, Py_BuildValue("(O)", &PyGooCanvasItemModelSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_RECT_MODEL);
    pygobject_register_class(d, "GooCanvasStyle", GOO_TYPE_CANVAS_STYLE, &PyGooCanvasStyle_Type, Py_BuildValue("(O)", &PyGObject_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_STYLE);
    pygobject_register_class(d, "GooCanvasTable", GOO_TYPE_CANVAS_TABLE, &PyGooCanvasTable_Type, Py_BuildValue("(O)", &PyGooCanvasGroup_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_TABLE);
    pygobject_register_class(d, "GooCanvasTableModel", GOO_TYPE_CANVAS_TABLE_MODEL, &PyGooCanvasTableModel_Type, Py_BuildValue("(O)", &PyGooCanvasGroupModel_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_TABLE_MODEL);
    pygobject_register_class(d, "GooCanvasText", GOO_TYPE_CANVAS_TEXT, &PyGooCanvasText_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_TEXT);
    pygobject_register_class(d, "GooCanvasTextModel", GOO_TYPE_CANVAS_TEXT_MODEL, &PyGooCanvasTextModel_Type, Py_BuildValue("(O)", &PyGooCanvasItemModelSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_TEXT_MODEL);
    pygobject_register_class(d, "GooCanvasWidget", GOO_TYPE_CANVAS_WIDGET, &PyGooCanvasWidget_Type, Py_BuildValue("(O)", &PyGooCanvasItemSimple_Type));
    pyg_set_object_has_new_constructor(GOO_TYPE_CANVAS_WIDGET);
}
