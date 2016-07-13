#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <json/json.h>
#include <json_object.h>

#include "mosquitto_client.h"
#include "main.h"
#include "read_json.h"
#include "init_device.h"
#include "i2c_button.h"

char device_id[32];

extern struct mpd_connection *conn;

int main(int argc, char * argv[])
{
	int ret = 0;
        if(2 != argc)
        {
//		get_device_id_from_cofig_file();
                printf("ERROR:USAGE:%s device_id\n", argv[0]);
                return -1;
        }
        if(DEVICE_ID_LEN != strlen(argv[1]))
        {
                printf("ERROR:The length of the device ID need %d bit\n", DEVICE_ID_LEN);
                return -1;
        }
	strcpy(device_id, argv[1]);

	/*mosquitto init*/
        ret = msquitto_pub_sub_init(device_id);
        if(0 != ret)
        {
                printf("mosquitto pub & sub init");
                return -1;
        }

 	printf("FUNC: %s LINE: %d Init test start \n", __FUNCTION__, __LINE__);
        system("dd if=/dev/zero of=/tmp/zero.dat bs=1024 count=10");
	
	/*Init_device*/
	Init_device();
	
//	while(1)
	{
		printf("sleep.......\n");
		sleep(5);
	}
        /*doworking*/
	select_my_fd(device_id);

        close(get_read_xfchat_fifo_value());
        close(get_xfchat_fifo_value());

        close_connection(conn);

	return 0;
}

