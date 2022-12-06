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

char *mode = "client";
char *locator = NULL;
z_owned_publisher_t pub;

void data_handler(const z_sample_t *sample, void *arg)
{
    z_publisher_put(
        z_loan(pub),
        (const uint8_t *)sample->payload.start,
        sample->payload.len,
        NULL
    );
}

int main(int argc, char **argv)
{
    // Parse arguments
    int c;
    while((c = getopt(argc, argv, "e:m:h")) != -1 ){
        switch (c) {
            case 'e':
                locator = optarg;
                break;
            case 'm':
                mode = optarg;
                break;
            case 'h':
                printf("USAGE:\n\tz_sub_thr -e <zenoh-locator> -m <zenoh-mode> -h <help>\n\n");
                exit(-1);
            default:
                break;
        }
    }

    // Initialize Zenoh Session and other parameters
    z_owned_config_t config = z_config_default();
    if (locator == NULL) {
        printf("No locator provied!\n");
        exit(-1);
    }
    zp_config_insert(z_loan(config), Z_CONFIG_PEER_KEY, z_string_make(locator));
    zp_config_insert(z_loan(config), Z_CONFIG_MODE_KEY, z_string_make(mode));

    // Open Zenoh session
    z_owned_session_t s = z_open(z_move(config));
    if (!z_check(s)) {
        printf("Failed to open zenoh session!\n");
        return -1;
    }

    // Start the receive and the session lease loop for zenoh-pico
    zp_start_read_task(z_loan(s), NULL);
    zp_start_lease_task(z_loan(s), NULL);

    z_owned_closure_sample_t callback = z_closure(data_handler);
    z_owned_subscriber_t sub = z_declare_subscriber(z_loan(s), z_keyexpr("ping"), z_move(callback), NULL);
    if (!z_check(sub)) {
        printf("Failed to declare subscriber.\n");
        return -1;
    }

    pub = z_declare_publisher(z_loan(s), z_keyexpr("pong"), NULL);
    if (!z_check(pub)) {
        printf("Failed to declare publisher.\n");
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
