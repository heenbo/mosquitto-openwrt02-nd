#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
#include <curl/curl.h>

#include "read_json.h"
#include "i2c_button.h"
#include "init_device.h"

extern int i2c_file;
extern pthread_mutex_t mutex;
extern struct mpd_connection *conn;

/*Create the thread of ReadJSONThread*/
int nReadJSON = -1;
pthread_t pthdReadJSON;
pthread_rwlock_t json_rwlock_voice;
pthread_rwlock_t json_rwlock_download;

/*Create the thread of DownloadThread*/
int nDownload = -1;
pthread_t pthdDownload;

/*JSON init*/
void InitJSON(void)
{
        int res;
        if(-1 == access(PLAY_VOICE_JSON_PATH, F_OK))
        {
                //file does not exist, create it
                if(creat(PLAY_VOICE_JSON_PATH, 777) < 0)
                {
                        printf("FUNC: %s LINE: %d :creat json file error", __FUNCTION__, __LINE__);
                        exit(-1);
                }
		init_json_object_array_to_file(PLAY_VOICE_JSON_PATH, &json_rwlock_voice);
        }
        else
        {
                printf("%s exist\n", PLAY_VOICE_JSON_PATH);
        }
        if(-1 == access(DOWNLOAD_JSON_PATH, F_OK))
        {
                //file does not exist, create it
                if(creat(DOWNLOAD_JSON_PATH, 777) < 0)
                {
                        printf("FUNC: %s LINE: %d :creat json file error", __FUNCTION__, __LINE__);
                        exit(-1);
                }
		init_json_object_array_to_file(DOWNLOAD_JSON_PATH, &json_rwlock_download);
        }
        else
        {
                printf("%s exist\n", DOWNLOAD_JSON_PATH);
        }
        res = pthread_rwlock_init(&json_rwlock_voice, NULL);
        if(res != 0)
        {
                perror("voice rwlock initialization failed");
                exit(-1);
        }

        res = pthread_rwlock_init(&json_rwlock_download, NULL);
        if(res != 0)
        {
                perror("download rwlock initialization failed");
                exit(-1);
        }

	/*Create the thread of ReadJSONThread*/
        nReadJSON = pthread_create(&pthdReadJSON, NULL, ReadJSONThread, NULL);

        /*Create the thread of DownloadThread*/
        nDownload = pthread_create(&pthdDownload, NULL, DownloadThread, NULL);
}

