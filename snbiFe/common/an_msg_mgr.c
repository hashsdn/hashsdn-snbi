/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include "an_tlv.h"
#include "an_nd.h"
#include "an_ni.h"
#include "an_msg_mgr.h"
#include "an_if_mgr.h"
#include "an_bs.h"
#include "an_anra.h"
#include "an_acp.h"
#include "../al/an_types.h"
#include "../al/an_icmp6.h"
#include "../al/an_pak.h"
#include "../al/an_ipv6.h"
#include "../al/an_udp.h"
#include "../al/an_ipv6_nd.h"
#include "../al/an_logger.h"
#include "../al/an_cert.h"
#include "../al/an_addr.h"
#include "../al/an_mem.h"
#include "../al/an_str.h"
#include "../al/an_if.h"

an_msg_delivery_e an_msg_delivery = AN_MSG_DELIVERY_UDP;
an_nd_delivery_e an_nd_delivery = AN_ND_DELIVERY_UDP;
extern boolean global_dom_id_set;
extern boolean global_dev_id_set;

typedef struct an_parse_grid_ {
    an_header header;
    uint8_t* tlv[AN_TLV_TYPE_LIMIT][AN_STLV_TYPE_LIMIT];
} an_parse_grid;

an_parse_grid parse_grid;    
    
an_msg_package *
an_msg_mgr_get_hello_message_package (an_if_t ifhndl)
{
    return (an_nd_get_hello(ifhndl));
}

void
an_msg_mgr_init_header (an_msg_package *msg_package, 
                        uint16_t protocol_type,
                        uint16_t msg_type)
{
    if (!msg_package) {
        an_log(AN_LOG_ERR, "AN: MSG Init NULL msg package");
        return;
    }
    msg_package->header.ver = 1;
    msg_package->header.protocol_type = protocol_type;
    msg_package->header.msg_type = msg_type;
    msg_package->header.hop_limit = 255;
    msg_package->header.length = 0; 

    switch (protocol_type) {
    case AN_PROTO_CHANNEL_DISCOVERY: 
        switch (msg_type) {
        case AN_MSG_UNTRUSTED_L2_CHANNEL_HELLO_REQ: 
        case AN_MSG_UNTRUSTED_L2_CHANNEL_HELLO_RESP: 
            break;
        default: 
            break;
        }
        break;
    case AN_PROTO_ADJACENCY_DISCOVERY:
        switch (msg_type) {
        case AN_ADJACENCY_DISCOVERY_HELLO:
            break;
        default:
            break;
        }
        break;
    case AN_PROTO_ACP:
    default:
        break;
    }
    msg_package->header.hop_limit = 255;
}


an_msg_package *
an_msg_mgr_get_empty_message_package (void)
{
    an_msg_package *msg_package = NULL;

    msg_package = (an_msg_package *)an_malloc_guard(sizeof(an_msg_package), 
                                    "AN Empty MSG package");
    if (!msg_package) {
        return (NULL);
    }

    an_memset_guard((uint8_t *)msg_package, 0, sizeof(an_msg_package));
    return (msg_package);
}

void
an_msg_mgr_free_message_package (an_msg_package *msg_package)
{
    if (!msg_package) {
         return;
    }

    if (msg_package->udi.data) {
        an_free_guard(msg_package->udi.data);
    }
    if (msg_package->sudi.data) {
        an_free_guard(msg_package->sudi.data);
    }
    if (msg_package->domain_cert.data) {
        an_free_guard(msg_package->domain_cert.data);
    }
    if (msg_package->device_id) {
        an_free_guard(msg_package->device_id);   
    }
    if (msg_package->domain_id) {
        an_free_guard(msg_package->domain_id);
    }
    if (msg_package->anra_sign.data) {
        an_free_guard(msg_package->anra_sign.data);
    }
    if (msg_package->anra_cert.data) {
        an_free_guard(msg_package->anra_cert.data);
    }
    if (msg_package->cert_request.data) {
        an_free_guard(msg_package->cert_request.data);
    }
    if (msg_package->cert_req_sign.data) {
        an_free_guard(msg_package->cert_req_sign.data);
    }
    if (msg_package->public_key.data) {
        an_free_guard(msg_package->public_key.data);
    }
    if (msg_package->payload.data) {
        an_free_guard(msg_package->payload.data);
    }
    if (msg_package->if_name) {
        an_free_guard(msg_package->if_name);
    }
    an_free_guard(msg_package);
}

static void
an_msg_mgr_init_parse_grid (void)
{
    an_memset(&parse_grid, 0, sizeof(an_parse_grid));
}

static uint8_t *
an_msg_mgr_get_tlv_from_parse_grid (uint8_t type, uint8_t stype)
{
    uint8_t *tlv = parse_grid.tlv[type][stype];

    if (!tlv) {
        return (NULL);
    }
    
/*
    if (((uint64_t) tlv)%4 != 0) {
        printf("\nTLV is not aligned %s", __FUNCTION__);
    }
*/    
    return (tlv);

}

static boolean 
an_msg_mgr_copy_parsed_value_after_alloc (uint8_t **ptr, uint8_t type, 
                                          uint8_t stype)
{
    uint8_t *tlv = an_msg_mgr_get_tlv_from_parse_grid(type, stype);
    uint16_t len = 0;

    if (!ptr || !tlv) {
        return (FALSE);
    }

    len = an_tlv_get_length(tlv) - AN_TLV_HDR_SIZE;

    if (len) {
        *ptr = (uint8_t *)an_malloc_guard(len, "AN Parsed value");
        if (!*ptr) {
            return (FALSE);
        }
        an_memcpy_guard(*ptr, an_tlv_get_value(tlv), len);

    } else {
        *ptr = NULL;
    }

    return (TRUE);
}

static boolean 
an_msg_mgr_copy_parsed_value (uint8_t *ptr, uint8_t type, uint8_t stype)
{
    uint8_t *tlv = an_msg_mgr_get_tlv_from_parse_grid(type, stype);
    uint16_t len = 0;

    if (!ptr || !tlv) {
        return (FALSE);
    }

    len = an_tlv_get_length(tlv) - AN_TLV_HDR_SIZE;

    if (len) {
        an_memcpy(ptr, an_tlv_get_value(tlv), len);

    } else {
        ptr = NULL;
    }

    return (TRUE);
}

#if 0
static void
an_msg_mgr_copy_parsed_len (uint16_t *ptr, uint8_t type, uint8_t stype)
{
    uint8_t *tlv = an_msg_mgr_get_tlv_from_parse_grid(type, stype);

    if (!ptr || !tlv) {
        return;
    }

    *ptr = an_tlv_get_length(tlv) - AN_TLV_HDR_SIZE;
}
#endif

