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
#include <mpd-ng/plugins/common.h>

#define MPDNG_OUTPUT_API_GETFRAMES 0

#ifndef MPDNG_MAIN_BUILD // only for plugin include
    PluginType_T MpdNG_PluginType() { return MPDNG_PLUGIN_TYPE_OUTPUT; }

    /*
     * Functions available to the plugin
     */
    float* (*Player_GetFrames)(int,int*);
    void MpdNG_OutputPlugin_Api(int index, void* fun) {
        switch (index) {
            case MPDNG_OUTPUT_API_GETFRAMES: {
                Player_GetFrames = (float* (*)(int, int*)) fun;
            }
        }
    }

#endif // MPDNG_MAIN_BUILD

/*
 * To write an output plugin for mpd-ng:
 *  - include this file
 *  - write a non static "MpdNG_output_info" function in your plugin
 *  - put your plugin in either $(libdir)/mpd-ng/plugins or
 *  $HOME/.mpd-ng/plugins
 *
 *  See the documentation of "MpdNG_output_info" at the end of this
 *  file.
 */

typedef struct {
    int wanted_rate;
    int wanted_channels;
} OutputParams_T;

/*
 * Output plugin callbacks
 */
typedef struct {
    char *name;
    int	revision;
    int (*Open)(OutputParams_T*);
    void (*Close)();
    int (*GetRate)();
    int (*GetChannels)();
    char* (*GetFormFromConfig)();
    int (*SetConfigFromForm)(char*);
} OutputPluginInfo_T;

/*
 * void MpdNG_OutputInfo(MpdNG_OutputInfo_T *info);
 * This is the unique function that must be implemented on the
 * plugin side. Must return the type of plugin, and the relevant
 * union (input or output) filled with relevant data.
 * MpdNG_output_plugin_info_t and MpdNG_input_plugin_info_t are
 * self explanatory, see default mpd-ng plugins source for
 * examples.
 *
 * The MpdNG_api_t struct contains mpd-ng function available to
 * plugins.
 */
 void MpdNG_OutputPluginInfo(OutputPluginInfo_T*);

#endif // OUTPUT_PLUGIN_H
