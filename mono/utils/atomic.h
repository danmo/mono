/*
 * atomic.h:  Atomic operations
 *
 * Author:
 *	Dick Porter (dick@ximian.com)
 *
 * (C) 2002 Ximian, Inc.
 * Copyright 2012 Xamarin Inc
 */

#ifndef _WAPI_ATOMIC_H_
#define _WAPI_ATOMIC_H_

#include "config.h"
#include <glib.h>

#ifdef ENABLE_EXTENSION_MODULE
#include "../../../mono-extensions/mono/utils/atomic.h"
#endif

/* On Windows, we always use the functions provided by the Windows API. */
#if defined(__WIN32__) || defined(_WIN32)

#include <windows.h>

/* mingw is missing InterlockedCompareExchange64 () from winbase.h */
#if HAVE_DECL_INTERLOCKEDCOMPAREEXCHANGE64==0
static inline gint64 InterlockedCompareExchange64(volatile gint64 *dest, gint64 exch, gint64 comp)
{
	return __sync_val_compare_and_swap (dest, comp, exch);
}
#endif

/* mingw is missing InterlockedExchange64 () from winbase.h */
#if HAVE_DECL_INTERLOCKEDEXCHANGE64==0
static inline gint64 InterlockedExchange64(volatile gint64 *val, gint64 new_val)
{
	gint64 old_val;
	do {
		old_val = *val;
	} while (InterlockedCompareExchange64 (val, new_val, old_val) != old_val);
	return old_val;
}
#endif

/* mingw is missing InterlockedIncrement64 () from winbase.h */
#if HAVE_DECL_INTERLOCKEDINCREMENT64==0
static inline gint64 InterlockedIncrement64(volatile gint64 *val)
{
	return __sync_add_and_fetch (val, 1);
}
#endif

/* mingw is missing InterlockedDecrement64 () from winbase.h */
#if HAVE_DECL_INTERLOCKEDDECREMENT64==0
static inline gint64 InterlockedDecrement64(volatile gint64 *val)
{
	return __sync_sub_and_fetch (val, 1);
}
#endif

/* mingw is missing InterlockedAdd () from winbase.h */
#if HAVE_DECL_INTERLOCKEDADD==0
static inline gint32 InterlockedAdd(volatile gint32 *dest, gint32 add)
{
	return __sync_add_and_fetch (dest, add);
}
#endif

/* mingw is missing InterlockedAdd64 () from winbase.h */
#if HAVE_DECL_INTERLOCKEDADD64==0
static inline gint64 InterlockedAdd64(volatile gint64 *dest, gint64 add)
{
	return __sync_add_and_fetch (dest, add);
}
#endif

/* And now for some dirty hacks... The Windows API doesn't
 * provide any useful primitives for this (other than getting
 * into architecture-specific madness), so use CAS. */

static inline gint32 InterlockedRead(volatile gint32 *src)
{
	return InterlockedCompareExchange (src, 0, 0);
}

static inline gint64 InterlockedRead64(volatile gint64 *src)
{
	return InterlockedCompareExchange64 (src, 0, 0);
}

static inline void InterlockedWrite(volatile gint32 *dst, gint32 val)
{
	InterlockedExchange (dst, val);
}

static inline void InterlockedWrite64(volatile gint64 *dst, gint64 val)
{
	InterlockedExchange64 (dst, val);
}

/* Prefer GCC atomic ops if the target supports it (see configure.in). */
#elif defined(USE_GCC_ATOMIC_OPS)

static inline gint32 InterlockedCompareExchange(volatile gint32 *dest,
						gint32 exch, gint32 comp)
{
	return __sync_val_compare_and_swap (dest, comp, exch);
}

static inline gpointer InterlockedCompareExchangePointer(volatile gpointer *dest, gpointer exch, gpointer comp)
{
	return __sync_val_compare_and_swap (dest, comp, exch);
}

static inline gint32 InterlockedAdd(volatile gint32 *dest, gint32 add)
{
	return __sync_add_and_fetch (dest, add);
}

static inline gint32 InterlockedIncrement(volatile gint32 *val)
{
	return __sync_add_and_fetch (val, 1);
}

static inline gint32 InterlockedDecrement(volatile gint32 *val)
{
	return __sync_sub_and_fetch (val, 1);
}

static inline gint32 InterlockedExchange(volatile gint32 *val, gint32 new_val)
{
	gint32 old_val;
	do {
		old_val = *val;
	} while (__sync_val_compare_and_swap (val, old_val, new_val) != old_val);
	return old_val;
}

static inline gpointer InterlockedExchangePointer(volatile gpointer *val,
						  gpointer new_val)
{
	gpointer old_val;
	do {
		old_val = *val;
	} while (__sync_val_compare_and_swap (val, old_val, new_val) != old_val);
	return old_val;
}

