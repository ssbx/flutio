#include "plugins.h"
#include "outputs.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <tcl.h>

/* tcl script api */
static int tclflutio_loadplugin(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int tclflutio_close(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int tclflutio_play(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int tclflutio_stop(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int tclflutio_setnext(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);
static int tclflutio_setpos(ClientData,Tcl_Interp*,int,Tcl_Obj*const[]);

/* Tcl custom event source and callbacks */
#define TCLFPLAYER_EVENT_CHECK_TIME 100000
static int         g_tclflutio_event_sigint   = 0;
static Tcl_Obj    *g_tclflutio_event_callback = NULL;
static Tcl_Interp *g_tclflutio_interp         = NULL;
static void tclflutio_event_setup(ClientData,int);
static void tclflutio_event_check(ClientData,int);
static int  tclflutio_event_proc_exit(Tcl_Event*,int);

/* tcl package init stop */
int         Tclflutio_Init(Tcl_Interp*);
static void Tclflutio_Exit_Signal(int);
static void Tclflutio_Exit(ClientData);

/*************************************************************************
 * Tcl script api
 *************************************************************************/
static int
tclflutio_loadplugin(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    if (objc != 2) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj(
                "Wrong arg number: Flutio::plugin::load pluginPath", -1));
        return TCL_ERROR;
    }

    Plugins_Load(Tcl_GetStringFromObj(objv[1], NULL));

    return TCL_OK;
}

static int
tclflutio_openoutput(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    if (Outputs_Open()) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj(
                "Error: Flutio::outputs::open", -1));
        return TCL_ERROR;

    }
    return TCL_OK;
}

static int
tclflutio_close(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
tclflutio_play(
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
tclflutio_stop(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
tclflutio_setnext(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
tclflutio_setpos(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
tclflutio_setvol(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}

static int
tclflutio_getplugins(
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
tclflutio_event_setup(ClientData d, int flags)
{
    Tcl_Time t = {0, TCLFPLAYER_EVENT_CHECK_TIME};
    if (g_tclflutio_event_sigint)
        t.usec = 0;

    Tcl_SetMaxBlockTime(&t);
}

static void
tclflutio_event_check(ClientData d, int flags)
{
    if (g_tclflutio_event_sigint) {
        Tcl_Event *e = (Tcl_Event*) ckalloc(sizeof(Tcl_Event));
        e->proc = tclflutio_event_proc_exit;
        Tcl_QueueEvent(e, TCL_QUEUE_TAIL);
    }
}

static int
tclflutio_event_proc_exit(Tcl_Event *e, int flags)
{
    Tcl_Exit(0);
    return 1;
}

/*************************************************************************
 * Tcl C extensions init and close
 *************************************************************************/
static void
Tclflutio_Exit_Signal(int d)
{
    g_tclflutio_event_sigint = 1;
}

static void
Tclflutio_Exit(ClientData d)
{
    Player_Clear();
    Outputs_Close();
    Plugins_ReleaseAll();
}

int
Tclflutio_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj("fplayer error at Tcl_Initstubs", -1));
        return TCL_ERROR;
    }

    Tcl_Namespace *nsPtr;
    if ((nsPtr =
      Tcl_CreateNamespace(interp, "::flutio::c", NULL, NULL)) == NULL) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj("fplayer error at Tcl_CreateNamespace", -1));
        return TCL_ERROR;
    }

    signal(SIGINT,  Tclflutio_Exit_Signal);
    signal(SIGTERM, Tclflutio_Exit_Signal);
    signal(SIGKILL, Tclflutio_Exit_Signal);

    Tcl_CreateExitHandler(Tclflutio_Exit, NULL);
    Tcl_CreateEventSource(
        tclflutio_event_setup, tclflutio_event_check, NULL);

    Tcl_CreateObjCommand(interp,
            "::flutio::c::plugins::load", tclflutio_loadplugin, NULL, NULL);

    Tcl_CreateObjCommand(interp,
            "::flutio::c::outputs::open", tclflutio_openoutput, NULL, NULL);


    Tcl_CreateObjCommand(interp,
            "::flutio::c::player::play", tclflutio_play, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::flutio::c::player::setnext", tclflutio_setnext, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::flutio::c::player::setpos", tclflutio_setpos, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::flutio::c::player::stop", tclflutio_stop, NULL, NULL);

    g_tclflutio_interp = interp;

    Player_Init();

    return TCL_OK;
}

