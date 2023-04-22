#pragma once
#pragma once
#define NOMINMAX
#include <array>
#include <windows.h>
struct field
{
	RECT position;
};

class paddle
{
public:
	static constexpr LONG columns = 10;
	static constexpr LONG rows = 10;
	static constexpr LONG margin = 10;
	static constexpr LONG field_count = rows * columns;
	static constexpr LONG field_size = 60;
	static constexpr LONG width = 10;
	static constexpr LONG height = 20;
	using field_array = std::array<field, field_count>;
	paddle();
	field_array const& fields() const { return m_fields; }
private:
	field_array m_fields;
};




//#pragma once
//#define NOMINMAX
//#include <array>
//#include <windows.h>
//
//struct field  //tile structure
//{
//	RECT position;
//};
//
//class board //constants for numerical values
//{
//public:
//	static constexpr LONG columns = 4;
//	static constexpr LONG rows = 4;
//	static constexpr LONG margin = 10;
//	static constexpr LONG field_count = rows * columns;
//	static constexpr LONG field_size = 60;
//	static constexpr LONG width = columns * (field_size + margin) + margin;
//	static constexpr LONG height = rows * (field_size + margin) + margin;
//	using field_array = std::array<field, field_count>;
//	board();
//	field_array const& fields() const { return m_fields; }
//private:
//	field_array m_fields;
//};