/*Create the thread of ReadJSONThread*/
void* ReadJSONThread(void *arg)
{
        int ret = 0;
	int len = 0;
        int online_size = 0;
        json_object * pjson_obj_read = NULL;
        char * url = NULL;

	while(true)
//	while(false)
        {
                pthread_rwlock_wrlock(&json_rwlock_voice);
                pjson_obj_read = json_object_from_file(PLAY_VOICE_JSON_PATH);
                pthread_rwlock_unlock(&json_rwlock_voice);
                ret = json_object_get_type(pjson_obj_read);
		if(json_type_array != ret)
		{
			init_json_object_array_to_file(PLAY_VOICE_JSON_PATH, &json_rwlock_voice);
                	json_object_put(pjson_obj_read);
                	usleep(600*1000);
			continue;
		}
		len = json_object_array_length(pjson_obj_read);
                json_object_put(pjson_obj_read);
		pjson_obj_read = NULL;

                online_size = get_file_size(ONLINE_LIST_PATH);
                if (((json_type_array == ret) && (0 < len) && (!get_bplay_audio_value())) || (online_size > 0))
                {
			
			printf("nd ReadJSONThread i2c_smbus_write_byte_data BLUE_BLIHK\n");
                        ret = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_BLIHK);
                        if (ret < 0)
                        {
                                printf("nd ReadJSONThread i2c_smbus_write_byte_data BLUE_BLIHK failed, res: %d\n", ret);
                        }
	
			set_bvoice_vod_value(true);
                }
                else
                {
			set_bvoice_vod_value(false);
                }

                while(get_bplay_audio_value())
                {

                        pthread_rwlock_wrlock(&json_rwlock_voice);
                        pjson_obj_read = json_object_from_file(PLAY_VOICE_JSON_PATH);
                      	pthread_rwlock_unlock(&json_rwlock_voice);
			len = json_object_array_length(pjson_obj_read);
                       	printf("FUNC:%s LINE:%d url---\n", __FUNCTION__, __LINE__);
                        if (0 < len)
                        {
                                printf("FUNC:%s LINE:%d url---\n", __FUNCTION__, __LINE__);
                                url = NULL;
                                url = (char *)json_object_get_string(json_object_object_get(json_object_array_get_idx(pjson_obj_read, 0), "voice_uri"));
                                printf("FUNC:%s LINE:%d url---\n", __FUNCTION__, __LINE__);
                                if(NULL == url)
                                {
                                        url = (char *)json_object_get_string(json_object_object_get(json_object_array_get_idx(pjson_obj_read, 0), "audiourl"));
                                }
                                printf("FUNC:%s LINE:%d url---\n", __FUNCTION__, __LINE__);
                                printf("url---: %s \n", url);
                                if (0 != strlen(url))
                                {
                                        system("rm /tmp/dwvoice.wav");
					curl_download_file_from_url(url, PLAY_VOICE_JSON_VOICE_PATH, PLAY_VOICE_JSON_VOICE_NAME);
					
                                        //printf("curl---------------\n");
                                        system("amrnb-dec /tmp/dwvoice.amr /tmp/dwvoice.wav");
                                        system("cat /tmp/zero.dat >> /tmp/dwvoice.wav");
                                        system("rm_wav_head      /tmp/dwvoice.wav");
                                        system("aplay -t raw -f S16_LE -r 8000 -c 1 /tmp/dwvoice.raw");
                                        //system("aplay /tmp/dwvoice.wav");
                                }

                                if(0 != delete_json_object_array0_from_file(PLAY_VOICE_JSON_PATH, &json_rwlock_voice))
                                {
                                        printf("delete_json_object_array0_from_file error\n");
                                }

                                ret = i2c_smbus_write_byte_data(i2c_file, 0x00, BLUE_OFF);
                                if (ret < 0)
                                {
                                        printf("nd ReadJSONThread i2c_smbus_write_byte_data BLUE_OFF failed, res: %d\n", ret);
                                }

		                json_object_put(pjson_obj_read);
				pjson_obj_read = NULL;

				set_bplay_audio_value(false);

                        }
                        else
                        {
				set_bplay_audio_value(false);
                        }
                }

                usleep(600*1000);
        }
        return NULL;
}

/*Create the thread of DownloadThread*/
void* DownloadThread(void *arg)
{
        int ret = 0;
        json_object * pjson_obj_read = NULL;
        char * url = NULL;
        char * filename = NULL;
        char MusicName[1024];

        while (1)
        {

                pthread_rwlock_wrlock(&json_rwlock_download);
                pjson_obj_read = json_object_from_file(DOWNLOAD_JSON_PATH);
                pthread_rwlock_unlock(&json_rwlock_download);

                ret = json_object_get_type(pjson_obj_read);
		if(json_type_array != ret)
		{
			init_json_object_array_to_file(DOWNLOAD_JSON_PATH, &json_rwlock_download);
                	json_object_put(pjson_obj_read);
                	usleep(600*1000);
			continue;
		}
//              json_object_put(pjson_obj_read);

                if ((json_type_array == ret) && (0 < json_object_array_length(pjson_obj_read)))
                {
                        if (0 < json_object_array_length(pjson_obj_read))
                        {
                                url = (char *)json_object_get_string(json_object_object_get(json_object_array_get_idx(pjson_obj_read, 0), "url"));
                                filename = (char *)json_object_get_string(json_object_object_get(json_object_array_get_idx(pjson_obj_read, 0), "filename"));
                        }
                }


                if ((ret == json_type_array) && (0 < json_object_array_length(pjson_obj_read)) && (0 != strlen(url)) && (0 != strlen(filename)))
                {
                        sprintf(MusicName, "%s.mp3", filename);
			curl_download_file_from_url(url, DOWNLOAD_JSON_MUSIC_PATH, MusicName);
                        pthread_mutex_lock(&mutex);
                        int ret = mpd_run_update(conn, "download");
                        printf("nd DownloadThread mpd_run_update ret: %d\n", ret);
                        system("rm -r /etc/config/.mpd/playlists/download.lst.m3u");
                        bool b_ret = mpd_run_playlist_add (conn, "download.lst", "download");
                        printf("nd DownloadThread mpd_run_playlist_add b_ret: %d\n", b_ret);
                        pthread_mutex_unlock(&mutex);

                        if(0 != delete_json_object_array0_from_file(DOWNLOAD_JSON_PATH, &json_rwlock_download))
                        {
                                printf("delete_json_object_array0_from_file error\n");
                        }

                        json_object_put(pjson_obj_read);
                        url = NULL;
                        filename = NULL;
                        pjson_obj_read = NULL;

			set_ncurrt_time_value(0);
                }

                sleep(1);
        }
}

