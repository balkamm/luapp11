#pragma once

namespace luapp11 {

class root {
 public:
  root() : L { luaL_newstate() }
  {
    luaL_openlibs(L);
    lua_atpanic(L, &panic);
  }

  var operator[](val key) const { return var(L, LUA_GLOBALSINDEX, key); }

 private:
  static int panic(lua_State* L) { throw luapp11::exception("lua panic", L); }

  lua_State* L;

  friend error do_chunk(const std::string& str);
  friend error do_file(const std::string& path);
};

static root root;

inline error do_chunk(const std::string& str) {
  int loadError = luaL_loadstring(root.L, str.c_str());
  if (loadError != 0) {
    return error(loadError, "Error loading chunk.", root.L);
  }
  int runError = lua_pcall(root.L, 0, LUA_MULTRET, 0);
  if (runError != 0) {
    return error(runError, "Error running chunk.", root.L);
  }
  return error();
}

inline error do_file(const std::string& path) {
  int loadError = luaL_loadfile(root.L, path.c_str());
  if (loadError != 0) {
    return error(loadError, "Error loading chunk.", root.L);
  }
  int runError = lua_pcall(root.L, 0, LUA_MULTRET, 0);
  if (runError != 0) {
    return error(runError, "Error running chunk.", root.L);
  }
  return error();
}

}