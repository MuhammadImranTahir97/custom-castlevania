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
# Export absolute butano path:
#---------------------------------------------------------------------------------------------------------------------
ifndef LIBBUTANOABS
	export LIBBUTANOABS	:=	$(realpath $(LIBBUTANO))
endif

#---------------------------------------------------------------------------------------------------------------------
# Include main makefile:
#---------------------------------------------------------------------------------------------------------------------
include $(LIBBUTANOABS)/butano.mak
