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
char *name = "publisher";
char *scenario = NULL;
char *mode = "client";
char *locator = NULL;
size_t payload_size = 64;
double interval = 0;


void data_handler(const z_sample_t *sample, void *arg)
{
    size_t *tv_sec = (size_t *) &sample->payload.start[0];
    size_t *tv_nsec = (size_t *) &sample->payload.start[sizeof(size_t)];
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    float elapsed = (end.tv_sec - *tv_sec) * 1000000 + (end.tv_nsec - *tv_nsec) / 1000;
    float latency = elapsed / 2.0;

    if (scenario != NULL) {
        printf("%s,%s,%s,%s,%.10f,%f\n", layer, name, test, scenario, interval, latency);
    } else {
        printf("%.10f,%f\n", interval, latency);
    }
    fflush(stdout);
}

int main(int argc, char **argv)
{

    // Parse arguments
    int c;
    while((c = getopt(argc, argv, "s:e:m:p:i:h")) != -1 ){
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
            case 'p':
                payload_size = atoi(optarg);
                break;
            case 'i':
                interval = atof(optarg);
                break;
            case 'h':
                printf("USAGE:\n\tz_sub_thr -s <scenario> -e <zenoh-locator> -m <zenoh-mode> -p <payload-size> -i <interval> -h <help>\n\n");
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

    z_owned_publisher_t pub = z_declare_publisher(z_loan(s), z_keyexpr("ping"), NULL);
    if (!z_check(pub)) {
        printf("Failed to declare publisher.\n");
        return -1;
    }

    z_owned_closure_sample_t callback = z_closure(data_handler);
    z_owned_subscriber_t sub = z_declare_subscriber(z_loan(s), z_keyexpr("pong"), z_move(callback), NULL);
    if (!z_check(sub)) {
        printf("Failed to declare subscriber.\n");
        return -1;
    }

    char *data = (char *)malloc(payload_size);
    while (1) {
        usleep((useconds_t)(interval * 1000000));
        memset(data, 0, payload_size);

        struct timespec start;
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        memcpy(&data[0], &start.tv_sec, sizeof(size_t));
        memcpy(&data[sizeof(size_t)], &start.tv_nsec, sizeof(size_t));

        z_publisher_put(z_loan(pub), (const uint8_t *)data, payload_size, NULL);
    }

    z_undeclare_publisher(z_move(pub));
    z_undeclare_subscriber(z_move(sub));

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));

    return 0;
}
