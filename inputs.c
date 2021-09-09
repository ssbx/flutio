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

#include "inputs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

InputPlugin_T *G_Inputs_plugins = NULL;

int
Inputs_Load(char* path, void* handle)
{
    char *error;

    InputPlugin_Info_T info;
    void (*input_info)(InputPlugin_Info_T *info);

    /* get input plugin info */
    input_info = dlsym(handle, "InputPlugin_Info");
    error = dlerror();
    if (error != NULL) {
        dlclose(handle);
        fprintf(stderr,
                "Could not get Flutio_InputInfo function: %s\n", error);
        return 1;
    }

    input_info(&info);

    /* create storage */
    InputPlugin_T *new = malloc(sizeof(InputPlugin_T));
    new->handle = handle;
    new->path = strdup(path);
    new->next = NULL;
    new->info = info;

    InputPlugin_T **it = &G_Inputs_plugins;
    while ((*it))
    {
        it = &(*it)->next;
    }
    *it = new;

    return 0;
}

Input_T*
Inputs_Open(char* fname) {
    InputPlugin_T *it = G_Inputs_plugins;
    while (it) {
        if (it->info.concerned(fname)) {
            PluginData_T d = it->info.open(fname);
            if (d) {
                Input_T *in = malloc(sizeof(Input_T));
                in->plugin = it;
                in->data = d;
                return in;
            }
        }
    }
    return NULL;
}

float*
Inputs_Read(Input_T* in, int want, int* got)
{
    return in->plugin->info.read(in->data, want, got);
}

void
Inputs_Close(Input_T* in)
{
    in->plugin->info.close(in->data);
    free(in);
}

void
Inputs_Release()
{
    InputPlugin_T *it = G_Inputs_plugins;
    while (it)
    {
        InputPlugin_T *next = it->next;

        dlclose(it->handle);
        free(it->path);
        free(it);

        it = next;
    }
}
