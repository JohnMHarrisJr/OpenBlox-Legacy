#include "ContentProvider.h"

namespace ob_instance{
	DEFINE_CLASS(ContentProvider, false, isDataModel);

	ContentProvider::ContentProvider() : Instance(){
		Name = ClassName;
	}

	ContentProvider::~ContentProvider(){}

	void ContentProvider::Preload(std::string url){
		OpenBlox::AssetLocator::getAsset(url);
	}

	Instance* ContentProvider::cloneImpl(){
		return NULL;
	}

	void ContentProvider::register_lua_methods(lua_State* L){
		luaL_Reg methods[]{
			{"Preload", [](lua_State* L)->int{
				Instance* inst = checkInstance(L, 1);
				if(ContentProvider* cp = dynamic_cast<ContentProvider*>(inst)){
					std::string url = std::string(luaL_checkstring(L, 2));

					cp->Preload(url);

					return 0;
				}
				return luaL_error(L, COLONERR, "Preload");
			}},
			{NULL, NULL}
		};
		luaL_register(L, NULL, methods);

		Instance::register_lua_methods(L);
	}
}