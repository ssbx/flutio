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

RM     ?= rm -f
CC 	   ?= cc
CFLAGS += -fPIC -I../include

default: all


#######################################################################
# inputLibsndfile.so
INPUT_SNDFILE = inputLibsndfile
INPUT_SNDFILE_LDFLAGS = -lsndfile
$(INPUT_SNDFILE).so: $(INPUT_SNDFILE).o
	$(CC) -shared -o $(INPUT_SNDFILE).so $(INPUT_SNDFILE).o $(INPUT_SNDFILE_LDFLAGS)

#######################################################################
# outputAlsa.so
OUTPUT_ALSA   = outputAlsa
OUTPUT_ALSA_LDFLAGS = -lasound -lpthread
$(OUTPUT_ALSA).so: $(OUTPUT_ALSA).o
	$(CC) -shared -o $(OUTPUT_ALSA).so $(OUTPUT_ALSA).o $(OUTPUT_ALSA_LDFLAGS)



.PHONY: all
all: \
	$(INPUT_SNDFILE).so \
	$(OUTPUT_ALSA).so

.PHONY: update
update:
	$(CC) -MM -I../include *c > Makefile.objects

.PHONY: clean
clean:
	$(RM) $(INPUT_SNDFILE).so $(INPUT_SNDFILE).o
	$(RM) $(OUTPUT_ALSA).so $(OUTPUT_ALSA).o
	$(RM) $(OTHER)

include Makefile.objects

