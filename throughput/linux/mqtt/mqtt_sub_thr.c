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

#include "MQTTAsync.h"

#define MQTT_BROKER   "tcp://127.0.0.1:1883"
#define MQTT_TOPIC    "/test/thr"
#define MQTT_CLIENTID "mqtt_sub_thr"
#define MQTT_QOS      0

#define N 100000

char *layer = "mqtt";
char *test = "throughput";
char *name = "subscriber";
char *scenario = "mpub-broker-msub";
int msg_size = 0;

volatile int ready = 0;
volatile int subscribed = 0;

volatile unsigned long long int count = 0;
volatile struct timeval start;
volatile struct timeval stop;

void print_stats(volatile struct timeval start, volatile struct timeval stop)
{
    double t0 = start.tv_sec + ((double)start.tv_usec / 1000000.0);
    double t1 = stop.tv_sec + ((double)stop.tv_usec / 1000000.0);
    double msgs_per_sec = N / (t1 - t0);
    printf("%s,%s,%s,%s,%d,%f\n", layer, name, test, scenario, msg_size, msgs_per_sec);
}

int data_handler(void *context, char *topicName, int topicLen, MQTTAsync_message *message)
{
    if (strncmp(topicName, MQTT_TOPIC, topicLen) != 0 )
        goto ERR;

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

    ERR:
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);

    return 1;
}

void on_subscribe(void* context, MQTTAsync_successData5* response)
{
    subscribed = 1;
}

void on_subscribe_failure(void* context, MQTTAsync_failureData5* response)
{
    printf("Subscribe failed, rc %d\n", response->code);
    exit(EXIT_FAILURE);
}


void on_connect_failure(void* context, MQTTAsync_failureData5* response)
{
    printf("Connect failed, rc %d\n", response->code);
    exit(EXIT_FAILURE);
}


void on_connect(void* context, MQTTAsync_successData5* response)
{
    ready = 1;
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("USAGE:\n\tmqtt_sub_thr <payload_size>\n\n");
        exit(EXIT_FAILURE);
    }

    msg_size = atoi(argv[1]);

    MQTTAsync client;
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    create_opts.MQTTVersion = MQTTVERSION_5;
    int rc = MQTTAsync_createWithOptions(&client, MQTT_BROKER, MQTT_CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to create client object, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer5;
    conn_opts.keepAliveInterval = 3;
    conn_opts.onSuccess5 = on_connect;
    conn_opts.onFailure5 = on_connect_failure;
    conn_opts.context = client;
    conn_opts.MQTTVersion = MQTTVERSION_5;
    conn_opts.cleanstart = 1;
    rc = MQTTAsync_connect(client, &conn_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start connect, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        exit(EXIT_FAILURE);
    }

    while (!ready);

    MQTTAsync_responseOptions sub_opts = MQTTAsync_responseOptions_initializer;
    sub_opts.onSuccess5 = on_subscribe;
    sub_opts.onFailure5 = on_subscribe_failure;
    sub_opts.context = client;
    rc = MQTTAsync_subscribe(client, MQTT_TOPIC, MQTT_QOS, &sub_opts);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        exit(EXIT_FAILURE);
    }

    rc = MQTTAsync_setCallbacks(client, client, NULL, data_handler, NULL);
    if (rc != MQTTASYNC_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        MQTTAsync_destroy(&client);
        exit(EXIT_FAILURE);
    }

    while (!subscribed);

    while (1);

    MQTTAsync_responseOptions uns_opts = MQTTAsync_responseOptions_initializer;
    uns_opts.context = client;
    rc = MQTTAsync_unsubscribe(client, MQTT_TOPIC, &uns_opts);

    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    rc = MQTTAsync_disconnect(client, &disc_opts);
    MQTTAsync_destroy(&client);

    exit(EXIT_SUCCESS);
}
