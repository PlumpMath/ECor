SCURABSPATH=$(dir $(abspath $(firstword $(MAKEFILE_LIST))))

SRCDIR=source
SINCDIR=include

POSTFIX=cpp
OUTFILENAME=outfile.out
TESTPOSTFIX=.test

SOURCES=$(notdir $(wildcard $(SRCDIR)/*.$(POSTFIX)))
OBJS=$(patsubst %.$(POSTFIX),%.o,$(SOURCES))

vpath %.h           $(INCDIR)
vpath %.$(POSTFIX)  $(SRCDIR)
vpath %.o           $(OBJDIR)
vpath %.d           $(DEPDIR)

.PHONY: techosvr release

SCFLAGS=-I$(SCURABSPATH)$(SINCDIR)

release:$(CURABSPATH)echosvr

$(CURABSPATH)echosvr:$(OBJS)
	$(CC) $(SCFLAGS) $(CFLAGS) -o $@ -L$(CURABSPATH)$(LIBDIR) $(patsubst %,$(OBJDIR)/%,$(OBJS)) -l$(LIBNAME)

%.o:%.$(POSTFIX)
	@mkdir -p $(OBJDIR)
	$(CC) -c $(SCFLAGS) $(CFLAGS) $< -o $(OBJDIR)/$@

$(DEPDIR)/%.d:%.$(POSTFIX)
	@mkdir -p $(DEPDIR)
	@$(RM) $@;
	@$(CC) -MM $(SCFLAGS) $(CBASEFLAGS) $< | sed 's/\($(patsubst %.$(POSTFIX),%,$(notdir $<))\)\.o *: */\1.o \1.d:/' > $@

ifneq ($(MAKECMDGOALS),clean)
sinclude $(patsubst %.o,$(DEPDIR)/%.d,$(OBJS))
endif

clean:
	$(RM) $(OBJDIR)/*.o
	$(RM) $(DEPDIR)/*.d
	$(RM) $(CURABSPATH)echosvr
