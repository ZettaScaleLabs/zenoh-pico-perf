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

char *layer = "opc-ua";
char *test = "latency";
char *name = "publisher";
char *scenario = "copc-sopc-copc";
size_t msg_size = sizeof(size_t) + sizeof(size_t);
size_t msgs_per_second;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("USAGE:\n\topcua_pub_lat <msgs_per_second>\n\n");
        exit(-1);
    }

    msgs_per_second = atoi(argv[1]);

    char *buf = (char *)malloc(msg_size + 1);

    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode status = UA_Client_connect(client, "opc.tcp://192.168.11.1:4840");
    if(status != UA_STATUSCODE_GOOD)
    {
        UA_Client_delete(client);
        return status;
    }

    while (1)
    {
        struct timeval start;
        gettimeofday(&start, 0);

        buf[msg_size] = '\0';
        memcpy(&buf[0], &start.tv_sec, sizeof(size_t));
        memcpy(&buf[sizeof(size_t)], &start.tv_usec, sizeof(size_t));
        buf[4] = 0x01;
        buf[5] = 0x01;
        buf[6] = 0x01;
        buf[7] = 0x01;
        buf[11] = 0x01;
        buf[12] = 0x01;
        buf[13] = 0x01;
        buf[14] = 0x01;
        buf[15] = 0x01;

        UA_String val = UA_STRING(buf);
        UA_Variant value;
        UA_Variant_init(&value);
        UA_Variant_setScalarCopy(&value, &val, &UA_TYPES[UA_TYPES_STRING]);

        status = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "/test/lat"), &value);

        UA_Variant_clear(&value);
        usleep(1000000 / msgs_per_second);
    }

    UA_Client_delete(client);
    return status;
}
