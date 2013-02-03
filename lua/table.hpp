#pragma once

namespace lua
{

class Table
{
public:
	Table(const State& state) : state_{state} {}

	template<typename T>
	Value<T> operator[](const Variable& key) const
	{
		
	}

private:
	const State& state_;
};

}