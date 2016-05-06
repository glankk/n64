# cb-n64.make
# -glank

N64ROOT  := $(abspath $(N64ROOT))
OBJDIR   := $(abspath $(OBJDIR))
OFILE    := $(abspath $(OFILE))
BINDIR    = $(dir $(OFILE))
OFBIN     = $(OFILE).bin
CSRC      = $(wildcard *.c)
CXXSRC    = $(wildcard *.cpp) $(wildcard *.cxx) $(wildcard *.cc) $(wildcard *.c++)
COBJ      = $(patsubst %,$(OBJDIR)/%.o,$(CSRC))
CXXOBJ    = $(patsubst %,$(OBJDIR)/%.o,$(CXXSRC))
OBJ       = $(COBJ) $(CXXOBJ)
GSC       = $(wildcard *.gsc)

all: $(OFBIN)
	$(LUAPATCH) "$(EMUDIR)\Lua\patch-data.lua" $(patsubst %,-text % ,$(GSC)) -bin $(ADDRESS) $(OFBIN)

cleanall:
	-rm -rf $(OBJDIR) $(BINDIR)

$(OFBIN): $(OFILE)
	$(OBJCOPY) $< $@ -O binary

$(OFILE): $(OBJ) | $(BINDIR)
	$(LD) $(LDFLAGS) $^ $(LIB) -o $@

$(COBJ): $(OBJDIR)/%.o: % | $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(CXXOBJ): $(OBJDIR)/%.o: % | $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJDIR):
	-mkdir $(OBJDIR)

$(BINDIR):
	-mkdir $(BINDIR)
