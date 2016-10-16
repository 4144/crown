/*
 * Copyright (c) 2012-2016 Daniele Bartolini and individual contributors.
 * License: https://github.com/taylor001/crown/blob/master/LICENSE-GPLv2
 */

using Gee;

namespace Crown
{
	public struct Vector2
	{
		public double x;
		public double y;

		public Vector2(double x, double y)
		{
			this.x = x;
			this.y = y;
		}

		public Vector2.from_array(ArrayList<Value?> arr)
		{
			this.x = (double)arr[0];
			this.y = (double)arr[1];
		}

		public string to_string()
		{
			return "%f, %f".printf(x, y);
		}
	}

	const Vector2 VECTOR2_ZERO = { 0.0, 0.0 };
	const Vector2 VECTOR2_ONE  = { 1.0, 1.0 };
}
