SRCS =	$(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
PRINTF_CHK_DLL = printf_chk.dll

DEBUG_FLAG = -g
PIC_FLAG = -fPIC
CXXFLAGS = $(DEBUG_FLAG) $(PIC_FLAG) -I$(HCC1_SRCDIR) -w

UNAME := $(shell uname)
DLL_FLAG =  -shared
ifneq (,$(findstring Darwin,$(UNAME)))
	DLL_FLAG = -dynamiclib
endif

turbo = $(if $(wildcard /etc/turbolinux-release),1,0)
ifeq ($(turbo),1)
  CXXFLAGS += -DTURBO_LINUX
endif

RM = rm -r -f

all:$(PRINTF_CHK_DLL)

$(PRINTF_CHK_DLL) : $(OBJS)
	$(CXX) $(DEBUG_FLAG) $(PROF_FLAG) $(DLL_FLAG) -o $@ $(OBJS)

clean:
	$(RM) *.o *~ $(PRINTF_CHK_DLL) x64 Debug .vs
	$(RM) -r .vs x64 Debug Release
