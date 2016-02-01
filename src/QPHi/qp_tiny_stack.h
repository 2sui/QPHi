
/**
  * Copyright (C) sui
  * 
  * Tiny network stack.
  */


#ifndef QP_TINY_STACK_H
#define QP_TINY_STACK_H


#ifdef __cplusplus
extern "C" {
#endif

    
#include "qp_o_memory.h"
 
    
#define QP_STACK_LEVEL_DATALINK    1
#define QP_STACK_LEVEL_NETWORK     2
#define QP_STACK_LEVEL_TRANSMIT    3
    
    
#define QP_STACK_PROTO_UNKNOWN            0X0000
    
/* datalink proto : 0x01 ~ 0x30 */
#define QP_STACK_PROTO_ETHERNET_II        0x0001
#define QP_STACK_PROTO_ETHERNET_NOVELL    0x0002
#define QP_STACK_PROTO_ETHERNET_802       0x0003
#define QP_STACK_PROTO_ETHERNET_SNAP      0x0004
    
/* network proto : 0x31 ~ 0xff */
#define QP_STACK_PROTO_IP                 0x0031
#define QP_STACK_PROTO_IPV6               0x0032
#define QP_STACK_PROTO_ARP                0x0033
#define QP_STACK_PROTO_RARP               0x0034
#define QP_STACK_PROTO_IPX                0x0035
#define QP_STACK_PROTO_NOVELL_IPX         0x0036
#define QP_STACK_PROTO_SNMP               0x0037
#define QP_STACK_PROTO_PPP                0x0038
#define QP_STACK_PROTO_GSMP               0x0039
#define QP_STACK_PROTO_AARP               0x003a
#define QP_STACK_PROTO_ESPS               0x003b
#define QP_STACK_PROTO_S_MPLS             0x003c
#define QP_STACK_PROTO_G_MPLS             0x003d
#define QP_STACK_PROTO_D_PPPOE            0x003e
#define QP_STACK_PROTO_S_PPPOE            0x003f
#define QP_STACK_PROTO_LWAPP              0x0040
#define QP_STACK_PROTO_LLDP               0x0041
#define QP_STACK_PROTO_EAPOL              0x0042
#define QP_STACK_PROTO_LOOKBACK           0x0043
#define QP_STACK_PROTO_VLAN1              0x0044
#define QP_STACK_PROTO_VLAN2              0x0045
    
/* transmit proto : 0x0100 ~ 0xffff */
#define QP_STACK_PROTO_ICMP               0x0100
#define QP_STACK_PROTO_IGMP               0x0101
#define QP_STACK_PROTO_TCP                0x0102
#define QP_STACK_PROTO_UDP                0x0103
#define QP_STACK_PROTO_GGP                0x0104
#define QP_STACK_PROTO_IPENCAP            0x0105
#define QP_STACK_PROTO_ST                 0x0106
#define QP_STACK_PROTO_EGP                0x0107
#define QP_STACK_PROTO_IGP                0x0108
#define QP_STACK_PROTO_PUP                0x0109
    
    
typedef struct qp_stack_mac_addr_s {
    qp_uint8_t    sit1;
    qp_uint8_t    sit2;
    qp_uint8_t    sit3;
    qp_uint8_t    sit4;
    qp_uint8_t    sit5;
    qp_uint8_t    sit6;
}__attribute__((__packed__)) qp_stack_mac_addr_t;


typedef struct qp_stack_mac_s {
    qp_stack_mac_addr_t    dst;
    qp_stack_mac_addr_t    src;
    qp_uint16_t            typeNlen;
}__attribute__((__packed__)) qp_stack_mac_t;


typedef struct qp_stack_802_s {
    qp_stack_mac_t    mac;
    qp_uint16_t       identify;
}__attribute__((__packed__)) qp_stack_802_t;


typedef struct qp_stack_ethernet_II_s {
    qp_stack_mac_t    mac;
}__attribute__((__packed__)) qp_stack_ethernet_II_t;

typedef qp_stack_ethernet_II_t    qp_stack_ethernet_novell_t;


typedef struct qp_stack_ethernet_802_s {
    qp_stack_mac_t    mac;
    qp_uint8_t        llc_dsap;
    qp_uint8_t        llc_ssap;
    qp_uint8_t        llc_ctl;
}__attribute__((__packed__)) qp_stack_ethernet_802_t;


typedef struct qp_stack_ethernet_snap_s {
    qp_stack_mac_t    mac;
    qp_uint8_t        llc_dsap;
    qp_uint8_t        llc_ssap;
    qp_uint8_t        llc_ctl;
    qp_uint8_t        org_code[3];
    qp_uint16_t       type;
}__attribute__((__packed__)) qp_stack_ethernet_snap_t;


typedef struct qp_stack_ipv4_addr_s {
    qp_uint8_t    sit1;
    qp_uint8_t    sit2;
    qp_uint8_t    sit3;
    qp_uint8_t    sit4;
}__attribute__((__packed__)) qp_stack_ipv4_addr_t;


typedef struct qp_stack_ipv6_addr_s {
    qp_uint8_t    sit1[2];
    qp_uint8_t    sit2[2];
    qp_uint8_t    sit3[2];
    qp_uint8_t    sit4[2];
    qp_uint8_t    sit5[2];
    qp_uint8_t    sit6[2];
    qp_uint8_t    sit7[2];
    qp_uint8_t    sit8[2];
}__attribute__((__packed__)) qp_stack_ipv6_addr_t;


typedef union {
    qp_stack_ipv4_addr_t    to_byte;
    qp_uint32_t             to_int;
}__attribute__((__packed__)) qp_stack_ipv4_addr_type_t;

typedef union {
    qp_stack_ipv6_addr_t    to_byte;
    qp_uint32_t             to_int[4];
    qp_uint64_t             to_long[2];
}__attribute__((__packed__)) qp_stack_ipv6_addr_type_t;


typedef struct qp_stack_ipv4_s {
#ifdef __QP_LITTLE_ENDIAN__
    qp_uint8_t    headerlen:4;
    qp_uint8_t    version:4;
    qp_uint8_t    dsf_dscp:6;
    qp_uint8_t    dsf_ecn:2;
#else 
# ifdef __QP_BIG_ENDIAN__
    qp_uint8_t    version:4;
    qp_uint8_t    headerlen:4;
    qp_uint8_t    dsf_ecn:2;
    qp_uint8_t    dsf_dscp:6;
# else 
# error "[Byte order : Byte order must be defined]"
# endif
#endif
    qp_uint16_t    len;
    qp_uint16_t    ident;
    qp_uint16_t    flagNoffset;
    qp_uint8_t     ttl;
    qp_uint8_t     proto;
    qp_uint16_t    crc;
    qp_stack_ipv4_addr_type_t    src;
    qp_stack_ipv4_addr_type_t    dst;
}__attribute__((__packed__)) qp_stack_ipv4_t;


typedef struct qp_stack_ipv6_s {
    qp_uint32_t    verNtypeNflag;
    qp_uint16_t    payload;
    qp_uint8_t     next;
    qp_uint8_t     ttl;
    qp_stack_ipv6_addr_type_t    src;
    qp_stack_ipv6_addr_type_t    dst;
}__attribute__((__packed__)) qp_stack_ipv6_t;


typedef struct qp_stack_tcp_s {
    qp_uint16_t    sport;
    qp_uint16_t    dport;
    qp_uint32_t    sseq;
    qp_uint32_t    sack;
#ifdef __QP_LITTLE_ENDIAN__
    qp_uint8_t     nce:1;
    qp_uint8_t     resl:3;
    qp_uint8_t     headerlen:4;
    qp_uint8_t     fin:1;
    qp_uint8_t     syn:1;
    qp_uint8_t     rst:1;
    qp_uint8_t     psh:1;
    qp_uint8_t     ack:1;
    qp_uint8_t     urg:1;
    qp_uint8_t     ece:1;
    qp_uint8_t     cwr:1;
#else 
# ifdef __QP_BIG_ENDIAN__
    qp_uint8_t     headerlen:4;
    qp_uint8_t     resl:3;
    qp_uint8_t     nce:1;
    qp_uint8_t     cwr:1;
    qp_uint8_t     ece:1;
    qp_uint8_t     urg:1;
    qp_uint8_t     ack:1;
    qp_uint8_t     psh:1;
    qp_uint8_t     rst:1;
    qp_uint8_t     syn:1;
    qp_uint8_t     fin:1;
# else 
# error "[Byte order : Byte order must be defined]"
# endif
#endif
    qp_uint16_t    win;
    qp_uint16_t    crc;
    qp_uint16_t    urg_ptr;
}__attribute__((__packed__)) qp_stack_tcp_t;


typedef struct qp_stack_udp_s {
    qp_uint16_t    sport;
    qp_uint16_t    dport;
    qp_uint16_t    len;
    qp_uint16_t    crc;
}__attribute__((__packed__)) qp_stack_udp_t;


typedef struct qp_stack_frame_result_s {
    qp_uint32_t    l2_type;
    qp_uint32_t    l2_offset;
    qp_uint32_t    l3_type;
    qp_uint32_t    l3_offset;
    qp_uint32_t    l4_type;
    qp_uint32_t    l4_offset;
    qp_uint32_t    data_offset;
    qp_uint16_t*    sport;
    qp_uint16_t*    dport;
    qp_stack_mac_addr_t*  smac;
    qp_stack_mac_addr_t*  dmac;
    
    union {
        qp_stack_ipv4_addr_type_t*    ipv4;
        qp_stack_ipv6_addr_type_t*    ipv6;
        qp_uint8_t*                   other;
    }              src;
    
    union {
        qp_stack_ipv4_addr_type_t*    ipv4;
        qp_stack_ipv6_addr_type_t*    ipv6;
        qp_uint8_t*                   other;
    }              dst;
}qp_stack_frame_result_t;


/**
 * Parse a frame. (Current support link type is DLT_EN10MB or 1)
 */
qp_int_t
qp_stack_parse(qp_uchar_t*  frame, qp_uint32_t len, qp_int_t link,\
    qp_stack_frame_result_t* result, qp_uint16_t level);

#ifdef __cplusplus
}
#endif

#endif /* QP_TINY_STACK_H */