static boolean 
an_msg_mgr_copy_parsed_value_len (uint8_t **value_ptr, uint16_t *len_ptr, 
                                  uint8_t type, uint8_t stype)
{   
    uint8_t *tlv = an_msg_mgr_get_tlv_from_parse_grid(type, stype);
    uint16_t len = 0;

    if (!value_ptr || !len_ptr || !tlv) {
        return (FALSE);
    }

    len = an_tlv_get_length(tlv) - AN_TLV_HDR_SIZE;
    *len_ptr = len;

    if (len) {
        *value_ptr = (uint8_t *)an_malloc_guard(len, "AN Parsed Value");
        if(!*value_ptr) {
           return (FALSE);
        }
        an_memcpy_guard(*value_ptr, an_tlv_get_value(tlv), len);

    } else {
        *value_ptr = NULL;
    }

    return (TRUE);
}

static boolean    
an_msg_mgr_copy_parsed_ipaddr (an_addr_t *addr, uint8_t type, uint8_t stype)
{
    uint8_t *tlv = an_msg_mgr_get_tlv_from_parse_grid(type, stype);
    an_v4addr_t v4addr = AN_V4ADDR_ZERO;
    an_v6addr_t v6addr = AN_V6ADDR_ZERO;

    *addr = AN_ADDR_ZERO;

    if (!tlv) {
        return (FALSE);
    }

    if (stype == AN_TLV_STYPE_IPV4_ADDR) {
        v4addr = an_addr_ntov4(an_tlv_get_value(tlv));
        an_addr_set_from_v4addr(addr, v4addr); 
        return (TRUE);

    } else if (stype == AN_TLV_STYPE_IPV6_ADDR) {
        v6addr = an_addr_ntov6(an_tlv_get_value(tlv));
        an_addr_set_from_v6addr(addr, v6addr); 
        return (TRUE);
    }

    return (FALSE);
} 

static boolean    
an_msg_mgr_copy_parsed_service_info (an_service_info_t *srvc_info , 
                                     uint8_t type)
{
    uint8_t *tlv = NULL;
    an_v4addr_t v4addr = AN_V4ADDR_ZERO;
    an_v6addr_t v6addr = AN_V6ADDR_ZERO;
    int32_t index = 0;

    for(index = AN_TLV_STYPE_AAA; index < AN_TLV_STYPE_MAX; index++) {
      if (parse_grid.tlv[type][index] != NULL) {
          tlv = parse_grid.tlv[type][index];
          srvc_info->srvc_type = AN_SERVICE_AAA + index;
          break;
      }
    }

    if (index == AN_TLV_STYPE_MAX) {
        return (FALSE);
    }

    an_log(AN_LOG_SRVC, "\n%sDecoding service info stype %d length %d", 
            an_srvc_prefix, srvc_info->srvc_type, an_tlv_get_length(tlv));

    if ((an_tlv_get_length(tlv) - AN_TLV_HDR_SIZE) == AN_ADDRLEN_IP) {
        v4addr = an_addr_ntov4(an_tlv_get_value(tlv));
        an_addr_set_from_v4addr(&srvc_info->srvc_ip, v4addr); 

    } else if ((an_tlv_get_length(tlv) - AN_TLV_HDR_SIZE) == AN_ADDRLEN_IPV6) {
        v6addr = an_addr_ntov6(an_tlv_get_value(tlv));
        an_addr_set_from_v6addr(&srvc_info->srvc_ip, v6addr); 

    } else {
        return (FALSE);
    }

    an_log(AN_LOG_SRVC, "\n%sDecoded IP in service info %s", 
            an_srvc_prefix, an_addr_get_string(&srvc_info->srvc_ip));
  
    return (TRUE);
} 
#if 0
static void    
an_msg_mgr_copy_parsed_routing_info (an_routing_cfg_t *routing_info, 
            uint8_t type, uint8_t stype)
{
    uint8_t *tlv = an_msg_mgr_get_tlv_from_parse_grid(type, stype);
    uint16_t len = 0;

    if (!tlv) {
        return;
    }

    len = an_tlv_get_length(tlv) - AN_TLV_HDR_SIZE;

    if (len) {
        an_memcpy(routing_info, an_tlv_get_value(tlv), len);
    } else {
        routing_info = NULL;
    }
} 
#endif

uint8_t *
an_msg_mgr_parse_header (uint8_t *msg_block, an_header *header)
{
    uint8_t value = 0;
    uint8_t *msg_block_p = NULL;

    msg_block_p = msg_block;

    msg_block_p = an_ntoh_1_byte_and_move(&value, msg_block_p);
        header->ver = (value & 0xF0) >> 4;
        header->reserved = value & 0x0F;
    msg_block_p = an_ntoh_1_byte_and_move(&header->protocol_type, msg_block_p);
    msg_block_p = an_ntoh_1_byte_and_move(&header->flags, msg_block_p);
    msg_block_p = an_ntoh_1_byte_and_move(&header->hop_limit, msg_block_p);
    msg_block_p = an_ntoh_2_bytes_and_move(&header->msg_type, msg_block_p);
    msg_block_p = an_ntoh_2_bytes_and_move(&header->length, msg_block_p);
    
    return (msg_block_p);
}

