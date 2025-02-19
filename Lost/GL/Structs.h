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

	};

}