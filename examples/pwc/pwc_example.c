#include "sys/syslog.h"
#include "pwc.h"

#include "FreeRTOS_POSIX/pthread.h"
#include "FreeRTOS_POSIX/unistd.h"
#include "FreeRTOS_POSIX/mqueue.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/fcntl.h"
#include "FreeRTOS_POSIX/errno.h"
#include "os_startup.h"

#include "pwm.h"

static const char* TAG = "pwc_example";
static pthread_t tid1;

uint32_t pwc_dev[PWC_DEVICE_MAX] = {0,1,2,3,4,5};

int pwc_callback_test(void *param)
{
    int num = *(uint32_t *)param;
    LOGE(__func__, "pwc[%d],call back test\n",num);
    // pwc_disable(num);
    return 0;
}

static void* thread1_entry(void* parameter)
{
    (void)parameter;

    pwc_device_number_t pwc_dev_num = PWC_DEVICE_4;

    /* You need connect pwm1 with PWC4 port pin */
    //  init PWM1 for test pwc
    pwm_device_number_t IO_num_rwhl_dir_pwm = PWM_DEVICE_1;
    pwm_init(IO_num_rwhl_dir_pwm);
    pwm_set_frequency(IO_num_rwhl_dir_pwm, 10*1000, 0.5f);
    pwm_set_enable(IO_num_rwhl_dir_pwm, 1);

    // pwc test.
    pwc_init(pwc_dev_num);
    pwc_set_system_clock(pwc_dev_num, 20 * 1000);
    pwc_set_threshold(pwc_dev_num, 50*1000);
    pwc_irq_register(pwc_dev_num, pwc_callback_test, &pwc_dev[pwc_dev_num], PWC_INTR_PRI);
    pwc_enable(pwc_dev_num);

    while(1) {
        LOGI(TAG, "pwc cnt: %d .", pwc_get_count(pwc_dev_num));
        sleep(1);
    }

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

    LOGI(TAG, "AIVA PWC EXAMPLE");

    /* Register custom entry */
    os_app_set_custom_entry(my_app_entry);

    /* Start scheduler, dead loop */
    os_app_main_entry();

    return 0;
}
