# masagin -- build front door.
#
#     make            the player (./masagin) and the show (build/masagin/)
#     ./masagin       play it, from this folder
#     make run        both of the above in one go
#
#     make deps       check/install what the build needs (only what is missing)
#     make content    the show alone; SHOW=NAME picks another content/ folder
#     make shaders    Slang -> SPIR-V, when a slangc is around
#     make test       the renderer test programs and their measurements
#     make vktest     one test program (also vkbloom, corehello)
#     make EXTRA=-DMSG_VK_FLIP_Y_VIEWPORT=0 ...   compile-time switches, HACKING.md
#     make clean      objects and programs;  make distclean  also build/ and deps/
#
# Toolchains come from env.sh (SLANGC, SDL3_CFLAGS, SDL3_LIBS can be set in the
# environment to override what it finds). The content pipeline stays in
# tools/build_content.py: it already knows how to rebuild incrementally.

CXX := g++
CC  := gcc
OBJ := obj

# ---------------------------------------------------------------- toolchains
# env.sh is sourced once per variable so that values with spaces survive.
SLANGC      := $(shell . ./env.sh 2>/dev/null; printf '%s' "$$SLANGC")
SDL3_CFLAGS := $(shell . ./env.sh 2>/dev/null; printf '%s' "$$SDL3_CFLAGS")
SDL3_LIBS   := $(shell . ./env.sh 2>/dev/null; printf '%s' "$$SDL3_LIBS")

# ---------------------------------------------------------------- flags
# The Altona and msglib sources are C++98; vkcore.cpp and sdl_shim.cpp are C++17.
# sCONFIG_RENDER_BLANK selects the renderer slot that graphics_vk.cpp fills.
CXX98 := -std=gnu++98 -fpermissive -fshort-wchar -fno-exceptions -fno-rtti -msse2 -O1 -g0 -w
CXX98 += -DsCONFIG_RENDER_BLANK -DsCONFIG_BUILD_RELEASE
CXX98 += -Isrc/altona/main -Isrc/altona/main/base -Isrc/libs -Isrc/libs/msglib -Isrc
CXX17 := -std=c++17 -O2 -w $(SDL3_CFLAGS)
CCFL  := -O2 -w
# The compile-time switches of HACKING.md are read by vkcore.cpp, graphics_vk.cpp
# and ipptool.cpp, so EXTRA goes to every group, not only to the C++98 one.
EXTRA ?=
DEPFL := -MMD -MP
LIBS  := $(SDL3_LIBS) -lvulkan -lpthread

# The compile-time switches change what every object means, so the objects of a
# flag set live in a folder named after those flags: switching EXTRA can never
# leave a stale object behind, and going back to a set that was built before
# costs nothing.  The whole player builds in a few seconds, so one hash for all
# the flags is cheaper than working out which switch belongs to which file.
FLAGS := $(shell printf '%s' '$(CXX98)|$(CXX17)|$(CCFL)|$(EXTRA)' | cksum | cut -d' ' -f1)
ODIR  := $(OBJ)/flags-$(FLAGS)

