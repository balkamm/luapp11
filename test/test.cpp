#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "lua/lua.hpp"

CATCH_TRANSLATE_EXCEPTION(lua::exception& e) {
  return std::string("Exception: ") + e.what() + "\n" + e.stack();
}