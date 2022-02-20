/***************************************************************************
                          libsysps.c  -  description
                             -------------------
    begin                : Feb 18 2022
    copyright            : (C) 2022 by Carl Edquist
    e-mail               : edquist@cs.wisc.edu
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


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>

#include "libsysps.h"


static const char sysps_base_dir[]       = "/sys/class/power_supply";
static const char sysps_ac_online_path[] = "/sys/class/power_supply/AC/online";

static const char *sysps_batpath(int bat, const char *fn)
{
    static char buf[64];

    sprintf(buf, "%s/BAT%d/%s", sysps_base_dir, bat, fn);

    return buf;
}

static int read_file_int(const char *path)
{
    int ret = 0;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open '%s'\n", path);
        return 0;
    }
    if (fscanf(fp, "%d", &ret) != 1) {
        fprintf(stderr, "Failed to read '%s'\n", path);
    }
    fclose(fp);
    return ret;
}

static char read_file_char(const char *path)
{
    int ret = 0;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open '%s'\n", path);
        return 0;
    }
    if ((ret = fgetc(fp)) == EOF) {
        fprintf(stderr, "Failed to read '%s'\n", path);
    }
    fclose(fp);
    return (char)ret;
}

static int sysps_get_bat_stat(int bat, const char *stat)
{
    return read_file_int(sysps_batpath(bat, stat));
}

static int sysps_get_bat_charging(int bat)
{
    /* Valid values are "Charging", "Discharging", "Not charging", *
     *    "Full" and "Unknown", but we only care about "Charging"  */
    return read_file_char(sysps_batpath(bat, "status")) == 'C';
}

static int sysps_get_ac_on_line(void)
{
    return read_file_int(sysps_ac_online_path);
}

static int sysps_get_time_remaining(const SYSPS_info *info)
{
    int ret;  /* time remaining in minutes */

    if (info->power_now == 0)
        ret = 0;
    else if (info->charging)
        ret = 60.0 * (info->energy_full - info->energy_now) / info->power_now;
    else
        ret = 60.0 * info->energy_now / info->power_now;

    if (ret <= 0)
        ret = 0;

    return ret;
}

void sysps_read_bat_info(int bat, SYSPS_info *info)
{
    info->present        = sysps_get_bat_stat(bat, "present");
    info->power_now      = sysps_get_bat_stat(bat, "power_now");
    info->energy_full    = sysps_get_bat_stat(bat, "energy_full");
    info->energy_now     = sysps_get_bat_stat(bat, "energy_now");
    info->capacity       = sysps_get_bat_stat(bat, "capacity");

    info->charging       = sysps_get_bat_charging(bat);
    info->ac_on_line     = sysps_get_ac_on_line();
    info->time_remaining = sysps_get_time_remaining(info);
}

/* check for /sys power_supply support */
int check_sysps(void)
{
    int sysps_bats = 0;
    int sysps_ac = access(sysps_ac_online_path, R_OK) == 0;
    while (access(sysps_batpath(sysps_bats, "present"), R_OK) == 0)
        sysps_bats++;

    if (sysps_ac + sysps_bats == 0)
        return 0;

    fprintf(stderr, "libsysps: found %d batter%s\n", sysps_bats,
					(sysps_bats == 1) ? "y" : "ies");

    return 1;
}


/* TODO:
 *
 * TEMP: /sys/class/thermal/thermal_zone0/temp -> 1000 * cpu temp in C
 * FAN:  /sys/devices/platform/thinkpad_hwmon/fan1_input -> fan speed in RPM
 *
 */

