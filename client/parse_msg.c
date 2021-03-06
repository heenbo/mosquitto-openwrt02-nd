#include <stdio.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <linux/input.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <i2c-dev.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <json/json.h>
#include <json_object.h>
#include <connection.h>
#include <playlist.h>
#include <player.h>
#include <status.h>
#include <response.h>
#include <queue.h>
#include <database.h>
#include <mosquitto.h>

#include "i2c_button.h"
#include "mosquitto_client.h"
#include "parse_msg.h"
#include "read_json.h"

bool bLogin = false;

/*NdInforThread*/
int nNdInfor = -1;
pthread_t pthdNdInfor;

/*BatteryCheckThread*/
int nBatteryCheck = -1;
pthread_t pthdBatteryCheck;
bool bBatteryAlarm = false;
char * ch_music_url = NULL;

extern char device_id[32];
extern int i2c_file;
extern pthread_mutex_t mutex;
extern struct mpd_connection *conn;
extern pthread_rwlock_t json_rwlock_voice;
extern pthread_rwlock_t json_rwlock_download;

/*parse_msg*/
int parse_msg(CONTEXT * ctx, json_object * pjson_obj_msg_recv)
{
	
        json_object * pjson_obj_cmd = NULL;
        json_object * pjson_obj_did = NULL;

	char * cmd_str = NULL;
        char * did_str = NULL;

        printf("line: %d len:%d str:%s\n", __LINE__, strlen(json_object_to_json_string(pjson_obj_msg_recv)), json_object_to_json_string(pjson_obj_msg_recv));
        if (json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_CMD, &pjson_obj_cmd) == TRUE)
        {
                if (json_type_string == json_object_get_type(pjson_obj_cmd))
                {
                        cmd_str = (char *)json_object_get_string(pjson_obj_cmd);
                        printf("cmd_str:%s\n", cmd_str);
                        json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_DEVICE_ID, &pjson_obj_did);
                        if (pjson_obj_did)
                        {
                                did_str = (char *)json_object_get_string(pjson_obj_did);
                        }
	
			if (!strcmp(cmd_str, CMD_STR_LOGIN))
                        {
				do_parse_msg_login(pjson_obj_msg_recv, did_str);
                        }
                        else if (!strcmp(cmd_str, CMD_STR_PLAY_MUSIC))
                        {
				do_parse_msg_play_music(pjson_obj_msg_recv, did_str);
                        }
			else if (!strcmp(cmd_str, CMD_STR_PLAY_VOICE))
			{
				do_parse_msg_play_voice(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_LEAVE_CONTENTPLAY))
			{
				do_parse_msg_leave_contentplay(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_LEAVE_AUDIO))
			{
				do_parse_msg_leave_audio(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_SET_VOL))
                        {
				do_parse_msg_set_vol(pjson_obj_msg_recv, did_str);
                        }
                        else if (!strcmp(cmd_str, CMD_STR_DOWNLOAD))
                        {
                                insert_json_object_to_file(DOWNLOAD_JSON_PATH, &json_rwlock_download, pjson_obj_msg_recv);
                        }
			else if (!strcmp(cmd_str, CMD_STR_PAUSE))
			{
				do_parse_msg_pause(pjson_obj_msg_recv, did_str);
			}
		 	else if (!strcmp(cmd_str, CMD_STR_STOP)) 
			{
				do_parse_msg_stop(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_POWER_OFF))
			{
				do_parse_msg_power_off(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_PLAY_STATUS))
			{
				do_parse_msg_play_status(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_LIST_FILE))
			{
				do_parse_msg_list_file(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_LIST_FILE_PLAY))
			{
				do_parse_msg_list_file_play(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_LIST_FILE_DELETE))
			{
				do_parse_msg_list_file_delete(pjson_obj_msg_recv, did_str);
			}
			else if (!strcmp(cmd_str, CMD_STR_XFCHATMUSIC))
			{
				do_parse_msg_xfchatmusic(pjson_obj_msg_recv, did_str);
			}

		}
	}
	return 0;
}

