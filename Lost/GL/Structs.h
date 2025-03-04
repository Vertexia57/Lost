#pragma once

namespace lost
{

	union Color
	{
		struct {
			float r, g, b, a;
		};
		float v[4];
		
		void normalize()
		{
			v[0] /= 255.0f;
			v[1] /= 255.0f;
			v[2] /= 255.0f;
			v[3] /= 255.0f;
		}

		Color normalized()
		{
			return { v[0] / 255.0f, v[1] / 255.0f, v[2] / 255.0f, v[3] / 255.0f };
		}

		void operator*=(const Color& other)
		{
			v[0] *= other.v[0];
			v[1] *= other.v[1];
			v[2] *= other.v[2];
			v[3] *= other.v[3];
		}

		Color operator*(const Color& other)
		{
			return Color{ v[0] * other.v[0], v[1] * other.v[1], v[2] * other.v[2], v[3] * other.v[3] };
		}

	};

}