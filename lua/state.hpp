#pragma once

#include "lua.hpp"
#include <utility>

namespace lua
{

class Variable;
struct StackGuard;

class State
{
private:
	lua_State* state_;
	friend class Variable;
	friend class StackGuard;
	friend class Value;

public:
	inline State() {
		state_ = luaL_newstate();
	}

	inline ~State() {
		lua_close(state_);
	}

	inline Variable Env();


	// Here we list all functions and types from the C API in alphabetical order. Each function has an indicator like this: [-o, +p, x]

	// The first field, o, is how many elements the function pops from the stack. The second field, p, is how many elements the function pushes onto the stack. (Any function always pushes its results after popping its arguments.) A field in the form x|y means the function can push (or pop) x or y elements, depending on the situation; an interrogation mark '?' means that we cannot know how many elements the function pops/pushes by looking only at its arguments (e.g., they may depend on what is on the stack). The third field, x, tells whether the function may throw errors: '-' means the function never throws any error; 'm' means the function may throw an error only due to not enough memory; 'e' means the function may throw other kinds of errors; 'v' means the function may throw an error on purpose.

	// lua_Alloc

	// typedef void * (*lua_Alloc) (void *ud,							   void *ptr,							   size_t osize,							   size_t nsize);
	// The type of the memory-allocation function used by Lua states. The allocator function must provide a functionality similar to realloc, but not exactly the same. Its arguments are ud, an opaque pointer passed to lua_newstate; ptr, a pointer to the block being allocated/reallocated/freed; osize, the original size of the block; nsize, the new size of the block. ptr is NULL if and only if osize is zero. When nsize is zero, the allocator must return NULL; if osize is not zero, it should free the block pointed to by ptr. When nsize is not zero, the allocator returns NULL if and only if it cannot fill the request. When nsize is not zero and osize is zero, the allocator should behave like malloc. When nsize and osize are not zero, the allocator behaves like realloc. Lua assumes that the allocator never fails when osize >= nsize.

	// Here is a simple implementation for the allocator function. It is used in the auxiliary library by luaL_newstate.

	//	  static void *l_alloc (void *ud, void *ptr, size_t osize,												  size_t nsize) {
	//		(void)ud;  (void)osize;  /* not used */
	//		if (nsize == 0) {
	//		  free(ptr);
	//		  return NULL;
	//		}
	//		else
	//		  return realloc(ptr, nsize);
	//	  }
	// This code assumes that free(NULL) has no effect and that realloc(NULL, size) is equivalent to malloc(size). ANSI C ensures both behaviors.

	// lua_atpanic
	// [-0, +0, -]
	// Sets a new panic function and returns the old one.
	// If an error happens outside any protected environment, Lua calls a panic function and then calls exit(EXIT_FAILURE), thus exiting the host application. Your panic function can avoid this exit by never returning (e.g., doing a long jump).
	// The panic function can access the error message at the top of the stack.
	inline lua_CFunction atpanic (lua_CFunction panicf) {
		return lua_atpanic(state_, panicf);
	}

	// lua_call
	// [-(nargs + 1), +nresults, e]
	// Calls a function.

	// To call a function you must use the following protocol: first, the function to be called is pushed onto the stack; then, the arguments to the function are pushed in direct order; that is, the first argument is pushed first. Finally you call lua_call; nargs is the number of arguments that you pushed onto the stack. All arguments and the function value are popped from the stack when the function is called. The function results are pushed onto the stack when the function returns. The number of results is adjusted to nresults, unless nresults is LUA_MULTRET. In this case, all results from the function are pushed. Lua takes care that the returned values fit into the stack space. The function results are pushed onto the stack in direct order (the first result is pushed first), so that after the call the last result is on the top of the stack.

	// Any error inside the called function is propagated upwards (with a longjmp).

	// The following example shows how the host program can do the equivalent to this Lua code:

	//	  a = f("how", t.x, 14)
	// Here it is in C:

	//	  lua_getfield(L, LUA_GLOBALSINDEX, "f"); /* function to be called */
	//	  lua_pushstring(L, "how");						/* 1st argument */
	//	  lua_getfield(L, LUA_GLOBALSINDEX, "t");   /* table to be indexed */
	//	  lua_getfield(L, -1, "x");		/* push result of t.x (2nd arg) */
	//	  lua_remove(L, -2);				  /* remove 't' from the stack */
	//	  lua_pushinteger(L, 14);						  /* 3rd argument */
	//	  lua_call(L, 3, 1);	 /* call 'f' with 3 arguments and 1 result */
	//	  lua_setfield(L, LUA_GLOBALSINDEX, "a");		/* set global 'a' */
	// Note that the code above is "balanced": at its end, the stack is back to its original configuration. This is considered good programming practice.
	inline void call (int nargs, int nresults) {
		lua_call(state_, nargs, nresults);
	}

	// lua_CFunction

	// typedef int (*lua_CFunction) (
	// Type for C functions.

	// In order to communicate properly with Lua, a C function must use the following protocol, which defines the way parameters and results are passed: a C function receives its arguments from Lua in its stack in direct order (the first argument is pushed first). So, when the function starts, lua_gettop(L) returns the number of arguments received by the function. The first argument (if any) is at index 1 and its last argument is at index lua_gettop(L). To return values to Lua, a C function just pushes them onto the stack, in direct order (the first result is pushed first), and returns the number of results. Any other value in the stack below the results will be properly discarded by Lua. Like a Lua function, a C function called by Lua can also return many results.

	// As an example, the following function receives a variable number of numerical arguments and returns their average and sum:

	//	  static int foo ({
	//		int n = lua_gettop(L);	/* number of arguments */
	//		lua_Number sum = 0;
	//		int i;
	//		for (i = 1; i <= n; i++) {
	//		  if (!lua_isnumber(L, i)) {
	//			lua_pushstring(L, "incorrect argument");
	//			lua_error(L);
	//		  }
	//		  sum += lua_tonumber(L, i);
	//		}
	//		lua_pushnumber(L, sum/n);		/* first result */
	//		lua_pushnumber(L, sum);		 /* second result */
	//		return 2;				   /* number of results */
	//	  }

	// lua_checkstack
	// [-0, +0, m]
	// Ensures that there are at least extra free stack slots in the stack. It returns false if it cannot grow the stack to that size. This function never shrinks the stack; if the stack is already larger than the new size, it is left unchanged.
	inline int checkstack (int extra) {
		return lua_checkstack(state_, extra);
	}

	// lua_close
	// [-0, +0, -]
	// Destroys all objects in the given Lua state (calling the corresponding garbage-collection metamethods, if any) and frees all dynamic memory used by this state. On several platforms, you may not need to call this function, because all resources are naturally released when the host program ends. On the other hand, long-running programs, such as a daemon or a web server, might need to release states as soon as they are not needed, to avoid growing too large.
	inline void close () {
		lua_close(state_);
	}

	// lua_concat
	// [-n, +1, e]
	// Concatenates the n values at the top of the stack, pops them, and leaves the result at the top. If n is 1, the result is the single value on the stack (that is, the function does nothing); if n is 0, the result is the empty string. Concatenation is performed following the usual semantics of Lua (see §2.5.4).
	inline void concat (int n) {
		lua_concat(state_, n);
	}

	// lua_cpcall
	// [-0, +(0|1), -]
	// Calls the C function func in protected mode. func starts with only one element in its stack, a light userdata containing ud. In case of errors, lua_cpcall returns the same error codes as lua_pcall, plus the error object on the top of the stack; otherwise, it returns zero, and does not change the stack. All values returned by func are discarded.
	inline int cpcall (lua_CFunction func, void *ud) {
		return lua_cpcall(state_, func, ud);
	}

	// lua_createtable
	// [-0, +1, m]
	// Creates a new empty table and pushes it onto the stack. The new table has space pre-allocated for narr array elements and nrec non-array elements. This pre-allocation is useful when you know exactly how many elements the table will have. Otherwise you can use the function lua_newtable.
	inline void createtable (int narr, int nrec) {
		lua_createtable(state_, narr, nrec);
	}