/*parse_cmd*/
int parse_cmdstr(json_object * pjson_obj_cmd)
{
        json_object *cmd = NULL, *did = NULL;
        char * str = NULL;
        //char * did_str = NULL;

        if (NULL == pjson_obj_cmd)
        {
                printf("parse_cmdstr Error, parse fifo json token , failed\n");
                return -1;
        }

        if (json_object_object_get_ex(pjson_obj_cmd, CMD_STR_CMD, &cmd) == TRUE)
        {
                if (json_object_get_type(cmd) == json_type_string)
                {
                        str = (char *)json_object_get_string(cmd);
                        json_object_object_get_ex(pjson_obj_cmd, CMD_STR_DEVICE_ID, &did);
                        if (did)
                        {
                                //did_str = (char *)json_object_get_string(did);
                                json_object_get_string(did);
                        }
                        if (!strcmp(str, CMD_STR_SEND_VOICE))
                        {
                                json_object *contact_id = NULL;
                                char *contact_str = NULL;
                                system("upvoice.sh");
                                json_object_object_get_ex(pjson_obj_cmd, CMD_STR_CONTACT_ID, &contact_id);
                                if (contact_id)
                                {
                                        contact_str = (char *)json_object_get_string(contact_id);
                                }
                                send_voice(contact_str);
                        }

                        if (!strcmp(str, CMD_STR_XFCHAT_MUSIC))
                        {
                                mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_cmd)), json_object_to_json_string(pjson_obj_cmd));
                        }

                        if (!strcmp(str, CMD_STR_STOP))
                        {
                                system("/usr/bin/ais_led blue off");
                        }
                }
        }
        return 0;
}


/*get_leave_audio*/
static int get_leave_audio(const char * device_id)
{
        json_object * pjson_obj_msg = NULL;
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(CMD_STR_LEAVE_AUDIO));
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;
        return 0;
}

/*get_leave_vod*/
static int get_leave_vod(const char *device_id)
{
        json_object * pjson_obj_msg = NULL;
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(CMD_STR_LEAVE_CONTENTPLAY));
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;

        return 0;
}

int send_set_volume_result(const char *device_id, const char *cmd, const char *result, const char *volume)
{
        json_object * pjson_obj_msg = NULL;
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(cmd));
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));
        json_object_object_add(pjson_obj_msg, CMD_STR_VOLUME, json_object_new_string(volume));
        json_object_object_add(pjson_obj_msg, CMD_STR_RESULT, json_object_new_string(result));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;

        return 0;
}

/*send_get_vol_result*/
int send_get_vol_result(const char * device_id, const char * cmd, const char * volume)
{
        json_object * pjson_obj_msg = NULL;
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(cmd));
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));
        json_object_object_add(pjson_obj_msg, CMD_STR_VOLUME, json_object_new_string(volume));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;

        return 0;
}

/*do_get_volume*/
static int do_get_volume(const char *device_id)
{
        FILE *fp = NULL;
        char volbuf[1024] = {0};
        int nVol = 0;
        char sVol[32] = {0};

        printf("do_get_volume: device_id: %s\n", device_id);
        system("amixer cget numid=6 | grep \": values=\" | awk -F = {'print $2'} | awk -F , {'print $1'} > /tmp/get_vol.txt");

        fp = fopen("/tmp/get_vol.txt", "r");
        if (!fp)
        {
                perror("open get_vol.txt ");
        }
        fread((void *)volbuf, 1024, 1, fp);
        fclose(fp);
        printf("nd do_get_volume read volbuf: %s\n", volbuf);

        nVol = atoi(volbuf);
        printf("nd do_get_volume: atoi(volbuf): %d\n", nVol);
        if(nVol <= 95)
        {
                nVol = 0;
        }
        else
        {
                nVol = (nVol -95) / 3;
        }
        sprintf(sVol, "%d", nVol);
        printf("nd do_get_volume: nVol: %d\n", nVol);
        printf("nd do_get_volume: sVol: %s\n", sVol);
        send_get_vol_result(device_id, CMD_STR_GET_VOL, sVol);

        return 0;
}

