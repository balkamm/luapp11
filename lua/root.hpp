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

	bool protected_calls() const {
		return protected_calls_;
	}

	void set_protected_calls(bool to_set) {
		protected_calls_ = to_set;
	}
private:
	static int panic(lua_State* L) {
		throw lua::exception("lua panic", L);
	}

	std::shared_ptr<error> call(lua_State* L, int nargs, int nresults, std::string message) const {
		if(protected_calls_) {
			auto err = lua_pcall(L, nargs, nresults, 0);
			if(err != 0) {
				return std::make_shared<error>((lua::error::type)err, message, L);
			}
		} else {
			lua_call(L, nargs, nresults);
		}
		return std::shared_ptr<error>();
	}

	std::shared_ptr<error> call(lua_State* L, int nargs, int nresults) const {
		return call(L, nargs, nresults, "Error calling lua function.");
	}

	lua_State* L;
	bool protected_calls_;

	friend void do_chunk(const std::string& str);
};

static root root;

void do_chunk(const std::string& str) {
	luaL_dostring(lua::root.L, str.c_str());
}

}