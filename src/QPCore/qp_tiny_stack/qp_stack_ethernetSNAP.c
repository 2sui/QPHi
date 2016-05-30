/**
  * Copyright (C) sui
  */


#include "qp_stack_datalink.h"


qp_uint16_t
qp_stack_datalink_ethernet_snap(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result)
{
    qp_stack_ethernet_snap_t* ethernet = \
        (qp_stack_ethernet_snap_t*)(frame + result->l2_offset);
    result->l3_offset = result->l2_offset + sizeof(qp_stack_ethernet_snap_t);
    
    if (len < result->l3_offset) {
        return QP_STACK_PROTO_UNKNOWN;
    }
    
    result->smac = &ethernet->mac.src;
    result->dmac = &ethernet->mac.dst;
    result->l2_type = QP_STACK_PROTO_ETHERNET_SNAP;
    result->l3_type = qp_stack_datalink_get_type(ntohs(ethernet->type));
    result->data_offset = result->l3_offset;
    return result->l2_type;
}


