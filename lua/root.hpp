#pragma once

namespace lua
{

root::root()
	: L{luaL_newstate()}
{
	luaL_openlibs(L);
	lua_atpanic(L, &panic);
}

var root::operator[](val key) const {
	return var(L, LUA_GLOBALSINDEX, key);
}

bool root::protected_calls() const {
	return protected_calls_;
}

void root::set_protected_calls(bool to_set) {
	protected_calls_ = to_set;
}

std::shared_ptr<error> root::call(int nargs, int nresults, std::string message) const {
	if(protected_calls_) {
		auto err = lua_pcall(L, nargs, nresults, 0);
		if(err != 0) {
			return std::shared_ptr<error>(new error((error::type)err, message, L));
		}
	} else {
		lua_call(L, nargs, nresults);
	}
	return std::shared_ptr<error>();
}

std::shared_ptr<error> root::call(int nargs, int nresults) const {
	return call(nargs, nresults, "Error calling lua function.");
}

void do_chunk(const std::string& str) {
	luaL_dostring(root.L, str.c_str());
}

}