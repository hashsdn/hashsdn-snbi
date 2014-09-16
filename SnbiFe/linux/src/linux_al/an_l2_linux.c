/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include "an_types.h"
#include "an_if.h"
#include "an_pak.h"
#include "an_logger.h"
#include "an_str.h"
//#include "an_parse_dummy.h"
#include "an_if_mgr.h"
#include "an_event_mgr.h"
#include "an_mem.h"
#include "an_cd.h"
#include "an_l2_linux.h"


#define AN_L2_HELLO_MSG_LEN 200
#define AN_L2_HDR_LEN       sizeof(an_header)
#define AN_HDR_HOP_LIMIT    255
#define AN_HDR_VER          1
#define AN_TYPE_LEN         2
#define AN_IF_NAME_MAX_LEN  99
#define AN_UDI_NAME_MAX_LEN 75
#define AN_EVC_NAME_MAX_LEN 100

#define AN_TLV_HDR_LEN 4

#define AN_MAX_VLAN       4090
#define AN_MIN_VLAN        300
#define AN_VLAN_START     2000

boolean gAN_platform_is_iol = FALSE;

an_cd_info_t *
an_cd_info_db_search(an_if_info_t *, an_cd_info_t *);
an_cd_info_t *
an_cd_info_vlan_db_search(an_if_info_t *, an_cd_info_t *);
boolean
an_cd_info_vlan_db_entry_exists(an_if_info_t *an_phy_info);

an_cd_info_t *an_cd_info_alloc(void);

boolean
an_cd_info_db_insert(an_if_info_t *, an_cd_info_t *);
boolean
an_cd_info_vlan_db_insert(an_if_info_t *, an_cd_info_t *);

boolean
an_cd_info_vlan_db_remove(an_if_info_t *, an_cd_info_t *);
boolean
an_cd_info_db_remove(an_if_info_t *, an_cd_info_t *);

boolean an_if_is_layer2 (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (FALSE);
}

boolean
an_l2_check_probe_possible (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (FALSE);
}

boolean
an_get_vlan_id (an_if_info_t *phy_an_if_info) 
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

void
an_pak_init_l2_header (an_pak_t *an_pak, 
                       an_proto_channel_discovery_msg_type_e msg_type)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_l2_nd_trigger_hello (an_if_t ifhndl, boolean request,
                        an_dot1q_qinq_vlan_id_t *vlan_sb,
                        an_udi_t *dest_udi)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
        return;
}

void
an_l2_disable_channel (an_if_t ifhndl, an_cd_info_t *an_cd_info)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
        return; 
}

boolean
an_l2_check_channel_configured (an_if_t ifhndl, an_cd_info_t *an_cd_info)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
   return (FALSE);
}

boolean
an_l2_enable_channel (an_if_t ifhndl, an_cd_info_t *an_cd_info)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

boolean
an_copy_seed_to_actual_cd_info (an_cd_info_t *an_seed_cd_info, 
                                an_cd_info_t *an_cd_info)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

void
an_handle_l2_pak (an_pak_t *an_pak) 
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

an_if_t
an_configure_vti_for_l2_port (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return 0;
}

void 
an_disable_switchport_on_if (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

void
an_l2_reuse_startup_config (an_if_t ifhndl)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return;
}

