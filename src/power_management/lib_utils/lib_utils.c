/***************************************************************************
                          lib_utils.c  -  description
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

#define MAX 255

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <envz.h>
#include "lib_utils.h"


char *jump_next_line(string ptr)
{
	char *temp;

	if (!ptr) return NULL;

	for (temp=ptr; temp[0]!='\0'; temp++)
		if (temp[0] == '\n') return (temp+1);

	return NULL;
}

string get_home_dir(string envp[])
{
  int i, e_len = 0;
  char *str;

  for (i=0; envp[i] != NULL; i++) e_len += strlen(envp[i]) + 1;
  str = envz_get(*envp, e_len, "HOME");

	return str;
}

void reallocate(string temp)
{
  int i;

  if (temp != NULL)
  {
    i=strlen(temp);
    temp= (string) realloc( (void *) temp, (sizeof(char)*(i+1)) );
    if ( (temp == NULL) && (i > 0) )
    {
      perror("\nreallocate: Error reallocating memory...\n");
      abort();
    }
    temp[i]= '\0';
  }
}

int log10(int n)
{
  int temp=0;

  while ( (n/=10) >= 1) temp++;

  return temp;
}

string int_to_str(int n)
{
  string temp;
  int lun;

  if (n<0) return NULL;
  lun= log10(n);
  temp= (string) calloc(lun+2, sizeof(char));
  temp[lun+1]= '\0';
  while (lun>=0)
  {
    temp[lun]= '0'+ n%10;
    n/=10; lun--;
  }

  return temp;
}

string right(string ptr, int n)
{
  int len;
	string result;

  if ( (ptr == NULL) || (n <= 0) ) return NULL;
  len= strlen(ptr);
  if (len == 0) return NULL;
  if (n >= len)
	{
	  result= (string) calloc(len+1, sizeof(char));
		strcpy(result, ptr);
  	return result;
	}
  result= (string) calloc(n+1, sizeof(char));
	strcpy(result, ptr+len-n);

	return (result);
}

int date_confrontation(date a, date b)
{
  if (a.year < b.year) return -1;
  if (a.year > b.year) return 1;
  if (a.month < b.month) return -1;
  if (a.month > b.month) return 1;
  if (a.day < b.day) return -1;
  if (a.day > b.day) return 1;
  return 0;
}

void file_copy(string orig, string dest)
{
  FILE *in, *out;
  int temp;

  if ( (in=fopen(orig, "r")) != NULL )
  {
    if ( (out=fopen(dest, "w")) != NULL)
    {
      while ( (temp=fgetc(in)) != EOF) fputc(temp, out);
      fclose(out);
    }
    fclose(in);
  }
}

string search_file(string dir, string pattern)
{
  string result=NULL;
  struct dirent *test;
	DIR *pippo;

	if ( (pippo= opendir(dir)) != NULL)
	{
	  test = (struct dirent *) calloc(1, sizeof(struct dirent));
    while ( (test= readdir(pippo)) != NULL )
	  {
	    if (strstr(test->d_name, pattern) != NULL)
			{
        result= (string) calloc(strlen(test->d_name)+1, sizeof(char));
				strcpy(result, test->d_name);
				closedir(pippo);
				free(test); test=NULL;
				return result;
			}
	  }
	  closedir(pippo);
	  free(test); test=NULL;
  }

	return NULL;
}

date *get_file_date(string file)
{
	struct stat temp;
	struct tm file_date;
	date *output;

	if (file == NULL) return NULL;
	if (stat(file, &temp) == -1) return NULL;
	if (localtime_r(&(temp.st_mtime), &file_date) == NULL) return NULL;
	if ( (output= (date *) calloc(1, sizeof(date))) == NULL) return NULL;
	output->day=file_date.tm_mday;
  output->month=file_date.tm_mon+1;
  output->year=file_date.tm_year+1900;

	return output;
}

void erase_dir(string dir)
{
  struct dirent *test;
	DIR *pippo;
	string filename;

  if (dir == NULL) return;

	pippo= opendir(dir);
	if (pippo == NULL)
	{
	  fprintf(stderr, "erase_dir: unable to open directory \"%s\"\n", dir);
		exit(-1);
	}

	test = (struct dirent *) calloc(1, sizeof(struct dirent));
	filename= (string) calloc(strlen(dir)+NAME_MAX+1, sizeof(char));
  if ( (test == NULL) || (filename == NULL) )
	{
	  fprintf(stderr, "erase_dir: unable to allocate memory\n");
		exit(-1);
	}

	while ( (test= readdir(pippo)) != NULL )
	{
	  if ( (strcmp(test->d_name, ".") != 0) && (strcmp(test->d_name, "..") != 0) )
		{
      strcat(filename, dir);
		  if (strcmp(right(filename, 1), separator) != 0) strcat(filename, separator);
		  strcat(filename, test->d_name);
      if (remove(filename) == -1)
		  {
		    fprintf(stderr, "erase_dir: unable to remove file \"%s\"\n", filename);
		  	exit(-1);
		  }
			strcpy(filename, "\0");
		}
	}
	closedir(pippo);
	free(test); free(filename);

	if (rmdir(dir) == -1)
	{
		fprintf(stderr, "erase_dir: unable to remove dir \"%s\"\n", dir);
		exit(-1);
	}
}

int scan_string(FILE *, char **);

int scan(FILE *fp, char *format, ...)
{
  va_list parameters;
  int result=0, temp_result;

  va_start(parameters, format);
  while (*format)
  {
    while (*format == ' ') *format++;      /*   removes blank spaces   */
    if ( (*format++) != '%' )              /*   checks for the %       */
    {
      va_end(parameters);
      return result;
    }
    switch(*format++)
    {
      case 's':           /* string */
      {        
        temp_result = scan_string(fp, va_arg(parameters, char **));
        if (temp_result == EOF) return EOF;
        else result+= temp_result;
        break;
      }
      case 'd':           /* integer */
      {
        temp_result = fscanf(fp, "%d", va_arg(parameters, int *) );
        if (temp_result == EOF) return EOF;
        else result+= temp_result;
        break;
      }
      case 'u':           /* unsigned integer */
      {
        temp_result = fscanf(fp, "%u", va_arg(parameters, unsigned int *) );
        if (temp_result == EOF) return EOF;
        else result+= temp_result;
        break;
      }
      case 'f':           /* floating point */
      {
        temp_result = fscanf(fp, "%f", va_arg(parameters, float *) );
        if (temp_result == EOF) return EOF;
        else result+= temp_result;
        break;
      }
      case 'c':           /* char */
      {
        temp_result = fscanf(fp, "%c", ((char *) va_arg(parameters, int)) );
        if (temp_result == EOF) return EOF;
        else result+= temp_result;
        break;
      }
      case 'h':           /* short ... */
      {
        switch(*format++)
        {
          case 'd':           /* short integer */
          {
            temp_result = fscanf(fp, "%hd", va_arg(parameters, short int *) );
            if (temp_result == EOF) return EOF;
            else result+= temp_result;
            break;
          }
          case 'u':           /* short unsigned integer */
          {
            temp_result = fscanf(fp, "%hu", va_arg(parameters, short unsigned int *) );
            if (temp_result == EOF) return EOF;
            else result+= temp_result;
            break;
          }
          default:
          {
            va_end(parameters);
            return result;
          }
        }
        break;
      }
      case 'l':           /* long ... */
      {
        switch(*format++)
        {
          case 'd':           /* long integer */
          {
            temp_result = fscanf(fp, "%ld", va_arg(parameters, long int *) );
            if (temp_result == EOF) return EOF;
            else result+= temp_result;
            break;
          }
          case 'u':           /* long unsigned integer */
          {
            temp_result = fscanf(fp, "%lu", va_arg(parameters, long unsigned int *) );
            if (temp_result == EOF) return EOF;
            else result+= temp_result;
            break;
          }
          case 'f':           /* long floating point */
          {
            temp_result = fscanf(fp, "%lf", va_arg(parameters, double *) );
            if (temp_result == EOF) return EOF;
            else result+= temp_result;
            break;
          }
          default:
          {
            va_end(parameters);
            return result;
          }
        }
        break;
      }
      case 'L':           /* really long ... */
      {
        switch(*format++)
        {
          case 'f':           /* long long floating point */
          {
            temp_result = fscanf(fp, "%Lf", va_arg(parameters, long double *) );
            if (temp_result == EOF) return EOF;
            else result+= temp_result;
            break;
          }
          default:
          {
            va_end(parameters);
            return result;
          }
        }
        break;
      }
      default:
      {
        va_end(parameters);
        return result;
      }
    }
  }
  va_end(parameters);

  return result;
}

