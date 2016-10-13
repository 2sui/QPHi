/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#ifndef QP_ATOMIC_H
#define QP_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif


#include "qp_defines.h"
    
    
typedef qp_ulong_t        qp_atom_uint_i;
typedef qp_atom_uint_i    qp_atom_uint_t;
typedef qp_atom_uint_t    qp_atom_t;


# if defined(__INTEL_COMPILER)||(__GNUC__ > 4)||((__GNUC__ == 4)&&(__GNUC_MINOR__ >= 1))

/* GCC 4.1 builtin atomic operations */
static inline qp_atom_t qp_atom_fetch_add(qp_atom_t* atom_ptr, qp_atom_t add) 
{
    return (qp_atom_t)__sync_fetch_and_add(atom_ptr, add); /* atom++ */
}


static inline qp_atom_t qp_atom_fetch_sub(qp_atom_t* atom_ptr, qp_atom_t sub) 
{
    return (qp_atom_t)__sync_fetch_and_sub(atom_ptr, sub); /* atom-- */
}


static inline qp_atom_t qp_atom_add_fetch(qp_atom_t* atom_ptr, qp_atom_t add) 
{
    return (qp_atom_t)__sync_add_and_fetch(atom_ptr, add); /* ++atom */
}


static inline qp_atom_t qp_atom_sub_fetch(qp_atom_t* atom_ptr, qp_atom_t sub) 
{
    return (qp_atom_t)__sync_sub_and_fetch(atom_ptr, sub); /* --atom */
}


static inline bool qp_atom_cmp_set(qp_atom_t* atom_ptr, \
    qp_atom_t oldv, qp_atom_t newv) 
{
    return __sync_bool_compare_and_swap(atom_ptr, oldv, newv);  /* swap */
}


# define  qp_atom_barrier  __sync_synchronize

#  if defined(__i386__)|| defined(__i386)|| defined(__amd64__)|| defined(__amd64 )
static inline void  qp_cpu_pause() 
{  
    __asm__("pause");
}
#  else
#  define  qp_cpu_pause()
#  endif

# else

static inline qp_atom_t
qp_atom_fetch_add(qp_atom_t* atom_ptr, qp_atom_t add)
{
    qp_atom_t old = *atom_ptr;
    *atom_ptr += add;
    return old;
}


static inline qp_atom_t
qp_atom_fetch_sub(qp_atom_t* atom_ptr, qp_atom_t sub)
{
    qp_atom_t old = *atom_ptr;
    *atom_ptr -= sub;
    return old;
}


static inline qp_atom_t
qp_atom_cmp_set(qp_atom_t* atom_ptr, qp_atom_t oldv, \
    qp_atom_t newv)
{
    if (*atom_ptr == oldv) {
        *atom_ptr = newv;
        return 1;
    }
    
    return 0;
}

# define qp_cpu_pause()
# define qp_atom_barrier()

# endif


#  if (_POSIX_PRIORITY_SCHEDULING)
#  define qp_sched_yield  sched_yield
#  else
#  define qp_sched_yield()  usleep(1)
#  endif

static inline qp_atom_t  qp_atom_add(qp_atom_t* atom_ptr) 
{
    return qp_atom_fetch_add(atom_ptr, 1);
}


static inline qp_atom_t qp_atom_sub(qp_atom_t* atom_ptr)
{
    return qp_atom_fetch_sub(atom_ptr, 1);
}


static inline qp_atom_t qp_atom_fetch(qp_atom_t* atom_ptr)     
{
    return qp_atom_fetch_add(atom_ptr, 0);
}

#ifdef __cplusplus
}
#endif

#endif /* QP_ATOMIC_H */
