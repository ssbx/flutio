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

#ifndef PLAYER_H
#define PLAYER_H

int  Player_Init();
void Player_Clear();
float* Player_GetFrames(int,int*);
int Player_Play(char*);
int Player_SetNext(char*,int);

/*
typedef enum {
    PLAYER_EVENT_STOPED,
    PLAYER_EVENT_STARTOFTRACK,
    PLAYER_EVENT_ERROR,
    PLAYER_EVENT_TRACKRELEASED
} player_event_type_t;

typedef struct _player_event_t player_event_t;
typedef struct _player_event_t {
    player_event_type_t type;
    player_event_t     *next;
    void               *data;
} player_event_t;


int  player_init();
void player_destroy();

int  player_have_events();
player_event_t* player_get_events();
void player_cmd_play(char *fpath);
void player_cmd_setnext(char *fpath, int isnatural);
void player_cmd_stop();
*/

#endif // FPLAYER_H
