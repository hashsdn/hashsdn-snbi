/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include "../al/an_types.h"
#include "../al/an_logger.h"
#include "../al/an_addr.h"
#include "../al/an_ipv6.h"
#include "../al/an_if.h"
#include "../al/an_cert.h"
#include "../al/an_sudi.h"
#include "../al/an_mem.h"
#include "../al/an_misc.h"
#include "../al/an_key.h"
#include "../al/an_tunnel.h"
#include "../al/an_timer.h"
#include "../al/an_str.h"
#include "an.h"
#include "an_nd.h"
#include "an_nbr_db.h"
#include "an_acp.h"
#include "an_event_mgr.h"
#include "an_anra.h"
#include "an_bs.h"
#include "an_anra_db.h"
//#include "an_topo_disc.h"
#include "an_if_mgr.h"

extern boolean global_ike_cli_executed_by_an;

boolean global_cfg_autonomic_enable = FALSE;

boolean global_dev_id_set = FALSE;
boolean global_dom_id_set = FALSE;
boolean g_an_rpl_float_root = TRUE;

//#define AN_CFG_BOOT_WO_SAVED_CONFIG 0x2142
//extern void nv_setconfig (ushort number);

static const uint8_t *an_msg_name_s[] = {
    "None",
    "ND Hello",
    "ND Bye",

    "ND Cert Request",
    "ND Cert Response",

    "BS Invite",
    "BS Reject",
    "BS Request",
    "BS Response",
    "BS Enroll Quarantine",

    "NBR Connect",
    "NBR Re-connect",
    "NBR Join",
    "NBR Leave",
    "NBR Lost",
    "NBR Modify",
    "ACP Data",
    "IDP Intent",
    "IDP ACK",
    "IDP Intent Version",
    "IDP Intent Request",
    "Service Info",
    "Service Info Ack",

    "CNP Solicit",
    "CNP Advertisement",
    "CNP Proposal",
    "CNP Accept",
    "CNP Reject",
    "CNP Error",
    "CNP Ack",
};

an_info_t an_info = {{0}};

const uint8_t *
an_get_msg_name (an_msg_type_e type)
{
    return (an_msg_name_s[type]);
}

/*
 *  To find if "autonomic" is enabled
 */
boolean
is_device_autonomic (void)
{
    /* TODO */
    return (TRUE);
}

/*
 * To find if the interface is autonomic
 */
boolean
is_int_autonomic (an_if_t an_if)
{
    /* TODO */
    return (TRUE);
}

boolean
an_get_udi (an_udi_t *udi)
{
    if ((an_info.udi.data != NULL) && 
        (an_info.udi.len != 0)) {
        udi->data = an_info.udi.data;
        udi->len = an_info.udi.len;
        return (TRUE);

    } else {
        udi->data = NULL;
        udi->len = 0;
        return (FALSE);
    }

}

boolean
an_get_domain_cert (an_cert_t *domain_cert)
{
    if (an_info.domain_cert.data && 
        an_info.domain_cert.len) {
        domain_cert->data = an_info.domain_cert.data;
        domain_cert->len = an_info.domain_cert.len;
        return (TRUE);

    } else {
        domain_cert->data = NULL;
        domain_cert->len = 0;
        return (FALSE);
    }
}

uint8_t *
an_get_device_id (void)
{
    return (an_info.device_id);
}

uint8_t *
an_get_domain_id (void)
{
    return (an_info.domain_id);
}

an_addr_t
an_get_anra_ip (void)
{
    return (an_info.anra_ip);
}

an_addr_t
an_get_aaa_ip (void)
{
    return (an_info.aaa_ip);
}

an_addr_t
an_get_nms_ip (void)
{
    return (an_info.nms_ip);
}

an_iptable_t
an_get_iptable ()
{
    return (an_info.iptable);
}

an_afi_t
an_get_afi ()
{
    return (an_info.afi);
}

an_addr_t
an_get_device_ip (void)
{
    return (an_info.device_ip);
}

an_if_t
an_get_anra_if (void)
{
    return (an_info.anra_ifhndl);
}

an_cert_t
an_get_anra_cert (void)
{
    return (an_info.anra_cert);
}

uint16_t
an_get_routing_ospf_pid (void)
{
    return (an_info.routing.ospf_pid);
}

