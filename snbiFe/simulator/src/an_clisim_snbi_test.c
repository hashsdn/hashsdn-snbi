/*
 * Vijay Anand R <vanandr@cisco.com>
 *
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <cparser.h>
#include <an_types.h>
#include <cparser_tree.h>
#include <an_conf_linux.h>


cparser_result_t
cparser_cmd_test_device_udi_udi (cparser_context_t *context,
                                 char **udi_ptr)
{
    if (!an_config_udi_cmd_handler(*udi_ptr)) {
        return CPARSER_NOT_OK;
    }

    return (CPARSER_OK);
}

cparser_result_t 
cparser_cmd_test_interface_intfname_enable (cparser_context_t *context, 
                                            char **intfname_ptr)
{
    uint32_t if_index = 0;

    if_index = if_nametoindex(*intfname_ptr);

    if (!an_config_intf_enable_cmd_handler(if_index)) {
        return CPARSER_NOT_OK;
    }
    return (CPARSER_OK);
}

cparser_result_t 
cparser_cmd_test_interface_intfname_disable (cparser_context_t *context,
                                             char **intfname_ptr)
{
    uint32_t if_index = 0;

    if_index = if_nametoindex(*intfname_ptr);

    if (!an_config_intf_disable_cmd_handler(if_index)) {
        return CPARSER_NOT_OK;
    }
    return (CPARSER_OK);
}

