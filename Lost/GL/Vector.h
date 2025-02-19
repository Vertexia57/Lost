#pragma once

namespace lost
{



	union Vec2
	{
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
		struct
		{
			int x, y;
		};
		int v[2];
	};

	union Vec3
	{
		struct
		{
			float x, y, z;
		};
		struct
		{
			Vec2 xy;
			float z;
		};
		float v[3];
	};

	union IVec3
	{
		struct
		{
			int x, y, z;
		};
		struct
		{
			IVec2 xy;
			int z;
		};
		int v[3];
	};

	union Vec4
	{
		struct
		{
			float x, y, z, w;
		};
		struct
		{
			Vec3 xyz;
			float w;
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