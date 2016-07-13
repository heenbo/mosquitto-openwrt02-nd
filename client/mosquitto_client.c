#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <json/json.h>
#include <json_object.h>

#include "main.h"
#include "mosquitto_client.h"
#include "i2c_button.h"
#include "parse_msg.h"

struct mosquitto *mosq = NULL;
extern char device_id[32];

/*mosquitto pub & sub init*/
int msquitto_pub_sub_init(char * device_id)
{
	int rc = 0;
	/*init*/
        mosquitto_lib_init();
        /*config*/
        mosq = mosquitto_new(device_id, true, NULL);  //set new client id for mosq
        mosquitto_tls_opts_set(mosq, 1, TLS_VERSION, NULL);     //don`t have to set
        mosquitto_tls_set(mosq, CA_FILE_PATH, NULL, CERT_FILE_PATH, KEY_FILE_PATH, NULL);       //set ca file path
        mosquitto_username_pw_set(mosq, USER_NAME, USER_PASSWD);        //set user name passwd  
        mosquitto_connect_callback_set(mosq, on_connect);       //connect success callback
        mosquitto_disconnect_callback_set(mosq, on_disconnect);         //disconnect callback
        mosquitto_message_callback_set(mosq, receive_message_callback); //receive message callback
        mosquitto_log_callback_set(mosq, on_log_callback);      //log callback
	char buf_will[] = "will message: client xxx logout!  hahahahaha!!!";
	mosquitto_will_set(mosq, "gpio", strlen(buf_will), buf_will, QOS_LEVEL, false);

        /*connect*/
        rc = mosquitto_connect(mosq, HOST_ADDRESS, HOST_PORT, KEEPALIVE);
	if(rc != MOSQ_ERR_SUCCESS)
	{
        	printf("connect error 00\n");
	}
	/*create thread keep the network conection*/
        mosquitto_loop_start(mosq);
	
	return 0;
}

/*connect success callback*/
void on_connect(struct mosquitto *mosq, void *obj, int rc)
{
        int mid = 0;
//      mosquitto_subscribe(mosq, NULL, "qos0/test", 0);
        mosquitto_subscribe(mosq, &mid, device_id, QOS_LEVEL);
        printf("connect success 01\n");
}

/*disconnect callback*/
void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
        printf("disconnect 01\n");
}

/*on_log_callback*/
void on_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str)
{
        printf("%s\n", str);
}

/*receive message callback*/
void receive_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
        printf("\n------------------------------------------------------\n");
        printf("\nreceive message callback: length: %d \n", message->payloadlen);
        fwrite(message->payload, 1, message->payloadlen, stdout);
        printf("\n------------------------------------------------------\n");
        json_object * pjson_obj_msg_recv = NULL;
        pjson_obj_msg_recv = json_tokener_parse(message->payload);
        if(pjson_obj_msg_recv  == NULL)
        {
                printf("got error as expected\n");
        }
	else
	{
        	parse_msg((CONTEXT *)NULL, pjson_obj_msg_recv);
      	  	json_object_put(pjson_obj_msg_recv);

	}
        pjson_obj_msg_recv = NULL;
        printf("------------------------------------------------------\n");
}

/*mosquitto_publish_send_msg*/
int mosquitto_publish_send_msg(const char * topic, int payloadlen, const void * payload)
{
        mosquitto_publish(mosq, NULL, topic, payloadlen, payload, QOS_LEVEL, false);
        return 0;
}