uint16_t
an_get_routing_ospf_area (void)
{
    return (an_info.routing.ospf_area);
}

uint32_t
an_get_routing_ospf_rid (void)
{
    return (an_info.routing.ospf_rid);
}

an_routing_cfg_t
an_get_routing_info (void)
{
    return (an_info.routing);
}

an_rpl_info_t
an_get_rpl_routing_info (void)
{
    return (an_info.routing.an_rpl_info);
}

void
an_set_rpl_float_root_enable_flag (bool enable)
{
    bool changed = FALSE;
    if (g_an_rpl_float_root != enable) {
        g_an_rpl_float_root = enable;
        changed = TRUE;
    }
    printf("\nnon-anra device %s to use rpl '%s'",
           changed ? "setting changed" : "already set",
           enable ? "floating-root" : "node");
}

boolean
an_get_rpl_floating_root_enable_flag (void)
{
    return (g_an_rpl_float_root);
}

void
an_reset_rpl_routing_info (void)
{
    an_memset(&an_info.routing.an_rpl_info, 0, 
                    sizeof(an_info.routing.an_rpl_info));    
    return;
}

an_key_t
an_get_public_key (uint8_t *key_label)
{
    an_key_t public_key = {};

    if (an_key_get_public_key(key_label, &public_key)) {
        return (public_key);
    }
    
    if (!an_key_generate_keypair(key_label)) { 
         DEBUG_AN_LOG(AN_LOG_BS_PACKET, AN_DEBUG_MODERATE, NULL,
                     "\n%sAN Domain Key generation failed",
                     an_bs_pak);

        an_syslog(AN_SYSLOG_DOMAIN_KEY_GEN_FAIL, key_label);
        return (public_key);

    } else if (an_key_get_public_key(key_label, &public_key)) {
        return (public_key);
    
    } else {
        return (public_key);
    }
}

void
an_set_udi (an_udi_t udi)
{
    if (an_info.udi.data) {
        an_free_guard(an_info.udi.data);
    }
        an_info.udi.data = NULL;
        an_info.udi.len = 0;

    if (!udi.len || !udi.data) {
        return;
    }

    an_info.udi.len = udi.len;
    an_info.udi.data = (uint8_t *)an_malloc_guard(udi.len, "AN an_info.udi");
    if (an_info.udi.data) {
        an_memcpy_guard(an_info.udi.data, udi.data, udi.len);
    }
    an_free_guard(udi.data);
}

void
an_set_device_id (uint8_t *device_id)
{
    if (an_info.device_id) {
        an_free_guard(an_info.device_id);
        an_info.device_id = NULL;
    }
    
    if(!device_id) {
        return;
    }

    an_info.device_id = (uint8_t*)an_malloc_guard(an_strlen(device_id) + 1, 
                                                  "AN an_info.device_id");
    if (an_info.device_id) {
        an_memcpy_guard(an_info.device_id, device_id, an_strlen(device_id)+1);
        global_dev_id_set = TRUE;
    }
}

void
an_set_domain_id (uint8_t *domain_id)
{
    if (an_info.domain_id) {
        an_free_guard(an_info.domain_id);
        an_info.domain_id = NULL;
    }

    if (!domain_id) {
        return;
    }

    an_info.domain_id = (uint8_t*)an_malloc_guard(an_strlen(domain_id)+1, 
                                                  "AN an_info.domain_id");
    if (an_info.domain_id) {
        an_memcpy_guard(an_info.domain_id, domain_id, an_strlen(domain_id)+1);
        global_dom_id_set = TRUE;
    }
}

void
an_set_domain_cert (an_cert_t domain_cert)
{
    if (an_info.domain_cert.data) {
        an_free_guard(an_info.domain_cert.data);
    }
    an_info.domain_cert.len = 0;
    an_info.domain_cert.data = NULL;

    if (!domain_cert.data || !domain_cert.len) {
        return;
    }

    an_info.domain_cert.len = domain_cert.len;
    an_info.domain_cert.data = (uint8_t *)an_malloc_guard(
                          an_info.domain_cert.len, "AN an_info.domain_cert");
    if (!an_info.domain_cert.data) {
        return;
    }
    an_memcpy_guard(an_info.domain_cert.data, 
                                domain_cert.data, an_info.domain_cert.len);

    an_event_domain_device_cert_learnt();
}

