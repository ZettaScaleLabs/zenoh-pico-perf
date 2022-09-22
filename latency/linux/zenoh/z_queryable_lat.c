//
// Copyright (c) 2022 ZettaScale Technology
//
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// http://www.eclipse.org/legal/epl-2.0, or the Apache License, Version 2.0
// which is available at https://www.apache.org/licenses/LICENSE-2.0.
//
// SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
//
// Contributors:
//   ZettaScale Zenoh Team, <zenoh@zettascale.tech>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zenoh-pico.h"

char *layer = "zenoh-pico";
char *test = "latency";
char *name = "queryable";
char *scenario = NULL;
int msg_size = 0;

char *data = NULL;

void query_handler(z_query_t *query, void *ctx)
{
    z_query_reply(query, z_keyexpr("/test/lat"), (const unsigned char *)data, msg_size, NULL);
}

int main(int argc, char **argv)
{
    if (argc != 3 && argc != 5) {
        printf("USAGE:\n\tz_queryable_lat <scenario> <payload_size> [<zenoh-locator> <zenoh-mode>]\n\n");
        exit(-1);
    }

    scenario = argv[1];
    msg_size = atoi(argv[2]);

    // Initialize Zenoh Session and other parameters
    z_owned_config_t config = z_config_default();
    if (argc == 5) {
        zp_config_insert(z_loan(config), Z_CONFIG_PEER_KEY, z_string_make(argv[3]));
        zp_config_insert(z_loan(config), Z_CONFIG_MODE_KEY, z_string_make(argv[4]));
    }

    // Open Zenoh session
    z_owned_session_t s = z_open(z_move(config));
    if (!z_check(s)) {
        return -1;
    }

    // Start the receive and the session lease loop for zenoh-pico
    zp_start_read_task(z_loan(s), NULL);
    zp_start_lease_task(z_loan(s), NULL);

    data = (char *)malloc(msg_size);
    memset(data, 1, msg_size);

    // Declare Zenoh queryable
    z_owned_closure_query_t callback = z_closure(query_handler);
    z_owned_queryable_t qable = z_declare_queryable(z_loan(s), z_keyexpr("test/lat"), z_move(callback), NULL);
    if (!z_check(qable)) {
        return -1;
    }

    char c = 0;
    while (c != 'q') {
        c = fgetc(stdin);
    }

    z_undeclare_queryable(z_move(qable));

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));

    return 0;
}
