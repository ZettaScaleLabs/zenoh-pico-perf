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
        printf("USAGE:\n\ttcp_client_lat <msgs_per_second>\n\n");
        exit(-1);
    }

    msgs_per_second = atoi(argv[1]);

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        return -1;

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = inet_addr(IP);
    saddr.sin_port = htons(PORT);
   
    if (connect(sock, (struct sockaddr *) &saddr, sizeof(saddr)) != 0)
        return -1;

    char *buf = (char *)malloc(msg_size);
    while (1)
    {
        struct timeval start;
        gettimeofday(&start, 0);

        memset(buf, 0, msg_size);
        memcpy(&buf[0], &start.tv_sec, sizeof(size_t));
        memcpy(&buf[sizeof(size_t)], &start.tv_usec, sizeof(size_t));

        int n = write(sock, buf, msg_size);
        if (n < 0) 
            break;

        usleep(1000000 / msgs_per_second);
    }
   
    close(sock);

    exit(EXIT_SUCCESS);
}