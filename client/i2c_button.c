#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <linux/input.h>
#include <sys/time.h>
#include <sys/types.h>
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
#include "parse_msg.h"
#include "read_json.h"
#include "mosquitto_client.h"
#include "init_device.h"

int i2c_file = -1;
bool bread_register = false;
int ncurrt_time = 0; 
bool bRecord = false;
int nmpc_list = -1;
bool bplay_audio = false;
int xfchat_fifo = -1;
int read_xfchat_fifo = -1;
char *playlists[] = {"eg.lst", "gs.lst", "gx.lst", "yy.lst" , "yyqm.lst" , "teyy.lst", "gdyy.lst", "download.lst"};

/*static*/
/*btn5_click*/
static void btn5_click(CONTEXT *ctx);
/*btn6_click*/
static void btn6_click(CONTEXT *ctx);
/*btn7_click*/
static void btn7_click(CONTEXT *ctx);
/*btn8_click*/
static void btn8_click(CONTEXT *ctx);

static KEY_DESC keys[MAX_BTNS] = {
        {{{0, 0}, 0, BTN_5, 0}, {btn5_click }, 0, 0, KEY_IDLE},
        {{{0, 0}, 0, BTN_6, 0}, {btn6_click }, 0, 0, KEY_IDLE},
        {{{0, 0}, 0, BTN_7, 0}, {btn7_click }, 0, 0, KEY_IDLE},
        {{{0, 0}, 0, BTN_8, 0}, {btn8_click }, 0, 0, KEY_IDLE},
};

/*extern*/
extern pthread_mutex_t mutex;
extern struct mpd_connection *conn;
extern pthread_rwlock_t json_rwlock_voice;
extern pthread_rwlock_t json_rwlock_download;
extern char device_id[32];

/*Create the thread of I2C*/
int nI2C = -1;
pthread_t pthdI2C;

/*Init I2CDev*/
int InitI2CDev(void)
{
        char filename[20];
        i2c_file = open(filename, O_RDWR);

        if (i2c_file < 0 && errno == ENOENT)
        {
                sprintf(filename, "/dev/i2c-%d", 0);
                i2c_file = open(filename, O_RDWR);
        }

        if (i2c_file < 0)
        {
                printf("open error\n");
                return -1;
        }

        if (ioctl(i2c_file, I2C_SLAVE, 0x27) < 0) {
                fprintf(stderr,"Error: Could not set address to : %s\n", strerror(errno));
                return -errno;
        }
	
	/*Create the thread of I2C*/
        nI2C = pthread_create(&pthdI2C, NULL, I2CThread, NULL);

        return 0;
}

