#ifndef _I2C_BUTTON_
#define _I2C_BUTTON_

#include <connection.h>

typedef struct _CONTEXT{
        int mode;
        int state;
        void *pdata;
}CONTEXT;

#define SET_VOL_MIN                     75
#define SET_VOL_ULTIPLE                 5


#define ONLINE_LIST_PATH                "/etc/config/.mpd/playlists/online.lst.m3u"

#define MAX(a, b) ((a) > (b) ? (a):(b))

/*sleep*/
#define SLEEP_DEVICE_TIMES              (60*15)
#define FOREVER_SLEEP_DEVICE_TIMES      (60*5)


/*button*/
#define MAX_BTNS                        4
#define KEY_IDLE                        0
#define KEY_PRESS                       1
#define KEY_RELEASE                     2
#define BTN_ACTIONS                     1

#define NOT_BTN_EVENT                   0x00
#define BTN1_AUDIO_SPPEK_ARECORD        0x10
#define BTN1_SHORT_PRESS                0x11
#define BTN1_LONG_PRESS                 0x12
#define BTN2_SHORT_PRESS                0x21
#define BTN2_LONG_PRESS                 0x22
#define BTN3_SHORT_PRESS                0X31
#define BTN3_LONG_PRESS                 0x32
#define BTN4_SHORT_PRESS                0x41
#define BTN4_LONG_PRESS                 0x42
#define BTN5_QUESTION_ANSWER_ARECORD    0x50
#define BTN5_SHORT_PRESS                0x51
#define BTN5_LONG_PRESS                 0x52

/*light control*/
#define BLUE_OFF        0x10
#define BLUE_ON         0x11
#define BLUE_BLIHK      0x12
#define BLUE_BREATH     0x14
#define GREEN_OFF       0x20
#define GREEN_ON        0x21
#define GREEN_BLIHK     0x22
#define RED_OFF         0x40
#define RED_ON          0x41
#define RED_BLIHK       0x42

/*pen*/
//#define PEN_SUPPORT
#define PEN_START       0x61

/*register*/
#define REGISTER1       0x01
#define REGISTER2       0x02
#define REGISTER3       0x03
#define REGISTER4       0x04
#define REGISTER7       0x07
#define REGISTER8       0x08
#define REGISTER9       0x09
#define REGISTER10      0x0A
#define REGISTER12      0x0C
#define REGISTER13      0x0D
#define REGISTER14      0x0E
#define REGISTER15      0x0F

/*?????*/
#define SWITCH_BTN_SHUT_DWON    0x00
#define SHUT_DWON               0x01
#define AUTO_SLEEP              0xFF
#define AUTO_SLEEP_TIMES        0x0F//0x0F
#define FOREVER_SLEEP_TIMES     0xFFF
#define SHUT_DOWN_SLEEP_TIMES   0X3C

/***********************/
typedef enum _WORK_MODE
{
        MODE_SLEEP,
        MODE_ACTIVE
}WORK_MODE;

typedef enum _ACTIVE_STATE
{
        STATE_UNLOGIN = 0,
        STATE_LOGINED,
        //STATE_RECV_COMMAND,
        //STATE_WECHAT_VOICE,
        //STATE_PLAYING_CLOUD,
        //STATE_PLAYING_LOCAL,
        //STATE_PLAYING_VOICE,
        //STATE_RECORDING,
        //STATE_STANDBY
}ACTIVE_STATE;


typedef void (*event_func) (CONTEXT *ctx);

typedef struct _KEY_DESC
{
        struct input_event ev;
        event_func func[BTN_ACTIONS];
        long long press_time;
        long long release_time;
        int status;
}KEY_DESC;


/*Init I2CDev*/
extern int InitI2CDev(void);

/*The thread of I2C*/
extern void * I2CThread(void *arg);

/*btn1_short_press*/
extern void btn1_short_press(void);

/*btn1_long_press*/
extern void btn1_long_press(void);

/*btn1_audio_sppek_arecord*/
extern void btn1_audio_sppek_arecord(int ret);

/*btn2_short_press*/
extern void btn2_short_press(void);

/*btn2_long_press*/
extern void btn2_long_press(void);

/*btn3_short_press*/
extern void btn3_short_press(void);

/*btn3_long_press*/
extern void btn3_long_press(void);

/*btn4_short_press*/
extern void btn4_short_press(void);

/*btn4_long_press*/
extern void btn4_long_press(void);

/*btn5_short_press*/
extern void btn5_short_press(void);

/*btn5_long_press*/
extern void btn5_long_press(void);

/*btn5_question_answer_arecord*/
extern void btn5_question_answer_arecord(int ret);

/*send_voice*/
extern void send_voice(const char * contact);

/*do_send_voice*/
extern int do_send_voice(const char * did, const char * uri, const char * contact);

/*get_file_size*/
extern int get_file_size(const char *file_name);

/*get_mpc_quere_len*/
extern int get_mpc_quere_len(struct mpd_connection *conn);

/*sck_write_xfchat_fifo*/
extern void sck_write_xfchat_fifo(void);

extern int set_ncurrt_time_value(int value);

extern int get_ncurrt_time_value(void);

extern int set_ncurrt_time_add_one(void);

extern int set_xfchat_fifo_value(int value);

extern int get_xfchat_fifo_value(void);

extern int set_read_xfchat_fifo_value(int value);

extern int get_read_xfchat_fifo_value(void);

extern bool set_bplay_audio_value(bool value);

extern bool get_bplay_audio_value(void);

extern void select_my_fd(char *device_id);

/*event_action*/
extern int event_action(CONTEXT *ctx, struct input_event * ev, int count);
#endif//_I2C_BUTTON_