	// lua_dump
	// [-0, +0, m]
	// Dumps a function as a binary chunk. Receives a Lua function on the top of the stack and produces a binary chunk that, if loaded again, results in a function equivalent to the one dumped. As it produces parts of the chunk, lua_dump calls function writer (see lua_Writer) with the given data to write them.
	// The value returned is the error code returned by the last call to the writer; 0 means no errors.
	// This function does not pop the Lua function from the stack.
	inline int dump (lua_Writer writer, void *data) {
		return lua_dump(state_, writer, data);
	}

	// lua_equal
	// [-0, +0, e]
	// Returns 1 if the two values in acceptable indices index1 and index2 are equal, following the semantics of the Lua == operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid.
	inline int equal (int index1, int index2) {
		return lua_equal(state_, index1, index2);
	}

	// lua_error
	// [-1, +0, v]
	// Generates a Lua error. The error message (which can actually be a Lua value of any type) must be on the stack top. This function does a long jump, and therefore never returns. (see luaL_error).
	inline int error () {
		return lua_error(state_);
	}

	// lua_gc
	// [-0, +0, e]
	// Controls the garbage collector.
	// This function performs several tasks, according to the value of the parameter what:
	// LUA_GCSTOP: stops the garbage collector.
	// LUA_GCRESTART: restarts the garbage collector.
	// LUA_GCCOLLECT: performs a full garbage-collection cycle.
	// LUA_GCCOUNT: returns the current amount of memory (in Kbytes) in use by Lua.
	// LUA_GCCOUNTB: returns the remainder of dividing the current amount of bytes of memory in use by Lua by 1024.
	// LUA_GCSTEP: performs an incremental step of garbage collection. The step "size" is controlled by data (larger values mean more steps) in a non-specified way. If you want to control the step size you must experimentally tune the value of data. The function returns 1 if the step finished a garbage-collection cycle.
	// LUA_GCSETPAUSE: sets data as the new value for the pause of the collector (see §2.10). The function returns the previous value of the pause.
	// LUA_GCSETSTEPMUL: sets data as the new value for the step multiplier of the collector (see §2.10). The function returns the previous value of the step multiplier.
	inline int gc (int what, int data) {
		return lua_gc(state_, what, data);
	}

	// lua_getallocf
	// [-0, +0, -]
	// Returns the memory-allocation function of a given state. If ud is not NULL, Lua stores in *ud the opaque pointer passed to lua_newstate.
	inline lua_Alloc getallocf (void **ud) {
		return lua_getallocf(state_, ud);
	}

	// lua_getfenv
	// [-0, +1, -]
	// Pushes onto the stack the environment table of the value at the given index.
	inline void getfenv (int index) {
		lua_getfenv(state_, index);
	}

	// lua_getfield
	// [-0, +1, e]
	// Pushes onto the stack the value t[k], where t is the value at the given valid index. As in Lua, this function may trigger a metamethod for the "index" event (see §2.8).
	inline void getfield (int index, const char* k) {
		lua_getfield(state_, index, k);
	}

	// lua_getglobal
	// [-0, +1, e]
	// Pushes onto the stack the value of the global name. It is defined as a macro:
		 // #define lua_getglobal(L,s)  lua_getfield(L, LUA_GLOBALSINDEX, s)
	inline void getglobal (const char *name) {
		lua_getglobal(state_, name);
	}

	// lua_getmtatable
	// [-0, +(0|1), -]
	// Pushes onto the stack the metatable of the value at the given acceptable index. If the index is not valid, or if the value does not have a metatable, the function returns 0 and pushes nothing on the stack.
	inline int getmetatable (int index) {
		return lua_getmetatable(state_, index);
	}

	// lua_gettable
	// [-1, +1, e]
	// Pushes onto the stack the value t[k], where t is the value at the given valid index and k is the value at the top of the stack.
	// This function pops the key from the stack (putting the resulting value in its place). As in Lua, this function may trigger a metamethod for the "index" event (see §2.8).
	inline void gettable (int index) {
		lua_gettable(state_, index);
	}

	// lua_gettop
	// [-0, +0, -]
	// Returns the index of the top element in the stack. Because indices start at 1, this result is equal to the number of elements in the stack (and so 0 means an empty stack).
	inline int gettop () {
		return lua_gettop(state_);
	}

	// lua_insert
	// [-1, +1, -]
	// Moves the top element into the given valid index, shifting up the elements above this index to open space. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position.
	inline void insert (int index) {
		lua_insert(state_, index);
	}

	// lua_Integer

	// typedef ptrdiff_t lua_Integer;
	// The type used by the Lua API to represent integral values.

	// By default it is a ptrdiff_t, which is usually the largest signed integral type the machine handles "comfortably".

	// lua_isboolean
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index has type boolean, and 0 otherwise.
	inline int isboolean (int index) {
		return lua_isboolean(state_, index);
	}

	// lua_iscfunction
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a C function, and 0 otherwise.
	inline int iscfunction (int index) {
		return lua_iscfunction(state_, index);
	}

	// lua_isfunction
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a function (either C or Lua), and 0 otherwise.
	inline int isfunction (int index) {
		return lua_isfunction(state_, index);
	}

	// lua_islightuserdata
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a light userdata, and 0 otherwise.
	inline int islightuserdata (int index) {
		return lua_islightuserdata(state_, index);
	}

	// lua_isnil
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is nil, and 0 otherwise.
	inline int isnil (int index) {
		return lua_isnil(state_, index);
	}

	// lua_isnone
	// [-0, +0, -]
	// Returns 1 if the given acceptable index is not valid (that is, it refers to an element outside the current stack), and 0 otherwise.
	inline int isnone (int index) {
		return lua_isnone(state_, index);
	}

	// lua_isnoneornil
	// [-0, +0, -]
	// Returns 1 if the given acceptable index is not valid (that is, it refers to an element outside the current stack) or if the value at this index is nil, and 0 otherwise.
	inline int isnoneornil (int index) {
		return lua_isnoneornil(state_, index);
	}

	// lua_isnumber
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a number or a string convertible to a number, and 0 otherwise.
	inline int isnumber (int index) {
		return lua_isnumber(state_, index);
	}

	// lua_isstring
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a string or a number (which is always convertible to a string), and 0 otherwise.
	inline int isstring (int index) {
		return lua_isstring(state_, index);
	}

	// lua_istable
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a table, and 0 otherwise.
	inline int istable (int index) {
		return lua_istable(state_, index);
	}

	// lua_isthread
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a thread, and 0 otherwise.
	inline int isthread (int index) {
		return lua_isthread(state_, index);
	}

	// lua_isuserdata
	// [-0, +0, -]
	// Returns 1 if the value at the given acceptable index is a userdata (either full or light), and 0 otherwise.
	inline int isuserdata (int index) {
		return lua_isuserdata(state_, index);
	}

	// lua_lessthan
	// [-0, +0, e]
	// Returns 1 if the value at acceptable index index1 is smaller than the value at acceptable index index2, following the semantics of the Lua < operator (that is, may call metamethods). Otherwise returns 0. Also returns 0 if any of the indices is non valid.
	inline int lessthan (int index1, int index2) {
		return lua_lessthan(state_, index1, index2);
	}

	// lua_load
	// [-0, +1, -]
	// Loads a Lua chunk. If there are no errors, lua_load pushes the compiled chunk as a Lua function on top of the stack. Otherwise, it pushes an error message. The return values of lua_load are:
	// 0: no errors;
	// LUA_ERRSYNTAX: syntax error during pre-compilation;
	// LUA_ERRMEM: memory allocation error.
	// This function only loads a chunk; it does not run it.
	// lua_load automatically detects whether the chunk is text or binary, and loads it accordingly (see program luac).
	// The lua_load function uses a user-supplied reader function to read the chunk (see lua_Reader). The data argument is an opaque value passed to the reader function.
	// The chunkname argument gives a name to the chunk, which is used for error messages and in debug information (see §3.8).
	inline int load (lua_Reader reader, void *data, const char *chunkname) {
		return lua_load(state_, reader, data, chunkname);
	}