boolean 
an_set_service_info (an_service_type_e srvc_type, an_addr_t *srvc_ip)
{
    an_addr_t *srvc_ip_p = NULL;

    if ((srvc_type < AN_SERVICE_AAA) || srvc_type >= AN_SERVICE_MAX) {
         return (FALSE);
    }

    switch (srvc_type) {
        case AN_SERVICE_AAA:
             srvc_ip_p = &an_info.aaa_ip; 
             break;
        case AN_SERVICE_MAX:
             return (FALSE);
    }

    an_memcpy(srvc_ip_p, srvc_ip, sizeof(an_addr_t)); 

    return (TRUE);
        
}

void
an_set_anra_ip (an_addr_t ip)
{
    an_info.anra_ip = ip;
}

void
an_set_iptable (an_iptable_t iptable)
{
    an_info.iptable = iptable;
}

void
an_set_afi (an_afi_t afi)
{
    an_info.afi = afi;
}

void
an_set_device_ip (an_addr_t ip)
{
    an_info.device_ip = ip;
}

void
an_set_anra_cert (an_cert_t cert)
{
    if (an_info.anra_cert.data) {
        an_free_guard(an_info.anra_cert.data);
    }
    an_info.anra_cert.len = 0;
    an_info.anra_cert.data = NULL;

    if (!cert.data || !cert.len) {
        return;
    }

    an_info.anra_cert.len = cert.len;
    an_info.anra_cert.data = (uint8_t *)an_malloc_guard(
                             an_info.anra_cert.len, "AN an_info.anra_cert");
    if (!an_info.anra_cert.data) {
        return;
    }
    an_memcpy_guard(an_info.anra_cert.data, cert.data, an_info.anra_cert.len);

    an_event_domain_ca_cert_learnt();
}

void
an_reset_anra_cert (void)
{
    if (an_info.anra_cert.data) {
        an_free_guard(an_info.anra_cert.data);
    }
    an_info.anra_cert.len = 0;
    an_info.anra_cert.data = NULL;

    an_cert_reset_domain_ca_cert();
}

void
an_set_anra_if (an_if_t ifhndl)
{
    an_info.anra_ifhndl = ifhndl;
}

void
an_set_routing_ospf_pid (uint16_t pid)
{
    an_info.routing.ospf_pid = pid;
}

void
an_set_routing_ospf_area (uint16_t area)
{
    an_info.routing.ospf_area = area;
}

void
an_set_routing_ospf_rid (uint32_t rid)
{
    an_info.routing.ospf_rid = rid;
}

void
an_reset_global_info (void)
{
    an_cert_t empty_cert = {};

    an_set_device_id(NULL);
    an_set_domain_id(NULL);
    an_set_domain_cert(empty_cert);
    an_set_anra_cert(empty_cert);
    an_set_anra_ip(AN_ADDR_ZERO);
    an_set_iptable(0);
    an_set_afi(0);
    an_set_device_ip(AN_ADDR_ZERO);
    an_set_anra_if(0);
    //an_set_routing_ospf_pid(0);
    // an_set_routing_ospf_area(0);
    // an_set_routing_ospf_rid(0);

//    set_boot_configregister(AN_CFG_BOOT_WO_SAVED_CONFIG);
//    nv_setconfig(AN_CFG_BOOT_WO_SAVED_CONFIG);
}

void
an_autonomic_disable (boolean called_from_proc)
{
    if (!an_is_global_cfg_autonomic_enabled()) {
        return;
    }

    an_event_autonomics_uninit(called_from_proc);

    an_set_global_cfg_autonomic_enable(FALSE);
    an_set_ike_cli_autonomically_created(FALSE);
}

void
an_autonomic_enable (boolean called_from_proc)
{
    if (an_is_global_cfg_autonomic_enabled()) {
        return;
    }

    an_event_autonomics_init();

    an_set_global_cfg_autonomic_enable(TRUE);
}

void
an_reset_an_info (void)
{
    boolean from_proc = FALSE;
    boolean an_is_enabled = an_is_global_cfg_autonomic_enabled();

    an_autonomic_disable(from_proc);

    if (an_is_enabled) {
        an_autonomic_enable(from_proc);
    }
}

boolean
an_is_global_cfg_autonomic_enabled (void)
{
    return (global_cfg_autonomic_enable);
}

