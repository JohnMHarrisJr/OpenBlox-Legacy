#include "OpenBlox.h"

#include "../ob_instance/DataModel.h"

#include <unistd.h>
#include "WindowUtils.h"

GLFWwindow* window;
OpenBlox::BaseGame* game;

void render(){
	float ratio;
	int width, height;

	glfwGetFramebufferSize(window, &width, &height);
	ratio = width / (float)height;

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	glMatrixMode(GL_MODELVIEW);

	ob_instance::DataModel* dm = game->getDataModel();
	if(dm != NULL){
		dm->render();
		dm->renderChildren();
	}
}

void* luaThread(void* arg){
	lua_State* L = OpenBlox::BaseGame::getGlobalState();

	ob_instance::DataModel* dm = game->getDataModel();
	int gm = dm->wrap_lua(L);
	lua_pushvalue(L, -gm);
	lua_setglobal(L, "game");

	lua_pushvalue(L, -gm);
	lua_setglobal(L, "Game");

	lua_pop(L, gm);

	char* script = "local tester = Instance.new('TestClass'); print(Instance.new('DataModel')); print(tester.ClassName, tester); print(game.ClassName, game); print(game:GetFullName()); print(getmetatable(game))";
	int s = luaL_loadbuffer(L, script, strlen(script), "@game.Workspace.Script");
	if(s == 0){
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
	}

	if(s != 0){
		game->handle_lua_errors(L);
	}
	lua_close(L);

	pthread_exit(NULL);
	return NULL;
}

void glfw_error_callback(int error, const char* description){
	std::cout << "[GRAPHICS]" << description << std::endl;
}

void window_size_callback(GLFWwindow* window, int width, int height){
	glfwMakeContextCurrent(window);
	glViewport(0, 0, width, height);
	glfwMakeContextCurrent(NULL);
	//render();
	//glfwSwapBuffers(window);
}

void window_pos_callback(GLFWwindow* window, int x, int y){
	//render();
	//glfwSwapBuffers(window);
}

void* renderThread(void* arg){
	while(!glfwWindowShouldClose(window)){
		//Fire RunService.Stepped, then RunService.RenderStepped
		glfwMakeContextCurrent(window);
		render();
		glfwSwapBuffers(window);
		glfwMakeContextCurrent(NULL);
		usleep(10);
	}
	pthread_exit(NULL);
	return NULL;
}

int main(){
	glfwSetErrorCallback(glfw_error_callback);
	if(!glfwInit()){
		std::cerr << "Failed to initialize GLFW3." << std::endl;
		return 1;
	}

	OpenBlox::BaseGame::InstanceFactory = new OpenBlox::Factory();

	game = new OpenBlox::BaseGame();

	static_init::execute();

	#ifdef __APPLE__
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
	#endif

	window = glfwCreateWindow(640, 480, "OpenBlox", NULL, NULL);
	if(!window){
		std::cerr << "Could not create window." << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK){
		std::cerr << "Failed to initialize GLEW." << std::endl;
		glfwTerminate();
		return 1;
	}

	pthread_t lua_thread;
	int val;
	val = pthread_create(&lua_thread, NULL, luaThread, NULL);
	if(val){
		std::cerr << "Failed to create logic thread." << std::endl;
		glfwTerminate();
		return 1;
	}

	{
		const GLubyte* vendor = glGetString(GL_VENDOR);
		const GLubyte* renderer = glGetString(GL_RENDERER);
		const GLubyte* version = glGetString(GL_VERSION);
		const GLubyte* shading_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

		std::cout << "Vendor: " << vendor << std::endl;
		std::cout << "Renderer: " << renderer << std::endl;
		std::cout << "OpenGL Version: " << version << std::endl;
		std::cout << "Shading Language Version: " << shading_version << std::endl;
	}

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetWindowPosCallback(window, window_pos_callback);

	glfwMakeContextCurrent(NULL);

	glfwMaximizeWindow(window);

	pthread_t render_thread;
	val = pthread_create(&render_thread, NULL, renderThread, NULL);
	if(val){
		std::cerr << "Failed to create render thread." << std::endl;
		glfwTerminate();
		return 1;
	}

	while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
	}

	void* status;
	pthread_join(lua_thread, &status);
	pthread_join(render_thread, &status);

	glfwDestroyWindow(window);
	OpenBlox::BaseGame::getInstanceFactory()->releaseTable();
	glfwTerminate();
	return 0;
}
