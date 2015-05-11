#ifndef _LINUX_RATELIMIT_H
#define _LINUX_RATELIMIT_H
#include <linux/param.h>

#define DEFAULT_RATELIMIT_INTERVAL (5 * HZ)
#define DEFAULT_RATELIMIT_BURST 10

struct ratelimit_state {
	int interval;
	int burst;
	int printed;
	int missed;
	unsigned long begin;
};

#define DEFINE_RATELIMIT_STATE(name, interval, burst)		\
		struct ratelimit_state name = {interval, burst,}

extern struct ratelimit_state printk_ratelimit_state;

extern int ___ratelimit(struct ratelimit_state *rs, const char *func);
#define __ratelimit(state) ___ratelimit(state, __func__)

#endif
