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
int msg_size = 0;

volatile unsigned long long int count = 0;
volatile struct timeval start;
volatile struct timeval stop;

void print_stats(volatile struct timeval start, volatile struct timeval stop)
{
    double t0 = start.tv_sec + ((double)start.tv_usec / 1000000.0);
    double t1 = stop.tv_sec + ((double)stop.tv_usec / 1000000.0);
    double msgs_per_sec = N / (t1 - t0);
    printf("%s,%s,%s,%s,%d,%f\n", layer, name, test, scenario, msg_size, msgs_per_sec);
}

void data_handler(const z_sample_t *sample, void *arg)
{
    if (count == 0)
    {
        gettimeofday(&start, 0);
        count++;
    }
    else if (count < N)
    {
        count++;
    }
    else
    {
        gettimeofday(&stop, 0);
        print_stats(start, stop);
        count = 0;
    }
}

int main(int argc, char **argv)
{
    if (argc != 3 && argc != 5) {
        printf("USAGE:\n\tz_sub_thr <scenario> <payload_size> [<zenoh-locator> <zenoh-mode>]\n\n");
        exit(-1);
    }

    scenario = argv[1];
    msg_size = atoi(argv[2]);

    // Initialize Zenoh Session and other parameters
    z_owned_config_t config = zp_config_default();
    if (argc == 5) {
        zp_config_insert(z_loan(config), Z_CONFIG_MODE_KEY, z_string_make(argv[3]));
        zp_config_insert(z_loan(config), Z_CONFIG_PEER_KEY, z_string_make(argv[4]));
    }

    // Open Zenoh session
    z_owned_session_t s = z_open(z_move(config));
    if (!z_check(s)) {
        return -1;
    }

    // Start the receive and the session lease loop for zenoh-pico
    zp_start_read_task(z_loan(s));
    zp_start_lease_task(z_loan(s));

    z_owned_closure_sample_t callback = z_closure(data_handler);
    z_owned_subscriber_t sub = z_declare_subscriber(z_loan(s), z_keyexpr("test/thr"), z_move(callback), NULL);
    if (!z_check(sub)) {
        return -1;
    }

    while (1);

    z_undeclare_subscriber(z_move(sub));

    // Stop the receive and the session lease loop for zenoh-pico
    zp_stop_read_task(z_loan(s));
    zp_stop_lease_task(z_loan(s));

    z_close(z_move(s));

    return 0;
}
