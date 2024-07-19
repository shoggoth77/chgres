#ifdef __PUREC__
#include <portab.h>
#include <aes.h>
#include <wdlgwdlg.h>
#include <wdlglbox.h>
#include <tos.h>
#else
#include <gemx.h>
#include <osbind.h>
#endif
#include <mint/falcon.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "chgres.h"
#include "extern.h"
#include "vgainf.h"

extern char gdos_drivers[11][16];


/* ------------------------------------------------------------
	Atari ST/STE shifter
   --------------------------------------------------------- */
/*static long get_st_monitor(void) {
	unsigned char shiftmod = *((unsigned char *)0xFFFF8260UL);
	if (((shiftmod & 3) + 2) == REZ_ST_HIGH)
		return MON_MONO;
	return MON_COLOR;
}*/
short initres_st(short* outrez, short* outmode)
{
	create_mode(0, 640, 400, 1, REZ_ST_HIGH, 	0, "ST Hoch");
	create_mode(1, 640, 200, 2, REZ_ST_MED, 	0, "ST Mittel");
	create_mode(2, 320, 200, 4, REZ_ST_LOW, 	0, "ST Niedrig");
	*outrez = Getrez() + 2;
	*outmode = 0;
	return 1;
}

/* ------------------------------------------------------------
	Atari TT shifter
   --------------------------------------------------------- */
static long get_tt_monitor(void) {
	unsigned char shiftmod = *((volatile unsigned char *)0xFFFF8262UL);
	if (((shiftmod & 7) + 2) == REZ_TT_HIGH)
		return MON_MONO;
	return MON_COLOR;
}
short initres_tt(short* outrez, short* outmode)
{
	short monitor_type = Supexec(get_tt_monitor);
	if (monitor_type == MON_MONO) {
		create_mode(0, 1280, 960, 1, REZ_TT_HIGH,	0, "TT Hoch");
	} else {
		create_mode(2,  640, 480, 4, REZ_TT_MED,	0, "TT Mittel");
		create_mode(3,  320, 480, 8, REZ_TT_LOW,	0, "TT Niedrig");
		create_mode(0,  640, 400, 1, REZ_ST_HIGH, 	0, "ST Hoch");
		create_mode(1,  640, 200, 2, REZ_ST_MED, 	0, "ST Mittel");
		create_mode(2,  320, 200, 4, REZ_ST_LOW, 	0, "ST Niedrig");
	}
	*outrez = Getrez() + 2;
	*outmode = 0;
	return 1;
}

/* ------------------------------------------------------------
	Atari Falcon Videl
   --------------------------------------------------------- */
