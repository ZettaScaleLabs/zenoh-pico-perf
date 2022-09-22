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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include <uxr/client/client.h>
#include <ucdr/microcdr.h>

#define DDS_IP "192.168.11.1"
#define DDS_PORT "5555"
#define STREAM_HISTORY  8
#define BUFFER_SIZE UXR_CONFIG_UDP_TRANSPORT_MTU * STREAM_HISTORY

char *layer = "XRCE-DDS";
char *test = "throughput";
char *name = "publisher";
char *scenario = "dpub-dagent-dsub";
size_t msg_size = sizeof(size_t) + sizeof(size_t);
size_t msgs_per_second = 1;

typedef struct
{
    uint8_t buf[8192];
} MyDataType;

bool MyDataType_deserialize_topic(ucdrBuffer* reader, MyDataType* data)
{
    ucdr_deserialize_array_uint8_t(reader, data->buf, msg_size + 1);
    return !reader->error;
}

uint32_t MyDataType_size_of_topic(const MyDataType* data, uint32_t size)
{
    uint32_t previousSize = size;
    size += (uint32_t)(ucdr_alignment(size, 4) + 4 + msg_size + 1);
    return size - previousSize;
}

void on_topic(uxrSession* session, uxrObjectId object_id, uint16_t request_id, uxrStreamId stream_id, struct ucdrBuffer* ub, uint16_t length, void* args)
{
    MyDataType data;
    MyDataType_deserialize_topic(ub, &data);
    size_t *sec = (size_t *) &data.buf[0];
    size_t *usec = (size_t *) &data.buf[sizeof(size_t)];

    struct timeval stop;
    gettimeofday(&stop, 0);

    double lat = stop.tv_usec - *usec;
    if (lat <= 0)
        lat += (stop.tv_sec - *sec) * 1000000;

    printf("%s,%s,%s,%s,%lu,%lu,%f\n", layer, name, test, scenario, msgs_per_second, *sec, lat);
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("USAGE:\n\tddsxrce_sub_lat <msgs_per_second>\n\n");
        exit(EXIT_FAILURE);
    }

    msgs_per_second = atoi(argv[1]);

    uxrUDPTransport transport;
    if (!uxr_init_udp_transport(&transport, UXR_IPv4, DDS_IP, DDS_PORT))
        exit(EXIT_FAILURE);

    uxrSession session;
    uxr_init_session(&session, &transport.comm, 0x22222222);
    uxr_set_topic_callback(&session, on_topic, NULL);
    if (!uxr_create_session(&session))
        exit(EXIT_FAILURE);

    uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
    uxrStreamId reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
    uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    uxrStreamId besteffort_in = uxr_create_input_best_effort_stream(&session);

    uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    const char* participant_xml = "<dds><participant><rtps><name>default_xrce_participant</name></rtps></participant></dds>";
    uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0, participant_xml, UXR_REPLACE);

    uxrObjectId topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
    const char* topic_xml = "<dds><topic><name>HelloWorldTopic</name><dataType>MyDataType</dataType></topic></dds>";
    uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id, participant_id, topic_xml, UXR_REPLACE);

    uxrObjectId subscriber_id = uxr_object_id(0x01, UXR_SUBSCRIBER_ID);
    const char* subscriber_xml = "";
    uint16_t subscriber_req = uxr_buffer_create_subscriber_xml(&session, reliable_out, subscriber_id, participant_id, subscriber_xml, UXR_REPLACE);

    uxrObjectId datareader_id = uxr_object_id(0x01, UXR_DATAREADER_ID);
    const char* datareader_xml = "<dds><data_reader><topic><kind>NO_KEY</kind><name>HelloWorldTopic</name><dataType>MyDataType</dataType></topic></data_reader></dds>";
    uint16_t datareader_req = uxr_buffer_create_datareader_xml(&session, reliable_out, datareader_id, subscriber_id, datareader_xml, UXR_REPLACE);

    uint8_t status[4];
    uint16_t requests[4] = {participant_req, topic_req, subscriber_req, datareader_req};
    if (!uxr_run_session_until_all_status(&session, 1000, requests, status, 4))
        exit(EXIT_FAILURE);

    uxrDeliveryControl delivery_control = {0};
    delivery_control.max_samples = UXR_MAX_SAMPLES_UNLIMITED;
    uxr_buffer_request_data(&session, reliable_out, datareader_id, besteffort_in, &delivery_control);

    while (1)
    {
        uxr_run_session_time(&session, 1000);
    }

    uxr_delete_session(&session);
    uxr_close_udp_transport(&transport);

    exit(EXIT_SUCCESS);
}
