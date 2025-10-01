// vim: set fdm=indent :
#include <GL/glew.h>
#include <SDL2/SDL.h>

void mainloop(SDL_Window *window);

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
	(void)hInstance; (void)hPrevInstance; (void)lpCmdLine; (void)nShowCmd;
#else
int main(int argc, char **argv) {
	(void)argc; (void)argv;
#endif

	SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to init SDL: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Window *window = SDL_CreateWindow("***", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!window) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s\n", SDL_GetError());
		SDL_Quit();
		return 3;
	}
	// init opengl
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GLContext context = SDL_GL_CreateContext(window);
	if (!context) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create OpenGL context: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return -1;
	}
	{
		GLenum glewErr = glewInit();
		if (glewErr != GLEW_OK) {
			SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Cannnot initialize glew\n");
			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(window);
			SDL_Quit();
			return -1;
		}
	}
	{
		GLubyte const *gl_vendor = glGetString(GL_VENDOR);
		GLubyte const *gl_renderer = glGetString(GL_RENDERER);
		GLubyte const *gl_version = glGetString(GL_VERSION);
		SDL_LogDebug(
			SDL_LOG_CATEGORY_APPLICATION,
			"GL vendor: %s\nGL renderer: %s\nGL version: %s\n",
			gl_vendor, gl_renderer, gl_version
		);
	}

	// Adaptive VSync
	// https://wiki.libsdl.org/SDL2/SDL_GL_SetSwapInterval
	(void)SDL_GL_SetSwapInterval(-1); // i dont care if you have vsync or not 
	
	// the core
	mainloop(window);

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}

