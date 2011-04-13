#include "../../rtt-config.h"

#ifndef __GCC_ORO_SYSTEM_H
#define __GCC_ORO_SYSTEM_H

#define oro_cmpxchg(ptr,o,n)\
	((__typeof__(*(ptr)))__sync_val_compare_and_swap((ptr),(o),(n)))

#endif // __GCC_ORO_SYSTEM_H

