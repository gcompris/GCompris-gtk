/* gcompris - python.c
 *
 * Copyright (C) 2003 GCompris Developpement Team
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <Python.h>
#include <pygobject.h>
#include "gcompris/gcompris.h"
#include "py-gcompris-board.h"
#include "py-mod-gcompris.h"

static GcomprisBoard *gcomprisBoard = NULL;
static PyObject* python_gcomprisBoard = NULL;
static PyObject* python_board_module = NULL;
static PyObject* python_board_instance = NULL;

static void	 pythonboard_start (GcomprisBoard *agcomprisBoard);
static void	 pythonboard_pause (gboolean pause);
static void	 pythonboard_end (void);

static gboolean	 pythonboard_is_our_board (GcomprisBoard *agcomprisBoard);

static gint	 pythonboard_key_press (guint keyval);
static void	 pythonboard_ok (void);
static void	 pythonboard_set_level (guint level);
static void	 pythonboard_config(void);
static void	 pythonboard_repeat (void);


/* Description of this plugin */
BoardPlugin pythonboard_bp =
{
   NULL,
   NULL,
   N_("Python Board"),
   N_("Special board that embed python into gcompris."),
   "Olivier Samyn <osamyn@ulb.ac.be>",
   NULL,
   NULL,
   NULL,
   NULL,
   pythonboard_start,
   pythonboard_pause,
   pythonboard_end,
   pythonboard_is_our_board,
   pythonboard_key_press,
   pythonboard_ok,
   pythonboard_set_level,
   pythonboard_config,
   pythonboard_repeat
};


/*
 * Return the plugin structure (Common to all gcompris boards
 */

BoardPlugin 
*get_bplugin_info(void)
{
   return &pythonboard_bp;
}


/*
 * Start the board. 
 * In this case: 
 * - initialize python interpreter
 * - import gcompris functions/objects
 * - import gtk/gnome functions/objects
 * - load the python written board
 * - call the board start function
 */
void pythonboard_start (GcomprisBoard *agcomprisBoard){
  PyObject* main_module;
  PyObject* py_function_result;
  PyObject* module_dict;
  PyObject* py_boardclass;
  PyObject* py_boardclass_args;
  PyObject* globals;
  static char *python_args[]={ "" };
  static char* python_prog_name="gcompris";
  char* boarddir;
  char* boardclass;
  
  if(agcomprisBoard!=NULL){
    /* Initialize the python interpreter */
    Py_SetProgramName(python_prog_name);
    Py_Initialize();
  
    PySys_SetArgv(1, python_args);
    
    init_pygobject();

    main_module = PyImport_AddModule("__main__");
    globals = PyModule_GetDict(main_module);
    
    if(globals==NULL){
      g_print("Cannot get infos from the python interpreter. Seems there is a problem with this one.\n");  
      return;
    } else {
      gcomprisBoard = agcomprisBoard;
    }

    /* Add the python plugins dir to the python's search path */
    boarddir = g_strdup_printf("import sys; sys.path.append('%s/python')",PLUGIN_DIR); 
    PyRun_SimpleString(boarddir); 
    g_free(boarddir); 

    /* Load the gcompris modules */
    python_gcompris_module_init();

    /* Python is now initialized we create some usefull variables */
    boardclass = g_strdup_printf("Gcompris_%s", agcomprisBoard->name);

    /* Insert the board module into the python's interpreter */
    python_board_module = PyImport_ImportModuleEx(agcomprisBoard->name,  
 						  globals,
 						  globals,
 						  NULL); 
    
    if(python_board_module!=NULL){ 
      /* Get the module dictionnary */
      module_dict = PyModule_GetDict(python_board_module); 

      /* Get the python board class */
      py_boardclass = PyDict_GetItemString(module_dict, boardclass);

      /* Create a python gcompris board */
      python_gcomprisBoard=gcompris_new_pyGcomprisBoardObject(agcomprisBoard); 
      
      /* Create an instance of the board class */
      py_boardclass_args = PyTuple_New(1);
      Py_INCREF(python_gcomprisBoard);
      PyTuple_SetItem(py_boardclass_args, 0, python_gcomprisBoard);
      python_board_instance = PyInstance_New(py_boardclass, py_boardclass_args, NULL);
      Py_DECREF(py_boardclass_args);
      
      /* Call the function */ 
      py_function_result = PyObject_CallMethod(python_board_instance, "start", NULL);
      if( py_function_result != NULL){
	Py_DECREF(py_function_result);
      } else {
	PyErr_Print(); 
      }
    } else { 
      PyErr_Print(); 
    }
    
    g_free(boardclass); 
  }
}

/*
 * Pause the board.
 */
void pythonboard_pause (gboolean pause){
  PyObject* result = NULL;

  result = PyObject_CallMethod(python_board_instance, "pause", "i", pause);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print(); 
  }
}

/*
 * End the board.
 * In this case:
 * - call the board end function
 * - finalise python interpreter
 */
void pythonboard_end (void){
  PyObject* result = NULL;

  if(python_gcomprisBoard!=NULL){
    result = PyObject_CallMethod(python_board_instance, "end", NULL);
    if( result == NULL){
      PyErr_Print(); 
    } else {
      Py_DECREF(result);
    }
    Py_XDECREF(python_board_module);
    Py_XDECREF(python_board_instance);
    Py_XDECREF(python_gcomprisBoard);
    Py_Finalize();
  }
}

/*
 * Return TRUE if the board is a python one.
 */
gboolean pythonboard_is_our_board (GcomprisBoard *agcomprisBoard){
  if (agcomprisBoard!=NULL)
    {
      if(g_strcasecmp(agcomprisBoard->type, "pythonboard")==0)
        {
          /* Set the plugin entry */
          agcomprisBoard->plugin=&pythonboard_bp;
	  
	  g_print("pythonboard: is our board = TRUE\n");
	  
          return TRUE;
        }
    }
  return FALSE;  
}

/*
 * Key press
 */
gint pythonboard_key_press (guint keyval){
  PyObject* result = NULL;

  result = PyObject_CallMethod(python_board_instance, "key_press", "i", keyval);

  if (result==NULL) return FALSE;

  if (PyInt_AsLong(result)>0){
    Py_DECREF(result);
    return TRUE;
  } else {
    Py_DECREF(result);
    return FALSE;
  }
}

/*
 * OK button pressed
 */
void pythonboard_ok (void){
  PyObject* result = NULL;
  result = PyObject_CallMethod(python_board_instance, "ok", NULL);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print(); 
  }
}

/*
 * Set Level
 */
void pythonboard_set_level (guint level){
  PyObject* result = NULL;

  result = PyObject_CallMethod(python_board_instance, "set_level", "i", level);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print(); 
  }
}

/*
 * Config
 */
void pythonboard_config(void){
  PyObject* result = NULL;
  result = PyObject_CallMethod(python_board_instance, "config", NULL);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print(); 
  }
}

/*
 * Repeat
 */
void pythonboard_repeat (void){
  PyObject* result = NULL;
  result = PyObject_CallMethod(python_board_instance, "repeat", NULL);
  if( result != NULL){
    Py_DECREF(result);
  } else {
    PyErr_Print(); 
  }
}
