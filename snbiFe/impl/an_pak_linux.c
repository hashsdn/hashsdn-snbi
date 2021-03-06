/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include <an_pak.h>
#include <an_str.h>
#include <net/if.h>
#include <an_ipv6.h>
#include <an_mem.h>
#include <an_types.h>
#include <an_logger.h>
#include <an_if_mgr.h>
#include <olibc_pak.h>
#include "an_l2_linux.h"
#include "an_pak_linux.h"
#include "an_proc_linux.h"
#include <an_udp.h>

boolean
an_pak_init_ipv6_udp_hdr (olibc_pak_hdl pak_hdl)
{
    olibc_retval_t retval;
    uint32_t data_length = 0;
    uint8_t *data_buff = NULL;
    uint8_t *udp_hdr = NULL;
    struct sockaddr_storage src_addr, dst_addr;
    struct sockaddr_in6 *s6_src = NULL, *s6_dst = NULL;

    retval = olibc_pak_get_data_buffer(pak_hdl, &data_buff, &data_length);

    if (retval != OLIBC_RETVAL_SUCCESS) {
        DEBUG_AN_LOG(AN_LOG_ND_PACKET, AN_DEBUG_MODERATE, NULL,
                  "\nFailed to get data buffer");
        return FALSE;
    }

    memset(&src_addr, 0, sizeof(struct sockaddr_storage));
    memset(&dst_addr, 0, sizeof(struct sockaddr_storage));

    retval = olibc_pak_get_src_addr(pak_hdl, &src_addr);

    if (retval != OLIBC_RETVAL_SUCCESS) {
        DEBUG_AN_LOG(AN_LOG_ND_PACKET, AN_DEBUG_MODERATE, NULL,
                  "\nFailed to get IPv6 Src address");
        return FALSE;
    }

    retval = olibc_pak_get_dst_addr(pak_hdl, &dst_addr);
    if (retval != OLIBC_RETVAL_SUCCESS) {
        DEBUG_AN_LOG(AN_LOG_ND_PACKET, AN_DEBUG_MODERATE, NULL,
                  "\nFailed to get IPv6 dst address");
        return FALSE;
    }

    if (src_addr.ss_family != AF_INET6 || dst_addr.ss_family != AF_INET6) {
        DEBUG_AN_LOG(AN_LOG_ND_PACKET, AN_DEBUG_MODERATE, NULL,
                "\nInvalid addres family found src_af %d, "
                "dst _af %d",src_addr.ss_family, dst_addr.ss_family);
        return FALSE;
    }

    s6_src = (struct sockaddr_in6 *)&src_addr;
    s6_dst = (struct sockaddr_in6 *)&dst_addr;

    if (!an_ipv6_hdr_init(data_buff,
                AN_DEFAULT_TOS, AN_DEFAULT_FLOW_LABEL,
                data_length, AN_UDP_PROTOCOL, AN_DEFAULT_HOP_LIMIT,
                &s6_src->sin6_addr, &s6_dst->sin6_addr)) {
        DEBUG_AN_LOG(AN_LOG_ND_PACKET, AN_DEBUG_MODERATE, NULL,
                  "\nFailed to init packet header");
        return FALSE;
    }

    udp_hdr = data_buff + AN_IPV6_HDR_SIZE;

    if (!an_udp_build_header(data_buff, udp_hdr, AN_UDP_PORT, AN_UDP_PORT,
                data_length - AN_IPV6_HDR_SIZE - AN_UDP_HDR_SIZE)) {
        DEBUG_AN_LOG(AN_LOG_ND_PACKET, AN_DEBUG_MODERATE, NULL,
                "\nFailed to init UDP header");
        return FALSE;
    }

    return TRUE;
}

inline uint8_t* an_pak_get_network_hdr (an_pak_t *pak) 
{   
    olibc_retval_t retval;
    uint8_t *data_buff = NULL;
    uint32_t data_len = 0;

    if (pak) {
        retval = olibc_pak_get_data_buffer(pak, &data_buff, &data_len);
        if (retval == OLIBC_RETVAL_SUCCESS) {
            return data_buff;
        }
    }
    return NULL;
}

inline uint8_t* an_pak_get_datagram_hdr (an_pak_t *pak)
{
#ifdef PRINT_STUBS_PRINTF    
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
#endif
    return (0);
}