static boolean
an_msg_mgr_parse_message (uint8_t *msg_block, an_pak_t *pak, 
                          an_msg_package *msg_package)
{
    uint16_t parsed_len = 0, msg_len = 0;
    an_ipv6_hdr_t *ipv6_hdr = NULL;
    an_log_type_e log;

    if (!msg_package || !msg_block || !pak) {
        return (FALSE);
    }

    an_msg_mgr_init_parse_grid(); 

    msg_block = an_msg_mgr_parse_header(msg_block, &parse_grid.header);
    msg_len = parse_grid.header.length;
    parsed_len = sizeof(an_header);
    
    log = an_get_log_type(parse_grid.header.protocol_type, 
                          parse_grid.header.msg_type);

    for (; ((parsed_len < msg_len) && msg_block && (an_tlv_get_length(msg_block) > 0)); 
        msg_block = an_tlv_get_next_tlv(msg_block)) {
        parse_grid.tlv[an_tlv_get_type(msg_block)][an_tlv_get_subtype(msg_block)] 
                       = msg_block; 
        parsed_len += an_tlv_get_length(msg_block);

        DEBUG_AN_LOG(log, AN_DEBUG_INFO, NULL, "\n%sParsed message "
                     "of Tlv Type = %s, Tlv Subtype = %s, Msg Len = %d bytes, "
                     "Parsed Len = %d bytes", an_get_log_str(log),
                     an_tlv_get_tlv_type_str(msg_block),
                     an_tlv_get_tlv_subtype_str(msg_block),
                     an_tlv_get_length(msg_block), parsed_len);

    }

    if (parsed_len > msg_len) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sParsed len = [%d] "
                        "is more than message len = [%d]", 
                        an_get_log_str(log), parsed_len, msg_len);
        an_syslog(AN_SYSLOG_TLV_PARSE_LEN_INCORRECT, parsed_len, msg_len);
        an_msg_mgr_init_parse_grid();
        return (FALSE);
    }

    ipv6_hdr = (an_ipv6_hdr_t *)an_pak_get_network_hdr(pak);
    an_addr_set_from_v6addr(&msg_package->src, an_ipv6_hdr_get_src(ipv6_hdr));
    an_addr_set_from_v6addr(&msg_package->dest, an_ipv6_hdr_get_dest(ipv6_hdr));
    msg_package->ifhndl = an_pak_get_input_if(pak);
    msg_package->iptable = an_pak_get_iptable(pak);

    an_memcpy((uint8_t *)&msg_package->header, (uint8_t *)&parse_grid.header, 
              sizeof(an_header));
    if (an_msg_mgr_copy_parsed_value_len(&msg_package->udi.data, 
                                         &msg_package->udi.len,
                                         AN_TLV_TYPE_UDI, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_UDI);
    }

    if (an_msg_mgr_copy_parsed_value_len(&msg_package->sudi.data, 
                                         &msg_package->sudi.len,
                                         AN_TLV_TYPE_CERTIFICATE, 
                                         AN_TLV_STYPE_SUDI)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_SUDI);
    }

    if (an_msg_mgr_copy_parsed_value_len(&msg_package->domain_cert.data, 
                                         &msg_package->domain_cert.len,
                                         AN_TLV_TYPE_CERTIFICATE, 
                                         AN_TLV_STYPE_DOMAIN_CERT)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DOMAIN_CERT);
    }

    if (an_msg_mgr_copy_parsed_value_len(&msg_package->anra_cert.data, 
                                         &msg_package->anra_cert.len,
                                         AN_TLV_TYPE_CERTIFICATE, 
                                         AN_TLV_STYPE_ANRA_CERT)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_CERT);
    }

    if (an_msg_mgr_copy_parsed_value_after_alloc(&msg_package->device_id, 
                                                 AN_TLV_TYPE_DEVICE_ID, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DEVICE_ID);
    }

    if (an_msg_mgr_copy_parsed_value_after_alloc(&msg_package->domain_id, 
                                                 AN_TLV_TYPE_DOMAIN_ID, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DOMAIN_ID);
    }
    
    if (an_msg_mgr_copy_parsed_ipaddr(&msg_package->if_ipaddr, 
                            AN_TLV_TYPE_IF_IPADDR, AN_TLV_STYPE_IPV6_ADDR)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_IF_IPADDR);
    }
    
    if (an_msg_mgr_copy_parsed_value_after_alloc(&msg_package->if_name, 
                                                 AN_TLV_TYPE_IF_NAME, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_IF_NAME);
    }

    if (an_msg_mgr_copy_parsed_ipaddr(&msg_package->anra_ipaddr, 
                        AN_TLV_TYPE_ANRA_IPADDR, AN_TLV_STYPE_IPV6_ADDR)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_IPADDR);
    }
    
    if (an_msg_mgr_copy_parsed_value_len(&msg_package->anra_sign.data, 
                                         &msg_package->anra_sign.len,
                                         AN_TLV_TYPE_ANRA_SIGN, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_SIGN);
    }

    if (an_msg_mgr_copy_parsed_value_len(&msg_package->masa_sign.data, 
                                         &msg_package->masa_sign.len,
                                         AN_TLV_TYPE_MASA_SIGN, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_MASA_SIGN);
    }

    if (an_msg_mgr_copy_parsed_value_len(&msg_package->cert_request.data, 
                                         &msg_package->cert_request.len,
                                         AN_TLV_TYPE_CERT_REQ, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_CERT_REQ);
    }

    if (an_msg_mgr_copy_parsed_value_len(&msg_package->cert_req_sign.data, 
                                         &msg_package->cert_req_sign.len,
                                         AN_TLV_TYPE_CERT_REQ_SIGN, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_CERT_REQ_SIGN);
    }
    
    if (an_msg_mgr_copy_parsed_value_len(&msg_package->public_key.data, 
                                         &msg_package->public_key.len,
                                         AN_TLV_TYPE_PUBLIC_KEY, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_PUB_KEY);
    }
    
    if (an_msg_mgr_copy_parsed_value_len(&msg_package->payload.data, 
                                         &msg_package->payload.len,
                                         AN_TLV_TYPE_ACP_PAYLOAD, 1)) {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ACP_PAYLOAD);
    }

    if (an_msg_mgr_copy_parsed_service_info(&msg_package->srvc_info, 
                                            AN_TLV_TYPE_SERVICE))
    {
        AN_SET_BIT_FLAGS(msg_package->interest, AN_MSG_INT_SERVICE_INFO);
    }

    return (TRUE);
}

static uint16_t
an_msg_mgr_calculate_ipaddr_value_len (an_addr_t addr)
{
    uint16_t len = an_addr_get_len(addr);
    
    if (len) {
        return (len);
    } else {
        return (sizeof(AN_V6ADDR_ZERO));
    }
}

uint16_t
an_msg_mgr_calculate_msg_len (an_msg_package *msg_package)
{
    uint16_t msg_len = 0;
    an_log_type_e log;

    if (!msg_package) {
        return (0);
    }    
    log = an_get_log_type(msg_package->header.protocol_type,
                          msg_package->header.msg_type);
    
    msg_len += sizeof(an_header);
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_UDI) ?
               msg_package->udi.len + AN_TLV_HDR_SIZE : 0;  
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DOMAIN_CERT) ?
               msg_package->domain_cert.len + AN_TLV_HDR_SIZE : 0;  
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_SUDI) ?
               msg_package->sudi.len + AN_TLV_HDR_SIZE : 0;  
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DEVICE_ID) ?
               an_strlen(msg_package->device_id) + AN_TLV_HDR_SIZE + 1 : 0;  
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DOMAIN_ID) ?
               an_strlen(msg_package->domain_id) + AN_TLV_HDR_SIZE + 1 : 0;  
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_IF_IPADDR) ?
               an_msg_mgr_calculate_ipaddr_value_len(msg_package->if_ipaddr) + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_IF_NAME) ?
               an_strlen(msg_package->if_name) + AN_TLV_HDR_SIZE + 1 : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_IPADDR) ?
               an_msg_mgr_calculate_ipaddr_value_len(msg_package->anra_ipaddr) + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_SIGN) ?
               msg_package->anra_sign.len + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_CERT) ?
               msg_package->anra_cert.len + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_CERT_REQ) ?
               msg_package->cert_request.len + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_CERT_REQ_SIGN) ?
               msg_package->cert_req_sign.len + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_PUB_KEY) ?
               msg_package->public_key.len + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ACP_PAYLOAD) ?
               msg_package->payload.len + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_MASA_SIGN) ?
               msg_package->masa_sign.len + AN_TLV_HDR_SIZE : 0;
    msg_len += AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_SERVICE_INFO) ?
                an_addr_get_len(msg_package->srvc_info.srvc_ip) + AN_TLV_HDR_SIZE : 0;
    
    return (msg_len);
}

