﻿/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE
 */

using System.Collections;
using System;

namespace Crown
{
	public struct Vector3
	{
		public float x, y, z;

		public Vector3(float x, float y, float z)
		{
			this.x = x;
			this.y = y;
			this.z = z;
		}

		public Vector3(double x, double y, double z)
		{
			this.x = (float)x;
			this.y = (float)y;
			this.z = (float)z;
		}

		public Vector3(ArrayList arr)
		{
			this.x = (float)(double)arr[0];
			this.y = (float)(double)arr[1];
			this.z = (float)(double)arr[2];
		}

		public static Vector3 operator+(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
		}

		public static Vector3 operator-(Vector3 a, Vector3 b)
		{
			return new Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
		}

		public static Vector3 operator*(Vector3 a, float k)
		{
			return new Vector3(a.x * k, a.y * k, a.z * k);
		}

		public static Vector3 operator*(float k, Vector3 a)
		{
			return a * k;
		}

		public override string ToString()
		{
			return string.Format("{0}, {1}, {2}", x, y, z);
		}
	}
}
