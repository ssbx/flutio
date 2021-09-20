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

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <tcl.h>

#define SOCKNAME_LENGTH   (10 + sizeof(void*) * 2 + 1)
#define SOCKNAME_TEMPLATE "unixsocket%lx"
#define LISTEN_BACKLOG     10

#if 0
#define USOCKET_TRACE(...) fprintf(stderr, __VA_ARGS__)
#else
#define USOCKET_TRACE(...)
#endif

typedef enum { UNIXSOCK_SERVER, UNIXSOCK_CLIENT } UnixSocketType_T;
typedef struct _UnixSocketState_T UnixSocketState_T;
typedef struct _UnixSocketState_T {
    Tcl_Channel        channel;
    Tcl_Interp        *interp;
    int                fd;
    UnixSocketType_T   type;
    UnixSocketState_T *prev;
    UnixSocketState_T *next;
    char              *path;
    Tcl_Obj           *srv_acceptfun;
} UnixSocketState_T;

static int  UnixSocket_Listen(
    ClientData,Tcl_Interp*,int,Tcl_Obj*const objv[]);
static int  UnixSocket_Open(
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
    Tcl_Namespace *nsPtr;

    if (Tcl_InitStubs(interp, TCL_VERSION, 0) == NULL)
        return TCL_ERROR;

    if ((nsPtr = Tcl_CreateNamespace(interp, "::unixsocket", NULL, NULL)) == NULL)
        return TCL_ERROR;

    if (Tcl_PkgProvide(interp, "unixsocket", "1.0") != TCL_OK)
        return TCL_ERROR;

    Tcl_CreateObjCommand(interp,
            "::unixsocket::listen", UnixSocket_Listen, NULL, NULL);
    Tcl_CreateObjCommand(interp,
            "::unixsocket::open", UnixSocket_Open, NULL, NULL);

    return TCL_OK;
}

static void
UnixSocket_Accept(
    ClientData data,
    int        mask)
{
    UnixSocketState_T *state = (UnixSocketState_T*) data;

    struct sockaddr_un addr;
    socklen_t len = sizeof(struct sockaddr_un);
    int sock = accept(state->fd, (struct sockaddr*) &addr, &len);
    if (sock < 0) {
        return;
    }

    USOCKET_TRACE("accept! %i\n", sock);
    fcntl(sock, F_SETFD, FD_CLOEXEC);

    UnixSocketState_T *cstate = ckalloc(sizeof(UnixSocketState_T));
    cstate->type = UNIXSOCK_CLIENT;
    cstate->next = NULL;
    cstate->prev = NULL;
    cstate->fd   = sock;
    cstate->path = NULL;
    cstate->srv_acceptfun = NULL;
    cstate->interp = NULL;

    char tclname[SOCKNAME_LENGTH];
    sprintf(tclname, SOCKNAME_TEMPLATE, (long) cstate);

    USOCKET_TRACE("accept called %s\n", tclname);
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

    UnixSocketState_T *tail = state;
    while (tail->next)
        tail = tail->next;

    tail->next = cstate;
    cstate->prev = tail;
}

