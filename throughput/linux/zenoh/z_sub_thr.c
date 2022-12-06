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

#define N 100000

char *layer = "zenoh-pico";
char *test = "throughput";
char *name = "subscriber";
char *scenario = NULL;
int payload_size = 8;
char *locator = NULL;
char *mode = "client";

volatile unsigned long long int counter = 0;


void data_handler(const z_sample_t *sample, void *arg)
{
    payload_size = sample->payload.len;
    __atomic_fetch_add(&counter, 1, __ATOMIC_RELAXED);
}

int main(int argc, char **argv)
{
    // Parse arguments
    int c;
    while((c = getopt(argc, argv, "s:e:m:h")) != -1 ){
        switch (c) {
            case 's':
                scenario = optarg;
                break;
            case 'e':
                locator = optarg;
                break;
            case 'm':
                mode = optarg;
                break;
            case 'h':
                printf("USAGE:\n\tz_sub_thr -s <scenario> -e <zenoh-locator> -m <zenoh-mode> -h <help>\n\n");
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
    z_owned_subscriber_t sub = z_declare_subscriber(z_loan(s), z_keyexpr("test/thr"), z_move(callback), NULL);
    if (!z_check(sub)) {
        printf("Failed to declare subscriber.\n");
        return -1;
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);
    while (1) {
        sleep(1);
        u_int64_t received;
        __atomic_load(&counter, &received, __ATOMIC_RELAXED);
        if (received > 0) {
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            float elapsed = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
            float msgs_per_sec = (float)received * 1000000.0 / elapsed;

            if (scenario != NULL) {
                printf("%s,%s,%s,%s,%d,%f\n", layer, name, test, scenario, payload_size, msgs_per_sec);
            } else {
                printf("%d,%f\n", payload_size, msgs_per_sec);
            }
            fflush(stdout);

            u_int64_t zero = 0;
            __atomic_exchange(&counter, &zero, &received, __ATOMIC_RELAXED);
            clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        }
    }

    z_undeclare_subscriber(z_move(sub));

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));

    return 0;
}