int do_set_volume(const char *did, const char *vol)
{
        char cmdstr[4096];
        int nVol = atoi(vol);
        int nSetVol = 0;

        if(0 == nVol)
        {
                nSetVol = 0;
        }
        else
        {
                nSetVol = SET_VOL_MIN + SET_VOL_ULTIPLE*nVol;
        }

        printf("do_set_volume: vol: %s, nVol: %d, nSetVol: %d\n", vol, nVol, nSetVol);
        snprintf(cmdstr, 4096, "amixer cset numid=6 %d,%d", nSetVol, nSetVol);
        if (!system(cmdstr))
        {
                send_set_volume_result(did, CMD_STR_SET_VOL, CMD_STR_OK, vol);
        }
        else
        {
                send_set_volume_result(did, CMD_STR_SET_VOL, CMD_STR_FAILED, vol);
        }

        return 0;
}

/*send_battery_indicator_result*/
int send_battery_indicator_result(const char * device_id, const char * cmd, int n_value)
{
        json_object * pjson_obj_msg = NULL;
        printf("nd send_battery_indicator_result did: %s, cmd: %s, n_value: %d\n", device_id, cmd, n_value);
//      snprintf(str, 4096, "{\"cmd\":\"%s\", \"device_id\":\"%s\", \"value\":\"%d\"}", cmd, device_id, n_value);
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(cmd));
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));
        json_object_object_add(pjson_obj_msg, CMD_STR_VALUE, json_object_new_int(n_value));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;

        return 0;
}

/*do_getcontacts*/
int do_getcontacts(const char *device_id)
{
        json_object * pjson_obj_msg = NULL;
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string("get_contact"));
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;
        return 0;
}

/*NdInforThread*/
void* NdInforThread(void *arg)
{
        while (1)
        {
                sleep(2);
                get_leave_audio(device_id);
                get_leave_vod(device_id);
                do_get_volume(device_id);
                if(-1 == nBatteryCheck)
                {
                        nBatteryCheck = pthread_create(&pthdBatteryCheck, NULL, BatteryCheckThread, NULL);
                }
                break;
        }
        return NULL;
}

/*BatteryCheckThread*/
void * BatteryCheckThread(void * arg)
{
        int res = 0;

        while (1)
        {
                res = 0;
                if(i2c_file > 0)
                {
                        res = i2c_smbus_read_byte_data(i2c_file, REGISTER4);
                        printf("nd BatteryCheckThread res: %d\n", res);
                        //send_battery_indicator_result(device_id, "full_battery_indicator", res);      

                        FILE *fp_gpio7 = NULL;
                        char ch_gpio7_value[1024] = {0};
                        fp_gpio7 = fopen("/sys/class/gpio/gpio7/value", "r");
                        if (!fp_gpio7)
                        {
                                perror("open /sys/class/gpio/gpio7/value ");
                        }
                        else
                        {
                                if(0 >= fread((void *)ch_gpio7_value, 1024, 1, fp_gpio7))
                                {
                                        printf("error: fread /sys/class/gpio/gpio7/value failure\n");
                                }

                                fclose(fp_gpio7);
                        }

                        if(res <= 20 && !bBatteryAlarm)
                        {
                                bBatteryAlarm = true;
                                send_battery_indicator_result(device_id, "battery_indicator", res);
                        }
                        else if(res >= 100 && !bBatteryAlarm && ch_gpio7_value[0] == '1')
                        {
                                bBatteryAlarm = true;
                                send_battery_indicator_result(device_id, "full_battery_indicator", res);
                        }
                        else if(res > 20 && res < 100)
                        {
                                bBatteryAlarm = false;
                        }
                }

                //sleep(20);
                sleep(20*100);
        }
}

/*send_respone*/
int send_respone(const char * device_id, const char * cmd, const char * result, const char * error_code, const char * uri)
{
        json_object * pjson_obj_msg = NULL;
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(cmd));
        json_object_object_add(pjson_obj_msg, CMD_STR_RESULT, json_object_new_string(result));
        json_object_object_add(pjson_obj_msg, CMD_STR_CODE, json_object_new_string(error_code));
        json_object_object_add(pjson_obj_msg, CMD_STR_VOICE_URI, json_object_new_string(uri));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;

        return 0;
}