	// luaL_newstate

	// [-0, +0, -]
	// lua_State *luaL_newstate (void);
	// Creates a new Lua state. It calls lua_newstate with an allocator based on the standard C realloc function and then sets a panic function (see lua_atpanic) that prints an error message to the standard error output in case of fatal errors.

	// Returns the new state, or NULL if there is a memory allocation error.

	// lua_newtable
	// [-0, +1, m]
	// Creates a new empty table and pushes it onto the stack. It is equivalent to lua_createtable(L, 0, 0).
	inline void newtable () {
		lua_newtable(state_);
	}

	// lua_newthread
	// [-0, +1, m]
	// Creates a new thread, pushes it on the stack, and returns a pointer to a lua_State that represents this new thread. The new state returned by this function shares with the original state all global objects (such as tables), but has an independent execution stack.
	// There is no explicit function to close or to destroy a thread. Threads are subject to garbage collection, like any Lua object.
	inline lua_State* newthread () {
		return lua_newthread(state_);
	}

	// lua_newuserdata
	// [-0, +1, m]
	// This function allocates a new block of memory with the given size, pushes onto the stack a new full userdata with the block address, and returns this address.
	// Userdata represent C values in Lua. A full userdata represents a block of memory. It is an object (like a table): you must create it, it can have its own metatable, and you can detect when it is being collected. A full userdata is only equal to itself (under raw equality).
	// When Lua collects a full userdata with a gc metamethod, Lua calls the metamethod and marks the userdata as finalized. When this userdata is collected again then Lua frees its corresponding memory.
	inline void * newuserdata (size_t size) {
		return lua_newuserdata(state_, size);
	}

	// lua_next
	// [-1, +(2|0), e]
	// Pops a key from the stack, and pushes a key-value pair from the table at the given index (the "next" pair after the given key). If there are no more elements in the table, then lua_next returns 0 (and pushes nothing).
	// A typical traversal looks like this:

	//	  /* table is in the stack at index 't' */
	//	  lua_pushnil(L);  /* first key */
	//	  while (lua_next(L, t) != 0) {
	//		 uses 'key' (at index -2) and 'value' (at index -1)
	//		printf("%s - %s\n", lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
	//		/* removes 'value'; keeps 'key' for next iteration */
	//		lua_pop(L, 1);
	//	  }
	// While traversing a table, do not call lua_tolstring directly on a key, unless you know that the key is actually a string. Recall that lua_tolstring changes the value at the given index; this confuses the next call to lua_next.
	inline int next (int index) {
		return lua_next(state_, index);
	}

	// lua_Number

	// typedef double lua_Number;
	// The type of numbers in Lua. By default, it is double, but that can be changed in luaconf.h.

	// Through the configuration file you can change Lua to operate with another type for numbers (e.g., float or long).

	// lua_objlen
	// [-0, +0, -]
	// Returns the "length" of the value at the given acceptable index: for strings, this is the string length; for tables, this is the result of the length operator ('#'); for userdata, this is the size of the block of memory allocated for the userdata; for other values, it is 0.
	inline size_t objlen (int index) {
		return lua_objlen(state_, index);
	}

	// lua_pcall
	// [-(nargs + 1), +(nresults|1), -]
	// Calls a function in protected mode.
	// Both nargs and nresults have the same meaning as in lua_call. If there are no errors during the call, lua_pcall behaves exactly like lua_call. However, if there is any error, lua_pcall catches it, pushes a single value on the stack (the error message), and returns an error code. Like lua_call, lua_pcall always removes the function and its arguments from the stack.
	// If errfunc is 0, then the error message returned on the stack is exactly the original error message. Otherwise, errfunc is the stack index of an error handler function. (In the current implementation, this index cannot be a pseudo-index.) In case of runtime errors, this function will be called with the error message and its return value will be the message returned on the stack by lua_pcall.
	// Typically, the error handler function is used to add more debug information to the error message, such as a stack traceback. Such information cannot be gathered after the return of lua_pcall, since by then the stack has unwound.
	// The lua_pcall function returns 0 in case of success or one of the following error codes (defined in lua.h):
	// LUA_ERRRUN: a runtime error.
	// LUA_ERRMEM: memory allocation error. For such errors, Lua does not call the error handler function.
	// LUA_ERRERR: error while running the error handler function.
	inline int pcall (int nargs, int nresults, int errfunc) {
		return lua_pcall(state_, nargs, nresults, errfunc);
	}

	// lua_pop
	// [-n, +0, -]
	// Pops n elements from the stack.
	inline void pop (int n) {
		lua_pop(state_, n);
	}

	// lua_pushboolean
	// [-0, +1, -]
	// Pushes a boolean value with value b onto the stack.
	inline void pushboolean (int b) {
		lua_pushboolean(state_, b);
	}

	// lua_pushcclosure
	// [-n, +1, m]
	// Pushes a new C closure onto the stack.
	// When a C function is created, it is possible to associate some values with it, thus creating a C closure (see §3.4); these values are then accessible to the function whenever it is called. To associate values with a C function, first these values should be pushed onto the stack (when there are multiple values, the first value is pushed first). Then lua_pushcclosure is called to create and push the C function onto the stack, with the argument n telling how many values should be associated with the function. lua_pushcclosure also pops these values from the stack.
	// The maximum value for n is 255.
	inline void pushcclosure (lua_CFunction fn, int n) {
		lua_pushcclosure(state_, fn, n);
	}

	// lua_pushcfunction
	// [-0, +1, m]
	// Pushes a C function onto the stack. This function receives a pointer to a C function and pushes onto the stack a Lua value of type function that, when called, invokes the corresponding C function.
	// Any function to be registered in Lua must follow the correct protocol to receive its parameters and return its results (see lua_CFunction).
	// lua_pushcfunction is defined as a macro:
	//	  #define lua_pushcfunction(L,f)  lua_pushcclosure(L,f,0)
	inline void pushcfunction (lua_CFunction f) {
		lua_pushcfunction(state_, f);
	}

	// lua_pushfstring
	// [-0, +1, m]
	// Pushes onto the stack a formatted string and returns a pointer to this string. It is similar to the C function sprintf, but has some important differences:
	// You do not have to allocate space for the result: the result is a Lua string and Lua takes care of memory allocation (and deallocation, through garbage collection).
	// The conversion specifiers are quite restricted. There are no flags, widths, or precisions. The conversion specifiers can only be '%%' (inserts a '%' in the string), '%s' (inserts a zero-terminated string, with no size restrictions), '%f' (inserts a lua_Number), '%p' (inserts a pointer as a hexadecimal numeral), '%d' (inserts an int), and '%c' (inserts an int as a character).
	template<typename... Parameters>
	inline const char * pushfstring (const char *fmt, Parameters... params) {
		return lua_pushfstring(state_, fmt, std::forward<Parameters>(params)...);
	}

	// lua_pushinteger
	// [-0, +1, -]
	// Pushes a number with value n onto the stack.
	inline void pushinteger (lua_Integer n) {
		lua_pushinteger(state_, n);
	}

	// lua_pushlightuserdata
	// [-0, +1, -]
	// Pushes a light userdata onto the stack.
	// Userdata represent C values in Lua. A light userdata represents a pointer. It is a value (like a number): you do not create it, it has no individual metatable, and it is not collected (as it was never created). A light userdata is equal to "any" light userdata with the same C address.
	inline void pushlightuserdata (void *p) {
		lua_pushlightuserdata(state_, p);
	}

	// lua_pushliteral

	// [-0, +1, m]
	// void lua_pushliteral (lua_State *L, const char *s);
	// This macro is equivalent to lua_pushlstring, but can be used only when s is a literal string. In these cases, it automatically provides the string length.

