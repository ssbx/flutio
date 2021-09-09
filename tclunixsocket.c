#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <tcl.h>

#define SOCKNAME_TEMPLATE "unixsocket%d"
#define SOCKNAME_SIZE      32
#define LISTEN_BACKLOG     10

typedef enum { UNIXSOCK_SERVER, UNIXSOCK_CLIENT } unixsocket_type_t;
typedef struct _unixsocket_state_t unixsocket_state_t;
typedef struct _unixsocket_state_t {
    Tcl_Channel       channel;
    Tcl_Interp       *interp;
    int               fd;
    unixsocket_type_t   type;
    unixsocket_state_t *prev;
    unixsocket_state_t *next;
    char             *srv_path;
    Tcl_Obj          *srv_acceptfun;
} unixsocket_state_t;

static int  unixsocket_listen(
    ClientData,Tcl_Interp*,int,Tcl_Obj*const objv[]);
static int  unixsocket_connect(
    ClientData,Tcl_Interp*,int,Tcl_Obj*const objv[]);

static int  unixsocketCloseProc(ClientData,Tcl_Interp*);
static int  unixsocketInputProc(ClientData,char*,int,int*);
static int  unixsocketOutputProc(ClientData,const char*,int,int*);
static void unixsocketWatchProc(ClientData,int);
static int  unixsocketGetHandleProc(ClientData,int,ClientData*);
static int  unixsocketBlockModeProc(ClientData,int);

static const Tcl_ChannelType unixsocketChannelType = {
    "unix",                     /* Type name. */
    TCL_CHANNEL_VERSION_2,      /* v5 channel */
    unixsocketCloseProc,        /* Close proc. */
    unixsocketInputProc,        /* Input proc. */
    unixsocketOutputProc,       /* Output proc. */
    NULL,                       /* Seek proc. */
    NULL,                       /* Set option proc. */
    NULL,                       /* Get option proc. */
    unixsocketWatchProc,        /* Initialize notifier. */
    unixsocketGetHandleProc,    /* Get OS handles out of channel. */
    NULL,                       /* Close2 proc. */
    unixsocketBlockModeProc,    /* Set blocking or non-blocking mode.*/
    NULL,                       /* flush proc. */
    NULL,                       /* handler proc. */
    NULL,                       /* wide seek proc. */
    NULL,                       /* thread action proc. */
    NULL                        /* truncate proc. */
};

int
Tclunixsocket_Init(Tcl_Interp *interp)
{
    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL)
        return TCL_ERROR;


    Tcl_Namespace *nsPtr = 
        Tcl_CreateNamespace(interp, "::unixsocket", NULL, NULL);
    if (nsPtr == NULL)
        return TCL_ERROR;

    Tcl_CreateObjCommand(interp,
            "::unixsocket::listen",  unixsocket_listen,  NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::unixsocket::connect",  unixsocket_connect,  NULL, NULL);

    return TCL_OK;
}

static void
unixsocketet_accept(
    ClientData data,
    int        mask)
{
    unixsocket_state_t *state = (unixsocket_state_t*) data;

    struct sockaddr_un addr;
    socklen_t len = sizeof(struct sockaddr_un);
    int sock = accept(state->fd, (struct sockaddr*) &addr, &len);
    if (sock < 0) {
        return;
    }

    fprintf(stderr, "accept! %i\n", sock);
    fcntl(sock, F_SETFD, FD_CLOEXEC);

    unixsocket_state_t *cstate = ckalloc(sizeof(unixsocket_state_t));
    cstate->type = UNIXSOCK_CLIENT;
    cstate->next = NULL;
    cstate->prev = NULL;
    cstate->fd   = sock;
    cstate->srv_path = NULL;
    cstate->srv_acceptfun = NULL;
    cstate->interp = NULL;

    char tclname[SOCKNAME_SIZE];
    sprintf(tclname, SOCKNAME_TEMPLATE, cstate->fd);

    fprintf(stderr, "accept called %s\n", tclname);
    cstate->channel = Tcl_CreateChannel(
        &unixsocketChannelType,
        tclname,
        cstate,
        (TCL_READABLE | TCL_WRITABLE));
    Tcl_RegisterChannel(state->interp, cstate->channel);
    Tcl_SetChannelOption(state->interp,
                                    cstate->channel, "-translation", "binary");

    Tcl_Obj *fun = Tcl_DuplicateObj(state->srv_acceptfun);
    Tcl_ListObjAppendElement(state->interp,fun,Tcl_NewStringObj(tclname, -1));

    int res = Tcl_EvalObjEx(state->interp, fun, TCL_EVAL_GLOBAL);

    if (res != TCL_OK) {
        Tcl_BackgroundError(state->interp);
        unixsocketCloseProc(cstate, state->interp);
        return;
    }

    unixsocket_state_t *tail = state;
    while (tail->next)
        tail = tail->next;

    tail->next = cstate;
    cstate->prev = tail;
}

static int
unixsocket_connect(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    return TCL_OK;
}


