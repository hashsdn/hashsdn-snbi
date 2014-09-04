/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include "an_udp.h"
#include "an_ipv6.h"
#include "an_pak.h"
#include "an_msg_mgr.h"

boolean 
an_udp_build_header (uint8_t *ip_block, uint8_t  *udp_block, 
                     an_udp_port_t dest_port, an_udp_port_t source_port, 
                     uint16_t msg_len)
{ 
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}

boolean
an_udp_update_checksum (an_pak_t *pak)
{
printf("\n[SRK_DBG] %s():%d - START ....",__FUNCTION__,__LINE__);
    return (TRUE);
}
