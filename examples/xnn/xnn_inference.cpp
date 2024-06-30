#include "syslog.h"
#include "os_startup.h"
#include "os_init_app.h"
#include "os_shell_app.h"
#include "net.h"
#include "aiva_malloc.h"
#include "sysctl.h"
#include "fs_file.h"
#include "xnn_hal_substract_mean_normalize.h"
#include <stdlib.h>

static xnn::Net m_net;
static const char* xfile_name = "close_eye_detection.xfile";
static const char* input_file_name[] = {"close_eye_32x32_rgba.bin", "open_eye_32x32_rgba.bin"};
static const int input_w = 32;
static const int input_h = 32;
static const int input_c = 4;

static int load_model(const uint8_t* xfile_mem, int len)
{
    int ret;
    ret = m_net.load_xfile(xfile_mem, len);
    if (ret != 0)
    {
        LOGE(__func__, "liveness ir load model fail: addr=0x%x, len=%d", xfile_mem, len);
        return -1;
    }

    return 0;
}

static int run_model(const xnn::Mat &input_img, float &score)
{
    float mean[3];
    float norm[3];

    m_net.get_mean_value(mean);
    m_net.get_norm_value(norm);

    sysctl_clock_enable(SYSCTL_CLOCK_XNN);

    xnn::Mat in;
    if (input_img.dataformat == xnn::Mat::DATA_FORMAT_HWC_BGR) {
        in = xnn::Mat::from_pixels((unsigned char*)input_img.data, xnn::Mat::PIXEL_BGR2RGB, input_img.w, input_img.h);
        in.substract_mean_normalize(mean, norm);
    } else if (input_img.dataformat == xnn::Mat::DATA_FORMAT_HWC_RGBA) {
        dcache_flush((const void*)input_img.data, input_w * input_h * input_c);
        float input_blob_top_scale;
        if (m_net.get_input_blob_top_scale(input_blob_top_scale) != 0) {
            LOGE(__func__, "get input blob top scale fail");
            sysctl_clock_disable(SYSCTL_CLOCK_XNN);
            return -1;
        }
        //LOGI(__func__, "IR input_blob_top_scale: %f", input_blob_top_scale);
        xnn::xnn_hal_substract_mean_normalize((const uint8_t*)input_img.data,
                                              (int)input_img.w * input_c,
                                              (mean[0] + mean[1] + mean[2]) / 3,
                                              (norm[0] + norm[1] + norm[2]) / 3,
                                              input_blob_top_scale,
                                              input_img.w,
                                              input_img.h,
                                              in);
        // Note: the output xnn Mat in is C16N format
    } else {
        LOGE(__func__, "invalid input data format: %d", input_img.dataformat);
        sysctl_clock_disable(SYSCTL_CLOCK_XNN);
        return -1;
    }

    xnn::Extractor ex = m_net.create_extractor();
    ex.input("input", in);

    xnn::Mat output;
    ex.extract("output", output);

    if (output.elemsize != 4) {
        LOGE(__func__, "output elemsize error: %d", output.elemsize);
        sysctl_clock_disable(SYSCTL_CLOCK_XNN);
        return -1;
    }

    float val0 = ((float*)output.data)[0];
    float val1 = ((float*)output.data)[1];
    float max = std::max(val0, val1);
    val0 = std::exp(val0-max);
    val1 = std::exp(val1-max);
    float sum = val0 + val1;
    score = val1 / sum;
    sysctl_clock_disable(SYSCTL_CLOCK_XNN);

    return 0;
}

static uint8_t* file2buf(const char *file)
{
    fs_file_t *fp = fs_file_open(file, FS_O_RDONLY);
    if (!fp) {
        LOGE(__func__, "read file %s failed", file);
        return NULL;
    }

    uint32_t file_size = fs_file_size(fp);

    uint8_t *buf = (uint8_t*)malloc(file_size);
    if (buf == NULL) {
        LOGE(__func__, "malloc failed with size:%u", file_size);
        return NULL;
    }

    fs_file_read(fp, buf, file_size);
    // NOTE: flush cache incase this buffer will be used by hardware
    dcache_flush(buf, file_size);

    if (fp) {
        fs_file_close(fp);
    }

    return buf;
}

static int get_filesize(const char* file)
{
    fs_file_t *fp = fs_file_open(file, FS_O_RDONLY);
    if (!fp) {
        LOGE(__func__, "read file %s failed", file);
        return 0;
    }

    return fs_file_size(fp);
}

static int xnn_inference_entry(void)
{
    int ret = 0;
    uint8_t* xfile = NULL;
    uint8_t* xnn_workbuffer = NULL;

    os_mount_fs();
    //
    os_shell_app_entry();


    do
    {
        xnn::Mat xnn_input;

        // 1. read xnn xfile from file system
        xfile = file2buf(xfile_name);
        if (xfile == NULL) {
            LOGE(__func__, "read xfile failed");
            ret = -1;
            break;
        }
        // 2. load model
        ret = load_model(xfile, get_filesize(xfile_name));
        if (ret) {
            LOGE(__func__, "load model failed");
            ret = -1;
            break;
        }
        // 3. prepare work buffer for this model; only need to provide maximum workbuffer if there are multipy xnn models
        int workbuffer_size = m_net.get_workbuffer_size();
        xnn_workbuffer = (uint8_t*)aiva_malloc_aligned(workbuffer_size, 32);
        // TODO: release old workbuffer when call set_workbuffer
        m_net.set_workbuffer((uint32_t)xnn_workbuffer, workbuffer_size);

        // 3. inference
        for (unsigned int i = 0; i < sizeof(input_file_name)/sizeof(input_file_name[0]); i++)
        {
            const char *file_name = input_file_name[i];
            uint8_t* input_img = file2buf(file_name);
            if (input_img == NULL) {
                LOGE(__func__, "read input_img failed");
                continue;
            }
            // Mat(int w, int h, int c, void* data, size_t elemsize, int packing, DataFormat dataformat, Allocator* allocator = 0);
            xnn::Mat input(input_w, input_h, input_c, input_img, 1u, 1, xnn::Mat::DATA_FORMAT_HWC_RGBA);
            float score = 0;
            ret = run_model(input, score);
            if (ret == 0) {
                LOGI(__func__, "img:%s is eye closed:%d score is:%f", file_name, (int)(score < 0.5), score);
            } else {
                LOGE(__func__, "run model failed");
            }
            if (input_img) {
                free(input_img);
                input_img = NULL;
            }
        }
    } while (0);

    // release buffers
    if (xfile) {
        free(xfile);
        xfile = NULL;
    }

    if (xnn_workbuffer) {
        aiva_free(xnn_workbuffer);
        xnn_workbuffer = NULL;
    }

    return ret;
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    /* Register custom entry */
    os_app_set_custom_entry(xnn_inference_entry);

    /* Start scheduler, dead loop */
    os_app_main_entry();

    return 0;
}