static int
unixsocket_listen(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{

    fprintf(stderr, "listen called\n");

    if (objc != 3) {
        Tcl_Obj *res = Tcl_GetObjResult(interp);
        Tcl_AppendStringsToObj(res,
            "Wrong # of arguments.  Must be \"2\"", NULL);
        return TCL_ERROR;
    }

    //
    // unix socket path
    //
    int   str_len;
    char *str = Tcl_GetStringFromObj(objv[1], &str_len);
    if (str_len > 107) {
        Tcl_Obj *res = Tcl_GetObjResult(interp);
        Tcl_AppendStringsToObj(res, "path cannot exceed 107 characters", NULL);
        return TCL_ERROR;
    }

    char *path = ckalloc(strlen(str) + 1);
    strcpy(path, str);

    //
    // Delete socket path if present
    //
    unlink(path);

    //
    // Create socket
    //
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    //
    // did not understant COLEXEC, but seams required
    //
    fcntl(sock, F_SETFD, FD_CLOEXEC);

    //
    // Give it a name
    //
    struct sockaddr_un name;
    memset(&name, 0, sizeof(struct sockaddr_un));

    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, path, sizeof(name.sun_path) - 1);

    if (bind(sock,(struct sockaddr *) &name,sizeof(name)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    //
    // Set this socket as passive and only use it to "accept incomming
    // connexions
    //
    if (listen(sock, LISTEN_BACKLOG) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //
    // Set up tcl channel stuff
    //
    unixsocket_state_t *state = ckalloc(sizeof(unixsocket_state_t));
    state->type = UNIXSOCK_SERVER;
    state->next = NULL;
    state->prev = NULL;
    state->fd   = sock;
    state->srv_path = path;
    state->srv_acceptfun = objv[2];
    state->interp = interp;
    Tcl_IncrRefCount(objv[2]);
    Tcl_CreateFileHandler(
        sock, TCL_READABLE, unixsocketet_accept, (ClientData) state);

    char tclname[SOCKNAME_SIZE];
    sprintf(tclname, SOCKNAME_TEMPLATE, state->fd);

    state->channel = Tcl_CreateChannel(
                        &unixsocketChannelType, tclname, state, 0);

    return TCL_OK;
}

static int
unixsocketBlockModeProc(
    ClientData cdata,
    int        mode)
{
    fprintf(stderr, "block mdoe proc called\n");
    unixsocket_state_t *state = (unixsocket_state_t*) cdata;
    int err;

    int flags = fcntl(state->fd, F_GETFL);
    if (mode == TCL_MODE_BLOCKING) {
        flags &= ~O_NONBLOCK;
    } else {
        //fprintf(stderr, "UNIX SOCKET WARNING: async mode on unix socket is not\n");
        //fprintf(stderr, "UNIX SOCKET WARNING: implemented in this code. \n");
        flags |= O_NONBLOCK;

    }

    if (fcntl(state->fd, F_SETFL, flags) < 0)
        return errno;

    return 0;
}

static void
unixsocketWatchProc(
    ClientData cdata,
    int        mask)
{
    fprintf(stderr, "watch proc called %i\n", mask);
    unixsocket_state_t *state = (unixsocket_state_t*)cdata;

    if (state->type == UNIXSOCK_SERVER)
        return;

    /*
     * On remote connexion reset at least, read/recv indefinitely return 0 len
     * messages, and this tcl channel thing do not detect the exception. So
     * adding this TCL_EXCEPTION here fixes it.
     *
     * Ontoher way to fix it is in the read proc, if the read is 0, return -1
     * with errno=ECONNRESET.
     *
     * Do not know what is the best.
     */
    //mask |= TCL_EXCEPTION;
    if (mask) {
        Tcl_CreateFileHandler(
            state->fd,
            mask,
            (Tcl_FileProc*) Tcl_NotifyChannel,
            (ClientData) state->channel);
    } else {

        Tcl_DeleteFileHandler(state->fd);

    }
}

static int
unixsocketInputProc(
    ClientData cdata,
    char      *buf,
    int        bufSize,
    int       *errorCodePtr)
{
    int got;
    unixsocket_state_t *state = (unixsocket_state_t*)cdata;

    *errorCodePtr = 0;
    got = recv(state->fd, buf, (size_t) bufSize, 0);
    if (got == -1)
        *errorCodePtr = errno;

    fprintf(stderr, "input proc called %i %i %s\n", state->fd, got, buf);
    if (got == 0) {
        *errorCodePtr = ECONNRESET;
        return -1;
    }

    return got;
}

static int
unixsocketOutputProc(
    ClientData  cdata,
    const char *buf,
    int         toWrite,
    int *       errorCodePtr)
{
    fprintf(stderr, "output proc called\n");
    unixsocket_state_t *state = (unixsocket_state_t*)cdata;

    int wrote;
    wrote = send(state->fd, buf, (size_t)toWrite, 0);

    if (wrote == -1)
        *errorCodePtr = errno;

    return wrote;

}

static int
unixsocketCloseProc(
    ClientData  cdata,
    Tcl_Interp *interp)
{
    fprintf(stderr, "close proc called\n");
    unixsocket_state_t *state = (unixsocket_state_t*) cdata;
    int errorCode = 0;

    if (state->type == UNIXSOCK_SERVER) {

        unixsocket_state_t *child = state->next;
        while (child) {
            Tcl_DeleteFileHandler(child->fd);
            if (close(child->fd) < 0)
                errorCode = errno;
            unixsocket_state_t *next = child->next;
            ckfree(child);
            child = next;
        }
        Tcl_DeleteFileHandler(state->fd);
        if (close(state->fd) < 0)
            errorCode = errno;
        unlink(state->srv_path);
        ckfree(state->srv_path);
        Tcl_DecrRefCount(state->srv_acceptfun);
        ckfree(state);

    } else {

        Tcl_DeleteFileHandler(state->fd);
        if (close(state->fd) < 0)
            errorCode = errno;

        if (state->prev)
            state->prev->next = state->next;
        if (state->next)
            state->next->prev = state->prev;
        ckfree(state);

    }

    return errorCode;
}

static int
unixsocketGetHandleProc(
    ClientData  cdata,
    int         direction,
    ClientData *handlePtr)
{
    unixsocket_state_t *state = (unixsocket_state_t*)cdata;

    *handlePtr = (ClientData) (intptr_t) state->fd;

    return TCL_OK;
}