boolean
an_msg_mgr_compose_message (uint8_t *msg_block, an_msg_package *msg_package)
{
    
    if (!msg_package || !msg_block) {
        return (FALSE);
    }

    msg_block = an_header_compose_and_move(msg_block, msg_package->header);

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_UDI)) {
        msg_block = an_tlv_compose_udi_and_move(msg_block, msg_package->udi);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_SUDI)) {
        msg_block = an_tlv_compose_sudi_and_move(msg_block, msg_package->sudi);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DOMAIN_CERT)) {
        msg_block = an_tlv_compose_domain_cert_and_move(msg_block, 
                                                        msg_package->domain_cert);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DEVICE_ID)) {
        msg_block = an_tlv_compose_device_id_and_move(msg_block, msg_package->device_id);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_DOMAIN_ID)) {
        msg_block = an_tlv_compose_domain_id_and_move(msg_block, msg_package->domain_id);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_IF_IPADDR)) {
        msg_block = an_tlv_compose_if_ipaddr_and_move(msg_block, msg_package->if_ipaddr);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_IF_NAME)) {
        msg_block = an_tlv_compose_if_name_and_move(msg_block, msg_package->if_name);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_IPADDR)) {
        msg_block = an_tlv_compose_anra_ipaddr_and_move(msg_block, msg_package->anra_ipaddr);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_MASA_SIGN)) {
        msg_block = an_tlv_compose_masa_sign_and_move(msg_block, msg_package->masa_sign);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_SIGN)) {
        msg_block = an_tlv_compose_anra_sign_and_move(msg_block, msg_package->anra_sign);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ANRA_CERT)) {
        msg_block = an_tlv_compose_anra_cert_and_move(msg_block, msg_package->anra_cert);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_CERT_REQ)) {
        msg_block = an_tlv_compose_cert_request_and_move(msg_block, msg_package->cert_request);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_CERT_REQ_SIGN)) {
        msg_block = an_tlv_compose_cert_req_sign_and_move(msg_block, msg_package->cert_req_sign);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_PUB_KEY)) {
        msg_block = an_tlv_compose_public_key_and_move(msg_block, msg_package->public_key);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_ACP_PAYLOAD)) {
        msg_block = an_tlv_compose_acp_client_data_and_move(msg_block, msg_package->payload);
    }

    if (AN_CHECK_BIT_FLAGS(msg_package->interest, AN_MSG_INT_SERVICE_INFO)) {
        msg_block = an_tlv_compose_service_info_data_and_move(msg_block, 
                                     &msg_package->srvc_info);
    }

    return (TRUE);
}

static uint8_t *
an_msg_mgr_extract_message_block_from_icmp (an_ipv6_hdr_t *ipv6_hdr, 
                                          an_icmp6_hdr_t *icmp6_hdr)
{
    uint16_t icmp_len = 0;
    an_ipv6_nd_opt_hdr *option_hdr = NULL;
    an_ipv6_nd_opt *option = NULL;
    uint8_t *msg_block = NULL;

    if (!ipv6_hdr || !icmp6_hdr) {
        return (NULL);
    }

    if (AN_ND_HELLO != an_icmp6_get_type(icmp6_hdr)) {
        return (NULL);
    }
    
    icmp_len = an_icmp6_get_len(icmp6_hdr, ipv6_hdr);
    option_hdr = an_icmp6_get_an_nd_opt_hdr(icmp6_hdr, icmp_len);
    if (!option_hdr) {
        return (NULL);
    }
    option = (an_ipv6_nd_opt *)option_hdr;
    msg_block = (uint8_t *)option->value;

    return (msg_block);
}

static boolean 
an_msg_mgr_create_message_block_in_icmp (an_pak_t **pak, uint8_t **msg_block, uint16_t block_len)
{
    uint16_t extra_len = 0, icmp_len = 0;
    an_ipv6_hdr_t *ipv6_hdr = NULL;
    an_icmp6_hdr_t *icmp6_hdr = NULL;
    an_ipv6_nd_opt *an_opt = NULL;

    if (!pak || !*pak) {
        return (FALSE);
    }

    ipv6_hdr = (an_ipv6_hdr_t *)an_pak_get_network_hdr(*pak);
    icmp6_hdr = (an_icmp6_hdr_t *)an_pak_get_transport_hdr(*pak);

    /* Resize the packet to carry the AN Hello options */
    extra_len = block_len; 
    if (!an_pak_grow(pak, extra_len)) {
        an_log(AN_LOG_MSG_MGR | AN_LOG_ERR, 
               "\nFailed to create AN message block in ICMP packet");
        return (FALSE);
    }

    /* Reset the pointers */
    ipv6_hdr = (an_ipv6_hdr_t *)an_pak_get_network_hdr(*pak);
    icmp6_hdr = (an_icmp6_hdr_t *)an_pak_get_transport_hdr(*pak);

    /* Get AN Option Header */
    icmp_len = an_icmp6_get_len(icmp6_hdr, ipv6_hdr);
    an_opt = (an_ipv6_nd_opt *)((uint8_t*)ipv6_hdr + sizeof(an_ipv6_hdr_t) +
              an_ipv6_hdr_get_paylen(ipv6_hdr));
    an_opt->type = AN_ND_OPT;
    an_opt->length = (block_len)/8;
    an_opt->padd[0] = 0;
    an_opt->padd[1] = 0;

    an_log(AN_LOG_MSG_MGR, "\n%sCreated AN message block in ICMPv6-ND message",
            an_msg_mgr_prefix);

    *msg_block = (uint8_t *)an_opt->value;
    
    return (TRUE);
}

