#ifndef TRACK_H
#define TRACK_H
#include "inputs.h"

typedef struct _Track_T Track_T;
typedef struct _Track_T {
    Track_T *prev;
    Track_T *next;
    Input_T *input;
} Track_T;

Track_T* Track_Open(char*);
float* Track_Read(Track_T*,int,int*);
void Track_Close(Track_T*);
void Track_Enqueue(Track_T**, Track_T*);


#endif // TRACK_H
