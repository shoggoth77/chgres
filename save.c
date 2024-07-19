#ifdef __PUREC__
#include <portab.h>
#include <aes.h>
#include <tos.h>
#else
#include <gem.h>
#include <osbind.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "extern.h"
#include "vgainf.h"
#include <mint/sysbind.h>
#include <mint/osbind.h>

static char tmpbuf[256];
char gdos_drivers[11][16] = { 0 };

char* default_newdesk = 
"#a000000\r\n\
#b000000\r\n\
#c7770007000600070055200505552220770557075055507703111103\r\n\
#d                                             \r\n\
#K 4F 53 4C 00 46 42 43 57 45 58 00 00 00 00 00 00 00 00 00 00 00 00 00 52 00 00 4D 56 00 00 00 @\r\n\
#E 18 13 00 06 00 00 00 00 00 00 \r\n\
#Q 41 40 43 40 43 40 \r\n\
#W 00 00 0C 06 26 0C 00 @\r\n\
#W 00 00 02 0B 26 09 00 @\r\n\
#W 00 00 0A 0F 1A 09 00 @\r\n\
#W 00 00 0E 01 1A 09 00 @\r\n\
#W 00 00 04 07 26 0C 00 @\r\n\
#W 00 00 0C 0B 26 09 00 @\r\n\
#W 00 00 08 0F 1A 09 00 @\r\n\
#W 00 00 06 01 1A 09 00 @\r\n\
#N FF 04 000 @ *.*@ @ \r\n\
#D FF 01 000 @ *.*@ @ \r\n\
#G 03 FF 000 *.APP@ @ @ \r\n\
#G 03 FF 000 *.PRG@ @ @ \r\n\
#Y 03 FF 000 *.GTP@ @ @ \r\n\
#P 03 FF 000 *.TTP@ @ @ \r\n\
#F 03 04 000 *.TOS@ @ @ \r\n\
#M 00 01 00 FF C HARD DISK@ @ \r\n\
#M 00 00 00 FF A FLOPPY DISK@ @ \r\n\
#M 01 00 00 FF B FLOPPY DISK@ @ \r\n\
#T 00 03 02 FF   TRASH@ @ ";

short save_magx_inf(struct res* r)
{
	WORD ret;
	char *buf;
	long size;
	char *p;
	char linebuf[40];
	long linesize;
	long remain;
	short fd;

	ret = FALSE;
	buf = load_file("\\MAGX.INF", &size);
	if (buf != NULL)
	{
		p = find_line(buf, "#_DEV", size, &remain, &linesize);
		if (remain == 0)
		{
			p = find_line(buf, "#[aes]", size, &remain, &linesize);
			p += linesize;
			linesize = 0;
		}
		fd = (short)Fcreate("\\MAGX.INF", 0);
		if (fd > 0)
		{
			if (remain > 0) {
				Fwrite(fd, p - buf, buf);
				p += linesize;
			} else {
				p = buf;
			}
			sprintf(linebuf, "#_DEV %d %d\r\n", r->base.rez, r->base.vmode);
			Fwrite(fd, strlen(linebuf), linebuf);
			Fwrite(fd, size - (p - buf), p);
			Fclose(fd);
			ret = 1;
		}
		free(buf);
	}
	return ret;
}

