/***************************************************************************
                      power_management.c  -  description
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
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib_utils/lib_utils.h"
#include "power_management.h"
#include "libacpi.h"
#include "libapm.h"
#include "../toshiba/toshiba_lib.h"

int fast_charge_mode=0;
int toshiba_machine;
int pm_type=PM_Error;
int ac_on_line;
int battery_percentage;
int battery_present;
int noflushd=0;
int use_lin_seti=1;
int use_noflushd=1;
int use_toshiba=1;

/* For ACPI systems: for now we support only battery n.1 */
int Battery=1; 

int get_fan_status();
void get_temperature(int *temperature, int *temp_is_celsius);
int fast_battery_charge(int toggle);

int pm_support(void)
{
  if (!use_noflushd) fprintf(stderr, "use of noflushd is disabled\n");
  if (!use_lin_seti) fprintf(stderr, "use of lin_seti is disabled\n");

  /* Is this a Toshiba Laptop? */
  if (use_toshiba)
  {  
    if ((toshiba_machine=toshiba_support())) fprintf(stderr, "This is a Toshiba laptop ");
    else fprintf(stderr, "Unknown computer ");
  }  
    
  /* Is this an acpi system? */
  if (check_acpi() == 0)
  {
    pm_type= PM_ACPI;
    fprintf(stderr, "with an ACPI subsystem...\n");
    return 1;
  }  
  
  /* Is this an APM system?  */
  if (apm_exists())
	{
	  apm_data= (struct_apm_data *) calloc(1, sizeof(struct_apm_data));
    if (apm_read(apm_data))
		{
		  free(apm_data);
		  return 0;
		}	
		free(apm_data);
    pm_type= PM_APM;
    fprintf(stderr, "with an APM subsystem...\n");
    return 1; 
	}

  fprintf(stderr, "with no power management subsystem...\n"); 
	return 0;	
}

void get_power_status(pm_status *power_status)
{  

  /* Is this an ACPI system? */
	if (pm_type == PM_ACPI)
	{	
	  /* Allocate structures and read ACPI data */	
    acpistate = (ACPIstate *) calloc (1, sizeof (ACPIstate));
    acpiinfo = (ACPIinfo *) calloc (1, sizeof (ACPIinfo));
    acadstate = (ACADstate *) calloc (1, sizeof (ACADstate));
    read_acpi_info (Battery - 1);
    read_acpi_state (Battery - 1);
    read_acad_state ();

    /* Check if we are on ac power */
    ac_on_line = power_status->ac_on_line = acadstate->state;
	
    /* Check to see if we are charging. */
    if (acpistate->state == CHARGING) power_status->battery_charging=1;
    else power_status->battery_charging=0;
		
		/* Check battery time and percentage */
    power_status->battery_time = acpistate->rtime;  
    battery_percentage = power_status->battery_percentage = acpistate->percentage;
           
    /* Check if battery is plugged in */
		battery_present = power_status->battery_present = acpistate->present;

	  /* Get temperature and fan status */
		power_status->fan_status=get_fan_status();
	  get_temperature(&(power_status->temperature), &(power_status->temp_is_celsius));
		
    /* Release structures from memory */
    free (acpistate); free(acpiinfo); free(acadstate);
    if (fast_charge_mode && (power_status->battery_percentage == 100)) fast_battery_charge(0);

		return;
  }

  /* it seems not, so it must be an apm system */
  apm_data= (struct_apm_data *) calloc(1, sizeof(struct_apm_data));
  if (apm_read(apm_data))
  {
    fprintf(stderr, "Cannot read APM information:\n");
    free(apm_data);
    exit(1);
  }    
  ac_on_line = power_status->ac_on_line = apm_data->ac_line_status;
  if ( apm_data->battery_percentage == -1)
  {
    battery_present = power_status->battery_present = 0;
    battery_percentage = power_status->battery_percentage = 0;
  }
  else
  {
    battery_present = power_status->battery_present = 1;
    battery_percentage = power_status->battery_percentage = apm_data->battery_percentage;
  }
  if ( (int)(apm_data->battery_status) == 3)
    power_status->battery_charging=1;
  else power_status->battery_charging=0;
  power_status->battery_time = (apm_data->using_minutes) ? apm_data->battery_time : apm_data->battery_time / 60;
  power_status->fan_status=get_fan_status();  
  get_temperature(&(power_status->temperature), &(power_status->temp_is_celsius));
  free(apm_data);
  if (fast_charge_mode && (power_status->battery_percentage == 100)) fast_battery_charge(0);
  
}

