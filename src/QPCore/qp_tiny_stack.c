/*
 * The MIT License
 *
 * Copyright © 2016 2sui.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "qp_tiny_stack.h"
#include "qp_tiny_stack/qp_stack_datalink.h"
#include "qp_tiny_stack/qp_stack_network.h"
#include "qp_tiny_stack/qp_stack_transmit.h"
#include "qp_tiny_stack/bpf.h"


qp_uint16_t
qp_stack_datalink_parse(qp_uchar_t* frame, qp_uint32_t len, qp_int_t link,\
    qp_stack_frame_result_t* result)
{   
    if (!frame || !result) {
        return QP_STACK_PROTO_UNKNOWN;
    }
    
    switch (link)
    {
    /* BSD loop */
    case DLT_NULL: break;
    /* Ethernet */
    case DLT_EN10MB: {
        
        if (ntohs(((qp_stack_mac_t*) frame)->typeNlen) <= 0x05dc) {
            
            switch (htons(((qp_stack_802_t*) frame)->identify)) {
                
            case 0xffff: {
                //Novell
                result->l2_type = QP_STACK_PROTO_ETHERNET_NOVELL;
                return qp_stack_datalink_ethernet_novell(frame, len, result);
            }break;
            
            case 0xaaaa: {
                //SNAP
                result->l2_type = QP_STACK_PROTO_ETHERNET_SNAP;
                return qp_stack_datalink_ethernet_snap(frame, len, result);
            }break;
            
            default: {
                //IEEE 802.2
                result->l2_type = QP_STACK_PROTO_ETHERNET_802;
                return qp_stack_datalink_ethernet_802(frame, len, result);
            }break;
            }
            
        } else {
            //Ethernet II
            return qp_stack_datalink_ethernet_II(frame, len, result);
        }
    }
        break;
    /*  IEEE802.5 token ring */
    case DLT_IEEE802: break;
    /* ARCNET */
    case DLT_ARCNET: break;
    /* SLIP */
    case DLT_SLIP: break;
    /* PPP if first byte is "0xff" or "0x03",it is HDLC PPP */
    case DLT_PPP: break;
    /* FDDI */
    case DLT_FDDI: break;
    /* RFC1483LLC/SNAP ATM；Begin with IEEE802.2 LLC header */
    case DLT_ATM_RFC1483: break;
    /* Raw IP */
    case DLT_RAW: break;
    /* 按照RFC1662，基于类HDLC帧的PPP，或者按照RFC1547的4.3.1，
     * 基于HDLC帧的Cisco PPP；前者的第一个字节是0xFF，后者的第一个字节是0x0F或0x8F
    */
    case DLT_PPP_SERIAL: break;
    /* 按照RFC2516，PPPoE；数据包以PPPoE头开始 */
    case DLT_PPP_ETHER: break;
    /* 按照RFC1547的4.3.1，基于HDLC帧的Cisco PPP */
    case DLT_C_HDLC: break;
    /* IEEE802.11 */
    case DLT_IEEE802_11: break;
    /* 帧中继（Frame Relay) */
    case DLT_FRELAY: break;
    /* OpenBSD loop */
    case DLT_LOOP: break;
    /* Linux */
    case DLT_LINUX_SLL: break;
    /* 苹果的LocalTalk，数据包以AppleTalk LLAP头开始 */
    case DLT_LTALK: break;
    /* OpenBSD pflog */
    case DLT_PFLOG: break;
    /* 后接802.11头的棱镜监视器模式（Prism monitor mode）信息 */
    case DLT_PRISM_HEADER: break;
    /* RFC2625 IP-over-Fiber 频道，以RFC2625中定义的Network_Header开始 */
    case  DLT_IP_OVER_FC: break;
    /* SunATM设备 */
    case DLT_SUNATM: break;
    /* 后接802.11头的链路层信息 */
    case DLT_IEEE802_11_RADIO: break;
    /* 没有异常帧的ARCNET */
    case DLT_ARCNET_LINUX: break;
    /* Linux-IrDA数据包，DLT_LINUX_SLL头后接IrLAP头 */
    case DLT_LINUX_IRDA: break;
    default: break;
    }


    return result->l2_type;
}

