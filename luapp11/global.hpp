#pragma once

namespace luapp11 {

class global {
 public:
  global() : L{luaL_newstate()} {
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

static global global;

inline error do_chunk(const std::string& str) {
  int loadError = luaL_loadstring(global.L, str.c_str());
  if (loadError != 0) {
    return error(loadError, "Error loading chunk.", global.L);
  }
  int runError = lua_pcall(global.L, 0, LUA_MULTRET, 0);
  if (runError != 0) {
    return error(runError, "Error running chunk.", global.L);
  }
  return error();
}

inline error do_file(const std::string& path) {
  int loadError = luaL_loadfile(global.L, path.c_str());
  if (loadError != 0) {
    return error(loadError, "Error loading chunk.", global.L);
  }
  int runError = lua_pcall(global.L, 0, LUA_MULTRET, 0);
  if (runError != 0) {
    return error(runError, "Error running chunk.", global.L);
  }
  return error();
}
}