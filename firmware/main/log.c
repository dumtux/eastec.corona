
#include "log.h"


void init_sample(void)
{

}

void add_sample(const measurement_t * sample)
{

}
void get_sample(uint32_t sample_idx, measurement_t * sample)
{
	measurement_t meas = {sample_idx,0,101,201,301,401,1};
	*sample = meas;
}
