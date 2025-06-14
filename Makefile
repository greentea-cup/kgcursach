CC = gcc
CXX = g++
# CFLAGS = -O2 -Wall -Wextra -Werror -DNDEBUG -std=c11
CFLAGS =
GPPFLAGS =
CXXFLAGS = -O2 -Wall -Wextra -Werror -DNDEBUG -std=c++17
LDFLAGS =
LDLIBS = -lm -lSDL2 -lGL -lGLEW

CXXFLAGS += -isystem /usr/include/SDL2
CXXFLAGS += -isystem /usr/include/GL
# or -isystem /usr/include/glm
CXXFLAGS += -isystem ./glm
CXXFLAGS += -isystem ./stb_image
# CXXFLAGS += -isystem ./glew
# LDFLAGS += -L

# CXXFLAGS += -fsanitize=address -g

INCDIR=include
SRCDIR=src
BUILDDIR=build

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
DEPS = $(patsubst %.o,%.d,$(OBJS))

.PHONY: all clean distclean cleandeps

all: regendeps $(BUILDDIR)/app compile_commands.json

$(OBJS): $(BUILDDIR)/%.o: $(SRCDIR)/%.cpp Makefile
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -I$(INCDIR) -MMD $< -o $@

-include $(DEPS)

$(OBJS): | $(BUILDDIR)

$(BUILDDIR)/app: $(OBJS) | $(BUILDDIR)
	$(LINK.cc) $^ $(LDLIBS) -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

compile_commands.json: Makefile
	python3 gen_compile_commands.py

regendeps: glm

glm:
	git clone --depth 1 --single-branch --branch 1.0.1 -- https://github.com/g-truc/glm.git glm
	# Я знаю про sparse/partial clone и знаю, что он появился только недавно. А rm работает всегда
	$(RM) -r glm/.git glm/.github glm/test glm/doc glm/cmake glm/CMakeLists.txt glm/glm/CMakeLists.txt glm/util glm/readme.md glm/manual.md

# glew:
# 	git clone --depth 1 --single-branch --branch glew-2.2.0 -- https://github.com/nigels-com/glew.git glew
# 	$(RM) -r glew/.git

cleandeps:
	$(RM) -r glm

clean:
	$(RM) $(OBJS) $(DEPS)

distclean: clean
	$(RM) -r $(BUILDDIR)

