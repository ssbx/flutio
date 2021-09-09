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

#include "player.h"
#include "track.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t g_player_mutex;

static Track_T* g_current_track = NULL;
static Track_T* g_next_track = NULL;
static Track_T* g_fading_tracks = NULL;

int
Player_Init()
{
    pthread_mutex_init(&g_player_mutex, NULL);
    return 0;
}

int
Player_Play(char* path)
{
    Track_T *t;
    Track_T *cancel_next = NULL;

    if ((t = Track_Open(path)) == NULL)
        return 1;

    pthread_mutex_lock(&g_player_mutex);

    if (g_current_track)
        Track_Enqueue(&g_fading_tracks, g_current_track);
    g_current_track = t;

    // close cancel_next out of locked code
    cancel_next = g_next_track;

    g_next_track = NULL;

    pthread_mutex_unlock(&g_player_mutex);


    if (cancel_next)
        Track_Close(cancel_next);

    return 0;
}

float*
Player_GetFrames(int want, int* got)
{
    int numFrames = 0;
    float *buff = NULL;
    Track_T *ended_tracks = NULL;

    pthread_mutex_lock(&g_player_mutex);

    while (g_current_track) {
        buff = Track_Read(g_current_track, want, &numFrames);
        if (numFrames) {
            break;
        } else {
            Track_Enqueue(&ended_tracks, g_current_track);
            g_current_track = NULL;
            if (g_next_track) {
                g_current_track = g_next_track;
                g_next_track = NULL;
            }
        }
    }

    pthread_mutex_unlock(&g_player_mutex);

    if (ended_tracks) {
        Track_T *it = ended_tracks;
        while (it) {
            Track_T *next = it->next;
            Track_Close(it);
            it = next;
        }
        ended_tracks = NULL;
    }
    *got = numFrames;
    return buff;
}


int
Player_SetNext(char* path, int natural) {

}

void
Player_Clear()
{
    pthread_mutex_lock(&g_player_mutex);

    if (g_current_track) {
        Track_Close(g_current_track);
        g_current_track = NULL;
    }
    if (g_next_track) {
        Track_Close(g_next_track);
        g_next_track = NULL;
    }
    if (g_fading_tracks) {
        Track_T *it = g_fading_tracks;
        do {
            Track_T *next = it->next;
            Track_Close(it);
            it = next;
        } while (it);
        g_fading_tracks = NULL;
    }

    pthread_mutex_unlock(&g_player_mutex);
}