	// lua_pushlstring
	// [-0, +1, m]
	// Pushes the string pointed to by s with size len onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string can contain embedded zeros.
	inline void pushlstring (const char *s, size_t len) {
		lua_pushlstring(state_, s, len);
	}

	// lua_pushnil
	// [-0, +1, -]
	// Pushes a nil value onto the stack.
	inline void pushnil () {
		lua_pushnil(state_);
	}

	// lua_pushnumber
	// [-0, +1, -]
	// Pushes a number with value n onto the stack.
	inline void pushnumber (lua_Number n) {
		lua_pushnumber(state_, n);
	}

	// lua_pushstring
	// [-0, +1, m]
	// Pushes the zero-terminated string pointed to by s onto the stack. Lua makes (or reuses) an internal copy of the given string, so the memory at s can be freed or reused immediately after the function returns. The string cannot contain embedded zeros; it is assumed to end at the first zero.
	inline void pushstring (const char *s) {
		lua_pushstring(state_, s);
	}

	// lua_pushthread
	// [-0, +1, -]
	// Pushes the thread represented by L onto the stack. Returns 1 if this thread is the main thread of its state.
	inline int pushthread () {
		return lua_pushthread(state_);
	}

	// lua_pushvalue
	// [-0, +1, -]
	// Pushes a copy of the element at the given valid index onto the stack.
	inline void pushvalue (int index) {
		lua_pushvalue(state_, index);
	}

	// lua_pushvfstring
	// [-0, +1, m]
	// Equivalent to lua_pushfstring, except that it receives a va_list instead of a variable number of arguments.
	// inline const char * pushvfstring (const char *fmt, va_list argp) {
	// 	lua_pushvfstring(state_, fmt, va_list argp);
	// }

	// lua_rawequal
	// [-0, +0, -]
	// Returns 1 if the two values in acceptable indices index1 and index2 are primitively equal (that is, without calling metamethods). Otherwise returns 0. Also returns 0 if any of the indices are non valid.
	inline int rawequal (int index1, int index2) {
		return lua_rawequal(state_, index1, index2);
	}

	// lua_rawget
	// [-1, +1, -]
	// Similar to lua_gettable, but does a raw access (i.e., without metamethods).
	inline void rawget (int index) {
		lua_rawget(state_, index);
	}

	// lua_rawgeti
	// [-0, +1, -]
	// Pushes onto the stack the value t[n], where t is the value at the given valid index. The access is raw; that is, it does not invoke metamethods.
	inline void rawgeti (int index, int n) {
		lua_rawgeti(state_, index, n);
	}

	// lua_rawset
	// [-2, +0, m]
	// Similar to lua_settable, but does a raw assignment (i.e., without metamethods).
	inline void rawset (int index) {
		lua_rawset(state_, index);
	}

	// lua_rawseti
	// [-1, +0, m]
	// Does the equivalent of t[n] = v, where t is the value at the given valid index and v is the value at the top of the stack.
	// This function pops the value from the stack. The assignment is raw; that is, it does not invoke metamethods.
	inline void rawseti (int index, int n) {
		lua_rawseti(state_, index, n);
	}

	// lua_Reader

	// typedef const char * (*lua_Reader) (void *data, size_t *size);
	// The reader function used by lua_load. Every time it needs another piece of the chunk, lua_load calls the reader, passing along its data parameter. The reader must return a pointer to a block of memory with a new piece of the chunk and set size to the block size. The block must exist until the reader function is called again. To signal the end of the chunk, the reader must return NULL or set size to zero. The reader function may return pieces of any size greater than zero.

	// lua_register
	// [-0, +0, e]
	// Sets the C function f as the new value of global name. It is defined as a macro:
		 // #define lua_register(L,n,f) (lua_pushcfunction(L, f), lua_setglobal(L, n))
	inline void register_ (const char *name, lua_CFunction f) {
		lua_register(state_, name, f);
	}

	// lua_remove
	// [-1, +0, -]
	// Removes the element at the given valid index, shifting down the elements above this index to fill the gap. Cannot be called with a pseudo-index, because a pseudo-index is not an actual stack position.
	inline void remove (int index) {
		lua_remove(state_, index);
	}

	// lua_replace
	// [-1, +0, -]
	// Moves the top element into the given position (and pops it), without shifting any element (therefore replacing the value at the given position).
	inline void replace (int index) {
		lua_replace(state_, index);
	}

	// lua_resume
	// [-?, +?, -]
	// Starts and resumes a coroutine in a given thread.
	// To start a coroutine, you first create a new thread (see lua_newthread); then you push onto its stack the main function plus any arguments; then you call lua_resume, with narg being the number of arguments. This call returns when the coroutine suspends or finishes its execution. When it returns, the stack contains all values passed to lua_yield, or all values returned by the body function. lua_resume returns LUA_YIELD if the coroutine yields, 0 if the coroutine finishes its execution without errors, or an error code in case of errors (see lua_pcall). In case of errors, the stack is not unwound, so you can use the debug API over it. The error message is on the top of the stack. To restart a coroutine, you put on its stack only the values to be passed as results from yield, and then call lua_resume.
	inline int resume (int narg) {
		return lua_resume(state_, narg);
	}

	// lua_setallocf
	// [-0, +0, -]
	// Changes the allocator function of a given state to f with user data ud.
	inline void setallocf (lua_Alloc f, void *ud) {
		lua_setallocf(state_, f, ud);
	}

	// lua_setfenv
	// [-1, +0, -]
	// Pops a table from the stack and sets it as the new environment for the value at the given index. If the value at the given index is neither a function nor a thread nor a userdata, lua_setfenv returns 0. Otherwise it returns 1.
	inline int setfenv (int index) {
		return lua_setfenv(state_, index);
	}

	// lua_setfield
	// [-1, +0, e]
	// Does the equivalent to t[k] = v, where t is the value at the given valid index and v is the value at the top of the stack.
	// This function pops the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see §2.8).
	inline void setfield (int index, const char *k) {
		lua_setfield(state_, index, k);
	}

	// lua_setglobal
	// [-1, +0, e]
	// Pops a value from the stack and sets it as the new value of global name. It is defined as a macro:
		 // #define lua_setglobal(L,s)   lua_setfield(L, LUA_GLOBALSINDEX, s)
	inline void setglobal (const char *name) {
		lua_setglobal(state_, name);
	}

	// lua_setmetatable
	// [-1, +0, -]
	// Pops a table from the stack and sets it as the new metatable for the value at the given acceptable index.
	inline int setmetatable (int index) {
		return lua_setmetatable(state_, index);
	}

	// lua_settable
	// [-2, +0, e]
	// Does the equivalent to t[k] = v, where t is the value at the given valid index, v is the value at the top of the stack, and k is the value just below the top.
	// This function pops both the key and the value from the stack. As in Lua, this function may trigger a metamethod for the "newindex" event (see §2.8).
	inline void settable (int index) {
		lua_settable(state_, index);
	}

	// lua_settop
	// [-?, +?, -]
	// Accepts any acceptable index, or 0, and sets the stack top to this index. If the new top is larger than the old one, then the new elements are filled with nil. If index is 0, then all stack elements are removed.
	inline void settop (int index) {
		lua_settop(state_, index);
	}

	// lua_State

	// typedef struct lua_State lua_State;
	// Opaque structure that keeps the whole state of a Lua interpreter. The Lua library is fully reentrant: it has no global variables. All information about a state is kept in this structure.

	// A pointer to this state must be passed as the first argument to every function in the library, except to lua_newstate, which creates a Lua state from scratch.

	// lua_status
	// [-0, +0, -]
	// Returns the status of the thread L.
	// The status can be 0 for a normal thread, an error code if the thread finished its execution with an error, or LUA_YIELD if the thread is suspended.
	inline int status () {
		return lua_status(state_);
	}

