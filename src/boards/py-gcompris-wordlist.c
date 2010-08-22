/* gcompris
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
#include "py-gcompris-wordlist.h"
#define NO_IMPORT_PYGOBJECT 1
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
  gc_wordlist_free((GcomprisWordlist *)self->cdata);
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
    GcomprisWordlist *wl = self->cdata;
    /* Wordlist filename */
    if(strcmp(name,"filename")==0) return Py_BuildValue("s", wl->filename);
    if(strcmp(name,"locale")==0) return Py_BuildValue("z", wl->locale);
    if(strcmp(name,"description")==0) return Py_BuildValue("z", wl->description);
    if(strcmp(name,"number_of_level")==0) return Py_BuildValue("i", wl->number_of_level);

    /* list */
    if(strcmp(name,"words")==0){
      PyObject *pydict;
      PyObject *pylist;
      GSList *levelList, *list_words;

      pydict = PyDict_New();

      // Fixme does not work, I get only the level 2 no idea why
      for (levelList = wl->levels_words; levelList;
	   levelList = levelList->next){
	LevelWordlist *lw = levelList->data;
	gint level = lw->level;

	pylist = PyList_New(0);
	for (list_words = lw->words; list_words !=NULL;
	     list_words = list_words->next){
	  PyList_Append(pylist, Py_BuildValue("s", (gchar *)list_words->data));
	}

	PyDict_SetItem( pydict, PyInt_FromLong(	(long) level), pylist);

      return pydict;
      }
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
#if defined(WIN32)
  PyObject_HEAD_INIT(NULL)
#else /* ! WIN32 */
  PyObject_HEAD_INIT(&PyType_Type)
#endif
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
