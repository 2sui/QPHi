
/*
  * Copyright (C) ShuaiXX
  */


#include <qp_cdocker_core.h>


#define   FILE_NAME_SIZE                                     256
#define  QP_CDOCKER_DEFAULT_STACKSIZE       (512*1024)  // default 512kB stack
#define  QP_CDOCKER_DEFAULT_HOSTNAME     "QPcdocker"
#define  QP_CDOCKER_DEFAULT_ROOTDIR         "/default"
#define  QP_CDOCKER_DEFAULT_WORKDIR        QP_CDOCKER_DEFAULT_ROOTDIR


/* child process run stack  */
static qp_char_t qp_cdocker_default_stack[QP_CDOCKER_DEFAULT_STACKSIZE] = {
    0
};

static qp_char_t* const qp_cdocker_default_cmd[] = {
    "/bin/bash",
    NULL
};

static qp_char_t* const qp_cdocker_default_mount[] = {
    "","","",
    "","","",
    NULL
};

/* default cdocker config */
static qp_cdocker_conf_t   qp_cdocker_default_conf = {
    qp_cdocker_default_stack,
    QP_CDOCKER_DEFAULT_STACKSIZE,
    QP_CDOCKER_DEFAULT_WORKDIR,
    QP_CDOCKER_DEFAULT_ROOTDIR,
    QP_CDOCKER_DEFAULT_HOSTNAME,
    qp_cdocker_default_cmd,
    qp_cdocker_default_mount
};

/* process notification */
static qp_int_t qp_cdocker_notify_fd[2] = { 0 };

/*
 * Write map file for id map.
*/
qp_int_t
qp_cdocker_set_id_map(const qp_char_t *file, qp_uint32_t inside_id, qp_uint32_t outside_id,
    qp_int_t len)
{
    FILE* mapfd = fopen(file, "w");
    if (NULL == mapfd) {
        return QP_ERROR;
    }

    /* map id for inside - outside - len */
    fprintf(mapfd, "%d %d %d", inside_id, outside_id, len);
    fclose(mapfd);
    return QP_SUCCESS;
}

/*
 * Write map file for outside uid to inside uid.
*/
qp_int_t
qp_cdocker_map_uid(pid_t containerpid, qp_uint32_t inside_id, qp_uint32_t outside_id,
    qp_int_t len)
{
    char file[FILE_NAME_SIZE] = { 0 };
    sprintf(file, "/proc/%d/uid_map", containerpid);
    return qp_cdocker_set_id_map(file, inside_id, outside_id, len);
}

/*
 * Write map file for outside gid to inside gid.
*/
qp_int_t
qp_cdocker_map_gid(pid_t containerpid, qp_uint32_t inside_id, qp_uint32_t outside_id, qp_int_t len)
{
    char file[FILE_NAME_SIZE] = { 0 };
    sprintf(file, "/proc/%d/gid_map", containerpid);
    return qp_cdocker_set_id_map(file, inside_id, outside_id, len);
}

/*
 * Entry for cdocker process.It will set up environment and args and then run cmd in container.
*/
qp_int_t
qp_cdocker_container(void* arg)
{
#ifdef QP_STDERR_LOG_FOR_CDOCKER
        fprintf(stderr, "\n[QPcdocker] Container run...");
#endif

    qp_cdocker_conf_t* conf = (qp_cdocker_conf_t*) arg;

    /* waiting father process for finishing uid/gid map */
    char   ch;
    close(qp_cdocker_notify_fd[1]);
#ifdef QP_STDERR_LOG_FOR_CDOCKER
    fprintf(stderr, "\n[QPcdocker] Container waiting for notification...");
#endif
    read(qp_cdocker_notify_fd[0], &ch, 1);

    /* set container host name */
    if (QP_ERROR == sethostname(conf->cdocker_hostname, strlen(conf->cdocker_hostname) + 1)) {
#ifdef QP_STDERR_LOG_FOR_CDOCKER
        fprintf(stderr, "\n[QPcdocker] Container set hostname fail.");
#endif
        return QP_ERROR;
    }

    /* mount file system */
//    for (int i = 0;
//           (NULL != conf->cdocker_mount[i])
//           && (NULL != conf->cdocker_mount[i + 1])
//           && (NULL != conf->cdocker_mount[i + 2]);
//           i++)
//    {

//        if (0 == strcmp("proc", conf->cdocker_mount[i])) {

//            continue;
//        }

//        if (0 == strcmp("sysfs", conf->cdocker_mount[i])) {

//            continue;
//        }

//        if (0 == strcmp("tmpfs", conf->cdocker_mount[i])) {

//            continue;
//        }

//        if (0 == strcmp("devtmpfs", conf->cdocker_mount[i])) {

//            continue;
//        }

//        if (0 == strcmp("devpts", conf->cdocker_mount[i])) {

//            continue;
//        }

//    }


//    if (QP_SUCCESS != mount("proc", "/home/docker/docker_c/proc", "proc", 0, NULL)) {
//        return QP_ERROR;
//    }

//    if (QP_SUCCESS != mount("sysfs", "/home/docker/docker_c/sys", "sysfs", 0, NULL)) {
//        return QP_ERROR;
//    }

//    if (QP_SUCCESS != mount("none", "/home/docker/docker_c/tmp", "tmpfs", 0, NULL)) {
//        return QP_ERROR;
//    }

//    if (QP_SUCCESS != mount("udev", "/home/docker/docker_c/dev", "devtmpfs", 0, NULL)) {
//        return QP_ERROR;
//    }

//    if (QP_SUCCESS != mount("devpts", "/home/docker/docker_c/dev/pts", "devpts", 0, NULL)) {
//        return QP_ERROR;
//    }

//    if (QP_SUCCESS != mount("shm", "/home/docker/docker_c/dev/shm", "tmpfs", 0, NULL)) {
//        return QP_ERROR;
//    }

//    if (QP_SUCCESS != mount("tmpfs", "/home/docker/docker_c/run", "tmpfs", 0, NULL)) {
//        return QP_ERROR;
//    }

    /* chroot */
//    if (QP_SUCCESS != chdir(conf->cdocker_work_dir)
//        || QP_SUCCESS != chroot(conf->cdocker_root_dir))
//    {
//#ifdef QP_STDERR_LOG_FOR_CDOCKER
//        fprintf(stderr, "\n[QPcdocker] Container change work dir [%s] / root dir [%s] fail.",
//                    conf->cdocker_work_dir,
//                    conf->cdocker_root_dir);
//#endif
//        return QP_ERROR;
//    }

    /* exec cdocker command */
    execv(conf->cdocker_cmd[0], conf->cdocker_cmd);
#ifdef QP_STDERR_LOG_FOR_CDOCKER
    fprintf(stderr, "\n[QPcdocker] Container run command fail.");
#endif
    return QP_ERROR;
}

