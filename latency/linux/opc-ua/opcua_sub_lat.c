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
char *name = "subscriber";
char *scenario = "copc-sopc-copc";
size_t msg_size = sizeof(size_t) + sizeof(size_t);
size_t msgs_per_second;

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("USAGE:\n\topcua_sub_lat <msgs_per_second>\n\n");
        exit(EXIT_FAILURE);
    }

    msgs_per_second = atoi(argv[1]);

    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode status = UA_Client_connect(client, "opc.tcp://192.168.11.1:4840");
    if(status != UA_STATUSCODE_GOOD)
    {
        UA_Client_delete(client);
        return status;
    }

    UA_Variant value;
    UA_Variant_init(&value);

    char *buf = NULL;
    char *prev = (char *)malloc(msg_size + 1);
    prev[msg_size] = '\0';
    while (1)
    {
        status = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, "/test/lat"), &value);
        if(status == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING]))
        {
            struct timeval stop;
            gettimeofday(&stop, 0);

            buf = (char *)(*(UA_String*)value.data).data;
            buf[4] = 0x00;
            buf[5] = 0x00;
            buf[6] = 0x00;
            buf[7] = 0x00;
            buf[11] = 0x00;
            buf[12] = 0x00;
            buf[13] = 0x00;
            buf[14] = 0x00;
            buf[15] = 0x00;

            size_t *sec = (size_t *) &buf[0];
            size_t *usec = (size_t *) &buf[sizeof(size_t)];

            double lat = stop.tv_usec - *usec;
            if (lat <= 0)
                lat += (stop.tv_sec - *sec) * 1000000;

            if (memcmp(buf, prev, msg_size) == 0)
                continue;
            memcpy(prev, buf, msg_size);

            printf("%s,%s,%s,%s,%lu,%lu,%f\n", layer, name, test, scenario, msgs_per_second, *sec, lat);
        }
    }

    UA_Variant_clear(&value);
    UA_Client_delete(client);
    return status;
}
