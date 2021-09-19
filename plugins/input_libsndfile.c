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

#include <mpd-ng/plugins/input.h>
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

void MpdNG_InputPluginInfo(InputPluginInfo_T *info)
{
    info->name            = PLUGIN_NAME;
    info->revision        = PLUGIN_REVISION;
    info->concerned       = SFile_concerned;
    info->close           = SFile_close;
    info->open            = SFile_open;
    info->frameGen.read            = SFile_read;
    info->frameGen.getRate         = SFile_getRate;
    info->frameGen.getFrameCount   = SFile_getFrames;
    info->frameGen.getChannelCount = SFile_getChannels;
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