	// lua_toboolean
	// [-0, +0, -]
	// Converts the Lua value at the given acceptable index to a C boolean value (0 or 1). Like all tests in Lua, lua_toboolean returns 1 for any Lua value different from false and nil; otherwise it returns 0. It also returns 0 when called with a non-valid index. (If you want to accept only actual boolean values, use lua_isboolean to test the value's type.)
	inline int toboolean (int index) {
		return lua_toboolean(state_, index);
	}

	// lua_tocfunction
	// [-0, +0, -]
	// Converts a value at the given acceptable index to a C function. That value must be a C function; otherwise, returns NULL.
	inline lua_CFunction tocfunction (int index) {
		return lua_tocfunction(state_, index);
	}

	// lua_tointeger
	// [-0, +0, -]
	// Converts the Lua value at the given acceptable index to the signed integral type lua_Integer. The Lua value must be a number or a string convertible to a number (see §2.2.1); otherwise, lua_tointeger returns 0.
	// If the number is not an integer, it is truncated in some non-specified way.
	inline lua_Integer tointeger (int index) {
		return lua_tointeger(state_, index);
	}

	// lua_tolstring
	// [-0, +0, m]
	// Converts the Lua value at the given acceptable index to a C string. If len is not NULL, it also sets *len with the string length. The Lua value must be a string or a number; otherwise, the function returns NULL. If the value is a number, then lua_tolstring also changes the actual value in the stack to a string. (This change confuses lua_next when lua_tolstring is applied to keys during a table traversal.)
	// lua_tolstring returns a fully aligned pointer to a string inside the Lua state. This string always has a zero ('\0') after its last character (as in C), but can contain other zeros in its body. Because Lua has garbage collection, there is no guarantee that the pointer returned by lua_tolstring will be valid after the corresponding value is removed from the stack.
	inline const char * tolstring (int index, size_t *len) {
		return lua_tolstring(state_, index, len);
	}

	// lua_tonumber
	// [-0, +0, -]
	// Converts the Lua value at the given acceptable index to the C type lua_Number (see lua_Number). The Lua value must be a number or a string convertible to a number (see §2.2.1); otherwise, lua_tonumber returns 0.
	inline lua_Number tonumber (int index) {
		return lua_tonumber(state_, index);
	}

	// lua_topointer
	// [-0, +0, -]
	// Converts the value at the given acceptable index to a generic C pointer (void*). The value can be a userdata, a table, a thread, or a function; otherwise, lua_topointer returns NULL. Different objects will give different pointers. There is no way to convert the pointer back to its original value.
	// Typically this function is used only for debug information.
	inline const void * topointer (int index) {
		return lua_topointer(state_, index);
	}

	// lua_tostring
	// [-0, +0, m]
	// Equivalent to lua_tolstring with len equal to NULL.
	inline const char * tostring (int index) {
		return lua_tostring(state_, index);
	}

	// lua_tothread
	// [-0, +0, -]
	// Converts the value at the given acceptable index to a Lua thread (represented as lua_State*). This value must be a thread; otherwise, the function returns NULL.
	inline lua_State * tothread (int index) {
		return lua_tothread(state_, index);
	}

	// lua_touserdata
	// [-0, +0, -]
	// If the value at the given acceptable index is a full userdata, returns its block address. If the value is a light userdata, returns its pointer. Otherwise, returns NULL.
	inline void * touserdata (int index) {
		return lua_touserdata(state_, index);
	}

	// lua_type
	// [-0, +0, -]
	// Returns the type of the value in the given acceptable index, or LUA_TNONE for a non-valid index (that is, an index to an "empty" stack position). The types returned by lua_type are coded by the following constants defined in lua.h: LUA_TNIL, LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING, LUA_TTABLE, LUA_TFUNCTION, LUA_TUSERDATA, LUA_TTHREAD, and LUA_TLIGHTUSERDATA.
	inline int type (int index) {
		return lua_type(state_, index);
	}

	// lua_typename
	// [-0, +0, -]
	// Returns the name of the type encoded by the value tp, which must be one the values returned by lua_type.
	inline const char * typename_  (int tp) {
		return lua_typename(state_, tp);
	}

	// lua_Writer

	// typedef int (*lua_Writer) (const void* p, size_t sz, void* ud);
	// The type of the writer function used by lua_dump. Every time it produces another piece of chunk, lua_dump calls the writer, passing along the buffer to be written (p), its size (sz), and the data parameter supplied to lua_dump.

	// The writer returns an error code: 0 means no errors; any other value means an error and stops lua_dump from calling the writer again.

	// lua_xmove
	// [-?, +?, -]
	// Exchange values between different threads of the same global state.
	// This function pops n values from the stack from, and pushes them onto the stack to.
	inline void xmove (lua_State *to, int n) {
		lua_xmove(state_, to, n);
	}

	// lua_yield
	// [-?, +?, -]
	// Yields a coroutine.
	// This function should only be called as the return expression of a C function, as follows:
	//	  return lua_yield (L, nresults);
	// When a C function calls lua_yield in that way, the running coroutine suspends its execution, and the call to lua_resume that started this coroutine returns. The parameter nresults is the number of values from the stack that are passed as results to lua_resume.
	inline int yield  (int nresults) {
		return lua_yield(state_, nresults);
	}

	// 3.8 – The Debug Interface

	// Lua has no built-in debugging facilities. Instead, it offers a special interface by means of functions and hooks. This interface allows the construction of different kinds of debuggers, profilers, and other tools that need "inside information" from the interpreter.

	// lua_Debug

	// typedef struct lua_Debug {
	//   int event;
	//   const char *name;		   /* (n) */
	//   const char *namewhat;	   /* (n) */
	//   const char *what;		   /* (S) */
	//   const char *source;		 /* (S) */
	//   int currentline;			/* (l) */
	//   int nups;				   /* (u) number of upvalues */
	//   int linedefined;			/* (S) */
	//   int lastlinedefined;		/* (S) */
	//   char short_src[LUA_IDSIZE]; /* (S) */
	//   /* private part */
	//   other fields
	// } lua_Debug;
	// A structure used to carry different pieces of information about an active function. lua_getstack fills only the private part of this structure, for later use. To fill the other fields of lua_Debug with useful information, call lua_getinfo.

	// The fields of lua_Debug have the following meaning:

	// source: If the function was defined in a string, then source is that string. If the function was defined in a file, then source starts with a '@' followed by the file name.
	// short_src: a "printable" version of source, to be used in error messages.
	// linedefined: the line number where the definition of the function starts.
	// lastlinedefined: the line number where the definition of the function ends.
	// what: the string "Lua" if the function is a Lua function, "C" if it is a C function, "main" if it is the main part of a chunk, and "tail" if it was a function that did a tail call. In the latter case, Lua has no other information about the function.
	// currentline: the current line where the given function is executing. When no line information is available, currentline is set to -1.
	// name: a reasonable name for the given function. Because functions in Lua are first-class values, they do not have a fixed name: some functions can be the value of multiple global variables, while others can be stored only in a table field. The lua_getinfo function checks how the function was called to find a suitable name. If it cannot find a name, then name is set to NULL.
	// namewhat: explains the name field. The value of namewhat can be "global", "local", "method", "field", "upvalue", or "" (the empty string), according to how the function was called. (Lua uses the empty string when no other option seems to apply.)
	// nups: the number of upvalues of the function.

	// lua_gethook
	// [-0, +0, -]
	// Returns the current hook function.
	inline lua_Hook gethook () {
		return lua_gethook(state_);
	}

	// lua_gethookcount
	// [-0, +0, -]
	// Returns the current hook count.
	inline int gethookcount () {
		return lua_gethookcount(state_);
	}

	// lua_gethookmask
	// [-0, +0, -]
	// Returns the current hook mask.
	inline int gethookmask () {
		return lua_gethookmask(state_);
	}

