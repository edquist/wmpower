/***************************************************************************
                          libacpi.c  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003 by Noberasco Michele
    e-mail               : 2001s098@educ.disi.unige.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.              *
 *                                                                         *
 ***************************************************************************/

 /***************************************************************************
        Originally written by Costantino Pistagna for his wmacpimon
 ***************************************************************************/


#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "libacpi.h"
#include "power_management.h"
#include "lib_utils.h"


/* here we put temp stuff read from proc files */
char buf[512];

static char batteries[MAXBATT][128];
static char  battinfo[MAXBATT][128];


/* see if we have ACPI support */
int check_acpi(void)
{
  DIR *battdir;
  struct dirent *batt;
  char *name;

	/* do proc entries for acpi exist? */
	if (access("/proc/acpi/info", R_OK) != 0) return 0;

  /* now enumerate batteries */
  batt_count = 0;
  battdir = opendir ("/proc/acpi/battery");
  if (!battdir) return 0;
  while ((batt = readdir (battdir)))
  {
    name = batt->d_name;

    /* skip . and .. */
    if (!strncmp (".", name, 1) || !strncmp ("..", name, 2)) continue;
    if (!access("/proc/acpi/battery/1/status", R_OK))
	    sprintf (batteries[batt_count], "/proc/acpi/battery/%s/status", name);
    else
	    sprintf (batteries[batt_count], "/proc/acpi/battery/%s/state", name);
    sprintf (battinfo[batt_count], "/proc/acpi/battery/%s/info", name);
    batt_count++;
  }
  closedir (battdir);

  return 1;
}


void read_acad_state (ACADstate *acadstate)
{
	FILE *fp;
	char *where = NULL;

	if (!(fp = fopen ("/proc/acpi/ac_adapter/0/status", "r")))
    if (!(fp = fopen ("/proc/acpi/ac_adapter/ACAD/state", "r")))
      if (!(fp = fopen ("/proc/acpi/ac_adapter/AC/state", "r")))
        if (!(fp = fopen ("/proc/acpi/ac_adapter/ADP1/state", "r")))
	        return;

	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);

	if (strncmp(buf, "state:",  6) == 0) where = buf + 26;
	if (strncmp(buf, "Status:", 7) == 0) where = buf + 26;

	if (where)
	{
		if (where[0] == 'n') acadstate->state = 1;
		if (where[0] == 'f') acadstate->state = 0;
	}
}


void read_acpi_info (ACPIinfo *acpiinfo, int battery)
{
	FILE *fp;
	char *ptr = buf;
	int offset = 25;

  if (battery > MAXBATT) return;
  if (!(fp = fopen (battinfo[battery], "r"))) return;

	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);

	while (ptr)
	{
		static int count = 0;

		if (!strncmp(ptr+1, "resent:", 7))
		{
			if (ptr[offset] != 'y')
			{
				acpiinfo->present = 0;
				acpiinfo->design_capacity = 0;
				acpiinfo->last_full_capacity = 0;
				acpiinfo->battery_technology = 0;
				acpiinfo->design_voltage = 0;
				acpiinfo->design_capacity_warning = 0;
				acpiinfo->design_capacity_low = 0;
				return;
			}
			acpiinfo->present = 1;
			ptr = jump_next_line(ptr);
			if (!ptr) break;
		}
		if (!strncmp(ptr, "design capacity:", 16) || !strncmp(ptr, "Design Capacity:", 16))
    {
      sscanf (ptr+offset, "%d", &(acpiinfo->design_capacity));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "last full capacity:", 19) || !strncmp(ptr, "Last Full Capacity:", 19))
    {
      sscanf (ptr+offset, "%d", &(acpiinfo->last_full_capacity));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "battery technology:", 19) || !strncmp(ptr, "Battery Technology:", 19))
    {
      switch (ptr[offset])
	    {
	      case 'n':
	        acpiinfo->battery_technology = 1;
	        break;
	      case 'r':
	        acpiinfo->battery_technology = 0;
	        break;
	    }
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "design voltage:", 15) || !strncmp(ptr, "Design Voltage:", 15))
    {
      sscanf (ptr+offset, "%d", &(acpiinfo->design_voltage ));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
    }
		if (!strncmp(ptr, "design capacity warning:", 24) || !strncmp(ptr, "Design Capacity Warning:", 24))
		{
      sscanf (ptr+offset, "%d", &(acpiinfo->design_capacity_warning));
			ptr = jump_next_line(ptr);
			if (!ptr) break;
		}
		if (!strncmp(ptr, "design capacity low:", 20) || !strncmp(ptr, "Design Capacity Low:", 20))
		{
			sscanf (ptr+offset, "%d", &(acpiinfo->design_capacity_low));
			if (!count) return; /* we did read all stuff in just one passage! */
			ptr = jump_next_line(ptr);
			if (!ptr) break;
		}

		ptr = jump_next_line(ptr);
		count++;
	}
}


