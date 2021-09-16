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

#ifndef OUTPUT_PLUGIN_H
#define OUTPUT_PLUGIN_H
#include <flutio/plugins/common.h>

#define FLUTIO_OUTPUT_API_GETFRAMES 0

#ifndef FLUTIO_MAIN_BUILD // only for plugin include
    PluginType_E Flutio_PluginType() { return FLUTIO_PLUGIN_TYPE_OUTPUT; }

    /*
     * Functions available to the plugin
     */
    float* (*Player_GetFrames)(int,int*);
    void Flutio_OutputPlugin_Api(int index, void* fun) {
        switch (index) {
            case FLUTIO_OUTPUT_API_GETFRAMES: {
                Player_GetFrames = (float* (*)(int, int*)) fun;
            }
        }
    }

#endif // FLUTIO_MAIN_BUILD

/*
 * To write an output plugin for Flutio:
 *  - include this file
 *  - write a non static "flutio_output_info" function in your plugin
 *  - put your plugin in either $(libdir)/flutio/plugins or
 *  $HOME/.flutio/plugins
 *
 *  See the documentation of "flutio_output_info" at the end of this
 *  file.
 */

typedef struct _OutputParams_S {
    int wanted_rate;
    int wanted_channels;
} OutputParams_S;

/*
 * Output plugin callbacks
 */
typedef struct _OutputPluginInfo_S {
    char *name;
    int	revision;
    int (*Open)(OutputParams_T*);
    void (*Close)();
    int (*GetRate)();
    int (*GetChannels)();
    char* (*GetFormFromConfig)();
    int (*SetConfigFromForm)(char*);
} OutputPluginInfo_S;

/*
 * void Flutio_OutputInfo(Flutio_OutputInfo_T *info);
 * This is the unique function that must be implemented on the
 * plugin side. Must return the type of plugin, and the relevant
 * union (input or output) filled with relevant data.
 * flutio_output_plugin_info_t and flutio_input_plugin_info_t are
 * self explanatory, see default flutio plugins source for
 * examples.
 *
 * The flutio_api_t struct contains flutio function available to
 * plugins.
 */
 void Flutio_OutputPluginInfo(OutputPluginInfo_S*);

#endif // OUTPUT_PLUGIN_H
