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


#ifndef QP_PROCESS_H
#define QP_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif


#include "core/qp_defines.h"

    
# define QP_THREAD_INVALID        QP_ERROR
# define QP_PROCESS_INVALID       QP_ERROR
# define QP_PROCESS_STACK_SIZE    8388608 /* system default 8M */

    
enum qp_process_type_e {
    QP_PROCESS_TYPE_UNKNOW,
    QP_PROCESS_TYPE_FORK,
    QP_PROCESS_TYPE_EXEC,
    QP_PROCESS_TYPE_FORK_EXEC,
    QP_PROCESS_TYPE_CLONE
};


typedef enum qp_process_type_e    qp_process_type_t;
typedef struct qp_thread_s*       qp_thread_t;
typedef struct qp_process_s*      qp_process_t;
    

/*
 * Init a thread struct.If detach is true, then the thread is detached.
*/
qp_thread_t
qp_thread_init(qp_thread_t thread, bool detach);  


/*
 * Destroy a thread struct.If thread is running it will wait for
*/
qp_int_t
qp_thread_destroy(qp_thread_t thread);


/*
 * Start a thread.
*/
qp_int_t
qp_thread_start(qp_thread_t thread, void* (*handler_ptr)(void*), void* arg);


/**
 * Wait for thread stop.
 */
qp_int_t
qp_thread_stop(qp_thread_t thread);


/**
 * Get the return value pointer.
 */
void*
qp_thread_return(qp_thread_t thread);



/**
 * Init a process struct.
 */
qp_process_t
qp_process_init(qp_process_t process);


/**
 * Destroy a process struct.
 */
qp_int_t
qp_process_destroy(qp_process_t process);


/**
 * Create process using execv().
 */
qp_int_t
qp_process_set_exec(qp_process_t process, qp_char_t* const *argv);


/**
 * Create process using vfork and execv.
 */
qp_int_t
qp_process_set_vfork_exec(qp_process_t process, qp_char_t* const *argv);


/**
 * Create process using clone().
 */
qp_int_t
qp_process_set_clone(qp_process_t process, qp_int_t flag, \
    int (*fn)(void*), void* arg, size_t stack_size);


/**
 * Start the process.
 */
pid_t
qp_process_start(qp_process_t process);


/**
 * Wait for stopped process.If force is true , it wil kill the childprocess
 * if it is not stopped.
 */
qp_int_t
qp_process_stop(qp_process_t process, bool force);


/**
 * Send a signal to process.
 */
qp_int_t
qp_process_kill(qp_process_t process, qp_int_t sig);


pid_t
qp_process_pid(qp_process_t process);

#ifdef __cplusplus
}
#endif

#endif /* QP_PROCESS_H */

