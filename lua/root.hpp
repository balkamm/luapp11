#pragma once

namespace lua
{

class root {
public:
	root()
		: L{luaL_newstate()}
	{}

	var operator[](val key) const {
		return var(L, LUA_GLOBALSINDEX, key);
	}
private:
	lua_State* L;
};

static root root;


}