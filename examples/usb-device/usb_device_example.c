#include "sys/syslog.h"
#include "os_startup.h"
#include "os_shell_app.h"
#include "os_init_app.h"
#include "fs.h"
#include "flash_part.h"
#include "udevice.h"
#include "usb_func.h"

#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/unistd.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/fcntl.h"
#include "FreeRTOS_POSIX/errno.h"
#include "os_startup.h"


static const char* TAG = "usb_example";
static pthread_t tid1;

static int usb_test()
{
    int ret = 0;
    LOGD(__func__, "in...");
    struct udevice *udev = NULL;
    const char *usb_gadget_name = "lpbk_gadget";
    
    ret = device_find_by_id_name(UCLASS_USB_GARGET, usb_gadget_name, &udev);
    if (ret < 0) { 
        LOGE(__func__, "find usb gadget device(%s) error!", usb_gadget_name);
        return ret;
    }

    if (usb_device_init(udev->driver) != 0) {
        LOGE(__func__, "usb func init fail\n");
        return -1;
    }
    return 0;
}

static void* thread1_entry(void* parameter)
{
    (void)parameter;

    usb_test();
    
    return NULL;
}

static int my_app_entry(void)
{
    int result;

    result = pthread_create(&tid1, NULL, thread1_entry, NULL);
    if (result < 0) {
        LOGE(__func__, "Create tid1 fail!");
    }

    return result;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    LOGI(TAG, "AIVA USB EXAMPLE");
    
    /* Mount file system */
    os_mount_fs();

    /* Start shell */
    os_shell_app_entry();

    /* Register custom entry */
    os_app_set_custom_entry(my_app_entry);

    /* Start scheduler, dead loop */
    os_app_main_entry();

    return 0;
}
