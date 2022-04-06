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
#include <sys/time.h>

#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>

#define N 100000

char *layer = "opc-ua";
char *test = "throughput";
char *name = "publisher";
char *scenario = "copc-sopc-copc";
int msg_size = 0;

void print_stats(volatile struct timeval start, volatile struct timeval stop)
{
    double t0 = start.tv_sec + ((double)start.tv_usec / 1000000.0);
    double t1 = stop.tv_sec + ((double)stop.tv_usec / 1000000.0);
    double msgs_per_sec = N / (t1 - t0);
    printf("%s,%s,%s,%s,%d,%f\n", layer, name, test, scenario, msg_size, msgs_per_sec);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("USAGE:\n\topcua_pub_thr <payload_size>\n\n");
        exit(-1);
    }

    msg_size = atoi(argv[1]);

    char buf[msg_size + 1];
    memset(&buf, 0x61, msg_size);
    buf[msg_size] = '\0';

    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode status = UA_Client_connect(client, "opc.tcp://localhost:4840");
    if(status != UA_STATUSCODE_GOOD)
    {
        UA_Client_delete(client);
        return status;
    }

    UA_String val = UA_STRING(buf);
    UA_Variant value;
    UA_Variant_init(&value);
    UA_Variant_setScalarCopy(&value, &val, &UA_TYPES[UA_TYPES_STRING]);
    unsigned long long int count = 0;
    struct timeval start;
    struct timeval stop;
    while (1)
    {
        status = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "/test/thr"), &value);
        if(status == UA_STATUSCODE_GOOD)
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
    }

    UA_Variant_clear(&value);
    UA_Client_delete(client);
    return status;
}
