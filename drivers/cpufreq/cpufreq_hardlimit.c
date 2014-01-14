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

/*
 * CPU freq hard limit - SysFS interface :
 * ---------------------------------------
 *
 * /sys/kernel/cpufreq/hardlimit (rw)
 *
 *   set or show the real hard CPU frequency limit when screen is on
 *
 * /sys/kernel/cpufreq/hardlimit_screen_off (rw)
 *
 *   set or show the real hard CPU frequency limit when screen is off
 *
 * /sys/kernel/cpufreq/available_frequencies (ro)
 *
 *   display list of available CPU frequencies for convenience
 *
 * /sys/kernel/cpufreq/screen_state (ro)
 *
 *   display current screen state as seen by CPU hardlimit
 *
 * /sys/kernel/cpufreq/current_limit (ro)
 *
 *   display current applied hardlimit
 *
 * /sys/kernel/cpufreq/version (ro)
 *
 *   display CPU freq hard limit version information
 *
 */

#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/cpufreq_hardlimit.h>
#include <linux/cpufreq.h>
#include <linux/powersuspend.h>

unsigned int hardlimit_max_screen_on  = CPUFREQ_HARDLIMIT_MAX_SCREEN_ON_STOCK;
unsigned int hardlimit_max_screen_off = CPUFREQ_HARDLIMIT_MAX_SCREEN_OFF_STOCK;
unsigned int screen_state = CPUFREQ_HARDLIMIT_SCREEN_ON;

/* Externally reachable function call */

unsigned int check_cpufreq_hardlimit(unsigned int freq)
{
	switch(screen_state) {
		case CPUFREQ_HARDLIMIT_SCREEN_ON:	return min(hardlimit_max_screen_on , freq); /* Enforce screen  on max hard limit */
		case CPUFREQ_HARDLIMIT_SCREEN_OFF:	return min(hardlimit_max_screen_off, freq); /* Enforce screen off max hard limit */
	}

	return freq; /* This should never happen */
}

/* Update scaling_max_freq */
void reapply_hard_limit(void)
{
	switch(screen_state) {
		case CPUFREQ_HARDLIMIT_SCREEN_ON:	update_scaling_max(hardlimit_max_screen_on);
							break;
		case CPUFREQ_HARDLIMIT_SCREEN_OFF:	update_scaling_max(hardlimit_max_screen_off);
							break;
	}
}

/* Powersuspend */
static void cpufreq_hardlimit_suspend(struct power_suspend * h)
{
	screen_state = CPUFREQ_HARDLIMIT_SCREEN_OFF;
	reapply_hard_limit();
	return;
}

static void cpufreq_hardlimit_resume(struct power_suspend * h)
{
	screen_state = CPUFREQ_HARDLIMIT_SCREEN_ON;
	reapply_hard_limit();
	return;
}

static struct power_suspend cpufreq_hardlimit_suspend_data =
{
	.suspend = cpufreq_hardlimit_suspend,
	.resume = cpufreq_hardlimit_resume,
};

/* sysfs interface for "hardlimit" */
static ssize_t hardlimit_max_screen_on_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", hardlimit_max_screen_on);
}

static ssize_t hardlimit_max_screen_on_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	unsigned int new_hardlimit, i;

	struct cpufreq_frequency_table *table;

	if (!sscanf(buf, "%du", &new_hardlimit))
		return -EINVAL;

	if (new_hardlimit == hardlimit_max_screen_on)
		return count;

	table = cpufreq_frequency_get_table(0); /* Get frequency table */

	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
		if (table[i].frequency == new_hardlimit) {
		    hardlimit_max_screen_on = new_hardlimit;
		    reapply_hard_limit();
		    return count;
		}

	return -EINVAL;

}

static struct kobj_attribute hardlimit_max_screen_on_attribute =
__ATTR(hardlimit, 0666, hardlimit_max_screen_on_show, hardlimit_max_screen_on_store);

/* sysfs interface for "hardlimit_screen_off" */
static ssize_t hardlimit_max_screen_off_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", hardlimit_max_screen_off);
}

