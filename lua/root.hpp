#pragma once

namespace lua
{

class root {
public:
	root()
		: L{luaL_newstate()}
	{
		luaL_openlibs(L);
		lua_atpanic(L, &panic);
	}

	var operator[](val key) const {
		return var(L, LUA_GLOBALSINDEX, key);
	}
private:
	static int panic(lua_State* L) {
		stackdump_g(L);
		throw "Lua fail!";
	}

	lua_State* L;
};

static root root;


}