/*The thread of I2C*/
void * I2CThread(void *arg)
{
        int res = 0;

        while (1)
        {
                res = 0;
                if (i2c_file > 0)
                {
                        if (!bread_register)
                        {
                                res = i2c_smbus_read_word_data(i2c_file, REGISTER3);
                                switch (res)
                                {
                                        case SWITCH_BTN_SHUT_DWON:
                                        {
                                                printf("nd I2CThread SWITCH_BTN_SHUT_DWON: 0x%0*x\n", 2, res);
                                                break;
                                        }
                                        case SHUT_DWON:
                                        {
                                                printf("nd I2CThread BTN3 SHUT_DWON: 0x%0*x\n", 2, res);
                                                break;
                                        }
                                        case AUTO_SLEEP:
                                        {
                                                printf("nd I2CThread AUTO_SLEEP: 0x%0*x\n", 2, res);
                                                break;
                                        }
                                }

                                bread_register = true;
                        }

                        res = i2c_smbus_read_word_data(i2c_file, REGISTER2);
                        //printf("md I2CThread i2c_file res: 0x%0*x\n", 2, res);
                        switch (res)
                        {
                                case BTN1_SHORT_PRESS:
                                {
                                        printf("nd I2CThread BTN1_SHORT_PRESS: 0x%0*x\n", 2, res);
                                        btn1_short_press();
                                        break;
                                }
                                case BTN1_LONG_PRESS:
                                {
                                        printf("nd I2CThread BTN1_LONG_PRESS: 0x%0*x\n", 2, res);
                                        btn1_long_press();
                                        break;
                                }
                                case BTN1_AUDIO_SPPEK_ARECORD:
                                {
					printf("nd BTN1_AUDIO_SPPEK_ARECORD :: 0x%0*x\n", 2, res);
                                        btn1_audio_sppek_arecord(res);
                                        break;
                                }
                                case BTN2_SHORT_PRESS:
                                {
                                        printf("nd I2CThread BTN2_SHORT_PRESS: 0x%0*x\n", 2, res);
                                        btn2_short_press();
                                        break;
                                }
                                case BTN2_LONG_PRESS:
                                {
                                        printf("nd I2CThread BTN2_LONG_PRESS: 0x%0*x\n", 2, res);
                                        btn2_long_press();
                                        break;
                                }
                                case BTN3_SHORT_PRESS:
                                {
                                        printf("nd I2CThread BTN3_SHORT_PRESS: 0x%0*x\n", 2, res);
                                        btn3_short_press();
                                        break;
                                }
                                case BTN3_LONG_PRESS:
                                {
                                        printf("nd I2CThread BTN3_LONG_PRESS: 0x%0*x\n", 2, res);
                                        btn3_long_press();
                                     break;
                                }
                                case BTN4_SHORT_PRESS:
                                {
                                        printf("nd I2CThread BTN4_SHORT_PRESS: 0x%0*x\n", 2, res);
                                        btn4_short_press();
                                        break;
                                }
                                case BTN4_LONG_PRESS:
                                {
                                        printf("nd I2CThread BTN4_LONG_PRESS: 0x%0*x\n", 2, res);
                                        btn4_long_press();
                                        break;
                                }
                                case BTN5_SHORT_PRESS:
                                {
                                        printf("nd I2CThread BTN5_SHORT_PRESS: 0x%0*x\n", 2, res);
                                        btn5_short_press();
                                        break;
                                }
                                case BTN5_LONG_PRESS:
                                {
                                        printf("nd I2CThread BTN5_LONG_PRESS: 0x%0*x\n", 2, res);
                                        btn5_long_press();
                                        break;
                                }
                                case BTN5_QUESTION_ANSWER_ARECORD:
                                {
                                        btn5_question_answer_arecord(res);
                                        break;
                                }

                                case PEN_START:
                                {
#ifdef PEN_SUPPORT                                      
                                        printf("nd I2CThread PEN_START: 0x%0*x\n", 2, res);
                                        int nValue = 0, nValue8 = 0, nValue9 = 0, nValue10 = 0, nValue89 = 0;

                                        nValue10 = i2c_smbus_read_word_data(i2c_file, REGISTER10);
                                        printf("nd I2CThread REGISTER10: 0x%0*x, %d\n", 2, nValue10, nValue10);

                                        nValue9 = i2c_smbus_read_word_data(i2c_file, REGISTER9);
                                        printf("nd I2CThread REGISTER9: 0x%0*x, %d\n", 2, nValue9, nValue9);

                                        nValue8 = i2c_smbus_read_word_data(i2c_file, REGISTER8);
                                        printf("nd I2CThread REGISTER8: 0x%0*x, %d\n", 2, nValue8, nValue8);

                                        nValue89 = nValue8 | ((nValue9 << 8) & 0xff00);
                                        printf("nd I2CThread nValue89: %x, nValue89: %d, \n", nValue89, nValue89);

                                        nValue = nValue8 | ((nValue9 << 8) & 0xff00) | ((nValue10 << 16) & 0xff0000);
                                        printf("nd I2CThread nValue: %x, \n", nValue, nValue);

                                        if (nValue == 0x60fff8)
                                        {
                                                pthread_mutex_lock(&mutex);
                                                mpd_run_pause(conn, true);
                                                mpd_run_clear(conn);
                                                mpd_run_load(conn, "ljyy.lst");
                                                pthread_mutex_unlock(&mutex);
                                        }
                                        else if (nValue == 0x60fff7)
                                        {

                                        }
                                        else if (nValue89 > 52900)
                                        {
                                                int nPos = nValue89 - 52901;
                                                printf("nd I2CThread nPos: %d, \n", nPos);
                                                pthread_mutex_lock(&mutex);
                                                mpd_run_pause(conn, true);
                                                mpd_run_single(conn, true);
                                                mpd_run_play_pos(conn, nPos);
                                                pthread_mutex_unlock(&mutex);
                                        }

                                        res = i2c_smbus_read_word_data(i2c_file, REGISTER2);
                                        printf("nd I2CThread REGISTER2: 0x%0*x\n", 2, res);
#endif

                                        break;
                                }
                                case NOT_BTN_EVENT:
                                {
                                        break;
                                }
                        }
                }
                else
                {
                        printf("nd I2CThread i2c_file: %d\n", i2c_file);
                        InitI2CDev();
                }

                usleep(50*1000);
        }
}


