
#include <pygobject.h>

void pycccanvas_register_classes (PyObject *d); 
extern PyMethodDef pycccanvas_functions[];

void pycccanvas_add_constants(PyObject *module, const gchar *strip_prefix);

DL_EXPORT(void) initcccanvas(void);

DL_EXPORT(void)
initcccanvas(void)
{
    PyObject *m, *d;
 
    init_pygobject ();
 
    m = Py_InitModule ("cccanvas", pycccanvas_functions);
    d = PyModule_GetDict (m);
 
    pycccanvas_register_classes (d);
    pycccanvas_add_constants (m, "CC_");
 
    if (PyErr_Occurred ()) {
        Py_FatalError ("can't initialise module cccanvas");
    }
}
