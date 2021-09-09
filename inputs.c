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
