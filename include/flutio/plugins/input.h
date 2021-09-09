#ifndef FLUTIO_INPUT_PLUGIN_H
#define FLUTIO_INPUT_PLUGIN_H
#include <flutio/plugins/common.h>

#ifndef FLUTIO_MAIN_BUILD // only for plugin include
    int PluginType() {
        return FLUTIO_PLUGIN_TYPE_INPUT;
    }
#endif // FLUTIO_MAIN_BUILD

/*
 * To write a plugin for Flutio:
 *  - include this file
 *  - write a "input_info" function in your plugin
 *  - put your plugin in either $(libdir)/flutio/plugins or
 *  $HOME/.flutio/plugins
 *
 *  See the documentation of "input_info" at the end of this
 *  file.
 */

/*
 * Input plugin interface
 */
typedef struct _InputPlugin_Info_T {
    char *name;
    int	revision;
    int (*concerned)(char*);
    PluginData_T (*open)(char*);
    float* (*read)(PluginData_T,int,int*);
    int (*getRate)(PluginData_T);
    int (*getFrameCount)(PluginData_T);
    int (*getChannelCount)(PluginData_T);
    void (*close)(PluginData_T);
    int (*seek)(PluginData_T,int,int);
} InputPlugin_Info_T;

/*
 * This is the unique function that must be implemented on the
 * plugin side. Must return the type of plugin, and the relevant
 * union (input or output) filled with relevant data.
 * input_info_t is self explanatory, see default flutio plugins
 * source for examples.
 */
void InputPlugin_Info(InputPlugin_Info_T *info);

#endif // FLUTIO_INPUT_PLUGIN_H
