/**
  * Copyright (C) sui
  */

#ifndef QP_STACK_TRANSMIT_H
#define QP_STACK_TRANSMIT_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../qp_tiny_stack.h"


qp_uint16_t 
qp_stack_transmit_tcp(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result);

qp_uint16_t 
qp_stack_transmit_udp(qp_uchar_t* frame, qp_uint32_t len,\
    qp_stack_frame_result_t* result);

    
#ifdef __cplusplus
}
#endif

#endif /* QP_STACK_TRANSMIT_H */

