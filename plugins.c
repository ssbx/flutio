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
    plugin_type = dlsym(handle, "PluginType");
    if ((error = dlerror()) != NULL) {
        dlclose(handle);
        fprintf(stderr,
                    "Could not find PluginType function: %s\n", error);
        return 1;
    }

    switch (plugin_type()) {
        case FLUTIO_PLUGIN_TYPE_INPUT:  return Inputs_Load(path, handle);
        case FLUTIO_PLUGIN_TYPE_OUTPUT: return Outputs_Load(path, handle);
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
