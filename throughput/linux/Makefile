SHELL := /bin/bash

all:
	mkdir -p ./target

	# TCP
	gcc -O3 -Wall tcp/tcp_pub_thr.c -o target/tcp_pub_thr
	gcc -O3 -Wall tcp/tcp_sub_thr.c -o target/tcp_sub_thr

	# MQTT: paho-mqtt-c v1.3.8
	gcc -O3 -Wall mqtt/mqtt_pub_thr.c -lpaho-mqtt3a -o target/mqtt_pub_thr
	gcc -O3 -Wall mqtt/mqtt_sub_thr.c -lpaho-mqtt3a -o target/mqtt_sub_thr

	# DDS-XRCE
	gcc -O3 -Wall dds-xrce/ddsxrce_pub_thr.c -lmicroxrcedds_client -lmicrocdr -o target/ddsxrce_pub_thr
	gcc -O3 -Wall dds-xrce/ddsxrce_sub_thr.c -lmicroxrcedds_client -lmicrocdr -o target/ddsxrce_sub_thr

	# OPC-UA
	gcc -O3 -Wall opc-ua/opcua_pub_thr.c -lopen62541 -o target/opcua_pub_thr
	gcc -O3 -Wall opc-ua/opcua_sub_thr.c -lopen62541 -o target/opcua_sub_thr
	gcc -O3 -Wall opc-ua/opcua_server_thr.c -lopen62541 -o target/opcua_server_thr

build_zenoh:
	@../../scripts/build-zenoh.sh zenoh

clean:
	rm -rf target
