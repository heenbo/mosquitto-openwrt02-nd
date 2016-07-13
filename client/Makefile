CROSS_COMPILE = mips-openwrt-linux-
CC = gcc

CFLAGS = -Wall -ggdb -O2 -I./include
#CFLAGS = -Wall -O2 -I./include
JSON_CFLAGS = -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include/json-c 
MPD_CFLAGS = -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include/mpd
#I2C_CFLAGS = -I/home/heenbo/work/openwrt-bb/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/include/linux
CLIENT_CFLAGS = ${CFLAGS} ${JSON_CFLAGS} ${MPD_CFLAGS} ${I2C_CFLAGS} -I/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/include/

CLIENT_LDFLAGS = ${CFLAGS} -L/home/heenbo/work/openwrt-bb/staging_dir/target-mips_34kc_uClibc-0.9.33.2/usr/lib -lssl -lcrypto -lcares -lmosquitto -pthread -ljson-c -lmpdclient -lcurl -lpolarssl

.PHONY: all reallyclean clean

all : mosquitto_client

mosquitto_client : main.o mosquitto_client.o parse_msg.o read_json.o init_device.o i2c_button.o
	${CROSS_COMPILE}${CC} $^ -o $@ ${CLIENT_LDFLAGS}

main.o : main.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

mosquitto_client.o : mosquitto_client.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

parse_msg.o : parse_msg.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

read_json.o : read_json.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS} 

init_device.o : init_device.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

i2c_button.o : i2c_button.c
	${CROSS_COMPILE}${CC} -c $< -o $@ ${CLIENT_CFLAGS}

reallyclean : clean
	
clean : 
	-rm -f *.o mosquitto_client
