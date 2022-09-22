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
char *name = "get";
char *scenario;
size_t msgs_per_second;

volatile struct timeval start;
volatile struct timeval stop;

void reply_handler(z_owned_reply_t *oreply, void *ctx)
{
    if (z_reply_is_ok(oreply)) {
        struct timeval stop;
        gettimeofday(&stop, 0);
        double lat = stop.tv_usec - start.tv_usec;
        if (lat <= 0)
            lat += (stop.tv_sec - start.tv_sec) * 1000000;

        printf("%s,%s,%s,%s,%lu,%lu,%f\n", layer, name, test, scenario, msgs_per_second, start.tv_sec, lat);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3 && argc != 5) {
        printf("USAGE:\n\tzn_querier_lat <scenario> <msgs_per_second> [<zenoh-locator> <zenoh-mode>]\n\n");
        exit(-1);
    }

    scenario = argv[1];
    msgs_per_second = atoi(argv[2]);

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

    while (1) {
        gettimeofday(&start, 0);

        z_get_options_t opts = z_get_options_default();
        opts.target = Z_QUERY_TARGET_ALL;
        z_owned_closure_reply_t callback = z_closure(reply_handler);
        z_get(z_loan(s), z_keyexpr("test/lat"), "", z_move(callback), &opts);

        usleep(1000000 / msgs_per_second);
    }

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));

    return 0;
}