static int
UnixSocket_Open(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{
    if (objc != 2) {
        Tcl_SetObjResult(interp,
            Tcl_NewStringObj("unixsocket::open requires one argument", -1));
        return TCL_ERROR;
    }

    int pathLen;
    char* serverPath = Tcl_GetStringFromObj(objv[1], &pathLen);
    if (pathLen > 107) {
    Tcl_SetObjResult(interp,
            Tcl_NewStringObj("unixsocket::open requires one argument", -1));
        return TCL_ERROR;

    }

    if (access(serverPath, R_OK | W_OK) != 0) {
        Tcl_SetObjResult(interp, Tcl_ObjPrintf(
                    "unixsocket::open couldn't access path: %s", serverPath));
        return TCL_ERROR;
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    fcntl(fd, F_SETFD, FD_CLOEXEC);

    struct sockaddr_un address;
    strcpy(address.sun_path, serverPath);
    address.sun_family = AF_UNIX;

    int ret = connect(fd, (struct sockaddr*) &address,
                                            sizeof(struct sockaddr_un));
    if (ret == -1) {
        Tcl_SetObjResult(interp, Tcl_ObjPrintf(
                "unixsocket::open couldn't open socket: %s", serverPath));
        return TCL_ERROR;
    }


    UnixSocketState_T* state = (UnixSocketState_T*)
                                    ckalloc(sizeof(UnixSocketState_T));

    char channelName[SOCKNAME_LENGTH];
    sprintf(channelName, SOCKNAME_TEMPLATE, (long) state);
    state->next = NULL;
    state->prev = NULL;
    state->type = UNIXSOCK_CLIENT;
    state->interp = interp;
    state->path = ckalloc(strlen(serverPath) + 1);
    state->fd = fd;
    strcpy(state->path, serverPath);

    state->channel = Tcl_CreateChannel(&unixsocketChannelType, channelName,
        (ClientData) state, (TCL_READABLE | TCL_WRITABLE));

    Tcl_RegisterChannel(interp, state->channel);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(channelName, -1));

    return TCL_OK;
}


static int
UnixSocket_Listen(
    ClientData     cdata,
    Tcl_Interp    *interp,
    int            objc,
    Tcl_Obj *const objv[])
{

    USOCKET_TRACE("listen called\n");

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
    UnixSocketState_T *state = ckalloc(sizeof(UnixSocketState_T));
    state->type = UNIXSOCK_SERVER;
    state->next = NULL;
    state->prev = NULL;
    state->fd   = sock;
    state->path = path;
    state->srv_acceptfun = objv[2];
    state->interp = interp;
    Tcl_IncrRefCount(objv[2]);
    Tcl_CreateFileHandler(
        sock, TCL_READABLE, UnixSocket_Accept, (ClientData) state);

    char tclname[SOCKNAME_LENGTH];
    sprintf(tclname, SOCKNAME_TEMPLATE, (long) state);

    state->channel = Tcl_CreateChannel(
                        &unixsocketChannelType, tclname, state, 0);

    return TCL_OK;
}

static int
unixsocketBlockModeProc(
    ClientData cdata,
    int        mode)
{
    USOCKET_TRACE("block mdoe proc called\n");
    UnixSocketState_T *state = (UnixSocketState_T*) cdata;
    int err;

    int flags = fcntl(state->fd, F_GETFL);
    if (mode == TCL_MODE_BLOCKING) {
        flags &= ~O_NONBLOCK;
    } else {
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
    USOCKET_TRACE("watch proc called %i\n", mask);
    USOCKET_TRACE(" %i %i %i\n", TCL_READABLE, TCL_WRITABLE, TCL_EXCEPTION);
    UnixSocketState_T *state = (UnixSocketState_T*)cdata;

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
    UnixSocketState_T *state = (UnixSocketState_T*)cdata;

    *errorCodePtr = 0;
    got = recv(state->fd, buf, (size_t) bufSize, 0);
    if (got == -1)
        *errorCodePtr = errno;

    USOCKET_TRACE("input proc called %i %i %s\n", state->fd, got, buf);
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
    USOCKET_TRACE("output proc called\n");
    UnixSocketState_T *state = (UnixSocketState_T*)cdata;

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
    USOCKET_TRACE("close proc called\n");
    UnixSocketState_T *state = (UnixSocketState_T*) cdata;
    int errorCode = 0;

    if (state->type == UNIXSOCK_SERVER) {

        UnixSocketState_T *child = state->next;
        while (child) {
            Tcl_DeleteFileHandler(child->fd);
            if (close(child->fd) < 0)
                errorCode = errno;
            UnixSocketState_T *next = child->next;
            ckfree(child);
            child = next;
        }
        Tcl_DeleteFileHandler(state->fd);
        if (close(state->fd) < 0)
            errorCode = errno;
        unlink(state->path);
        ckfree(state->path);
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
        if (state->path)
            ckfree(state->path);

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
    UnixSocketState_T *state = (UnixSocketState_T*)cdata;

    *handlePtr = (ClientData) (intptr_t) state->fd;

    return TCL_OK;
}

