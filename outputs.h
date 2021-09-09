#ifndef OUTPUTS_H
#define OUTPUTS_H
#define FLUTIO_MAIN_BUILD
#include <flutio/plugins/output.h>

typedef struct _OutputPlugin_T OutputPlugin_T;
typedef struct _OutputPlugin_T {
    void* handle;
    char* path;
    OutputPlugin_Info_T info;
    OutputPlugin_T *next;
} OutputPlugin_T;

extern OutputPlugin_T *G_Outputs_plugins;

int  Outputs_Open();
void Outputs_Close();

int Outputs_Load(char*,void*);
void Outputs_Release();

#endif // OUTPUTS_H
