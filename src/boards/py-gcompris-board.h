#ifndef _PY_GCOMPRIS_BOARD_H_
#define _PY_GCOMPRIS_BOARD_H_

#include <Python.h>
#include "gcompris/gcompris.h"

PyObject* gcompris_new_pyGcomprisBoardObject(GcomprisBoard* aboard);

typedef struct{
  PyObject_HEAD
  GcomprisBoard* cdata;
} pyGcomprisBoardObject;

#endif
