
#ifndef __foo_canvas_marshal_MARSHAL_H__
#define __foo_canvas_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:INT,INT,INT,INT (assetml-marshal.list:1) */
extern void foo_canvas_marshal_VOID__INT_INT_INT_INT (GClosure     *closure,
                                                      GValue       *return_value,
                                                      guint         n_param_values,
                                                      const GValue *param_values,
                                                      gpointer      invocation_hint,
                                                      gpointer      marshal_data);

/* BOOLEAN:BOXED (assetml-marshal.list:2) */
extern void foo_canvas_marshal_BOOLEAN__BOXED (GClosure     *closure,
                                               GValue       *return_value,
                                               guint         n_param_values,
                                               const GValue *param_values,
                                               gpointer      invocation_hint,
                                               gpointer      marshal_data);

G_END_DECLS

#endif /* __foo_canvas_marshal_MARSHAL_H__ */