short initres_falc(short* outrez, short* outmode)
{
	short monitor_type = VgetMonitor();
	if (monitor_type == MON_MONO) {
		create_mode(0,  640, 400, 1, REZ_FALCON, STMODES|COL80|BPS1, 					"ST Hoch");
	} else if (monitor_type == MON_VGA) {
		create_mode(0,  640, 240,  1, REZ_FALCON, VERTFLAG|VGA|COL80|BPS1, 				0);
		create_mode(0,  640, 400,  1, REZ_FALCON, STMODES|VGA|COL80|BPS1, 				"ST Hoch");
		create_mode(0,  640, 240,  1, REZ_FALCON, VERTFLAG|VGA|COL80|BPS1, 				0);
		create_mode(1,  320, 240,  2, REZ_FALCON, VERTFLAG|VGA|BPS2, 					0);
		create_mode(1,  320, 480,  2, REZ_FALCON, VGA|BPS2, 							0);
		create_mode(1,  640, 200,  2, REZ_FALCON, VERTFLAG|STMODES|VGA|COL80|BPS2, 		"ST Mittel");
		create_mode(1,  640, 240,  2, REZ_FALCON, VERTFLAG|VGA|COL80|BPS2, 				0);
		create_mode(1,  640, 480,  2, REZ_FALCON, VGA|COL80|BPS2, 						0);
		create_mode(2,  320, 200,  4, REZ_FALCON, VERTFLAG|STMODES|VGA|BPS4,			"ST Niedrig");
		create_mode(2,  320, 240,  4, REZ_FALCON, VERTFLAG|VGA|BPS4,					0);
		create_mode(2,  320, 480,  4, REZ_FALCON, VGA|BPS4,								0);
		create_mode(2,  640, 240,  4, REZ_FALCON, VERTFLAG|VGA|COL80|BPS4,				0);
		create_mode(2,  640, 480,  4, REZ_FALCON, VGA|COL80|BPS4,						0);
		create_mode(3,  320, 240,  8, REZ_FALCON, VERTFLAG|VGA|BPS8,					0);
		create_mode(3,  320, 480,  8, REZ_FALCON, VGA|BPS8,								0);
		create_mode(3,  640, 240,  8, REZ_FALCON, VERTFLAG|VGA|COL80|BPS8,				0);
		create_mode(3,  640, 480,  8, REZ_FALCON, VGA|COL80|BPS8,						0);
		create_mode(4,  320, 240, 16, REZ_FALCON, VERTFLAG|VGA|BPS16,					0);
		create_mode(4,  320, 480, 16, REZ_FALCON, VGA|BPS16,							0);
	} else {
		create_mode(0,  640, 240,  1, REZ_FALCON, TV|COL80|BPS1,		 				0);
		create_mode(0,  640, 400,  1, REZ_FALCON, VERTFLAG|TV|STMODES|COL80|BPS1,		"ST Hoch");
		create_mode(0,  768, 240,  1, REZ_FALCON, TV|OVERSCAN|COL80|BPS1, 				0);
		create_mode(0,  768, 480,  1, REZ_FALCON, VERTFLAG|TV|OVERSCAN|COL80|BPS1, 		0);
		create_mode(1,  320, 200,  2, REZ_FALCON, TV|BPS2,				 				0);
		create_mode(1,  320, 400,  2, REZ_FALCON, VERTFLAG|TV|BPS2,		 				"interlaced");
		create_mode(1,  320, 240,  2, REZ_FALCON, TV|OVERSCAN|BPS2,		 				0);
		create_mode(1,  320, 480,  2, REZ_FALCON, VERTFLAG|TV|OVERSCAN|BPS2,			"interlaced");
		create_mode(1,  640, 200,  2, REZ_FALCON, TV|STMODES|COL80|BPS2,				"ST Mittel");
		create_mode(1,  640, 400,  2, REZ_FALCON, VERTFLAG|TV|COL80|BPS2,				"interlaced");
		create_mode(1,  768, 240,  2, REZ_FALCON, TV|OVERSCAN|COL80|BPS2,				0);
		create_mode(1,  768, 480,  2, REZ_FALCON, VERTFLAG|TV|OVERSCAN|COL80|BPS2,		"interlaced");
		create_mode(2,  320, 200,  4, REZ_FALCON, TV|STMODES|BPS4,		 				"ST Niedrig");
		create_mode(2,  320, 400,  4, REZ_FALCON, VERTFLAG|TV|BPS4,		 				"interlaced");
		create_mode(2,  384, 240,  4, REZ_FALCON, OVERSCAN|BPS4,		 				0);
		create_mode(2,  384, 480,  4, REZ_FALCON, VERTFLAG|TV|OVERSCAN|BPS4,			"interlaced");
		create_mode(2,  640, 200,  4, REZ_FALCON, COL80|BPS4,			 				0);
		create_mode(2,  640, 400,  4, REZ_FALCON, VERTFLAG|TV|COL80|BPS4,				"interlaced");
		create_mode(2,  768, 240,  4, REZ_FALCON, OVERSCAN|COL80|BPS4,	 				0);
		create_mode(2,  768, 480,  4, REZ_FALCON, VERTFLAG|TV|OVERSCAN|COL80|BPS4,		"interlaced");
		create_mode(3,  320, 200,  8, REZ_FALCON, TV|BPS8,		 						0);
		create_mode(3,  320, 400,  8, REZ_FALCON, VERTFLAG|TV|BPS8,		 				"interlaced");
		create_mode(3,  384, 240,  8, REZ_FALCON, TV|OVERSCAN|BPS8,		 				0);
		create_mode(3,  384, 480,  8, REZ_FALCON, VERTFLAG|TV|OVERSCAN|BPS8,			"interlaced");
		create_mode(3,  640, 200,  8, REZ_FALCON, TV|COL80|BPS8,						0);
		create_mode(3,  640, 400,  8, REZ_FALCON, VERTFLAG|TV|COL80|BPS8,				"interlaced");
		create_mode(3,  768, 240,  8, REZ_FALCON, TV|OVERSCAN|COL80|BPS8,				0);
		create_mode(3,  768, 480,  8, REZ_FALCON, VERTFLAG|TV|OVERSCAN|COL80|BPS8,		"interlaced");
		create_mode(4,  320, 200, 16, REZ_FALCON, TV|BPS16,		 						0);
		create_mode(4,  320, 400, 16, REZ_FALCON, VERTFLAG|TV|BPS16,		 			"interlaced");
		create_mode(4,  384, 240, 16, REZ_FALCON, TV|OVERSCAN|BPS16,		 			0);
		create_mode(4,  384, 480, 16, REZ_FALCON, VERTFLAG|TV|OVERSCAN|BPS16,			"interlaced");
		create_mode(4,  640, 200, 16, REZ_FALCON, TV|COL80|BPS16,						0);
		create_mode(4,  640, 400, 16, REZ_FALCON, VERTFLAG|TV|COL80|BPS16,				"interlaced");
		create_mode(4,  768, 240, 16, REZ_FALCON, TV|OVERSCAN|COL80|BPS16,				0);
		create_mode(4,  768, 480, 16, REZ_FALCON, VERTFLAG|TV|OVERSCAN|COL80|BPS16,		"interlaced");
	}
	*outrez = REZ_FALCON;
	*outmode = VsetMode(-1) & ~PAL;
	return 1;
}

