#include "py-gcompris-wordlist.h"
#include <pygobject.h>
#include "py-gcompris-profile.h"
#include "py-gcompris-board.h"

staticforward PyTypeObject pyGcomprisWordlistType;

//static char pyGcomprisWordlistType_doc[]= "Python GcomprisBoars structure binding";


/* Special function created for the python plugin to be able to create
 * a pyGcomprisBoardObject form the existing GcomprisBoard structure
 */
PyObject* 
gcompris_new_pyGcomprisWordlistObject(GcomprisWordlist* wordlist)
{
  pyGcomprisWordlistObject* thewordlist = NULL;

  thewordlist = PyObject_New(pyGcomprisWordlistObject, &pyGcomprisWordlistType);
  if (thewordlist!=NULL)
    thewordlist->cdata = wordlist;

  return (PyObject*)thewordlist;
}


/* Free the python gcompris wordlist */
static void 
pyGcomprisWordlistType_dealloc(pyGcomprisWordlistObject *self)
{
  gcompris_wordlist_free((GcomprisWordlist *)self->cdata);
  self->cdata = NULL;
  PyObject_DEL(self);
}
                       

/* Methods defined in the pyGcomprisWordlist class */
static PyMethodDef pyGcomprisWordlistType_methods[] = {
        {NULL,          NULL}           /* sentinel */
};
 

/* Return the value of the members contained in the GcomprisWordlist structure */
static PyObject *
pyGcomprisWordlistType_getattr(pyGcomprisWordlistObject *self, char *name)
{
  if (self->cdata != NULL) {
    /* Wordlist filename */
    if(strcmp(name,"filename")==0) return Py_BuildValue("s", self->cdata->filename);
    if(strcmp(name,"locale")==0) return Py_BuildValue("z", self->cdata->locale);
    if(strcmp(name,"level")==0) return Py_BuildValue("z", self->cdata->level);

    if(strcmp(name,"description")==0) return Py_BuildValue("z", self->cdata->description);

    /* list */
    if(strcmp(name,"words")==0){
      PyObject *pylist;
      GList *list;

      pylist = PyList_New(0);
      for (list = self->cdata->words; list !=NULL; list = list->next){
	PyList_Append(pylist, Py_BuildValue("s", (gchar *)list->data));
      }

      return pylist;
    }


  }
  return Py_FindMethod(pyGcomprisWordlistType_methods, (PyObject *)self, name);
}

/* Set the value of a GcomprisWordlist structure member */
static int
pyGcomprisWordlistType_setattr(pyGcomprisWordlistObject *self, char *name, PyObject *v)
{
  /* members are supposed to be read only */

  return -1;
}
                                                                                
static PyTypeObject pyGcomprisWordlistType = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,                                        /*ob_size*/
  "pyGcomprisWordlist",                        /*tp_name*/
  sizeof(pyGcomprisWordlistObject),            /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  /* methods */
  (destructor)pyGcomprisWordlistType_dealloc,  /*tp_dealloc*/
  0,                                        /*tp_print*/
  (getattrfunc)pyGcomprisWordlistType_getattr, /*tp_getattr*/
  (setattrfunc)pyGcomprisWordlistType_setattr, /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash*/
};