int get_fan_status(void)
{
  FILE *fp;
	string proc_fan_status="/proc/acpi/toshiba/fan";
	string fan_status;
  int result;
  
	if (use_toshiba) if (toshiba_machine) return toshiba_get_fan_status();
  
  if (pm_type == PM_ACPI)
    /* Check for fan status in PROC filesystem */
	  if ( (fp=fopen(proc_fan_status, "r")) != NULL )
    {
      scan(fp, "%s%s", NULL, &fan_status);
      fclose(fp);	
      result = atoi(fan_status);
      free(fan_status);
      return result;
    }
  
	return PM_Error;
}

void get_temperature(int *temperature, int *temp_is_celsius)
{
  FILE *fp;
	string proc_temperature="/proc/acpi/thermal_zone/THRM/temperature";
	string temp, unit;
	
	if (pm_type != PM_ACPI)
  {
	  (*temperature)     = PM_Error;
		(*temp_is_celsius) = PM_Error;
		return;
  }
  
  if ( (fp=fopen(proc_temperature, "r")) == NULL)
	{
	  (*temperature)     = PM_Error;
		(*temp_is_celsius) = PM_Error;
		return;
	}
	scan(fp, "%s%s%s", NULL, &temp, &unit);
	fclose(fp);
	
  (*temperature)= atoi(temp);
	if (strcmp(unit, "C") == 0) (*temp_is_celsius)=1;
	else (*temp_is_celsius)=0;
}


void internal_set_pm_features(int ac_status)
{
  static int seti_status=0;
    
  if (fast_charge_mode) ac_status=0;
  
  if (!toshiba_machine && (pm_type == PM_ACPI))
  {
    /* Set ACPI specific features here... */    
    return;
  }
  
  /* Set APM specific features here...*/  
  if (ac_status)
  { 
    /* Stop noflushd damon (disable HD spindown) */
    if (use_noflushd) if (noflushd)
    {      
      system("/etc/init.d/noflushd stop >/dev/null 2>/dev/null");
      noflushd = 0;
    }  
    /* Start lin-seti */    
    if (use_lin_seti) if (seti_status == 0)
    {      
      system("/etc/init.d/lin-seti start >/dev/null 2>/dev/null");
      seti_status = 1;
    }    
    if (use_toshiba) if (toshiba_machine)
    {      
      /* Set LCD to maximum brightness */
      toshiba_set_lcd_brightness(LCD_MAX);
      /* Start fan */
      if (pm_type != PM_ACPI) toshiba_set_fan_status(1);
    }      
  }
  else
  { 
    if (use_noflushd) if (!noflushd)
    {
      /* Start noflushd damon (enable HD spindown) */
      system("/etc/init.d/noflushd start >/dev/null 2>/dev/null");
      noflushd = 1;
    }  
    /* Stop lin-seti */
    if (use_lin_seti) if (seti_status == 1)
    {
      system("/etc/init.d/lin-seti stop >/dev/null 2>/dev/null");
      seti_status = 0;
    }  
    if (use_toshiba) if (toshiba_machine)
    {    
      /* Set LCD to minimum brightness */
      toshiba_set_lcd_brightness(LCD_MIN);
      /* Stop fan */
      if (pm_type != PM_ACPI) toshiba_set_fan_status(0);      
    }  
  }
  
}


void set_pm_features(void)
{
  
  internal_set_pm_features(ac_on_line);
  
}


int fast_battery_charge(int toggle)
{ 
  
  if (toggle && !battery_present)
  {
    fprintf(stderr, "Fast charge mode without a battery? I think not ;-)\n");
    return 0;
  }  
  if (toggle && !ac_on_line)
  {
    fprintf(stderr, "Fast charge mode while on battery? I think not ;-)\n");
    return 0;      
  }  
  if (toggle && (battery_percentage == 100))
  {
    fprintf(stderr, "Battery is already at maximum capacity: fast charging not enabled\n");
    return 0;      
  }  

  if (!toggle && fast_charge_mode)
  {
    if (battery_percentage == 100) fprintf(stderr, "battery at maximum capacity: ");
    fprintf(stderr, "disabling fast charge mode\n");
    fast_charge_mode=0;
    internal_set_pm_features(ac_on_line);    
    return 0;
  }  

  if (toggle)
  {
    fprintf(stderr, "enabling fast charge mode\n");    
    internal_set_pm_features(0);
    fast_charge_mode=1;
    return 1;
  }
   
  return fast_charge_mode;
}


void set_noflushd_use(int toggle)
{
  use_noflushd = toggle;
}


void set_lin_seti_use(int toggle)
{
  use_lin_seti = toggle;
}


void set_toshiba_use(int toggle)
{
  use_toshiba = toggle;
}