/* ------------------------------------------------------------
	Vampire SAGA
   --------------------------------------------------------- */
short initres_saga(short* outrez, short* outmode)
{
	short i,j,bpp,vmode;
	struct sagares { short reg, width, height; };
	const struct sagares saga_resolutions[] = {
		{ 0x01,  320, 200 },		/*  16:10 */
		{ 0x02,  320, 240 },		/*   4:3  */
		{ 0x03,  320, 256 },		/*   5:4  */
		{ 0x04,  640, 400 },		/*  16:10 */
		{ 0x05,  640, 480 },		/*   4:3  */
		{ 0x06,  640, 512 },		/*   5:4  */
		{ 0x07,  960, 540 },		/*  16:9  */
		{ 0x08,  480, 270 },		/*  16:9  */
		{ 0x09,  304, 224 },
		{ 0x0A, 1280, 720 },		/*  16:9  */
		{ 0x0B,  640, 360 },		/*  16:9  */
		{ 0x0C,  800, 600 },		/*   4:3  */
		{ 0x0D, 1024, 768 },		/*   4:3  */
		{ 0x0E,  720, 576 },		/*   5:4  */
		{ 0x0F,  848, 480 },
		{ 0x10,  640, 200 }
	};
	const short num_saga_resolutions = sizeof(saga_resolutions) / sizeof(saga_resolutions[0]);

	struct sagabpp { short reg, bpp; };
	struct sagabpp saga_depths[MAX_DEPTHS] = {
		{ 0x08,  1 },
		{ 0x09,  2 },
		{ 0x0A,  4 },
		{ 0x01,  8 },
		{ 0x03, 15 },
		{ 0x02, 16 },
		{ 0x04, 24 },
		{ 0x05, 32 }
	};

#if 0
	/* non-atari modes require driver */
	for (short i = 3; i < MAX_DEPTHS; i++) {
		if (*(vdidriver[i]) == 0)
			saga_depths[i].reg = saga_depths[i].bpp = 0;
	}
#endif

	/* get rid of 15 & 24bpp */
	saga_depths[4].reg = saga_depths[4].bpp = 0;
	saga_depths[6].reg = saga_depths[6].bpp = 0;

	/* fVDI driver for SAGA doesn't support less than 16bpp */
	if (sys_vdi & F_VDI_FVDI)
		for (i=0; i<4; i++)
			saga_depths[i].reg = saga_depths[i].bpp = 0;

	/* atari compatible modes */
	if ((sys_vdi & F_VDI_FVDI) == 0) {
		create_mode(2, 640, 480, 4, REZ_FALCON, VGA|COL80|BPS4,						"TT Mittel");
		create_mode(0, 640, 400, 1, REZ_FALCON, STMODES|VGA|COL80|BPS1, 			"ST Hoch");
		create_mode(1, 640, 200, 2, REZ_FALCON, VERTFLAG|STMODES|VGA|COL80|BPS2, 	"ST Mittel");
		create_mode(2, 320, 200, 4, REZ_FALCON, VERTFLAG|STMODES|VGA|BPS4, 			"ST Niedrig");
	}

	/* saga modes */
	for (i = 0; i < MAX_DEPTHS; i++)
	{
		bpp = saga_depths[i].bpp;
		if (bpp > 0)
		{
			for (j = 0; j < num_saga_resolutions; j++)
			{
				vmode = 0x4000 | (saga_resolutions[j].reg << 8) | saga_depths[i].reg;
				if (!find_mode(i, saga_resolutions[j].width, saga_resolutions[j].height, bpp))
				{
					create_mode(
						i,
						saga_resolutions[j].width,
						saga_resolutions[j].height,
						bpp,
						REZ_FALCON,
						vmode,
						NULL);
				}
			}
		}
	}

	*outrez = REZ_FALCON;
	*outmode = VsetMode(-1) & ~PAL;
	return 1;
}


