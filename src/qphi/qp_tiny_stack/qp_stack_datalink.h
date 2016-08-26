/**
  * Copyright (C) sui
  */

#ifndef QP_STACK_DATALINK_H
#define QP_STACK_DATALINK_H

#ifdef __cplusplus
extern "C" {
#endif


#include "../qp_tiny_stack.h"
    
    
qp_uint16_t
qp_stack_datalink_get_type(qp_uint16_t type);

qp_uint16_t 
qp_stack_datalink_ethernet_II(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result);

qp_uint16_t 
qp_stack_datalink_ethernet_802(qp_uchar_t* frame, qp_uint32_t len,\
    qp_stack_frame_result_t* result);

qp_uint16_t 
qp_stack_datalink_ethernet_novell(qp_uchar_t* frame, qp_uint32_t len,\
    qp_stack_frame_result_t* result);

qp_uint16_t 
qp_stack_datalink_ethernet_snap(qp_uchar_t* frame, qp_uint32_t len,\
    qp_stack_frame_result_t* result);


#ifdef __cplusplus
}
#endif

#endif /* QP_STACK_DATALINK_H */

