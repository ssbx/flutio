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

export RM      = rm -f
export MAKE    = make
export INSTALL = install
export LN      = ln

LIBMPDNG  = libtclmpdng.so

CFLAGS  = -fPIC -DUSE_TCL_STUBS -I/usr/include/tcl8.6 -Iinclude
LDFLAGS = -L/usr/lib -ltclstub8.6

#
# Default target
#
.PHONY: all
all: $(LIBMPDNG)
	$(MAKE) -C plugins
	$(MAKE) -C pkgs

#
# libtclmpdng.so
#
MPDNG_OBJECTS = tclmpdng.o player.o plugins.o track.o inputs.o \
                 outputs.o pre_outputs.o post_inputs.o fade.o
MPDNG_LDFLAGS = $(LDFLAGS) -lpthread
$(LIBMPDNG): $(MPDNG_OBJECTS)
	$(CC) -shared -o $(LIBMPDNG) $(MPDNG_OBJECTS) $(MPDNG_LDFLAGS)


#
# clean
#
.PHONY: clean
clean:
	$(RM) $(MPDNG_OBJECTS) $(LIBMPDNG)
	$(MAKE) -C plugins clean
	$(MAKE) -C pkgs clean

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
prefix         = $(HOME)
bindir         = $(prefix)/bin
mandir         = $(prefix)/man
incdir         = $(prefix)/include
libexecdir     = $(prefix)/libexec
mpdng_inst_dir = $(libexecdir)/mpd-ng
mpdng_inc_dir  = $(incdir)/mpd-ng
export tclpkgdir = $(mpdng_inst_dir)/pkgs
install: all
	$(INSTALL) -d -m 755 '$(DESTDIR)$(bindir)'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(mandir)'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(mpdng_inc_dir)/plugins'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(mpdng_inc_dir)/interfaces'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(mpdng_inst_dir)'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(mpdng_inst_dir)/plugins'
	$(INSTALL) -d -m 755 '$(DESTDIR)$(mpdng_inst_dir)/pkgs'
	$(INSTALL) -m 444 $(LIBMPDNG) '$(DESTDIR)$(mpdng_inst_dir)'
	$(INSTALL) -m 444 plugins/*.so '$(DESTDIR)$(mpdng_inst_dir)/plugins'
	$(INSTALL) -m 444 include/mpd-ng/plugins/*.h '$(DESTDIR)$(mpdng_inc_dir)/plugins'
	$(INSTALL) -m 444 include/mpd-ng/interfaces/*.h '$(DESTDIR)$(mpdng_inc_dir)/plugins'
	$(INSTALL) -m 555 mpd-ng '$(DESTDIR)$(mpdng_inst_dir)'
	$(LN) -fs '$(mpdng_inst_dir)/mpd-ng' '$(DESTDIR)$(bindir)'
	make -C pkgs install

