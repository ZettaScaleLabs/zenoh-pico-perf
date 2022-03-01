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

void data_handler(const zn_sample_t *sample, const void *arg)
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
    if (argc != 3 || argc != 5)
    {
        printf("USAGE:\n\tzn_sub_thr <scenario> <payload_size> [<zenoh-locator> <zenoh-mode>]\n\n");
        exit(-1);
    }

    scenario = argv[1];
    msg_size = atoi(argv[2]);

    zn_properties_t *config = zn_config_default();
    if (argc == 5)
    {
        zn_properties_insert(config, ZN_CONFIG_PEER_KEY, z_string_make(argv[3]));
        zn_properties_insert(config, ZN_CONFIG_PEER_KEY, z_string_make(argv[4]));
    }

    zn_session_t *s = zn_open(config);
    if (s == 0)
        exit(-1);

    // Start the read session session lease loops
    znp_start_read_task(s);
    znp_start_lease_task(s);

    zn_reskey_t rid = zn_rid(zn_declare_resource(s, zn_rname("/test/thr")));
    zn_subscriber_t *sub = zn_declare_subscriber(s, rid, zn_subinfo_default(), data_handler, NULL);
    if (sub == 0)
        exit(-1);

    while (1);

    zn_undeclare_subscriber(sub);

    znp_start_read_task(s);
    znp_start_lease_task(s);
    zn_close(s);

    exit(EXIT_SUCCESS);
}