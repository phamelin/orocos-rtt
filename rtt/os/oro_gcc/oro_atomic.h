#ifndef __GCC_ORO_ATOMIC__
#define __GCC_ORO_ATOMIC__

#include "../../rtt-config.h"

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
typedef struct { volatile int counter; } oro_atomic_t;

#define ORO_ATOMIC_INIT(i)	{ (i) }
#define ORO_ATOMIC_SETUP	oro_atomic_set
#define ORO_ATOMIC_CLEANUP(v)	

/**
 * oro_atomic_read - read atomic variable
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically reads the value of @v.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */ 
#define oro_atomic_read(v)		((v)->counter)

/**
 * oro_atomic_set - set atomic variable
 * @v: pointer of type oro_atomic_t
 * @i: required value
 * 
 * Atomically sets the value of @v to @i.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */ 
#define oro_atomic_set(v,i)		(((v)->counter) = (i))

/**
 * oro_atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically adds @i to @v.  Note that the guaranteed useful range
 * of an oro_atomic_t is only 24 bits.
 */
static __inline__ void oro_atomic_add(int i, oro_atomic_t *v)
{
    (void)__sync_add_and_fetch(&v->counter, i);
}

/**
 * oro_atomic_sub - subtract the atomic variable
 * @i: integer value to subtract
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically subtracts @i from @v.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */
static __inline__ void oro_atomic_sub(int i, oro_atomic_t *v)
{
    (void)__sync_sub_and_fetch(&v->counter, i);
}

/**
 * oro_atomic_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */
static __inline__ int oro_atomic_sub_and_test(int i, oro_atomic_t *v)
{
    return !(__sync_sub_and_fetch(&v->counter, i));
}

/**
 * oro_atomic_inc - increment atomic variable
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically increments @v by 1.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */ 
static __inline__ void oro_atomic_inc(oro_atomic_t *v)
{
    (void)__sync_fetch_and_add(&v->counter, 1);
}

/**
 * oro_atomic_dec - decrement atomic variable
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically decrements @v by 1.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */ 
static __inline__ void oro_atomic_dec(oro_atomic_t *v)
{
    (void)__sync_fetch_and_sub(&v->counter, 1);
}

/**
 * oro_atomic_dec_and_test - decrement and test
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */ 
static __inline__ int oro_atomic_dec_and_test(oro_atomic_t *v)
{
    return !(__sync_sub_and_fetch(&v->counter, 1));
}

/**
 * oro_atomic_inc_and_test - increment and test 
 * @v: pointer of type oro_atomic_t
 * 
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */ 
static __inline__ int oro_atomic_inc_and_test(oro_atomic_t *v)
{
    return !(__sync_add_and_fetch(&v->counter, 1));
}

/**
 * oro_atomic_add_negative - add and test if negative
 * @v: pointer of type oro_atomic_t
 * @i: integer value to add
 * 
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.  Note that the guaranteed
 * useful range of an oro_atomic_t is only 24 bits.
 */ 
static __inline__ int oro_atomic_add_negative(int i, oro_atomic_t *v)
{
    return (__sync_add_and_fetch(&v->counter, i) < 0);
}


#endif // __GCC_ORO_ATOMIC__

