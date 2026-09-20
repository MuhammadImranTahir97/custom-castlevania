#---------------------------------------------------------------------------------------------------------------------
# See tools/butano/template/Makefile for the full field reference.
#---------------------------------------------------------------------------------------------------------------------
TARGET      	:=  cot-hack
BUILD       	:=  build
LIBBUTANO   	:=  tools/butano/butano
PYTHON      	:=  python
SOURCES     	:=  src/game src/platform generated/src
INCLUDES    	:=  src/game src/platform generated/include
DATA        	:=
GRAPHICS    	:=  assets/sprites
AUDIO       	:=
AUDIOBACKEND	:=  maxmod
AUDIOTOOL		:=
DMGAUDIO    	:=
DMGAUDIOBACKEND	:=  default
ROMTITLE    	:=  COTM DEV
ROMCODE     	:=  CTHK
USERFLAGS   	:=
USERCXXFLAGS	:=
USERASFLAGS 	:=
USERLDFLAGS 	:=
USERLIBDIRS 	:=
USERLIBS    	:=
DEFAULTLIBS 	:=
STACKTRACE		:=
USERBUILD   	:=  generated
EXTTOOL     	:=  @$(PYTHON) -B tools/convert_rooms.py --rooms-dir assets/rooms --build generated

#---------------------------------------------------------------------------------------------------------------------
# Local toolchain config (WONDERFUL_TOOLCHAIN or DEVKITARM): .env is
# gitignored and machine-specific -- see .env.example and CLAUDE.md's
# "Build environment" section. Loading it here means `make` finds the
# toolchain on its own, without relying on the variable already being set
# in whatever shell invoked make.
#---------------------------------------------------------------------------------------------------------------------
-include .env

# tools/butano/butano/butano.mak picks DEVKITARM over WONDERFUL_TOOLCHAIN
# whenever DEVKITARM is non-empty, with no way to say "prefer Wonderful."
# A stray/incorrect DEVKITARM left over in a shell's environment (not from
# our .env) would silently hijack the build onto the wrong toolchain, so
# when .env asks for Wonderful, we make sure it wins.
ifneq ($(strip $(WONDERFUL_TOOLCHAIN)),)
	override DEVKITARM :=
endif

export WONDERFUL_TOOLCHAIN
export DEVKITARM

# The compiler binaries under $(WONDERFUL_TOOLCHAIN)/toolchain/.../bin need
# the shared MinGW runtime DLLs (libgcc_s_seh-1.dll, libiconv-2.dll, ...)
# that live in $(WONDERFUL_TOOLCHAIN)/bin itself -- normally added to PATH
# by Wonderful's own installer. Add it here too so a plain `make` doesn't
# depend on that having happened in this shell.
ifneq ($(strip $(WONDERFUL_TOOLCHAIN)),)
	export PATH := $(WONDERFUL_TOOLCHAIN)/bin:$(PATH)
endif

# GCC needs a writable TMP/TEMP for its intermediate files. These may not
# reach the compiler's environment from whatever shell invoked `make`
# (observed on Windows: TMP/TEMP set in the calling shell aren't inherited
# by the toolchain's recursive sub-make), and when neither is set the
# Win32 fallback is the Windows install directory itself, which isn't
# writable by a normal user. Point both at our own build dir instead --
# it always exists and is always writable.
ifndef TMP
	export TMP := $(CURDIR)/$(BUILD)
endif
ifndef TEMP
	export TEMP := $(CURDIR)/$(BUILD)
endif

#---------------------------------------------------------------------------------------------------------------------
# Export absolute butano path:
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANOABS
	export LIBBUTANOABS	:=	$(realpath $(LIBBUTANO))
endif

#---------------------------------------------------------------------------------------------------------------------
# Include main makefile:
#---------------------------------------------------------------------------------------------------------------------
include $(LIBBUTANOABS)/butano.mak