	// lua_getinfo
	// [-(0|1), +(0|1|2), m]
	// Returns information about a specific function or function invocation.
	// To get information about a function invocation, the parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook).
	// To get information about a function you push it onto the stack and start the what string with the character '>'. (In that case, lua_getinfo pops the function in the top of the stack.) For instance, to know in which line a function f was defined, you can write the following code:
	//	  lua_Debug ar;
	//	  lua_getfield(L, LUA_GLOBALSINDEX, "f");  /* get global 'f' */
	//	  lua_getinfo(L, ">S", &ar);
	//	  printf("%d\n", ar.linedefined);
	// Each character in the string what selects some fields of the structure ar to be filled or a value to be pushed on the stack:

	// 'n': fills in the field name and namewhat;
	// 'S': fills in the fields source, short_src, linedefined, lastlinedefined, and what;
	// 'l': fills in the field currentline;
	// 'u': fills in the field nups;
	// 'f': pushes onto the stack the function that is running at the given level;
	// 'L': pushes onto the stack a table whose indices are the numbers of the lines that are valid on the function. (A valid line is a line with some associated code, that is, a line where you can put a break point. Non-valid lines include empty lines and comments.)
	// This function returns 0 on error (for instance, an invalid option in what).
	inline int getinfo (const char *what, lua_Debug *ar) {
		return lua_getinfo(state_, what, ar);
	}

	// lua_getlocal
	// [-0, +(0|1), -]
	// Gets information about a local variable of a given activation record. The parameter ar must be a valid activation record that was filled by a previous call to lua_getstack or given as argument to a hook (see lua_Hook). The index n selects which local variable to inspect (1 is the first parameter or active local variable, and so on, until the last active local variable). lua_getlocal pushes the variable's value onto the stack and returns its name.
	// Variable names starting with '(' (open parentheses) represent internal variables (loop control variables, temporaries, and C function locals).
	// Returns NULL (and pushes nothing) when the index is greater than the number of active local variables.
	inline const char * getlocal (lua_Debug *ar, int n) {
		return lua_getlocal(state_, ar, n);
	}

	// lua_getstack
	// [-0, +0, -]
	// Get information about the interpreter runtime stack.
	// This function fills parts of a lua_Debug structure with an identification of the activation record of the function executing at a given level. Level 0 is the current running function, whereas level n+1 is the function that has called level n. When there are no errors, lua_getstack returns 1; when called with a level greater than the stack depth, it returns 0.
	inline int getstack (int level, lua_Debug *ar) {
		return lua_getstack(state_, level, ar);
	}

	// lua_getupvalue
	// [-0, +(0|1), -]
	// Gets information about a closure's upvalue. (For Lua functions, upvalues are the external local variables that the function uses, and that are consequently included in its closure.) lua_getupvalue gets the index n of an upvalue, pushes the upvalue's value onto the stack, and returns its name. funcindex points to the closure in the stack. (Upvalues have no particular order, as they are active through the whole function. So, they are numbered in an arbitrary order.)
	// Returns NULL (and pushes nothing) when the index is greater than the number of upvalues. For C functions, this function uses the empty string "" as a name for all upvalues.
	inline const char * getupvalue (int funcindex, int n) {
		return lua_getupvalue(state_, funcindex, n);
	}

	// lua_Hook

	// typedef void (*lua_Hook) (lua_Debug *ar);
	// Type for debugging hook functions.

	// Whenever a hook is called, its ar argument has its field event set to the specific event that triggered the hook. Lua identifies these events with the following constants: LUA_HOOKCALL, LUA_HOOKRET, LUA_HOOKTAILRET, LUA_HOOKLINE, and LUA_HOOKCOUNT. Moreover, for line events, the field currentline is also set. To get the value of any other field in ar, the hook must call lua_getinfo. For return events, event can be LUA_HOOKRET, the normal value, or LUA_HOOKTAILRET. In the latter case, Lua is simulating a return from a function that did a tail call; in this case, it is useless to call lua_getinfo.

	// While Lua is running a hook, it disables other calls to hooks. Therefore, if a hook calls back Lua to execute a function or a chunk, this execution occurs without any calls to hooks.

	// lua_sethook
	// [-0, +0, -]
	// Sets the debugging hook function.
	// Argument f is the hook function. mask specifies on which events the hook will be called: it is formed by a bitwise or of the constants LUA_MASKCALL, LUA_MASKRET, LUA_MASKLINE, and LUA_MASKCOUNT. The count argument is only meaningful when the mask includes LUA_MASKCOUNT. For each event, the hook is called as explained below:
	// The call hook: is called when the interpreter calls a function. The hook is called just after Lua enters the new function, before the function gets its arguments.
	// The return hook: is called when the interpreter returns from a function. The hook is called just before Lua leaves the function. You have no access to the values to be returned by the function.
	// The line hook: is called when the interpreter is about to start the execution of a new line of code, or when it jumps back in the code (even to the same line). (This event only happens while Lua is executing a Lua function.)
	// The count hook: is called after the interpreter executes every count instructions. (This event only happens while Lua is executing a Lua function.)
	// A hook is disabled by setting mask to zero.
	inline int sethook (lua_Hook f, int mask, int count) {
		return lua_sethook(state_, f, mask, count);
	}

	// lua_setlocal
	// [-(0|1), +0, -]
	// Sets the value of a local variable of a given activation record. Parameters ar and n are as in lua_getlocal (see lua_getlocal). lua_setlocal assigns the value at the top of the stack to the variable and returns its name. It also pops the value from the stack.
	// Returns NULL (and pops nothing) when the index is greater than the number of active local variables.
	inline const char * setlocal (lua_Debug *ar, int n) {
		return lua_setlocal(state_, ar, n);
	}

	// lua_setupvalue
	// [-(0|1), +0, -]
	// Sets the value of a closure's upvalue. It assigns the value at the top of the stack to the upvalue and returns its name. It also pops the value from the stack. Parameters funcindex and n are as in the lua_getupvalue (see lua_getupvalue).
	// Returns NULL (and pops nothing) when the index is greater than the number of upvalues.
	inline const char * setupvalue (int funcindex, int n) {
		return lua_setupvalue(state_, funcindex, n);
	}

	// 4 – The Auxiliary Library

	// The auxiliary library provides several convenient functions to interface C with Lua. While the basic API provides the primitive functions for all interactions between C and Lua, the auxiliary library provides higher-level functions for some common tasks.

	// All functions from the auxiliary library are defined in header file lauxlib.h and have a prefix luaL_.

	// All functions in the auxiliary library are built on top of the basic API, and so they provide nothing that cannot be done with this API.

	// Several functions in the auxiliary library are used to check C function arguments. Their names are always luaL_check* or luaL_opt*. All of these functions throw an error if the check is not satisfied. Because the error message is formatted for arguments (e.g., "bad argument #1"), you should not use these functions for other stack values.

	// 4.1 – Functions and Types

	// Here we list all functions and types from the auxiliary library in alphabetical order.

	// luaL_addchar
	// [-0, +0, m]
	// Adds the character c to the buffer B (see luaL_Buffer).
	inline void addchar (luaL_Buffer *B, char c) {
		luaL_addchar(B, c);
	}

	// luaL_addlstring
	// [-0, +0, m]
	// Adds the string pointed to by s with length l to the buffer B (see luaL_Buffer). The string may contain embedded zeros.
	inline void addlstring (luaL_Buffer *B, const char *s, size_t l) {
		luaL_addlstring(B, s, l);
	}

	// luaL_addsize
	// [-0, +0, m]
	// Adds to the buffer B (see luaL_Buffer) a string of length n previously copied to the buffer area (see luaL_prepbuffer).
	inline void addsize (luaL_Buffer *B, size_t n) {
		luaL_addsize(B, n);
	}

	// luaL_addstring
	// [-0, +0, m]
	// Adds the zero-terminated string pointed to by s to the buffer B (see luaL_Buffer). The string may not contain embedded zeros.
	inline void addstring (luaL_Buffer *B, const char *s) {
		luaL_addstring(B, s);
	}

