/* gcompris - py-mod-gcompris.c
 *
 * Copyright (C) 2003, 2008 Olivier Samyn <osamyn@ulb.ac.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include "py-gcompris-profile.h"
#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>

staticforward PyTypeObject pyGcomprisUserType;

//static char pyGcomprisUserType_doc[]= "Python GcomprisBoars structure binding";


/* Special function created for the python plugin to be able to create
 * a pyGcomprisBoardObject form the existing GcomprisBoard structure
 */
PyObject*
gcompris_new_pyGcomprisUserObject(GcomprisUser* user)
{
  pyGcomprisUserObject* theuser = NULL;

  theuser = PyObject_New(pyGcomprisUserObject, &pyGcomprisUserType);
  if (theuser!=NULL)
    theuser->cdata = user;

  return (PyObject*)theuser;
}


/* Free the python gcompris user */
static void
pyGcomprisUserType_dealloc(pyGcomprisUserObject *self)
{
  self->cdata = NULL;
  PyObject_DEL(self);
}


/* Methods defined in the pyGcomprisUser class */
static PyMethodDef pyGcomprisUserType_methods[] = {
        {NULL,          NULL}           /* sentinel */
};


/* Return the value of the members contained in the GcomprisUser structure */
static PyObject *
pyGcomprisUserType_getattr(pyGcomprisUserObject *self, char *name)
{
    /* int */
    if(strcmp(name,"user_id")==0) return Py_BuildValue("i", self->cdata->user_id);
    /* int */
    if(strcmp(name,"class_id")==0) return Py_BuildValue("i", self->cdata->class_id);
    /* str */
    if(strcmp(name,"login")==0) return Py_BuildValue("s", self->cdata->login);
    /* str */
    if(strcmp(name,"lastname")==0) return Py_BuildValue("s", self->cdata->lastname);
    /* str */
    if(strcmp(name,"firstname")==0) return Py_BuildValue("s", self->cdata->firstname);
    /* str */
    if(strcmp(name,"birthdate")==0) return Py_BuildValue("s", self->cdata->birthdate);
    /* u int */
    if(strcmp(name,"session_id")==0) return Py_BuildValue("i", self->cdata->session_id);

  return Py_FindMethod(pyGcomprisUserType_methods, (PyObject *)self, name);
}

/* Set the value of a GcomprisUser structure member */
static int
pyGcomprisUserType_setattr(pyGcomprisUserObject *self, char *name, PyObject *v)
{
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

static PyTypeObject pyGcomprisUserType = {
#if defined(WIN32)
  PyObject_HEAD_INIT(NULL)
#else /* ! WIN32 */
  PyObject_HEAD_INIT(&PyType_Type)
#endif
  0,                                        /*ob_size*/
  "pyGcomprisUser",                        /*tp_name*/
  sizeof(pyGcomprisUserObject),            /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  /* methods */
  (destructor)pyGcomprisUserType_dealloc,  /*tp_dealloc*/
  0,                                        /*tp_print*/
  (getattrfunc)pyGcomprisUserType_getattr, /*tp_getattr*/
  (setattrfunc)pyGcomprisUserType_setattr, /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash*/
};
