#ifndef OUTPUT_PLUGIN_H
#define OUTPUT_PLUGIN_H
#include <flutio/plugins/common.h>

#define FLUTIO_OUTPUT_API_GETFRAMES 0

#ifndef FLUTIO_MAIN_BUILD // only for plugin include
    int PluginType() {
        return FLUTIO_PLUGIN_TYPE_OUTPUT;
    }

    /*
    * Functions available to the plugin
    */
    float* (*Player_GetFrames)(int,int*);
    void OutputPlugin_Api(int index, void* fun) {
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

typedef struct _OutputParams_T {
    int wanted_rate;
    int wanted_channels;
} OutputParams_T;

/*
 * Output plugin callbacks
 */
typedef struct _OutputPlugin_Info_T {
    char *name;
    int	revision;
    int (*Open)(OutputParams_T*);
    void (*Close)();
    int (*GetRate)();
    int (*GetChannels)();
    char* (*GetFormFromConfig)();
    int (*SetConfigFromForm)(char*);
} OutputPlugin_Info_T;

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


#endif // OUTPUT_PLUGIN_H
