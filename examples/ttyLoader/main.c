#include "sys/syslog.h"
#include "os_startup.h"
#include "os_shell_app.h"
#include "os_init_app.h"
#include "fs.h"
#include "spi.h"
#include "flash_part.h"

#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/unistd.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/fcntl.h"
#include "FreeRTOS_POSIX/errno.h"
#include "gpio.h"

static pthread_t tid;

static void* main_thread(void* parameter)
{
    (void)parameter;

    while (1) {
        sleep(2);
          LOGE(__func__, "Create tid2 fail!");
    }

    return NULL;
}

int my_app_entry(void)
{
    int result;
 
    /* Mount file system */
    os_mount_fs();

    /* Start shell */
    os_shell_app_entry();

    result = pthread_create(&tid, NULL, main_thread, NULL);
    if (result < 0) {
        LOGE(__func__, "Create tid fail!");
    }

    gpio_pin_t power_latch_pin = GPIOB_PIN9;
    gpio_set_drive_mode(power_latch_pin, GPIO_DM_OUTPUT);
    gpio_set_pin(power_latch_pin, GPIO_PV_HIGH);

    return result;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    /* Register custom entry */
    os_app_set_custom_entry(my_app_entry);

    /* Start scheduler, dead loop */
    os_app_main_entry();

    return 0;
}
