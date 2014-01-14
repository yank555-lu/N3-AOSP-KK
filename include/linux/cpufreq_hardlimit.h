/*
 * Author: Jean-Pierre Rasquin <yank555.lu@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LINUX_CPUFREQ_HARDLIMIT_H
#define _LINUX_CPUFREQ_HARDLIMIT_H

#define CPUFREQ_HARDLIMIT_VERSION "v1.2 by Yank555.lu"

#define CPUFREQ_HARDLIMIT_MAX_SCREEN_ON_STOCK  2265600	/* default */
#define CPUFREQ_HARDLIMIT_MAX_SCREEN_OFF_STOCK 2265600	/* default */

#define CPUFREQ_HARDLIMIT_SCREEN_ON	0		/* default, consider we boot with screen on */
#define CPUFREQ_HARDLIMIT_SCREEN_OFF	1

unsigned int check_cpufreq_hardlimit(unsigned int freq);

void update_scaling_max(unsigned int freq);		/* Hook in cpufreq */

#endif

