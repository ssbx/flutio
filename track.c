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
