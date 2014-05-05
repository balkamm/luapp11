#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

namespace luapp11 {
class var;
class var_iterator;
class var_const_iterator;

class val;

template <typename T>
class userdata;

template <typename T>
class ptr;

template <typename T>
class result;
class error;
class exception;
class global;
}

#include "val_fwd.hpp"
#include "var_fwd.hpp"