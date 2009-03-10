#ifndef _PY_GCOMPRIS_BOARDCONFIG_H_
#define _PY_GCOMPRIS_BOARDCONFIG_H_

#include <Python.h>
#include "gcompris/gcompris.h"

PyObject* gcompris_new_pyGcomprisBoardConfigObject(GcomprisBoardConf * boardconfig);

typedef struct{
  PyObject_HEAD
  GcomprisBoardConf* cdata;
} pyGcomprisBoardConfigObject;

#endif
