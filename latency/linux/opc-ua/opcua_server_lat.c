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

#include <signal.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

UA_Boolean running = true;
void signalHandler(int sig)
{
    running = false;
}

int main(int argc, char** argv)
{
    signal(SIGINT, signalHandler);

    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;
    attr.displayName = UA_LOCALIZEDTEXT("en-US", "/test/lat");
    UA_String val = UA_STRING("1234567890123456");
    UA_Variant_setScalar(&attr.value, &val, &UA_TYPES[UA_TYPES_STRING]);

    UA_NodeId newNodeId = UA_NODEID_STRING(1, "/test/lat");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_NodeId variableType = UA_NODEID_NULL;
    UA_QualifiedName browseName = UA_QUALIFIEDNAME(1, "/test/lat");

    UA_Server_addVariableNode(server, newNodeId, parentNodeId, parentReferenceNodeId,
                              browseName, variableType, attr, NULL, NULL);

    UA_StatusCode status = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return status;
}