static boolean
an_msg_mgr_close_message_block_in_icmp (an_pak_t *pak, uint8_t *msg_block,
                     uint16_t msg_len, uint16_t icmp_opt_len)
{
    uint16_t /*pak_len = 0,*/ icmp_cksum = 0, pad_len = 0;
    an_ipv6_hdr_t *ipv6_hdr = NULL;
    an_icmp6_hdr_t *icmp6_hdr = NULL;

    if (!pak || !msg_block) {
        return (FALSE);
    }

    ipv6_hdr = (an_ipv6_hdr_t *)an_pak_get_network_hdr(pak);
    icmp6_hdr = (an_icmp6_hdr_t *)an_pak_get_transport_hdr(pak);

    /* Update IPv6 Payload Length */
    an_ipv6_hdr_set_paylen(ipv6_hdr, an_ipv6_hdr_get_paylen(ipv6_hdr) + icmp_opt_len);

    /* Padd AN-ICMP Option with all zeros */    
    pad_len = icmp_opt_len - sizeof(an_ipv6_nd_opt) - msg_len;
    an_memset(msg_block + sizeof(an_ipv6_nd_opt) + msg_len, 0, pad_len);

    /* Re-Calculate the ICMPv6 Checksum */
    an_icmp6_reset_cksum(icmp6_hdr);
    icmp_cksum = an_ipv6_calculate_cksum(ipv6_hdr, icmp6_hdr, IPPROTO_ICMPV6);
    an_icmp6_set_cksum(icmp6_hdr, icmp_cksum);

    an_log(AN_LOG_MSG_MGR, "\n%sClosed AN message block in ICMPv6-ND message",
            an_msg_mgr_prefix);

    return (TRUE);
}

boolean
an_msg_mgr_deliver_incoming_message (an_msg_package *msg_package, 
                           an_pak_t *pak, an_if_t ifhndl)
{
    an_if_info_t* an_if_info = NULL;
    uint16_t msg_len;

    an_log_type_e log;
    log = an_get_log_type(msg_package->header.protocol_type,
                          msg_package->header.msg_type);

    if (ifhndl != 0) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                     "\n%sIncoming Message %s of length [%d Bytes] on %s\n",
                     an_get_log_str(log), 
                     an_get_msg_name(msg_package->header.msg_type),
                     (msg_len =  an_msg_mgr_calculate_msg_len(msg_package)), 
                     an_if_get_name(ifhndl));
    } else {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                     "\n%sIncoming Message %s for bootstrapping thyself\n",
                     an_get_log_str(log), 
                     an_get_msg_name(msg_package->header.msg_type));
    } 
    
    an_msg_mgr_log_message(msg_package, log);

    if (!an_is_global_cfg_autonomic_enabled()) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sAutonomic is disabled " 
                     "globally, hence don't handle the Incoming Message %s", 
                     an_get_log_str(log),
                     an_get_msg_name(msg_package->header.msg_type));
        an_msg_mgr_free_message_package(msg_package);
        return (FALSE);
    }
    
    /*
     * Only for interfaces which are valid do the check - if autonomic 
     * is enabled on the interfaces.
     * For bootstrapping ourselves, there wouldn't be an input interface.
     */
    if (ifhndl) {
        an_if_info = an_if_info_db_search(ifhndl, FALSE);  
        if (!an_if_is_cfg_autonomic_enabled(an_if_info)) {
           DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sAutonomic disabled "
                     "on the interface %s hence do not handle the Incoming " 
                     "Message %s", an_get_log_str(log), an_if_get_name(ifhndl), 
                     an_get_msg_name(msg_package->header.msg_type));
            an_msg_mgr_free_message_package(msg_package);
            return (FALSE);        
        }
    }

    switch (msg_package->header.msg_type) {
    case AN_MSG_ND_HELLO:
        an_nd_incoming_hello(msg_package, pak, ifhndl);
        break;

    case AN_MSG_ND_CERT_REQUEST:
        an_ni_incoming_cert_request(msg_package);
        break;

    case AN_MSG_ND_CERT_RESPONSE:
        an_ni_incoming_cert_response(msg_package);
        break;

    case AN_MSG_BS_INVITE:
        an_bs_incoming_invite_message(msg_package);
        break;

    case AN_MSG_BS_REJECT:
        an_bs_incoming_reject_message(msg_package);
        break;

    case AN_MSG_BS_ENROLL_QUARANTINE:
        an_bs_incoming_enroll_quarantine_message(msg_package);
        break;

    case AN_MSG_NBR_CONNECT:
        an_anra_incoming_nbr_connect_message(msg_package);
        break;

    case AN_MSG_BS_REQUEST:
        an_anra_incoming_bs_request_message(msg_package);
        break;

    case AN_MSG_BS_RESPONSE:
        an_bs_incoming_response_message(msg_package);
        break;

    case AN_MSG_ACP_DATA:
        an_acp_incoming_message(msg_package);
        break;

    default:
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sUnrecognized Msg Type "
                     "[%s]", an_get_log_str(log), 
                     an_get_msg_name(msg_package->header.msg_type)); 
        an_syslog(AN_SYSLOG_MSG_INVALID_HEADER,msg_package->header.msg_type);
        break;
    }
    
    return (TRUE);
}

boolean 
an_msg_mgr_deliver_outgoing_message (an_pak_t *pak, an_msg_package *msg_package)
{   
    uint16_t msg_len = 0;
    if (!msg_package) {
        return (FALSE);
    }
    an_log_type_e log;
    log = an_get_log_type(msg_package->header.protocol_type,
                          msg_package->header.msg_type);

    DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                 "\n%sSending %s Message of length [%d Bytes] on %s\n",
                 an_get_log_str(log), 
                 an_get_msg_name(msg_package->header.msg_type),
                 (msg_len =  an_msg_mgr_calculate_msg_len(msg_package)),
                 an_if_get_name(msg_package->ifhndl));
    
    an_msg_mgr_log_message(msg_package, log);

    switch (msg_package->header.msg_type) {
    case AN_MSG_ND_HELLO:
        if (an_nd_delivery == AN_ND_DELIVERY_IPV6_ND) {
        break;
        }
    
    case AN_MSG_ND_CERT_REQUEST:
    case AN_MSG_ND_CERT_RESPONSE:
    case AN_MSG_NBR_CONNECT:
    case AN_MSG_BS_INVITE:
    case AN_MSG_BS_REJECT:
    case AN_MSG_BS_REQUEST:
    case AN_MSG_BS_RESPONSE:
    case AN_MSG_BS_ENROLL_QUARANTINE:
    case AN_MSG_ACP_DATA:
    case AN_MSG_IDP_INTENT:
    case AN_MSG_IDP_INTENT_VERSION:
    case AN_MSG_IDP_INTENT_REQUEST:
    case AN_MSG_IDP_ACK:
    case AN_MSG_SERVICE_INFO:
    case AN_MSG_SERVICE_INFO_ACK:

        if (msg_package->ifhndl) {
            an_ipv6_preroute_pak(pak, msg_package->ifhndl, msg_package->dest);
        }
        an_ipv6_forward_pak(pak);
        break;

    default:
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sUnrecognized Msg Type "
                     "[%s]", an_get_log_str(log), 
                     an_get_msg_name(msg_package->header.msg_type)); 
        an_syslog(AN_SYSLOG_MSG_INVALID_HEADER,msg_package->header.msg_type);
        break;
    }

    return (TRUE);
}

