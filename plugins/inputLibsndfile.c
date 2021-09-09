#include <flutio/plugins/input.h>
#include <sndfile.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Interface things
 */
static char PLUGIN_NAME[]   = "libsndfile input";
static int  PLUGIN_REVISION = 1;

#define DEFAULT_FRAMES_PER_BUFF 1000
static int g_initialFramesPerBuff = DEFAULT_FRAMES_PER_BUFF;
static PluginData_T SFile_open(char*);
static int  SFile_concerned(char*);
static float*  SFile_read(PluginData_T,int,int*);
static void SFile_close(PluginData_T);
static int  SFile_getRate(PluginData_T);
static int  SFile_getFrames(PluginData_T);
static int  SFile_getChannels(PluginData_T);

void InputPlugin_Info(InputPlugin_Info_T *info)
{
    info->name            = PLUGIN_NAME;
    info->revision        = PLUGIN_REVISION;
    info->open            = SFile_open;
    info->concerned       = SFile_concerned;
    info->read            = SFile_read;
    info->getRate         = SFile_getRate;
    info->getFrameCount   = SFile_getFrames;
    info->getChannelCount = SFile_getChannels;
    info->close           = SFile_close;
    fprintf(stderr, "sucessssssssssssssssssss!\n");
}

/*
 * Implementation
 */
typedef struct {
    SNDFILE *sf;
    SF_INFO info;
    int    framesPerBuff;
    float *buffer;
} SFile_T;

static int
SFile_concerned(char *fname)
{
    return 1;
}


static PluginData_T
SFile_open(char *fname)
{
    SF_INFO sf_info;
    sf_info.format = 0;
    SNDFILE *sf;
    if ((sf = sf_open(fname, SFM_READ, &sf_info)) == NULL) {
        return NULL;
    }

    SFile_T *data = (SFile_T*) malloc(sizeof(SFile_T));
    data->info = sf_info;
    data->sf   = sf;
    data->framesPerBuff = g_initialFramesPerBuff;
    data->buffer = malloc(sizeof(float) *
                                data->framesPerBuff * data->info.channels);
    return (PluginData_T) data;
}

static float*
SFile_read(PluginData_T pdata, int want, int *got)
{
    SFile_T *data = (SFile_T*) pdata;
    if (want > data->framesPerBuff) {
        g_initialFramesPerBuff = want;
        data->framesPerBuff = g_initialFramesPerBuff;
        data->buffer = realloc(data->buffer, sizeof(float) *
                                data->framesPerBuff * data->info.channels);
    }
    *got = sf_readf_float(data->sf, data->buffer, want);
    return data->buffer;
}

static void
SFile_close(PluginData_T pdata)
{
    SFile_T *data = (SFile_T*) pdata;
    sf_close(data->sf);
    free(data);
}

static int
SFile_getRate(PluginData_T pdata)
{
    SFile_T *data = (SFile_T*) pdata;
    return data->info.samplerate;
}

static int
SFile_getFrames(PluginData_T pdata)
{
    SFile_T *data = (SFile_T*) pdata;
    return data->info.frames;
}

static int
SFile_getChannels(PluginData_T pdata)
{
    SFile_T *data = (SFile_T*) pdata;
    return data->info.channels;
}

