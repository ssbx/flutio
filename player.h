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
