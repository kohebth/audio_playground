#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUFFERS 4
#define BUFFER_SIZE 8192

struct audio_buffer {
    uint8_t data[BUFFER_SIZE];
    uint32_t size;
    volatile int ready;
};

struct data {
    struct pw_main_loop *loop;
    struct pw_stream *capture_stream;
    struct pw_stream *playback_stream;
    struct audio_buffer buffers[MAX_BUFFERS];
    volatile int write_idx;
    volatile int read_idx;
};

static void on_capture_process(void *userdata) {
    struct data *d = (struct data *)userdata;
    struct pw_buffer *b;
    struct spa_buffer *buf;
    int idx;

    if ((b = pw_stream_dequeue_buffer(d->capture_stream)) == NULL) {
        return;
    }

    buf = b->buffer;
    if (buf->datas[0].data != NULL && buf->datas[0].chunk->size > 0) {
        idx = d->write_idx;
        uint32_t size = buf->datas[0].chunk->size;

        if (size > BUFFER_SIZE)
            size = BUFFER_SIZE;

        // Write to buffer if not in use
        if (!d->buffers[idx].ready) {
            memcpy(d->buffers[idx].data, buf->datas[0].data, size);
            d->buffers[idx].size = size;
            __sync_synchronize();  // Memory barrier
            d->buffers[idx].ready = 1;
            d->write_idx = (idx + 1) % MAX_BUFFERS;
        }
    }

    pw_stream_queue_buffer(d->capture_stream, b);
}

static void on_playback_process(void *userdata) {
    struct data *d = (struct data *)userdata;
    struct pw_buffer *b;
    struct spa_buffer *buf;
    int idx;

    if ((b = pw_stream_dequeue_buffer(d->playback_stream)) == NULL) {
        return;
    }

    buf = b->buffer;
    if (buf->datas[0].data != NULL) {
        idx = d->read_idx;

        // Read from buffer if ready
        if (d->buffers[idx].ready) {
            uint32_t size = d->buffers[idx].size;
            if (size > buf->datas[0].maxsize)
                size = buf->datas[0].maxsize;

            memcpy(buf->datas[0].data, d->buffers[idx].data, size);
            buf->datas[0].chunk->size = size;
            buf->datas[0].chunk->stride = 4;

            __sync_synchronize();  // Memory barrier
            d->buffers[idx].ready = 0;
            d->read_idx = (idx + 1) % MAX_BUFFERS;
        } else {
            // Output silence if no data available
            memset(buf->datas[0].data, 0, buf->datas[0].maxsize);
            buf->datas[0].chunk->size = buf->datas[0].maxsize;
            buf->datas[0].chunk->stride = 4;
        }
    }

    pw_stream_queue_buffer(d->playback_stream, b);
}

static const struct pw_stream_events capture_events = {
    PW_VERSION_STREAM_EVENTS,
    .process = on_capture_process,
};

static const struct pw_stream_events playback_events = {
    PW_VERSION_STREAM_EVENTS,
    .process = on_playback_process,
};

static void do_quit(void *userdata, int signal_number) {
    struct data *d = (struct data *)userdata;
    pw_main_loop_quit(d->loop);
}

int main(int argc, char *argv[]) {
    struct data data;
    const struct spa_pod *params[1];
    uint8_t buffer[1024];
    struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    struct spa_audio_info_raw audio_info;

    memset(&data, 0, sizeof(data));

    pw_init(&argc, &argv);

    data.loop = pw_main_loop_new(NULL);

    pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGINT, do_quit, &data);
    pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGTERM, do_quit, &data);

    // Create capture stream (microphone)
    data.capture_stream = pw_stream_new_simple(
        pw_main_loop_get_loop(data.loop),
        "audio-capture",
        pw_properties_new(
            PW_KEY_MEDIA_TYPE, "Audio",
            PW_KEY_MEDIA_CATEGORY, "Capture",
            PW_KEY_MEDIA_ROLE, "Music",
            NULL
        ),
        &capture_events,
        &data
    );

    // Create playback stream (speaker)
    data.playback_stream = pw_stream_new_simple(
        pw_main_loop_get_loop(data.loop),
        "audio-playback",
        pw_properties_new(
            PW_KEY_MEDIA_TYPE, "Audio",
            PW_KEY_MEDIA_CATEGORY, "Playback",
            PW_KEY_MEDIA_ROLE, "Music",
            NULL
        ),
        &playback_events,
        &data
    );

    // Set audio format: 48kHz, 16-bit, 2 channels (stereo)
    audio_info.format = SPA_AUDIO_FORMAT_S16;
    audio_info.channels = 2;
    audio_info.rate = 48000;

    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &audio_info);

    // Connect capture stream
    pw_stream_connect(data.capture_stream,
        PW_DIRECTION_INPUT,
        PW_ID_ANY,
        (enum pw_stream_flags)(
            PW_STREAM_FLAG_AUTOCONNECT |
            PW_STREAM_FLAG_MAP_BUFFERS |
            PW_STREAM_FLAG_RT_PROCESS
        ),
        params, 1);

    // Rebuild params for playback stream
    b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &audio_info);

    // Connect playback stream
    pw_stream_connect(data.playback_stream,
        PW_DIRECTION_OUTPUT,
        PW_ID_ANY,
        (enum pw_stream_flags)(
            PW_STREAM_FLAG_AUTOCONNECT |
            PW_STREAM_FLAG_MAP_BUFFERS |
            PW_STREAM_FLAG_RT_PROCESS
        ),
        params, 1);

    printf("Real-time mic to speaker forwarding. Press Ctrl+C to stop.\n");
    printf("Warning: This may cause feedback! Keep volume low or use headphones.\n");
    pw_main_loop_run(data.loop);

    pw_stream_destroy(data.capture_stream);
    pw_stream_destroy(data.playback_stream);
    pw_main_loop_destroy(data.loop);
    pw_deinit();
    
    return 0;
}