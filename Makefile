CURABSPATH=$(dir $(abspath $(firstword $(MAKEFILE_LIST))))

SRCDIR=source
INCDIR=include
OBJDIR=.obj
DEPDIR=.dep
LIBDIR=libary
LIBNAME=ecor

POSTFIX=cpp

export CURABSPATH
export OBJDIR
export DEPDIR
export LIBDIR
export LIBNAME

SOURCES=$(notdir $(wildcard $(SRCDIR)/*.$(POSTFIX)))
OBJS=$(patsubst %.$(POSTFIX),%.o,$(SOURCES))

CBASEFLAGS= -Wall -I$(CURABSPATH)$(INCDIR) -std=c++0x -Wno-invalid-offsetof -DNDEBUG -lstdc++ 
OPTIMIZELEVEL=-o2
CFLAGS=$(CBASEFLAGS) $(OPTIMIZELEVEL)

export CBASEFLAGS
export OPTIMIZELEVEL
export CFLAGS

vpath %.h           $(INCDIR)
vpath %.$(POSTFIX)  $(SRCDIR)
vpath %.o           $(OBJDIR)
vpath %.d           $(DEPDIR)

.PHONY: release techosvr

release:$(LIBDIR)/lib$(LIBNAME).a

all:release techosvr

techosvr:echosvr

echosvr:release
	cd example/$(patsubst %.eg,%,$(notdir $@)) && $(MAKE)

$(LIBDIR)/lib$(LIBNAME).a:$(OBJS)
	@mkdir -p $(LIBDIR)
	ar -rcs $(LIBDIR)/lib$(LIBNAME).a $(patsubst %,$(OBJDIR)/%,$(OBJS))

%.o:%.$(POSTFIX)
	@mkdir -p $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $(OBJDIR)/$@

$(DEPDIR)/%.d:%.$(POSTFIX)
	@mkdir -p $(DEPDIR)
	@$(RM) $@;
	$(CC) -MM $(CBASEFLAGS) $< | sed 's/\($(patsubst %.$(POSTFIX),%,$(notdir $<))\)\.o *: */\1.o \1.d:/' > $@

ifneq ($(MAKECMDGOALS),clean)
sinclude $(patsubst %.o,$(DEPDIR)/%.d,$(OBJS))
endif

clean:
	$(RM) $(OBJDIR)/*.o
	$(RM) $(DEPDIR)/*.d
	$(RM) $(LIBDIR)/lib$(LIBNAME).a
	cd example/echosvr && $(MAKE) clean

