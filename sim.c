#include "codexion.h"

void	log_state(t_coder *coder, const char *msg)
{
	long	ts;
	// The lock exists so that when coder 1 and coder 2 both print at the same instant, their lines don't interleave character-by-character into garbage.
	pthread_mutex_lock(&coder->sim->print_lock);
	ts = now_in_ms() - coder->sim->start_time;
	printf("%ld %d %s\n", ts, coder->id, msg);
	pthread_mutex_unlock(&coder->sim->print_lock);
}
