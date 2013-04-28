#pragma once

#include <iostream>

namespace lua
{

class var {
public:
	// Gets the value of the var
	val get_value() const {
		push();
		return val(L);
	}

	// Gets the value of the var typed
	template <typename T>
	T get() const {
		return get_value().get<T>();	
	}

	template <typename T>
	bool is() const {
		stack_guard g(L);
		push();
		return typed_is<T>::is(L);
	}

	template <typename T>
	T as(T&& fallback) {
		stack_guard g(L);
		push();
		return typed_is<T>::is(L) ? val(L).get<T>() : fallback;
	}

	// Assigns the var with another var
	var& operator=(const var& var) {
		stack_guard g(L);
		parent_key_();
		if(L == var.L) {
			stack_guard g2(L, true);
			var.push();
		} else {
			var.get_value().push(L);
		}
		lua_settable(L, virtual_index_ ? virtual_index_ : -3);
		return *this;
	}
	
	var& operator=(const val& val) {
		stack_guard g(L);
		parent_key_();
		val.push(L);
		lua_settable(L, virtual_index_ ? virtual_index_ : -3);
		return *this;
	}

	var operator[](val idx) {
		return var(*this, idx);
	}

	var operator[](var idx) {
		return var(*this, idx.get_value());
	}
protected:
	void push() const {
		parent_key_();
		lua_gettable(L, virtual_index_ ? virtual_index_ : -2);
	}

	template<typename T, class Enable = void>
	struct typed_is {
		static inline bool is(lua_State* L) {
			return false;
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_isnumber(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
		static inline bool is(lua_State* L) {
			std::cout << "Is it a string?" << !lua_isnoneornil(L,-1) << lua_isstring(L,-1) << std::endl;
			return !lua_isnoneornil(L,-1) && lua_isstring(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_same<T, const char*>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_isstring(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_same<T, bool>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_isboolean(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_function<T>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_isfunction(L,-1);
		}
	};

	template<typename T>
	struct typed_is<T, typename std::enable_if<std::is_pointer<T>::value>::type> {
		static inline bool is(lua_State* L) {
			return !lua_isnoneornil(L,-1) && lua_islightuserdata(L,-1);
		}
	};


	var(var var, val key) 
		: L{var.L}
		, parent_key_{[var, key]() { var.push(); key.push(var.L); }}
		, virtual_index_{0}
	{}

	var(lua_State* L, int virtual_index, val key) 
		: L{L}
		, parent_key_{[L, key](){key.push(L);}}
		, virtual_index_{virtual_index}
	{}

	var(const var& other) = default;
	var(var&& other) = default;

	lua_State* L;
	std::function<void(void)> parent_key_;
	int virtual_index_;

	friend class root;
};

}