/*delete_json_object_array0_from_file*/
int delete_json_object_array0_from_file(char * path, pthread_rwlock_t * json_rwlock)
{
        json_object * pjson_obj_read = NULL;
        json_object * pjson_obj_write = NULL;
        int i = 0;

        pthread_rwlock_wrlock(json_rwlock);
        pjson_obj_read = json_object_from_file(path);
        if((json_type_array == json_object_get_type(pjson_obj_read)) && (0 < json_object_array_length(pjson_obj_read)))
        {
                pjson_obj_write = json_object_new_array();
                for(i = 1; i < json_object_array_length(pjson_obj_read); i++)
                {
                        json_object_array_add(pjson_obj_write, json_object_array_get_idx(pjson_obj_read, i));
                }
                json_object_to_file(path, pjson_obj_write);
        }
        else
        {
                return -1;
        }
        pthread_rwlock_unlock(json_rwlock);
        json_object_put(pjson_obj_read);
        json_object_put(pjson_obj_write);

        return 0;
}

/*insert_json_object_to_file*/
int insert_json_object_to_file(char * path, pthread_rwlock_t *json_rwlock, json_object * pjson_obj)
{
        json_object * pjson_obj_read = NULL;

        pthread_rwlock_wrlock(json_rwlock);
        pjson_obj_read = json_object_from_file(path);
        if(json_type_array == json_object_get_type(pjson_obj_read))
        {
                json_object_array_add(pjson_obj_read, pjson_obj);
        }
        else
        {
                pjson_obj_read = json_object_new_array();
                json_object_array_add(pjson_obj_read, pjson_obj);
        }
        json_object_to_file(path, pjson_obj_read);
        pthread_rwlock_unlock(json_rwlock);
        json_object_put(pjson_obj_read);
        printf("test  FUNC: %s LINE: %d--------------------------\n", __FUNCTION__, __LINE__);

        return 0;
}

/*init_json_object_array_to_file*/
int init_json_object_array_to_file(char * path, pthread_rwlock_t * json_rwlock)
{
	json_object * pjson_obj_array = NULL;

	pjson_obj_array = json_object_new_array();
        pthread_rwlock_wrlock(json_rwlock);
        json_object_to_file(path, pjson_obj_array);
        pthread_rwlock_unlock(json_rwlock);
        json_object_put(pjson_obj_array);
	return 0;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
        size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
        return written;
}

int curl_download_file_from_url(char * url, char * path, char * file_name)
{
        CURL *curl_handle;
        FILE *pfile;
        int len = 0;
        char * save_file_path = NULL;

        len = strlen(path) + strlen(file_name);
        save_file_path = malloc(len*sizeof(char));
        if(NULL == save_file_path)
        {
                printf("error save_file_path malloc failure\n");
                return -1;
        }
        strcpy(save_file_path, path);
        strcat(save_file_path, file_name);
        printf("save_file_path:%s\n", save_file_path);

        curl_global_init(CURL_GLOBAL_ALL);
        /* init the curl session */
        curl_handle = curl_easy_init();
        /* set URL to get here */
        curl_easy_setopt(curl_handle, CURLOPT_URL, url);
        /* Switch on full protocol/debug output while testing */
        curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
        /* disable progress meter, set to 0L to enable and disable debug output */
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        /* open the file */
        pfile = fopen(save_file_path, "wb");
        if(pfile)
        {
                /* write the page body to this file handle */
                curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pfile);
                /* get it! */
                curl_easy_perform(curl_handle);
                /* close the header file */
                fclose(pfile);
        }

        free(save_file_path);
        /* cleanup curl stuff */
        curl_easy_cleanup(curl_handle);

        return 0;
}

