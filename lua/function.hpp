#pragma once

extern "C" {
	#include "lua.h"
}

#include <string>

#include "LuaState.hpp"
#include "LuaValue.hpp"

inline bool Unpack() { return 0; }

template <typename T, typename... Args>
inline int Unpack(T&& arg, Args&&... args) {
	LuaValue<T>::Push(arg);
	return 1 + Unpack<Args...>(args...);
}	

template <typename T, typename... Args>
inline int Unpack(LuaValue<T>&& arg, Args&&... args) {
	arg.push();
	return 1 + Unpack<Args...>(args...);
}

template <typename RetVal, typename... Args>
class LuaFunction : std::function<RetVal, Args...>
{
private:
	std::string funcName_;

public:
	LuaFunction(std::string funcName) 
		: funcName_{funcName}
	{}

	inline RetVal operator()(Args&&... args) const
	{
		lua_getglobal(LuaState::State(), funcName_.c_str());
		int nargs = Unpack<Args...>(args...);
		lua_call(LuaState::State(), nargs, LuaValue<RetVal>::StackSpace());
		return LuaValue<RetVal>::Pop();
	}
};