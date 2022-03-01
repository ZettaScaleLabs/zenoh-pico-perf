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

#include <arpa/inet.h>
#include <netdb.h>

#define IP "127.0.0.1"
#define PORT 5555

char *layer = "tcp-raw-perf";
char *test = "latency";
char *name = "baseline";
char *scenario = "ctcp-stcp";
size_t msg_size = sizeof(size_t) + sizeof(size_t);
size_t msgs_per_second;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("USAGE:\n\ttcp_server_lat <msgs_per_second>\n\n");
        exit(EXIT_FAILURE);
    }

    msgs_per_second = atoi(argv[1]);

    int ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ssock < 0)
        return -1;

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(PORT);

    if (bind(ssock, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) 
        exit(EXIT_FAILURE);

    if (listen(ssock, 1) < 0)
        exit(EXIT_FAILURE);

    struct sockaddr_in caddr;
    socklen_t caddrlen = sizeof(caddr);
    while (1)
    {
        int csock = accept(ssock, (struct sockaddr *) &caddr, &caddrlen);
        if (csock < 0) 
            break;

        char *buf = (char *)malloc(msg_size);
        while (1)
        {
            int n = read(csock, buf, msg_size);
            if (n < 0)
                break;
            
            size_t *sec = (size_t *) &buf[0];
            size_t *usec = (size_t *) &buf[sizeof(size_t)];

            struct timeval stop;
            gettimeofday(&stop, 0);

            double lat = stop.tv_usec - *usec;
            if (lat <= 0)
                lat += (stop.tv_sec - *sec) * 1000000;

            printf("%s,%s,%s,%s,%lu,%lu,%f\n", layer, name, test, scenario, msgs_per_second, *sec, lat);
        }
        close(csock);
    }
    close (ssock);

    exit(EXIT_SUCCESS);
}