static short save_desk_inf_base(WORD rez, WORD vmode, const char* filename, const short eoffset, char* defaultbuf, long defaultsize)
{
	char modestr[6];
	long size, remain, linesize;
	short fd;
	char *p, *buf, *lbuf;
	int ret;

	lbuf = load_file(filename, &size);
	if (lbuf) {
		buf = lbuf;
	} else {
		if (defaultbuf && defaultsize) {
			buf = defaultbuf;
			size = defaultsize;
		} else {
			return 0;
		}
	}

	ret = 0;
	p = find_line(buf, "#E ", size, &remain, &linesize);
	if (remain > 0)
	{
		if ((vmode == 0) && (sys_aes != F_SYSTEM_EMUTOS) && (linesize >= 8))
		{
			sprintf(modestr, "%1x", rez - 2);
			strupr(modestr);
			memcpy(&p[7], modestr, 1);
			ret = 1;
		}
		else
		{
			if (linesize >= (eoffset + 5))
			{
				if ((vmode == 0) && (sys_aes == F_SYSTEM_EMUTOS))
					sprintf(modestr, "FF %02x", rez - 2);
				else
					sprintf(modestr, "%02x %02x", (vmode >> 8), (vmode & 0xFF));
				strupr(modestr);
				memcpy(&p[eoffset], modestr, 5);
				ret = 1;
			}
		}

		if (ret)
		{
			fd = (short)Fcreate(filename, 0);
			if (fd)
			{
				Fwrite(fd, size, buf);
				Fclose(fd);
			}
			ret = fd ? 1 : 0;
		}
	}

	if (lbuf)
		free(lbuf);

	return ret;
}

short save_emudesk_inf(struct res* r)
{
	sprintf(tmpbuf, "%s\\emudesk.inf", get_system_drive());
	return save_desk_inf_base(r->base.rez, r->base.vmode, tmpbuf, 9, 0, 0);
}

short save_newdesk_inf(struct res* r)
{
	sprintf(tmpbuf, "%s\\newdesk.inf", get_system_drive());
	return save_desk_inf_base(r->base.rez, r->base.vmode, tmpbuf, 15, default_newdesk, strlen(default_newdesk));
}

short save_olddesk_inf(struct res* r)
{
	(void)r;
	sprintf(tmpbuf, "%s\\desktop.inf", get_system_drive());
	return 0;
}

short save_naes_inf(struct res* r)
{
	char* aesdir = get_process_path("AESSYS");
	if (aesdir == NULL) {
		aesdir = getenv("AESDIR");
		if (aesdir == NULL || *aesdir == 0)
			return 0;
	}
	sprintf(tmpbuf, "%s/n_desk.inf", aesdir);
	return save_desk_inf_base(r->base.rez, r->base.vmode, tmpbuf, 15, default_newdesk, strlen(default_newdesk));
}

short save_xaaes_inf(struct res* r)
{
	long sc, sn;
	long s1, s2;
	char *lc, *ln, *buf;
	short fd;
	int ret = 0;

	char* aesdir = get_process_path("XaSYS");
	if (aesdir) {
		sprintf(tmpbuf, "%s/xaaes.cnf", aesdir);
	} else {
		aesdir = getenv("SYSDIR");
		if (aesdir == NULL)
			return 0;
		sprintf(tmpbuf, "%s/xaaes/xaaes.cnf", aesdir);
	}

	buf = load_file(tmpbuf, &sc);
	if (buf)
	{
		fd = (short)Fopen(tmpbuf, 1);
		if (fd > 0)
		{
			lc = buf;
			ln = remove_line(buf, "SCREENDEV", sc, &sn);
			if (ln) { lc = ln; sc = sn; }
			ln = remove_line(buf, "MODECODE", sc, &sn);
			if (ln) { lc = ln; sc = sn; }
			s1 = lc - buf;
			s2 = sn - s1;
			if (s1 > 0) { Fwrite(fd, s1, buf); }
			sprintf(tmpbuf, "SCREENDEV = %d\r\nMODECODE = 0x%04x\r\n", r->base.rez, r->base.vmode);
			Fwrite(fd, strlen(tmpbuf), tmpbuf);
			if (s2 > 0) { Fwrite(fd, s2, lc); }
			Fclose(fd);
			ret = 1;
		}
		free(buf);
	}
	return ret;
}

