#pragma once

namespace luapp11 {

template<typename TDerived>
class userdata
{
public:
  virtual ~userdata() = default;

protected:
  userdata();
};

}