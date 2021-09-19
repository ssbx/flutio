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

#ifndef MPDNG_INPUT_PLUGIN_H
#define MPDNG_INPUT_PLUGIN_H
#include <mpd-ng/plugins/common.h>
#include <mpd-ng/interfaces/frame_generator.h>

#ifndef MPDNG_MAIN_BUILD
    PluginType_T MpdNG_PluginType() { return MPDNG_PLUGIN_TYPE_INPUT; }
#endif

/*
 * To write a plugin for MpdNG:
 *  - include this file
 *  - write a "input_info" function in your plugin
 *  - put your plugin in either $(libdir)/mpd-ng/plugins or
 *  $HOME/.mpd-ng/plugins
 *
 *  See the documentation of "input_info" at the end of this
 *  file.
 */

/*
 * Input plugin interface
 */
typedef struct {
    char          *name;
    int	           revision;
    FrameGen_I     frameGen;
    int          (*concerned) (char*);
    char**       (*listOptions)();
    char*        (*getOption) (char*);
    int          (*setOption) (char*,char*);
    PluginData_T (*open)      (char*);
    void         (*close)     (PluginData_T);
} InputPluginInfo_T;

/*
 * This is the unique function that must be implemented on the
 * plugin side. Must return the type of plugin, and the relevant
 * union (input or output) filled with relevant data.
 * input_info_t is self explanatory, see default mpd-ng plugins
 * source for examples.
 */
void MpdNG_InputPluginInfo(InputPluginInfo_T*);

#endif // MPDNG_INPUT_PLUGIN_H
