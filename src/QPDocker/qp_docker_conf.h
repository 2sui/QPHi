
/**
 * Copyright (C) 2sui.
 */

#ifndef QP_DOCKER_CONF_H
#define QP_DOCKER_CONF_H


#include <qp_core.h>

typedef struct {
    qp_char_t*           docker_stack;
    qp_uint64_t          docker_stack_size;
    const qp_char_t*     docker_work_dir;
    const qp_char_t*     docker_root_dir;
    qp_int_t             docker_ns;
    qp_int_t             docker_cg;
    const qp_char_t*     docker_hostname;
    
    qp_char_t* const*    docker_entrypoint;
    qp_char_t* const*    docker_cmd;
    qp_char_t* const*    docker_mount;
} qp_docker_conf_t;

#endif /* QP_DOCKER_CONF_H */

