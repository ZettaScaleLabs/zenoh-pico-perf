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
char *test = "overhead";
char *name = "subscriber";
char *scenario = "copc-sopc-copc";
int msg_size = 0;

void print_stats(volatile struct timeval start, volatile struct timeval stop)
{
    double t0 = start.tv_sec + ((double)start.tv_usec / 1000000.0);
    double t1 = stop.tv_sec + ((double)stop.tv_usec / 1000000.0);
    double msgs_per_sec = N / (t1 - t0);
    printf("%s,%s,%s,%s,%d,%f\n", layer, name, test, scenario, msg_size, msgs_per_sec);
}

int main(int argc, char *argv[])
{
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode status = UA_Client_connect(client, "opc.tcp://localhost:4840");
    if(status != UA_STATUSCODE_GOOD)
    {
        UA_Client_delete(client);
        return status;
    }

    UA_Variant value;
    UA_Variant_init(&value);
    unsigned long long int count = 0;
    struct timeval start;
    struct timeval stop;
    status = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, "/test/ovh"), &value);
    if(status == UA_STATUSCODE_GOOD && UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING]))
    {
        printf("%.*s\n", (*(UA_String*)value.data).length, (*(UA_String*)value.data).data);
    }

    UA_Variant_clear(&value);
    UA_Client_delete(client);
    return status;
}