/*add_song_to_mpc*/
static bool add_song_to_mpc(struct mpd_connection *conn, const char *path)
{
	bool b_add_song = false;
        printf("nd addsong:, path: %s\n", path);
//	printf("nd addsong: conn: 0x%x, path: %s\n", conn, path);
	//b_add_song = mpd_run_add(conn, path);
	pthread_mutex_lock(&mutex);
	b_add_song = mpd_run_playlist_add(conn, "online.lst", path);
	if(!b_add_song)
	{
		printf("nd addsong: add_song failt b_add_song: %d\n", b_add_song);
	}
	pthread_mutex_unlock(&mutex);

	return b_add_song;
}

/*do_play_music*/
int do_play_music(const char * device_id, const char *uri)
{
        if(0 == strlen(uri))
        {
                send_respone(device_id, CMD_STR_PLAY_MUSIC, CMD_STR_FAILED, "URI_UNREACHABLE", uri);
                return -1;
        }
        //mpd_run_update(conn, "/tmp/music/");
        if (add_song_to_mpc(conn, uri))
        {
                send_respone(device_id, CMD_STR_PLAY_MUSIC, CMD_STR_OK, "SUCCESS", uri);
        }
        else
        {
                send_respone(device_id, CMD_STR_PLAY_MUSIC, CMD_STR_FAILED, "URI_UNREACHABLE", uri);
                return -1;
        }

        return 0;
}

/*do_play_voice*/
static int do_play_voice(const char *device_id, const char *uri)
{

        if (!strcmp(uri, "null"))
        {
                uri = "http://www.aistoy.com:9331/voice/uuid/dde35281d0756aaa3940fc6275328610c106d0b1.amr";
                send_respone(device_id, CMD_STR_PLAY_VOICE, CMD_STR_FAILED, "URI_UNREACHABLE", uri);
        }
        else
        {
                send_respone(device_id, CMD_STR_PLAY_VOICE, CMD_STR_OK, "SUCCESS", uri);
        }

        return 0;
}

void do_parse_msg_login(json_object * pjson_obj_msg_recv, char *did_str)
{

	json_object *result = NULL;
	char * result_str = NULL;
	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_RESULT, &result);
	if (result)
	{
		result_str = (char *)json_object_get_string(result);
	}
	if (!strcmp(result_str, CMD_STR_OK))
	{
//		ctx->state = STATE_LOGINED;
		do_getcontacts(device_id);
		bLogin = true;  
//		bBatteryAlarm = false;        
		if (-1 == nNdInfor)
		{
			nNdInfor = pthread_create(&pthdNdInfor, NULL, NdInforThread, NULL);
		}
	}

}

void do_parse_msg_play_music(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_uri = NULL;
	char * uri_str = NULL;

	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_VOICE_URI, &pjson_obj_uri);
       	if (pjson_obj_uri)
	{
		uri_str = (char *)json_object_get_string(pjson_obj_uri);
		do_play_music(did_str, uri_str);
	}

}

void do_parse_msg_play_voice(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_uri = NULL;
	char * uri_str = NULL;
	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_VOICE_URI, &pjson_obj_uri);
	if (pjson_obj_uri)
	{
		uri_str = (char *)json_object_get_string(pjson_obj_uri);
		if(0 != strlen(uri_str))
		{
			printf("%s\n",json_object_to_json_string(pjson_obj_msg_recv));
			insert_json_object_to_file(PLAY_VOICE_JSON_PATH, &json_rwlock_voice, pjson_obj_msg_recv);
			do_play_voice(did_str, uri_str);
			printf("test  FUNC: %s LINE: %d--------------------------\n", __FUNCTION__, __LINE__);
		}
	}
}

void do_parse_msg_leave_contentplay(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_data = NULL;
	json_object * pjson_obj_file_url = NULL;
	json_object * pjson_obj_uri = NULL;
	char * uri_str = NULL;
	int i = 0;

	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_DATA, &pjson_obj_data);
	if (pjson_obj_data)
	{
		for (i = 0; i < json_object_array_length(pjson_obj_data); i++)
		{
       			pjson_obj_file_url = json_object_array_get_idx(pjson_obj_data, i);
			if (pjson_obj_file_url)
			{
				json_object_object_get_ex(pjson_obj_file_url, CMD_STR_FILEURL, &pjson_obj_uri);
				uri_str = (char *)json_object_get_string(pjson_obj_uri);
				printf("uri_str:%s \n", uri_str);
				do_play_music(did_str, uri_str);
			}
		}
	}
}