qp_uint16_t
qp_stack_network_parse(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result)
{
    if (!frame || !result) {
        return QP_STACK_PROTO_UNKNOWN;
    }
    
    switch (result->l3_type) {
        
    case QP_STACK_PROTO_IP: {
        return qp_stack_network_ipv4(frame, len, result);
    }break;
        
    case QP_STACK_PROTO_IPV6: {
        return qp_stack_network_ipv6(frame, len, result);
    }break;
    
    case QP_STACK_PROTO_ARP: break;
    case QP_STACK_PROTO_RARP: break;
    case QP_STACK_PROTO_IPX: break;
    case QP_STACK_PROTO_NOVELL_IPX: break;
    case QP_STACK_PROTO_AARP: break;
    case QP_STACK_PROTO_ESPS: break;
    case QP_STACK_PROTO_SNMP: break;
    case QP_STACK_PROTO_PPP: break;
    case QP_STACK_PROTO_GSMP: break;
    case QP_STACK_PROTO_S_MPLS: break;
    case QP_STACK_PROTO_G_MPLS: break;
    case QP_STACK_PROTO_D_PPPOE: break;
    case QP_STACK_PROTO_S_PPPOE: break;
    case QP_STACK_PROTO_LWAPP: break;
    case QP_STACK_PROTO_LLDP: break;
    case QP_STACK_PROTO_EAPOL: break;
    case QP_STACK_PROTO_LOOKBACK: break;
    case QP_STACK_PROTO_VLAN1: break;
    case QP_STACK_PROTO_VLAN2: break;
    default: break;
    }
    
    result->l3_type = QP_STACK_PROTO_UNKNOWN;
    return result->l3_type;
}

qp_uint16_t
qp_stack_transmit_parse(qp_uchar_t* frame, qp_uint32_t len, \
    qp_stack_frame_result_t* result)
{
    if (!frame || !result) {
        return QP_STACK_PROTO_UNKNOWN;
    }
    
    switch (result->l4_type) {
        
    case QP_STACK_PROTO_ICMP: break;
    case QP_STACK_PROTO_IGMP: break;
    case QP_STACK_PROTO_GGP: break;
    case QP_STACK_PROTO_IPENCAP: break;
    case QP_STACK_PROTO_ST: break;
    
    case QP_STACK_PROTO_TCP: {
        return qp_stack_transmit_tcp(frame, len, result);
    }break;
    
    case QP_STACK_PROTO_EGP: break;
    case QP_STACK_PROTO_IGP: break;
    case QP_STACK_PROTO_PUP: break;
    case QP_STACK_PROTO_UDP: {
        return qp_stack_transmit_udp(frame, len, result);
    }break;
    
    default: break;
    }
    
    result->l4_type = QP_STACK_PROTO_UNKNOWN;
    return result->l4_type;
}

qp_int_t
qp_stack_parse(qp_uchar_t* frame, qp_uint32_t len, qp_int_t link,\
    qp_stack_frame_result_t* result, qp_uint16_t level)
{
    if (!frame || !result) {
        return QP_ERROR;
    }
    
    if (level > QP_STACK_LEVEL_TRANSMIT) {
        level = QP_STACK_LEVEL_TRANSMIT;
    }
    
    if (level < QP_STACK_LEVEL_DATALINK) {
        level = QP_STACK_LEVEL_DATALINK;
    }
    
    memset(result, 0, sizeof(qp_stack_frame_result_t));
    
    qp_uint16_t i = 0;
    
    for (; i < level; i++) {
        
        switch (i) {
        
        case 0: {
            
            if (QP_STACK_PROTO_UNKNOWN == \
                qp_stack_datalink_parse(frame, len, link, result)) 
            {
                
                if (i != (level - 1)) {
                    return QP_ERROR;
                    
                } else {
                    return QP_SUCCESS;
                }
            }
            
        }break;
        
        case 1: {
            
            if (QP_STACK_PROTO_UNKNOWN == \
                qp_stack_network_parse(frame, len, result)) 
            {
                
                if (i != (level - 1)) {
                    return QP_ERROR;
                    
                } else {
                    return QP_SUCCESS;
                }
            }
            
        }break;
            
        default: {
            
            if (QP_STACK_PROTO_UNKNOWN == \
                qp_stack_transmit_parse(frame, len, result)) 
            {
                
                if (i != (level - 1)) {
                    return QP_ERROR;
                    
                } else {
                    return QP_SUCCESS;
                }
            }
            
        }break;
        
        }
    }
    
    return QP_SUCCESS;
}
