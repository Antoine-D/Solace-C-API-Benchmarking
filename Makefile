ifeq ($(findstring Rules.mk, $(wildcard *.mk)), Rules.mk)
   include Rules.mk
endif

CXX:=gcc -g

ifeq ($(CCSMPHOME),)
	CCSMPHOME:=.
endif

INCDIRS:=-I${CCSMPHOME} \
	 -I$(CCSMPHOME)/include \
	 -I.

_OS=$(shell uname)
_ARCH := $(shell uname -m)
ifeq ($(_OS),HP-UX)
    _CPU=$(shell uname -m)
else
    _CPU=$(shell uname -p)
endif
_REV=$(shell uname -r)
ARCHFLAGS:=
ifeq ($(SIXTY_FOUR_COMPAT),) 
ifeq ($(_ARCH),x86_64)
    ARCHFLAGS:=-D_LINUX_X86_64
    SIXTY_FOUR_COMPAT:=
    LINUXLIBDIR=$(CCSMPHOME)/obj_Linux26-x86_64_debug/lib
else
ifeq ($(_ARCH),armv6l)
    SIXTY_FOUR_COMPAT:=
    LINUXLIBDIR=$(CCSMPHOME)/obj_RaspberryPi-arm_debug/lib
else
    SIXTY_FOUR_COMPAT:=-m32
    LINUXLIBDIR=$(CCSMPHOME)/obj_Linux26-i386_debug/lib
endif
endif
ifeq ($(findstring 2.4, $(_REV)), 2.4)
    LINUXLIBDIR=$(CCSMPHOME)/obj_Linux24-i386_debug/lib
endif
endif
ifeq ($(_ARCH),ppc64le)
    SIXTY_FOUR_COMPAT:= -pthread
    LINUXLIBDIR=$(CCSMPHOME)/obj_Linux319-ppc64le_debug/lib -pthread
    LLSYS=$(SIXTY_FOUR_COMPAT) -lrt -pthread
endif
ifeq ($(_ARCH),ppc64)
    SIXTY_FOUR_COMPAT:= -pthread
    LINUXLIBDIR=$(CCSMPHOME)/obj_Linux30-ppc64_debug/lib -pthread
    LLSYS=$(SIXTY_FOUR_COMPAT) -lrt -pthread
endif
ifeq ($(_OS), HP-UX)
    LIBDIRS:=-L$(CCSMPHOME)/lib -L$(CCSMPHOME)/obj_HP-UX-ia64_debug/lib
    SIXTY_FOUR_COMPAT:=-mlp64 -pthread
    LINUXLIBDIR=$(CCSMPHOME)/obj_HP-UX-ia64_debug/lib
    LLSYS:= $(SIXTY_FOUR_COMPAT) -lrt -lxnet -pthread
endif
ifeq ($(_OS), AIX)
    SIXTY_FOUR_COMPAT:=-maix64 -pthread -DSOLCLIENT_AIX_BUILD
    LINKFLAGS:= -Xlinker -brtl 
    LIBDIRS:=-L$(CCSMPHOME)/lib -L$(CCSMPHOME)/obj_AIX6-powerpc_64_debug/lib
    LLSYS:= $(SIXTY_FOUR_COMPAT) -lrt -pthread
endif
ifeq ($(_OS), SunOS)
ifeq ($(_CPU), sparc)
    LIBDIRS:=-L$(CCSMPHOME)/lib -L$(CCSMPHOME)/obj_SunOS10-sparc_debug/lib
else
    LIBDIRS:=-L$(CCSMPHOME)/lib -L$(CCSMPHOME)/obj_SunOS10-i386_debug/lib
endif
    LLSYS:= -lsocket $(SIXTY_FOUR_COMPAT) -lrt
else
    LIBDIRS:=-L$(CCSMPHOME)/lib -L$(LINUXLIBDIR)
    LLSYS:= $(SIXTY_FOUR_COMPAT)
endif

COMPILEFLAG:= $(COMPILEFLAG) $(INCDIRS) $(ARCHFLAGS) -DPROVIDE_LOG_UTILITIES -g
LINKFLAGS:= $(LIBDIRS) -lsolclient $(LLSYS) -L$(CCSMPHOME)/include/jsmn -ljsmn
DEPENDS:= os.o
DEPENDS:= jsmn/libjsmn.a
DEPENDS:= config.o

%.o:	%.c
	$(CXX) $(COMPILEFLAG) $(SIXTY_FOUR_COMPAT)  -c $< -o $@

EXECS:= SolaceProducer SolaceConsumer

all: $(EXECS)

clean:
	rm -f $(EXECS) SolaceProducer.o SolaceConsumer.o os.o config.o

SolaceProducer : SolaceProducer.o $(DEPENDS) 
	$(CXX) -o $@ $^ $(LINKFLAGS)

SolaceConsumer : SolaceConsumer.o $(DEPENDS)
	$(CXX) -o $@ $^ $(LINKFLAGS)

$(shell export LD_LIBRARY=../lib)