void do_parse_msg_leave_audio(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_data = NULL;
	json_object * pjson_obj_audio_url = NULL;
	json_object * pjson_obj_uri = NULL;
	char * uri_str = NULL;
	int i = 0;

	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_DATA, &pjson_obj_data);
	if (pjson_obj_data)
	{
		for (i = 0; i < json_object_array_length(pjson_obj_data); i++)
		{
       			pjson_obj_audio_url = json_object_array_get_idx(pjson_obj_data, i);
			if (pjson_obj_audio_url)
			{
				json_object_object_get_ex(pjson_obj_audio_url, CMD_STR_AUDIOURL, &pjson_obj_uri);
				uri_str = (char *)json_object_get_string(pjson_obj_uri);
				printf("uri_str:%s \n", uri_str);
				insert_json_object_to_file(PLAY_VOICE_JSON_PATH, &json_rwlock_voice, pjson_obj_audio_url);
			}
		}
	}
}

void do_parse_msg_set_vol(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_vol = NULL;
	char * vol_str = NULL;

	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_VOLUME, &pjson_obj_vol);
	if(pjson_obj_vol)
	{
		vol_str = (char *)json_object_get_string(pjson_obj_vol);
	}
	do_set_volume(did_str, vol_str);
}

void do_parse_msg_pause(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_msg = NULL;

	pthread_mutex_lock(&mutex);
	mpd_run_pause(conn, true);
	pthread_mutex_unlock(&mutex);
	
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(CMD_STR_PAUSE));
        json_object_object_add(pjson_obj_msg, CMD_STR_RESULT, json_object_new_string(CMD_STR_OK));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;

}

void do_parse_msg_stop(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_msg = NULL;

	pthread_mutex_lock(&mutex);
	//mpd_run_stop(conn);
	pthread_mutex_unlock(&mutex);
	
        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string("stop"));
        json_object_object_add(pjson_obj_msg, CMD_STR_RESULT, json_object_new_string(CMD_STR_OK));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;
}

void do_parse_msg_power_off(json_object * pjson_obj_msg_recv, char * did_str)
{
	int res = 0;

	i2c_smbus_write_byte_data(i2c_file, REGISTER12, 00);
	res = i2c_smbus_write_byte_data(i2c_file, REGISTER13, 00);
	res = i2c_smbus_write_byte_data(i2c_file, REGISTER14, 03);
	res = i2c_smbus_write_byte_data(i2c_file, REGISTER15, 84);
	
	usleep(50*1000);
	res = i2c_smbus_write_byte_data(i2c_file, REGISTER7, SHUT_DWON);
	if(res < 0)
	{
		printf("nd shut_down i2c_smbus_write_byte_data 0x07, 0x01 failed, res: %d\n", res);
	}
}

void do_parse_msg_play_status(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_msg = NULL;
	char buf[1024] = {0};
	int len = 0;
	int fd = 0;

	system("/usr/bin/play_status.sh");
	fd = open("/tmp/play_status.txt", O_RDONLY);
	if (fd < 0) {
		perror("open play_status.txt");
		return;	
	}
	len = read(fd, (void *)&buf[0], 1024);
	if (len <= 0) {
		perror("read play_status.txt");
		return;
	}
	close(fd);	

        pjson_obj_msg = json_object_new_object();
        json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));
        json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(CMD_STR_PLAY_STATUS));
        json_object_object_add(pjson_obj_msg, CMD_STR_RESULT, json_object_new_string(CMD_STR_OK));

        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        json_object_put(pjson_obj_msg);
        pjson_obj_msg = NULL;
}