/*static*/ boolean
an_msg_mgr_receive_an_message (uint8_t *msg_block, an_pak_t *pak, an_if_t ifhndl)
{
    an_msg_package *msg_package = NULL;
    boolean result = FALSE;

    if (!msg_block || !pak) {
        return (result);
    }
    
    msg_package = an_msg_mgr_get_empty_message_package();
    
    an_msg_mgr_parse_message(msg_block, pak, msg_package);

    result = an_msg_mgr_deliver_incoming_message(msg_package, pak, ifhndl);
    if(!result) {
        return (result);
    }

    return (result);
}

void
an_msg_mgr_incoming_message (an_pak_t *pak)
{
    an_if_t ifhndl = 0;
    uint8_t *msg_block = NULL;
    an_ipv6_hdr_t *ipv6_hdr = NULL;
    uint16_t protocol = 0;
    uint8_t linktype;

    if (!pak) {
        return;
    }

    linktype = an_pak_get_linktype(pak);
    if (an_linktype_is_an(linktype)) {
        an_handle_l2_pak(pak);
        return;
    }

    ifhndl = an_pak_get_input_if(pak);
    ipv6_hdr = (an_ipv6_hdr_t *)an_pak_get_network_hdr(pak); 
    if (!ipv6_hdr) {
        return;
    }
    
    protocol = an_ipv6_hdr_get_next(ipv6_hdr);

    if (AN_IPV6_PROTOCOL == protocol) {
        msg_block = an_pak_get_network_hdr(pak) + AN_IPV6_HDR_SIZE; 
    } else if (AN_UDP_PROTOCOL == protocol) {
        msg_block = an_pak_get_network_hdr(pak) +
                    AN_IPV6_HDR_SIZE + AN_UDP_HDR_SIZE; 
    } else {
        an_pak_free(pak);
        return;
    }
    an_msg_mgr_receive_an_message(msg_block, pak, ifhndl);
    an_pak_free(pak);
}

uint8_t *
an_msg_mgr_create_message_block_in_packet (an_pak_t **pak_out, 
                an_msg_package *message, uint16_t msg_len)
{   
    an_pak_t *pak = NULL;
    uint8_t *iphdr = NULL, *udp_hdr = NULL;
    uint16_t iphdr_len = 0, udp_hdr_len = 0, paklen = 0;
    an_addr_t source = AN_ADDR_ZERO, destination = AN_ADDR_ZERO;
    uint8_t *msg_block = NULL;
    an_v6addr_t src_v6addr = AN_V6ADDR_ZERO, dest_v6addr = AN_V6ADDR_ZERO;
    an_cerrno rc = EOK;
    
    an_log_type_e log;
    log = an_get_log_type(message->header.protocol_type,
                          message->header.msg_type);

    /* get the source & destination addresses */
    destination = message->dest;
    dest_v6addr = an_addr_get_v6addr(destination);

    if (!an_addr_is_zero(message->src)) {
        source = message->src;
    } else if (an_addr_is_ipv6_linklocal(destination)) {
        if (!message->ifhndl) {
            return (NULL);
        }
        src_v6addr = an_ipv6_get_ll(message->ifhndl);
        an_addr_set_from_v6addr(&source, src_v6addr);
    } else if (an_addr_is_zero(message->src)) {
        source = an_ipv6_get_best_source_addr(destination, message->iptable);
    } else {
        source = message->src;
    }
    src_v6addr = an_addr_get_v6addr(source);

    /* allocate the packet */
    iphdr_len = AN_IPV6_HDR_SIZE;
    udp_hdr_len = AN_UDP_HDR_SIZE;
    if (an_msg_delivery == AN_MSG_DELIVERY_UDP) {
        paklen = iphdr_len + udp_hdr_len + msg_len;  
    } else if (an_msg_delivery == AN_MSG_DELIVERY_IPV6) {
        paklen = iphdr_len + msg_len;  
    } else {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sUnsupported Msg Delivery", 
                     an_get_log_str(log));
        return (NULL);
    }

    rc = an_pak_alloc(paklen, &pak);
    if (CERR_IS_NOTOK(rc)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sMemory Alloc failed " 
                      "for pak", an_get_log_str(log));
        return (NULL);
    } 

    an_pak_set_iptable(pak, message->iptable);
    *pak_out = pak;

    if (an_msg_delivery == AN_MSG_DELIVERY_UDP) {
    /* setup the ipv6 header */
        iphdr = an_pak_get_network_hdr(pak);
        an_ipv6_hdr_init(iphdr,
                AN_DEFAULT_TOS,
                AN_DEFAULT_FLOW_LABEL,
                msg_len + udp_hdr_len,
                AN_UDP_PROTOCOL,
                AN_DEFAULT_HOP_LIMIT,
                &src_v6addr,
                &dest_v6addr);

        /* setup the udp header */
        udp_hdr = iphdr + iphdr_len;
        an_udp_build_header(iphdr, udp_hdr, AN_UDP_PORT, AN_UDP_PORT, msg_len);
        msg_block = iphdr + iphdr_len + udp_hdr_len;

    } else if (an_msg_delivery == AN_MSG_DELIVERY_IPV6) {
        /* setup the ipv6 header */
        iphdr = an_pak_get_network_hdr(pak);
        an_ipv6_hdr_init(iphdr,
                AN_DEFAULT_TOS,
                AN_DEFAULT_FLOW_LABEL,
                msg_len,
                AN_IPV6_PROTOCOL,
                AN_DEFAULT_HOP_LIMIT,
                &src_v6addr,
                &dest_v6addr);

        msg_block = iphdr + iphdr_len;

    } else {
      DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sUnsupported Msg Delivery", 
                     an_get_log_str(log));
    }
    
    return (msg_block);
}

void 
an_msg_mgr_close_message_block_in_packet (an_pak_t *pak, uint8_t *msg_block, 
                                          uint16_t msg_len)
{
    if (an_msg_delivery == AN_MSG_DELIVERY_UDP) {
        an_udp_update_checksum(pak); 
    }
}

