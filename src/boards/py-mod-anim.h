#ifndef _PY_MOD_ANIM_H_
#define _PY_MOD_ANIM_H_

#include <Python.h>
#include "gcompris/anim.h"

void python_gcompris_anim_module_init();

typedef struct {
  PyObject_HEAD
  GcomprisAnimation *a;
} py_GcomprisAnimation;

#endif
