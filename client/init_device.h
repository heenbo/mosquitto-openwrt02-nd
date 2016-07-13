#ifndef __INIT_DEVICE_H_
#define __INIT_DEVICE_H_

#include "connection.h"

#define XF_CHAT_FIFO_PATH                 	"/tmp/xfchat.fifo"

#define ONLINE_LIST_PATH                    	"/etc/config/.mpd/playlists/online.lst.m3u"

/*Init_device*/
extern int Init_device(void);

/*MPC init*/
extern void InitMPC(void);

/*Close Light*/
extern void CloseLight(void);

/*my_mpd_run_pause*/
extern void my_mpd_run_pause(void);

/*new_connection*/
extern int new_connection(struct mpd_connection **conn);

/*InitFifo*/
extern int InitFifo(void);

/*init_gpio7*/
extern int init_gpio7(void);

extern int InitCommFifo(void);

extern int InitXfchatFifo(void);

/*Create the thread of SleepDevice*/
extern void* SleepDevice(void *arg);

extern int close_connection(struct mpd_connection *conn);

extern bool set_bvoice_vod_value(bool value);

#endif// __INIT_DEVICE_H_
