/***************************************************************************
                          lib_utils.h  -  description
                             -------------------
    begin                : Sun Jan 20 15:34:25 CET 2002
    copyright            : (C) 2002 by Noberasco Michele
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

/* simple date data type */
typedef struct
  {
    int day;
    int month;
    int year;
  } date;

/* string type */
typedef char *string;

/* my rewrite of scanf and fscanf...
   it allows use of unallocated strings */
int scan(FILE *, char *, ...);

/* Finds user Home Dir */
char *get_home_dir(void);

/* reallocate a string using the minimum amount of memory */
void reallocate(char *stringa);

/* Converts a string to an integer */
/* int str_to_int(char *stringa);  */

/* Converts an integer to a string */
char *int_to_str(int intero);

/* Gets the first right <lung> chars of <stringa> */
char *right(char *stringa, int lung);

/* date_confrontation > 0 --> a>b
   date_confrontation = 0 --> a=b
   date_confrontation < 0 --> a<b     */
int date_confrontation(date a, date b);

/* Copy text file <*orig> to <*dest>
   <*dest>, if existing, is overwritten  */
void file_copy(char *orig, char *dest);

/* The following are to allow usage
   of run_time loaded messages within programs    */
int load_language_file(char *, char *);
char *messages(int);