/*btn1_short_press*/
void btn1_short_press(void)
{
        int res = 0;
        ncurrt_time = 0;
        bRecord = false;
        system("killall arecord");
        res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
        if (res < 0)
        {
                printf("nd btn1_short_press i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
        }
}

/*btn1_long_press*/
void btn1_long_press(void)
{
        int res = 0;
        ncurrt_time = 0;
        bRecord = false;
        system("killall arecord");
        res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
        if (res < 0)
        {
                printf("nd BTN1_LONG_PRESS i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
        }
        json_object * pjson_obj_cmd = NULL;
        pjson_obj_cmd = json_object_new_object();
        json_object_object_add(pjson_obj_cmd, "cmd", json_object_new_string("send_voice"));
        json_object_object_add(pjson_obj_cmd, "contact_id", json_object_new_string("surfL"));

        parse_cmdstr(pjson_obj_cmd);
        json_object_put(pjson_obj_cmd);
}

/*btn1_audio_sppek_arecord*/
void btn1_audio_sppek_arecord(int ret)
{
        ncurrt_time = 0;

        if (!bRecord)
        {
                int res = ret;
                printf("nd BTN1_AUDIO_SPPEK_ARECORD: 0x%0*x\n", 2, res);
                my_mpd_run_pause();
                system("killall aplay");
                system("killall xfchat");
                system("arecord -f S16_LE -r 8000 -c 2 /tmp/upvoice.wav &");
                res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_ON);
                if (res < 0)
                {
                        printf("nd BTN1_AUDIO_SPPEK_ARECORD i2c_smbus_write_byte_data BLUE_ON failed, res: %d\n", res);
                }

                bRecord = true;
        }
}
/*btn2_short_press*/
void btn2_short_press(void)
{
        ncurrt_time = 0;
        pthread_mutex_lock(&mutex);
        mpd_run_previous(conn);
        pthread_mutex_unlock(&mutex);
}

/*btn2_long_press*/
void btn2_long_press(void)
{
        ncurrt_time = 0;
        system("killall aplay");
        system("killall xfchat");

        if (nmpc_list <= 0)
        {
                nmpc_list = 7;
        }
        else
        {
                nmpc_list--;
        }
        char* p = playlists[nmpc_list];

        pthread_mutex_lock(&mutex);
        mpd_run_clear(conn);
        //printf("nd btn4_long_press nmpc_list: %d, p1: %s\n", nmpc_list, p);
        mpd_run_load(conn, p);
        mpd_run_play(conn);
        pthread_mutex_unlock(&mutex);
}

/*btn3_short_press*/
void btn3_short_press(void)
{
        int res = json_type_null;
        json_object * pjson_obj_read = NULL;
        struct mpd_status *status = NULL;

        ncurrt_time = 0;
        system("killall aplay");
        system("killall xfchat");

        pthread_rwlock_wrlock(&json_rwlock_voice);
        pjson_obj_read = json_object_from_file(PLAY_VOICE_JSON_PATH);
        pthread_rwlock_unlock(&json_rwlock_voice);
        res = json_object_get_type(pjson_obj_read);
        printf("json_type: %u \n", json_object_get_type(pjson_obj_read));
//        printf("json_length: %u \n", json_object_array_length(pjson_obj_read));

        json_object_put(pjson_obj_read);
        if ((json_type_array == res) && (0 < json_object_array_length(pjson_obj_read)) && (!bplay_audio))
        {
                printf("nd btn3_short_press ret: %d, bplay_audio: %d\n", res, bplay_audio);
                my_mpd_run_pause();
                bplay_audio = true;
        }
        else
        {
                int quere_len = 0;
                int online_size = 0;
                bplay_audio = false;

                online_size = get_file_size(ONLINE_LIST_PATH);
                if (online_size > 0)
                {
                        quere_len = get_mpc_quere_len(conn);

                        pthread_mutex_lock(&mutex);
                        if (quere_len > 0)
                        {
                                mpd_run_stop(conn);
                                mpd_run_clear(conn);
                        }

                        bool mpc_load = mpd_run_load(conn, "online.lst");
                        bool mpc_list_clear = mpd_run_playlist_clear(conn, "online.lst");
                        printf("nd btn3_short_press mpc_load: %d, mpc_list_clear: %d\n", mpc_load, mpc_list_clear);
                        pthread_mutex_unlock(&mutex);

                        res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
                        if (res < 0)
                        {
                                printf("nd btn3_short_press i2c_smbus_write_byte_data BLUE_OFF failed, ret: %d\n", res);
                        }
                }

                quere_len = get_mpc_quere_len(conn);
                if (quere_len > 0)
                {
                        printf("nd btn3_short_press get_mpc_quere_len quere_len: %d\n", quere_len);
                        pthread_mutex_lock(&mutex);
                        status = mpd_run_status(conn);
                        if (!status)
                        {
                                printf("nd btn3_short_press mpd_run_status2 %s \n", mpd_connection_get_error_message(conn));
                        }
                        else
                        {
                                if (mpd_status_get_state(status) == MPD_STATE_PLAY)
                                {
                                        printf("nd btn3_short_press mpd_status_get_state MPD_STATE_PLAY\n");
                                        mpd_run_pause(conn, true);
                                }
                                else
                                {
                                        printf("nd btn3_short_press mpd_status_get_state other state\n");
                                        mpd_run_play(conn);
                                }

                                mpd_status_free(status);
                        }
                        pthread_mutex_unlock(&mutex);
                }
  }
}

/*btn3_long_press*/
void btn3_long_press(void)
{
        ncurrt_time = 0;
}

/*btn4_short_press*/
void btn4_short_press(void)
{
        ncurrt_time = 0;
        pthread_mutex_lock(&mutex);
        mpd_run_next(conn);
        pthread_mutex_unlock(&mutex);
}

/*btn4_long_press*/
void btn4_long_press(void)
{
        ncurrt_time = 0;
        system("killall aplay");
        system("killall xfchat");

        if (nmpc_list >= 7)
        {
                nmpc_list = 0;
        }
        else
        {
                nmpc_list++;
        }
        char* p = playlists[nmpc_list];

        pthread_mutex_lock(&mutex);
        mpd_run_clear(conn);
        mpd_run_load(conn, p);
        mpd_run_play(conn);
        pthread_mutex_unlock(&mutex);
}

/*btn5_short_press*/
void btn5_short_press(void)
{
        int res = 0;

        ncurrt_time = 0;
        bRecord = false;
        res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
        if (res < 0)
        {
                printf("nd BTN5_SHORT_PRESS i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
        }
        sck_write_xfchat_fifo();
}

/*btn5_long_press*/
void btn5_long_press(void)
{
        int res = 0;

        ncurrt_time = 0;
        bRecord = false;
        res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
        if (res < 0)
        {
                printf("nd BTN5_LONG_PRESS i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", res);
        }
        sck_write_xfchat_fifo();
}

/*btn5_question_answer_arecord*/
void btn5_question_answer_arecord(int ret)
{
        ncurrt_time = 0;

        if (!bRecord)
        {
                int res = ret;
                printf("nd BTN5_QUESTION_ANSWER_ARECORD: 0x%0*x\n", 2, res);

                char buf[1024] = {0};
                if (read(read_xfchat_fifo, buf, 1024) < 0)
                {
                        perror("nd btn5_question_answer_arecord read erro: ");
                }
                printf("nd btn5_question_answer_arecord read_xfchat_fifo read buf: %s\n", buf);

                my_mpd_run_pause();
                system("killall xfchat");
                system("killall aplay");
                system("xfchat &");

                usleep(1000*150);
                res = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_ON);
                if (res < 0)
                {
                        printf("nd BTN5_QUESTION_ANSWER_ARECORD i2c_smbus_write_byte_data BLUE_ON failed, res: %d\n", res);
                }
                printf("nd BTN5_QUESTION_ANSWER_ARECORD2: 0x%0*x\n", 2, res);
                bRecord = true;
        }
}

/*send_voice*/
void send_voice(const char * contact)
{
        char uri[4096] = {0};
        FILE *fp = NULL;
        //size_t nmber;
        fp = fopen("/tmp/vouri.txt", "r");
        if (!fp)
        {
                perror("open vouri.txt ");
                return;
        }
        fread((void *)&uri[0], 4096, 1, fp);
        fclose(fp);

        uri[strlen(uri) - 1] = '\0';
//      printf("send voice uri:%s\ndevice_id:%s, contact:%s\n", uri, device_id, contact);
        do_send_voice(device_id, uri, contact);

        return;

}

/*do_send_voice*/
int do_send_voice(const char * did, const char * uri, const char * contact)
{
        json_object * pjson_obj_msg = NULL;

        printf("send voice uri 2:%s\n", uri);
        pjson_obj_msg = json_object_new_object();
        printf("send voice uri 3:%s\n", uri);
        json_object_object_add(pjson_obj_msg, "cmd",            json_object_new_string("send_voice"));
        printf("send voice uri 3:%s\n", uri);
        json_object_object_add(pjson_obj_msg, "device_id",      json_object_new_string(did));
        printf("send voice uri 3:%s\n", uri);
        json_object_object_add(pjson_obj_msg, "contact_id",     json_object_new_string(contact));
        json_object_object_add(pjson_obj_msg, "uri",             json_object_new_string(uri));

        printf("send voice uri 2:%s\n", uri);
        printf("msg:%s\n", json_object_to_json_string(pjson_obj_msg));
        mosquitto_publish_send_msg(MQTT_SERVER_TOPIC, strlen(json_object_to_json_string(pjson_obj_msg)), json_object_to_json_string(pjson_obj_msg));

        json_object_put(pjson_obj_msg);
        return 0;

}

/*get_file_size*/
int get_file_size(const char *file_name)
{
        FILE * pFile;
        long size = 0;

        pFile = fopen(file_name,"rb");
        if (pFile == NULL)
        {
                perror("nd get_file_size Error opening file");
        }
        else
        {
                fseek(pFile, 0, SEEK_END);
                size=ftell (pFile);
                fclose(pFile);
        //printf ("Size of file_name: %s size: %ld bytes.\n", file_name, size);
        }
        return size;
}

/*get_mpc_quere_len*/
int get_mpc_quere_len(struct mpd_connection *conn)
{
        int quere_len = 0;
        pthread_mutex_lock(&mutex);
        struct mpd_status *status;
        status = mpd_run_status(conn);
        if (!status)
        {
                printf("nd get_mpc_quere_len erro %s\n", mpd_connection_get_error_message(conn));
                return -1;
        }

        quere_len = mpd_status_get_queue_length(status);

        mpd_status_free(status);
        mpd_response_finish(conn);
        //CHECK_CONNECTION(conn);
        pthread_mutex_unlock(&mutex);

        return quere_len;
}

/*sck_write_xfchat_fifo*/
void sck_write_xfchat_fifo(void)
{
        int ret = 0;
        char ch_xfchat[1024] = {0};
        snprintf(&ch_xfchat[0], 1024, "{\"cmd\":\"xfchat\", \"arecord\":\"stop\"}");
        printf("arecord stop2!\n");
//        ret = sck_write_fifo(xfchat_fifo, ch_xfchat, 1024);
	ret = write(xfchat_fifo, ch_xfchat, 1024);
        if (ret < 0)
        {
                perror("nd sck_write_xfchat_fifo sck_write fifo: ");
        }
        printf("nd sck_write_xfchat_fifo arecord stop1! ch_xfchat: %s\n", ch_xfchat);
}


int set_ncurrt_time_value(int value)
{
	ncurrt_time = value;
	return ncurrt_time;
}

int set_ncurrt_time_add_one(void)
{
	ncurrt_time ++;
	return ncurrt_time;
}

int get_ncurrt_time_value(void)
{
	return ncurrt_time;
}

int set_xfchat_fifo_value(int value)
{
	xfchat_fifo = value;
	return xfchat_fifo;
}

int get_xfchat_fifo_value(void)
{
	return xfchat_fifo;
}

int set_read_xfchat_fifo_value(int value)
{
	read_xfchat_fifo = value;
	return read_xfchat_fifo;
}

int get_read_xfchat_fifo_value(void)
{
	return read_xfchat_fifo;
}

bool set_bplay_audio_value(bool value)
{
	bplay_audio = value;	
	return bplay_audio;
}

bool get_bplay_audio_value(void)
{
	return bplay_audio;
}


/*doworking*/
void select_my_fd(char *device_id)
{
	int ret = 0;
	int event_fd = -1 ;
	int fifo_fd = -1;
	int maxfd = 0;
	int rd = 0;
	char cmd_str[1024] = {0};
	
	fd_set rfds;
	struct input_event ev[64];
	memset(&ev[0], 0, 64*sizeof(struct input_event));
	
	CONTEXT ctx;
	memset(&ctx, 0, sizeof(CONTEXT));
	ctx.mode = MODE_ACTIVE;
	ctx.state = STATE_UNLOGIN;
	ctx.pdata = NULL;
	
	struct timeval tv;
	memset(&tv, 0, sizeof(struct timeval));
	
	event_fd = open("/dev/input/event0", O_RDONLY);
	if (event_fd < 0) 
	{
		perror("open event0 file");
	}

	while(1)
	{
		if (fifo_fd < 0)
		{
			fifo_fd = open("/var/run/comm.fifo", O_RDONLY | O_NONBLOCK);
		}
		
		FD_ZERO(&rfds);
		if (event_fd > 0)
		{
			FD_SET(event_fd, &rfds);
		}
			
		if (fifo_fd > 0)
		{
			FD_SET(fifo_fd, &rfds);
		}
		
		maxfd = MAX(fifo_fd, event_fd);
		//printf("doworking  maxfd: %d\n", maxfd);
		

                ret = select(maxfd + 1, &rfds, NULL, NULL, NULL);

                if (ret < 0)
                {
                        perror("select error");
                        break;
                }
                else if (FD_ISSET(event_fd, &rfds))
                {
                        printf("get event\n");
                        rd = read(event_fd, ev, sizeof(struct input_event) *64);
                        if (rd < (int )sizeof(struct input_event))
                        {
                                perror("reading input_event");
                                continue;
                        }
                        event_action(&ctx, ev, rd / sizeof(struct input_event));
                }
                else if (FD_ISSET(fifo_fd, &rfds))
                {
                        printf("get cmd!\n");
                        memset(cmd_str, 0, 1024);
                        rd = read(fifo_fd, cmd_str, 1024);
                        if (rd < 0)
                        {
                                perror("read command fifo");
                                //continue;     
                        }

                        printf("get cmd:%s from fifo\n", cmd_str);
                        struct json_object * pjson_obj_cmd = NULL;
                        pjson_obj_cmd = json_tokener_parse(cmd_str);
                        parse_cmdstr(pjson_obj_cmd);
                        json_object_put(pjson_obj_cmd);
                        close(fifo_fd);
                        fifo_fd = -1;
                }
        }

        printf("doworking end!\n");
}

/*btn5_click*/
static void btn5_click(CONTEXT *ctx)
{
        struct input_event * ev = (struct input_event *)(ctx->pdata);
        if (ev->value == 1) {
                printf("###pressed btn5_click###\n");
                keys[0].status = KEY_PRESS;
                keys[0].press_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
        } else {
                printf("###released btn5_click###\n");
                keys[0].status = KEY_RELEASE;
                keys[0].release_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
        }
        return;
}

/*btn6_click*/
static void btn6_click(CONTEXT *ctx)
{
        struct input_event * ev = (struct input_event *)(ctx->pdata);
        if (ev->value == 1) {
                printf("###pressed btn6_click###\n");
                keys[1].status = KEY_PRESS;
                keys[1].press_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
                printf("press: press_time:%ld, %ld, keys[1].press_time:%lld\n", ev->time.tv_sec, ev->time.tv_usec, keys[1].press_time);
        } else {
                printf("###released btn6_click###\n");
                keys[1].status = KEY_RELEASE;
                keys[1].release_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
                printf("release: keys[1].release_time:%lld\n", keys[1].release_time);
        }
        return;
}

/*btn7_click*/
static void btn7_click(CONTEXT *ctx)
{
        struct input_event * ev = (struct input_event *)(ctx->pdata);
        if (ev->value == 1) {
                printf("###pressed btn7_click###\n");
                keys[2].status = KEY_PRESS;
                keys[2].press_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
        } else {
                printf("###released btn7_click###\n");
                keys[2].status = KEY_RELEASE;
                keys[2].release_time = ev->time.tv_sec *1000*1000 + ev->time.tv_usec;
        }

        return;
}

/*btn8_click*/
static void btn8_click(CONTEXT *ctx)
{
        printf("###pressed btn8_click###\n");
        return;
}


/*event_action*/
int event_action(CONTEXT *ctx, struct input_event * ev, int count)
{
        int i;

        for (i=0; i < count; i++)
        {
                ctx->pdata = ev + i;
                switch (ev[i].code)
                {
                        case BTN_5:
                                keys[0].func[0](ctx);
                                break;
                        case BTN_6:
                                //keys[1].func[0](ctx);
                                break;
                        case BTN_7:
                                //keys[2].func[0](ctx);
                                break;
                        default:
                                break;
                }
        }

        return 0;
}
