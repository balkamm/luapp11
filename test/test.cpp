#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "luapp11/lua.hpp"

CATCH_TRANSLATE_EXCEPTION(luapp11::exception& e) {
  return std::string("Exception: ") + e.what() + "\n" + e.stack();
}