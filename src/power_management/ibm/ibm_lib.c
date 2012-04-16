#include <stdio.h>
#include <stdlib.h>

#include "lib_utils.h"
#include "power_management.h"

#define FAN_FILE "/proc/acpi/ibm/fan"
#define THERMAL_FILE "/proc/acpi/ibm/thermal"

/* we check for /proc/acpi/ibm/fan */
int machine_is_ibm(void)
{
	FILE *fp = fopen(FAN_FILE, "r");

	if (fp)
	{
		fclose(fp);
		return 1;
	}

	return 0;
}

int ibm_get_fan_status(void)
{
	FILE *fp = fopen(FAN_FILE, "r");
	int fan_status = 0;

	if (!fp) return PM_Error;

	fscanf(fp, "%*s%*s%*s%d", &fan_status);
	fclose(fp);

	return(fan_status);
}

int ibm_get_temperature(void)
{
	FILE *fp = fopen(THERMAL_FILE, "r");
	int result;	

	if (!fp) return PM_Error;

	if (fscanf(fp, "%*s%d", &result) == 1)
	{
		fclose(fp);
		return result;
	}
	fclose(fp);

	return PM_Error;
}
