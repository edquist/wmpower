/***************************************************************************
                        toshiba_lib.c  -  description
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
    Many thanks to Jonathan A. Buzzard for his Toshiba(tm) Linux Utilities
                   I could never have done this otherwise
 ***************************************************************************/

#define FAN_OFF 1
#define FAN_ON 0

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>

#ifdef __GLIBC__
#include <sys/perm.h>
#endif

#include "hci.h"
#include "sci.h"


void UglyHack(void);
int Get_Fan_Status(void);


int toshiba_get_fan_status(void)
{

	if (Get_Fan_Status()<0x00) 
  {
		fprintf(stderr, "toshiba_lib: laptop does not have cooling fan or kernel module not installed.\n");
		return -1;
	}

	if (Get_Fan_Status()==0x00) return 0;
	else return 1;

}


void toshiba_set_fan_status(int status)
{
	SMMRegisters reg;
  static int enable_spinoff=1;
  static time_t timer;
  
  if (status == 1)
  {
    if (toshiba_get_fan_status() == 1) return;    
  	reg.eax = HCI_SET;
	  reg.ebx = HCI_FAN;
	  reg.ecx = HCI_ENABLE;
	  HciFunction(&reg);
    usleep(100000);
    if( toshiba_get_fan_status() == 1)
      fprintf(stderr, "toshiba_lib: cooling fan turned on\n");    
    else       
      fprintf(stderr, "toshiba_lib: unable to turn on cooling fan\n");
	  return;
  }
    
  if (status == 0)
  {
    if (toshiba_get_fan_status() == 0) return;    
    if (!enable_spinoff)
    {
      if ( (time(NULL)-timer) >= 15 ) enable_spinoff=1;
      else return;
    }
    enable_spinoff=0;
    timer = time(NULL);
    UglyHack();    
    if( toshiba_get_fan_status() == 0)
      fprintf(stderr, "toshiba_lib: cooling fan turned off\n");    
    else       
      fprintf(stderr, "toshiba_lib: unable to turn off cooling fan\n");
	  return;
  }

  fprintf(stderr, "toshiba_lib: selected invalid fan status\n");    
  
}


void toshiba_set_lcd_brightness(int brightness)
{
  static int current_brightness=-1;
  unsigned short lcd;
  static unsigned short lcdtype=999;
	SMMRegisters reg;

  lcd = brightness;
  
  if (lcdtype==999)
  {
    SciOpenInterface();
    reg.ebx = SCI_LCD_BRIGHTNESS;
	  if (SciGet(&reg)==SCI_SUCCESS) lcdtype = SCI_LCD_BRIGHTNESS;
	  else lcdtype = SCI_LCD_BACKLIGHT;
	  reg.ebx = SCI_LCD_MAXBRIGHT;
	  if (SciGet(&reg)==SCI_SUCCESS) 
    {
		  lcdtype = SCI_LCD_MAXBRIGHT;
		  if (lcd>1) lcd = 1;
	  }
    fprintf(stderr, "toshiba_lib: lcdtype set to %d\n", lcdtype);
    SciCloseInterface();
  }  
  
  if (lcd != current_brightness)
  {
    SciOpenInterface();
  	reg.ebx = lcdtype;
		reg.ecx = lcd;
		if (SciSet(&reg)==SCI_FAILURE)
			fprintf(stderr, "toshiba_lib: unable to set LCD brightness\n");
    else 
    {
      fprintf(stderr, "toshiba_lib: changed LCD brightness to %d\n", lcd);
      current_brightness=lcd;
    }  
    SciCloseInterface();
  }
  
}

