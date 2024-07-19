#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <mint/sysbind.h>
#include <mint/osbind.h>

char* get_system_drive()
{
	return "C:";
}

short get_process_id(const char* processname)
{
	char filter[64];
	short pid = -1;
	sprintf(filter, "u:/proc/%s*.*", processname);
	if (Fsfirst(filter, 0x7) == 0)
	{
		_DTA* dta = Fgetdta();
		if (dta)
		{
			long len = strnlen(dta->dta_name, 12);
			if (len > 4)
			{
				pid = atoi(dta->dta_name+strlen(dta->dta_name)-3);
			}
		}
	}
	return pid;
}

char* get_process_path(const char* processname)
{
	static char buf[256];
	short pid = get_process_id(processname);
	if (pid >= 0) {
		sprintf(buf, "u:/kern/%d/fname", pid);
		short fh = Fopen(buf, 0);
		if (fh > 0)
		{
			memset(buf, 0, 256);
			Fread(fh, 255, buf);
			Fclose(fh);
			char* p = strrchr(buf, '/');
			if (p && (p != buf))
			{
				*p = 0;
				return buf;
			}
		}
		sprintf(buf, "u:/kern/%d/cwd", pid);
		return buf;
	}
	return NULL;
}

char *load_file(const char *filename, long *size)
{
	*size = 0;
	char* buf = NULL;
	short fd = (short)Fopen(filename, FO_READ);
	if (fd > 0)
	{
		long length = Fseek(0, fd, SEEK_END);
		Fseek(0, fd, SEEK_SET);
		buf = (char *)malloc(length);
		if (buf != NULL)
		{
			*size = Fread(fd, length, buf);
			if (*size != length)
			{
				free(buf);
				buf = NULL;
			}
		}
		Fclose(fd);
	}
	return buf;
}

char* skip_white(char *s)
{
	while (*s == ' ' || *s == '\t')
		s++;
	return s;
}

static char *find_str(char *line, const char *str)
{
	char* start = skip_white(line);
	char* p = start;
	while (*p && (*p == *str)) {
		p++; str++;
	}
	return (*str == '\0') ? start : NULL;
}

long next_line(char *s, long maxlen, long *valid)
{
	*valid = 0;
	char* p = s;
	short comment = 0;
	while (maxlen > 0)
	{
		char c = *p++;
		maxlen--;
		if (c == 0x0d)
		{
			if (*p == 0x0a)
			{
				p++;
				maxlen--;
			}
			break;
		}
		if (c == 0x0a)
			break;
		if (c == ';' || c == '*')
			comment = 1;
		if (comment == 0)
			++(*valid);
	}
	return p - s;
}

char *find_line(char *p, const char *str, long size, long *remain, long *linesize)
{
	short i;
	char linebuf[512];
	long lsize = 0;
	do
	{
		long valid = 0;
		lsize = next_line(p, size, &valid);
		size -= lsize;
		if (valid > 511)
			valid = 511;
		for (i = 0; i < valid; i++)
			linebuf[i] = p[i];
		linebuf[i] = '\0';
		if (find_str(linebuf, str) != NULL)
			break;
		p += lsize;
	} while (size > 0 && lsize > 0);
	*remain = size;
	*linesize = lsize;
	return p;
}

char *remove_line(char *p, const char *str, long size, long *newsize)
{
	long remain = 0;
	long linesize = 0;
	char* fl = find_line(p, str, size, &remain, &linesize);
	if (remain == 0)
		return NULL;

	char* nl = fl + linesize;
	memmove(fl, nl, remain);
	*newsize = (size - linesize);
	return fl;
}
