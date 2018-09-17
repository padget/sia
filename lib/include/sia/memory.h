#ifndef __sia_memory_h__
#  define __sia_memory_h__

#  include <stdlib.h>

#  define sia_malloc(num, type) (type*) malloc(sizeof(type) * num) 

#endif