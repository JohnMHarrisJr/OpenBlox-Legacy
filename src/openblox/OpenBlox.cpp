#include "OpenBlox.h"

#include "../ob_instance/DataModel.h"

#include <unistd.h>
#include <stdio.h>

#ifndef OPENBLOX_SERVER
#include "WindowUtils.h"

#include "OpenBloxRenderUtil.h"
#endif

#ifdef _WIN32
//#include <winsock2.h>
#endif

OpenBlox::BaseGame* game;

lua_State* L = NULL;

#ifndef OPENBLOX_SERVER
OpenBlox::Thread* renderThread;

double lastTime = glfwGetTime();
int nbFrames = 0;

void render(){
	OpenBlox::SoundManager::update();

	OpenBlox::renderLock = true;
	
	ob_instance::DataModel* dm = game->getDataModel();

	double currentTime = glfwGetTime();
	nbFrames++;
	if(currentTime - lastTime >= 1.0){
		//std::string newTitle = "OpenBlox - ";
		double tpf = 1000.0/double(nbFrames);//Milliseconds per frame
		double fps = 1000/tpf;

		OpenBlox::calculatedFPS = fps;

		nbFrames = 0;
		lastTime += 1.0;
	}

	//float ratio;
	int width, height;

	OpenBlox::getFramebufferSize(&width, &height);
	//ratio = width / (float)height;

	glViewport(0, 0, width, height);
	glClear(GL_COLOR_BUFFER_BIT);

	if(dm){
		dm->render();
	}

	OpenBlox::renderLock = false;
}

bool wasResized = false;
#endif

bool shouldClose = false;

void taskLoop(){
	while(!shouldClose){
		ob_instance::DataModel* dm = game->getDataModel();
		OpenBlox::ThreadScheduler::Tick();

		if(dm){
			if(dm->runService){
				//double idiv = 0.33333333;
				//ob_type::VarWrapper* wrapi = new ob_type::VarWrapper(idiv);
				if(dm->runService->Heartbeat){
					std::vector<ob_type::VarWrapper> args = std::vector<ob_type::VarWrapper>();
					//args.push_back(wrapi);

					dm->runService->Heartbeat->Fire(args);
				}
				if(dm->runService->Stepped){
					std::vector<ob_type::VarWrapper> args = std::vector<ob_type::VarWrapper>();
					args.push_back((OpenBlox::currentTimeMillis() - OpenBlox::BaseGame::elapsedTime()) / 1000.0);
					//args.push_back(wrapi);

					dm->runService->Stepped->Fire(args);
				}
				if(dm->runService->RenderStepped){
					std::vector<ob_type::VarWrapper> args = std::vector<ob_type::VarWrapper>();
					dm->runService->RenderStepped->Fire(args);
				}
			}
			#ifndef OPENBLOX_SERVER
			if(wasResized){
				wasResized = false;
				if(dm->starterGui){
					int w;
					int h;
					OpenBlox::getFramebufferSize(&w, &h);

					dm->starterGui->sizeChanged(w, h);
				}
			}
			#endif
		}
		usleep(10000);
	}
}

#ifndef OPENBLOX_SERVER
void size_callback(int width, int height){
	wasResized = true;
}
#endif

#ifndef OPENBLOX_ANDROID
#ifndef OPENBLOX_SERVER
void glfw_error_callback(int error, const char* description){
	LOGE("[GLFW] %s", description);
}

void glfw_window_size_callback(GLFWwindow* window, int width, int height){
	size_callback(width, height);
}

void glfw_window_click_callback(GLFWwindow* window, int btn, int action, int mods){
	double x;
	double y;
	glfwGetCursorPos(window, &x, &y);

	bool down = action == GLFW_PRESS;

	ob_enum::MouseButton mbtn;

	if(btn == GLFW_MOUSE_BUTTON_LEFT){
		mbtn = ob_enum::MouseButton::Left;
	}else if(btn == GLFW_MOUSE_BUTTON_RIGHT){
		mbtn = ob_enum::MouseButton::Right;
	}else if(btn == GLFW_MOUSE_BUTTON_MIDDLE){
		mbtn = ob_enum::MouseButton::Middle;
	}

	ob_instance::DataModel* dm = game->getDataModel();
	if(dm->starterGui){
		dm->starterGui->onClick(x, y, mbtn, down);
	}
}