static inline gint32 InterlockedExchangeAdd(volatile gint32 *val, gint32 add)
{
	return __sync_fetch_and_add (val, add);
}

static inline gint32 InterlockedRead(volatile gint32 *src)
{
	/* Kind of a hack, but GCC doesn't give us anything better, and it's
	   certainly not as bad as using a CAS loop. */
	return __sync_fetch_and_add (src, 0);
}

static inline void InterlockedWrite(volatile gint32 *dst, gint32 val)
{
	/* Nothing useful from GCC at all, so fall back to CAS. */
	InterlockedExchange (dst, val);
}

#if defined (TARGET_OSX) || defined (__arm__) || (defined (__mips__) && !defined (__mips64)) || (defined (__powerpc__) && !defined (__powerpc64__))
#define BROKEN_64BIT_ATOMICS_INTRINSIC 1
#endif

#if !defined (BROKEN_64BIT_ATOMICS_INTRINSIC)

static inline gint64 InterlockedCompareExchange64(volatile gint64 *dest, gint64 exch, gint64 comp)
{
	return __sync_val_compare_and_swap (dest, comp, exch);
}

static inline gint64 InterlockedAdd64(volatile gint64 *dest, gint64 add)
{
	return __sync_add_and_fetch (dest, add);
}

static inline gint64 InterlockedIncrement64(volatile gint64 *val)
{
	return __sync_add_and_fetch (val, 1);
}

static inline gint64 InterlockedDecrement64(volatile gint64 *val)
{
	return __sync_sub_and_fetch (val, 1);
}

static inline gint64 InterlockedExchangeAdd64(volatile gint64 *val, gint64 add)
{
	return __sync_fetch_and_add (val, add);
}

static inline gint64 InterlockedRead64(volatile gint64 *src)
{
	/* Kind of a hack, but GCC doesn't give us anything better. */
	return __sync_fetch_and_add (src, 0);
}

#else

/* Implement 64-bit cmpxchg by hand or emulate it. */
extern gint64 InterlockedCompareExchange64(volatile gint64 *dest, gint64 exch, gint64 comp);

/* Implement all other 64-bit atomics in terms of a specialized CAS
 * in this case, since chances are that the other 64-bit atomic
 * intrinsics are broken too.
 */

static inline gint64 InterlockedExchangeAdd64(volatile gint64 *dest, gint64 add)
{
	gint64 old_val;
	do {
		old_val = *dest;
	} while (InterlockedCompareExchange64 (dest, old_val + add, old_val) != old_val);
	return old_val;
}

static inline gint64 InterlockedIncrement64(volatile gint64 *val)
{
	gint64 get, set;
	do {
		get = *val;
		set = get + 1;
	} while (InterlockedCompareExchange64 (val, set, get) != get);
	return set;
}

static inline gint64 InterlockedDecrement64(volatile gint64 *val)
{
	gint64 get, set;
	do {
		get = *val;
		set = get - 1;
	} while (InterlockedCompareExchange64 (val, set, get) != get);
	return set;
}

static inline gint64 InterlockedAdd64(volatile gint64 *dest, gint64 add)
{
	gint64 get, set;
	do {
		get = *dest;
		set = get + add;
	} while (InterlockedCompareExchange64 (dest, set, get) != get);
	return set;
}

static inline gint64 InterlockedRead64(volatile gint64 *src)
{
	return InterlockedCompareExchange64 (src, 0, 0);
}

#endif

/* We always implement this in terms of a 64-bit cmpxchg since
 * GCC doesn't have an intrisic to model it anyway. */
static inline gint64 InterlockedExchange64(volatile gint64 *val, gint64 new_val)
{
	gint64 old_val;
	do {
		old_val = *val;
	} while (InterlockedCompareExchange64 (val, new_val, old_val) != old_val);
	return old_val;
}

static inline void InterlockedWrite64(volatile gint64 *dst, gint64 val)
{
	/* Nothing useful from GCC at all, so fall back to CAS. */
	InterlockedExchange64 (dst, val);
}

#elif (defined(sparc) || defined (__sparc__)) && defined(__GNUC__)

G_GNUC_UNUSED 
static inline gint32 InterlockedCompareExchange(volatile gint32 *_dest, gint32 _exch, gint32 _comp)
{
       register volatile gint32 *dest asm("g1") = _dest;
       register gint32 comp asm("o4") = _comp;
       register gint32 exch asm("o5") = _exch;

       __asm__ __volatile__(
               /* cas [%%g1], %%o4, %%o5 */
               ".word 0xdbe0500c"
               : "=r" (exch)
               : "0" (exch), "r" (dest), "r" (comp)
               : "memory");

       return exch;
}

