#include "track.h"
#include "inputs.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

Track_T*
Track_Open(char* fname)
{
    fprintf(stderr, "track_open\n");
    Input_T *input = Inputs_Open(fname);
    if (!input) {
        fprintf(stderr, "no input!!!\n");
        return NULL;
    }

    Track_T *t = malloc(sizeof(Track_T));
    t->prev = NULL;
    t->next = NULL;
    t->input = input;
    return t;
}

float* Track_Read(Track_T *t, int want, int *got)
{
    return Inputs_Read(t->input, want, got);
}

void
Track_Enqueue(Track_T **head, Track_T *t)
{

    if ((*head) == NULL) {
        *head = t;
        t->next = t->prev = NULL;
        return;
    }

    Track_T *tail = (*head);
    while (tail->next) {
        tail = tail->next;
    }
    tail->next = t;
    t->prev = tail;

}

void Track_Close(Track_T *t)
{
    Inputs_Close(t->input);
    free(t);
}
