#include "paddle.h"
paddle_dimensions::paddle_dimensions()
	: m_fields{ }
{
	for (LONG row = 0; row < rows; ++row)
		for (LONG column = 0; column < columns; ++column)
		{
			auto& f = m_fields[row * columns + column];
			f.paddle_position.top =
				row * (field_size + margin) + margin;

			f.paddle_position.left =
				column * (field_size + margin) + margin;
			f.paddle_position.bottom = f.paddle_position.top + field_size;
			f.paddle_position.right = f.paddle_position.left + field_size;
		}
}