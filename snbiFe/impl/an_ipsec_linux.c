/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */


#include <an_types.h>
#include <an_ipsec.h>
#include <an_ike.h>
#include <an_logger.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "../al/an_key.h"
#include "../al/an_if.h"
#include "../al/an_addr.h"

uint8_t an_ipsec_profile_name[AN_IPSEC_PROFILE_NAME_BUF_SIZE] = {};
uint32_t an_ipsec_profile_id = AN_IPSEC_PROFILE_NUM_START;
#define ipsec_file "./snbi_ipsec.conf"
#define ipsec_debug_file "./snbi_ipsec.debug"

void
an_ipsec_define_profile_name (void)
{
        return;
}

void
create_debug_file (void)
{
    FILE* fd = NULL, *fd_debug = NULL;
    char cwd[256];

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return;
    }

    fd = fopen(ipsec_debug_file, "w+");
    if (fd == NULL) {
        return;
    }

    fprintf(fd, "%s", "charon {\n");
    fprintf(fd, "%s", "    filelog {\n");
    fprintf(fd, "%s", "        /var/log/charon.log {\n");
    fprintf(fd, "%s", "            # add a timestamp prefix\n");
    fprintf(fd, "%s", "            time_format = %b %e %T\n");
    fprintf(fd, "%s", "            append = no\n");
    fprintf(fd, "%s", "            default = -1\n");
// Uncomment below to enable debugging.
//    fprintf(fd, "%s", "            default = 4\n");
    fprintf(fd, "%s", "            ike = 4\n");
    fprintf(fd, "%s", "            flush_line = yes\n");
    fprintf(fd, "%s", "        }\n");
    fprintf(fd, "%s", "        stderr {\n");
    fprintf(fd, "%s", "            ike = 5\n");
    fprintf(fd, "%s", "            knl = 5\n");
    fprintf(fd, "%s", "           ike_name = yes\n");
    fprintf(fd, "%s", "        }\n");
    fprintf(fd, "%s", "        }\n");
    fprintf(fd, "%s", "}\n");

    fclose(fd);

    fd_debug = fopen("/etc/strongswan.conf", "a+");

    if (fd_debug == NULL) {
        return;
    }

    fprintf(fd_debug, "%s %s%s%s%s","include", cwd, "/",ipsec_debug_file,"\n");
    fclose(fd_debug);
}


void
an_ipsec_linux_start (void)
{
    create_debug_file();
    system("ipsec start > /dev/null 2>&1 &");
}

void
an_ipsec_profile_init (void)
{
    char cwd[256];

    FILE *fd, *fd_ipsec, *fd_secret = NULL;

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return;
    }
    fd = fopen(ipsec_file, "w+");

    if (fd == NULL) {
        return;
    }

    fprintf(fd, "%s","ca strongswan\n");
    fprintf(fd, "%s%s%s%s%s","        cacert=",cwd,"/",CA_CERT_LOCATION,"\n");
    fprintf(fd, "%s","        auto=start\n");
    fprintf(fd, "%s","conn snbi_default\n");
    fprintf(fd, "%s","        ikelifetime=1440m\n");
    fprintf(fd, "%s","        keylife=60m\n");
    fprintf(fd, "%s","        rekeymargin=3m\n");
    fprintf(fd, "%s","        keyingtries=1\n");
    fprintf(fd, "%s","        keyexchange=ikev2\n");
    fprintf(fd, "%s","        authby=rsa\n");
    fprintf(fd, "%s%s%s%s%s","        leftcert=",cwd,"/",DEVICE_CERT_LOCATION,"\n");
    fprintf(fd, "%s","        ike=aes128-aes192-aes256-sha256-sha384-sha512-sha-md5-prfsha512-prfsha384-prfsha256-prfmd5-modp1024-modp1536!\n");
    fprintf(fd, "%s","        esp=aes,sha!\n");
    fprintf(fd, "%s","        keyexchange=ikev2\n");
    fprintf(fd, "%s","        type=transport\n");

    fclose(fd);

    fd_ipsec = fopen("/etc/ipsec.conf", "a+");

    if (fd_ipsec == NULL) {
        return;
    }

    fprintf(fd_ipsec, "%s %s%s%s%s","include", cwd, "/", ipsec_file, "\n");
    fclose(fd_ipsec);

    fd_secret = fopen("/etc/ipsec.secrets", "a+");

    if (fd_secret == NULL) {
        return;
    }

    fprintf(fd_secret, "%s %s%s%s%s"," : RSA", cwd, "/",PRIVATE_KEY_LOCATION,"\n");
    fclose(fd_secret);

    system("ipsec rereadall > /dev/null 2>&1 &");
    system("ipsec update > /dev/null 2>&1 &");
    return;
}

