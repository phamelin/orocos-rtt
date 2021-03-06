/***************************************************************************
  tag: The SourceWorks  Tue Sep 7 00:55:18 CEST 2010  oro_atomic.h

                        oro_atomic.h -  description
                           -------------------
    begin                : Tue September 07 2010
    copyright            : (C) 2010 The SourceWorks
    email                : peter@thesourceworks.com

 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/


#include "../../rtt-config.h"
#ifndef __ARCH_MSVC_ORO_ATOMIC__
#define __ARCH_MSVC_ORO_ATOMIC__

#include <windows.h>
#undef interface
#include <intrin.h>

/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

typedef volatile long oro_atomic_t;

#define ORO_ATOMIC_INIT(i)	{ (i) }
#define ORO_ATOMIC_SETUP	oro_atomic_set
#define ORO_ATOMIC_CLEANUP(v)	

/**
 * oro_atomic_read - read atomic variable
 * @v: pointer of type oro_atomic_t
 *
 * Atomically reads the value of @v.
 */
#define oro_atomic_read(v)		(*(v))

/**
 * oro_atomic_set - set atomic variable
 * @v: pointer of type oro_atomic_t
 * @i: required value
 *
 * Atomically sets the value of @v to @i.
 */
#define oro_atomic_set(v,i)		(*(v) = (i))

/**
 * oro_atomic_add - add integer to atomic variable
 * @i: integer value to add
 * @v: pointer of type oro_atomic_t
 *
 * Atomically adds @i to @v.
 */
static __forceinline void oro_atomic_add(int i, oro_atomic_t *v)
{
	_InterlockedExchangeAdd((long *)v, i);
}

/**
 * oro_atomic_sub - subtract the atomic variable
 * @i: integer value to subtract
 * @v: pointer of type oro_atomic_t
 *
 * Atomically subtracts @i from @v.
 */
static __forceinline void oro_atomic_sub(int i, oro_atomic_t *v)
{
	oro_atomic_add(-i, v);
}

/**
 * oro_atomic_sub_and_test - subtract value from variable and test result
 * @i: integer value to subtract
 * @v: pointer of type oro_atomic_t
 *
 * Atomically subtracts @i from @v and returns
 * true if the result is zero, or false for all
 * other cases.
 */
static __forceinline int oro_atomic_sub_and_test(int i, oro_atomic_t *v)
{
	return ((_InterlockedExchangeAdd((long *)v, -i) - i) == 0);
}

/**
 * oro_atomic_inc - increment atomic variable
 * @v: pointer of type oro_atomic_t
 *
 * Atomically increments @v by 1.
 */
static __forceinline void oro_atomic_inc(oro_atomic_t *v)
{
	_InterlockedIncrement((long *)v);
}

/**
 * oro_atomic_dec - decrement atomic variable
 * @v: pointer of type oro_atomic_t
 *
 * Atomically decrements @v by 1.
 */
static __forceinline void oro_atomic_dec(oro_atomic_t *v)
{
	_InterlockedDecrement((long *)v);
}

/**
 * oro_atomic_dec_and_test - decrement and test
 * @v: pointer of type oro_atomic_t
 *
 * Atomically decrements @v by 1 and
 * returns true if the result is 0, or false for all other
 * cases.
 */
static __forceinline int oro_atomic_dec_and_test(oro_atomic_t *v)
{
	return (_InterlockedDecrement((long *)v) == 0);
}

/**
 * oro_atomic_inc_and_test - increment and test
 * @v: pointer of type oro_atomic_t
 *
 * Atomically increments @v by 1
 * and returns true if the result is zero, or false for all
 * other cases.
 */
static __forceinline int oro_atomic_inc_and_test(oro_atomic_t *v)
{
	return (_InterlockedIncrement((long *)v) == 0);
}

/**
 * oro_atomic_add_negative - add and test if negative
 * @v: pointer of type oro_atomic_t
 * @i: integer value to add
 *
 * Atomically adds @i to @v and returns true
 * if the result is negative, or false when
 * result is greater than or equal to zero.
 */
static __forceinline int oro_atomic_add_negative(int i, oro_atomic_t *v)
{
	return ((_InterlockedExchangeAdd((long *)v, i) + i) < 0);
}

/**
 * oro_atomic_add_return - add and return
 * @v: pointer of type oro_atomic_t
 * @i: integer value to add
 *
 * Atomically adds @i to @v and returns @i + @v
 */
static __forceinline int oro_atomic_add_return(int i, oro_atomic_t *v)
{
	return _InterlockedExchangeAdd((long *)v, i) + i;
}

static __forceinline int oro_atomic_sub_return(int i, oro_atomic_t *v)
{
	return oro_atomic_add_return(-i, v);
}

static __forceinline int oro_atomic_inc_return(oro_atomic_t *v)
{
	return _InterlockedIncrement((long *)v);
}

static __forceinline int oro_atomic_dec_return(oro_atomic_t *v)
{
	return _InterlockedDecrement((long *)v);
}

static __forceinline int oro_atomic_clear_mask(int mask, oro_atomic_t *v)
{
	return _InterlockedAnd((long *)v, ~mask);
}

static __forceinline int oro_atomic_set_mask(int mask, oro_atomic_t *v)
{
	return _InterlockedOr((long *)v, mask);
}

#endif
