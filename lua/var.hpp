#pragma once

namespace lua
{

class var {
public:
	// Gets the value of the var
	val get_value() const {
		push();
		return val(); // TODO: Implement
	}

	// Gets the value of the var typed
	template <typename T>
	T get() const {
		return get_value().get<T>();	
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
		stack_guard g(L);
		parent_key_();
		lua_gettable(L, virtual_index_ ? virtual_index_ : -2);
	}

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