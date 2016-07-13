#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <json_object.h>
#include <connection.h>
#include <status.h>
#include <player.h>
#include <i2c-dev.h>

#include "init_device.h"
#include "read_json.h"
#include "i2c_button.h"

struct mpd_connection *conn = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern int i2c_file;

/*sleep*/
bool bvoice_vod = false;

/*Create the thread of button*/
int nSleepDevice = -1;
pthread_t pthdSleepDevice;

/*Init_device*/
int Init_device(void)
{
	/*JSON init*/
        InitJSON();

	/*MPC init*/
        InitMPC();

        /*Init I2CDev*/
        InitI2CDev();

        /*Close Light*/
        CloseLight();

	/*Create the thread of SleepDevice*/
        nSleepDevice = pthread_create(&pthdSleepDevice, NULL, SleepDevice, NULL);

        /*InitFifo*/
        InitFifo();

        /*init_gpio7*/
        init_gpio7();

	return 0;
}


/*MPC init*/
void InitMPC(void)
{
        FILE * pFile;
        pFile = fopen (ONLINE_LIST_PATH,"wb");
        if (pFile==NULL)
        {
                perror ("nd InitMPC create online.lst.m3u Error\n");
        }
        fclose(pFile);

        new_connection(&conn);

//	printf("nd addsong: conn: 0x%x\n", conn);
}

/*new_connection*/
int new_connection(struct mpd_connection **conn)
{
        *conn = mpd_connection_new(NULL, 0, 30000);
        if (*conn == NULL)
        {
                printf("nd new_connection %s\n", "Out of memory");
                return -1;
        }

        if (mpd_connection_get_error(*conn) != MPD_ERROR_SUCCESS)
        {
                printf("nd new_connection %s\n", mpd_connection_get_error_message(*conn));
                mpd_connection_free(*conn);
                *conn = NULL;
                return -1;
        }
        return 0;
}

/*my_mpd_run_pause*/
void my_mpd_run_pause(void)
{
        struct mpd_status *status = NULL;

        pthread_mutex_lock(&mutex);
        status = mpd_run_status(conn);
        if (!status)
        {
                printf("nd my_mpd_run_pause mpd_run_status1 %s \n", mpd_connection_get_error_message(conn));
        }
        else
        {
                if (mpd_status_get_state(status) == MPD_STATE_PLAY)
                {
                        mpd_run_pause(conn, true);
                }

                mpd_status_free(status);
        }

        pthread_mutex_unlock(&mutex);
}

/*Close Light*/
void CloseLight(void)
{
        int ret = 0;
        ret = i2c_smbus_write_byte_data(i2c_file, 0x00, 0x00);
        if (ret < 0)
        {
                printf("nd CloseLight i2c_smbus_write_byte_data BLUE_ON failed, res: %d\n", ret);
        }
}

/*InitFifo*/
int InitFifo(void)
{
        InitCommFifo();
        InitXfchatFifo();

        return 0;
}

/*init_gpio7*/
int init_gpio7(void)
{
        system("echo 7 > /sys/class/gpio/export");
        system("echo in > /sys/class/gpio/gpio7/direction");

        return 0;
}

int InitCommFifo(void)
{
        if (mkfifo("/var/run/comm.fifo", 0644) < 0)
        {
                perror("nd InitCommFifo mkfifo: ");
        }

        return 0;
}

int InitXfchatFifo(void)
{
	/*fd*/
	int xfchat_fifo_fd;
	int read_xfchat_fifo_fd;
        if (mkfifo(XF_CHAT_FIFO_PATH, 0644) < 0)
        {
                perror("nd InitXfchatFifo mkfifo: ");
                printf("FUNC:%s LINE:%d \n", __FUNCTION__, __LINE__);
        }
#if 1
        read_xfchat_fifo_fd = open(XF_CHAT_FIFO_PATH, O_RDONLY | O_NONBLOCK);
	set_xfchat_fifo_value(read_xfchat_fifo_fd);
        if (read_xfchat_fifo_fd < 0)
        {
                perror("nd InitXfchatFifo open read fifo: ");
                printf("FUNC:%s LINE:%d \n", __FUNCTION__, __LINE__);
        }

        xfchat_fifo_fd = open(XF_CHAT_FIFO_PATH, O_WRONLY | O_NONBLOCK);
	set_xfchat_fifo_value(xfchat_fifo_fd);
        if (xfchat_fifo_fd < 0)
        {
                perror("nd InitXfchatFifo open sck_write fifo: ");
                printf("FUNC:%s LINE:%d \n", __FUNCTION__, __LINE__);
        }
#endif
        return 0;
}

/*Create the thread of SleepDevice*/
void* SleepDevice(void *arg)
{
        while(1)
        {
                int res = 0;

                pthread_mutex_lock(&mutex);
                struct mpd_status *status = NULL;
                status = mpd_run_status(conn);
                if (!status)
                {
                        printf("nd SleepDevice mpd_run_status1 %s \n", mpd_connection_get_error_message(conn));
                        close_connection(conn);
                        new_connection(&conn);
                }
                else
                {
                        if (mpd_status_get_state(status) == MPD_STATE_PLAY/* || bBtnEvent*/)
                        {
				set_ncurrt_time_value(0);
                        }

                        mpd_status_free(status);
                }
                pthread_mutex_unlock(&mutex);

                if (get_ncurrt_time_value() >= SLEEP_DEVICE_TIMES)
                {
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER12, 00);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER13, 00);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER14, 03);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER15, 84);

                        usleep(50*1000);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER7, SHUT_DWON);
                        if (res < 0)
                        {
                                printf("nd SleepDevice i2c_smbus_write_byte_data 0x07, 0x01 failed, res: %d\n", res);
                        }
                }
                else if (bvoice_vod && (get_ncurrt_time_value() == FOREVER_SLEEP_DEVICE_TIMES))
                {
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER12, 00);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER13, 00);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER14, 255);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER15, 255);

                        usleep(50*1000);
                        res = i2c_smbus_write_byte_data(i2c_file, REGISTER7, SHUT_DWON);
                        if (res < 0)
                        {
                                printf("nd SleepDevice i2c_smbus_write_byte_data 0x07, 0x01 failed, res: %d\n", res);
                        }
                }

//		set_ncurrt_time_add_one();
                sleep(1);
        }
}

int close_connection(struct mpd_connection *conn)
{
	if(conn != NULL)
	{
		mpd_connection_free(conn);
		conn = NULL;
	}
	
	return 0;
}

bool set_bvoice_vod_value(bool value)
{
	bvoice_vod = value;
	return bvoice_vod;
}