void read_acpi_state (ACPIstate *acpistate, ACPIinfo *acpiinfo, int battery)
{
	FILE *fp;
	char *ptr = buf;
	int offset = 25;

  if (battery > MAXBATT) return;
	if (!(fp = fopen (batteries[battery], "r"))) return;

	fread_unlocked (buf, 512, 1, fp);
	fclose(fp);

	while (ptr)
	{
		static int count = 0;

		if (!strncmp(ptr+1, "resent:", 7))
		{
			if (ptr[offset] != 'y')
			{
				acpistate->present = 0;
				acpistate->state = UNKNOW;
				acpistate->prate = 0;
				acpistate->rcapacity = 0;
				acpistate->pvoltage = 0;
				acpistate->rtime = 0;
				acpistate->percentage = 0;
				return;
			}
			acpistate->present = 1;
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "capacity state:", 15))
		{
			/* nothing to do here... */
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "charging state:", 15) || !strncmp(ptr, "State:", 6))
		{
			switch (ptr[offset])
			{
				case 'd':
					acpistate->state = 1;
					break;
				case 'c':
					if (*(ptr + 33) == '/') acpistate->state = 0;
					else acpistate->state = 2;
					break;
				case 'u':
					acpistate->state = 3;
					break;
			}
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "present rate:", 13) || !strncmp(ptr, "Present Rate:", 13))
		{
			sscanf (ptr+offset, "%d", &(acpistate->prate));
			/* if something wrong */
			if (acpistate->prate <= 0) acpistate->prate = 0;
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "remaining capacity:", 19) || !strncmp(ptr, "Remaining Capacity:", 19))
		{
			sscanf (ptr+offset, "%d", &(acpistate->rcapacity));
			acpistate->percentage =	(float) ((float) acpistate->rcapacity / (float) acpiinfo->last_full_capacity) * 100;
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}
		if (!strncmp(ptr, "present voltage:", 16) || !strncmp(ptr, "Battery Voltage:", 16))
		{
			sscanf (ptr+offset, "%d", &(acpistate->pvoltage));
			if (!count) break; /* we did read all stuff in just one passage! */
			ptr = jump_next_line(ptr);
			if (!ptr) return;
		}

		ptr = jump_next_line(ptr);
		count++;
	}

	/* time remaining in minutes */
	if (!acpistate->prate) return;
	acpistate->rtime = ((float) ((float) acpistate->rcapacity / (float) acpistate->prate)) * 60;
	if (acpistate->rtime <= 0) acpistate->rtime = 0;
}


void acpi_get_temperature(int *temperature, int *temp_is_celsius)
{
	FILE *fp;
	char *temp = NULL;
	char *unit = NULL;

	if (!(fp=fopen("/proc/acpi/thermal_zone/THRM/temperature", "r")))
		if (!(fp=fopen("/proc/acpi/thermal_zone/THR2/temperature", "r")))
			if (!(fp=fopen("/proc/acpi/thermal_zone/ATF0/temperature", "r")))
			{
				(*temperature)     = PM_Error;
				(*temp_is_celsius) = PM_Error;
				return;
			}
	scan(fp, "%s%s%s", NULL, &temp, &unit);
	fclose(fp);

	(*temperature) = atoi(temp);
	if (strcmp(unit, "C") == 0) (*temp_is_celsius) = 1;
	else (*temp_is_celsius) = 0;
}
