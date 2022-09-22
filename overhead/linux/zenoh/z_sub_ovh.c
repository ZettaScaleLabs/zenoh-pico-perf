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
char *test = "overhead";
char *name = "subscriber";
char *scenario = "ppub-zenohd-psub";

bool received = false;

void data_handler(const z_sample_t *sample, void *arg)
{
    received = true;
}

int main(int argc, char **argv)
{
    if (argc < 1) {
        printf("USAGE:\n\tz_sub_ovh [<zenoh-locator>]\n\n");
        return -1;
    }

    // Initialize Zenoh Session and other parameters
    z_owned_config_t config = z_config_default();
    if (argc == 3) {
        zp_config_insert(z_loan(config), Z_CONFIG_PEER_KEY, z_string_make(argv[1]));
    }

    // Open Zenoh session
    z_owned_session_t s = z_open(z_move(config));
    if (!z_check(s)) {
        return -1;
    }

    // Start the receive and the session lease loop for zenoh-pico
    zp_start_read_task(z_loan(s), NULL);
    zp_start_lease_task(z_loan(s), NULL);

    z_owned_closure_sample_t callback = z_closure(data_handler);
    z_owned_subscriber_t sub = z_declare_subscriber(z_loan(s), z_keyexpr("test/ovh"), z_move(callback), NULL);
    if (!z_check(sub)) {
        return -1;
    }

    while (received);

    z_undeclare_subscriber(z_move(sub));

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));

    return 0;
}
