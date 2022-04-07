/*
 * Copyright (c) 2021, 2022 ZettaScale Technology SARL
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
 * which is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *
 * Contributors:
 *   ZettaScale zenoh team, <zenoh@zettascale.tech>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zenoh-pico.h"

char *layer = "zenoh-pico";
char *test = "latency";
char *name = "eval";
char *scenario = NULL;
int msg_size = 0;

char *data = NULL;

void query_handler(zn_query_t *query, const void *arg)
{
    zn_send_reply(query, query->rname, (const unsigned char *)data, msg_size);
}

int main(int argc, char **argv)
{
    if (argc != 3 && argc != 5)
    {
        printf("USAGE:\n\tzn_eval_lat <scenario> <payload_size> [<zenoh-locator> <zenoh-mode>]\n\n");
        exit(-1);
    }

    scenario = argv[1];
    msg_size = atoi(argv[2]);

    zn_properties_t *config = zn_config_default();
    if (argc == 5)
    {
        zn_properties_insert(config, ZN_CONFIG_PEER_KEY, z_string_make(argv[3]));
        zn_properties_insert(config, ZN_CONFIG_MODE_KEY, z_string_make(argv[4]));
    }

    zn_session_t *s = zn_open(config);
    if (s == 0)
        exit(-1);

    // Start the read session session lease loops
    znp_start_read_task(s);
    znp_start_lease_task(s);

    data = (char *)malloc(msg_size);
    memset(data, 1, msg_size);

    zn_queryable_t *qable = zn_declare_queryable(s, zn_rname("/test/lat"), ZN_QUERYABLE_EVAL, query_handler, NULL);
    if (qable == 0)
        exit(-1);

    char c = 0;
    while (c != 'q')
    {
        c = fgetc(stdin);
    }

    zn_undeclare_queryable(qable);

    znp_stop_read_task(s);
    znp_stop_lease_task(s);
    zn_close(s);

    return 0;
}
