all:
	mkdir -p ./target

	# TCP
	gcc -O3 -Wall tcp/tcp_pub_lat.c -o target/tcp_pub_lat
	gcc -O3 -Wall tcp/tcp_sub_lat.c -o target/tcp_sub_lat

	# MQTT: paho-mqtt-c v1.3.8
	gcc -O3 -Wall mqtt/mqtt_pub_lat.c -lpaho-mqtt3a -o target/mqtt_pub_lat
	gcc -O3 -Wall mqtt/mqtt_sub_lat.c -lpaho-mqtt3a -o target/mqtt_sub_lat

	# DDS-XRCE
	gcc -O3 -Wall dds-xrce/ddsxrce_pub_lat.c -lmicroxrcedds_client -lmicrocdr -o target/ddsxrce_pub_lat
	gcc -O3 -Wall dds-xrce/ddsxrce_sub_lat.c -lmicroxrcedds_client -lmicrocdr -o target/ddsxrce_sub_lat

	# OPC-UA
	gcc -O3 -Wall opc-ua/opcua_pub_lat.c -lopen62541 -o target/opcua_pub_lat
	gcc -O3 -Wall opc-ua/opcua_sub_lat.c -lopen62541 -o target/opcua_sub_lat
	gcc -O3 -Wall opc-ua/opcua_server_lat.c -lopen62541 -o target/opcua_server_lat

	# ZENOH-PICO
	gcc -O3 -Wall -DZENOH_LINUX zenoh/zn_pub_lat.c -lzenohpico -o target/zn_pub_lat
	gcc -O3 -Wall -DZENOH_LINUX zenoh/zn_sub_lat.c -lzenohpico -o target/zn_sub_lat

clean:
	rm -rf target