	// luaL_addvalue
	// [-1, +0, m]
	// Adds the value at the top of the stack to the buffer B (see luaL_Buffer). Pops the value.
	// This is the only function on string buffers that can (and must) be called with an extra element on the stack, which is the value to be added to the buffer.
	inline void addvalue (luaL_Buffer *B) {
		luaL_addvalue(B);
	}

	// luaL_argcheck
	// [-0, +0, v]
	// Checks whether cond is true. If not, raises an error with the following message, where func is retrieved from the call stack:
		 // bad argument #<narg> to <func> (<extramsg>)
	inline void argcheck (int cond, int narg, const char *extramsg) {
		luaL_argcheck(state_, cond, narg, extramsg);
	}

	// luaL_argerror
	// [-0, +0, v]
	// Raises an error with the following message, where func is retrieved from the call stack:
	//	  bad argument #<narg> to <func> (<extramsg>)
	// This function never returns, but it is an idiom to use it in C functions as return luaL_argerror(args).
	inline int argerror (int narg, const char *extramsg) {
		return luaL_argerror(state_, narg, extramsg);
	}

	// luaL_Buffer

	// typedef struct luaL_Buffer luaL_Buffer;
	// Type for a string buffer.

	// A string buffer allows C code to build Lua strings piecemeal. Its pattern of use is as follows:

	// First you declare a variable b of type luaL_Buffer.
	// Then you initialize it with a call luaL_buffinit(L, &b).
	// Then you add string pieces to the buffer calling any of the luaL_add* functions.
	// You finish by calling luaL_pushresult(&b). This call leaves the final string on the top of the stack.
	// During its normal operation, a string buffer uses a variable number of stack slots. So, while using a buffer, you cannot assume that you know where the top of the stack is. You can use the stack between successive calls to buffer operations as long as that use is balanced; that is, when you call a buffer operation, the stack is at the same level it was immediately after the previous buffer operation. (The only exception to this rule is luaL_addvalue.) After calling luaL_pushresult the stack is back to its level when the buffer was initialized, plus the final string on its top.

	// luaL_buffinit
	// [-0, +0, -]
	// Initializes a buffer B. This function does not allocate any space; the buffer must be declared as a variable (see luaL_Buffer).
	inline void buffinit (luaL_Buffer *B) {
		luaL_buffinit(state_, B);
	}

	// luaL_callmeta
	// [-0, +(0|1), e]
	// Calls a metamethod.
	// If the object at index obj has a metatable and this metatable has a field e, this function calls this field and passes the object as its only argument. In this case this function returns 1 and pushes onto the stack the value returned by the call. If there is no metatable or no metamethod, this function returns 0 (without pushing any value on the stack).
	inline int callmeta (int obj, const char *e) {
		return luaL_callmeta(state_, obj, e);
	}

	// luaL_checkany
	// [-0, +0, v]
	// Checks whether the function has an argument of any type (including nil) at position narg.
	inline void checkany (int narg) {
		luaL_checkany(state_, narg);
	}

	// luaL_checkint
	// [-0, +0, v]
	// Checks whether the function argument narg is a number and returns this number cast to an int.
	inline int checkint (int narg) {
		return luaL_checkint(state_, narg);
	}

	// luaL_checkinteger
	// [-0, +0, v]
	// Checks whether the function argument narg is a number and returns this number cast to a lua_Integer.
	inline lua_Integer checkinteger (int narg) {
		return luaL_checkinteger(state_, narg);
	}

	// luaL_checklong
	// [-0, +0, v]
	// Checks whether the function argument narg is a number and returns this number cast to a long.
	inline long checklong (int narg) {
		return luaL_checklong(state_, narg);
	}

	// luaL_checklstring
	// [-0, +0, v]
	// Checks whether the function argument narg is a string and returns this string; if l is not NULL fills *l with the string's length.
	// This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here.
	inline const char * checklstring (int narg, size_t *l) {
		return luaL_checklstring(state_, narg, l);
	}

	// luaL_checknumber
	// [-0, +0, v]
	// Checks whether the function argument narg is a number and returns this number.
	inline lua_Number checknumber (int narg) {
		return luaL_checknumber(state_, narg);
	}

	// luaL_checkoption
	// [-0, +0, v]
	// Checks whether the function argument narg is a string and searches for this string in the array lst (which must be NULL-terminated). Returns the index in the array where the string was found. Raises an error if the argument is not a string or if the string cannot be found.
	// If def is not NULL, the function uses def as a default value when there is no argument narg or if this argument is nil.
	// This is a useful function for mapping strings to C enums. (The usual convention in Lua libraries is to use strings instead of numbers to select options.)
	inline int checkoption (int narg, const char *def, const char *const lst[]) {
		return luaL_checkoption(state_, narg, def, lst);
	}

	// luaL_checkstack
	// [-0, +0, v]
	// Grows the stack size to top + sz elements, raising an error if the stack cannot grow to that size. msg is an additional text to go into the error message.
	inline void checkstack (int sz, const char *msg) {
		luaL_checkstack(state_, sz, msg);
	}

	// luaL_checkstring
	// [-0, +0, v]
	// Checks whether the function argument narg is a string and returns this string.
	// This function uses lua_tolstring to get its result, so all conversions and caveats of that function apply here.
	inline const char * checkstring (int narg) {
		return luaL_checkstring(state_, narg);
	}

	// luaL_checktype
	// [-0, +0, v]
	// Checks whether the function argument narg has type t. See lua_type for the encoding of types for t.
	inline void checktype (int narg, int t) {
		luaL_checktype(state_, narg, t);
	}

	// luaL_checkudata
	// [-0, +0, v]
	// Checks whether the function argument narg is a userdata of the type tname (see luaL_newmetatable).
	inline void * checkudata (int narg, const char *tname) {
		return luaL_checkudata(state_, narg, tname);
	}

	// luaL_dofile
	// [-0, +?, m]
	// Loads and runs the given file. It is defined as the following macro:
	//	  (luaL_loadfile(L, filename) || lua_pcall(L, 0, LUA_MULTRET, 0))
	// It returns 0 if there are no errors or 1 in case of errors.
	inline int dofile (const char *filename) {
		return luaL_dofile(state_, filename);
	}

	// luaL_dostring
	// [-0, +?, m]
	// Loads and runs the given string. It is defined as the following macro:
	//	  (luaL_loadstring(L, str) || lua_pcall(L, 0, LUA_MULTRET, 0))
	// It returns 0 if there are no errors or 1 in case of errors.
	inline int dostring (const char *str) {
		return luaL_dostring(state_, str);
	}

	// luaL_error
	// [-0, +0, v]
	// Raises an error. The error message format is given by fmt plus any extra arguments, following the same rules of lua_pushfstring. It also adds at the beginning of the message the file name and the line number where the error occurred, if this information is available.
	// This function never returns, but it is an idiom to use it in C functions as return luaL_error(args).
	template<typename... Parameters>
	inline int error (const char *fmt, Parameters... params) {
		return luaL_error(state_, fmt, std::forward<Parameters>(params)...);
	}

	// luaL_getmetafield
	// [-0, +(0|1), m]
	// Pushes onto the stack the field e from the metatable of the object at index obj. If the object does not have a metatable, or if the metatable does not have this field, returns 0 and pushes nothing.
	inline int getmetafield (int obj, const char *e) {
		return luaL_getmetafield(state_, obj, e);
	}

	// luaL_getmetatable
	// [-0, +1, -]
	// Pushes onto the stack the metatable associated with name tname in the registry (see luaL_newmetatable).
	inline void getmetatable (const char *tname) {
		luaL_getmetatable(state_, tname);
	}

	// luaL_gsub
	// [-0, +1, m]
	// Creates a copy of string s by replacing any occurrence of the string p with the string r. Pushes the resulting string on the stack and returns it.
	inline const char * gsub (const char *s, const char *p, const char *r) {
		return luaL_gsub(state_, s, p, r);
	}