static ssize_t hardlimit_max_screen_off_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{

	unsigned int new_hardlimit, i;

	struct cpufreq_frequency_table *table;

	if (!sscanf(buf, "%du", &new_hardlimit))
		return -EINVAL;

	if (new_hardlimit == hardlimit_max_screen_off)
		return count;

	table = cpufreq_frequency_get_table(0); /* Get frequency table */

	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
		if (table[i].frequency == new_hardlimit) {
		    hardlimit_max_screen_off = new_hardlimit;
		    reapply_hard_limit();
		    return count;
		}

	return -EINVAL;

}

static struct kobj_attribute hardlimit_max_screen_off_attribute =
__ATTR(hardlimit_screen_off, 0666, hardlimit_max_screen_off_show, hardlimit_max_screen_off_store);

/* sysfs interface for "available_frequencies" */
static ssize_t available_frequencies_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	unsigned int i;
	ssize_t j = 0;

	struct cpufreq_frequency_table *table;

	table = cpufreq_frequency_get_table(0); /* Get frequency table */

	for (i = 0; (table[i].frequency != CPUFREQ_TABLE_END); i++)
		j += sprintf(&buf[j], "%d ", table[i].frequency);

	j += sprintf(&buf[j], "\n");
	return j;
}

static struct kobj_attribute available_frequencies_attribute =
__ATTR(available_frequencies, 0444, available_frequencies_show, NULL);

/* sysfs interface for "screen_state" */
static ssize_t screen_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	switch(screen_state) {
		case CPUFREQ_HARDLIMIT_SCREEN_ON:	return sprintf(buf, "Screen on\n");
		case CPUFREQ_HARDLIMIT_SCREEN_OFF:	return sprintf(buf, "Screen off\n");
	}

	return sprintf(buf, "undefined\n");
}

static struct kobj_attribute screen_state_attribute =
__ATTR(screen_state, 0444, screen_state_show, NULL);

/* sysfs interface for "current_limit" */
static ssize_t current_limit_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	switch(screen_state) {
		case CPUFREQ_HARDLIMIT_SCREEN_ON:	return sprintf(buf, "%d\n", hardlimit_max_screen_on);
		case CPUFREQ_HARDLIMIT_SCREEN_OFF:	return sprintf(buf, "%d\n", hardlimit_max_screen_off);
	}

	return sprintf(buf, "undefined\n");
}

static struct kobj_attribute current_limit_attribute =
__ATTR(current_limit, 0444, current_limit_show, NULL);

/* sysfs interface for "version" */
static ssize_t version_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", CPUFREQ_HARDLIMIT_VERSION);
}

static struct kobj_attribute version_attribute =
__ATTR(version, 0444, version_show, NULL);

/* Initialize sysfs folder */
static struct kobject *hardlimit_kobj;

static struct attribute *hardlimit_attrs[] = {
	&hardlimit_max_screen_on_attribute.attr,
	&hardlimit_max_screen_off_attribute.attr,
	&available_frequencies_attribute.attr,
	&screen_state_attribute.attr,
	&current_limit_attribute.attr,
	&version_attribute.attr,
	NULL,
};

static struct attribute_group hardlimit_attr_group = {
.attrs = hardlimit_attrs,
};

int hardlimit_init(void)
{
	int hardlimit_retval;

        hardlimit_kobj = kobject_create_and_add("cpufreq", kernel_kobj);
        if (!hardlimit_kobj) {
                return -ENOMEM;
        }
        hardlimit_retval = sysfs_create_group(hardlimit_kobj, &hardlimit_attr_group);
        if (hardlimit_retval)
                kobject_put(hardlimit_kobj);
	else
		/* Only register to powersuspend if we were able to create the sysfs interface */
		register_power_suspend(&cpufreq_hardlimit_suspend_data);

        return (hardlimit_retval);
}
/* end sysfs interface */

void hardlimit_exit(void)
{
	unregister_power_suspend(&cpufreq_hardlimit_suspend_data);
	kobject_put(hardlimit_kobj);
}

module_init(hardlimit_init);
module_exit(hardlimit_exit);
