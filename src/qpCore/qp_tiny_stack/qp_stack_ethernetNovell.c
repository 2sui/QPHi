/**
  * Copyright (C) sui
  */


#include "qp_stack_datalink.h"


qp_uint16_t
qp_stack_datalink_ethernet_novell(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result)
{
    qp_stack_ethernet_novell_t* ethernet = \
        (qp_stack_ethernet_novell_t*)(frame + result->l2_offset);
    result->l3_offset = result->l2_offset + sizeof(qp_stack_ethernet_novell_t);
    
    if (len < result->l3_offset) {
        return QP_STACK_PROTO_UNKNOWN;
    }
    
    result->smac = &ethernet->mac.src;
    result->dmac = &ethernet->mac.dst;
    result->l2_type = QP_STACK_PROTO_ETHERNET_NOVELL;
    result->l3_type = QP_STACK_PROTO_IPX;
    result->data_offset = result->l3_offset;
    return result->l2_type;
}