static int do_get_localfile(const char *did_str, const char *page_str, const char* ch_open_id, const char* ch_dir);
void do_parse_msg_list_file(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * pjson_obj_page = NULL;     
	char * page_str = NULL;       
	json_object * pjson_obj_open_id = NULL;  
	char * open_id_str = NULL;    
	json_object * pjson_obj_file_type = NULL;
	char * file_type_str = NULL;  
	
	//printf("pjson_obj_msg_recv : %s\n", json_object_to_json_string(pjson_obj_msg_recv));

	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_OPEN_ID, &pjson_obj_open_id);
	if (pjson_obj_open_id)
	{
		open_id_str = (char *)json_object_get_string(pjson_obj_open_id);
	}

	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_FILE_TYPE, &pjson_obj_file_type);
	if (pjson_obj_file_type)
	{
		file_type_str = (char *)json_object_get_string(pjson_obj_file_type);
	}

	json_object_object_get_ex(pjson_obj_msg_recv, CMD_STR_PAGE_NUM, &pjson_obj_page);
	if (pjson_obj_page) 
	{
		page_str = (char *)json_object_get_string(pjson_obj_page);
		do_get_localfile(did_str, page_str, open_id_str, file_type_str);
	}

}

static int do_get_localfile(const char *did_str, const char *page_str, const char* ch_open_id, const char* ch_dir)
{
#define EACH_PAGE_FILES	10
	int i = 0;
	int page_num = 0;
	DIR *dirptr = NULL;
	struct dirent *entry;
	char str_music_dir[1024] = {0};
	char buf_count[32] = {0};

	strcpy(str_music_dir, "/tmp/music/");
	strcat(str_music_dir, ch_dir);
	printf("str_music_dir : %s \n", str_music_dir);

        json_object * pjson_obj_msg = NULL;
        pjson_obj_msg = json_object_new_object();
	json_object * pjson_obj_array_files = NULL;
	pjson_obj_array_files = json_object_new_array();
        json_object * pjson_obj_element = NULL;
        pjson_obj_element = json_object_new_object();
	
	json_object_object_add(pjson_obj_element, "count", json_object_new_string("0"));
	json_object_array_add(pjson_obj_array_files, pjson_obj_element);

	if (page_str)
	{
		page_num = atoi(page_str)-1;
	}	
	if ((dirptr = opendir(str_music_dir)) != NULL) 
	{
		while ((entry = readdir(dirptr)) != NULL) 
		{
			printf("i/xx = %d, entry->d_type = %d\n", i/(EACH_PAGE_FILES), entry->d_type);
			if (strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) 
			{
				continue;  
			}
			if ((i/(EACH_PAGE_FILES) == page_num) && (entry->d_type == 8)) 
			{
				printf("111,++++++++++++++++++++++++++++++++++++++++++++++\n");
				pjson_obj_element = NULL;
        			pjson_obj_element = json_object_new_object();
				char filename_str[512] = {0};
				memcpy(filename_str, entry->d_name, strlen(entry->d_name) - strlen(".mp3"));
				char fileurl_str[512] = {0};
				strcpy(fileurl_str, ch_dir);
				strcat(fileurl_str, "/");
				strcat(fileurl_str, entry->d_name);

				char * picture_str = "http://www.aistoy.com:9331/res-audio/erge/logo.jpg";
				char filetype_str[512];
				strcpy(filetype_str, ch_dir);
				json_object_object_add(pjson_obj_element, "filename", json_object_new_string(filename_str));
				json_object_object_add(pjson_obj_element, "fileurl", json_object_new_string(fileurl_str));
				json_object_object_add(pjson_obj_element, "picture", json_object_new_string(picture_str));
				json_object_object_add(pjson_obj_element, "filetype", json_object_new_string(filetype_str));
				json_object_array_add(pjson_obj_array_files, pjson_obj_element);
			}
			i++;
		}
        	pjson_obj_element = json_object_new_object();
		sprintf(buf_count, "%d", i);
		json_object_object_add(pjson_obj_element, "count", json_object_new_string(buf_count));
		json_object_array_put_idx(pjson_obj_array_files, 0, pjson_obj_element);
	}
	else
	{
		perror("nd do_get_localfile perro:");
		printf("nd do_get_localfile errno: %d\n", errno);
	}

	json_object_object_add(pjson_obj_msg, "cmd", json_object_new_string("list_file"));
	json_object_object_add(pjson_obj_msg, "device_id", json_object_new_string(did_str));
	json_object_object_add(pjson_obj_msg, "open_id", json_object_new_string(ch_open_id));
	json_object_object_add(pjson_obj_msg, "files", pjson_obj_array_files);

	printf("send local file list:%s\n", json_object_to_json_string(pjson_obj_msg));
        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));

        json_object_put(pjson_obj_msg);
        json_object_put(pjson_obj_array_files);
        json_object_put(pjson_obj_element);

	return 0;
}

