#ifndef OPENBLOX_BASEGAME_H_
#define OPENBLOX_BASEGAME_H_

#include "OpenBlox.h"
namespace OpenBlox{
	class Factory;

	class BaseGame{
		public:
			BaseGame();
			virtual ~BaseGame();

			int print(lua_State *L);
			int warn(lua_State *L);
			void handle_errors(lua_State *L);

			static lua_State *getGlobalState();
			static Factory *getInstanceFactory();
			static Factory *InstanceFactory;

		private:
			lua_State* newLuaState();

			static lua_State *GlobalLuaState;
	};
}
#endif
