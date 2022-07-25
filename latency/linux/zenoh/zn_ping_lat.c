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

char *layer = "zenoh-pico";
char *test = "latency";
char *name = "publisher";
char *scenario;
size_t msg_size = sizeof(size_t) + sizeof(size_t);
size_t msgs_per_second;

void data_handler(const zn_sample_t *sample, const void *arg)
{
    size_t *sec = (size_t *) &sample->value.val[0];
    size_t *usec = (size_t *) &sample->value.val[sizeof(size_t)];

    struct timeval stop;
    gettimeofday(&stop, 0);

    double lat = stop.tv_usec - *usec;
    if (lat <= 0)
        lat += (stop.tv_sec - *sec) * 1000000;

    printf("%s,%s,%s,%s,%lu,%lu,%f\n", layer, name, test, scenario, msgs_per_second, *sec, lat / 2);
}

int main(int argc, char **argv)
{
    if (argc != 3 && argc != 5)
    {
        printf("USAGE:\n\tzn_sub_lat <scenario> <msgs_per_second> [<zenoh-locator> <zenoh-mode>]\n\n");
        exit(-1);
    }

    scenario = argv[1];
    msgs_per_second = atoi(argv[2]);

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

    zn_reskey_t reskey = zn_rid(zn_declare_resource(s, zn_rname("/test/thr")));
    zn_publisher_t *pub = zn_declare_publisher(s, reskey);
    if (pub == 0)
        exit(-1);

    zn_reskey_t rid2 = zn_rid(zn_declare_resource(s, zn_rname("/test/ack")));
    zn_subscriber_t *sub = zn_declare_subscriber(s, rid2, zn_subinfo_default(), data_handler, NULL);
    if (sub == 0)
        exit(-1);

    char *data = (char *)malloc(msg_size);
    while (1)
    {
        struct timeval start;
        gettimeofday(&start, 0);

        memset(data, 0, msg_size);
        memcpy(&data[0], &start.tv_sec, sizeof(size_t));
        memcpy(&data[sizeof(size_t)], &start.tv_usec, sizeof(size_t));

        zn_write_ext(s, reskey, (const uint8_t *)data, msg_size, Z_ENCODING_DEFAULT, Z_DATA_KIND_DEFAULT, zn_congestion_control_t_BLOCK);

        usleep(1000000 / msgs_per_second);
    }

    znp_stop_read_task(s);
    znp_stop_lease_task(s);
    zn_close(s);

    exit(EXIT_SUCCESS);
}
