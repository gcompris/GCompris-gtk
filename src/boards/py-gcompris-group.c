#include "py-gcompris-profile.h"
#include <pygobject.h>

staticforward PyTypeObject pyGcomprisGroupType;

//static char pyGcomprisGroupType_doc[]= "Python GcomprisBoars structure binding";


/* Special function created for the python plugin to be able to create
 * a pyGcomprisBoardObject form the existing GcomprisBoard structure
 */
PyObject* 
gcompris_new_pyGcomprisGroupObject(GcomprisGroup* group)
{
  pyGcomprisGroupObject* thegroup = NULL;

  thegroup = PyObject_New(pyGcomprisGroupObject, &pyGcomprisGroupType);
  if (thegroup!=NULL)
    thegroup->cdata = group;

  return (PyObject*)thegroup;
}


/* Free the python gcompris group */
static void 
pyGcomprisGroupType_dealloc(pyGcomprisGroupObject *self)
{
  self->cdata = NULL;
  PyObject_DEL(self);
}
                       

/* Methods defined in the pyGcomprisGroup class */
static PyMethodDef pyGcomprisGroupType_methods[] = {
        {NULL,          NULL}           /* sentinel */
};
 

/* Return the value of the members contained in the GcomprisGroup structure */
static PyObject *
pyGcomprisGroupType_getattr(pyGcomprisGroupObject *self, char *name)
{
    /* int */
    if(strcmp(name,"group_id")==0) return Py_BuildValue("i", self->cdata->group_id);
    /* int */
    if(strcmp(name,"name")==0) return Py_BuildValue("s", self->cdata->name);
    /* string */
    if(strcmp(name,"description")==0) return Py_BuildValue("s", self->cdata->description);
    /* int */
    if(strcmp(name,"class_id")==0) return Py_BuildValue("i", self->cdata->class_id);

    /* list */
    if(strcmp(name,"user_ids")==0){
      PyObject *pylist;
      GList *list;

      pylist = PyList_New(0);
      for (list = self->cdata->user_ids; list !=NULL; list = list->next)
	PyList_Append(pylist, Py_BuildValue("i", *((int *)list->data)));

      return pylist;
    }

  return Py_FindMethod(pyGcomprisGroupType_methods, (PyObject *)self, name);
}

/* Set the value of a GcomprisGroup structure member */
static int
pyGcomprisGroupType_setattr(pyGcomprisGroupObject *self, char *name, PyObject *v)
{
  int value;
  
  if (self->cdata==NULL) return -1;
  if (v==NULL) return -1;

  /*  if (strcmp(name,"level")==0){
    value = (int) PyInt_AsLong(v);
    if ( value < 0 ) return -1;
    self->cdata->level=value;
    return 0;
    } */
  /* members are supposed to be read only */

  return -1;
}
                                                                                
static PyTypeObject pyGcomprisGroupType = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,                                        /*ob_size*/
  "pyGcomprisGroup",                        /*tp_name*/
  sizeof(pyGcomprisGroupObject),            /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  /* methods */
  (destructor)pyGcomprisGroupType_dealloc,  /*tp_dealloc*/
  0,                                        /*tp_print*/
  (getattrfunc)pyGcomprisGroupType_getattr, /*tp_getattr*/
  (setattrfunc)pyGcomprisGroupType_setattr, /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash*/
};
