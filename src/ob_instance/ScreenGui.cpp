#include "ScreenGui.h"

namespace ob_instance{
	struct ScreenGuiClassMaker: public OpenBlox::ClassMaker{
		ob_instance::Instance* getInstance() const{
			return new ScreenGui;
		}

		bool isA(const ob_instance::Instance* obj){
			return (dynamic_cast<const ScreenGui*>(obj)) != 0;
		}

		bool isInstantiatable(){
			return true;
		}

		bool isService(bool isDataModel){
			return false;
		}
	};

	STATIC_INIT(ScreenGui){
		OpenBlox::BaseGame::getInstanceFactory()->addClass(ClassName, new ScreenGuiClassMaker());

		registerLuaClass(LuaClassName, register_lua_metamethods, register_lua_methods, register_lua_property_getters, register_lua_property_setters, register_lua_events);
	}
	char* ScreenGui::ClassName = "ScreenGui";
	char* ScreenGui::LuaClassName = "luaL_Instance_ScreenGui";

	ScreenGui::ScreenGui() : LayerCollector(){}

	ScreenGui::~ScreenGui(){}

	void ScreenGui::render(){
		glLoadIdentity();
		glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);

		glBegin(GL_TRIANGLES);
		{
			glColor3f(1.f, 0.f, 0.f);
			glVertex3f(-0.6f, -0.4f, 0.f);
			glColor3f(0.f, 1.f, 0.f);
			glVertex3f(0.6f, -0.4f, 0.f);
			glColor3f(0.f, 0.f, 1.f);
			glVertex3f(0.f, 0.6f, 0.f);
		}
		glEnd();
	}

	int ScreenGui::wrap_lua(lua_State* L){
		ScreenGui** udata = (ScreenGui**)lua_newuserdata(L, sizeof(*this));
		*udata = this;

		luaL_getmetatable(L, LuaClassName);
		lua_setmetatable(L, -2);

		return 1;
	}

	Instance* ScreenGui::cloneImpl(){
		ScreenGui* newGuy = new ScreenGui;
		newGuy->Name = Name;
		return newGuy;
	}

	char* ScreenGui::getClassName(){
		return ClassName;
	}
}