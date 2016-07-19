#ifndef _PARSE_MSG_H_
#define _PARSE_MSG_H_

//parse msg
#define CMD_STR_CMD		"cmd"
#define CMD_STR_DEVICE_ID	"device_id"
#define CMD_STR_LOGIN		"login"
#define CMD_STR_PLAY_MUSIC	"play_music"
#define CMD_STR_PLAY_VOICE	"play_voice"
#define CMD_STR_LEAVE_CONTENTPLAY	"leave_contentplay"
#define CMD_STR_LEAVE_AUDIO	"leave_audio"
#define CMD_STR_SET_VOL		"set_vol"
#define CMD_STR_GET_VOL		"get_vol"
#define CMD_STR_DOWNLOAD	"download"
#define CMD_STR_PAUSE		"pause"
#define CMD_STR_VOLUME		"volume"
#define CMD_STR_STOP		"stop"
#define CMD_STR_POWER_OFF	"power_off"
#define CMD_STR_PLAY_STATUS	"play_status"
#define CMD_STR_DATA		"data"
#define CMD_STR_AUDIOURL	"audiourl"
#define CMD_STR_FILEURL		"fileurl"
#define CMD_STR_VOICE_URI	"voice_uri"
#define CMD_STR_CONTACT_ID	"contact_id"
#define CMD_STR_VALUE		"value"
#define CMD_STR_CODE		"code"
#define CMD_STR_LIST_FILE	"list_file"
#define CMD_STR_OPEN_ID		"open_id"
#define CMD_STR_FILE_TYPE	"file_type"
#define CMD_STR_PAGE_NUM	"page_num"
#define CMD_STR_LIST_FILE_PLAY	"list_file_play"
#define CMD_STR_LIST_FILE_DELETE	"list_file_delete"
#define CMD_STR_XFCHATMUSIC	"xfchatmusic"


#define CMD_STR_RESULT		"result"
#define CMD_STR_OK		"ok"
#define CMD_STR_FAILED		"failed"

//parse cmd
#define CMD_STR_SEND_VOICE	"send_voice"
#define CMD_STR_XFCHAT_MUSIC	"xfchat_music"
#define CMD_STR_STOP		"stop"


/*parse_msg*/
extern int parse_msg(CONTEXT * ctx, json_object * pjson_obj_msg_recv);

/*parse_cmd*/
extern int parse_cmdstr(json_object * pjson_obj_cmd);

/*NdInforThread*/
extern void* NdInforThread(void *arg);

extern void * BatteryCheckThread(void * arg);

extern int send_set_volume_result(const char *device_id, const char *cmd, const char *result, const char *volume);

extern int do_set_volume(const char *did, const char *vol);

/*send_get_vol_result*/
extern int send_get_vol_result(const char * device_id, const char * cmd, const char * volume);

/*do_getcontacts*/
extern int do_getcontacts(const char *device_id);

/*send_battery_indicator_result*/
extern int send_battery_indicator_result(const char * device_id, const char * cmd, int n_value);

extern void do_parse_msg_play_music(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_login(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_play_voice(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_leave_contentplay(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_leave_audio(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_set_vol(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_pause(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_stop(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_power_off(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_play_status(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_list_file(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_list_file_play(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_list_file_delete(json_object * pjson_obj_msg_recv, char * did_str);

extern void do_parse_msg_xfchatmusic(json_object * pjson_obj_msg_recv, char * did_str);

extern void * LoginThread(void * arg);

extern int get_bLogin(void);

extern int set_bLogin(bool value);
/*do_play_music*/
extern int do_play_music(const char * device_id, const char *uri);

/*send_respone*/
extern int send_respone(const char * device_id, const char * cmd, const char * result, const char * error_code, const char * uri);

#endif//_PARSE_MSG_H_
