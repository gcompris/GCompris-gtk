#include "py-gcompris-boardconfig.h"
#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>
#include "py-gcompris-profile.h"
#include "py-gcompris-board.h"

staticforward PyTypeObject pyGcomprisBoardConfigType;

/* Special function created for the python plugin to be able to create
 * a pyGcomprisBoardObject form the existing GcomprisBoard structure
 */
PyObject*
gcompris_new_pyGcomprisBoardConfigObject(GcomprisBoardConf* boardconfig)
{
  pyGcomprisBoardConfigObject* theboardconf = NULL;

  theboardconf = PyObject_New(pyGcomprisBoardConfigObject, &pyGcomprisBoardConfigType);
  if (theboardconf!=NULL)
    theboardconf->cdata = boardconfig ;

  return (PyObject*)theboardconf;
}


/* Free the python gcompris board config */
static void
pyGcomprisBoardConfigType_dealloc(pyGcomprisBoardConfigObject *self)
{
  self->cdata = NULL;
  PyObject_DEL(self);
}


/* Methods defined in the pyGcomprisBoardConfig class */
static PyMethodDef pyGcomprisBoardConfigType_methods[] = {
        {NULL,          NULL}           /* sentinel */
};


/* Return the value of the members contained in the GcomprisBoardConfig structure */
static PyObject *
pyGcomprisBoardConfigType_getattr(pyGcomprisBoardConfigObject *self, char *name)
{
  return Py_FindMethod(pyGcomprisBoardConfigType_methods, (PyObject *)self, name);

}

/* Set the value of a GcomprisBoardConfig structure member */
static int
pyGcomprisBoardConfigType_setattr(pyGcomprisBoardConfigObject *self, char *name, PyObject *v)
{
  /* members are supposed to be read only */

  return -1;
}

static PyTypeObject pyGcomprisBoardConfigType = {
#if defined(WIN32)
  PyObject_HEAD_INIT(NULL)
#else /* ! WIN32 */
  PyObject_HEAD_INIT(&PyType_Type)
#endif
  0,                                        /*ob_size*/
  "pyGcomprisBoardConfig",                        /*tp_name*/
  sizeof(pyGcomprisBoardConfigObject),            /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  /* methods */
  (destructor)pyGcomprisBoardConfigType_dealloc,  /*tp_dealloc*/
  0,                                        /*tp_print*/
  (getattrfunc)pyGcomprisBoardConfigType_getattr, /*tp_getattr*/
  (setattrfunc)pyGcomprisBoardConfigType_setattr, /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash*/
};
