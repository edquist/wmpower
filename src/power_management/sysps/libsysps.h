/***************************************************************************
                          libsysps.h  -  description
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


typedef struct
{
    int present;             /* 1 if present, 0 if no battery   */
    int power_now;           /* 1000 * present rate / mW        */
    int energy_full;         /* 1000 * last full capacity / mWh */
    int energy_now;          /* 1000 * remaining capacity / mWh */
    int capacity;            /* battery percentage              */

    int ac_on_line;
    int charging;
    int time_remaining;      /* time remaining in minutes       */
} SYSPS_info;

void sysps_read_bat_info(int bat, SYSPS_info *info);
int check_sysps(void);