/* Check if we are running on a Toshiba laptop */
int toshiba_support(void)
{
	int version,bios,id;
	  
  /* do some quick checks on the laptop */
	if (SciSupportCheck(&version)==1) 
  {
		fprintf(stderr, "toshiba_lib: this computer is not supported or the kernel module not installed.\n");
		return 0;
	}
	bios = HciGetBiosVersion();
	if (bios==0)
  {
		fprintf(stderr, "toshiba_lib: unable to get BIOS version\n");
		return 0;
	}
	if (HciGetMachineID(&id)==HCI_FAILURE)
  {
		fprintf(stderr, "toshiba_lib: unable to get machine identification\n");
		return 0;
	}

	switch (id)
  {
		case 0xfc00: /* Satellite 2140CDS/2180CDT/2675DVD */
		case 0xfc01: /* Satellite 2710xDVD */
		case 0xfc02: /* Satellite Pro 4270CDT//4280CDT/4300CDT/4340CDT */
		case 0xfc04: /* Portege 3410CT, 3440CT */
		case 0xfc08: /* Satellite 2100CDS/CDT 1550CDS */
		case 0xfc09: /* Satellite 2610CDT, 2650XDVD */
		case 0xfc0a: /* Portage 7140 */
		case 0xfc0b: /* Satellite Pro 4200 */
		case 0xfc0c: /* Tecra 8100x */
		case 0xfc0f: /* Satellite 2060CDS/CDT */
		case 0xfc10: /* Satellite 2550/2590 */
		case 0xfc11: /* Portage 3110CT */
		case 0xfc12: /* Portage 3300CT */
		case 0xfc13: /* Portage 7020CT */
		case 0xfc15: /* Satellite 4030/4030X/4050/4060/4070/4080/4090/4100X CDS/CDT */
		case 0xfc17: /* Satellite 2520/2540 CDS/CDT */
		case 0xfc18: /* Satellite 4000/4010 XCDT */
		case 0xfc19: /* Satellite 4000/4010/4020 CDS/CDT */
		case 0xfc1a: /* Tecra 8000x */
		case 0xfc1c: /* Satellite 2510CDS/CDT */
		case 0xfc1d: /* Portage 3020x */
		case 0xfc1f: /* Portage 7000CT/7010CT */
		case 0xfc39: /* T2200SX */
		case 0xfc40: /* T4500C */
		case 0xfc41: /* T4500 */
		case 0xfc45: /* T4400SX/SXC */
		case 0xfc51: /* Satellite 2210CDT, 2770XDVD */
		case 0xfc52: /* Satellite 2775DVD, Dynabook Satellite DB60P/4DA */
		case 0xfc53: /* Portage 7200CT/7220CT, Satellite 4000CDT */
		case 0xfc54: /* Satellite 2800DVD */
		case 0xfc56: /* Portage 3480CT */
		case 0xfc57: /* Satellite 2250CDT*/
		case 0xfc5a: /* Satellite Pro 4600 */
		case 0xfc5d: /* Satellite 2805 */
		case 0xfc5f: /* T3300SL */
		case 0xfc61: /* Tecra 8200 */
		case 0xfc64: /* Satellite 1800 */
		case 0xfc69: /* T1900C */
		case 0xfc6a: /* T1900 */
		case 0xfc6d: /* T1850C */
		case 0xfc6e: /* T1850 */
		case 0xfc6f: /* T1800 */
		case 0xfc72: /* Satellite 1800 */
		case 0xfc7e: /* T4600C */
		case 0xfc7f: /* T4600 */
		case 0xfc8a: /* T6600C */
		case 0xfc91: /* T2400CT */
		case 0xfc97: /* T4800CT */
		case 0xfc99: /* T4700CS */
		case 0xfc9b: /* T4700CT */
		case 0xfc9d: /* T1950 */
		case 0xfc9e: /* T3400/T3400CT */
		case 0xfcb2: /* Libretto 30CT */
		case 0xfcba: /* T2150 */
		case 0xfcbe: /* T4850CT */
		case 0xfcc0: /* Satellite Pro 420x */
		case 0xfcc1: /* Satellite 100x */
		case 0xfcc3: /* Tecra 710x/720x */
		case 0xfcc6: /* Satellite Pro 410x */
		case 0xfcca: /* Satellite Pro 400x */
		case 0xfccb: /* Portage 610CT */
		case 0xfccc: /* Tecra 700x */
		case 0xfccf: /* T4900CT */
		case 0xfcd0: /* Satellite 300x */
		case 0xfcd1: /* Tecra 750CDT */
		case 0xfcd2: /* Vision Connect -- what is this??? */
		case 0xfcd3: /* Tecra 730XCDT*/
		case 0xfcd4: /* Tecra 510x */
		case 0xfcd5: /* Satellite 200x */
		case 0xfcd7: /* Satellite Pro 430x */
		case 0xfcd8: /* Tecra 740x */
		case 0xfcd9: /* Portage 660CDT */
		case 0xfcda: /* Tecra 730CDT */
		case 0xfcdb: /* Portage 620CT */
		case 0xfcdc: /* Portage 650CT */
		case 0xfcdd: /* Satellite 110x */
		case 0xfcdf: /* Tecra 500x */
		case 0xfce0: /* Tecra 780DVD */
		case 0xfce2: /* Satellite 300x */
		case 0xfce3: /* Satellite 310x */
		case 0xfce4: /* Satellite Pro 490x */
		case 0xfce5: /* Libretto 100CT */
		case 0xfce6: /* Libretto 70CT */
		case 0xfce7: /* Tecra 540x/550x */
		case 0xfce8: /* Satellite Pro 470x/480x */
		case 0xfce9: /* Tecra 750DVD */
		case 0xfcea: /* Libretto 60 */
		case 0xfceb: /* Libretto 50CT */
		case 0xfcec: /* Satellite 320x/330x, Satellite 2500CDS */
		case 0xfced: /* Tecra 520x/530x */
		case 0xfcef: /* Satellite 220x, Satellite Pro 440x/460x */
			printf("toshiba_lib: machine id: 0x%04x    BIOS version: %d.%d    SCI version: %d.%d\n", 
        id, (bios & 0xff00)>>8, bios & 0xff, 
        (version & 0xff00)>>8, version & 0xff);
			break;
		default:
			printf("toshiba_lib: unrecognized machine identification:\n");
			printf("             machine id : 0x%04x    BIOS version : %d.%d    SCI version: %d.%d\n", 
        id, (bios & 0xff00)>>8, bios & 0xff,
				(version & 0xff00)>>8, version & 0xff);
	}
  
  return 1;
}
  
  