	// luaL_loadbuffer
	// [-0, +1, m]
	// Loads a buffer as a Lua chunk. This function uses lua_load to load the chunk in the buffer pointed to by buff with size sz.
	// This function returns the same results as lua_load. name is the chunk name, used for debug information and error messages.
	inline int loadbuffer (const char *buff, size_t sz, const char *name) {
		return luaL_loadbuffer(state_, buff, sz, name);
	}

	// luaL_loadfile
	// [-0, +1, m]
	// Loads a file as a Lua chunk. This function uses lua_load to load the chunk in the file named filename. If filename is NULL, then it loads from the standard input. The first line in the file is ignored if it starts with a #.
	// This function returns the same results as lua_load, but it has an extra error code LUA_ERRFILE if it cannot open/read the file.
	// As lua_load, this function only loads the chunk; it does not run it.
	inline int loadfile (const char *filename) {
		return luaL_loadfile(state_, filename);
	}

	// luaL_loadstring
	// [-0, +1, m]
	// Loads a string as a Lua chunk. This function uses lua_load to load the chunk in the zero-terminated string s.
	// This function returns the same results as lua_load.
	// Also as lua_load, this function only loads the chunk; it does not run it.
	inline int loadstring (const char *s) {
		return luaL_loadstring(state_, s);
	}

	// luaL_newmetatable
	// [-0, +1, m]
	// If the registry already has the key tname, returns 0. Otherwise, creates a new table to be used as a metatable for userdata, adds it to the registry with key tname, and returns 1.
	// In both cases pushes onto the stack the final value associated with tname in the registry.
	inline int newmetatable (const char *tname) {
		return luaL_newmetatable(state_, tname);
	}

	// luaL_newstate

	// [-0, +0, -]
	// lua_State *luaL_newstate (void);
	// Creates a new Lua state. It calls lua_newstate with an allocator based on the standard C realloc function and then sets a panic function (see lua_atpanic) that prints an error message to the standard error output in case of fatal errors.

	// Returns the new state, or NULL if there is a memory allocation error.

	// luaL_openlibs
	// [-0, +0, m]
	// Opens all standard Lua libraries into the given state.
	inline void openlibs () {
		luaL_openlibs(state_);
	}

	// luaL_optint
	// [-0, +0, v]
	// If the function argument narg is a number, returns this number cast to an int. If this argument is absent or is nil, returns d. Otherwise, raises an error.
	inline int optint (int narg, int d) {
		return luaL_optint(state_, narg, d);
	}

	// luaL_optinteger
	// [-0, +0, v]
	// If the function argument narg is a number, returns this number cast to a lua_Integer. If this argument is absent or is nil, returns d. Otherwise, raises an error.
	inline lua_Integer optinteger (int narg, lua_Integer d) {
		return luaL_optinteger(state_, narg, d);
	}

	// luaL_optlong
	// [-0, +0, v]
	// If the function argument narg is a number, returns this number cast to a long. If this argument is absent or is nil, returns d. Otherwise, raises an error.
	inline long optlong (int narg, long d) {
		return luaL_optlong(state_, narg, d);
	}

	// luaL_optlstring
	// [-0, +0, v]
	// If the function argument narg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error.
	// If l is not NULL, fills the position *l with the results's length.
	inline const char * optlstring (int narg, const char *d, size_t *l) {
		return luaL_optlstring(state_, narg, d, l);
	}

	// luaL_optnumber
	// [-0, +0, v]
	// If the function argument narg is a number, returns this number. If this argument is absent or is nil, returns d. Otherwise, raises an error.
	inline lua_Number optnumber (int narg, lua_Number d) {
		return luaL_optnumber(state_, narg, d);
	}

	// luaL_optstring
	// [-0, +0, v]
	// If the function argument narg is a string, returns this string. If this argument is absent or is nil, returns d. Otherwise, raises an error.
	inline const char * optstring (int narg, const char *d) {
		return luaL_optstring(state_, narg, d);
	}

	// luaL_prepbuffer
	// [-0, +0, -]
	// Returns an address to a space of size LUAL_BUFFERSIZE where you can copy a string to be added to buffer B (see luaL_Buffer). After copying the string into this space you must call luaL_addsize with the size of the string to actually add it to the buffer.
	inline void * prepbuffer (luaL_Buffer *B) {
		return luaL_prepbuffer(B);
	}

	// luaL_pushresult
	// [-?, +1, m]
	// Finishes the use of buffer B leaving the final string on the top of the stack.
	inline void pushresult (luaL_Buffer *B) {
		luaL_pushresult(B);
	}

	// luaL_ref
	// [-1, +0, m]
	// Creates and returns a reference, in the table at index t, for the object at the top of the stack (and pops the object).
	// A reference is a unique integer key. As long as you do not manually add integer keys into table t, luaL_ref ensures the uniqueness of the key it returns. You can retrieve an object referred by reference r by calling lua_rawgeti(L, t, r). Function luaL_unref frees a reference and its associated object.
	// If the object at the top of the stack is nil, luaL_ref returns the constant LUA_REFNIL. The constant LUA_NOREF is guaranteed to be different from any reference returned by luaL_ref.
	inline int ref (int t) {
		return luaL_ref(state_, t);
	}

	// luaL_Reg

	// typedef struct luaL_Reg {
	//   const char *name;
	//   lua_CFunction func;
	// } luaL_Reg;
	// Type for arrays of functions to be registered by luaL_register. name is the function name and func is a pointer to the function. Any array of luaL_Reg must end with an sentinel entry in which both name and func are NULL.

	// luaL_register
	// [-(0|1), +1, m]
	// Opens a library.
	// When called with libname equal to NULL, it simply registers all functions in the list l (see luaL_Reg) into the table on the top of the stack.
	// When called with a non-null libname, luaL_register creates a new table t, sets it as the value of the global variable libname, sets it as the value of package.loaded[libname], and registers on it all functions in the list l. If there is a table in package.loaded[libname] or in variable libname, reuses this table instead of creating a new one.
	// In any case the function leaves the table on the top of the stack.
	inline void register_L (const char *libname, const luaL_Reg *l) {
		luaL_register(state_, libname, l);
	}

	// luaL_typename
	// [-0, +0, -]
	// Returns the name of the type of the value at the given index.
	inline const char * typename_L (int index) {
		return luaL_typename(state_, index);
	}

	// luaL_typerror
	// [-0, +0, v]
	// Generates an error with a message like the following:
	//	  location: bad argument narg to 'func' (tname expected, got rt)
	// where location is produced by luaL_where, func is the name of the current function, and rt is the type name of the actual argument.
	inline int typerror (int narg, const char *tname) {
		return luaL_typerror(state_, narg, tname);
	}

	// luaL_unref
	// [-0, +0, -]
	// Releases reference ref from the table at index t (see luaL_ref). The entry is removed from the table, so that the referred object can be collected. The reference ref is also freed to be used again.
	// If ref is LUA_NOREF or LUA_REFNIL, luaL_unref does nothing.
	inline void unref (int t, int ref) {
		luaL_unref(state_, t, ref);
	}

	// luaL_where
	// [-0, +1, m]
	// Pushes onto the stack a string identifying the current position of the control at level lvl in the call stack. Typically this string has the following format:
	//	  chunkname:currentline:
	// Level 0 is the running function, level 1 is the function that called the running function, etc.

	// This function is used to build a prefix for error messages
	inline void where (int lvl) {
		luaL_where(state_, lvl);
	}
};

struct StackGuard {
	State* state_;
	int initTop_;
	bool ret_;
	StackGuard(State* state, bool ret = false)
		: state_ {state}
		, initTop_{state->gettop()}
		, ret_{ret}
	{}

	~StackGuard()
	{
		if(ret_) {

		}
		auto toPop = state_->gettop() - initTop_;
		if(toPop > 0) {
			state_->pop(toPop);
		}
	}
};

}