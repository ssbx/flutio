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

#ifndef FLUTIO_POST_INPUT_PLUGIN_H
#define FLUTIO_POST_INPUT_PLUGIN_H
#include <flutio/plugins/common.h>

#ifndef FLUTIO_MAIN_BUILD // only for plugin include
    int PluginType() {
        return FLUTIO_PLUGIN_TYPE_POST_INPUT;
    }
#endif // FLUTIO_MAIN_BUILD
#include <flutio/plugins/input.h>

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
 * PostInput plugin interface
 */
typedef struct _PostInputPlugin_Info_T {
    char *name;
    int	revision;
    InputPlugin_Info_T* getFrameGeneratorInterface(PluginData_T);
    int concerned(InputPlugin_Info_T*);
    PluginData_T init(InputPlugin_Info_T*);
} PostInputPlugin_Info_T;

/*
 * This is the unique function that must be implemented on the
 * plugin side. Must return the type of plugin, and the relevant
 * union (input or output) filled with relevant data.
 * input_info_t is self explanatory, see default flutio plugins
 * source for examples.
 */
void PostInputPlugin_Info(PostInputPlugin_Info_T *info);

#endif // FLUTIO_POST_INPUT_PLUGIN_H