/* INTERNAL FUNCTIONS */


int Get_Fan_Status(void)
{
	SMMRegisters reg;

	reg.eax = HCI_GET;
	reg.ebx = HCI_FAN;
	HciFunction(&reg);

	if ((reg.eax & 0xff00)!=HCI_SUCCESS)
		return -1;
	else
		return (int) (reg.ecx & 0xff);
}

void UglyHack(void)
{
	unsigned short save=0,display=512,sleep=0,speed=1,cooling=1;	
	SMMRegisters reg;
	int user;
  
	SciOpenInterface();
	reg.ebx = SCI_BATTERY_SAVE;
	reg.ecx = save;
	if (SciSet(&reg)==SCI_SUCCESS) user = 0;
  else user = 1;
	if ((save==SCI_USER_SETTINGS) || (user==1)) 
  {
		reg.ebx = SCI_DISPLAY_AUTO;
		reg.ecx = display;
		if (SciSet(&reg)==SCI_FAILURE) {}
		reg.ebx = SCI_SLEEP_MODE;
		reg.ecx = sleep;
		if (SciSet(&reg)==SCI_FAILURE) {}    
		reg.ebx = SCI_PROCESSING;
		reg.ecx = speed;
		if (SciSet(&reg)==SCI_FAILURE) {}
		reg.ebx = SCI_COOLING_METHOD;
		reg.ecx = cooling;
		if (SciSet(&reg)==SCI_FAILURE) {}
		if (user==0) 
    {
			reg.ebx = SCI_BATTERY_SAVE;
			reg.ecx = 1;
			if (SciSet(&reg)==SCI_FAILURE) {}
			reg.ebx = SCI_BATTERY_SAVE;
			reg.ecx = save;
			if (SciSet(&reg)==SCI_FAILURE) {}
		}
	}
	SciCloseInterface();
  reg.eax = HCI_SET;
  reg.ebx = HCI_FAN;
  reg.ecx = HCI_DISABLE;
  HciFunction(&reg);
  usleep(100000);
  SciOpenInterface();
  reg.ebx = SCI_COOLING_METHOD;
	reg.ecx = FAN_OFF;
  SciCloseInterface();
  usleep(100000);

	return;
}