void do_parse_msg_list_file_play(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * json_obj_open_id = NULL;
	char * open_id_str = NULL;
	json_object * json_obj_file_url = NULL;
	char * file_url_str = NULL;
        json_object * pjson_obj_msg = NULL;

	json_object_object_get_ex(pjson_obj_msg_recv, "open_id", &json_obj_open_id);
	if (json_obj_open_id) 
	{
		open_id_str = (char *)json_object_get_string(json_obj_open_id);
	}

	json_object_object_get_ex(pjson_obj_msg_recv, "file_url", &json_obj_file_url);
	if (json_obj_file_url)
	{
		file_url_str = (char *)json_object_get_string(json_obj_file_url);
		if(0 != strlen(file_url_str))
		{
        		pjson_obj_msg = json_object_new_object();
			if (add_song_to_mpc(conn, file_url_str))
			{
				json_object_object_add(pjson_obj_msg, "device_id", json_object_new_string(device_id));
				json_object_object_add(pjson_obj_msg, "cmd", json_object_new_string("list_file_play"));
				json_object_object_add(pjson_obj_msg, "result", json_object_new_string("ok"));
				json_object_object_add(pjson_obj_msg, "open_id", json_object_new_string(open_id_str));
			} 
			else
			{
				json_object_object_add(pjson_obj_msg, "device_id", json_object_new_string(device_id));
				json_object_object_add(pjson_obj_msg, "cmd", json_object_new_string("list_file_play"));
				json_object_object_add(pjson_obj_msg, "result", json_object_new_string("failed"));
				json_object_object_add(pjson_obj_msg, "open_id", json_object_new_string(open_id_str));
			}	
			printf("send pjson_obj_msg:%s\n", json_object_to_json_string(pjson_obj_msg));
			mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        		json_object_put(pjson_obj_msg);
		}
	}	

}

void do_parse_msg_list_file_delete(json_object * pjson_obj_msg_recv, char * did_str)
{
	int ret = 0;
	json_object * json_obj_open_id = NULL;
	char * open_id_str = NULL;
	json_object * json_obj_file_url = NULL;
	char * file_url_str = NULL;
        json_object * pjson_obj_msg = NULL;
	char music_url_str[512] = {0};
	char rm_lst_str[512] = {0};
	char filelst_m3u_str[128] = {0};
	char filelst_str[128] = {0};
	char filedir_str[128] = {0};

	json_object_object_get_ex(pjson_obj_msg_recv, "open_id", &json_obj_open_id);
	if (json_obj_open_id) 
	{
		open_id_str = (char *)json_object_get_string(json_obj_open_id);
	}

	json_object_object_get_ex(pjson_obj_msg_recv, "file_url", &json_obj_file_url);
	if (json_obj_file_url)
	{
		file_url_str = (char *)json_object_get_string(json_obj_file_url);
		if(0 != strlen(file_url_str))
		{
			strcpy(music_url_str, "/tmp/music/");
			strcat(music_url_str, file_url_str);
			printf("music_url_str:%s\n", music_url_str);
			strncpy(filedir_str, file_url_str, strstr(file_url_str, "/")-file_url_str);
			printf("filedir_str: %s\n", filedir_str);
			
        		pjson_obj_msg = json_object_new_object();
			if(0 == unlink(music_url_str))
			{
				strcpy(filelst_m3u_str, filedir_str);
				strcat(filelst_m3u_str, ".lst.m3u");
				printf("filelst_m3u_str: %s\n", filelst_m3u_str);

				strcpy(filelst_str, filedir_str);
				strcat(filelst_str, ".lst");
				printf("filelst_str: %s\n", filelst_str);
				snprintf(rm_lst_str, 512, "rm -r /etc/config/.mpd/playlists/%s", filelst_m3u_str);
				printf("rm_lst_str: %s\n", rm_lst_str);

				pthread_mutex_lock(&mutex);
				ret = mpd_run_update(conn, filedir_str);
				printf("LINE: %d ret: %d \n", __LINE__, ret);
				system(rm_lst_str);
				printf("LINE: %d ret: %d \n", __LINE__, ret);
				ret = mpd_run_playlist_add(conn, filelst_str, filedir_str);
				printf("LINE: %d ret: %d \n", __LINE__, ret);
				pthread_mutex_unlock(&mutex);
				if(ret)
				{
		
				}

				json_object_object_add(pjson_obj_msg, "device_id", json_object_new_string(device_id));
				json_object_object_add(pjson_obj_msg, "cmd", json_object_new_string("list_file_delete"));
				json_object_object_add(pjson_obj_msg, "result", json_object_new_string("ok"));
				json_object_object_add(pjson_obj_msg, "open_id", json_object_new_string(open_id_str));

			}
			else
			{
				json_object_object_add(pjson_obj_msg, "device_id", json_object_new_string(device_id));
				json_object_object_add(pjson_obj_msg, "cmd", json_object_new_string("list_file_delete"));
				json_object_object_add(pjson_obj_msg, "result", json_object_new_string("failed"));
				json_object_object_add(pjson_obj_msg, "open_id", json_object_new_string(open_id_str));
			}
			printf("send pjson_obj_msg:%s\n", json_object_to_json_string(pjson_obj_msg));
			mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
        		json_object_put(pjson_obj_msg);
		}
	}	

}

