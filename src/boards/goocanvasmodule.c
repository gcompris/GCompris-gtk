#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <pygobject.h>
#define NO_IMPORT_PYGTK
#include <pygtk/pygtk.h>
#include <goocanvas.h>

# include <pycairo.h>
Pycairo_CAPI_t *Pycairo_CAPI;


void pygoocanvas_register_classes (PyObject *d);
void pygoocanvas_add_constants(PyObject *module, const gchar *strip_prefix);

extern PyMethodDef pygoocanvas_functions[];

static PyObject *
_cairo_matrix_from_gvalue(const GValue *value)
{
    return PycairoMatrix_FromMatrix((cairo_matrix_t *) g_value_get_boxed(value));
}

static int
_cairo_matrix_to_gvalue(GValue *value, PyObject *obj)
{
    if (!(PyObject_IsInstance(obj, (PyObject *) &PycairoMatrix_Type)))
        return -1;

    g_value_set_boxed(value, &((PycairoMatrix*)(obj))->matrix);
    return 0;
}

static PyObject *
_cairo_pattern_from_gvalue(const GValue *value)
{
#if defined WIN32
    // On windows we have the old cairo API for now
    return PycairoPattern_FromPattern(cairo_pattern_reference((cairo_pattern_t *) g_value_get_boxed(value)));
#else
    return PycairoPattern_FromPattern(cairo_pattern_reference((cairo_pattern_t *) g_value_get_boxed(value)), NULL);
#endif
}

static int
_cairo_pattern_to_gvalue(GValue *value, PyObject *obj)
{
    if (obj == Py_None) {
        g_value_set_boxed(value, NULL);
        return 0;
    }

    if (!(PyObject_IsInstance(obj, (PyObject *) &PycairoPattern_Type)))
        return -1;

    g_value_set_boxed(value, ((PycairoPattern*)(obj))->pattern);
    return 0;
}

DL_EXPORT (void)
initgoocanvas (void)
{
    PyObject *m, *d;


    Pycairo_IMPORT;
    if (Pycairo_CAPI == NULL)
        return;

    m = Py_InitModule ("goocanvas", pygoocanvas_functions);
    d = PyModule_GetDict (m);

    init_pygobject ();

    pygoocanvas_register_classes (d);
    pygoocanvas_add_constants(m, "GOO_CANVAS_");
    PyModule_AddObject(m, "TYPE_CAIRO_MATRIX", pyg_type_wrapper_new(GOO_TYPE_CAIRO_MATRIX));
    pyg_register_gtype_custom(GOO_TYPE_CAIRO_MATRIX,
			      _cairo_matrix_from_gvalue,
			      _cairo_matrix_to_gvalue);
    PyModule_AddObject(m, "TYPE_CAIRO_PATTERN", pyg_type_wrapper_new(GOO_TYPE_CAIRO_PATTERN));
    pyg_register_gtype_custom(GOO_TYPE_CAIRO_PATTERN,
			      _cairo_pattern_from_gvalue,
			      _cairo_pattern_to_gvalue);

    PyModule_AddObject(m, "pygoocanvas_version",
                       Py_BuildValue("iii",
                                     PYGOOCANVAS_MAJOR_VERSION,
                                     PYGOOCANVAS_MINOR_VERSION,
                                     PYGOOCANVAS_MICRO_VERSION));

    if (PyErr_Occurred ())
        Py_FatalError ("can't initialise module goocanvas");
}
