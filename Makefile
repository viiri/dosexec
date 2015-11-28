DEBUG      ?= 0
SYMBOLS    ?= 0
PROFILING  ?= 0
RELEASE    ?= 0
# "Base" interface objects
OBJECTS    += obj/intf/main.o
# System core objects
OBJECTS    += obj/core/util.o
# Emulator core objects
OBJECTS    += obj/emu/emu.o \
		obj/emu/mem.o
# BIOS simulation
OBJECTS    += obj/emu/bios/bios.o \
		obj/emu/bios/mem.o \
		obj/emu/bios/bios_int21h.o
# Program loader
OBJECTS    += obj/emu/loader/loader.o \
		obj/emu/loader/com.o \
		obj/emu/loader/mz.o
# CPU core
OBJECTS  += obj/emu/cpu/new/v30mz.o

TARGET      = dosexec
INCLUDE    += -Isrc/
LIBS       += -lm -liconv
ifeq ($(DEBUG),1)
  SYMBOLS  := 1
  OPTIMIZE  = -O0
else
  OPTIMIZE  = -O2
endif
ifeq ($(SYMBOLS),1)
  CFLAGS   += -g
  LDFLAGS  += -g
endif
ifeq ($(PROFILING),1)
  CFLAGS   += -pg
  LDFLAGS  += -pg
endif
ifeq ($(RELEASE),1)
  CFLAGS   += -DRELEASE=1
else
  CFLAGS   += -DRELEASE=0
  CFLAGS   += -march=native
  LDFLAGS  += -march=native
endif
CFLAGS     += $(OPTIMIZE) $(INCLUDE) -std=gnu99 -Wall -Wextra -Werror
LDFLAGS    += $(LIBS)
CLEANED     = $(OBJECTS) $(TARGET)
INSTALL_PATH	?= /usr/local
dir_guard   = @mkdir -p $(@D)

.PHONY: all clean rebuild install uninstall

all: $(OBJECTS) $(TARGET)

obj/%.o: src/%.c
	$(dir_guard)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJECTS)
	$(dir_guard)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	$(RM) $(CLEANED)
rebuild:
	make clean
	make all

install: $(TARGET)
	install	$(TARGET) $(INSTALL_PATH)/bin
uninstall:
	install	/dev/null $(INSTALL_PATH)/bin/$(TARGET)

obj/emu/cpu/new/v30mz.o: src/emu/cpu/new/v30mz.c src/emu/cpu/new/v30mz.h src/emu/cpu/new/mem.inc.c src/emu/cpu/new/irq.inc.c src/emu/cpu/new/exec.inc.c
