#include <Foundation/Foundation.h>
#include "gcompris.h"

gchar *gcompris_nsbundle_resource(void)
{
#ifdef NSBUNDLE
  NSAutoreleasePool *pool;
  gchar *resourcePath = NULL;
  pool = [NSAutoreleasePool new];
  
  resourcePath = g_strdup_printf("%s",[[[NSBundle mainBundle] resourcePath] UTF8String]);

  printf("NSBundle resourcePath %s\n", resourcePath);

  RELEASE(pool);
  return resourcePath;
#else
  return NULL;
#endif
}
