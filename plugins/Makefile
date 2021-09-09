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