# ---------------------------------------------------------------- sources
# base/*.cpp are all C++98 except the two C++17 files; ogg_example.cpp is an
# example, not part of the build.
BASE98  := $(filter-out src/altona/main/base/vkcore.cpp src/altona/main/base/sdl_shim.cpp,\
                        $(wildcard src/altona/main/base/*.cpp))
MSG98   := $(filter-out src/libs/msglib/ogg_example.cpp,$(wildcard src/libs/msglib/*.cpp))
SRC98   := $(BASE98) $(wildcard src/altona/main/util/*.cpp) $(MSG98) $(wildcard src/masagin/*.cpp)
SRC17   := src/altona/main/base/vkcore.cpp src/altona/main/base/sdl_shim.cpp
SRCC    := src/altona/main/util/stb_image.c

OBJ98  := $(SRC98:src/%.cpp=$(ODIR)/%.o)
OBJ17  := $(SRC17:src/%.cpp=$(ODIR)/%.o)
OBJC   := $(SRCC:src/%.c=$(ODIR)/%.o)
OBJS   := $(OBJ98) $(OBJ17) $(OBJC)
# the test programs bring their own sMain(), so they link without the player's
PLAYER_OBJS := $(filter-out $(ODIR)/masagin/main.o,$(OBJ98))
TEST_OBJS := $(addsuffix .o,$(addprefix $(ODIR)/test/,vktest vkbloom corehello))
$(OBJ17): CXXFLAGS := $(CXX17)
$(OBJ98): CXXFLAGS := $(CXX98)

# ---------------------------------------------------------------- rules
# the goal is named instead of being the first target in the file
.DEFAULT_GOAL := all
.PHONY: all run deps shaders content test clean distclean
.DELETE_ON_ERROR:

all: masagin shaders content

# Dependencies are checked before anything is compiled: setup_deps.sh installs
# only what is missing and is quiet when nothing is, so this costs little and
# repairs a package that went away since the last build.
deps:
	@./setup_deps.sh

masagin: $(OBJS)
	$(CXX) $^ $(LIBS) -o $@

$(OBJS): | deps
$(ODIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(EXTRA) $(DEPFL) -c $< -o $@
$(ODIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CCFL) $(EXTRA) $(DEPFL) -c $< -o $@
$(ODIR)/test/%.o: test/%.cpp | deps
	@mkdir -p $(dir $@)
	$(CXX) $(CXX98) $(EXTRA) $(DEPFL) -c $< -o $@

# vktest and vkbloom drive the player without its main(); corehello goes straight
# to the renderer core, so it links without the player at all.
vktest vkbloom: %: $(ODIR)/test/%.o $(PLAYER_OBJS) $(OBJ17) $(OBJC) | deps
	$(CXX) $^ $(LIBS) -o $@
corehello: $(ODIR)/test/corehello.o $(OBJ17) $(OBJC)
	$(CXX) $^ $(LIBS) -o $@

# The SPIR-V in shaders/ is committed, so the compiler run is cached on the
# content of the Slang sources instead of on their timestamps: checking a tree
# out with equal mtimes does not trigger a rebuild, editing a source does.
# Without a compiler the committed set is only checked for completeness.
SPV := shaders/msg.vsPlain.spv shaders/msg.psPlain.spv shaders/msg.vsGrowable.spv \
       shaders/msg.psGrowable.spv shaders/ipp.vsMain.spv shaders/ipp.psMain.spv \
       shaders/simple.vsMain.spv shaders/simple.psMain.spv
SHSTAMP := $(OBJ)/.shaders-$(shell cat shaders/*.slang 2>/dev/null | cksum | cut -d' ' -f1)
shaders: | deps
	@if [ -z "$(SLANGC)" ]; then \
	  for f in $(SPV); do \
	    [ -f $$f ] || { echo "no $$f and no slangc: ./setup_deps.sh installs the compiler, or set SLANGC"; exit 1; }; \
	  done; \
	  echo "shaders: keeping the SPIR-V committed in shaders/ (no slangc found)"; \
	elif [ -f $(SHSTAMP) ] && ls $(SPV) >/dev/null 2>&1; then : ; \
	else \
	  mkdir -p $(OBJ) && SLANGC="$(SLANGC)" ./compile_shaders.sh && touch $(SHSTAMP); \
	fi

# SHOW selects which folder under content/ is built and played.
SHOW ?= masagin
content: | deps
	python3 tools/build_content.py --root . --show $(SHOW) $(CONTENTFLAGS)

run: all
	./masagin -show $(SHOW) $(MSGFLAGS)

# The measurements described in HACKING.md, into /tmp.
TESTOUT ?= /tmp/masagin-test
test: vktest vkbloom corehello
	rm -rf $(TESTOUT) && mkdir -p $(TESTOUT)/quads $(TESTOUT)/bloom $(TESTOUT)/flat
	./vktest  -headless -frames 2 -dump $(TESTOUT)/quads -dumpstart 1
	./vkbloom -headless -frames 2 -dump $(TESTOUT)/bloom -dumpstart 1
	./vkbloom -nobloom -headless -frames 2 -dump $(TESTOUT)/flat -dumpstart 1
	python3 test/measure.py $(TESTOUT)/bloom/frame_00001.png $(TESTOUT)/flat/frame_00001.png
	./corehello

clean:
	rm -rf $(OBJ) masagin vktest vkbloom corehello
distclean: clean
	rm -rf build deps

-include $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)
