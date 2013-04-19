#pragma once

#include "lua/state.hpp"
#include "lua/exception.hpp"
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <memory>
#include <utility>
#include <unordered_map>

namespace lua
{

template <typename T>
class Ptr {
public:
	T* get() {
		return *p_;
	}

	const T* get() const {
		return *p_;
	}

	T& operator ->() {
		return **p_;
	}

	T& operator *() {
		return **p_;
	}

	bool expired() const {
		return *p_ == nullptr;
	}

private:
	std::shared_ptr<T*> p_;
	lua_State* s_;
	
	Ptr(void* luaData, lua_State* s)
		: p_(reinterpret_cast<T*>(luaData))
		, s_(s)
	{
		push();
		lua_newtable(s_);
		lua_pushcfunction(s_, [this](lua_State* s) { return this->expire(); });
		lua_setfield(s_, -2, "__gc");
		lua_setmetatable(s_, -2);
		lua_pop(s_, 1);
	}

	Ptr(lua_State* s) : Ptr(lua_topointer(s, -1), s) {}

	void push() {

	}
	
	bool expire() {
		p_.reset();
		return true;
	}
};

template<typename T>
class UserData
{
public:
	
};

class Table;

class Value {
public:
	Value() : type_{Type::Nil} { val_.ptr = nullptr; }
	Value(lua_Number n) : type_{Type::Number} { val_.n = n;  }
	Value(bool b) : type_{Type::Boolean} { val_.b = b; }
	Value(const std::string& s) : type_{Type::String} { val_.s = s.c_str(); }
	Value(const char* s) : type_{Type::String} { val_.s = s; }
	Value(Table t) : type_{Type::Table} { val_.t = t; }
	Value(lua_CFunction f) : type_{Type::Function} { val_.func = f; }
	Value(lua_State* s) : type_{Type::Thread} { val_.thread = s; }
	Value(void* lud) : type_{Type::Lightuserdata} { val_.ptr = lud; }
	
	// template<typename T>
	// Value(UserData<T>&& data) 
	// 	: type_{Type::Userdata}
	// {
	// 	val_.userData = UD{ 
	// 		sizeof(UserData<T>),
	// 		new UserData<T>(std::forward(data))
	// 	}
	// }

	enum class Type : int {
		Nil = LUA_TNIL,
		Number = LUA_TNUMBER,
		Boolean = LUA_TBOOLEAN,
		String = LUA_TSTRING,
		Table = LUA_TTABLE,
		Function = LUA_TFUNCTION,
		// Userdata = LUA_TUSERDATA,
		Thread = LUA_TTHREAD,
		Lightuserdata = LUA_TLIGHTUSERDATA,
	};

	template <typename T>
	T Get() {

	}

	 static const Value Nil;
private:
	// Puts on the top of the stack -0, +1, -
	virtual void push(State* s) const {
		switch(type_) {
			case Type::Nil: s->pushnil(); break;
			case Type::Number: s->pushnumber(val_.n); break;
			case Type::Boolean: s->pushboolean(val_.b); break;
			case Type::String: s->pushstring(val_.s); break;
			case Type::Table: {

				break;
			}
			case Type::Function: s->pushcfunction(val_.func); break;
			// case Type::Userdata: {
			// 	auto data = s->newuserdata(val_.userData.Size);
			// 	std::copy(val_.userData.Data, val_.userData.Data[val_.userData.Size], data);
			// 	break;
			// }
			case Type::Thread: lua_pushthread(val_.thread); break;
			case Type::Lightuserdata: s->pushlightuserdata(val_.ptr); break;
		}
	}

	struct UD {
		size_t Size;
		void * Data;
	};

	Type type_;
	union {
		void* ptr;
		bool  b;
		lua_Number n;
		const char * s;
		Table t;
		lua_CFunction func;
		lua_State* thread;
		// UD userData;
	} val_;

	class valueHasher {
		size_t operator()(const Value& v) const {
			std::hash<lua_Number> hasher;
			return hasher(v.val_.n);
		}
	};
	friend class Variable;
	friend class Table;
};

const Value Value::Nil = Value();


class Table {
public:
	Table() = default;
	
	Table(std::initializer_list<std::pair<Value, Value>> pairs) {
		for(auto& p: pairs) {
			(*map_)[p.first] = p.second;
		}
	}

	Table(Table& t) : map_(t.map_) {}

	Value& operator[](Value key) {
		return (*map_)[key];
	}

private:
	std::unordered_map<Value, Value, Value::valueHasher> map_;
};

class Variable {
public:
	// Gets the value of the variable
	Value GetValue() const {
		StackGuard g(s);
		push();
		return Value();
	}

	// Gets the value of the variable typed
	template <typename T>
	T Get() const {
		return GetValue().Get<T>();	
	}

	// Assigns the variable with another variable
	Variable& operator=(const Variable& var) {
		StackGuard g(s);
		parent_();
		key_.push(s);
		if(s == var.s) {
			StackGuard g2(s, true);
			var.push();
		} else {
			var.GetValue().push(s);
		}
		s->settable(-3);
		return *this;
	}
	
	Variable& operator=(const Value& val) {
		StackGuard g(s);
		parent_();
		key_.push(s);
		val.push(s);
		s->settable(-3);
		return *this;
	}

	Variable operator[](Value idx) {
		return Variable(*this, idx);
	}

	Variable operator[](Variable idx) {
		return Variable(*this, idx.GetValue());
	}
private:
	void push() const {
		if(parent_ != nullptr) {
			parent_();
			key_.push(s);
			s->gettable(-2);
		}
	}

	Variable(Variable var, Value key);
	Variable(Value key); // Environment Variables;
	Variable(const Variable& other) = default;
	Variable(Variable&& other);

	State* s;
	std::function<void(void)> parent_;
	Value key_;
};

}