void
an_msg_mgr_send_message (an_msg_package *msg_package)
{
    uint8_t *msg_block = NULL, *temp_msg_block = NULL;
    uint16_t msg_len = 0;
    an_pak_t *pak = NULL;
    boolean status = FALSE;
    an_log_type_e log;

    if (!msg_package) {
        return;
    }

    log = an_get_log_type(msg_package->header.protocol_type,
                          msg_package->header.msg_type);
    msg_len = an_msg_mgr_calculate_msg_len(msg_package);
    msg_package->header.length = msg_len;

    if (an_addr_equal(&msg_package->dest, &msg_package->src)) {
        /* message to/from local anra, switch the message locally */
        if (msg_package->ifhndl == 0) {
            DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                     "\n%sSending Message [%s] to thyself\n",
                     an_get_log_str(log), 
                     an_get_msg_name(msg_package->header.msg_type));
        } else {
            DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                     "\n%sSending Message [%s] on %s",
                     an_get_log_str(log), 
                     an_get_msg_name(msg_package->header.msg_type),
                     an_if_get_name(msg_package->ifhndl));
        }
        an_msg_mgr_log_message(msg_package, log); 
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "\n%sSince Src [%s] & "
                     "Dest [%s] are equal,\n%38shence punting %s message to "
                     "AN Registrar", an_get_log_str(log), 
                     an_addr_get_string(&msg_package->src), 
                     an_addr_get_string(&msg_package->dest), " ",
                     an_get_msg_name(msg_package->header.msg_type));
        an_msg_mgr_deliver_incoming_message(msg_package, NULL, 0);
        return;
    }

    temp_msg_block = (uint8_t *)an_malloc_guard(msg_len, "AN MSG Temp Block");
    if (!temp_msg_block) {
        return;
    }

    status = an_msg_mgr_compose_message(temp_msg_block, msg_package);
    if (!status) {
        an_free_guard(temp_msg_block);    
        return;
    }

    msg_block = an_msg_mgr_create_message_block_in_packet(&pak, msg_package, 
                                                          msg_len);

    an_memcpy(msg_block, temp_msg_block, msg_len);
    an_free_guard(temp_msg_block);

    an_msg_mgr_close_message_block_in_packet(pak, msg_block, msg_len);
    an_msg_mgr_deliver_outgoing_message(pak, msg_package);

    an_msg_mgr_free_message_package(msg_package);
}

boolean
an_msg_mgr_incoming_ipv6_na (an_pak_t *pak, an_if_t ifhndl, 
                             an_ipv6_hdr_t *ipv6_hdr,
                             an_icmp6_hdr_t *icmp6_hdr)
{
    uint8_t *msg_block = NULL;

    if (!pak || !ipv6_hdr || !icmp6_hdr) {
        return (TRUE);
    }

    msg_block = an_msg_mgr_extract_message_block_from_icmp(ipv6_hdr, icmp6_hdr);
    if (!msg_block) {
        return (TRUE);
    }    

    an_msg_mgr_receive_an_message(msg_block, pak, ifhndl);

    return (TRUE);
}

boolean
an_msg_mgr_outgoing_ipv6_na (an_pak_t *pak, an_if_t ifhndl, 
                             an_ipv6_hdr_t *ipv6_hdr,
                             an_icmp6_hdr_t *icmp6_hdr)
{
    uint16_t msg_len = 0, icmp_opt_len = 0;
    uint8_t *msg_block = NULL, *temp_msg_block = NULL;
    an_msg_package *msg_package = NULL;
    boolean status = FALSE;
    an_if_info_t *an_if_info = NULL;

    if (!pak || !ipv6_hdr || !icmp6_hdr) {
        return (TRUE);
    }

    if (an_nd_delivery != AN_ND_DELIVERY_IPV6_ND) {
        return (TRUE);
    }

    an_if_info = an_if_info_db_search(ifhndl, FALSE);
    if (!an_if_info) {
        return (TRUE);
    } 

    if (!an_if_info->nd_hello_pending) {
        return (TRUE);
    }
    an_if_info->nd_hello_pending = FALSE;

    msg_package = an_msg_mgr_get_hello_message_package(ifhndl);
    if (!msg_package) {
        return (TRUE);
    }

    msg_len = an_msg_mgr_calculate_msg_len(msg_package);
    msg_package->header.length = msg_len;

    temp_msg_block = an_malloc_guard(msg_len, "AN MSG Temp Block");
    if (!temp_msg_block) {
        an_msg_mgr_free_message_package(msg_package);
        return (FALSE);
    }

    status = an_msg_mgr_compose_message(temp_msg_block, msg_package);
    if (!status) {
        an_free_guard(temp_msg_block);
        an_msg_mgr_free_message_package(msg_package);
        return (FALSE);
    }

    icmp_opt_len = msg_len + sizeof(an_ipv6_nd_opt); 
    if ((icmp_opt_len%8) != 0) {
        icmp_opt_len += (8 - (icmp_opt_len%8));
    }

    status = an_msg_mgr_create_message_block_in_icmp(&pak, &msg_block, 
             icmp_opt_len);
    if (!status) {
        an_msg_mgr_free_message_package(msg_package);
        return (FALSE);
    }

    an_memcpy(msg_block, temp_msg_block, msg_len);
    an_free_guard(temp_msg_block);

    status = an_msg_mgr_close_message_block_in_icmp(pak, msg_block, msg_len, 
                                                    icmp_opt_len);
    if (!status) {
        an_msg_mgr_free_message_package(msg_package);
        return (FALSE);
    }

    status = an_msg_mgr_deliver_outgoing_message(pak, msg_package);
    if (!status) {
        an_msg_mgr_free_message_package(msg_package);
        return (FALSE);
    }

    an_msg_mgr_free_message_package(msg_package);
    return (TRUE);
}

void
an_msg_mgr_log_message (an_msg_package *message, an_log_type_e log)
{
    if (!message) {
        return;
    }
    
    DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sMessage: %s\n", " ",
                 an_get_msg_name(message->header.msg_type));
    DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sSrc: %s,  "
                 "Dest: %s\n", " ",
                 an_addr_get_string(&message->src), 
                 an_addr_get_string(&message->dest));
    
    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_UDI)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sUdi: %s\n", 
                     " ", message->udi.data);
    }
#if 0
    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_SUDI)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sSudi Cert ", " "); 
        if (an_log_is_enabled_for_type_lev(log, AN_DEBUG_MODERATE)) {
            //an_cert_short_display(message->sudi, AN_LOG_MSG_MGR | log);
            an_certificate_display(message->sudi, log); 
        }
    }
