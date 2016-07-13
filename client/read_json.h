#ifndef _READ_JSON_H_
#define _READ_JSON_H_

#define PLAY_VOICE_JSON_PATH                    "/etc/config/play_voice.json"
#define PLAY_VOICES_JSON_KEY_NAME               "play_voices"
#define PLAY_VOICE_JSON_KEY_NAME                "play_voice"
#define PLAY_VOICE_JSON_VOICE_PATH		"/tmp/"
#define PLAY_VOICE_JSON_VOICE_NAME		"dwvoice.amr"

#define DOWNLOAD_JSON_PATH                      "/etc/config/download.json"
#define DOWNLOADS_JSON_KEY_NAME                 "downloads"
#define DOWNLOAD_JSON_KEY_NAME                  "download"
#define DOWNLOAD_JSON_MUSIC_PATH		"/tmp/music/download/"


/*JSON init*/
extern void InitJSON(void);


/*Create the thread of ReadJSONThread*/
extern void* ReadJSONThread(void *arg);

/*Create the thread of DownloadThread*/
extern void* DownloadThread(void *arg);

/*delete_json_object_array0_from_file*/
extern int delete_json_object_array0_from_file(char * path, pthread_rwlock_t * json_rwlock);

/*insert_json_object_to_file*/
extern int insert_json_object_to_file(char * path, pthread_rwlock_t *json_rwlock, json_object * pjson_obj);


/*init_json_object_array_to_file*/
extern int init_json_object_array_to_file(char * path, pthread_rwlock_t * json_rwlock);

extern int curl_download_file_from_url(char * url, char * path, char * file_name);

#endif//_READ_JSON_H_
