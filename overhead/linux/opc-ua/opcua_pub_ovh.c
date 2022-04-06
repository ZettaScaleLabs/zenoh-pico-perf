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
char *test = "overhead";
char *name = "publisher";
char *scenario = "mpub-broker-msub";
size_t msg_size = sizeof(size_t) + sizeof(size_t);

int main(int argc, char **argv)
{
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
    status = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, "/test/ovh"), &value);
    if(status == UA_STATUSCODE_GOOD)
    {
        // Do nothing
    }

    UA_Variant_clear(&value);
    UA_Client_delete(client);
    return status;
}
