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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "zenoh-pico.h"

char *layer = "zenoh-pico";
char *test = "latency";
char *name = "subscriber";
char *scenario;
size_t msgs_per_second;
size_t msg_size = sizeof(size_t) + sizeof(size_t);
char *data = NULL;

z_owned_publisher_t pub;

void data_handler(const z_sample_t *sample, void *arg)
{
    z_publisher_put(z_loan(pub), (const uint8_t *)data, msg_size, NULL);
}

int main(int argc, char **argv)
{
    if (argc != 3 && argc != 5)
    {
        printf("USAGE:\n\tz_pong_lat <scenario> <msgs_per_second> [<zenoh-locator> <zenoh-mode>]\n\n");
        exit(-1);
    }

    scenario = argv[1];
    msgs_per_second = atoi(argv[2]);

    // Initialize Zenoh Session and other parameters
    z_owned_config_t config = z_config_default();
    if (argc == 5)
    {
        zp_config_insert(z_loan(config), Z_CONFIG_MODE_KEY, z_string_make(argv[3]));
        zp_config_insert(z_loan(config), Z_CONFIG_PEER_KEY, z_string_make(argv[4]));
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

    z_owned_closure_sample_t callback = z_closure(data_handler);
    z_owned_subscriber_t sub = z_declare_subscriber(z_loan(s), z_keyexpr("test/lat"), z_move(callback), NULL);
    if (!z_check(sub)) {
        return -1;
    }

    pub = z_declare_publisher(z_loan(s), z_keyexpr("test/lat"), NULL);
    if (!z_check(pub)) {
        return -1;
    }

    while (1);

    z_undeclare_subscriber(z_move(sub));
    z_undeclare_publisher(z_move(pub));

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));

    return 0;
}
