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

#include "plugins.h"
#include "outputs.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <tcl.h>

/* tcl script api */
static int TclMpdNG_loadplugin(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int TclMpdNG_close(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int TclMpdNG_play(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int TclMpdNG_stop(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int TclMpdNG_setnext(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int TclMpdNG_setpos(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);

/* Tcl custom event source and callbacks */
#define TCLMPDNG_EVENT_CHECK_TIME 100000
static int         g_TclMpdNG_event_sigint   = 0;
static Tcl_Obj    *g_TclMpdNG_event_callback = NULL;
static Tcl_Interp *g_TclMpdNG_interp         = NULL;
static void TclMpdNG_event_setup(ClientData,int);
static void TclMpdNG_event_check(ClientData,int);
static int  TclMpdNG_event_proc_exit(Tcl_Event*,int);

/* tcl package init stop */
int         Tclmpdng_Init(Tcl_Interp*);
static void Tclmpdng_Exit_Signal(int);
static void Tclmpdng_Exit(ClientData);

/*************************************************************************
 * Tcl script api
 *************************************************************************/
static int
TclMpdNG_loadplugin(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    if (objc != 2) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj(
                "Wrong arg number: mpdng::plugin::load pluginPath", -1));
        return TCL_ERROR;
    }

    Plugins_Load(Tcl_GetStringFromObj(objv[1], NULL));

    return TCL_OK;
}

static int
TclMpdNG_openoutput(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    if (Outputs_Open()) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj(
                "Error: mpdng::outputs::open", -1));
        return TCL_ERROR;

    }
    return TCL_OK;
}

static int
TclMpdNG_close(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
TclMpdNG_play(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    if (objc < 2)
        return TCL_ERROR;
    char *filename = Tcl_GetStringFromObj(objv[1], NULL);

    if (Player_Play(filename) != 0)
        return TCL_ERROR;

    return TCL_OK;
}

static int
TclMpdNG_stop(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
TclMpdNG_setnext(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
TclMpdNG_setpos(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
TclMpdNG_setvol(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
TclMpdNG_getplugins(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

//static Tcl_Obj    *g_callback        = NULL;
//static Tcl_Interp *g_callback_interp = NULL;

//typedef struct {
 //   Tcl_Event      tcl;
  //  Mixer_event_t *player;
//} tcl_fplayer_custom_event_t;


/***************************************************************************
 * Tcl custom event source and callbacks
 **************************************************************************/
static void
TclMpdNG_event_setup(ClientData d, int flags)
{
    Tcl_Time t = {0, TCLMPDNG_EVENT_CHECK_TIME};
    if (g_TclMpdNG_event_sigint)
        t.usec = 0;

    Tcl_SetMaxBlockTime(&t);
}

static void
TclMpdNG_event_check(ClientData d, int flags)
{
    if (g_TclMpdNG_event_sigint) {
        Tcl_Event *e = (Tcl_Event*) ckalloc(sizeof(Tcl_Event));
        e->proc = TclMpdNG_event_proc_exit;
        Tcl_QueueEvent(e, TCL_QUEUE_TAIL);
    }
}

static int
TclMpdNG_event_proc_exit(Tcl_Event *e, int flags)
{
    Tcl_Exit(0);
    return 1;
}

/*************************************************************************
 * Tcl C extensions init and close
 *************************************************************************/
static void
Tclmpdng_Exit_Signal(int d)
{
    g_TclMpdNG_event_sigint = 1;
}

static void
Tclmpdng_Exit(ClientData d)
{
    Player_Clear();
    Outputs_Close();
    Plugins_ReleaseAll();
}

int
Tclmpdng_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj("mpdng error at Tcl_Initstubs", -1));
        return TCL_ERROR;
    }

    Tcl_Namespace *nsPtr;
    if ((nsPtr =
      Tcl_CreateNamespace(interp, "::mpdng::c", NULL, NULL)) == NULL) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj("mpdng error at Tcl_CreateNamespace", -1));
        return TCL_ERROR;
    }

    signal(SIGINT,  Tclmpdng_Exit_Signal);
    signal(SIGTERM, Tclmpdng_Exit_Signal);
    signal(SIGKILL, Tclmpdng_Exit_Signal);

    Tcl_CreateExitHandler(Tclmpdng_Exit, NULL);
    Tcl_CreateEventSource(
        TclMpdNG_event_setup, TclMpdNG_event_check, NULL);

    Tcl_CreateObjCommand(interp,
            "::mpdng::c::plugins::load", TclMpdNG_loadplugin, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::mpdng::c::outputs::open", TclMpdNG_openoutput, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::mpdng::c::player::play", TclMpdNG_play, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::mpdng::c::player::setnext", TclMpdNG_setnext, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::mpdng::c::player::setpos", TclMpdNG_setpos, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::mpdng::c::player::stop", TclMpdNG_stop, NULL, NULL);

    g_TclMpdNG_interp = interp;

    Player_Init();

    return TCL_OK;
}

