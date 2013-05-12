#pragma once

#include <memory>

namespace lua {

class var;
class val;
class error;

class root {
public:
	root();
	var operator[](val key) const;

	bool protected_calls() const;
	void set_protected_calls(bool to_set);
private:
	std::shared_ptr<error> call(int nargs, int nresults, std::string message) const;
	std::shared_ptr<error> call(int nargs, int nresults) const;

	static int panic(lua_State* L) {
		throw lua::exception("lua panic", L);
	}

	lua_State* L;
	bool protected_calls_;

	friend void do_chunk(const std::string& str);
	friend class var;
	friend class val;
};

static root root;

}