#ifndef INPUTS_H
#define INPUTS_H
#define FLUTIO_MAIN_BUILD
#include <flutio/plugins/input.h>

typedef struct _InputPlugin_T InputPlugin_T;
typedef struct _InputPlugin_T {
    void* handle;
    char* path;
    InputPlugin_Info_T info;
    InputPlugin_T *next;
} InputPlugin_T;

extern InputPlugin_T *G_Inputs_plugins;


typedef struct _Input_T {
    InputPlugin_T *plugin;
    PluginData_T   data;
} Input_T;

int Inputs_Load(char*,void*);
void Inputs_Release();

Input_T* Inputs_Open(char*);
void Inputs_Close(Input_T*);
float* Inputs_Read(Input_T*,int,int*);

#endif // INPUTS_H
