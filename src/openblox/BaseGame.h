#ifndef OPENBLOX_BASEGAME_H_
#define OPENBLOX_BASEGAME_H_

#include "OpenBlox.h"

#ifndef OPENBLOX_JNI
#define notDefined
#endif

namespace ob_instance{
	class DataModel;
}

namespace OpenBlox{
	class Factory;
	class ThreadScheduler;

	class BaseGame{
		public:
			BaseGame();
			virtual ~BaseGame();

			void alert(std::string output, std::string title = "Alert");
			void print(std::string output);
			void warn(std::string output);
			void print_error(std::string output);
			void handle_lua_errors(lua_State* L);

			ob_instance::DataModel* getDataModel();
			ThreadScheduler* getThreadScheduler();

			static lua_State* getGlobalState();
			static Factory* getInstanceFactory();
			static Factory* InstanceFactory;
			static long appStarted();

			static lua_State* newLuaState();

			static BaseGame* getInstance();

			static double elapsedTime();

			static int lua_wait(lua_State* L);
			static int lua_delay(lua_State* L);
			static int lua_spawn(lua_State* L);

			static int lua_loadlibrary(lua_State* L);

			static int lua_tick(lua_State* L);
			static int lua_print(lua_State* L);
			static int lua_elapsedTime(lua_State* L);
			static int lua_warn(lua_State* L);

			static int lua_alert(lua_State* L);

			static int lua_newInstance(lua_State* L);

			static int lua_newVector3(lua_State* L);

			static int lua_newVector2(lua_State* L);

			static int lua_newColor3(lua_State* L);

			static int lua_newUDim(lua_State* L);

			static int lua_newUDim2(lua_State* L);
		private:
			ob_instance::DataModel* datamodel;

			static lua_State* GlobalLuaState;
	};
}
#endif