static void play_failed_sound(void)
{
	system("echo 0 > /sys/class/gpio/gpio1/value");
	system("aplay -t raw -f S16_LE -r 16000 /usr/share/failed.raw &");
	return;
}

static void play_music(const char * url_str)
{
	printf("play_music LINE:%d  url: %s\n", __LINE__, url_str);
	system("killall aplay");
	system("killall xfchat");
	if(0 == strlen(url_str))
	{
		printf("play_music LINE:%d  url: %s\n", __LINE__, url_str);
		play_failed_sound();
	}
	else
	{
		printf("play_music LINE:%d  url: %s\n", __LINE__, url_str);
		pthread_mutex_lock(&mutex);
		mpd_run_clear(conn);
		mpd_run_add(conn, url_str);
		bool bplay_music = mpd_run_play(conn);
		printf("@@ play music mpd_run_play bplay_music:%d \n", bplay_music);
		pthread_mutex_unlock(&mutex);
	}
	return;
}

void do_parse_msg_xfchatmusic(json_object * pjson_obj_msg_recv, char * did_str)
{
	json_object * json_obj_url = NULL;
	char * url_str = NULL;

	json_object_object_get_ex(pjson_obj_msg_recv, "url", &json_obj_url);
	if (json_obj_url) 
	{
		url_str = (char *)json_object_get_string(json_obj_url);
		if(0 != strlen(url_str))
		{
			strcpy(ch_music_url, url_str);	
			printf("parse_msg ch_music_url: %s, uri_str: %s\n", ch_music_url, url_str);
			play_music(ch_music_url);
			memset(ch_music_url, 0, strlen(ch_music_url));
		}
	}
}

void * LoginThread(void * arg)
{
	while(1)
	{
		printf("FUNC: %s LINE: %d \n", __FUNCTION__, __LINE__);

		if(false == get_bLogin())
		{
			json_object * pjson_obj_msg ;
			pjson_obj_msg = json_object_new_object();
			json_object_object_add(pjson_obj_msg, CMD_STR_CMD, json_object_new_string(CMD_STR_LOGIN));
			json_object_object_add(pjson_obj_msg, CMD_STR_DEVICE_ID, json_object_new_string(device_id));

			printf("send pjson_obj_msg:%s\n", json_object_to_json_string(pjson_obj_msg));
			mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));
			json_object_put(pjson_obj_msg);
			pjson_obj_msg = NULL;
		}
		else
		{
			break;
		}
		sleep(5);
	}
	return NULL;
}

int get_bLogin(void)
{
	return bLogin;
}

int set_bLogin(bool value)
{
	bLogin = value;
	return bLogin;
}
