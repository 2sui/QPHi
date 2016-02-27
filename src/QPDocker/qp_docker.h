
/**
 * Copyright (C) 2sui.
 */


#ifndef QP_CDOCKER_CORE_H
#define QP_CDOCKER_CORE_H


#include <qp_core.h>


#define  QP_STDERR_LOG_FOR_CDOCKER


struct qp_docker_conf_s {
    qp_char_t*           cdocker_stack;
    size_t               cdocker_stack_size;
    const qp_char_t*     cdocker_work_dir;
    const qp_char_t*     cdocker_root_dir;
    const qp_char_t*     cdocker_hostname;
    qp_char_t* const*    cdocker_entrypoint;
    qp_char_t* const*    cdocker_cmd;
    qp_char_t* const*    cdocker_mount;
};

typedef struct qp_docker_conf_s  qp_docker_conf_t;


/*
 * Set the cdocker outside environment.
*/
qp_int_t
qp_ddocker(qp_docker_conf_t* conf = NULL);

#endif // QP_CDOCKER_CORE_H
