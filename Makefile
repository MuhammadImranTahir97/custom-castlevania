#---------------------------------------------------------------------------------------------------------------------
# See tools/butano/template/Makefile for the full field reference.
#---------------------------------------------------------------------------------------------------------------------
TARGET      	:=  cot-hack
BUILD       	:=  build
LIBBUTANO   	:=  tools/butano/butano
PYTHON      	:=  python
SOURCES     	:=  src/game src/platform
INCLUDES    	:=  src/game src/platform
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
USERBUILD   	:=
EXTTOOL     	:=

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