/* ------------------------------------------------------------
	NVDIVGA Graphics card
   --------------------------------------------------------- */

struct et4000mode { short index; short planes; char* driver; short rez; short count; };
struct et4000mode et4000modes[] =
{
	{ 0,  1, "XVGA2.SYS", 	0, 0},
	{ 2,  4, "XVGA16.SYS",	0, 0 },
	{ 3,  8, "XVGA256.SYS", 0, 0 },
	{ 4, 15, "XVGA32K.SYS", 0, 0 },
	{ 5, 16, "XVGA65K.SYS", 0, 0 },
	{ 6, 24, "XVGA16M.SYS", 0, 0 },
};
const short num_et4000modes = sizeof(et4000modes) / sizeof(et4000modes[0]);

static short get_gdos_driver(const char* name)
{
	short i;
	for (i = 1; i < 12; i++) {
		if (strcmp(gdos_drivers[i], name) == 0) {
			return i;
		}
	}
	return 0;
}

short save_nvdivga_inf(struct res *res)
{
	int i;
	long fd, offset;
	short vmode;
	struct vgainf_mode mode;

	char buf[64];
	sprintf(buf, "%s\\auto\\nvdivga.inf", get_system_drive());
	fd = Fopen(buf, FO_WRITE);
	if (fd > 0) {
		for (i = 0; i < num_et4000modes; i++) {
			if (et4000modes[i].planes == res->planes) {
				vmode = res->base.vmode | 0xc000;
				offset = offsetof(struct vgainf, vgainf_defmode);
				offset += 2 * (long)i;
				Fseek(offset, (short)fd, SEEK_SET);
				Fwrite((short)fd, sizeof(vmode), &vmode);
				break;
			}
		}
		mode.vga_xres = res->base.virt_hres - 1;
		mode.vga_yres = res->base.virt_vres - 1;
		offset = offsetof(struct vgainf_mode, vga_xres) + res->mode_offset;
		Fseek(offset, (short)fd, SEEK_SET);
		Fwrite((short)fd, 2 * sizeof(mode.vga_xres), &mode.vga_xres);
		Fclose((short)fd);
		return 1;
	}
	return 0;
}

