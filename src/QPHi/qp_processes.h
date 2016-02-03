
/**
  * Copyright (C) sui
  *
  * Basic thread/process operations.
  */


#ifndef QP_PROCESSES_H
#define QP_PROCESSES_H


#ifdef __cplusplus
extern "C" {
#endif


#include "qp_o_ipc.h"
    
    
#define QP_THREAD_INVALID        QP_IPC_INVALID
#define QP_PROCESS_INVALID       QP_IPC_INVALID
#define QP_PROCESS_STACK_SIZE    (8*1024*1024) /* system default */


struct  qp_thread_handler_s {
    void*          (*handler_ptr)(void*);
    void*          args_ptr;
    void*          ret;
};

struct qp_process_handler_s { 
    size_t              stack_size;
    void*               args_ptr;
    qp_int_t            (*handler_ptr)(void *);
    qp_char_t* const    *argv; 
    qp_int_t            flag; 
    qp_int_t            ret;
};

typedef struct qp_thread_handler_s    qp_thread_handler_t;

typedef struct qp_process_handler_s   qp_process_handler_t;


enum qp_process_type_e {
    QP_PROCESS_TYPE_UNKNOW,
    QP_PROCESS_TYPE_FORK,
    QP_PROCESS_TYPE_EXEC,
    QP_PROCESS_TYPE_FORK_EXEC,
    QP_PROCESS_TYPE_CLONE
};

typedef enum qp_process_type_e    qp_process_type_t;

struct  qp_thread_s {
    pthread_t                     tid;
    qp_thread_handler_t           handler;
    bool                          is_inited;
    bool                          is_alloced;
    bool                          is_detach;
    bool                          is_running;
};

typedef struct qp_thread_s        qp_thread_t;


struct qp_process_s {
    pid_t                         pid;
    qp_process_handler_t          handler;
    qp_process_type_t             type;
    qp_char_t*                    stack;
    bool                          is_inited;
    bool                          is_alloced;
    bool                          is_running;
};

typedef struct qp_process_s    qp_process_t;


inline bool
qp_thread_is_alloced(qp_thread_t* thread);

inline bool
qp_thread_is_inited(qp_thread_t* thread);

inline bool
qp_thread_is_detach(qp_thread_t* thread);

inline bool
qp_thread_is_running(qp_thread_t* thread);

inline bool
qp_process_is_alloced(qp_process_t* process);

inline bool
qp_process_is_inited(qp_process_t* process);

inline bool
qp_process_is_running(qp_process_t* process);


/*
 * Create an empty thread struct.If thread is NULL, it will allocate one.
*/
qp_thread_t*
qp_thread_create(qp_thread_t* thread);

/*
 * Init a thread struct.If detach is true, then the thread is detached.
*/
qp_thread_t*
qp_thread_init(qp_thread_t* thread, bool detach);   

/*
 * Destroy a thread struct.If thread is running it will wait for
*/
qp_int_t
qp_thread_destroy(qp_thread_t* thread);

/*
 * Start a thread.
*/
qp_int_t
qp_thread_start(qp_thread_t* thread, void* (*handler_ptr)(void*), void* arg);

/**
 * Wait for thread stop.
 */
qp_int_t
qp_thread_stop(qp_thread_t* thread);

/**
 * Get the return value pointer.
 */
void*
qp_thread_return(qp_thread_t* thread);


/**
 * Init a process struct.
 */
qp_process_t*
qp_process_init(qp_process_t* process);

/**
 * Destroy a process struct.
 */
qp_int_t
qp_process_destroy(qp_process_t* process);

/**
 * Create process using execv().
 */
qp_int_t
qp_process_set_exec(qp_process_t* process, qp_char_t* const *argv);

/**
 * Create process using vfork and execv.
 */
qp_int_t
qp_process_set_vfork_exec(qp_process_t* process, qp_char_t* const *argv);

/**
 * Create process using clone().
 */
qp_int_t
qp_process_set_clone(qp_process_t* process, qp_int_t flag, \
    int (*fn)(void*), void* arg, size_t stack_size);

/**
 * Start the process.
 */
pid_t
qp_process_start(qp_process_t* process);

/**
 * Wait for stopped process.If force is true , it wil kill the childprocess
 * if it is not stopped.
 */
qp_int_t
qp_process_stop(qp_process_t* process, bool force);

/**
 * Send a signal to process.
 */
qp_int_t
qp_process_kill(qp_process_t* process, qp_int_t sig);


#ifdef __cplusplus
}
#endif

#endif 