void renderLoop(){
	GLFWwindow* window = OpenBlox::getWindow();
	glfwMakeContextCurrent(window);

	{
		const GLubyte* vendor = glGetString(GL_VENDOR);
		const GLubyte* renderer = glGetString(GL_RENDERER);
		const GLubyte* version = glGetString(GL_VERSION);
		const GLubyte* shading_version = glGetString(GL_SHADING_LANGUAGE_VERSION);

		LOGI("[GL] Vendor: %s", vendor);
		LOGI("[GL] Renderer: %s", renderer);
		LOGI("[GL] OpenGL Version: %s", version);
		LOGI("[GL] Shading Language Version: %s", shading_version);
	}

	while(!shouldClose){
		render();

		glfwSwapBuffers(window);
		usleep(500);
	}
}
#endif

#include "../easywsclient/easywsclient.hpp"

void init(){
	OpenBlox::BaseGame::InstanceFactory = new OpenBlox::Factory();

	game = new OpenBlox::BaseGame();

	static_init::execute();

	L = OpenBlox::BaseGame::newLuaState();
}

void cleanup(){
	#ifndef __unix__
		lua_close(L);
	#endif

	OpenBlox::SoundManager::cleanup();

	OpenBlox::BaseGame::getInstanceFactory()->releaseTable();
	delete game;
}

int main(){
	#ifdef _WIN32
		INT rc;
		WSADATA wsaData;

		rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if(rc){
			LOGW("WSAStartup failed.");
		}

		//timeBeginPeriod(1);
	#endif

	#ifndef OPENBLOX_SERVER
	glfwSetErrorCallback(glfw_error_callback);
	if(!glfwInit()){
		LOGE("[GLFW] Failed to initialize library.");
		return 1;
	}
	#endif

	init();

	#ifndef OPENBLOX_SERVER

	#ifdef __APPLE__
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}
	#endif
	glfwWindowHint(GLFW_SAMPLES, OPENBLOX_AA_SAMPLES);

	if(!OpenBlox::createGLContext()){
		return 1;
	}

	GLFWwindow* window = OpenBlox::getWindow();

	glfwMakeContextCurrent(window);
	{
		glewExperimental = GL_TRUE;
		if(glewInit() != GLEW_OK){
			LOGE("[GLFW] Failed to initialize GLEW.");
			glfwTerminate();
			return 1;
		}
		if(GLEW_VERSION_3_0){
			if(GLEW_ARB_texture_non_power_of_two){
				SOIL_non_NPOT_capability();
			}
		}else{
			const char* EXTS = (char const*)glGetString(GL_EXTENSIONS);
			if(EXTS){
				if(strstr(EXTS, "GL_ARB_texture_non_power_of_two")){
					SOIL_non_NPOT_capability();
				}
			}
		}
	}
	glfwMakeContextCurrent(NULL);

	glfwSetWindowSizeCallback(window, glfw_window_size_callback);

	glfwSetMouseButtonCallback(window, glfw_window_click_callback);

	renderThread = new OpenBlox::Thread(renderLoop);
	#endif
	OpenBlox::ThreadScheduler::taskThread = new OpenBlox::Thread(taskLoop);

	#ifndef OPENBLOX_SERVER
	renderThread->start();
	#endif
	OpenBlox::ThreadScheduler::taskThread->start();

	//TODO: Keep? Test a little more before deciding..
	//L = OpenBlox::BaseGame::newLuaState();

	OpenBlox::ThreadScheduler::RunOnTaskThread([](va_list args){
		lua_resume(L, 0);

		char* fileName = "init.lua";

		if(!OpenBlox::fileIsReadable(fileName)){
			LOGE("init.lua does not exist or is not readable.");
			#ifndef OPENBLOX_SERVER
			glfwSetWindowShouldClose(OpenBlox::getWindow(), true);
			#else
			shouldClose = true;
			#endif
			return;
		}

		char* initFile = OpenBlox::fileGetContents(fileName);

		int s = luaL_loadbuffer(L, initFile, strlen(initFile), "@game.Workspace.Script");
		if(s == 0){
			s = lua_pcall(L, 0, LUA_MULTRET, 0);
		}

		if(s != 0){
			game->handle_lua_errors(L);
		}
	}, 100);

	#ifndef OPENBLOX_SERVER
	while(!glfwWindowShouldClose(window)){
		glfwWaitEvents();
	}
	shouldClose = true;

	renderThread->join();
	#endif
	OpenBlox::ThreadScheduler::taskThread->join();

	#ifndef OPENBLOX_SERVER
	glfwDestroyWindow(window);
	#endif

	cleanup();

	#ifndef OPENBLOX_SERVER
	glfwTerminate();
	#endif
	#ifdef _WIN32
		WSACleanup();
	#endif
	return 0;
}
#endif