static void create_nvdivga_mode(struct vgainf *inf, struct vgainf_mode *mode, short modeidx, short idx)
{
	struct res *res;
	
	res = (struct res *)malloc(sizeof(*res));
	if (res == NULL)
		return;
	res->base.selected = FALSE;
	res->base.desc = res->descbuf;
	res->base.vmode = modeidx + SVEXT;
	res->base.flags = FLAG_INFO;
	res->planes = et4000modes[idx].planes;
	res->base.rez = et4000modes[idx].rez;
	res->base.virt_hres = mode->vga_xres + 1;
	res->base.virt_vres = mode->vga_yres + 1;
	res->base.hres = mode->vga_visible_xres + 1;
	res->base.vres = mode->vga_visible_yres + 1;
	res->freq = mode->vga_vfreq / 10;
	res->mode_offset = (long)mode - (long)inf;
	if (modeidx == inf->vgainf_defmode[idx])
		res->base.flags |= FLAG_DEFMODE;
	sprintf(res->base.desc, "%4d x %4d, ET4000", res->base.virt_hres, res->base.virt_vres);
	if (res->base.hres != res->base.virt_hres || res->base.vres != res->base.virt_vres)
	{
		strcat(res->base.desc, ", ");
		strcat(res->base.desc, rs_string(FS_VIRTUAL));
	}
	add_mode(et4000modes[idx].index, res);
}

short initres_nvdivga(short* outrez, short* outmode)
{
	char fname[64];
	short i,j;
	long size;
	struct vgainf *inf;
	struct vgainf_mode *mode;

	if ((sys_vdi & F_VDI_NVDI) == 0)
		return 0;

	/* check if nvdivga drivers are being used */
	for (i = 0; i < num_et4000modes; i++) {
		j = get_gdos_driver(et4000modes[i].driver);
		sys_vdi |= (j > 0) ? F_VDI_NVDIVGA : 0;
		et4000modes[i].rez = j;
	}

	/* assume we're using nvdivga now */
	if ((sys_vdi & F_VDI_NVDIVGA) != F_VDI_NVDIVGA)
		return 0;

	/* get rid of previously added resolutions for modes owned by graphics card */
	for (i = 0; i < num_et4000modes; i++) {
		if (et4000modes[i].rez > 0) {
			remove_modes(et4000modes[i].index);
		}
	}

	/* add modes from nvdivga.inf */
	sprintf(fname, "%s\\auto\\nvdivga.inf", get_system_drive());
	inf = (struct vgainf *)load_file(fname, &size);
	if (inf)
	{
		mode = (struct vgainf_mode *)((char *)inf + sizeof(*inf));
		if (inf->vgainf_nummodes > 0)
		{
			while (mode != (struct vgainf_mode *)-1)
			{
				mode->vga_modename = (char *)((long)mode + (long)mode->vga_modename);
				mode->vga_ts_regs = (unsigned char *)((long)mode + (long)mode->vga_ts_regs);
				mode->vga_crtc_regs = (unsigned char *)((long)mode + (long)mode->vga_crtc_regs);
				mode->vga_atc_regs = (unsigned char *)((long)mode + (long)mode->vga_atc_regs);
				mode->vga_gdc_regs = (unsigned char *)((long)mode + (long)mode->vga_gdc_regs);
				if (mode->vga_next != (struct vgainf_mode *)-1) {
					mode->vga_next = (struct vgainf_mode *)((long)mode + (long)mode->vga_next);
				}
				for (i = 0; i < num_et4000modes; i++)
				{
					if (et4000modes[i].planes == mode->vga_planes) {
						create_nvdivga_mode(inf, mode, et4000modes[i].count, i);
						et4000modes[i].count++;
					}
				}
				mode = mode->vga_next;
			}
		}
		free(inf);
	}

	(void)outrez;
	(void)outmode;
	return 0;
}
