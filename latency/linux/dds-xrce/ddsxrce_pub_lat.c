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
#define STREAM_HISTORY  4
#define BUFFER_SIZE UXR_CONFIG_UDP_TRANSPORT_MTU * STREAM_HISTORY

char *layer = "XRCE-DDS";
char *test = "throughput";
char *name = "publisher";
char *scenario = "dpub-dagent-dsub";
size_t msg_size = sizeof(size_t) + sizeof(size_t);
size_t msgs_per_second = 1;

typedef struct
{
    uint8_t *buf;
} MyDataType;

bool MyDataType_serialize_topic(ucdrBuffer* writer, const MyDataType* data)
{
    ucdr_serialize_array_uint8_t(writer, data->buf, msg_size + 1);
    return !writer->error;
}

uint32_t MyDataType_size_of_topic(const MyDataType* data, uint32_t size)
{
    uint32_t previousSize = size;
    size += (uint32_t)(ucdr_alignment(size, 4) + 4 + msg_size + 1);
    return size - previousSize;
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("USAGE:\n\tddsxrce_pub_thr <msgs_per_second>\n\n");
        exit(EXIT_FAILURE);
    }

    msgs_per_second = atoi(argv[1]);

    uxrUDPTransport transport;
    if (!uxr_init_udp_transport(&transport, UXR_IPv4, DDS_IP, DDS_PORT))
        exit(EXIT_FAILURE);

    uxrSession session;
    uxr_init_session(&session, &transport.comm, 0x11111111);
    if (!uxr_create_session(&session))
        exit(EXIT_FAILURE);

    uint8_t output_reliable_stream_buffer[BUFFER_SIZE];
    uxrStreamId reliable_out = uxr_create_output_reliable_stream(&session, output_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    uint8_t input_reliable_stream_buffer[BUFFER_SIZE];
    uxr_create_input_reliable_stream(&session, input_reliable_stream_buffer, BUFFER_SIZE, STREAM_HISTORY);

    uint8_t output_besteffort_stream_buffer[BUFFER_SIZE];
    uxrStreamId besteffort_out = uxr_create_output_best_effort_stream(&session, output_besteffort_stream_buffer, BUFFER_SIZE);

    // Create entities
    uxrObjectId participant_id = uxr_object_id(0x01, UXR_PARTICIPANT_ID);
    const char* participant_xml = "<dds><participant><rtps><name>default_xrce_participant</name></rtps></participant></dds>";
    uint16_t participant_req = uxr_buffer_create_participant_xml(&session, reliable_out, participant_id, 0, participant_xml, UXR_REPLACE);

    uxrObjectId topic_id = uxr_object_id(0x01, UXR_TOPIC_ID);
    const char* topic_xml = "<dds><topic><name>HelloWorldTopic</name><dataType>MyDataType</dataType></topic></dds>";
    uint16_t topic_req = uxr_buffer_create_topic_xml(&session, reliable_out, topic_id, participant_id, topic_xml, UXR_REPLACE);

    uxrObjectId publisher_id = uxr_object_id(0x01, UXR_PUBLISHER_ID);
    const char* publisher_xml = "";
    uint16_t publisher_req = uxr_buffer_create_publisher_xml(&session, reliable_out, publisher_id, participant_id, publisher_xml, UXR_REPLACE);

    uxrObjectId datawriter_id = uxr_object_id(0x01, UXR_DATAWRITER_ID);
    const char* datawriter_xml = "<dds><data_writer><topic><kind>NO_KEY</kind><name>HelloWorldTopic</name><dataType>MyDataType</dataType></topic></data_writer></dds>";
    uint16_t datawriter_req = uxr_buffer_create_datawriter_xml(&session, reliable_out, datawriter_id, publisher_id, datawriter_xml, UXR_REPLACE);

    uint8_t status[4];
    uint16_t requests[4] = { participant_req, topic_req, publisher_req, datawriter_req};
    if (!uxr_run_session_until_all_status(&session, 1000, requests, status, 4))
        exit(EXIT_FAILURE);

    uint8_t *data = (uint8_t *)malloc(msg_size + 1);
    data[msg_size] = '\0';
    MyDataType topic = { data };

    bool connected = true;
    while (connected)
    {
        struct timeval start;
        gettimeofday(&start, 0);

        memcpy(&data[0], &start.tv_sec, sizeof(size_t));
        memcpy(&data[sizeof(size_t)], &start.tv_usec, sizeof(size_t));

        ucdrBuffer ub;
        uint32_t topic_size = MyDataType_size_of_topic(&topic, 0);
        uxr_prepare_output_stream(&session, besteffort_out, datawriter_id, &ub, topic_size);
        MyDataType_serialize_topic(&ub, &topic);

        connected = uxr_run_session_time(&session, 0);

        usleep(1000000 / msgs_per_second);
    }

    uxr_delete_session(&session);
    uxr_close_udp_transport(&transport);

    exit(EXIT_SUCCESS);
}
