#ifndef _MOSQUITTO_CLINET_H_
#define _MOSQUITTO_CLINET_H_

/*config*/
#define CLIENT_ID	"mosquitto_client_pub"
#define HOST_ADDRESS	"192.168.199.244"
#define HOST_PORT	8883
#define KEEPALIVE	60
#define QOS_LEVEL	2
#define MQTT_SERVER_TOPIC	"mqtt"

/*cafile  certfile  keyfile*/
#define TLS_VERSION 	"tlsv1"
#define CA_FILE_PATH	"/test/ca.crt"
#define CERT_FILE_PATH	"/test/client.crt"
#define KEY_FILE_PATH	"/test/client.key"

/*user name & passwd*/
#define USER_NAME 	"001"
#define USER_PASSWD	"001"


/*mosquitto pub & sub init*/
extern int msquitto_pub_sub_init(char * device_id);

/*connect success callback*/
extern void on_connect(struct mosquitto *mosq, void *obj, int rc);

/*disconnect callback*/
extern void on_disconnect(struct mosquitto *mosq, void *obj, int rc);

/*on_log_callback*/
extern void on_log_callback(struct mosquitto *mosq, void *obj, int level, const char *str);


/*receive message callback*/
extern void receive_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);

/*mosquitto_publish_send_msg*/
extern int mosquitto_publish_send_msg(const char * topic, int payloadlen, const void * payload);

#endif
