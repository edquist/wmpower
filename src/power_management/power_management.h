/***************************************************************************
                      power_management.h  -  description
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

#define PM_Error -1
#define PM_ACPI 2
#define PM_APM  3

typedef struct
{
	int battery_percentage;
	int battery_charging;
	int battery_time;
	int battery_present;
	int ac_on_line;
	int fan_status;
	int temperature;
	int temp_is_celsius;

} pm_status;

int pm_support(int which_battery);
void get_power_status(pm_status *power_status);
void set_pm_features(void);
int fast_battery_charge(int toggle);
void set_noflushd_use(int toggle);
void set_lin_seti_use(int toggle);
void set_toshiba_hardware_use(int toggle);
void lcdBrightness_UpOneStep();
void lcdBrightness_DownOneStep();
