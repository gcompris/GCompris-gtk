#include "py-gcompris-board.h"
#include <pygobject.h>

staticforward PyTypeObject pyGcomprisBoardType;

//static char pyGcomprisBoardType_doc[]= "Python GcomprisBoars structure binding";


/* Special function created for the python plugin to be able to create
 * a pyGcomprisBoardObject form the existing GcomprisBoard structure
 */
PyObject* 
gcompris_new_pyGcomprisBoardObject(GcomprisBoard* aboard)
{
  pyGcomprisBoardObject* theboard = NULL;

  theboard = PyObject_New(pyGcomprisBoardObject, &pyGcomprisBoardType);
  if (theboard!=NULL)
    theboard->cdata = aboard;

  return (PyObject*)theboard;
}


/* Free the python gcompris board */
static void 
pyGcomprisBoardType_dealloc(pyGcomprisBoardObject *self)
{
  self->cdata = NULL;
  PyObject_DEL(self);
}
                       

/* Methods defined in the pyGcomprisBoard class */
static PyMethodDef pyGcomprisBoardType_methods[] = {
        {NULL,          NULL}           /* sentinel */
};
 

/* Return the value of the members contained in the GcomprisBoard structure */
static PyObject *
pyGcomprisBoardType_getattr(pyGcomprisBoardObject *self, char *name)
{
  if (self->cdata != NULL) {
    /* Board type */
    if(strcmp(name,"type")==0) return Py_BuildValue("s", self->cdata->type);
    if(strcmp(name,"board_ready")==0){
      if(self->cdata->board_ready){
	Py_INCREF(Py_True);
	return Py_True;
      } else {
	Py_INCREF(Py_False);
	return Py_False;
      }
    }

    /* Board specific mode */
    if(strcmp(name,"mode")==0) return Py_BuildValue("s", self->cdata->mode);

    /* Name of this Board */
    if(strcmp(name,"name")==0) return Py_BuildValue("s", self->cdata->name);
    if(strcmp(name,"title")==0) return Py_BuildValue("s", self->cdata->title);
    if(strcmp(name,"description")==0) return Py_BuildValue("s", self->cdata->description);
    if(strcmp(name,"icon_name")==0) return Py_BuildValue("s", self->cdata->icon_name);
    if(strcmp(name,"author")==0) return Py_BuildValue("s", self->cdata->author);
    if(strcmp(name,"boarddir")==0) return Py_BuildValue("s", self->cdata->boarddir);
    if(strcmp(name,"filename")==0) return Py_BuildValue("s", self->cdata->filename);
    if(strcmp(name,"difficulty")==0) return Py_BuildValue("s", self->cdata->difficulty);
    if(strcmp(name,"mandatory_sound_file")==0) return Py_BuildValue("s", self->cdata->mandatory_sound_file);
    
    /* Menu positionning */
    if(strcmp(name,"section")==0) return Py_BuildValue("s", self->cdata->section);
    if(strcmp(name,"menuposition")==0) return Py_BuildValue("s", self->cdata->menuposition);

    /* Help Information */
    if(strcmp(name,"prerequisite")==0) return Py_BuildValue("s", self->cdata->prerequisite);
    if(strcmp(name,"goal")==0) return Py_BuildValue("s", self->cdata->goal);
    if(strcmp(name,"manual")==0) return Py_BuildValue("s", self->cdata->manual);
    if(strcmp(name,"credit")==0) return Py_BuildValue("s", self->cdata->credit);

    /* Size of the window in which we have to run */
    if(strcmp(name,"width")==0) return Py_BuildValue("i", self->cdata->width);
    if(strcmp(name,"height")==0) return Py_BuildValue("i", self->cdata->height);

    /* Dynamic board information (status) */
    if(strcmp(name,"level")==0) return Py_BuildValue("i", self->cdata->level);
    if(strcmp(name,"maxlevel")==0) return Py_BuildValue("i", self->cdata->maxlevel);
    if(strcmp(name,"sublevel")==0) return Py_BuildValue("i", self->cdata->sublevel);
    if(strcmp(name,"number_of_sublevel")==0) return Py_BuildValue("i", self->cdata->number_of_sublevel);

    if(strcmp(name,"previous_level")==0)
      return gcompris_new_pyGcomprisBoardObject(self->cdata->previous_board);
    
    if(strcmp(name,"canvas")==0)
      return (PyObject*) pygobject_new((GObject*)self->cdata->canvas);

    if(strcmp(name,"is_configurable")==0){
      if (!self->cdata->plugin){
	board_check_file(self->cdata);
	if (!self->cdata->plugin){
	  g_warning("board %s/%s seems not working !", self->cdata->section, self->cdata->name);
	  Py_INCREF(Py_False);
	  return Py_False;
	}
      }
      
      if (self->cdata->plugin->config_start && self->cdata->plugin->config_stop){
	printf ("Board %s is configurable \n", self->cdata->name);
	Py_INCREF(Py_True);
	return Py_True;
      } else {
	Py_INCREF(Py_False);
	return Py_False;
      }
    }

    if(strcmp(name,"board_id")==0) return Py_BuildValue("i", self->cdata->board_id);

    if(strcmp(name,"section_id")==0) return Py_BuildValue("i", self->cdata->section_id);
    
    /* Other members are special one... 
     * TODO: Does we need to write a BoardPlugin structure wrapper ?
     */    
  }
  return Py_FindMethod(pyGcomprisBoardType_methods, (PyObject *)self, name);
}

/* Set the value of a GcomprisBoard structure member */
static int
pyGcomprisBoardType_setattr(pyGcomprisBoardObject *self, char *name, PyObject *v)
{
  int value;
  
  if (self->cdata==NULL) return -1;
  if (v==NULL) return -1;

  if (strcmp(name,"level")==0){
    value = (int) PyInt_AsLong(v);
    if ( value < 0 ) return -1;
    self->cdata->level=value;
    return 0;
  }
  if (strcmp(name,"maxlevel")==0){
    value = PyInt_AsLong(v);
    if ( value < 0 ) return -1;
    self->cdata->maxlevel=value;
    return 0;
  }
  if (strcmp(name,"sublevel")==0){
    value = PyInt_AsLong(v);
    if ( value < 0 ) return -1;
    self->cdata->sublevel=value;
    return 0;
  }
  if (strcmp(name,"number_of_sublevel")==0){
    value = PyInt_AsLong(v);
    if ( value < 0 ) return -1;
    self->cdata->number_of_sublevel=value;
    return 0;
  }
  /* Other members are supposed to be read only */

  return -1;
}
                                                                                
static PyTypeObject pyGcomprisBoardType = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,                                        /*ob_size*/
  "pyGcomprisBoard",                        /*tp_name*/
  sizeof(pyGcomprisBoardObject),            /*tp_basicsize*/
  0,                                        /*tp_itemsize*/
  /* methods */
  (destructor)pyGcomprisBoardType_dealloc,  /*tp_dealloc*/
  0,                                        /*tp_print*/
  (getattrfunc)pyGcomprisBoardType_getattr, /*tp_getattr*/
  (setattrfunc)pyGcomprisBoardType_setattr, /*tp_setattr*/
  0,                                        /*tp_compare*/
  0,                                        /*tp_repr*/
  0,                                        /*tp_as_number*/
  0,                                        /*tp_as_sequence*/
  0,                                        /*tp_as_mapping*/
  0,                                        /*tp_hash*/
};
