#include <stdio.h>

float Q_rsqrt(float number)
{
	const float threehalfs = 1.5F;

	float x2 = number * 0.5F;
	float y = number;
	long i = * ( long * ) &y;
	i = 0x5f3759df - ( i >> 1 );
	y = * ( float * ) &i;
	y = y * ( threehalfs - ( x2 * y * y ) );  // 1st iteration
	y = y * ( threehalfs - ( x2 * y * y ) );  // 2nd iteration, this can be removed

	return y;
}

#include <cstdint>

float Q_rsqrt_safer(float number)
{
	const float threehalfs = 1.5F;

	float x2 = number * 0.5F;
	float y = number;

	static_assert(sizeof(float) == sizeof(int32_t));
	int32_t i = * reinterpret_cast<int32_t*>(&y);
	i = 0x5f3759df - ( i >> 1 );
	y = *reinterpret_cast<float *>(&i);
	y = y * ( threehalfs - ( x2 * y * y ) );  // 1st iteration
	y = y * ( threehalfs - ( x2 * y * y ) );  // 2nd iteration, this can be removed

    y = y * ( threehalfs - ( x2 * y * y ) );

	return y;
}

int main() {
    const float numbers[] = { 1 / 25.0F, 1 / 16.0F, 1 / 9.0F, 1 / 4.0F };
    for (float i : numbers) {
        printf("%f\n", Q_rsqrt(i));
    }
}