int scan_string(FILE *fp, char **string)
{
  struct stack
  {
    char el;
    struct stack *next;
  };

  struct stack *cod, *head;
  int lungh=0, start=1;
  char *temp;

  if ( (cod= (struct stack *) calloc(1, sizeof(struct stack))) == NULL) return 0;
  head= cod;

  while ( (cod->el=fgetc(fp))!= EOF)
  {
    while (start)
    {
      if ( (cod->el == ' ') || (cod->el == '\n') ) cod->el=fgetc(fp);
      else start=0;
      if (cod->el == EOF) return EOF;
    }
    if ( (cod->el == '\n') || (cod->el == ' ')) break;
    if (string != NULL)
    {
      if ( (cod->next= (struct stack *) calloc(1, sizeof(struct stack))) == NULL) return 0;
      cod= cod->next;
      lungh++;
    }
  }
  if ( (start) && (cod->el == EOF) ) return EOF;

  if (string == NULL) free(head);
  else
  {
    if ( (temp= calloc(lungh+1, sizeof(char))) == NULL) return 0;

    lungh=0;
    cod= head;
    while ( (cod->el != '\n') && (cod->el != ' ') )
    {
      temp[lungh]= cod->el;
      lungh++;
      head=cod;
      cod= cod->next;
      free(head);
    }
    free(cod);
    temp[lungh]= '\0';

    *string= temp;
  }

  return 1;
}

