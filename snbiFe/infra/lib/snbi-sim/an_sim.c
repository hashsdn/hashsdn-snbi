/*
 * Copyright (c) 2014  Cisco Systems, All rights reserved.
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <cparser.h>
#include <cparser_tree.h>

int
main (int argc, char *argv[])
{
    cparser_t parser;
    char *config_file = NULL;
    int ch, debug = 0;
    int interactive = 0;

    memset(&parser, 0, sizeof(parser));

    while (-1 != (ch = getopt(argc, argv, "pic:d"))) {
        switch (ch) {
            case 'p':
                printf("pid = %d\n", getpid());
                break;
            case 'i':
                interactive = 1;
                break;
            case 'c':
                config_file = optarg;
                break;
            case 'd':
                debug = 1;
                break;
        }
    }

    parser.cfg.root = &cparser_root;
    parser.cfg.ch_complete = '\t';
    /*
     * Instead of making sure the terminal setting of the target and
     * the host are the same. ch_erase and ch_del both are treated
     * as backspace.
     */
    parser.cfg.ch_erase = '\b';
    parser.cfg.ch_del = 127;
    parser.cfg.ch_help = '?';
    parser.cfg.flags = (debug ? CPARSER_FLAGS_DEBUG : 0);
    strcpy(parser.cfg.prompt, "snbi-sim > ");
    parser.cfg.fd = STDOUT_FILENO;
    cparser_io_config(&parser);

    if (CPARSER_OK != cparser_init(&parser.cfg, &parser)) {
        printf("Fail to initialize parser.\n");
        return -1;
    }
    if (interactive) {
        if (config_file) {
            (void)cparser_load_cmd(&parser, config_file);
        }
        cparser_run(&parser);
    }
    return 0;
}