luapp11 0.1
===========

Luapp11 is a set of lua bindings for C++11.  It's designed to abstract away much of the dynamic and stack based nature of the lua C API and provide a nice clean well typed interface between c++11 code and lua code.  The only dependency is the c++11 standard library and a c++11 compliant compiler.

Using
-----

The luapp11 api is built around 2 basic types `lua::var` and `lua::val`.  `lua::var` represents a specific "place" in the lua environment.  It's the primary way values are gotten from and assigned to the lua environment.  `lua::val` is a variant type that can hold any data which can be stored in a lua table.  The only type that can be assigned to a `lua::var` is `lua::val` but most types can be implicitly converted to a `lua::val`.  This allows you to initialize a lua table like this:

    lua::root["foo"] = {
    	{"bar", 19},
    	{"baz", true},
    	{"table", {
    			{"nested", &userData}
    		}
    	}
    };

`lua::root` represents the root of the environment i.e. `LUA_GLOBALSINDEX`.  You can then get data out again in the same way:

    lua::var node = lua::root["foo"]["bar"];
    node.get<int>(); // == 19

There are 3 templated methods which can be used to probe and extract data from a `lua::var`.  Those being `is<T>`, `as<T>`, and `get<T>`.  `is<T>` returns a boolean which confirms whether a given `lua::var` can be extracted as a given c++ type.  It is guaranteed not to throw.  `get<T>` returns the value for a given `lua::var` as the specified type T.  If the `lua::var` does not exist in the lua environment, or if the type that is found there is not convertible to T then a `lua::exception` is thrown.  `as<T>` is a gentler version of `get<T>` which is basically equivalent to:

	template<typename T>
    T as(T&& default) {
      if(is<T>()) {
      	return get<T>();
      } else {
      	return default;
      }
    }

The last major thing that you can do with a `lua::var` is to attempt to execute it as a lua function.  `lua::var` defines both an `operator()` and an `invoke<T>` method.  The invoke method is required if you'd like the function you are calling to return a value.  Both of them return a `lua::result<T>` which either contains a `lua::error` if there was an error executing the lua code.  If you prefer exceptions to explicitly handling errors, the `result<T>` is implicitly convertible to `T` but might throw an exception if there was an error executing.  If you would like to return multiple values from an invocation.  You should call invoke with a `std::tuple` type.

Finally, if you just want to execute lua code, you can do so by calling `do_chunk("code here")`  if you call `do_chunk` on `lua::root`, then the code is executed in the global scope.  If you call `do_chunk` on a `lua::var` then the first return value is assigned to the `lua::var` that you executed it on.

This is a very early release.  There are plans in the works to include file loading (with sandboxing), a threading model, c++ function binding (with lambdas), and other features.  See MILESTONES.md for more details.

License
-------
The MIT License (MIT)

Copyright (c) 2013 Matthew Balkam

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.