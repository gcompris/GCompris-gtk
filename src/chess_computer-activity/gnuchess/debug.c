/*
 * debug.c - Debugging output
 */

#include <config.h>
#include "common.h"

#ifdef DEBUG

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdarg.h>

#define MAX_DEBUG 1000
#define DEBUG_FILENAME "gnuchess.debug"

/* Default to stderr */
static int debug_fd = 2;

int dbg_open(const char *name)
{
   int flags = O_WRONLY | O_CREAT | O_APPEND;
   int mode = 0777;
   
   if (name == NULL) {
      debug_fd = open(DEBUG_FILENAME, flags, mode);
   } else {
      debug_fd = open(name, flags, mode);
   }
   if (-1 == debug_fd) {
      debug_fd = 2;
      return -1;
   }
   dbg_printf("--- Opening debug log ---\n");
   return debug_fd;
}

int dbg_close(void)
{
   /* We don't want to close stderr */
   if (debug_fd == 2) {
      return -1;
   }
   dbg_printf("--- Closing debug log ---\n");
   return close(debug_fd);
}

int dbg_printf(const char *fmt, ...)
{
   va_list ap;
   char buf[MAX_DEBUG];
   struct timeval tv;
   
   gettimeofday(&tv, NULL);
   sprintf(buf, "%010ld.%06ld: ", tv.tv_sec, tv.tv_usec);
   write(debug_fd, buf, strlen(buf));

   va_start(ap, fmt);
   vsnprintf(buf, MAX_DEBUG, fmt, ap);
   va_end(ap);

   return write(debug_fd, buf, strlen(buf));
}

#else /* !DEBUG */

int dbg_open(const char *name __attribute__((unused)) ) { return 0; }
int dbg_printf(const char *fmt __attribute__((unused)), ...) { return 0; }
int dbg_close(void) { return 0; }

#endif /* DEBUG */
