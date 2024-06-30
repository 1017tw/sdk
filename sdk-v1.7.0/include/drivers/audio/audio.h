#ifndef __AUDIO_H__
#define __AUDIO_H__

typedef struct audio_frame_buf {
    void*          addr0;
    int            num_samples;
} audio_frame_buf_t;

// TODO: move alloc/free audio_buffer function into audio.c

#endif // __AUDIO_H__
