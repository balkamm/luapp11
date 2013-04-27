#pragma once

#include "lua/internal/stack_guard.hpp"
#include "lua/exception.hpp"
#include <memory>
#include <utility>
#include <unordered_map>
#include <cstring>

namespace lua
{

void stackdump_g(lua_State* l)
{
    int i;
    int top = lua_gettop(l);
 
    printf("total in stack %d\n",top);
 
    for (i = 1; i <= top; i++)
    {  /* repeat for each level */
        int t = lua_type(l, i);
        switch (t) {
            case LUA_TSTRING:  /* strings */
                printf("string: '%s'\n", lua_tostring(l, i));
                break;
            case LUA_TBOOLEAN:  /* booleans */
                printf("boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:  /* numbers */
                printf("number: %g\n", lua_tonumber(l, i));
                break;
            default:  /* other values */
                printf("%s\n", lua_typename(l, t));
                break;
        }
        printf("  ");  /* put a separator */
    }
    printf("\n");  /* end the listing */
}


class val {
public:
	val() : type_{type::nil}, ptr{nullptr} {}
	val(lua_Number n) : type_{type::number}, num{n} {}
	val(bool b) : type_{type::boolean}, boolean{b} {}
	val(const std::string& s) : type_{type::string}, str{s.c_str()} {}
	val(const char* s) : type_{type::string}, str{s} {}
	val(lua_CFunction f) : type_{type::function}, func{f} {}
	val(lua_State* s) : type_{type::thread}, thread{s} {}
	val(void* lud) : type_{type::lightuserdata}, ptr{lud} {}
	val(std::initializer_list<std::pair<val, val>> t) 
		: type_{type::table}
		, table(new std::unordered_map<val, val, valueHasher>(t.begin(), t.end()))
	{}
	
	~val() {
		if(type_ == type::table) {
			table.reset();
		}		
	}

	val(const val& other)
		: type_{other.type_}
	{
		switch (type_) {
			case type::nil:
			case type::lightuserdata:
				ptr = other.ptr;
				break;
			case type::number: num = other.num; break;
			case type::boolean: boolean = other.boolean; break;
			case type::string: str = other.str; break;
			case type::table: table = other.table; break;
			case type::function: func = other.func; break;
			case type::thread: thread = other.thread; break;
		}
	}
	// template<typename T>
	// val(UserData<T>&& data) 
	// 	: type_{type::Userdata}
	// {
	// 	val_.userData = UD{ 
	// 		sizeof(UserData<T>),
	// 		new UserData<T>(std::forward(data))
	// 	}
	// }

	enum class type : int {
		nil = LUA_TNIL,
		number = LUA_TNUMBER,
		boolean = LUA_TBOOLEAN,
		string = LUA_TSTRING,
		table = LUA_TTABLE,
		function = LUA_TFUNCTION,
		// Userdata = LUA_TUSERDATA,
		thread = LUA_TTHREAD,
		lightuserdata = LUA_TLIGHTUSERDATA,
	};

	template<typename T, class Enable = void>
	struct get_number {
		static T get(const val& v) {
			throw "Invalid Type Error";
		}
	};

	template<typename T>
	struct get_number<T, typename std::enable_if<std::is_arithmetic<T>::value>::type> {
		static T get(const val& v) {
			return v.num;
		}
	};

	template<typename T, class Enable = void>
	struct get_boolean {
		static T get(const val& v) {
			throw "Invalid Type Error";
		}
	};

	template<typename T>
	struct get_boolean<T, typename std::enable_if<std::is_fundamental<T>::value>::type> {
		static T get(const val& v) {
			return v.boolean;
		}
	};

	template<typename T, class Enable = void>
	struct get_string {
		static T get(const val& v) {
			throw "Invalid Type Error";
		}
	};

	template<typename T>
	struct get_string<T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
		static std::string get(const val& v) {
			return std::string(v.str);
		}
	};

	template<typename T>
	struct get_string<T, typename std::enable_if<std::is_same<T, const char*>::value>::type> {
		static std::string get(const val& v) {
			return v.str;
		}
	};

	template<typename T>
	struct get_nil {
		static T get(const val& v) {
			return NULL;
		}
	};
	template<typename T>
	struct get_nil<T*> {
		static T* get(const val& v) {
			return nullptr;
		}
	};

	template<typename T, class Enable = void>
	struct get_table {
		static T get(const val& v) {
			throw "Invalid Type Error";
		}
	};

	template<typename T, class Enable = void>
	struct get_function {
		static T get(const val& v) {
			throw "Invalid Type Error";
		}
	};

	template<typename T>
	struct get_function<T, typename std::enable_if<std::is_function<T>::value>::type> {
		static T get(const val& v) {
			return dynamic_cast<T>(v.func);
		}
	};

	template<typename T>
	struct get_lightuserdata {
		static T get(const val& v) {
			return *(T*)v.ptr;
		}
	};
	template<typename T>
	struct get_lightuserdata<T*> {
		static T* get(const val& v) {
			return (T*)v.ptr;
		}
	};

	template <typename T>
	T get() {
		switch(type_) {
			case type::number: return get_number<T>::get(this);
			case type::boolean: return get_boolean<T>::get(this);
			case type::string: return get_string<T>::get(this);
			case type::nil: return get_nil<T>::get(this);
			case type::table: return get_table<T>::get(this);
			case type::function: return get_function<T>::get(this);
			case type::lightuserdata: return get_lightuserdata<T>::get(this);
			case type::thread:
			default:
				throw "Invalid Type Error";
		}
	}

	friend bool operator ==(const val& a, const val& b) {
		if(a.type_ != b.type_) {
			return false;
		}
		switch(a.type_) {
			case type::number:
				return a.num == b.num;
			case type::boolean:
				return a.boolean == b.boolean;
			case type::string:
				return strcmp(a.str, b.str);
			
			case type::nil:
			case type::table:
			case type::function:
			case type::thread:
			case type::lightuserdata:
				return a.ptr == b.ptr;
		}
		return false;
	}

	static const val nil;
private:
	// Puts on the top of the stack -0, +1, -
	virtual void push(lua_State* s) const {
		switch(type_) {
			case type::nil: lua_pushnil(s); break;
			case type::number: lua_pushnumber(s, num); break;
			case type::boolean: lua_pushboolean(s, boolean); break;
			case type::string: lua_pushstring(s, str); break;
			case type::table: {
				lua_newtable(s);
				for(auto p: *table) {
					p.first.push(s);
					p.second.push(s);
					lua_settable(s, -3);
				}
				break;
			}
			case type::function: lua_pushcfunction(s, func); break;
			// case type::Userdata: {
			// 	auto data = lua_newuserdata(s, userData.Size);
			// 	std::copy(userData.Data, userData.Data[userData.Size], data);
			// 	break;
			// }
			case type::thread: lua_pushthread(thread); break;
			case type::lightuserdata: lua_pushlightuserdata(s, ptr); break;
		}
		stackdump_g(s);
	}

	struct UD {
		size_t Size;
		void * Data;
	};

	class valueHasher {
	public:
		size_t operator()(const val& v) const {
			std::hash<lua_Number> hasher;
			return hasher(v.num);
		}
	};

	union {
		void* ptr;
		bool  boolean;
		lua_Number num;
		const char * str;
		lua_CFunction func;
		lua_State* thread;
		// UD userData;
	};
	std::shared_ptr<std::unordered_map<val, val, valueHasher>> table;

	type type_;
	friend class var;
};

const val val::nil = val();

}