#endif

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_DOMAIN_CERT)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sDomain Cert ", " "); 
        if (an_log_is_enabled_for_type_lev(log, AN_DEBUG_MODERATE)) {
            //an_cert_short_display(message->domain_cert, AN_LOG_MSG_MGR | log);
            an_certificate_display(message->domain_cert, log);
        }
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_DEVICE_ID) && (global_dev_id_set)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sDevice ID = %s,  ", 
                     " ", message->device_id);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_DOMAIN_ID) && (global_dom_id_set)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "Domain ID = %s\n",
                     message->domain_id);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_IF_IPADDR)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sIF IPAddr = %s,  ",
                     " ", an_addr_get_string(&message->if_ipaddr));
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_IF_NAME)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "IF Name = %s\n",
                     message->if_name);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_ANRA_IPADDR)) {
     DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sAN Registrar IPAddr = %s\n",
                     " ", an_addr_get_string(&message->anra_ipaddr));
    }


    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_ANRA_CERT)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sAN Registrar Cert ", 
                     " "); 
        if (an_log_is_enabled_for_type_lev(log, AN_DEBUG_MODERATE)) {
            //an_cert_short_display(message->anra_cert, log);
            an_certificate_display(message->anra_cert, log);
        }
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_ANRA_SIGN)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sRegistrar "
                     "Sign Len = %d Bytes\n", " ", message->anra_sign.len);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_CERT_REQ)) {
       DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sCert Req Len = %d Bytes,",
                     " ", message->cert_request.len);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_CERT_REQ_SIGN)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "  Cert Req Sign "
                     "Len = %d Bytes, ", message->cert_req_sign.len);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_PUB_KEY)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "Public Key "
                     "Len = %d Bytes\n", message->public_key.len);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_IDP_VERSION)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sIdp Verion = %d\n",
                     " ", message->intent_version);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_ACP_PAYLOAD)) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, "%41sjhfjsadjf = %d "
                     "Bytes\n", " ", message->payload.len);
    }

    if (AN_CHECK_BIT_FLAGS(message->interest, AN_MSG_INT_SERVICE_INFO)) {
    }

}

boolean
an_msg_mgr_add_anra_signature (an_msg_package *message)
{
    uint8_t *data_block = NULL;
    uint16_t data_len = 0;
    boolean result = FALSE;
    an_sign_api_ret_enum retval;
    an_log_type_e log;
    log = an_get_log_type(message->header.protocol_type,
                          message->header.msg_type);

    message->anra_sign.data = NULL;
    message->anra_sign.len = 0;

    data_len = an_msg_mgr_calculate_msg_len(message);
    data_block = (uint8_t *)an_malloc_guard(data_len, "AN MSG SIGN ADD");
    if (!data_block) {
        return (FALSE);
    }

    result = an_msg_mgr_compose_message(data_block, message);
    if (!result) {
        an_free_guard(data_block);
        return (result);
    }

    /* Dont forget to put message length before signing the block*/
    ((an_header *)data_block)->length = data_len;

    retval = an_sign_data(data_block, data_len, AN_SIGN_DIGEST_MD5, 
                          &message->anra_sign, ANRA_CS_TP_LABEL);
    if (retval != AN_SIGN_API_SUCCESS) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                     "\n%sAN Registrar Msg [%s] add signature result %s",
                     an_get_log_str(log), 
                     an_get_msg_name(message->header.msg_type),
                     an_sign_enum_get_string(result));
 
        an_free_guard(data_block);
        return (FALSE);
    }
    an_free_guard(data_block);
    return (TRUE); 
}

boolean
an_msg_mgr_verify_anra_signature (an_msg_package *message, an_cert_t cert)
{
    uint8_t *data_block = NULL;
    uint16_t data_len = 0;
    an_sign_t sign = {};
    boolean result = FALSE;
    an_sign_api_ret_enum retval;
    uint8_t* res_str = NULL;
    an_log_type_e log;
    log = an_get_log_type(message->header.protocol_type,
                          message->header.msg_type);

    if (!message->anra_sign.data || !message->anra_sign.len) { 
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                     "\n%sMissing Signature in Message [%s], cant verify it",
                     an_get_log_str(log), 
                     an_get_msg_name(message->header.msg_type));    
        return (FALSE);
    }

    if (!cert.data || !cert.len) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL,
                     "\n%sMissing Certificate in Message [%s], cant verify it",   
                     an_get_log_str(log), 
                     an_get_msg_name(message->header.msg_type));
        return (FALSE);
    }
    
    /* zero out signature in message, before verifying it*/
    sign.data = message->anra_sign.data;
    sign.len  = message->anra_sign.len;
    message->anra_sign.data = NULL;
    message->anra_sign.len = 0;
    AN_CLEAR_BIT_FLAGS(message->interest, AN_MSG_INT_ANRA_SIGN);

    data_len = an_msg_mgr_calculate_msg_len(message);
    data_block = (uint8_t *)an_malloc_guard(data_len, "AN MSG SIGN VERIFY");
    if (!data_block) {
        return (FALSE);
    }

    result = an_msg_mgr_compose_message(data_block, message);
    if (!result) {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                     "\n%sMessage [%s] Composition Failed", an_get_log_str(log),
                     an_get_msg_name(message->header.msg_type));  
        an_free_guard(data_block);    
        return (result);
    }

    /* donot forget to reduce the message length by signature's length */
    ((an_header *)data_block)->length = data_len;

    retval = an_verify_signature(data_block, data_len, AN_SIGN_DIGEST_MD5, 
                                 cert, sign);
    res_str = (retval == AN_SIGN_API_SUCCESS) ? "passed" : "failed";
    if (retval != AN_SIGN_API_SUCCESS) {
         DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL, 
                      "\n%s%s for %s message of size "
                      "[%d Bytes] failed", an_get_log_str(log), 
                      an_sign_enum_get_string(result),
                      an_get_msg_name(message->header.msg_type),
                      data_len);

        an_syslog(AN_SYSLOG_ANRA_SIGN_VERIFY_FAIL,"%s", res_str);
        an_free_guard(data_block);
        return (FALSE);
    } else {
        DEBUG_AN_LOG(log, AN_DEBUG_MODERATE, NULL,
                     "\n%sSignature Verification for the %s message of size " 
                     "[%d Bytes] passed", an_get_log_str(log),
                     an_get_msg_name(message->header.msg_type),
                     data_len);
    }


    /* put back the signature in the message & account its length */
    message->anra_sign.data = sign.data;
    message->anra_sign.len = sign.len;
    
    an_free_guard(data_block);
    return (TRUE); 
}