#include <stdio.h>
#include <stdlib.h>

#include "lib_utils.h"
#include "power_management.h"

#define PROC_FILE "/proc/i8k"

/* we check for /proc/i18k */
int machine_is_dell(void)
{
	FILE *fp = fopen(PROC_FILE, "r");

	if (fp)
	{
		fclose(fp);
		return 1;
	}

	return 0;
}

int dell_get_fan_status(void)
{
	FILE *fp = fopen(PROC_FILE, "r");
	string fan_status = NULL;
	string fan_2_status = NULL;
	int value;
	int result = 0;

	if (!fp) return PM_Error;

	scan(fp, "%s%s%s%s%s%s", NULL, NULL, NULL, NULL, &fan_status, &fan_2_status);
	fclose(fp);

	if (fan_status)
	{
		value = atoi(fan_status);
		free(fan_status);
		if (value > 0) result++;
	}
	if (fan_2_status)
	{
		value = atoi(fan_2_status);
		free(fan_2_status);
		if (value > 0) result++;
	}

	return result;
}

int dell_get_temperature(void)
{
	FILE *fp = fopen(PROC_FILE, "r");
	string temp = NULL;
	int result = PM_Error;

	if (!fp) return PM_Error;

	scan(fp, "%s%s%s%s", NULL, NULL, NULL, &temp);
	fclose(fp);

	if (temp)
	{
		result = atoi(temp);
		free(temp);
	}

	return result;
}
