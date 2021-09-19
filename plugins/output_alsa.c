/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3)The name of the author may not be used to
 *     endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <mpd-ng/plugins/output.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

static pthread_t g_thread;
static int g_thread_initialized;
static int g_thread_is_up;
static int g_thread_request_shutdown;
static int g_rate;
static int g_channels;
static int g_fragment_size;
static int g_fragment_frames;
static int g_latency_us;

static float *g_white_sound;

#define PLUGIN_NAME "alsa output"
#define PLUGIN_REVISION 1.0

int  Alsa_Open(OutputParams_T*);
void Alsa_Close();
int  Alsa_GetRate();
int  Alsa_GetChannels();

void MpdNG_OutputPluginInfo(OutputPluginInfo_T* info)
{
    info->name = PLUGIN_NAME;
    info->revision = PLUGIN_REVISION;
    info->Open = Alsa_Open;
    info->Close = Alsa_Close;
    info->GetRate = Alsa_GetRate;
    info->GetChannels = Alsa_GetChannels;
    info->GetFormFromConfig = NULL;
    info->SetConfigFromForm = NULL;
}

int Alsa_GetRate() {
    return g_rate;
}

int Alsa_GetChannels() {
    return g_channels;
}

void Alsa_Close() {
    g_thread_request_shutdown = 1;
    while (g_thread_is_up) usleep(30000);
}

static void* Alsa_ThreadFun(void*);
int Alsa_Open(OutputParams_T* params)
{
    // for now only stereo at 48000 supported
    g_rate = 48000;
    g_channels = 2;

    // TODO get the optimal fragment size and status of the stream
    float latency_seconds = 0.5;
    g_latency_us = (int) (0.5 * 1000000);
    g_fragment_frames = (int) (latency_seconds * g_rate);
    g_fragment_size = g_fragment_frames * g_channels;
    g_white_sound = malloc(sizeof(float) * g_fragment_size);
    int i;
    for (i=0; i < g_fragment_size; i++) {
        g_white_sound[i] = 0.0f;
    }

    // Launch a alsa output thread, the documented callback
    // mechanism does not work
    g_thread_initialized = 0;
    g_thread_is_up = 0;
    g_thread_request_shutdown = 0;
    if (pthread_create(
            &g_thread,
            NULL,
            Alsa_ThreadFun,
            (void*) NULL))
    {
        fprintf(stderr, "output thread creation error\n");
        exit(1);
    }

    while (g_thread_initialized != 1) usleep(30000);
    if (!g_thread_is_up)
        return 1;

    return 0;
}


static void*
Alsa_ThreadFun(void* d)
{

    // initialize alsa output
    snd_pcm_t* pcm;

    const int mode = SND_PCM_NO_AUTO_RESAMPLE | SND_PCM_NO_SOFTVOL;
    int err = snd_pcm_open(
        &pcm,
        "default",
        SND_PCM_STREAM_PLAYBACK,
        mode);
    if (err < 0)
    {
        fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
        g_thread_initialized = 1;
        return NULL;
    }

    err = snd_pcm_set_params(
        pcm,
        SND_PCM_FORMAT_FLOAT,
        SND_PCM_ACCESS_RW_INTERLEAVED,
        g_channels,
        g_rate,
        0,
        g_latency_us); /* 0.5sec */

    if (err < 0)
    {
        fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
        g_thread_initialized = 1;
        return NULL;
    }

    //
    // initialization sucessfull
    //
    g_thread_is_up = 1;
    g_thread_initialized = 1;
    while (g_thread_request_shutdown != 1) {
        int frames = 0;
        float *buffer = Player_GetFrames(g_fragment_frames, &frames);
        if (!frames) {
            buffer = g_white_sound;
            frames = g_fragment_frames;
        }

        while (frames) {
            snd_pcm_sframes_t wrote = snd_pcm_writei(pcm, buffer, frames);
            if (wrote < 0)
            {
                wrote = snd_pcm_recover(pcm, wrote, 0);
            }
            if (wrote < 0)
            {
                fprintf(stderr, "snd_pcm_writei failed: %s\n", snd_strerror(wrote));
            }
            if (wrote > 0 && wrote < frames)
            {
                fprintf(stderr, "Short write (expected %li, wrote %li)\n",
                        (long)sizeof(frames), wrote);
            }
            frames -= wrote;
            buffer += wrote * g_channels;
        }
    }

    snd_pcm_close(pcm);

    g_thread_is_up = 0;

    return NULL;
}

