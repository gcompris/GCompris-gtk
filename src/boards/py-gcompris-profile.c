#include "py-gcompris-profile.h"
#include <pygobject.h>

staticforward PyTypeObject pyGcomprisProfileType;

//static char pyGcomprisProfileType_doc[]= "Python GcomprisBoars structure binding";


/* Special function created for the python plugin to be able to create
 * a pyGcomprisBoardObject form the existing GcomprisBoard structure
 */
PyObject* 
gcompris_new_pyGcomprisProfileObject(GcomprisProfile* profile)
{
  if (!profile)
    return Py_None;

  pyGcomprisProfileObject* theprofile = NULL;

  theprofile = PyObject_New(pyGcomprisProfileObject, &pyGcomprisProfileType);
  if (theprofile!=NULL)
    theprofile->cdata = profile;

  return (PyObject*)theprofile;
}


/* Free the python gcompris profile */
static void 
pyGcomprisProfileType_dealloc(pyGcomprisProfileObject *self)
{
  self->cdata = NULL;
  PyObject_DEL(self);
}
                       

/* Methods defined in the pyGcomprisProfile class */
static PyMethodDef pyGcomprisProfileType_methods[] = {
        {NULL,          NULL}           /* sentinel */
};
 

/* Return the value of the members contained in the GcomprisProfile structure */
static PyObject *
pyGcomprisProfileType_getattr(pyGcomprisProfileObject *self, char *name)
{
    /* int */
    if(strcmp(name,"profile_id")==0) return Py_BuildValue("i", self->cdata->profile_id);
    /* int */
    if(strcmp(name,"name")==0) return Py_BuildValue("s", self->cdata->name);
    /* int */
    if(strcmp(name,"directory")==0) return Py_BuildValue("s", self->cdata->directory);
    /* int */
    if(strcmp(name,"description")==0) return Py_BuildValue("s", self->cdata->description);

    /* list */
    if(strcmp(name,"group_ids")==0){
      PyObject *pylist;
      GList *list;

      pylist = PyList_New(0);
      for (list = self->cdata->group_ids; list !=NULL; list = list->next)
	PyList_Append(pylist, Py_BuildValue("i", *((int *)list->data)));

      return pylist;
    }

    /* list */
    if(strcmp(name,"activities")==0){
      PyObject *pylist;
      GList *list;

      pylist = PyList_New(0);
      for (list = self->cdata->activities; list !=NULL; list = list->next)
	PyList_Append(pylist, Py_BuildValue("i", *((int *)list->data)));

      return pylist;
    }

  return Py_FindMethod(pyGcomprisProfileType_methods, (PyObject *)self, name);
}

/* Set the value of a GcomprisProfile structure member */
static int
pyGcomprisProfileType_setattr(pyGcomprisProfileObject *self, char *name, PyObject *v)
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
                                                                                
static PyTypeObject pyGcomprisProfileType = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,                                        /*ob_size*/
  "pyGcomprisProfile",                        /*tp_name*/
  sizeof(pyGcomprisProfileObject),            /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  /* methods */
  (destructor)pyGcomprisProfileType_dealloc,  /*tp_dealloc*/
  0,                                        /*tp_print*/
  (getattrfunc)pyGcomprisProfileType_getattr, /*tp_getattr*/
  (setattrfunc)pyGcomprisProfileType_setattr, /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash*/
};
