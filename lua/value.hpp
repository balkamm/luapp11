#pragma once

#include "lua/state.hpp"
#include "lua/exception.hpp"
#include <memory>
#include <utility>

namespace lua
{

// Full userdata (managed by Lua)
template <typename T>
class Value
{
public:
	int push(State& s)
	{
		T* ud = (T*)s.newuserdata(sizeof(T));
		*ud = value_;
		return s.gettop();
	}

	inline operator T() {
		return value_;
	}

private:
	T value_;
};

// light userdata (managed by us)
template <typename T>
class Value<T*>
{
public:
	Value(T* value) : value_{value} {}

	inline operator T() {
		return value_;
	}
private:
	T* value_;
};

template<>
class Value<double>
{
public:
	Value(double&& value) : value_{value} {}

private:
	double value_;
};

template<>
class Value<std::string>
{
public:
	// Value(std::string value) : value_{value} {}
	Value(std::string&& value) : value_(value) {}
	Value(const char* value) : value_(value) {}

	int push(State* state) {
		state->pushstring(value_.c_str());
		return state->gettop();
	}
private:
	std::string value_;
};

// An untyped variable
class Variable {
protected:
	State* state_;
	std::function<void(State*)> pushParent_;
	std::function<void(State*)> pushKey_;

	void push(State* s) {
		pushParent_(s);
		auto idx = s->gettop();
		pushKey_(s);
		s->gettable(idx);
	}
public:

	template<typename T>
	Value<T> Get()
	{
		StackGuard g(state_);
		push(state_);
		Value<T> ret = Value<T>::Pop(state_);
		return ret;
	}

	template<typename T>
	Variable operator[](Value<T> index) {
		return Variable(state_, this, index);
	}

	template<typename T>
	void operator=(T& value)
	{
		StackGuard g(state_);
		pushParent_(state_);
		auto idx = state_->gettop();

		pushKey_(state_);
		auto keyIdx = state_->gettop();

		value.push(state_);
		auto valueIdx = state_->gettop();

		if(valueIdx > keyIdx + 1) {
			state_->insert(keyIdx + 1);
			state_->pop(valueIdx - keyIdx + 1);
		}

		state_->settable(idx);
	}

private:
	Variable(State* state, Variable* parent, Variable* key)
		: state_{state}
	{}

	Variable(State* state, int index)
		: state_{state}
	{}

	friend class State;
};

Variable State::Env() {
	return Variable(this, LUA_GLOBALSINDEX);
}

}