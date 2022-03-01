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

#define N 100000

char *layer = "tcp-raw-perf";
char *test = "throughput";
char *name = "baseline";
char *scenario = "ctcp-stcp";
int msg_size = 0;

volatile unsigned long long int count = 0;
volatile unsigned long long int bytes = 0;
volatile struct timeval start;
volatile struct timeval stop;

void print_stats(volatile struct timeval start, volatile struct timeval stop)
{
    double t0 = start.tv_sec + ((double)start.tv_usec / 1000000.0);
    double t1 = stop.tv_sec + ((double)stop.tv_usec / 1000000.0);
    double bits_per_sec = (bytes * 8) / (t1 - t0);
    bytes= 0;
    printf("%s,%s,%s,%s,%d,%f\n", layer, name, test, scenario, msg_size, bits_per_sec);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("USAGE:\n\ttcp_server_thr <payload_size>\n\n");
        exit(EXIT_FAILURE);
    }

    msg_size = atoi(argv[1]);

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
        
        char buf[msg_size];
        memset(&buf, 1, msg_size);
    
        int n = 0;
        while (1)
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

            n = read(csock, buf, msg_size);
            if (n < 0)
                break;

            bytes += n;
        }
        close(csock);
    }
    close (ssock);

    exit(EXIT_SUCCESS);
}
