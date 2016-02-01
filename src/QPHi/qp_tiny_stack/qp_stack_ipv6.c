/**
  * Copyright (C) sui
  */


#include "qp_stack_network.h"


qp_uint16_t 
qp_stack_network_ipv6(qp_uchar_t* frame, qp_uint32_t len,\
    qp_stack_frame_result_t* result)
{
    qp_stack_ipv6_t* ip = (qp_stack_ipv6_t*)(frame + result->l3_offset);
    result->l4_offset = result->l3_offset + sizeof(qp_stack_ipv4_t);
    
    if (len < result->l4_offset) {
        return (result->l3_type = QP_STACK_PROTO_UNKNOWN);
    }
    
//    result->l4_type = qp_stack_network_get_type(ip->verNtypeNflag);
    result->l4_type = QP_STACK_PROTO_UNKNOWN;
    result->src.ipv6 = &ip->src;
    result->dst.ipv6 = &ip->dst;
    result->data_offset = result->l4_offset;
    return result->l3_type;
}