void load_language(FILE *);

string msg[MAX];
string old;

int load_language_file(string language, string directory)
{
  string language_file;
  int len;
  FILE *fp;

  if (language == NULL) return -1;

  if ( (old == NULL) || ((old != NULL) && (strcmp(old, language) != 0)) )
  {
    len= strlen(language) + strlen(directory);
    language_file= (string) calloc(6+len , sizeof(char));
    strcat(language_file, directory);
    strcat(language_file, language);
    strcat(language_file, ".lang");
    language_file[len+5]= '\0';

    if ( (fp= fopen(language_file, "r")) == NULL) return -1;
    load_language(fp);
    fclose(fp);

    if (old != NULL)
    {
      free(old);
      old=NULL;
    }
    old= (string) calloc(1+len, sizeof(char));
    strcpy(old, language);
    old[len]= '\0';
  }

  return 0;
}

string messages(int index)
{
  return msg[index];
}

void load_language(FILE *fp)
{
  int index=0, lun;
  string temp;

  temp= (string) calloc(MAX, sizeof(char));
  while (fgets(temp, MAX, fp) != NULL) if (temp[0] != '#')
  {
    lun= strlen(temp);
    temp[lun-1]= '\0';
    if (strlen(temp) > 0)
    {
      if (msg[index] != NULL) free(msg[index]);
      msg[index]= (string) calloc(lun, sizeof(char));
      strcpy(msg[index], temp);
      index++;
    }
  }
  free(temp);
}