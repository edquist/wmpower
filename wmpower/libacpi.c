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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include "libacpi.h"

static char batteries[MAXBATT][128];
static char battinfo[MAXBATT][128];

/* see if we have ACPI support */
int check_acpi(void)
{
  FILE *acpi;
  DIR *battdir;
  struct dirent *batt;
  char *name;

  if (!(acpi = fopen ("/proc/acpi/info", "r")))
  {
    return 1;
  }

  /* yep, all good */
  fclose (acpi);

  /* now enumerate batteries */
  batt_count = 0;
  battdir = opendir ("/proc/acpi/battery");
  if (battdir == 0)
  {
    return 2;
  }
  while ((batt = readdir (battdir)))
  {
    name = batt->d_name;

    /* skip . and .. */
    if (!strncmp (".", name, 1) || !strncmp ("..", name, 2)) continue;
    if (!(acpi = fopen ("/proc/acpi/battery/1/status", "r")))
	    sprintf (batteries[batt_count], "/proc/acpi/battery/%s/state", name);
    else
	    sprintf (batteries[batt_count], "/proc/acpi/battery/%s/status", name);
    sprintf (battinfo[batt_count], "/proc/acpi/battery/%s/info", name);
    batt_count++;
  }
  closedir (battdir);

  return 0;
}

int read_acad_state(void)
{
  FILE *acpi;
  char *ptr;
  char stat;

  if (!(acpi = fopen ("/proc/acpi/ac_adapter/0/status", "r")))
    if (!(acpi = fopen ("/proc/acpi/ac_adapter/ACAD/state", "r")))
      if (!(acpi = fopen ("/proc/acpi/ac_adapter/AC/state", "r")))
        if (!(acpi = fopen ("/proc/acpi/ac_adapter/ADP1/state", "r")))
	        return -1;

  fread (buf, 512, 1, acpi);
  fclose (acpi);

  if ( (ptr = strstr(buf, "state:")) )
  {
    stat = *(ptr + 26);
    if (stat == 'n') acadstate->state = 1;
    if (stat == 'f')
  	{
	    acadstate->state = 0;
	    return 0;
  	}
  }

  if ( (ptr = strstr (buf, "Status:")) )
  {
    stat = *(ptr + 26);
    if (stat == 'n') acadstate->state = 1;
    if (stat == 'f')
  	{
	    acadstate->state = 0;
	    return 0;
	  }
  }

  return 1;
}

int read_acpi_info(int battery)
{
  FILE *acpi;
  char *ptr;
  char stat;
  int temp;

  if (battery > MAXBATT) return -1;
  if (!(acpi = fopen (battinfo[battery], "r"))) return -1;

  fread (buf, 512, 1, acpi);
  fclose (acpi);

  if ((ptr = strstr (buf, "present:")) || (ptr = strstr (buf, "Present:")))
  {
    stat = *(ptr + 25);
    if (stat == 'y')
  	{
	    acpiinfo->present = 1;
  	  if ((ptr = strstr (buf, "design capacity:")) || (ptr = strstr (buf, "Design Capacity:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &temp);
	      acpiinfo->design_capacity = temp;
	    }
	    if ((ptr = strstr (buf, "last full capacity:")) || (ptr = strstr (buf, "Last Full Capacity:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &temp);
	      acpiinfo->last_full_capacity = temp;
	    }
	    if ((ptr = strstr (buf, "battery technology:")) || (ptr = strstr (buf, "Battery Technology:")))
	    {
	      stat = *(ptr + 25);
	      switch (stat)
		    {
		      case 'n':
		        acpiinfo->battery_technology = 1;
		        break;
		      case 'r':
		        acpiinfo->battery_technology = 0;
		        break;
		    }
	    }
	    if ((ptr = strstr (buf, "design voltage:")) || (ptr = strstr (buf, "Design Voltage:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &temp);
	      acpiinfo->design_voltage = temp;
	    }
	    if ((ptr = strstr (buf, "design capacity warning:")) || (ptr = strstr (buf, "Design Capacity Warning:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &temp);
	      acpiinfo->design_capacity_warning = temp;
	    }
  	  if ((ptr = strstr (buf, "design capacity low:")) || (ptr = strstr (buf, "Design Capacity Low:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &temp);
	      acpiinfo->design_capacity_low = temp;
	    }
	  }
    else /* Battery not present */
	  {
	    acpiinfo->present = 0;
	    acpiinfo->design_capacity = 0;
	    acpiinfo->last_full_capacity = 0;
	    acpiinfo->battery_technology = 0;
	    acpiinfo->design_voltage = 0;
	    acpiinfo->design_capacity_warning = 0;
	    acpiinfo->design_capacity_low = 0;
	    return 0;
	  }
  }
	
  return 1;
}

int read_acpi_state(int battery)
{
  FILE *acpi;
  char *ptr;
  char stat;

  int percent = 100;		/* battery percentage */
  int ptemp, rate, rtime = 0;

  if (!(acpi = fopen (batteries[battery], "r"))) return -1;

  fread (buf, 512, 1, acpi);
  fclose (acpi);

  if ((ptr = strstr (buf, "present:")) || (ptr = strstr (buf, "Present:")))
  {
    stat = *(ptr + 25);
    if (stat == 'y')
  	{
	    acpistate->present = 1;
	    if ((ptr = strstr (buf, "charging state:")) || (ptr = strstr (buf, "State:")))
	    {
	      stat = *(ptr + 25);
	      switch (stat)
		    {
		      case 'd':
		        acpistate->state = 1;
		        break;
		      case 'c':
		        if (*(ptr + 33) == '/')
		          acpistate->state = 0;
		        else
		          acpistate->state = 2;
		        break;
		      case 'u':
		        acpistate->state = 3;
		        break;
		    }
	    }
	    /* This section of the code will calculate "percentage remaining"
	     * using battery capacity, and the following formula 
	     * (acpi spec 3.9.2):
	     * 
	     * percentage = (current_capacity / last_full_capacity) * 100;
	     *
	     */
	    if ((ptr = strstr (buf, "remaining capacity:")) || (ptr = strstr (buf, "Remaining Capacity:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &ptemp);
	      acpistate->rcapacity = ptemp;
	      percent =	(float) ((float) ptemp / (float) acpiinfo->last_full_capacity) * 100;
	      acpistate->percentage = percent;
	    }
	    if ((ptr = strstr (buf, "present rate:")) || (ptr = strstr (buf, "Present Rate:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &rate);

	      /* if something wrong */
	      if (rate <= 0) rate = 0;

	      acpistate->prate = rate;
				
	      /* time remaining in minutes */
	      rtime = ((float) ((float) acpistate->rcapacity /
				(float) acpistate->prate)) * 60;
	      if (rtime <= 0) rtime = 0;
	      
				acpistate->rtime = rtime;
	    }
	    if ((ptr = strstr (buf, "present voltage:")) || (ptr = strstr (buf, "Battery Voltage:")))
	    {
	      ptr += 25;
	      sscanf (ptr, "%d", &ptemp);
	      acpistate->pvoltage = ptemp;
	    }
	  }
    else /* Battery not present */
	  {
	    acpistate->present = 0;
	    acpistate->state = UNKNOW;
	    acpistate->prate = 0;
	    acpistate->rcapacity = 0;
	    acpistate->pvoltage = 0;
	    acpistate->rtime = 0;
	    acpistate->percentage = 0;
	    return 0;
	  }
  }
	
  return 1;
}
