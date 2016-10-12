/**
  * Copyright (C) sui
  */


#include "qp_stack_transmit.h"


qp_uint16_t 
qp_stack_transmit_udp(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result)
{
    qp_stack_udp_t* udp = (qp_stack_udp_t*)(frame + result->l4_offset);
    result->data_offset = result->l4_offset + sizeof(qp_stack_udp_t);
            
    if (len < result->data_offset) {
        return (result->l4_type = QP_STACK_PROTO_UNKNOWN);
    }
    
    result->sport = udp->sport;
    result->dport = udp->dport;
    return result->l4_type;
}