void
an_ipsec_profile_uninit (void)
{
    int status;

    status = remove(ipsec_file);

    if (status == 0 ) {
       DEBUG_AN_LOG(AN_LOG_BS_EVENT, AN_DEBUG_MODERATE, NULL,
                         "\n%s:%s file deleted successfully.\n", an_bs_event,
                         ipsec_file);
    }
}

boolean
an_ipsec_apply_on_tunnel (an_if_t tunn_ifhndl, an_addr_t src_ip,
                          an_addr_t dst_ip, an_if_t local_ifhndl)
{
    FILE* fd = NULL;
    ssize_t nbytes;
    size_t bufsize = 0;
    char *buffer = NULL;
    int position_in_file = 0;

    fd = fopen(ipsec_file, "r+");
    if (fd == NULL) {
        return (FALSE);
    }

    while ((nbytes = getline(&buffer, &bufsize, fd))!= -1) {
        if (strstr(buffer, an_if_get_name(tunn_ifhndl)) != NULL) {
           fseek(fd,position_in_file,SEEK_SET);
           fprintf(fd, "%s%s%s","conn ", an_if_get_name(tunn_ifhndl),"\n");
           fprintf(fd, "%s%s%s%s%s","        left=", an_addr_get_string(&src_ip)                                ,"%", an_if_get_name(local_ifhndl),"\n");
           fprintf(fd, "%s%s%s","        leftid=\"CN=*, OU=",
                      an_get_domain_id(),", serialNumber=*\"\n");
           fprintf(fd, "%s%s%s%s%s","        right=",an_addr_get_string(&dst_ip)                        ,"%", an_if_get_name(local_ifhndl),"\n");
           fprintf(fd, "%s%s%s","        rightid=\"N=*, CN=*, OU=",
                      an_get_domain_id(),", serialNumber=*\"\n");
           fprintf(fd, "%s","        also=snbi_default\n");
           fprintf(fd, "%s","        auto=start\n");

           fclose(fd);
           system("ipsec update > /dev/null 2>&1 &");
           if (buffer) {
               free(buffer);
           }
           return (TRUE);
        }
        position_in_file = ftell(fd);
    }
    position_in_file = ftell(fd);
    fclose(fd);

    if (buffer) {
        free(buffer);
    }

    fd = fopen(ipsec_file, "a+");
    if (fd == NULL) {
        return (FALSE);
    }

    fprintf(fd, "%s%s%s","conn ", an_if_get_name(tunn_ifhndl),"\n");
    fprintf(fd, "%s%s%s%s%s","        left=", an_addr_get_string(&src_ip),
                           "%", an_if_get_name(local_ifhndl),"\n");
    fprintf(fd, "%s%s%s","        leftid=\"CN=*,OU=", an_get_domain_id(),
                        ", serialNumber=*\"\n");
    fprintf(fd, "%s%s%s%s%s","        right=", an_addr_get_string(&dst_ip), "%",
                            an_if_get_name(local_ifhndl),"\n");
    fprintf(fd, "%s%s%s","        rightid=\"N=*, CN=*, OU=",
                      an_get_domain_id(),", serialNumber=*\"\n");
    fprintf(fd, "%s","        also=snbi_default\n");
    fprintf(fd, "%s","        auto=start\n");
    fflush(fd);
    fclose(fd);
    system("ipsec update > /dev/null 2>&1 &");
    return (TRUE);
}

void
an_ipsec_remove_on_tunnel (an_if_t tunn_ifhndl)
{
    FILE* fd = NULL;
    ssize_t nbytes;
    size_t bufsize = 0;
    char *buffer = NULL, cmd1[100];
    int position_in_file = 0;
    boolean intfound = FALSE;

    if (!tunn_ifhndl) {
        return;
    }

    an_sprintf(cmd1, "%s %s %s","ipsec down ",
            an_if_get_name(tunn_ifhndl), "> /dev/null 2>&1 &");
    system(cmd1);

    fd = fopen(ipsec_file, "r+");
    if (fd == NULL) {
        return;
    }

    while ((nbytes = getline(&buffer, &bufsize, fd))!= -1) {
        if (strstr(buffer, an_if_get_name(tunn_ifhndl)) != NULL) {
            intfound = TRUE;
        }

        if (intfound && strstr(buffer,"auto")) {
           fseek(fd, position_in_file,SEEK_SET);
           fprintf(fd, "%s","        auto=ignore\n");
           break;
        }
        position_in_file = ftell(fd);
    }
    fclose(fd);
    if (buffer) {
        free(buffer);
    }
    system("ipsec update > /dev/null 2>&1 &");
}
