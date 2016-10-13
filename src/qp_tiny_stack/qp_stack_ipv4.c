/**
  * Copyright (C) sui
  */


#include "qp_stack_network.h"


qp_uint16_t
qp_stack_network_get_type(qp_uint8_t type)
{
    switch(type)
    {
    case 0x01:
        return QP_STACK_PROTO_ICMP;
    case 0x02:
        return QP_STACK_PROTO_IGMP;
    case 0x03:
        return QP_STACK_PROTO_GGP;
    case 0x04:
        return QP_STACK_PROTO_IPENCAP;
    case 0x05:
        return QP_STACK_PROTO_ST;
    case 0x06:
        return QP_STACK_PROTO_TCP;
    case 0x08:
        return QP_STACK_PROTO_EGP;
    case 0x09:
        return QP_STACK_PROTO_IGP;
    case 0x0c:
        return QP_STACK_PROTO_PUP;
    case 0x11:
        return QP_STACK_PROTO_UDP;
    default:
        return QP_STACK_PROTO_UNKNOWN;
    }
}

qp_uint16_t 
qp_stack_network_ipv4(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result)
{
    qp_stack_ipv4_t* ip = (qp_stack_ipv4_t*)(frame + result->l3_offset);
    result->l4_offset = result->l3_offset + sizeof(qp_stack_ipv4_t);
    
    if (len < result->l4_offset) {
        return (result->l3_type = QP_STACK_PROTO_UNKNOWN);
    }
    
    result->l4_type = qp_stack_network_get_type(ip->proto);
    result->src.ipv4 = &ip->src;
    result->dst.ipv4 = &ip->dst;
    result->data_offset = result->l4_offset;
    return result->l3_type;
}


