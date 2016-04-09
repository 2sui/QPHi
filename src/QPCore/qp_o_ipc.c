
/**
  * Copyright (C) 2sui.
  */


#include "qp_o_ipc.h"


inline bool
qp_lock_is_inited(qp_lock_t* lock)
{ return lock ? lock->is_inited : false; }

inline bool
qp_rwlock_is_inited(qp_rwlock_t* rwlock)
{ return rwlock ? rwlock->is_inited : false; }

inline bool
qp_cond_is_inited(qp_cond_t* cond)
{ return cond ? cond->is_inited : false; }

inline bool
qp_sem_is_inited(qp_sem_t* sem)
{ return sem ? sem->is_inited : false; }

inline bool
qp_shm_is_inited(qp_shm_t* shm)
{ return shm ? shm->is_inited : false; }

inline bool
qp_lock_is_alloced(qp_lock_t* lock)
{ return lock ? lock->is_alloced : false; }

inline bool
qp_rwlock_is_alloced(qp_rwlock_t* rwlock)
{ return rwlock ? rwlock->is_alloced : false; }

inline bool
qp_cond_is_alloced(qp_cond_t* cond)
{ return cond ? cond->is_alloced : false; }

inline bool
qp_sem_is_alloced(qp_sem_t* sem)
{ return sem ? sem->is_alloced : false; }

inline bool
qp_shm_is_alloced(qp_shm_t* shm)
{ return shm ? shm->is_alloced : false; }

inline bool
qp_lock_is_shared(qp_lock_t* lock)
{ return lock ? lock->is_shared : false; }

inline bool
qp_rwlock_is_shared(qp_rwlock_t* rwlock)
{ return rwlock ? rwlock->is_shared : false; }

inline bool
qp_cond_is_shared(qp_cond_t* cond)
{ return cond ? cond->is_shared : false; }

inline bool
qp_sem_is_shared(qp_sem_t* sem)
{ return sem ? sem->is_shared : false; }

inline bool
qp_lock_is_spin(qp_lock_t* lock)
{ return lock ? lock->is_spin : false; }
