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

#include "inputs.h"
#include "outputs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

int
Plugins_Load(char* path)
{
    char *error;
    void *handle;

    /* open the object */
    if ((handle = dlopen(path, RTLD_LAZY)) == NULL) {
        fprintf(stderr,
                    "Could not open plugin: %s\n", dlerror());
        return 1;
    }

    /* clear any existing error */
    dlerror();

    /* get the type function */
    int (*plugin_type)();
    plugin_type = dlsym(handle, "MpdNG_PluginType");
    if ((error = dlerror()) != NULL) {
        dlclose(handle);
        fprintf(stderr,
                    "Could not find MpdNG_PluginType function: %s\n", error);
        return 1;
    }

    switch (plugin_type()) {
        case MPDNG_PLUGIN_TYPE_INPUT:  return Inputs_Load(path, handle);
        case MPDNG_PLUGIN_TYPE_OUTPUT: return Outputs_Load(path, handle);
        default: {
            fprintf(stderr,"Unkown plugin type %i for %s\n", plugin_type(), path);
            dlclose(handle);
            return 1;
        }
    }
}

void
Plugins_ReleaseAll()
{
    Inputs_Release();
    Outputs_Release();
}
