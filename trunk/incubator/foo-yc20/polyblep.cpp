#include <sys/types.h>
#include <math.h>
#include <stdio.h>

inline float calculate_polyblep(float t)
{
	if (t>0) {
		return (t - (t*t)/2.0 - 0.5);
	} else {
		return ((t*t)/2.0 + t + 0.5);
	}
}

#define polyblep_it(x) calculate_polyblep((x))

float square_blep_x(int select, float x, float ph, float q)
{
	switch(select) {
	case 0:
		return x;
	case 1:
		return x + polyblep_it(ph / q)*2;
	case 2:
	default:
		return x - polyblep_it((ph - 0.5)/q)*2;
	}
}

float square_blep_prev(int select, float prev, float ph_1, float q)
{
	switch(select) {
	case 0:
		return prev;
	case 1:
		return prev + polyblep_it((ph_1 - 1.0)/q) * 2;
	case 2:
		return prev - polyblep_it((ph_1 - 0.5)/q) * 2;
	default:
		fprintf(stderr,"square_blep_prev(%d,%f,%f,%f)\n",select,prev,ph_1,q);
		return -100;
	}
}