inline uint8_t* an_pak_get_transport_hdr (an_pak_t *pak)
{
#ifdef PRINT_STUBS_PRINTF    
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
#endif
    return (0);
}

inline an_if_t an_pak_get_input_if (an_pak_t *pak)
{
    an_if_t if_index;
    olibc_retval_t retval;

    retval = olibc_pak_get_in_if_index(pak, &if_index);
    if (retval == OLIBC_RETVAL_SUCCESS) {
        return if_index;
    }
    return (0);
}

inline an_if_t an_pak_get_output_if (an_pak_t *pak)
{
    an_if_t if_index;
    olibc_retval_t retval;

    retval = olibc_pak_get_out_if_index(pak, &if_index);
    if (retval == OLIBC_RETVAL_SUCCESS) {
        return if_index;
    }
    return (0);
}

inline const uint8_t *an_pak_get_input_if_name (an_pak_t *pak)
{
    an_if_t if_index;

    if_index = an_pak_get_input_if(pak);

    if (if_index) {
        return (an_if_get_name(if_index));
    }
    return (NULL);
}

inline const uint8_t * an_pak_get_output_if_name (an_pak_t *pak)
{
    an_if_t if_index;

    if_index = an_pak_get_output_if(pak);

    if (if_index) {
        return (an_if_get_name(if_index));
    }
    return (NULL);
}

inline an_iptable_t an_pak_get_iptable (an_pak_t *pak)
{
    return (0);
}

inline void an_pak_set_output_if (an_pak_t *pak, an_if_t output_if)
{
    olibc_pak_set_out_if_index(pak, output_if);
    return;
}

inline void an_pak_set_input_if (an_pak_t *pak, an_if_t input_if)
{
    olibc_pak_set_in_if_index(pak, input_if);
    return;
}

inline void an_pak_set_iptable (an_pak_t *pak, an_iptable_t iptable)
{
    return;
}

inline void an_pak_set_datagram_size (an_pak_t *pak, uint16_t paklen)
{
    printf("\nTodo set datagram size");
//    pak->datagramsize = paklen;
    return;
}

inline void an_pak_set_linktype (an_pak_t *pak, uint8_t linktype)
{
    return;
}

inline uint8_t 
an_pak_get_linktype (an_pak_t *pak)
{
    return (0);
}

inline void an_pak_free (an_pak_t *pak)
{
    if (pak) {
        olibc_pak_destroy(&pak);
    }
    return;
}

an_pak_t *
an_plat_pak_alloc (uint16_t paklen, an_if_t ifhndl, uint16_t len)
{
    olibc_retval_t retval;
    olibc_pak_hdl pak_hdl;
    olibc_pak_info_t pak_info;
    memset(&pak_info, 0, sizeof(olibc_pak_hdl));

    pak_info.addr_family = AF_INET6;
    pak_info.pak_length = paklen;
    retval = olibc_pak_create(&pak_hdl, &pak_info);
    if (retval != OLIBC_RETVAL_SUCCESS) {
        DEBUG_AN_LOG(AN_LOG_ND_EVENT, AN_DEBUG_MODERATE, NULL, 
                "\nPak creation failed");
        return (NULL);
    }

    olibc_pak_set_out_if_index(pak_hdl, ifhndl);

    return pak_hdl;
}

boolean an_pak_grow (an_pak_t **pak_in, uint16_t extra_len)
{
#ifdef PRINT_STUBS_PRINTF    
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
#endif
    return (TRUE);
}

an_pak_t *an_pak_duplicate (an_pak_t *pak)
{
#ifdef PRINT_STUBS_PRINTF    
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
#endif
    return (NULL);
}

inline boolean an_linktype_is_an (uint8_t linktype)
{
    return (FALSE);
}

inline size_t
an_pak_subblock_getsize (an_pak_subblock_index_t idx)
{
#ifdef PRINT_STUBS_PRINTF    
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
#endif
    return (0);
}

void an_pak_subblock_setsize(an_pak_subblock_index_t idx, size_t size)
{
#ifdef PRINT_STUBS_PRINTF    
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
#endif
    return;
}


void
an_cp_msg_block_to_pak (an_pak_t * pak,uint8_t *msg_block,uint8_t
                        *temp_msg_block,uint16_t msg_len)
{
    an_memcpy_s(msg_block, msg_len, temp_msg_block, msg_len);
}

