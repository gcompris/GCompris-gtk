/* gcompris - py-mod-gcompris.c
 *
 * Copyright (C) 2003 Olivier Samyn <osamyn@ulb.ac.be>
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
#include "py-gcompris-properties.h"
#define NO_IMPORT_PYGOBJECT 1
#include <pygobject.h>
#include "py-gcompris-profile.h"
#include "py-gcompris-board.h"

staticforward PyTypeObject pyGcomprisPropertiesType;

//static char pyGcomprisPropertiesType_doc[]= "Python GcomprisBoars structure binding";


/* Special function created for the python plugin to be able to create
 * a pyGcomprisBoardObject form the existing GcomprisBoard structure
 */
PyObject*
gcompris_new_pyGcomprisPropertiesObject(GcomprisProperties* properties)
{
  pyGcomprisPropertiesObject* theproperties = NULL;

  theproperties = PyObject_New(pyGcomprisPropertiesObject, &pyGcomprisPropertiesType);
  if (theproperties!=NULL)
    theproperties->cdata = properties;

  return (PyObject*)theproperties;
}


/* Free the python gcompris properties */
static void
pyGcomprisPropertiesType_dealloc(pyGcomprisPropertiesObject *self)
{
  self->cdata = NULL;
  PyObject_DEL(self);
}


/* Methods defined in the pyGcomprisProperties class */
static PyMethodDef pyGcomprisPropertiesType_methods[] = {
        {NULL,          NULL}           /* sentinel */
};


/* Return the value of the members contained in the GcomprisProperties structure */
static PyObject *
pyGcomprisPropertiesType_getattr(pyGcomprisPropertiesObject *self, char *name)
{
  if (self->cdata != NULL) {
    /* Properties type */
    if(strcmp(name,"music")==0){
      if(self->cdata->music){
	Py_INCREF(Py_True);
	return Py_True;
      } else {
	Py_INCREF(Py_False);
	return Py_False;
      }
    }
    if(strcmp(name,"fx")==0){
      if(self->cdata->fx){
	Py_INCREF(Py_True);
	return Py_True;
      } else {
	Py_INCREF(Py_False);
	return Py_False;
      }
    }
    if(strcmp(name,"fullscreen")==0){
      if(self->cdata->fullscreen){
	Py_INCREF(Py_True);
	return Py_True;
      } else {
	Py_INCREF(Py_False);
	return Py_False;
      }
    }
    /* enum */
    if(strcmp(name,"screensize")==0) return Py_BuildValue("i", self->cdata->screensize);

    /* enum */
    if(strcmp(name,"defaultcursor")==0) return Py_BuildValue("i", self->cdata->defaultcursor);

    if(strcmp(name,"timer")==0) return Py_BuildValue("i", self->cdata->timer);

    /* enum */
    if(strcmp(name,"filter_style")==0) return Py_BuildValue("i", self->cdata->filter_style);

    if(strcmp(name,"difficulty_filter")==0) return Py_BuildValue("i", self->cdata->difficulty_filter);
    if(strcmp(name,"difficulty_max")==0) return Py_BuildValue("i", self->cdata->difficulty_max);
    if(strcmp(name,"root_menu")==0) return Py_BuildValue("s", self->cdata->root_menu);
    if(strcmp(name,"package_data_dir")==0) return Py_BuildValue("s", self->cdata->package_data_dir);
    if(strcmp(name,"package_plugin_dir")==0) return Py_BuildValue("s", self->cdata->package_plugin_dir);
    if(strcmp(name,"package_python_plugin_dir")==0) return Py_BuildValue("s", self->cdata->package_python_plugin_dir);
    if(strcmp(name,"locale")==0) return Py_BuildValue("s", self->cdata->locale);
    if(strcmp(name,"skin")==0) return Py_BuildValue("s", self->cdata->skin);
    if(strcmp(name,"config_dir")==0) return Py_BuildValue("s", self->cdata->config_dir);
    if(strcmp(name,"user_dir")==0) return Py_BuildValue("s", self->cdata->user_dir);

    if(strcmp(name,"profile")==0){
      if (self->cdata->profile)
	return gcompris_new_pyGcomprisProfileObject(self->cdata->profile);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }

    if(strcmp(name,"logged_user")==0) {
      if (self->cdata->logged_user)
	return gcompris_new_pyGcomprisUserObject(self->cdata->logged_user);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }

    if(strcmp(name,"menu_board")==0) {
      if (self->cdata->menu_board)
	return gcompris_new_pyGcomprisBoardObject(self->cdata->menu_board);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }

    if(strcmp(name,"database")==0) return Py_BuildValue("s", self->cdata->database);
    if(strcmp(name,"administration")==0){
      if(self->cdata->administration){
	Py_INCREF(Py_True);
	return Py_True;
      } else {
	Py_INCREF(Py_False);
	return Py_False;
      }
    }
    if(strcmp(name,"reread_menu")==0){
      if(self->cdata->reread_menu){
	Py_INCREF(Py_True);
	return Py_True;
      } else {
	Py_INCREF(Py_False);
	return Py_False;
      }
    }

    if(strcmp(name,"context")==0){
      if (self->cdata->context)
	return (PyObject*) pygobject_new((GObject*)self->cdata->context);
      else {
	Py_INCREF(Py_None);
	return Py_None;
      }
    }

    if(strcmp(name,"default_context")==0) return Py_BuildValue("s", self->cdata->default_context);

  }
  return Py_FindMethod(pyGcomprisPropertiesType_methods, (PyObject *)self, name);
}

/* Set the value of a GcomprisProperties structure member */
static int
pyGcomprisPropertiesType_setattr(pyGcomprisPropertiesObject *self, char *name, PyObject *v)
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

static PyTypeObject pyGcomprisPropertiesType = {
#if defined(WIN32)
  PyObject_HEAD_INIT(NULL)
#else /* ! WIN32 */
  PyObject_HEAD_INIT(&PyType_Type)
#endif
  0,                                        /*ob_size*/
  "pyGcomprisProperties",                        /*tp_name*/
  sizeof(pyGcomprisPropertiesObject),            /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  /* methods */
  (destructor)pyGcomprisPropertiesType_dealloc,  /*tp_dealloc*/
  0,                                        /*tp_print*/
  (getattrfunc)pyGcomprisPropertiesType_getattr, /*tp_getattr*/
  (setattrfunc)pyGcomprisPropertiesType_setattr, /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash*/
};
