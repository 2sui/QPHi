
/**
  * Copyright (C) 2sui.
  *
  * Atomic operation with gcc 4.1 or later.
  */


#ifndef QP_O_ATOMIC_H
#define QP_O_ATOMIC_H

#include "qp_o_typedef.h"

typedef qp_ulong_t       qp_atom_uint_i;
typedef qp_atom_uint_i            qp_atom_uint_t;
typedef qp_atom_uint_t            qp_atom_t;

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

static inline qp_long_t qp_cpu_num() {
    return ((sysconf(_SC_NPROCESSORS_ONLN) > 1) ? \
        sysconf(_SC_NPROCESSORS_ONLN) : 1);
}

#endif 