G_GNUC_UNUSED 
static inline gpointer InterlockedCompareExchangePointer(volatile gpointer *_dest, gpointer _exch, gpointer _comp)
{
       register volatile gpointer *dest asm("g1") = _dest;
       register gpointer comp asm("o4") = _comp;
       register gpointer exch asm("o5") = _exch;

       __asm__ __volatile__(
#ifdef SPARCV9
               /* casx [%%g1], %%o4, %%o5 */
               ".word 0xdbf0500c"
#else
               /* cas [%%g1], %%o4, %%o5 */
               ".word 0xdbe0500c"
#endif
               : "=r" (exch)
               : "0" (exch), "r" (dest), "r" (comp)
               : "memory");

       return exch;
}

G_GNUC_UNUSED 
static inline gint32 InterlockedIncrement(volatile gint32 *_dest)
{
       register volatile gint32 *dest asm("g1") = _dest;
       register gint32 tmp asm("o4");
       register gint32 ret asm("o5");

       __asm__ __volatile__(
               "1:     ld      [%%g1], %%o4\n\t"
               "       add     %%o4, 1, %%o5\n\t"
               /*      cas     [%%g1], %%o4, %%o5 */
               "       .word   0xdbe0500c\n\t"
               "       cmp     %%o4, %%o5\n\t"
               "       bne     1b\n\t"
               "        add    %%o5, 1, %%o5"
               : "=&r" (tmp), "=&r" (ret)
               : "r" (dest)
               : "memory", "cc");

        return ret;
}

G_GNUC_UNUSED 
static inline gint32 InterlockedDecrement(volatile gint32 *_dest)
{
       register volatile gint32 *dest asm("g1") = _dest;
       register gint32 tmp asm("o4");
       register gint32 ret asm("o5");

       __asm__ __volatile__(
               "1:     ld      [%%g1], %%o4\n\t"
               "       sub     %%o4, 1, %%o5\n\t"
               /*      cas     [%%g1], %%o4, %%o5 */
               "       .word   0xdbe0500c\n\t"
               "       cmp     %%o4, %%o5\n\t"
               "       bne     1b\n\t"
               "        sub    %%o5, 1, %%o5"
               : "=&r" (tmp), "=&r" (ret)
               : "r" (dest)
               : "memory", "cc");

        return ret;
}

G_GNUC_UNUSED
static inline gint32 InterlockedExchange(volatile gint32 *_dest, gint32 exch)
{
       register volatile gint32 *dest asm("g1") = _dest;
       register gint32 tmp asm("o4");
       register gint32 ret asm("o5");

       __asm__ __volatile__(
               "1:     ld      [%%g1], %%o4\n\t"
               "       mov     %3, %%o5\n\t"
               /*      cas     [%%g1], %%o4, %%o5 */
               "       .word   0xdbe0500c\n\t"
               "       cmp     %%o4, %%o5\n\t"
               "       bne     1b\n\t"
               "        nop"
               : "=&r" (tmp), "=&r" (ret)
               : "r" (dest), "r" (exch)
               : "memory", "cc");

        return ret;
}

G_GNUC_UNUSED
static inline gpointer InterlockedExchangePointer(volatile gpointer *_dest, gpointer exch)
{
       register volatile gpointer *dest asm("g1") = _dest;
       register gpointer tmp asm("o4");
       register gpointer ret asm("o5");

       __asm__ __volatile__(
#ifdef SPARCV9
               "1:     ldx     [%%g1], %%o4\n\t"
#else
               "1:     ld      [%%g1], %%o4\n\t"
#endif
               "       mov     %3, %%o5\n\t"
#ifdef SPARCV9
               /*      casx    [%%g1], %%o4, %%o5 */
               "       .word   0xdbf0500c\n\t"
#else
               /*      cas     [%%g1], %%o4, %%o5 */
               "       .word   0xdbe0500c\n\t"
#endif
               "       cmp     %%o4, %%o5\n\t"
               "       bne     1b\n\t"
               "        nop"
               : "=&r" (tmp), "=&r" (ret)
               : "r" (dest), "r" (exch)
               : "memory", "cc");

        return ret;
}

G_GNUC_UNUSED
static inline gint32 InterlockedExchangeAdd(volatile gint32 *_dest, gint32 add)
{
       register volatile gint32 *dest asm("g1") = _dest;
       register gint32 tmp asm("o4");
       register gint32 ret asm("o5");

       __asm__ __volatile__(
               "1:     ld      [%%g1], %%o4\n\t"
               "       add     %%o4, %3, %%o5\n\t"
               /*      cas     [%%g1], %%o4, %%o5 */
               "       .word   0xdbe0500c\n\t"
               "       cmp     %%o4, %%o5\n\t"
               "       bne     1b\n\t"
               "        add    %%o5, %3, %%o5"
               : "=&r" (tmp), "=&r" (ret)
               : "r" (dest), "r" (add)
               : "memory", "cc");

        return ret;
}

