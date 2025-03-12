#pragma once
#include <math.h>
#include "glm/glm.hpp"

#ifndef PI
#define PI 3.141592653589
#endif
#ifndef TWO_PI
#define TWO_PI 6.28318530717958
#endif
#ifndef TAU
#define TAU 6.28318530717958
#endif

namespace lost
{

	union Vec2
	{
		Vec2() : x(0.0f), y(0.0f) {};
		Vec2(float x_, float y_) : x(x_), y(y_) {};

		void operator+=(const Vec2& rhs)
		{
			v[0] += rhs.v[0];
			v[1] += rhs.v[1];
		}

		void operator-=(const Vec2& rhs)
		{
			v[0] -= rhs.v[0];
			v[1] -= rhs.v[1];
		}

		void operator*=(const Vec2& rhs)
		{
			v[0] *= rhs.v[0];
			v[1] *= rhs.v[1];
		}

		void operator/=(const Vec2& rhs)
		{
			v[0] /= rhs.v[0];
			v[1] /= rhs.v[1];
		}

		Vec2 operator+(const Vec2& rhs) const
		{
			return { v[0] + rhs.v[0], v[1] + rhs.v[1] };
		}

		Vec2 operator-(const Vec2& rhs) const
		{
			return { v[0] - rhs.v[0], v[1] - rhs.v[1] };
		}

		Vec2 operator*(const Vec2& rhs) const
		{
			return { v[0] * rhs.v[0], v[1] * rhs.v[1] };
		}

		Vec2 operator/(const Vec2& rhs) const
		{
			return { v[0] / rhs.v[0], v[1] / rhs.v[1] };
		}

		Vec2 operator*(float rhs) const
		{
			return { v[0] * rhs, v[1] * rhs };
		}

		Vec2 operator/(float rhs) const
		{
			return { v[0] / rhs, v[1] / rhs };
		}

		Vec2 normalized() const
		{
			float dist = sqrtf(powf(v[0], 2) + powf(v[1], 2));
			if (dist >= 0.00000000000001f)
				return { v[0] / dist, v[1] / dist };
			return { 0.0f, 0.0f };
		}

		void normalize()
		{
			float dist = sqrtf(powf(v[0], 2) + powf(v[1], 2));
			if (dist >= 0.00000000000001f)
			{
				v[0] /= dist;
				v[1] /= dist;
				return;
			}

			v[0] = 0.0f;
			v[1] = 0.0f;
		}

		glm::vec2 getGLM() const
		{
			return { v[0], v[1] };
		}

		float dot(const Vec2& rhs) const
		{
			return (v[0] * rhs.v[0]) + (v[1] * rhs.v[1]);
		}

		struct
		{
			float x, y;
		};
		struct
		{
			float w, h;
		};
		float v[2];
	};

	union IVec2
	{
		IVec2() : x(0), y(0) {};
		IVec2(int x_, int y_) : x(x_), y(y_) {};

		struct
		{
			int x, y;
		};
		int v[2];
	};

	union Vec3
	{
		Vec3() : x(0.0f), y(0.0f), z(0.0f) {};
		Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {};
		Vec3(Vec2 xy, float z_) : x(xy.x), y(xy.y), z(z_) {};

		void operator+=(const Vec3& rhs)
		{
			v[0] += rhs.v[0];
			v[1] += rhs.v[1];
			v[2] += rhs.v[2];
		}

		void operator-=(const Vec3& rhs)
		{
			v[0] -= rhs.v[0];
			v[1] -= rhs.v[1];
			v[2] -= rhs.v[2];
		}

		void operator*=(const Vec3& rhs)
		{
			v[0] *= rhs.v[0];
			v[1] *= rhs.v[1];
			v[2] *= rhs.v[2];
		}

		void operator/=(const Vec3& rhs)
		{
			v[0] /= rhs.v[0];
			v[1] /= rhs.v[1];
			v[2] /= rhs.v[2];
		}

		Vec3 operator+(const Vec3& rhs) const
		{
			return { v[0] + rhs.v[0], v[1] + rhs.v[1], v[2] + rhs.v[2] };
		}

		Vec3 operator-(const Vec3& rhs) const
		{
			return { v[0] - rhs.v[0], v[1] - rhs.v[1], v[2] - rhs.v[2] };
		}

		Vec3 operator*(const Vec3& rhs) const
		{
			return { v[0] * rhs.v[0], v[1] * rhs.v[1], v[2] * rhs.v[2] };
		}

		Vec3 operator/(const Vec3& rhs) const
		{
			return { v[0] / rhs.v[0], v[1] / rhs.v[1], v[2] / rhs.v[2] };
		}

		Vec3 operator*(float rhs) const
		{
			return { v[0] * rhs, v[1] * rhs, v[2] * rhs };
		}

		Vec3 operator/(float rhs) const
		{
			return { v[0] / rhs, v[1] / rhs, v[2] / rhs };
		}

		Vec3 normalized() const
		{
			float dist = sqrtf(powf(v[0], 2) + powf(v[1], 2) + powf(v[2], 2));
			if (dist >= 0.00000000000001f)
				return { v[0] / dist, v[1] / dist, v[2] / dist };
			return { 0.0f, 0.0f, 0.0f };
		}

		void normalize()
		{
			float dist = sqrtf(powf(v[0], 2) + powf(v[1], 2) + powf(v[2], 2));
			if (dist >= 0.00000000000001f)
			{
				v[0] /= dist;
				v[1] /= dist;
				v[2] /= dist;
				return;
			}

			v[0] = 0.0f;
			v[1] = 0.0f;
			v[2] = 0.0f;
		}

		glm::vec3 getGLM() const
		{
			return { v[0], v[1], v[2] };
		}

		float dot(const Vec3& rhs) const
		{
			return (v[0] * rhs.v[0]) + (v[1] * rhs.v[1]) + (v[2] * rhs.v[2]);
		}

		struct
		{
			float x, y, z;
		};
		struct
		{
			Vec2 xy;
			float z;
		};
		struct
		{
			float r, g, b;
		};
		float v[3];
	};

	union IVec3
	{
		IVec3() : x(0), y(0), z(0) {};
		IVec3(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {};
		IVec3(IVec2 xy, int z_) : x(xy.x), y(xy.y), z(z_) {};

		struct
		{
			int x, y, z;
		};
		struct
		{
			IVec2 xy;
			int z;
		};
		struct
		{
			int r, g, b;
		};
		int v[3];
	};

	union Vec4
	{
		Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {};
		Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {};
		Vec4(Vec3 xyz, float w_) : x(xyz.x), y(xyz.y), z(xyz.z), w(w_) {};
		Vec4(Vec2 xy, Vec2 zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {};


		glm::vec4 getGLM() const
		{
			return { v[0], v[1], v[2], v[3] };
		}

		struct
		{
			float x, y, z, w;
		};
		struct
		{
			Vec3 xyz;
			float w;
		};
		struct
		{
			float r, g, b, a;
		};
		float v[4];
	};

	union IVec4
	{
		struct
		{
			int x, y, z, w;
		};
		struct
		{
			IVec3 xyz;
			int w;
		};
		struct
		{
			int r, g, b, a;
		};
		int v[4];
	};

	union Bounds2D
	{
		struct
		{
			float x, y, w, h;
		};
		struct
		{
			Vec2 pos;
			Vec2 size;
		};
		float v[4];
	};
}