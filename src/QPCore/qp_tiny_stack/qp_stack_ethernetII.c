/**
  * Copyright (C) sui
  */


#include "qp_stack_datalink.h"


qp_uint16_t
qp_stack_datalink_get_type(qp_uint16_t type)
{
    if ((0x0101 <= type) && (0x01ff >= type)) {
        /* test */
        return QP_STACK_PROTO_UNKNOWN;
        
    } else {
        
        if((0x0660 <= type) && (0x0661 >= type)) {
            /* DLOG */
            return QP_STACK_PROTO_UNKNOWN;
            
        } else {
            
            switch (type) {
                
            case 0x0600: break;//XEROX NS IDP
            case 0x0800:
                return QP_STACK_PROTO_IP;//ip
            case 0x0801: break;//x.75 internet
            case 0x0802: break;//DBS internet
            case 0x0803: break;//ECMA internet
            case 0x0804: break;//chaosnet
            case 0x0805: break;//x.25 level 3
            case 0x0806:
                return QP_STACK_PROTO_ARP;//arp
            case 0x0808: break;//frame relay arp
            case 0x6559: break;//Original frame relay 
            case 0x8035:
                return QP_STACK_PROTO_RARP;//rarp
            case 0x8037:
                return QP_STACK_PROTO_NOVELL_IPX;//Novell Netware IPX
            case 0x809b: break;//ether talk
            case 0x80d5: break;//ibm sna service over ethernet
            case 0x80f3:
                return QP_STACK_PROTO_AARP;//apple talk arp(aarp)
            case 0x8100:
                return QP_STACK_PROTO_ESPS;//EAPS：Ethernet Automatic Protection Switching
            case 0x8137:
                return QP_STACK_PROTO_IPX;//ipx
            case 0x814c:
                return QP_STACK_PROTO_SNMP;//snmp
            case 0x86dd:
                return QP_STACK_PROTO_IPV6;//ipv6
            case 0x880b:
                return QP_STACK_PROTO_PPP;//ppp
            case 0x880c:
                return QP_STACK_PROTO_GSMP;//gsmp
            case 0x8847:
                return QP_STACK_PROTO_S_MPLS;//mpls
            case 0x8848:
                return QP_STACK_PROTO_G_MPLS;//mpls
            case 0x8863:
                return QP_STACK_PROTO_D_PPPOE;//PPPOE discovery
            case 0x8864:
                return QP_STACK_PROTO_S_PPPOE;//PPPOE session
            case 0x88bb:
                return QP_STACK_PROTO_LWAPP;//lwapp
            case 0x88cc:
                return QP_STACK_PROTO_LLDP;//LLDP
            case 0x8e88:
                return QP_STACK_PROTO_EAPOL;//EAP over LAN
            case 0x9000:
                return QP_STACK_PROTO_LOOKBACK;//lookback
            case 0x9100:
                return QP_STACK_PROTO_VLAN1;//VLAN Tag Protocol Identifier
            case 0x9200:
                return QP_STACK_PROTO_VLAN2;//VLAN Tag Protocol Identifier
            default: break;
            }
            
            return QP_STACK_PROTO_UNKNOWN;
        }
    }
}

qp_uint16_t 
qp_stack_datalink_ethernet_II(qp_uchar_t* frame, qp_uint32_t  len, \
    qp_stack_frame_result_t* result)
{
    qp_stack_ethernet_II_t* ethernet = \
        (qp_stack_ethernet_II_t*)(frame + result->l2_offset);
    result->l3_offset = result->l2_offset + sizeof(qp_stack_ethernet_II_t);
    
    if (len < result->l3_offset) {
        return QP_STACK_PROTO_UNKNOWN;
    }
    
    
    result->smac = &ethernet->mac.src;
    result->dmac = &ethernet->mac.dst;
    result->l2_type = QP_STACK_PROTO_ETHERNET_II;
    result->l3_type = qp_stack_datalink_get_type(ntohs(ethernet->mac.typeNlen));
    result->data_offset = result->l3_offset;
    return result->l2_type;
}

