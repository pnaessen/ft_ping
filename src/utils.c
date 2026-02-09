#include "ft_ping.h"

double get_time_now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

bool is_deadline_reached(t_ping *ping, double start_time)
{
    if (ping->deadline > 0) {
	double elapsed = get_time_now() - start_time;
	if (elapsed >= ping->deadline)
	    return true;
    }
    return false;
}
