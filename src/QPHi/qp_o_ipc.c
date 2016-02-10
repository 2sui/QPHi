
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


inline bool
qp_lock_is_inited(qp_lock_t* lock)
{ return lock->is_inited; }

inline bool
qp_rwlock_is_inited(qp_rwlock_t* rwlock)
{ return rwlock->is_inited; }

inline bool
qp_cond_is_inited(qp_cond_t* cond)
{ return cond->is_inited; }

inline bool
qp_sem_is_inited(qp_sem_t* sem)
{ return sem->is_inited; }

inline bool
qp_shm_is_inited(qp_shm_t* shm)
{ return shm->is_inited; }

inline bool
qp_lock_is_alloced(qp_lock_t* lock)
{ return lock->is_alloced; }

inline bool
qp_rwlock_is_alloced(qp_rwlock_t* rwlock)
{ return rwlock->is_alloced; }

inline bool
qp_cond_is_alloced(qp_cond_t* cond)
{ return cond->is_alloced; }

inline bool
qp_sem_is_alloced(qp_sem_t* sem)
{ return sem->is_alloced; }

inline bool
qp_shm_is_alloced(qp_shm_t* shm)
{ return shm->is_alloced; }

inline bool
qp_lock_is_shared(qp_lock_t* lock)
{ return lock->is_shared; }

inline bool
qp_rwlock_is_shared(qp_rwlock_t* rwlock)
{ return rwlock->is_shared; }

inline bool
qp_cond_is_shared(qp_cond_t* cond)
{ return cond->is_shared; }

inline bool
qp_sem_is_shared(qp_sem_t* sem)
{ return sem->is_shared; }

inline bool
qp_lock_is_spin(qp_lock_t* lock)
{ return lock->is_spin; }
