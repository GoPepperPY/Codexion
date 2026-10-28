#include "codexion.h"

static void     set_stop(t_sim *sim)
{
	pthread_mutex_lock(&sim->sim_lock);
	sim->stop = 1;
	pthread_mutex_unlock(&sim->sim_lock);
}

static int      reached_quota(t_sim *sim, int i)
{
	long    done;

	if (sim->cfg.number_of_compiles_required <= 0)
			return (0);
	pthread_mutex_lock(&sim->coder[i].state_lock);
	done = sim->coder[i].compiles_done;
	pthread_mutex_unlock(&sim->coder[i].state_lock);
	return (done >= sim->cfg.number_of_compiles_required);
}

static int      check_burnout(t_sim *sim, int i)
{
	long    last;
	long    now;

	pthread_mutex_lock(&sim->coder[i].state_lock);
	last = sim->coder[i].last_compile_start;
	pthread_mutex_unlock(&sim->coder[i].state_lock);
	now = now_in_ms();
	if (now - last > sim->cfg.time_to_bo)
	{
			pthread_mutex_lock(&sim->print_lock);
			set_stop(sim);
			printf("%ld %d burned out\n", now - sim->start_time, sim->coder[i].id);
			pthread_mutex_unlock(&sim->print_lock);
			return (1);
	}
	return (0);
}

void    *monitor_routine(void *data)
{
	t_sim   *sim;
	int             i;
	int             full;

	sim = (t_sim *)data;
	while (!should_stop(sim))
	{
			i = -1;
			full = 1;
			while (++i < sim->cfg.coders)
			{
					if (check_burnout(sim, i))
							return (NULL);
					if (!reached_quota(sim, i))
							full = 0;
			}
			if (sim->cfg.number_of_compiles_required > 0 && full)
					return (set_stop(sim), NULL);
			usleep(1000);
	}
	return (NULL);
}