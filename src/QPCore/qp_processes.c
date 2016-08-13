
/**
  * Copyright (C) 2sui.
  */


#include "qp_processes.h"
#include "qp_o_memory.h"


struct  qp_thread_handler_s {
    void*                         (*handler_ptr)(void*);
    void*                         args_ptr;
    void*                         ret;
};

struct qp_process_handler_s { 
    size_t                        stack_size;
    void*                         args_ptr;
    qp_int_t                      (*handler_ptr)(void *);
    qp_char_t* const              *argv; 
    qp_int_t                      flag; 
    qp_int_t                      ret;
};

struct  qp_thread_s {
    pthread_t                     tid;
    struct qp_thread_handler_s    handler;
    bool                          is_inited;
    bool                          is_alloced;
    bool                          is_detach;
    bool                          is_running;
};

struct qp_process_s {
    pid_t                         pid;
    struct qp_process_handler_s   handler;
    qp_process_type_t             type;
    qp_char_t*                    stack;
    bool                          is_inited;
    bool                          is_alloced;
    bool                          is_running;
};


static inline void
qp_thread_set_inited(qp_thread_t thread)
{ 
    thread ? thread->is_inited = true : 1;
}

static inline void
qp_thread_set_alloced(qp_thread_t thread)
{ 
    thread ? thread->is_alloced = true : 1;
}

static inline void
qp_thread_set_detach(qp_thread_t thread)
{ 
    thread ? thread->is_detach = true : 1;
}

static inline void
qp_thread_set_running(qp_thread_t thread)
{ 
    thread ? thread->is_running = true : 1;
}

static inline void
qp_process_set_inited(qp_process_t process)
{ 
    process ? process->is_inited = true : 1;
}

static inline void
qp_process_set_alloced(qp_process_t process)
{ 
    process ? process->is_alloced = true : 1;
}

static inline void
qp_process_set_running(qp_process_t process)
{ 
    process ? process->is_running = true : 1;
}

static inline void
qp_thread_unset_inited(qp_thread_t thread)
{ 
    thread ? thread->is_inited = false : 1;
}

static inline void
qp_thread_unset_alloced(qp_thread_t thread)
{ 
    thread ? thread->is_alloced = false : 1;
}

static inline void
qp_thread_unset_detach(qp_thread_t thread)
{ 
    thread ? thread->is_detach = false : 1;
}

static inline void
qp_thread_unset_running(qp_thread_t thread)
{ 
    thread ? thread->is_running = false : 1;
}

static inline void
qp_process_unset_inited(qp_process_t process)
{ 
    process ? process->is_inited = false : 1;
}

static inline void
qp_process_unset_alloced(qp_process_t process)
{ 
    process ? process->is_alloced = false : 1;
}

static inline void
qp_process_unset_running(qp_process_t process)
{ 
    process ? process->is_running = false : 1;
}

static inline bool
qp_thread_is_alloced(qp_thread_t thread) 
{ 
    return thread ? thread->is_alloced : false;
}

static inline bool
qp_thread_is_inited(qp_thread_t thread) 
{ 
    return thread ? thread->is_inited : false;
}

static inline bool
qp_thread_is_detach(qp_thread_t thread) 
{ 
    return thread ? thread->is_detach : false;
}

static inline bool
qp_thread_is_running(qp_thread_t thread) 
{ 
    return thread ? thread->is_running : 1;
}

static inline bool
qp_process_is_alloced(qp_process_t process) 
{ 
    return process ? process->is_alloced : false;
}

static inline bool
qp_process_is_inited(qp_process_t process)
{ 
    return process ? process->is_inited : false;
}

static inline bool
qp_process_is_running(qp_process_t process)
{ 
    return process ? process->is_running : false;
}


qp_thread_t
qp_thread_create(qp_thread_t thread)
{
    if (NULL == thread) {
        thread = (qp_thread_t)qp_alloc(sizeof(struct qp_thread_s));
        
        if (NULL == thread) {
            return NULL;
        }

        memset(thread, 0, sizeof(struct qp_thread_s));
        qp_thread_set_alloced(thread);

    } else {
        memset(thread, 0, sizeof(struct qp_thread_s));
    }

    thread->tid  = QP_THREAD_INVALID;
    qp_thread_set_inited(thread);
    return thread;
}

