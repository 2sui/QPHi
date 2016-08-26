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


#include "qp_system.h"
#include "core/qp_memory_core.h"


struct  qp_library_s {
    void*         handler;
    qp_char_t*    error;
    qp_int_t      stat;
};


qp_int_t
qp_daemonize(qp_int_t bg, qp_int_t holdfd, const char* workdir, 
    const char* rootdir)
{
    pid_t          pid;
    struct rlimit  limits;

    /* clear file mask */
    umask(0);

    if (bg) {
        /* ignore hup signal */
//        signal(SIGHUP, SIG_IGN);
        
        /* fork process for new session */
        if (0 > (pid = fork())) {
            /* fork error */
            return QP_ERROR;
        }

        /* exit parent */
        if (0 < pid) {
            _exit(0);
        }

        /* new session id */
        setsid();

        /* ignore hup signal */
        signal(SIGHUP, SIG_IGN);

        /* ensure that no tty will be got */
        if (0 > (pid = fork())) {
            return QP_ERROR;
        }

        if (0 < pid) {
           _exit(0);
        }
    }
    
    /* change root path */
    if (rootdir) {
        
        if (QP_SUCCESS != chroot(rootdir)) {
            return QP_ERROR;
        }
    }

    /* change work path */
    if (workdir) {

        if (QP_SUCCESS != chdir(workdir)) {
            return QP_ERROR;
        }
    }

    /* Get max number of file descriptors */
    if (0 != getrlimit(RLIMIT_NOFILE, &limits)) {
        /* can not get file limit */
        return QP_ERROR;
    }

    /* set sys resource limit and close fd */
    qp_uint64_t  i = 3;
    if(RLIM_INFINITY == limits.rlim_max) {

        for (; i < QP_FILENO_MAX; i++) {
            close(i);
        }

    } else {

        for (i = 3; i < limits.rlim_max; i++) {
            close(i);
        }
    }

    /* if do not hold stdin/stdout/stderr */
    if (!holdfd) {
        close(0);
        close(1);
        close(2);

        open("/dev/null", O_RDONLY);
        open("/dev/null", O_RDWR);
        open("/dev/null", O_RDWR);
    }

    return true;
}


qp_int_t
qp_limit_opt(qp_int_t opt, __rlimit_resource_t source, qp_limit_t* limit)
{
    if (!limit) {
        return QP_ERROR;
    }
    
    if (opt == QP_LIMIT_GET) {
#ifdef __USE_LARGEFILE64
        return getrlimit64(source, limit);
#else 
        return getrlimit(source, limit);
#endif
    }
    
#ifdef __USE_LARGEFILE64
        return setrlimit64(source, limit);
#else 
        return setrlimit(source, limit);
#endif
}


qp_int_t
qp_check_root()
{
    uid_t uid = getuid();
    gid_t gid = getgid();

    if ((0 == uid) && (0 == gid)) {
        return QP_SUCCESS;

    } else {
        return QP_ERROR;
    }
}


qp_int_t
qp_change_user_by_id(uid_t uid, gid_t gid)
{

    if (!qp_check_root()) {

        if ((0 == uid) && (0 == gid)) {
            return QP_ERROR;
        }

        if (0 > setgid(gid) || 0 > setuid(uid)) {
            return QP_ERROR;
        }

        return QP_SUCCESS;

    } else {
        return QP_ERROR;
    }
}


qp_int_t
qp_change_user_by_name(const char *uname)
{
    uid_t uid = 0;
    gid_t gid = 0;
    struct passwd *usr = getpwnam(uname);

    if (NULL != usr) {
        uid = usr->pw_uid;
        gid = usr->pw_gid;

    } else {
        return QP_ERROR;
    }

    return qp_change_user_by_id(uid, gid);
}


qp_library_t
qp_library_load(const qp_char_t* name, qp_int_t flag)
{
    qp_library_t libs = (qp_library_t)qp_alloc(sizeof(struct qp_library_s));
    
    if (!libs) {
        return NULL;
    }
    
    memset(libs, 0, sizeof(struct qp_library_s));
    libs->handler = dlopen(name, flag);

    if (NULL == libs->handler) {
        libs->error = dlerror();
        qp_free(libs);
        return NULL;
    }

    libs->error = NULL;
    libs->stat = QP_LIB_LOAD;
    return libs;
}


void*
qp_library_getFunc(qp_library_t libs, const char *func)
{
    if ((NULL == libs) || (QP_LIB_LOAD != libs->stat)) {
        return NULL;
    }

    void* fun_ptr = dlsym(libs->handler, func);
    
    if (NULL != (libs->error = dlerror())) {
        return NULL;
    }

    return fun_ptr;
}


void
qp_library_unload(qp_library_t libs)
{
    if ((NULL == libs) || (QP_LIB_LOAD != libs->stat)) {
        return;
    }

    dlclose(libs->handler);
    libs->handler = NULL;
    libs->stat = QP_LIB_UNLOAD;
    qp_free(libs);
}
