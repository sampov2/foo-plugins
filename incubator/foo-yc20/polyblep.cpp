#include <sys/types.h>
#include <math.h>
#include <stdio.h>

float *polyblep_table;
int polyblep_table_size;


double calculate_polyblep(double t)
{
	t -= 0.5;
	t *= 2.0;

	if (t>0) {
		return (t - (t*t)/2.0 - 0.5);
	} else {
		return ((t*t)/2.0 + t + 0.5);
	}
}

void polyblep_init()
{
	polyblep_table_size = 16384;
	polyblep_table = new float[polyblep_table_size];

	for (int i = 0; i < polyblep_table_size; ++i) {
		polyblep_table[i] = (float)calculate_polyblep( (double)i/(double)polyblep_table_size);
	}
}

// f ranges from -1 to +1
float polyblep_table_lookup(float f)
{
	int i = (1.0 + f) * (float)(polyblep_table_size-1) / 2.0;

	if (i < 0 || i >=polyblep_table_size) {
		fprintf(stderr,"PolyBLEP Error! index out of bounds, %d (%f)\n",i,f);
		return 0.0;
	}

	return polyblep_table[i];
}

//#define polyblep_it(x) polyblep_table_lookup((x))
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
	default:
		return prev - polyblep_it((ph_1 - 0.5)/q) * 2;
	}
}
