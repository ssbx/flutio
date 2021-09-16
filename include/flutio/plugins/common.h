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

#ifndef FLUTIO_PLUGINS_COMMON_H
#define FLUTIO_PLUGINS_COMMON_H

typedef enum {
    FLUTIO_PLUGIN_TYPE_INPUT      = 0,
    FLUTIO_PLUGIN_TYPE_POST_INPUT = 1,
    FLUTIO_PLUGIN_TYPE_PRE_OUTPUT = 2,
    FLUTIO_PLUGIN_TYPE_OUTPUT     = 3,
    FLUTIO_PLUGIN_TYPE_MSG_FORMAT = 4,
    FLUTIO_PLUGIN_TYPE_FADE_EFFECT= 5
} PluginType_T;

typedef void* PluginData_T;

typedef enum {INT_OPTION, FLOAT_OPTION, STRING_OPTION} PluginOptionType_T;
typedef union {
    int   intVal;
    float floatVal;
    char *stringVal;
} PluginOptionValue_T;

typedef struct {
    char* name;
    PluginOptionType_T  type;
    PluginOptionValue_T value;
} PluginOption_T;

typedef struct {
} FlutioApi_T;

#ifndef FLUTIO_MAIN_BUILD // only for plugin include

    static FlutioApi_T* g_api;

    void Flutio_ApiSet(FlutioApi_T *api) {
        g_api = api;
    }

#endif // FLUTIO_MAIN_BUILD

#endif // FLUTIO_PLUGINS_COMMON_H


