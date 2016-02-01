/**
  * Copyright (C) sui
  */


#include "qp_stack_datalink.h"


qp_uint16_t
qp_stack_datalink_ethernet_802(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result)
{
    qp_stack_ethernet_802_t* ethernet = \
        (qp_stack_ethernet_802_t*)(frame + result->l2_offset);
    result->l3_offset = result->l2_offset + sizeof(qp_stack_ethernet_802_t);
    
    if (len < result->l3_offset) {
        return QP_STACK_PROTO_UNKNOWN;
    }
    
    
    result->smac = &ethernet->mac.src;
    result->dmac = &ethernet->mac.dst;
    result->l2_type = QP_STACK_PROTO_ETHERNET_802;
    result->l3_type = qp_stack_datalink_get_type(ntohs(ethernet->mac.typeNlen));
    /* disable */
    result->l3_type = QP_STACK_PROTO_UNKNOWN;
    result->data_offset = result->l3_offset;
    return result->l2_type;
}