qp_thread_t
qp_thread_init(qp_thread_t  thread, bool detach)
{
    thread = qp_thread_create(thread);
    
    if (NULL == thread) {
        return NULL;
    }

    detach ? qp_thread_set_detach(thread) : 1;
    return thread;
}

qp_int_t
qp_thread_destroy(qp_thread_t  thread)
{
    if (qp_thread_is_inited(thread)) {
        
        if (qp_thread_is_running(thread)) {
            
            if (QP_ERROR == qp_thread_stop(thread)) {
                return QP_ERROR;
            }
        }
        
        qp_thread_unset_inited(thread);

        if (qp_thread_is_alloced(thread)) {
            qp_free(thread);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

static void* 
qp_thread_runner(void *arg)
{
    qp_thread_t thread = (qp_thread_t)arg;
    
    if (thread->tid != pthread_self()) {
        goto end;
    }
    
    if (qp_thread_is_detach(thread)) {
        
        if (QP_SUCCESS != pthread_detach(thread->tid)) {
            qp_thread_unset_detach(thread);
        }
    }
    
    qp_thread_set_running(thread);
    
    if (thread->handler.handler_ptr) {
        thread->handler.ret = \
            thread->handler.handler_ptr(thread->handler.args_ptr);
    }
    
end:
    qp_thread_unset_running(thread);
    pthread_exit(NULL);
    return  NULL;
}

qp_int_t
qp_thread_start(qp_thread_t thread, void* (*handler_ptr)(void*), void* arg)
{
    if (qp_thread_is_inited(thread)) {
        thread->handler.handler_ptr = handler_ptr;
        thread->handler.args_ptr = arg;
        
        if (QP_SUCCESS != pthread_create(&thread->tid, NULL, \
            qp_thread_runner, thread))
        {
            thread->tid = QP_THREAD_INVALID;
            thread->handler.handler_ptr = NULL;
            thread->handler.args_ptr = NULL;
            return QP_ERROR;
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_thread_stop(qp_thread_t thread)
{
    if (qp_thread_is_inited(thread)) {
        
        if (!qp_thread_is_detach(thread)) {
            pthread_join(thread->tid, NULL);
            
        } else {
            if (qp_thread_is_running(thread)) {
                return QP_ERROR;
            }
        }
        
        thread->tid = QP_THREAD_INVALID;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

void* 
qp_thread_return(qp_thread_t thread)
{
    if (qp_thread_is_inited(thread)) {
        return thread->handler.ret;
    }
    
    return NULL;
}

qp_process_t
qp_process_create(qp_process_t process) 
{
    if (NULL == process) {
        process = (qp_process_t)qp_alloc(sizeof(struct qp_process_s));
        
        if (NULL == process) {
            return NULL;
        }
        
        memset(process, 0, sizeof(struct qp_process_s));
        qp_process_set_alloced(process);
        
    } else {
        memset(process, 0, sizeof(struct qp_process_s));
    }
    
    process->pid = QP_PROCESS_INVALID;
    process->type = QP_PROCESS_TYPE_UNKNOW;
    qp_process_set_inited(process);
    return process;
}

qp_process_t
qp_process_init(qp_process_t process)
{
    process = qp_process_create(process);
    
    if (NULL == process) {
        return NULL;
    }
    
    process->type = QP_PROCESS_TYPE_FORK;
    return process;
}

qp_int_t
qp_process_destroy(qp_process_t process)
{
    if (qp_process_is_inited(process)) {
        
        if (qp_process_is_running(process)) {
            qp_process_stop(process, true);
        }
        
        if (process->stack) {
            qp_free(process->stack);
            process->stack = NULL;
        }
        
        process->pid = QP_PROCESS_INVALID;
        process->type = QP_PROCESS_TYPE_UNKNOW;
        qp_process_unset_inited(process);
        
        if (qp_process_is_alloced(process)) {
            qp_free(process);
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}


qp_int_t
qp_process_set_exec(qp_process_t process, qp_char_t* const *argv)
{
    if (qp_process_is_inited(process) && argv && argv[0]) {
        process->handler.argv = argv;
        process->type = QP_PROCESS_TYPE_EXEC;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_process_set_vfork_exec(qp_process_t process, qp_char_t* const *argv)
{
    if (qp_process_is_inited(process) && argv && argv[0]) {
        process->handler.argv = argv;
        process->type = QP_PROCESS_TYPE_FORK_EXEC;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_process_set_clone(qp_process_t process, qp_int_t flag, \
    int (*fn)(void*), void* arg, size_t stack_size)
{
    if (qp_process_is_inited(process) && fn) {
        process->handler.flag = flag;
        process->handler.stack_size = stack_size;
        process->handler.args_ptr = arg;
        process->handler.handler_ptr = fn;
        process->type = QP_PROCESS_TYPE_CLONE;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

pid_t
qp_process_start(qp_process_t process)
{
    if (qp_process_is_inited(process)) {
        
        switch (process->type) {
            
            case QP_PROCESS_TYPE_FORK: {
                process->pid = fork();
                
                switch (process->pid) {
                    
                    case -1: {
                        return QP_ERROR;
                    }break;
                    
                    case 0: {
                        return process->pid;
                    }break;
                    
                    default: {
                        qp_process_set_running(process);
                        return process->pid;
                    }
                }
                
            }break;
            
            case QP_PROCESS_TYPE_EXEC: {
                
                if (!process->handler.argv) {
                    return QP_ERROR;
                }
                
                execv(process->handler.argv[0], process->handler.argv);
                return QP_ERROR;
                
            }break;
            
            case QP_PROCESS_TYPE_FORK_EXEC: {
                
                if (!process->handler.argv) {
                    return QP_ERROR;
                }
                
                process->pid = vfork();
                
                switch (process->pid) {
                    
                    case -1: {
                        return QP_ERROR;
                    }break;
                    
                    case 0: {
                        execv(process->handler.argv[0], process->handler.argv);
                        exit(1);
                    }break;
                    
                    default: {
                        qp_process_set_running(process);
                        return process->pid;
                    }
                }
                
            }break;
            
            case QP_PROCESS_TYPE_CLONE: {
                
                if (!process->handler.stack_size) {
                    process->handler.stack_size = QP_PROCESS_STACK_SIZE;
                }
                
                if (NULL != process->stack) {
                    qp_free(process->stack);
                    process->stack = NULL;
                }
                
                process->stack = \
                    (qp_char_t*)qp_alloc(process->handler.stack_size);
                
                if (NULL == process->stack) {
                    return QP_ERROR;
                }
                
                process->pid = clone(process->handler.handler_ptr, \
                    process->stack + process->handler.stack_size,
                    process->handler.flag | SIGCHLD, \
                    process->handler.args_ptr);
                
                if (QP_ERROR == process->pid) {
                    return QP_ERROR;
                }
                
                qp_process_set_running(process);
                return process->pid;
                
            }break;
            
            default: {
            }
        }
    }
    
    return QP_ERROR;
}

qp_int_t
qp_process_stop(qp_process_t process, bool force)
{
    if (qp_process_is_inited(process)) {
        
        if (qp_process_is_running(process)) {
            pid_t rets = 0;
            
            if (force) {
                
                if (QP_ERROR == qp_process_kill(process, SIGKILL)) {
                    return QP_ERROR;
                }
            }
            
            if (process->pid != \
                (rets = waitpid(process->pid, &(process->handler.ret), 0))) 
            {
                if(ECHILD == errno) {
//                    qp_process_unset_running(process);
//                    return QP_ERROR;
                }
                
//                return QP_ERROR;
            }
            
            qp_process_unset_running(process);
        }
        
        process->pid = QP_PROCESS_INVALID;
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

qp_int_t
qp_process_kill(qp_process_t process, qp_int_t sig)
{
    if (qp_process_is_running(process)) {
        
        if (QP_ERROR == kill(process->pid, sig)) {
            return QP_ERROR;
        }
        
        return QP_SUCCESS;
    }
    
    return QP_ERROR;
}

pid_t
qp_process_pid(qp_process_t process) {
    if (qp_process_is_inited(process)) {
        return process->pid;
    }
    
    return QP_PROCESS_INVALID;
}
