#include <pipewire/pipewire.h>
#include <spa/param/audio/format-utils.h>
#include <cstdio>
#include <csignal>
#include <cstring>
// #include <buffer.h>

#define MAX_BUFFERS 4
#define BUFFER_SIZE 2048

struct audio_buffer {
    uint16_t data[BUFFER_SIZE];
    uint16_t size;
    volatile int ready;
};

struct node_buffer {
    audio_buffer chunks[MAX_BUFFERS];
    volatile uint8_t write_idx;
    volatile uint8_t read_idx;
};

struct node {
    pw_main_loop *loop;
    pw_stream *stream;
    node_buffer *buffer;
};

inline bool is_not_null(void *ptr) {
    return ptr != nullptr;
}

static void on_capture_process(void *userdata) {
    auto node = static_cast<::node *>(userdata);
    pw_buffer *b;

    if ((b = pw_stream_dequeue_buffer(node->stream)) == nullptr) {
        return;
    }

    const spa_buffer *buf = b->buffer;

    if (is_not_null(buf->datas[0].data) && buf->datas[0].chunk->size > 0) {
        uint16_t size = buf->datas[0].chunk->size;
        volatile uint8_t *idx = &node->buffer->write_idx;
        audio_buffer *chunk = &node->buffer->chunks[*idx];

        if (size > BUFFER_SIZE) size = BUFFER_SIZE;

        // Write to buffer if not in use
        // if (!chunk->ready) {
            memcpy(chunk->data, buf->datas[0].data, size);
            chunk->size = size;
            __sync_synchronize(); // Memory barrier
            chunk->ready = 1;
            *idx = (*idx + 1) & (MAX_BUFFERS - 1);
        // }
    }

    pw_stream_queue_buffer(node->stream, b);
}

static void on_playback_process(void *userdata) {
    const auto node = static_cast<::node *>(userdata);
    pw_buffer *b;

    if ((b = pw_stream_dequeue_buffer(node->stream)) == nullptr) {
        return;
    }

    const spa_buffer *buf = b->buffer;

    if (is_not_null(buf->datas[0].data)) {
        volatile uint8_t *idx = &node->buffer->read_idx;
        audio_buffer *chunk = &node->buffer->chunks[*idx];

        // Read from buffer if ready
        if (chunk->ready) {
            uint16_t size = chunk->size;

            if (size > buf->datas[0].maxsize)
                size = buf->datas[0].maxsize;

            memcpy(buf->datas[0].data, chunk->data, size);
            buf->datas[0].chunk->size = size;
            buf->datas[0].chunk->stride = 4;

            __sync_synchronize(); // Memory barrier
            chunk->ready = 0;
            *idx = (*idx + 1) & (MAX_BUFFERS - 1);
        } else {
            // Output silence if no data available
            memset(buf->datas[0].data, 0, buf->datas[0].maxsize);
            buf->datas[0].chunk->size = buf->datas[0].maxsize;
            buf->datas[0].chunk->stride = 4;
        }
    }

    pw_stream_queue_buffer(node->stream, b);
}

static const pw_stream_events capture_events = {
    PW_VERSION_STREAM_EVENTS,
    .process = on_capture_process,
};

static const pw_stream_events playback_events = {
    PW_VERSION_STREAM_EVENTS,
    .process = on_playback_process,
};

static void do_quit(void *loop, int signal_number) {
    pw_main_loop_quit(static_cast<pw_main_loop *>(loop));
}

int main(int argc, char *argv[]) {
    const spa_pod *params[1];
    uint8_t buffer[1024];
    spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    pw_init(&argc, &argv);
    pw_main_loop *loop = pw_main_loop_new(nullptr);
    pw_loop_add_signal(pw_main_loop_get_loop(loop), SIGINT, do_quit, &loop);
    pw_loop_add_signal(pw_main_loop_get_loop(loop), SIGTERM, do_quit, &loop);

    node_buffer io_buffer = {.chunks = {0}, .write_idx = 0, .read_idx = 0 };

    // Create capture stream (microphone)
    node capture = {
        .loop = loop,
        .stream = pw_stream_new_simple(
            pw_main_loop_get_loop(loop),
            "n!audio-capture",
            pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Audio",
                PW_KEY_MEDIA_CATEGORY, "Capture",
                PW_KEY_MEDIA_ROLE, "Music",
                NULL
            ),
            &capture_events,
            &capture
        ),
        .buffer = &io_buffer
    };

    // Create playback stream (speaker)
    node playback = {
        .loop = loop,
        .stream = pw_stream_new_simple(
            pw_main_loop_get_loop(loop),
            "n!audio-playback",
            pw_properties_new(
                PW_KEY_MEDIA_TYPE, "Audio",
                PW_KEY_MEDIA_CATEGORY, "Playback",
                PW_KEY_MEDIA_ROLE, "Music",
                NULL
            ),
            &playback_events,
            &playback
        ),
        .buffer = &io_buffer
    };

    // Set audio format: 48kHz, 16-bit, 2 channels (stereo)
    spa_audio_info_raw audio_info = {
        .format = SPA_AUDIO_FORMAT_S16,
        .rate = 48000,
        .channels = 2,
    };

    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &audio_info);

    // Connect capture stream
    pw_stream_connect(
        capture.stream,
        PW_DIRECTION_INPUT,
        PW_ID_ANY,
        (enum pw_stream_flags) (
            PW_STREAM_FLAG_AUTOCONNECT |
            PW_STREAM_FLAG_MAP_BUFFERS |
            PW_STREAM_FLAG_RT_PROCESS
        ),
        params, 1);

    // Rebuild params for playback stream
    b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
    params[0] = spa_format_audio_raw_build(&b, SPA_PARAM_EnumFormat, &audio_info);

    // Connect playback stream
    pw_stream_connect(
        playback.stream,
        PW_DIRECTION_OUTPUT,
        PW_ID_ANY,
        (enum pw_stream_flags) (
            PW_STREAM_FLAG_AUTOCONNECT |
            PW_STREAM_FLAG_MAP_BUFFERS |
            PW_STREAM_FLAG_RT_PROCESS
        ),
        params, 1);


    printf("Real-time mic to speaker forwarding. Press Ctrl+C to stop.\n");
    printf("Warning: This may cause feedback! Keep volume low or use headphones.\n");
    pw_main_loop_run(loop);

    pw_stream_destroy(capture.stream);
    pw_stream_destroy(playback.stream);
    pw_main_loop_destroy(loop);
    pw_deinit();

    return 0;
}
