#pragma once
#pragma once
#define NOMINMAX
#include <array>
#include <windows.h>

struct paddle_stuff  //tile structure
{
	RECT paddle_position;
};

class paddle_dimensions //constants for numerical values
{
public:
	static constexpr LONG columns = 10;
	static constexpr LONG rows = 10;
	static constexpr LONG margin = 10;
	static constexpr LONG field_count = rows * columns;
	static constexpr LONG field_size = 60;
	static constexpr LONG width = 5;
	static constexpr LONG height = 20;
	using field_array = std::array<paddle_stuff, field_count>;
	paddle_dimensions();
	field_array const& fields() const { return m_fields; }
private:
	field_array m_fields;
};