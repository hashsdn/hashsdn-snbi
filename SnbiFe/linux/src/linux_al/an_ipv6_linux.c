/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include "an_types.h"
#include "an.h"
#include "an_event_mgr.h"
#include "an_addr.h"
#include "an_logger.h"
#include "an_if.h"
#include "an_tunnel.h"
#include "an_if_mgr.h"
#include "an_acp.h"
#include "an_routing.h"
#include "an_ipv6.h"

boolean an_ipv6_unicast_routing_enabled = FALSE;

inline boolean an_ipv6_enable_on_interface (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

inline boolean an_ipv6_disable_on_interface (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

inline boolean an_ipv6_address_set_unnumbered (an_if_t ifhndl, an_if_t unnum_ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
        return (FALSE);
}


void
an_ipv6_configure_addr_on_interface (an_if_t ifhndl, an_addr_t addr, 
                                     uint32_t prefix_len)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_ipv6_configure_cga_on_interface (an_if_t ifhndl, an_v6addr_t v6addr, 
                                    uint32_t prefix_len)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_ipv6_unconfigure_addr_on_interface (an_if_t ifhndl, an_addr_t addr,
                                       uint32_t prefix_len)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

inline an_v6addr_t an_ipv6_get_ll (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (AN_V6ADDR_ZERO);
}

inline boolean an_ipv6_join_mld_group (an_if_t ifhndl, an_v6addr_t *group_addr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

inline boolean an_ipv6_leave_mld_group (an_if_t ifhndl, an_v6addr_t *group_addr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

boolean an_ipv6_preroute_pak (an_pak_t *pak, an_if_t ifhndl, an_addr_t nhop)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

boolean an_ipv6_forward_pak (an_pak_t *pak)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

inline uint8_t an_ipv6_hdr_get_version (an_ipv6_hdr_t *ipv6_hdr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (0);
}

inline an_v6addr_t an_ipv6_hdr_get_src (an_ipv6_hdr_t *ipv6_hdr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (AN_V6ADDR_ZERO);
}

inline an_v6addr_t an_ipv6_hdr_get_dest (an_ipv6_hdr_t *ipv6_hdr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (AN_V6ADDR_ZERO);
}

inline uint8_t an_ipv6_hdr_get_next (an_ipv6_hdr_t *ipv6_hdr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (0);
}

inline uint8_t an_ipv6_hdr_get_hlim (an_ipv6_hdr_t *ipv6_hdr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (0);
}

inline uint16_t an_ipv6_hdr_get_paylen (an_ipv6_hdr_t *ipv6_hdr)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (0);
}

inline void an_ipv6_hdr_set_paylen (an_ipv6_hdr_t *ipv6_hdr, uint16_t plen)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

inline uint16_t
an_ipv6_calculate_cksum (const an_ipv6_hdr_t *ipv6_hdr, 
                         const void *ulp_hdr, uint8_t ulp)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (0);
}

an_addr_t an_ipv6_get_best_source_addr (an_addr_t destination, 
                                        an_iptable_t iptable)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (AN_V6ADDR_ZERO); 
}

boolean an_ipv6_is_our_address (an_addr_t address, an_iptable_t iptable)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (FALSE);
}

inline boolean an_ipv6_hdr_init (uint8_t *ipv6_hdr, uint8_t tos, uint32_t flow_label, 
        uint16_t payload_len, uint8_t protocol, uint8_t hop_lim,
        an_v6addr_t *source, an_v6addr_t *destination)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE); 
}

void *an_rwatch_ctx = NULL;

void
an_rwatch_cb (void *app_ctx)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
        return;
}

boolean
an_rwatch_init (void)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (FALSE);
}

boolean
an_rwatch_uninit (void)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (FALSE);
}

void
an_rwatch_start_track_ipaddr (an_addr_t ipaddr, an_afi_t af, an_iptable_t iptable)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_rwatch_stop_track_ipaddr (void)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_ipv6_routing_start_global_unicast (void)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_ipv6_routing_stop_global_unicast (void)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_ipv6_unicast_routing_enable_disable_register (void)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_ipv6_unicast_routing_enable_disable_unregister (void)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

an_list_t* 
an_ipv6_get_list_of_ipv6_addresses_on_interface (an_if_t ifhndl) {
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return NULL;
}

void 
an_ipv6_clean_all_v6addr_on_interface (an_list_t *list, an_if_t ifhndl) {
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void 
an_ipv6_set_and_clean_v6addr_on_interface_and_nvgen (an_list_t *list, an_if_t ifhndl) {
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}