#pragma once

namespace lua
{

class var {
public:
	// Gets the value of the var
	val get_value() const {
		stack_guard g(s);
		push();
		return val();
	}

	// Gets the value of the var typed
	template <typename T>
	T get() const {
		return get_value().get<T>();	
	}

	// Assigns the var with another var
	var& operator=(const var& var) {
		stack_guard g(s);
		parent_();
		key_.push(s);
		if(s == var.s) {
			stack_guard g2(s, true);
			var.push();
		} else {
			var.get_value().push(s);
		}
		lua_settable(s, -3);
		return *this;
	}
	
	var& operator=(const val& val) {
		stack_guard g(s);
		parent_();
		key_.push(s);
		val.push(s);
		lua_settable(s, -3);
		return *this;
	}

	var operator[](val idx) {
		return var(*this, idx);
	}

	var operator[](var idx) {
		return var(*this, idx.get_value());
	}
private:
	void push() const {
		if(parent_ != nullptr) {
			parent_();
			key_.push(s);
			lua_gettable(s, -2);
		}
	}

	var(var var, val key);
	var(val key); // Environment Variables;
	var(const var& other) = default;
	var(var&& other);

	lua_State* s;
	std::function<void(void)> parent_;
	val key_;
};

}