#elif defined(__ia64__)

#ifdef __INTEL_COMPILER
#include <ia64intrin.h>
#endif

static inline gint32 InterlockedCompareExchange(gint32 volatile *dest,
						gint32 exch, gint32 comp)
{
	gint32 old;
	guint64 real_comp;

#ifdef __INTEL_COMPILER
	old = _InterlockedCompareExchange (dest, exch, comp);
#else
	/* cmpxchg4 zero extends the value read from memory */
	real_comp = (guint64)(guint32)comp;
	asm volatile ("mov ar.ccv = %2 ;;\n\t"
				  "cmpxchg4.acq %0 = [%1], %3, ar.ccv\n\t"
				  : "=r" (old) : "r" (dest), "r" (real_comp), "r" (exch));
#endif

	return(old);
}

static inline gpointer InterlockedCompareExchangePointer(gpointer volatile *dest,
						gpointer exch, gpointer comp)
{
	gpointer old;

#ifdef __INTEL_COMPILER
	old = _InterlockedCompareExchangePointer (dest, exch, comp);
#else
	asm volatile ("mov ar.ccv = %2 ;;\n\t"
				  "cmpxchg8.acq %0 = [%1], %3, ar.ccv\n\t"
				  : "=r" (old) : "r" (dest), "r" (comp), "r" (exch));
#endif

	return(old);
}

static inline gint32 InterlockedIncrement(gint32 volatile *val)
{
#ifdef __INTEL_COMPILER
	return _InterlockedIncrement (val);
#else
	gint32 old;

	do {
		old = *val;
	} while (InterlockedCompareExchange (val, old + 1, old) != old);

	return old + 1;
#endif
}

static inline gint32 InterlockedDecrement(gint32 volatile *val)
{
#ifdef __INTEL_COMPILER
	return _InterlockedDecrement (val);
#else
	gint32 old;

	do {
		old = *val;
	} while (InterlockedCompareExchange (val, old - 1, old) != old);

	return old - 1;
#endif
}

static inline gint32 InterlockedExchange(gint32 volatile *dest, gint32 new_val)
{
#ifdef __INTEL_COMPILER
	return _InterlockedExchange (dest, new_val);
#else
	gint32 res;

	do {
		res = *dest;
	} while (InterlockedCompareExchange (dest, new_val, res) != res);

	return res;
#endif
}

static inline gpointer InterlockedExchangePointer(gpointer volatile *dest, gpointer new_val)
{
#ifdef __INTEL_COMPILER
	return (gpointer)_InterlockedExchange64 ((gint64*)dest, (gint64)new_val);
#else
	gpointer res;

	do {
		res = *dest;
	} while (InterlockedCompareExchangePointer (dest, new_val, res) != res);

	return res;
#endif
}

static inline gint32 InterlockedExchangeAdd(gint32 volatile *val, gint32 add)
{
	gint32 old;

#ifdef __INTEL_COMPILER
	old = _InterlockedExchangeAdd (val, add);
#else
	do {
		old = *val;
	} while (InterlockedCompareExchange (val, old + add, old) != old);

	return old;
#endif
}

#else

#define WAPI_NO_ATOMIC_ASM

extern gint32 InterlockedCompareExchange(volatile gint32 *dest, gint32 exch, gint32 comp);
extern gint64 InterlockedCompareExchange64(volatile gint64 *dest, gint64 exch, gint64 comp);
extern gpointer InterlockedCompareExchangePointer(volatile gpointer *dest, gpointer exch, gpointer comp);
extern gint32 InterlockedAdd(volatile gint32 *dest, gint32 add);
extern gint64 InterlockedAdd64(volatile gint64 *dest, gint64 add);
extern gint32 InterlockedIncrement(volatile gint32 *dest);
extern gint64 InterlockedIncrement64(volatile gint64 *dest);
extern gint32 InterlockedDecrement(volatile gint32 *dest);
extern gint64 InterlockedDecrement64(volatile gint64 *dest);
extern gint32 InterlockedExchange(volatile gint32 *dest, gint32 exch);
extern gint64 InterlockedExchange64(volatile gint64 *dest, gint64 exch);
extern gpointer InterlockedExchangePointer(volatile gpointer *dest, gpointer exch);
extern gint32 InterlockedExchangeAdd(volatile gint32 *dest, gint32 add);
extern gint64 InterlockedExchangeAdd64(volatile gint64 *dest, gint64 add);
extern gint32 InterlockedRead(volatile gint32 *src);
extern gint64 InterlockedRead64(volatile gint64 *src);
extern void InterlockedWrite(volatile gint32 *dst, gint32 val);
extern void InterlockedWrite64(volatile gint64 *dst, gint64 val);

#endif

#endif /* _WAPI_ATOMIC_H_ */