/*
 * Set the cdocker outside environment.
*/
qp_int_t
qp_cdocker(qp_cdocker_conf_t* conf)
{

    if (NULL == conf) {
        conf = &qp_cdocker_default_conf;
    }

#ifdef QP_STDERR_LOG_FOR_CDOCKER
    fprintf(stderr, "\n[QPcdocker] Docker start...");
#endif

    if (QP_SUCCESS != pipe(qp_cdocker_notify_fd)) {
#ifdef QP_STDERR_LOG_FOR_CDOCKER
        fprintf(stderr, "\n[QPcdocker] Set notify pipe fail.");
#endif
        return QP_ERROR;
    }

    /* cdocker process uid and pid */
    uid_t qp_cdocker_uid = getuid();
    gid_t qp_cdocker_gid = getgid();

    /* container process pid */
    pid_t qp_cdocker_container_pid = clone(qp_cdocker_container,  \
                                                                         conf->cdocker_stack + conf->cdocker_stack_size, \
                                                                         SIGCHLD
                                                                         | CLONE_NEWUTS     /* set CLONE_NEWUTS for UTS namespace */
                                                                         | CLONE_NEWIPC     /* set CLONE_NEWIPC for IPC namespace */
                                                                         | CLONE_NEWPID     /* set CLONE_NEWPID for PID namespace*/
                                                                         | CLONE_NEWNS      /* set CLONE_NEWNS for mount(filesystem) namespace */
                                                                         | CLONE_NEWUSER  /* set CLONE_NEWUSER for uid and gid */
                                                                         | CLONE_NEWNET    /* set CLONE_NEWNET for network */
                                                                         , (void*)conf);

    if (QP_ERROR == qp_cdocker_container_pid) {
#ifdef QP_STDERR_LOG_FOR_CDOCKER
        fprintf(stderr, "\n[QPcdocker] Container create fail.");
#endif
        return QP_ERROR;
    }
#ifdef QP_STDERR_LOG_FOR_CDOCKER
    else {
        fprintf(stderr, "\n[QPcdocker] Container start: [%ld]", qp_cdocker_container_pid);
    }
#endif

    /* map uid and pid */
    if (QP_SUCCESS != qp_cdocker_map_uid(qp_cdocker_container_pid, 0, qp_cdocker_uid, 1)
        || QP_SUCCESS != qp_cdocker_map_gid(qp_cdocker_container_pid, 0, qp_cdocker_gid, 1))
    {
#ifdef QP_STDERR_LOG_FOR_CDOCKER
        fprintf(stderr, "\n[QPcdocker] UID/GID map fail.");
#endif
        return QP_ERROR;
    }

    /* notify container process */
    close(qp_cdocker_notify_fd[1]);

    /* wait for container */
    waitpid(qp_cdocker_container_pid, NULL, 0);

#ifdef QP_STDERR_LOG_FOR_CDOCKER
        fprintf(stderr, "\n[QPcdocker] Docker finished.");
#endif

    return QP_SUCCESS;
}

