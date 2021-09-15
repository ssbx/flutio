/* Copyright (C) 2003 Scott Wheeler <wheeler@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string.h>

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tpropertymap.h>

extern "C" {
    #include <tcl.h>
    typedef struct {
        char *key;
        char **vals;
        int    valn;
        int    intval;
    } taglib_t;
    int taglib_get(ClientData,Tcl_Interp*,int,Tcl_Obj *const objv[]);
    int Tcltaglib_Init(Tcl_Interp*);
}

using namespace std;

#define NUM_AUDIO_PROPS 5
/* audio props
 * 0 - lengthInSeconds
 * 1 - lengthInMilliseconds
 * 2 - bitrate
 * 3 - samplerate
 * 4 - channels
 */
int
taglib_get(
    ClientData cdata,
    Tcl_Interp  *interp,
    int         objc,
    Tcl_Obj *const objv[])
{

    // Only one arg
    if (objc != 2) {
        Tcl_Obj *ret =
            Tcl_NewStringObj("tcltaglib_get error need (only) one arg", -1);
        Tcl_SetObjResult(interp, ret);
        return TCL_ERROR;
    }
    char *fname = Tcl_GetStringFromObj(objv[1], NULL);


    taglib_t *alltags = NULL;;
    int         num     = 0;

    //
    // C++ clode begin
    //
    TagLib::FileRef f(fname);

    if (!f.isNull()) {
        TagLib::PropertyMap tags = f.file()->properties();

        num = tags.size() + NUM_AUDIO_PROPS;
        alltags = (taglib_t*) malloc(sizeof(taglib_t) * num);
        int id;
        for (id = 0; id < num; id++) {
            alltags[id].key  = NULL;
            alltags[id].vals = NULL;
            alltags[id].valn = 0;
            alltags[id].intval = 0;
        }

        alltags[0].key    = strdup("AUDIOPROP_SECONDS");
        alltags[0].intval = 0;
        alltags[1].key = strdup("AUDIOPROP_MILLISECONDS");
        alltags[1].intval = 0;
        alltags[2].key = strdup("AUDIOPROP_BITRATE");
        alltags[2].intval = 0;
        alltags[3].key = strdup("AUDIOPROP_SAMPLERATE");
        alltags[3].intval = 0;
        alltags[4].key = strdup("AUDIOPROP_CHANNELS");
        alltags[4].intval = 0;

        if(f.audioProperties()) {

            TagLib::AudioProperties *properties = f.audioProperties();

            //alltags[0].intval = properties->lengthInSeconds();
            //alltags[1].intval = properties->lengthInMilliseconds();
            alltags[2].intval = properties->bitrate();
            alltags[3].intval = properties->sampleRate();
            alltags[4].intval = properties->channels();

        }


        if (tags.size() > 0) {

            int id = NUM_AUDIO_PROPS;
            for(TagLib::PropertyMap::ConstIterator i = tags.begin();
                    i != tags.end(); ++i) {

                alltags[id].key = (char*) malloc(strlen(i->first.toCString()) + 1);
                strcpy(alltags[id].key, i->first.toCString());

                int valnum = i->second.size();
                if (valnum > 0) {
                    alltags[id].vals = (char**) malloc(sizeof(char) * valnum);
                    alltags[id].valn = valnum;
                }

                int lid = 0;
                for(TagLib::StringList::ConstIterator j = i->second.begin();
                        j != i->second.end(); ++j) {
                    alltags[id].vals[lid] = (char*) malloc(strlen(j->toCString()) + 1);
                    strcpy(alltags[id].vals[lid], j->toCString());
                    lid++;
                }
                id++;
            }
        }
    }
    //
    // C++ code end
    //

    Tcl_Obj* dict = Tcl_NewDictObj();
    int i;
    for (i = 0; i < num; i++) {

        Tcl_Obj* key = Tcl_NewStringObj(alltags[i].key, -1);
        Tcl_Obj* vals = Tcl_NewListObj(0, NULL);
        if (alltags[i].vals) {
            int j;
            for (j=0; j < alltags[i].valn; j++) {
                Tcl_ListObjAppendElement(
                    interp, vals,
                    Tcl_NewStringObj(alltags[i].vals[j], -1));
                free(alltags[i].vals[j]);
            }
            free(alltags[i].vals);
        } else {
            Tcl_ListObjAppendElement(
                    interp, vals,
                    Tcl_NewIntObj(alltags[i].intval));
        }
        Tcl_DictObjPut(interp, dict, key, vals);
        free(alltags[i].key);
    }
    free(alltags);
    Tcl_SetObjResult(interp, dict);
    return TCL_OK;
}

int
Tcltaglib_Init(Tcl_Interp *interp)
{
    Tcl_Namespace *nsPtr;
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL)
        return TCL_ERROR;

    if ((nsPtr = Tcl_CreateNamespace(interp, "::taglib", NULL, NULL)) == NULL)
        return TCL_ERROR;

    if (Tcl_PkgProvide(interp, "taglib", "1.0") != TCL_OK)
        return TCL_ERROR;

    Tcl_CreateObjCommand(interp, "::taglib::get", taglib_get, NULL, NULL);

    return TCL_OK;
}


