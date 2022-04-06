all:
	mkdir -p ./target

	# MQTT: paho-mqtt-c v1.3.8
	gcc -O3 -Wall mqtt/mqtt_pub_ovh.c -lpaho-mqtt3a -o target/mqtt_pub_ovh
	gcc -O3 -Wall mqtt/mqtt_sub_ovh.c -lpaho-mqtt3a -o target/mqtt_sub_ovh

	# DDS-XRCE
	gcc -O3 -Wall dds-xrce/ddsxrce_pub_ovh.c -lmicroxrcedds_client -lmicrocdr -o target/ddsxrce_pub_ovh
	gcc -O3 -Wall dds-xrce/ddsxrce_sub_ovh.c -lmicroxrcedds_client -lmicrocdr -o target/ddsxrce_sub_ovh

	# OPC-UA
	gcc -O3 -Wall opc-ua/opcua_pub_thr.c -lopen62541 -o target/opcua_pub_thr
	gcc -O3 -Wall opc-ua/opcua_sub_thr.c -lopen62541 -o target/opcua_sub_thr
	gcc -O3 -Wall opc-ua/opcua_server_thr.c -lopen62541 -o target/opcua_server_thr

	# ZENOH-PICO
	gcc -O3 -Wall -DZENOH_LINUX zenoh/zn_pub_ovh.c -lzenohpico -o target/zn_pub_ovh
	gcc -O3 -Wall -DZENOH_LINUX zenoh/zn_sub_ovh.c -lzenohpico -o target/zn_sub_ovh

clean:
	rm -rf target
