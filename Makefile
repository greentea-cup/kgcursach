CC = gcc
CXX = g++
# CFLAGS = -O2 -Wall -Wextra -Werror -DNDEBUG -std=c11
CFLAGS =
GPPFLAGS =
CXXFLAGS = -O2 -Wall -Wextra -Werror -DNDEBUG -std=c++17
LDFLAGS =
LDLIBS = -lm -lSDL2 -lGL -lGLEW
RM = rm -rf

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
OBJDIR=$(BUILDDIR)/obj

SRCS = $(wildcard $(SRCDIR)/*.cpp)
OBJS = $(addprefix $(OBJDIR)/,$(notdir $(patsubst %.cpp,%.o,$(SRCS))))

.PHONY: all clean distclean cleandeps

all: regendeps $(BUILDDIR)/app compile_commands.json

$(OBJS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp Makefile
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -I$(INCDIR) $< -o $@

$(OBJS): | $(OBJDIR)

$(BUILDDIR)/app: $(OBJS) | $(BUILDDIR)
	$(LINK.cc) $^ $(LDLIBS) -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

compile_commands.json: Makefile
	python3 gen_compile_commands.py

regendeps: glm

glm:
	git clone --depth 1 --single-branch --branch 1.0.1 -- https://github.com/g-truc/glm.git glm
	# Я знаю про sparse/partial clone и знаю, что он появился только недавно. А rm работает всегда
	$(RM) glm/.git glm/.github glm/test glm/doc glm/cmake glm/CMakeLists.txt glm/glm/CMakeLists.txt glm/util glm/readme.md glm/manual.md

# glew:
# 	git clone --depth 1 --single-branch --branch glew-2.2.0 -- https://github.com/nigels-com/glew.git glew
# 	$(RM) -r glew/.git

cleandeps:
	$(RM) glm

clean:
	$(RM) $(OBJDIR)

distclean: clean
	$(RM) $BUILDDIR)
