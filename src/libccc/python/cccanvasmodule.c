
#include <pygobject.h>
#include <ccc/cc-item.h>
#include <ccc/cc-brush.h>
#include <ccc/cc-color.h>
#include <ccc/cc-printer.h>
#include <ccc/cc-view-png.h>

void pycccanvas_register_classes (PyObject *d); 
extern PyMethodDef pycccanvas_functions[];

void pycccanvas_add_constants(PyObject *module, const gchar *strip_prefix);

DL_EXPORT(void) initcccanvas(void);

static void
sink_obj(GObject *object)
{
    if (g_object_is_floating (object)) {
	g_object_ref_sink(object);
    }
}

DL_EXPORT(void)
initcccanvas(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    pygobject_register_sinkfunc(CC_TYPE_ITEM, sink_obj);
    pygobject_register_sinkfunc(CC_TYPE_BRUSH, sink_obj);
    pygobject_register_sinkfunc(CC_TYPE_COLOR, sink_obj);
    pygobject_register_sinkfunc(CC_TYPE_PRINTER, sink_obj);
    pygobject_register_sinkfunc(CC_TYPE_VIEW_PNG, sink_obj);

    m = Py_InitModule ("cccanvas", pycccanvas_functions);
    d = PyModule_GetDict (m);
 
    pycccanvas_register_classes (d);
    pycccanvas_add_constants (m, "CC_");
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module cccanvas");
    }
}
