.SUFFIXES:

ifeq ($(strip $(DEVKITSH4)),)
$(error "Please set DEVKITSH4 in your environment. export DEVKITSH4=<path to sdk>")
endif
include $(DEVKITSH4)/exword_rules

TARGET       := savimp
MODNAME      := savimp
APPTITLE     := Gnuboy Save Importer
APPID        := SAVWR
APPMOD       := $(TARGET).d01

# User-supplied inputs. The save data is converted into a generated C source
# under build/ and is never copied into the release tree.
SAVE_FILE    ?=
SAVE_TARGET  ?=
SAVE_DRIVE   ?= drv0
PYTHON       ?= python3

SOURCEDIR    := src
HTMLDIR      := html
INSTALLDIR   := $(HOME)/.local/share/exword
BUILDS       := ja cn
GENERATEDDIR := build/generated
PAYLOAD_C    := $(GENERATEDDIR)/payload.c
CFILES       := $(SOURCEDIR)/main.c $(PAYLOAD_C) $(wildcard $(SOURCEDIR)/libc/*.c)
SFILES       := $(wildcard $(SOURCEDIR)/*.s) $(wildcard $(SOURCEDIR)/libc/*.s)
OBJECTS      := $(CFILES:.c=.o) $(SFILES:.s=.o)

CC_OPTS      :=
LDFLAGS      := -Wall -std=gnu17 -nostdlib -L$(DEVKITPRO)/libdataplus/lib -ldataplus -lgraphics -lsh4a
CFLAGS       := -Wall -std=gnu17 -fno-builtin -I$(DEVKITPRO)/libdataplus/include -I$(SOURCEDIR) -I$(SOURCEDIR)/libc/include -O3 $(CC_OPTS)
ASFLAGS      := -Wall -std=gnu17 -m4-nofpu

app: $(addprefix build/,$(addsuffix /$(APPID),$(BUILDS)))

ifeq ($(filter clean,$(MAKECMDGOALS)),)
ifeq ($(strip $(SAVE_FILE)),)
$(error SAVE_FILE is required, for example: make SAVE_FILE=game.sav SAVE_TARGET=game.sav)
endif
ifeq ($(strip $(SAVE_TARGET)),)
$(error SAVE_TARGET is required, for example: make SAVE_FILE=game.sav SAVE_TARGET=game.sav)
endif
endif

$(PAYLOAD_C): $(SAVE_FILE) tools/embed_save.py
	@mkdir -p $(GENERATEDDIR)
	$(PYTHON) tools/embed_save.py --input "$(SAVE_FILE)" --target "$(SAVE_TARGET)" --drive "$(SAVE_DRIVE)" --output "$@"

$(SOURCEDIR)/main.o: $(PAYLOAD_C)

.SECONDEXPANSION:
build/%/$(APPID): $(TARGET).d01 $$(wildcard $(HTMLDIR)/$$*/*.htm)
	@echo building $* version in $@...
	@mkdir -p $@
	@cp $(TARGET).d01 $@
	@for f in $(HTMLDIR)/$*/*.htm; do \
		sed -e 's/@APPTITLE/$(APPTITLE)/g' -e 's/@APPID/$(APPID)/g' -e 's/@APPMOD/$(APPMOD)/g' $$f > $@/$$(basename $$f); \
	done
	@touch $@/fileinfo.cji

$(TARGET).elf: $(OBJECTS)

install: app
	@echo installing to $(INSTALLDIR)...
	@mkdir -p $(INSTALLDIR)
	@cp -r build/* $(INSTALLDIR)/
	@echo 'You can now install this app to EX-word by `dict install $(APPID)` in libexword.'

clean:
	@echo clean $(OBJECTS) $(TARGET).elf $(TARGET).elf.map $(TARGET).d01
	@rm -fr build $(filter-out $(PAYLOAD_C:.c=.o),$(OBJECTS)) $(TARGET).elf $(TARGET).elf.map $(TARGET).d01
