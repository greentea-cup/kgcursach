# CC = cl
# CXX = cl
SYSINCLUDES = /external:anglebrackets /external:W0 /external:Iglm /external:Istb_image \
	/external:Iwin/glew-2.1.0/include /external:Iwin/SDL2-2.32.8/include /FIWindows.h /FIBaseTsd.h /Dssize_t=SSIZE_T
INCLUDES = /I$(INCDIR) $(SYSINCLUDES)
CXXFLAGS = /nologo /utf-8 /O2 /W3 /WX /DNDEBUG /std:c++17 $(INCLUDES) /EHsc /D_CRT_SECURE_NO_WARNINGS
LDLIBS_64 = win/glew-2.1.0/lib/Release/x64/glew32.lib win/SDL2-2.32.8/lib/x64/SDL2.lib opengl32.lib
LDLIBS_32 = win/glew-2.1.0/lib/Release/Win32/glew32.lib win/SDL2-2.32.8/lib/x86/SDL2.lib opengl32.lib
GLEW32_DLL_64 = win\glew-2.1.0\bin\Release\x64\glew32.dll
GLEW32_DLL_32 = win\glew-2.1.0\bin\Release\Win32\glew32.dll
SDL2_DLL_64 = win\SDL2-2.32.8\lib\x64\SDL2.dll
SDL2_DLL_32 = win\SDL2-2.32.8\lib\x86\SDL2.dll
LDFLAGS = /nologo
LDLIBS = $(LDLIBS_64)
GLEW32_DLL = $(GLEW32_DLL_64)
SDL2_DLL = $(SDL2_DLL_64)

INCDIR=include
SRCDIR=src
BUILDDIR=winbuild

SRCS = $(SRCDIR)/transform.cpp $(SRCDIR)/registry.cpp $(SRCDIR)/util.cpp $(SRCDIR)/mainloop.cpp \
	$(SRCDIR)/loader.cpp $(SRCDIR)/object.cpp $(SRCDIR)/main.cpp $(SRCDIR)/stb_image_impl.cpp \
	$(SRCDIR)/shader.cpp $(SRCDIR)/player.cpp
OBJS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.obj,$(SRCS))

all: regendeps $(BUILDDIR)/app

{$(SRCDIR)/}.cpp{$(BUILDDIR)/}.obj:
	$(CXX) $(CXXFLAGS) /c $< /Fo: $@

$(BUILDDIR)/app: $(OBJS)
	$(CXX) $(LDFLAGS) $** $(LDLIBS) /Fe: $@
	copy $(GLEW32_DLL) $(BUILDDIR)
	copy $(SDL2_DLL) $(BUILDDIR)

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

