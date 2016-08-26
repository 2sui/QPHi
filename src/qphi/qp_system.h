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


#ifndef QP_SYSTEM_H
#define QP_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif


#include "core/qp_defines.h"
#include <dlfcn.h>

    
# define  QP_FILENO_MAX    65535
# define  QP_LIB_LOAD      1
# define  QP_LIB_UNLOAD    0
# define  QP_LIMIT_GET     0
# define  QP_LIMIT_SET     1

    
typedef  struct qp_library_s*        qp_library_t;


# ifdef __USE_LARGEFILE64
typedef  struct rlimit64    qp_limit_t;
# else
typedef  struct rlimit      qp_limit_t;
# endif


/*
 * Daemonize the program. If bg is true the program will run in the backfound,
 *  otherwise run in terminal.
 * If holdfd is true, the stdin (0) stdout (1) stderr (2) will not be closed, 
 *  otherwise all the fd will be  closed.
 * If chdir is NULL, the work dir will not change,otherwise work dir will change 
 *  to chdir.
 * 
 * Return QP_SUCCESS If success ,otherwise return QP_ERROR.
*/
qp_int_t
qp_daemonize(qp_int_t bg, qp_int_t holdfd, const char* workdir, \
    const char* rootdir);


/**
 * Set system resource limit just like setrlimit/getrlimit.
 * If opt is QP_LIMIT_GET, limit will be filled with system limit info,
 * and if opt is QP_LIMIT_SET, system limit will be set.
 * Return QP_SUCCESS if success, otherwise return QP_ERROR.
 */
qp_int_t
qp_limit_opt(qp_int_t opt, __rlimit_resource_t source, qp_limit_t* limit);


/*
 * Check whether the effect user of the program is root.
 * If effect user is root ,return QP_SUCCESS, otherwise return QP_ERROR.
*/
qp_int_t
qp_check_root();


/*
 * Change the effect user to the specific uid (gid) or uname (gname).
 * The current user must be root before changing.
 * If success return ture, otherwise return false.
*/
qp_int_t
qp_change_user_by_id(uid_t uid, gid_t gid);


qp_int_t
qp_change_user_by_name(const qp_char_t* uname);


/*
 * Load dynamic library.
*/
qp_library_t
qp_library_load(const qp_char_t* name, qp_int_t flag);


/*
 * Get function ptr in loaded library.
*/
void*
qp_library_getFunc(qp_library_t libs, const qp_char_t* func);


/*
 * Unload library.
*/
void
qp_library_unload(qp_library_t libs);

#ifdef __cplusplus
}
#endif

#endif /* QP_SYSTEM_H */