void 
an_set_global_cfg_autonomic_enable (boolean flag)
{
    global_cfg_autonomic_enable = flag;
}

void 
an_set_ike_cli_autonomically_created (boolean flag)
{
    global_ike_cli_executed_by_an = flag;
}

boolean
an_hton_1_byte (uint8_t *target, int8_t src)
{
    if (!target) {
        return (FALSE);
    } 

    *target = src;
    return (TRUE);
}

uint8_t *
an_hton_1_byte_and_move (uint8_t *target, int8_t src)
{
    if (!an_hton_1_byte(target, src)) { 
        return (NULL);
    }

    return(target + 1);
}

boolean
an_hton_2_bytes (uint8_t *target, int16_t src)
{
    if (!target) {
        return (FALSE);
    } 

    target[0] = (uint8_t)(src >> 8);
    target[1] = (uint8_t)src;

    return (TRUE);
}

uint8_t *
an_hton_2_bytes_and_move (uint8_t *target, int16_t src)
{
    if (!an_hton_2_bytes(target, src)) {
        return (NULL);
    }

    return (target + 2);
}

boolean
an_hton_4_bytes (uint8_t *target, int32_t src)
{
    if (!target) {
        return (FALSE);
    } 

    target = an_hton_2_bytes_and_move(target, (uint16_t)(src >> 16)); 
    target = an_hton_2_bytes_and_move(target, (uint16_t)src); 
    return (TRUE);
}

uint8_t *
an_hton_4_bytes_and_move (uint8_t *target, int32_t src)
{
    if (!an_hton_4_bytes(target, src)) {
        return (NULL);
    }

    return (target + 4);
}

/* MSB in src[0] and LSB in src[3] */
boolean
an_hton_16_bytes (uint8_t *target, int32_t src[4])
{
    if (!target) {
        return (FALSE);
    }

    target = an_hton_4_bytes_and_move(target, src[0]);
    target = an_hton_4_bytes_and_move(target, src[1]);
    target = an_hton_4_bytes_and_move(target, src[2]);
    target = an_hton_4_bytes_and_move(target, src[3]);

    return (TRUE);
}

uint8_t *
an_hton_16_bytes_and_move (uint8_t *target, int32_t src[4])
{
    if (!an_hton_16_bytes(target, src)) {
        return (NULL);
    }

    return (target + 16);
}

uint8_t
an_ntoh_1_byte (uint8_t *src)
{
    if (!src) {
        return (0);
    }

    return (*src);
}

uint8_t *
an_ntoh_1_byte_and_move (uint8_t *target, uint8_t *src)
{
    if (!src || !target) {
        return (NULL);
    }

    *target = an_ntoh_1_byte(src);

    return (src + 1);
}

uint16_t
an_ntoh_2_bytes (uint8_t *src)
{
    if (!src) {
        return (0);
    }

    return ((src[0] << 8) | src[1]);
}

uint8_t *
an_ntoh_2_bytes_and_move (uint16_t *target, uint8_t *src)
{
    if (!src || !target) {
        return (NULL);
    }

    *target = an_ntoh_2_bytes(src);

    return (src + 2);
}

uint32_t
an_ntoh_4_bytes (uint8_t *src)
{
    if (!src) {
        return (0);
    }

    return ((an_ntoh_2_bytes(src) << 16) |
            (an_ntoh_2_bytes(src + 2)));
}

uint8_t *
an_ntoh_4_bytes_and_move (uint32_t *target, uint8_t *src)
{
    if (!src || !target) {
        return (NULL);
    }

    *target = an_ntoh_4_bytes(src);

    return (src + 4);
}

boolean
an_ntoh_16_bytes (uint32_t *target, uint8_t *src)
{
    if (!src || !target) {
        return (FALSE);
    }

    src = an_ntoh_4_bytes_and_move(target + 0, src);
    src = an_ntoh_4_bytes_and_move(target + 1, src);
    src = an_ntoh_4_bytes_and_move(target + 2, src);
    src = an_ntoh_4_bytes_and_move(target + 3, src);

    return (TRUE);
}

uint8_t *
an_ntoh_16_bytes_and_move (uint32_t *target, uint8_t *src)
{
    if (!an_ntoh_16_bytes(target, src)) {
        return (NULL);
    }

    return (src + 16);
}