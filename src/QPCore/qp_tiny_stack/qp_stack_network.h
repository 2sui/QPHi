/**
  * Copyright (C) sui
  */

#ifndef QP_STACK_NETWORK_H
#define QP_STACK_NETWORK_H

#ifdef __cplusplus
extern "C" {
#endif

    
#include "../qp_tiny_stack.h"


qp_uint16_t
qp_stack_network_get_type(qp_uint8_t type);

qp_uint16_t 
qp_stack_network_ipv4(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result);

qp_uint16_t 
qp_stack_network_ipv6(qp_uchar_t* frame, qp_uint32_t len,\
    qp_stack_frame_result_t* result);


#ifdef __cplusplus
}
#endif

#endif /* QP_STACK_NETWORK_H */

