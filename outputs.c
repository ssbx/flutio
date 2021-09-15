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

#include "outputs.h"
#include "player.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>


OutputPlugin_T *G_Outputs_plugins = NULL;
static OutputPlugin_T *g_selected_output = NULL;
static OutputPlugin_T *g_active_output = NULL;
static OutputParams_T  g_params;

int
Outputs_Open()
{
    if (!g_selected_output) {
        if (!G_Outputs_plugins) {
            fprintf(stderr, "Outputs_Open no output plugin available\n");
            return 1;
        } else {
            fprintf(stderr, "Outputs_Open no output plugin selected\n");
            return 1;
        }
    }

    if (g_active_output)
        Outputs_Close();

    if (g_selected_output->info.Open(&g_params) != 0) {
        fprintf(stderr, "Outputs_Open failed for %s\n", g_selected_output->path);
        return 1;
    }

    g_active_output = g_selected_output;
    return 0;
}

int
Outputs_Load(char* path, void* handle)
{
    char *error;
    OutputPlugin_Info_T info;
    void (*output_info)(OutputPlugin_Info_T *info);


    /* set output plugin "getFrame" functions */
    void (*out_api)(int,void*);
    out_api = dlsym(handle, "Flutio_OutputPlugin_Api");
    error = dlerror();
    if (error != NULL) {
        dlclose(handle);
        fprintf(stderr,
                "Could not get Flutio_OutputPlugin_Api variable: %s\n", error);
        return 1;
    }
    out_api(FLUTIO_OUTPUT_API_GETFRAMES, Player_GetFrames);

    /* get output plugin info */
    output_info = dlsym(handle, "Flutio_OutputPlugin_Info");
    error = dlerror();
    if (error != NULL) {
        dlclose(handle);
        fprintf(stderr,
                "Could not get Flutio_OutputPlugin_Info function: %s\n", error);
        return 1;
    }

    output_info(&info);

    OutputPlugin_T *new = malloc(sizeof(OutputPlugin_T));
    new->handle = handle;
    new->path = strdup(path);
    new->next = NULL;
    new->info = info;

    OutputPlugin_T **it = &G_Outputs_plugins;
    while ((*it)) {
        /*
        if ((strcmp(it->info.name, new->info.name) == 0) &&
                (it->info.revision <= new->info.revision))
        {
            new->next = it->next;
            dlclose(it->handle);
            free(it->path);
            free(it);
            it = NULL;
            break;
        }
        */
        it = &(*it)->next;
    }

    *it = new;

    g_selected_output = new;
    return 0;
}

void
Outputs_Close() {
    if (g_active_output)
        g_active_output->info.Close();
    g_active_output = NULL;
}

void
Outputs_Release()
{
    Outputs_Close();
    OutputPlugin_T *it = G_Outputs_plugins;
    while (it)
    {
        OutputPlugin_T *next = it->next;

        dlclose(it->handle);
        free(it->path);
        free(it);

        it = next;
    }
    g_selected_output = NULL;
}
