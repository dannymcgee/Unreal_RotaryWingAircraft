/**
 * This is the code that goes inside the custom expression node in
 * `/RotaryWingAircraft/Content/Editor/M_CubicBezier`. This file doesn't
 * actually do anything, it's just easier to write the code in an actual code
 * editor and then paste it into the Material node instead of trying to author
 * it there.
 */
float YForX(float X, float2 P0, float2 P1, float2 P2, float2 P3)
{
	int iterations = 1;
	float lower = 0.0;
	float upper = 1.0;

	float t = (upper + lower) / 2.0;

	float2 a = lerp(P0, P1, t);
	float2 b = lerp(P1, P2, t);
	float2 c = lerp(P2, P3, t);
	float2 d = lerp(a, b, t);
	float2 e = lerp(b, c, t);
	float2 result = lerp(d, e, t);

	while (abs(X - result.x) > 0.001) {
		if (++iterations > 9999)
			break;

		if (X > result.x)
			lower = t;
		else
			upper = t;

		t = (upper + lower) / 2.0;

		a = lerp(P0, P1, t);
		b = lerp(P1, P2, t);
		c = lerp(P2, P3, t);
		d = lerp(a, b, t);
		e = lerp(b, c, t);
		result = lerp(d, e, t);
	}

	return result.y;
}
