# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     (1) Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#
#     (2) Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
#
#     (3)The name of the author may not be used to
#     endorse or promote products derived from this software without
#     specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

RM      = rm -f
MAKE    = make
INSTALL = install
LN      = ln

FLUTIOLIB  = libtclflutio.so
TAGLIBLIB  = libtcltaglib.so
USOCKETLIB = libtclunixsocket.so
ALLLIBS    = $(FLUTIOLIB) $(USOCKETLIB) $(TAGLIBLIB)

CFLAGS = -fPIC -DUSE_TCL_STUBS -I/usr/include/tcl8.6 -Iinclude
CXXFLAGS = $(CFLAGS)
LDFLAGS = -L/usr/lib -ltclstub8.6

#
# Default target
#
.PHONY: all
all: $(ALLLIBS)
	$(MAKE) -C plugins


#
# libtcltaglib.so
#
TAGLIB_LDFLAGS = $(LDFLAGS) -ltag
TAGLIB_OBJECTS = tcltaglib.o
$(TAGLIBLIB): $(TAGLIB_OBJECTS)
	$(CXX) -shared -o $(TAGLIBLIB) $(TAGLIB_OBJECTS) $(TAGLIB_LDFLAGS)


#
# libtclunixsocket.so
#
USOCKET_LDFLAGS = $(LDFLAGS)
USOCKET_OBJECTS = tclunixsocket.o
$(USOCKETLIB): $(USOCKET_OBJECTS)
	$(CC) -shared -o $(USOCKETLIB) $(USOCKET_OBJECTS) $(USOCKET_LDFLAGS)


#
# libtclflutio.so
#
FLUTIO_OBJECTS = tclflutio.o player.o plugins.o track.o inputs.o \
				outputs.o
FLUTIO_LDFLAGS = $(LDFLAGS) -lpthread
$(FLUTIOLIB): $(FLUTIO_OBJECTS)
	$(CC) -shared -o $(FLUTIOLIB) $(FLUTIO_OBJECTS) $(FLUTIO_LDFLAGS)


#
# clean
#
.PHONY: clean
clean:
	$(RM) $(ALLLIBS) $(FLUTIO_OBJECTS) $(USOCKET_OBJECTS) $(TAGLIB_OBJECTS)
	$(MAKE) -C plugins clean

#
# update
#
.PHONY: update
update:
	$(CC) -MM -Iinclude *c > Makefile.objects
	$(MAKE) -C plugins update

#
# install
#
.PHONY: install
prefix = $(HOME)
bindir = $(prefix)/bin
mandir = $(prefix)/man
incdir = $(prefix)/include
libexecdir = $(prefix)/libexec
install: all
	$(INSTALL) -d -m 755 '$(DESTDIR)$(bindir)'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(mandir)'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(incdir)/flutio/plugins'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(libexecdir)/flutio'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(libexecdir)/flutio/plugins'
	$(INSTALL) -m 444 $(ALLLIBS) '$(DESTDIR)$(libexecdir)/flutio'
	$(INSTALL) -m 444 plugins/*.so '$(DESTDIR)$(libexecdir)/flutio/plugins'
	$(INSTALL) -m 444 include/flutio/plugins/common.h '$(DESTDIR)$(incdir)/flutio/plugins'
	$(INSTALL) -m 444 include/flutio/plugins/input.h '$(DESTDIR)$(incdir)/flutio/plugins'
	$(INSTALL) -m 444 include/flutio/plugins/output.h '$(DESTDIR)$(incdir)/flutio/plugins'
	$(INSTALL) -m 555 flutio '$(DESTDIR)$(libexecdir)/flutio/'
	$(LN) -fs '$(libexecdir)/flutio/flutio' '$(DESTDIR)$(bindir)'