short save_myaes_inf(struct res* r)
{
	int esetshift = 0;
	long sc, sn;
	long s1, s2;
	char *lc, *ln, *buf;
	short fd;
	int ret = 0;

	char* aesdir = get_process_path("myaes");
	if (aesdir == NULL)
		aesdir = "c:\\gemsys\\myaes";

	sprintf(tmpbuf, "%s/myaes.cnf", aesdir);
	buf = load_file(tmpbuf, &sc);
	if (buf)
	{
		fd = (short)Fopen(tmpbuf, 1);
		if (fd > 0)
		{
			lc = buf;
			ln = remove_line(buf, "vsetmode", sc, &sn);
			if (ln) { lc = ln; sc = sn; }
			ln = remove_line(buf, "rezmode", sc, &sn);
			if (ln) { lc = ln; sc = sn; }
			ln = remove_line(buf, "esetshift", sc, &sn);
			if (ln) { lc = ln; sc = sn; }
			s1 = lc - buf;
			s2 = sn - s1;
			if (s1 > 0) { Fwrite(fd, s1, buf); }
			// todo: esetshift for TT
			sprintf(tmpbuf, "vsetmode = %d\r\nrezmode = %d\r\nesetshift = %d\r\n", r->base.vmode, r->base.rez - 2, esetshift);
			Fwrite(fd, strlen(tmpbuf), tmpbuf);
			if (s2 > 0) { Fwrite(fd, s2, lc); }
			Fclose(fd);
			ret = 1;
		}
		free(buf);
	}
	return ret;
}

short save_fvdi_inf(struct res* r)
{
	char c;
	long dummy;
	long oldsize, newsize;
	char *lp, *lp2;
	char *buf;
	short fd;
	short i;
	long size1, size2;
	char driverid[32];
	int ret = -1;

	sprintf(tmpbuf, "%s\\fvdi.sys", get_system_drive());
	buf = load_file(tmpbuf, &oldsize);
	if (buf == NULL)
		return -1;

	sprintf(driverid, "#[driver%d]", r->planes);
	lp = find_line(buf, driverid, oldsize, &size1, &size2);
	if (size1 > 0) {
		lp2 = skip_white(lp + strlen(driverid));
		size2 = size2 - (lp2 - lp);
		if ((size2 > 0) && (size2 < 31))
		{
			for (i=0; i<size2; i++) {
				c = lp2[i];
				if (c == '\r' || c == '\n' || c == ' ')
					c = 0;
				driverid[i] = c;
			}
		}
	}

	if (driverid[0] != '#')
	{
		lp = remove_line(buf, "01r ", oldsize, &newsize);
		if (lp)
		{
			lp2 = remove_line(buf, "01r ", newsize, &dummy);
			if (lp2 == NULL)
			{
				fd = (short)Fcreate(tmpbuf, 0);
				if (fd > 0)
				{
					size1 = lp - buf;
					size2 = newsize - size1;
					Fwrite(fd, size1, buf);
					sprintf(tmpbuf, "01r %s mode %dx%dx%d@%d\r\n", driverid, r->base.hres, r->base.vres, r->planes, r->freq);
					Fwrite(fd, strlen(tmpbuf), tmpbuf);
					Fwrite(fd, size2, lp);
					Fclose(fd);
					ret = 1;
				}
			}
		}
	}

	free(buf);
	return ret;
}

void read_assign_sys()
{
	char buf[512];
	long size;
	long valid;
	char *assign;
	char *p, *l;
	short id;
	long linesize;
	int i;

	sprintf(buf, "%s\\assign.sys", get_system_drive());
	assign = load_file(buf, &size);
	if (assign == NULL) {
		return;
	}
	p = assign;
	do {
		linesize = next_line(p, size, &valid);
		size -= linesize;
		if (valid < 512)
		{
			memcpy(buf, p, valid);
			buf[valid] = '\0';
		} else
		{
			memcpy(buf, p, 512);
			buf[511] = '\0';
		}
		strupr(buf);
		l = strstr(buf, ".SYS");
		if (l) {
			while (l-- > buf)
			{
				if ((*l < '0') || (*l > 'Z'))
				{
					id = atoi(buf);
					if ((id > 0) && (id < 11)) {
						strncpy(gdos_drivers[id], l+1, 13);
					}
					break;
				}
			}
		}
		p += linesize;
	} while (size > 0 && linesize > 0);
	free(assign);
}
