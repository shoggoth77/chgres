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
#include <mint/sysvars.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "chgres.h"
#include "extern.h"
#include "vgainf.h"

extern void sleep(long s);

#define COOKIE_P ((long **)0x5a0)

#define SV_INQUIRE		0	/* inquire sysvar data, see mt_objc_sysvar() */
#define AD3DVAL      	6	/* AES 4.0     */
#define AES_OBJECT		13

WORD vdo;
WORD mch;
WORD cpu;

WORD sys_aes;			/* F_SYSTEM_x */
WORD sys_vdi;			/* F_VDI_x */
short aes_ver;

WORD tos_version;
WORD tos_emutos;
WORD aes_ver;

WORD gl_apid;
static WORD aes_handle;
WORD gl_wchar;
WORD gl_hchar;
WORD gl_wbox;
WORD gl_hbox;
static char **rs_frstr;
static OBJECT **rs_trindex;

static struct res *possible_resolutions[MAX_DEPTHS];
static WORD bpp_index;
static LIST_BOX *color_lbox; 
static DIALOG *dialog;
static const char *lbox_names[MAX_DEPTHS];
static short name_count;
static struct res *res_tab;
static struct res *cur_res;
static WORD must_shutdown;

static const char *const bpp_tab[MAX_DEPTHS] = {
	"    2",
	"    4",
	"   16",
	"  256",
	"  32K",
	"  65K",
	"  16M",
	"  16M"
};

#define N_ITEMS CHGRES_BOX_LAST-CHGRES_BOX_FIRST+1
static WORD const ctrl_objs[5] = { CHGRES_BOX, CHGRES_UP, CHGRES_DOWN, CHGRES_BACK, CHGRES_SLIDER };
static WORD const objs[N_ITEMS] = {
	CHGRES_BOX_FIRST,
	CHGRES_BOX_FIRST+1,
	CHGRES_BOX_FIRST+2,
	CHGRES_BOX_FIRST+3,
	CHGRES_BOX_FIRST+4,
	CHGRES_BOX_FIRST+5,
	CHGRES_BOX_FIRST+6,
	CHGRES_BOX_FIRST+7,
	CHGRES_BOX_FIRST+8,
	CHGRES_BOX_LAST
};

static short const valid_planes[MAX_DEPTHS] = { 1, 2, 4, 8, 15, 16, 24, 32 };


char* rs_string(long stringid)
{
	return rs_frstr[stringid];
}

static int do_dialog(OBJECT *tree)
{
	GRECT gr;
	void *flydial;
	WORD lastcrsr;
	int ret;
	
	wind_update(BEG_UPDATE);
	wind_update(BEG_MCTRL);
	form_center_grect(tree, &gr);
	form_xdial_grect(FMD_START, &gr, &gr, &flydial);
	objc_draw_grect(tree, ROOT, MAX_DEPTH, &gr);
	ret = form_xdo(tree, ROOT, &lastcrsr, NULL, flydial);
	ret &= 0x7fff;
	form_xdial_grect(FMD_FINISH, &gr, &gr, &flydial);
	wind_update(END_MCTRL);
	wind_update(END_UPDATE);
	tree[ret].ob_state &= ~OS_SELECTED;
	return ret;
}


static WORD _CDECL set_item(struct SET_ITEM_args args)
{
	OBJECT *tree = args.tree;
	char *text;
	struct res *res = (struct res *)args.item;
	WORD obj_index = args.obj_index;
	const char *src;
	
	text = tree[obj_index].ob_spec.tedinfo->te_ptext;
	if (res != NULL)
	{
		if (res->base.selected)
			tree[obj_index].ob_state |= OS_SELECTED;
		else
			tree[obj_index].ob_state &= ~OS_SELECTED;
		src = res->base.desc;
		if (*text)
			*text++ = ' ';
		while (*text && *src)
			*text++ = *src++;
	} else
	{
		tree[obj_index].ob_state &= ~OS_SELECTED;
	}
	while (*text)
		*text++ = ' ';
	return obj_index;
}


static void redraw_obj(DIALOG *dlg, WORD obj)
{
	OBJECT *tree;
	GRECT gr;
	
	wdlg_get_tree(dlg, &tree, &gr);
	wind_update(BEG_UPDATE);
	wdlg_redraw(dlg, &gr, obj, MAX_DEPTH);
	wind_update(END_UPDATE);
}


static void _CDECL select_item(struct SLCT_ITEM_args args)
{
	OBJECT *tree = args.tree;
	struct res *res = (struct res *)args.item;
	DIALOG *dialog = (DIALOG *)args.user_data;
	
	if (res != NULL)
	{
		if (cur_res == NULL)
		{
			tree[CHGRES_OK].ob_state &= ~OS_DISABLED;
			tree[CHGRES_OK].ob_flags |= OF_SELECTABLE;
			tree[CHGRES_OK].ob_flags |= OF_DEFAULT;
			redraw_obj(dialog, CHGRES_OK);
		}
		if (res->base.flags & FLAG_INFO)
		{
			tree[CHGRES_INFO].ob_state &= ~OS_DISABLED;
			tree[CHGRES_INFO].ob_flags |= OF_SELECTABLE;
		} else
		{
			tree[CHGRES_INFO].ob_state |= OS_DISABLED;
			tree[CHGRES_INFO].ob_flags &= ~OF_SELECTABLE;
		}
		redraw_obj(dialog, CHGRES_INFO);
		cur_res = res;
	}
}


static void set_bpp(WORD bpp_index)
{
	const char *src;
	char *dst;
	OBJECT *tree;
	
	src = bpp_tab[bpp_index];
	tree = rs_trindex[MAIN_DIALOG];
	dst = tree[CHGRES_COLORS].ob_spec.free_string;
	if (*dst)
		*dst++ = ' ';
	if (*dst)
		*dst++ = ' ';
	while (*dst && *src)
		*dst++ = *src++;
	while (*dst)
		*dst++ = ' ';
}


static void init_lbox_names(void)
{
	int i;
	const char **p;
	
	for (i = 0; i < MAX_DEPTHS; i++)
		lbox_names[i] = NULL;
	p = lbox_names;
	name_count = 0;
	for (i = 0; i < MAX_DEPTHS; i++)
	{
		if (possible_resolutions[i] != NULL)
		{
			*p++ = bpp_tab[i];
			name_count++;
		}
	}
}


static int popup_to_index(int idx)
{
	int i;
	
	for (i = 0; i < MAX_DEPTHS; i++)
	{
		switch (valid_planes[i])
		{
		default:
			if (possible_resolutions[i] != NULL)
				--idx;
			break;
		case 8:
		case 15:
		case 16:
		case 32:
			if (possible_resolutions[i] != NULL)
				--idx;
			break;
		}
		if (idx < 0)
			return i;
	}
	return -1;
}


static int index_to_popup(int bpp)
{
	int i;
	int idx;
	
	idx = -1;
	for (i = 0; i <= bpp; i++)
		if (possible_resolutions[i] != NULL)
			idx++;
	return idx;
}

static int selected_res_index(struct res *res)
{
	int idx = 0;
	while (res != NULL)
	{
		if (res->base.selected)
			return idx;
		res = res->base.next;
		idx++;
	}
	return -1;
}

static int count_res(struct res *res)
{
	int count;

	count = 0;
	while (res != NULL)
	{
		res = res->base.next;
		count++;
	}
	return count;
}


static int cmp_res(const void *res1, const void *res2)
{
	const struct res *r1 = *(const void *const *)res1;
	const struct res *r2 = *(const void *const *)res2;
	if ((r1->base.virt_hres > r2->base.virt_hres) ||
     ((r1->base.virt_hres >= r2->base.virt_hres &&
      ((r1->base.virt_vres > r2->base.virt_vres ||
       ((r1->base.virt_vres >= r2->base.virt_vres &&
        ((r1->base.hres > r2->base.hres ||
         ((r1->base.hres >= r2->base.hres &&
          ((r1->base.vres > r2->base.vres ||
           ((r1->base.vres >= r2->base.vres &&
            (((r1->base.flags & FLAG_INFO) != 0)) && (((r2->base.flags & FLAG_INFO) == 0 ||
             r1->freq > r2->freq)))))))))))))))))
	    return -1;
	return 1;
}


static struct res *sort_restab(struct res *res)
{
	struct res **table;
	int count;
	struct res **respp;
	struct res *ret;
	
	count = count_res(res);
	ret = res;
	table = (struct res **)malloc(count * sizeof(*table));
	if (table != NULL)
	{
		respp = table;
		while (res != NULL)
		{
			*respp++ = res;
			res = res->base.next;
		}
		qsort(table, count, sizeof(*table), cmp_res);
		ret = table[0];
		respp = table;
		while (count > 1)
		{
			respp[0]->base.next = respp[1];
			respp++;
			count--;
		}
		respp[0]->base.next = NULL;
		free(table);
	}
	return ret;
}


static void translate_restab(void)
{
	struct res *res;
	char *p;
	
	for (res = res_tab; res != NULL; res = res->base.next)
	{
		if ((p = strstr(res->base.desc, "Niedrig")) != NULL)
			strcpy(p, rs_string(FS_LOW));
		if ((p = strstr(res->base.desc, "Mittel")) != NULL)
			strcpy(p, rs_string(FS_MED));
		if ((p = strstr(res->base.desc, "Hoch")) != NULL)
			strcpy(p, rs_string(FS_HIGH));
	}
}


static void set_items(void)
{
	translate_restab();
	lbox_set_items(color_lbox, (LBOX_ITEM *)res_tab);
	lbox_update(color_lbox, NULL);
}


static void show_info(struct res *res)
{
	OBJECT *tree;
	long linesize;
	short orig_hres;
	short orig_vres;
	
	tree = rs_trindex[INFO_DIALOG];
	sprintf(tree[INFO_HRES].ob_spec.free_string, "%4d", res->base.hres);
	sprintf(tree[INFO_VRES].ob_spec.free_string, "%4d", res->base.vres);
	sprintf(tree[INFO_FREQ].ob_spec.free_string, "%4d", res->base.flags & FLAG_INFO ? res->freq : 0);
	sprintf(tree[INFO_VIRT_HRES].ob_spec.tedinfo->te_ptext, "%4d", res->base.virt_hres);
	sprintf(tree[INFO_VIRT_VRES].ob_spec.tedinfo->te_ptext, "%4d", res->base.virt_vres);
	if (do_dialog(tree) == INFO_OK)
	{
		orig_hres = res->base.virt_hres;
		orig_vres = res->base.virt_vres;
		
		res->base.virt_hres = atoi(tree[INFO_VIRT_HRES].ob_spec.tedinfo->te_ptext);
		res->base.virt_vres = atoi(tree[INFO_VIRT_VRES].ob_spec.tedinfo->te_ptext);
		if ((res->base.virt_hres & 15) != 0 || (res->base.virt_vres & 15) != 0)
		{
			res->base.virt_hres &= -16;
			res->base.virt_vres &= -16;
			do_dialog(rs_trindex[ERR_VIRTUAL_RES]);
		}
		if (res->base.virt_hres < res->base.hres)
			res->base.virt_hres = res->base.hres;
		if (res->base.virt_vres < res->base.vres)
			res->base.virt_vres = res->base.vres;
		if (res->planes >= 24)
			linesize = (long)res->base.virt_hres * 4;
		else if (res->planes >= 15)
			linesize = (long)res->base.virt_hres * 2;
		else
			linesize = ((long)res->base.virt_hres * res->planes) >> 3;
		if (linesize > 32700)
		{
			res->base.virt_hres = orig_hres;
			res->base.virt_vres = orig_vres;
			do_dialog(rs_trindex[ERR_RES_TOO_LARGE]);
		} else
		{
			if ((linesize * res->base.virt_vres) > 1048576L) /* FIXME: hard coded */
			{
				res->base.virt_hres = orig_hres;
				res->base.virt_vres = orig_vres;
				do_dialog(rs_trindex[ERR_RES_TOO_LARGE]);
			}
		}
		sprintf(res->base.desc, "%4d * %4d, ET 4000", res->base.virt_hres, res->base.virt_vres);
		if (res->base.hres != res->base.virt_hres || res->base.vres != res->base.virt_vres)
		{
			strcat(res->base.desc, ", ");
			strcat(res->base.desc, rs_string(FS_VIRTUAL));
		}
		res_tab = sort_restab(res_tab);
		set_items();
		redraw_obj(dialog, CHGRES_BOX);
	}
}

static WORD _CDECL hdl_obj(struct HNDL_OBJ_args args)
{
	DIALOG *dlg;
	WORD obj;
	WORD index;
	WORD offset;
	OBJECT *tree;
	GRECT gr;
	short new_bpp_index;
	
	dlg = args.dialog;
	obj = args.obj;
	wdlg_get_tree(dlg, &tree, &gr);

	if (obj < 0)
	{
		if (obj != HNDL_CLSD)
			return 1;
		return 0;
	}
	index = obj;
	if (args.clicks > 1)
		index |= 0x8000;
	if (lbox_do(color_lbox, index) == -1)
		obj = CHGRES_OK;
	switch (obj)
	{
	case CHGRES_BOX_FIRST:
	case CHGRES_BOX_FIRST+1:
	case CHGRES_BOX_FIRST+2:
	case CHGRES_BOX_FIRST+3:
	case CHGRES_BOX_FIRST+4:
	case CHGRES_BOX_FIRST+5:
	case CHGRES_BOX_FIRST+6:
	case CHGRES_BOX_FIRST+7:
	case CHGRES_BOX_FIRST+8:
	case CHGRES_BOX_LAST:
		break;
	case CHGRES_COLORS:
		index = index_to_popup(bpp_index);
		index = simple_popup(tree, CHGRES_COLORS, lbox_names, name_count, index);
		if (index >= 0)
		{
			new_bpp_index = popup_to_index(index);
			if ((new_bpp_index >= 0) && (new_bpp_index != bpp_index))
			{
				bpp_index = new_bpp_index;
				res_tab = possible_resolutions[bpp_index];
				offset = selected_res_index(res_tab);
				if (offset > 9) {
					offset = offset - 9;
				} else {
					offset = 0;
				}
				set_bpp(bpp_index);
				lbox_set_asldr(color_lbox, offset, NULL);
				set_items();
				cur_res = (struct res *)lbox_get_slct_item(color_lbox);
				if (cur_res != NULL)
				{
					tree[CHGRES_OK].ob_state &= ~OS_DISABLED;
					tree[CHGRES_OK].ob_flags |= OF_SELECTABLE;
					tree[CHGRES_OK].ob_flags |= OF_DEFAULT;
					if (cur_res->base.flags & FLAG_INFO)
					{
						tree[CHGRES_INFO].ob_state &= ~OS_DISABLED;
						tree[CHGRES_INFO].ob_flags |= OF_SELECTABLE;
					} else
					{
						tree[CHGRES_INFO].ob_state |= OS_DISABLED;
						tree[CHGRES_INFO].ob_flags &= ~OF_SELECTABLE;
					}
				} else
				{
					tree[CHGRES_OK].ob_state |= OS_DISABLED;
					tree[CHGRES_OK].ob_flags &= ~OF_SELECTABLE;
					tree[CHGRES_OK].ob_flags &= ~OF_DEFAULT;
					tree[CHGRES_INFO].ob_state |= OS_DISABLED;
					tree[CHGRES_INFO].ob_flags &= ~OF_SELECTABLE;
				}
				redraw_obj(dlg, ROOT);
			}
		}
		break;
	case CHGRES_INFO:
		show_info(cur_res);
		tree[CHGRES_INFO].ob_state &= ~OS_SELECTED;
		redraw_obj(dlg, CHGRES_INFO);
		break;
	case CHGRES_OK:
		must_shutdown = 1;
		return 0;
	case CHGRES_CANCEL:
		must_shutdown = 0;
		return 0;
	}
	return 1;
}


static int select_res(void)
{
	OBJECT *tree;
	EVNT events;
	WORD dummy;
	int offset;
	
	tree = rs_trindex[MAIN_DIALOG];
	if (cur_res != NULL)
	{
		tree[CHGRES_OK].ob_state &= ~OS_DISABLED;
		tree[CHGRES_OK].ob_flags |= OF_SELECTABLE;
		tree[CHGRES_OK].ob_flags |= OF_DEFAULT;
		tree[CHGRES_INFO].ob_state &= ~OS_DISABLED;
		tree[CHGRES_INFO].ob_flags |= OF_SELECTABLE;
	} else
	{
		tree[CHGRES_OK].ob_state |= OS_DISABLED;
		tree[CHGRES_OK].ob_flags &= ~OF_SELECTABLE;
		tree[CHGRES_OK].ob_flags &= ~OF_DEFAULT;
		tree[CHGRES_INFO].ob_state |= OS_DISABLED;
		tree[CHGRES_INFO].ob_flags &= ~OF_SELECTABLE;
	}

	/* tree[CHGRES_INFO].ob_flags &= ~OF_HIDETREE; */
	tree[CHGRES_INFO].ob_flags |= OF_HIDETREE;
	tree[CHGRES_NEW].ob_flags |= OF_HIDETREE;

	init_lbox_names();
	set_bpp(bpp_index);

	dialog = wdlg_create(hdl_obj, tree, NULL, 0, NULL, 0);
	if (dialog != NULL)
	{
		translate_restab();
		color_lbox = lbox_create(tree, select_item, set_item, (LBOX_ITEM *)res_tab,
			CHGRES_BOX_LAST-CHGRES_BOX_FIRST+1, 0,
			ctrl_objs, objs,
			LBOX_VERT|LBOX_AUTO|LBOX_AUTOSLCT|LBOX_REAL|LBOX_SNGL, 40,
			dialog, dialog, 0, 0, 0, 0);
		if (color_lbox != NULL)
		{
			offset = selected_res_index(res_tab);
			if (offset > 9) {
				offset = offset - 9;
				lbox_set_asldr(color_lbox, offset, NULL);
				set_items();
			}

			if (wdlg_open(dialog, rs_string(FS_CHANGE_RES), NAME|CLOSER|MOVER, -1, -1, 0, NULL))
			{
				do
				{
					events.mwhich = evnt_multi(MU_KEYBD|MU_BUTTON|MU_MESAG, 2, 1, 1,
						0, 0, 0, 0, 0,
						0, 0, 0, 0, 0,
						events.msg,
						0,
						&events.mx, &events.my, &events.mbutton, &events.kstate, &events.key, &events.mclicks);
				} while (wdlg_evnt(dialog, &events) != 0);
				wdlg_close(dialog, &dummy, &dummy);
			}
			lbox_delete(color_lbox);
		}
		wdlg_delete(dialog);
	}
	return must_shutdown;
}

static long get_cookie(long id)
{
	long *jar = *COOKIE_P;
	if (jar != 0) {
		while (jar[0] != 0) {
			if (jar[0] == id)
				return jar[1];
			jar += 2;
		}
	}
	return 0;
}

struct res* find_mode(short idx, short width, short height, short planes)
{
	struct res* r = possible_resolutions[idx];
	while (r)
	{
		if ((r->base.virt_hres == width) &&
			(r->base.virt_vres == height) &&
			(r->planes == planes))
		{
			return r;
		}
		r = r->base.next;
	}
	return NULL;
}

void add_mode(short idx, struct res* r)
{
	r->base.next = possible_resolutions[idx];
	possible_resolutions[idx] = r;
}

void remove_mode(struct res* r)
{
	struct res* n = r->base.next;
	for (int i=0; i<MAX_DEPTHS; i++) {
		struct res* p = possible_resolutions[i];
		while (r) {
			if (p->base.next == r)
				p->base.next = n;
			p = r->base.next;
		}
		if (r == possible_resolutions[i]) {
			possible_resolutions[i] = n;
		}
		free(r);
	}
}

void remove_modes(short idx)
{
	struct res* n = NULL;
	struct res* r = possible_resolutions[idx];
	while (r)
	{
		n = r->base.next;
		free(r);
		r = n;
	}
	possible_resolutions[idx] = r;
}

struct res* create_mode(short idx, short width, short height, short planes, short rez, short vmode, char* desc)
{
	struct res *res = (struct res *)malloc(sizeof(*res));
	if (res)
	{
		res->base.selected = FALSE;
		res->base.desc = res->descbuf;
		res->base.vmode = vmode;
		res->base.flags = FLAG_INFO;
		res->planes = planes;
		res->base.rez = rez;
		res->base.virt_hres = width;
		res->base.virt_vres = height;
		res->base.hres = width;
		res->base.vres = height;
		res->freq = 60;	/* todo */
		res->mode_offset = 0;
		sprintf(res->base.desc, "%4d x%4d", res->base.virt_hres, res->base.virt_vres);
		if (desc)
		{
			strcat(res->base.desc, ", ");
			strcat(res->base.desc, desc);
		}
		if (res->base.hres != res->base.virt_hres || res->base.vres != res->base.virt_vres)
		{
			strcat(res->base.desc, ", ");
			strcat(res->base.desc, rs_string(FS_VIRTUAL));
		}
		add_mode(idx, res);
	}
	return res;
}

#if 0
static int is_rez_assigned(int rez)
{
	int i;
	for (i = 0; i < NUM_ET4000; i++)
		if (rez == et4000_driver_ids[i])
			return TRUE;
	return FALSE;
}

static void create_vgainf_mode(struct vgainf *inf, struct vgainf_mode *mode, short modeidx, short idx)
{
	struct res *res;
	
	res = (struct res *)malloc(sizeof(*res));
	if (res == NULL)
		return;
	res->base.selected = FALSE;
	res->base.desc = res->descbuf;
	res->base.vmode = modeidx + SVEXT;
	res->base.flags = FLAG_INFO;
	res->planes = et4000_planes[idx];
	res->base.rez = et4000_driver_ids[idx];
	res->base.virt_hres = mode->vga_xres + 1;
	res->base.virt_vres = mode->vga_yres + 1;
	res->base.hres = mode->vga_visible_xres + 1;
	res->base.vres = mode->vga_visible_yres + 1;
	res->freq = mode->vga_vfreq / 10;
	res->mode_offset = (long)mode - (long)inf;
	if (modeidx == inf->vgainf_defmode[idx])
		res->base.flags |= FLAG_DEFMODE;
	sprintf(res->base.desc, "%4d * %4d, ET 4000", res->base.virt_hres, res->base.virt_vres);
	if (res->base.hres != res->base.virt_hres || res->base.vres != res->base.virt_vres)
	{
		strcat(res->base.desc, ", ");
		strcat(res->base.desc, rs_frstr[FS_VIRTUAL]);
	}
	res->base.next = vgainf_modes[idx];
	vgainf_modes[idx] = res;
}

#endif


static void init_res(void)
{
	int i;
	short currez = 0;
	short vmode = 0;

#if 0
	read_assign_sys(assign_sys);
	load_nvdivga_inf();
#endif	

	read_assign_sys();

	for (i = 0; i < MAX_DEPTHS; i++)
		possible_resolutions[i] = NULL;

	bpp_index = BPS1;
	res_tab = NULL;
	switch (vdo)
	{
	case 0:
	case 1:
		initres_st(&currez, &vmode);
		break;
	case 2:
		initres_tt(&currez, &vmode);
		break;
	case 3:
		initres_falc(&currez, &vmode);
		break;
	case 6:
		initres_saga(&currez, &vmode);
		break;
	default:
		break;
	}

	initres_nvdivga(&currez, &vmode);

#if 0	
	for (i = 0; i < MAX_DEPTHS; i++)
	{
		struct res **respp;
		struct res *res;
		int j;

		/*
		 * remove resolutions that are in use by graphic card drivers
		 */
		respp = &possible_resolutions[i];
		while (*respp != NULL)
		{
			res = *respp;
			for (j = 0; j < NUM_ET4000; j++)
			{
				if (res->base.rez == et4000_driver_ids[j])
					*respp = res->base.next;
			}
			respp = &res->base.next;
		}
		
		/*
		 * add graphic card resolutions to the end of the table
		 */
		for (j = 0; j < NUM_ET4000; j++)
		{
			if (valid_planes[i] == et4000_planes[j])
			{
				respp = &possible_resolutions[i];
				while (*respp != NULL)
					respp = &(*respp)->base.next;
				*respp = vgainf_modes[j];
				break;
			}
		}
	}

	if (is_rez_assigned(currez))
	{
		struct res *res;
		int j;

		for (j = 0; j < MAX_DEPTHS; j++)
		{
			res = possible_resolutions[j];
			while (res != NULL)
			{
				if (res->base.rez == currez && (res->base.flags & FLAG_DEFMODE))
				{
					vmode = res->base.vmode;
					break;
				}
				res = res->base.next;
			}
		}
	}
#endif
	
	/* default to best available resolution */
	for (i = MAX_DEPTHS - 1; i >= 0; i--) {
		if (possible_resolutions[i] != NULL) {
			possible_resolutions[i] = sort_restab(possible_resolutions[i]);
			res_tab = possible_resolutions[i];
			bpp_index = i;
			break;
		}
	}

	/* try to find and select current resolution */
	for (i = 0; i < MAX_DEPTHS; i++)
	{
		struct res *res;
		if (possible_resolutions[i] != NULL)
		{
			possible_resolutions[i] = sort_restab(possible_resolutions[i]);
			res = possible_resolutions[i];
			while (res != NULL)
			{
				if (res->base.rez == currez && res->base.vmode == vmode)
				{
					cur_res = res;
					res->base.selected = TRUE;
					bpp_index = i;
					res_tab = possible_resolutions[i];
					break;
				}
				res = res->base.next;
			}
		}
	}
}

static void fix_rsc(void)
{
	OBJECT *tree;
	RSHDR *rs_hdr;
	
	rs_hdr = *((void **)&aes_global[7]);
	rs_trindex = (OBJECT **)(((char *)rs_hdr + rs_hdr->rsh_trindex));
	rs_frstr = (char **)(((char *)rs_hdr + rs_hdr->rsh_frstr));
	tree = rs_trindex[MAIN_DIALOG];

	if ((aes_ver < 0x399) || (sys_aes & F_SYSTEM_MINT_MYAES)) {
		tree[CHGRES_UP].ob_spec.obspec.framesize = 1;
		tree[CHGRES_DOWN].ob_spec.obspec.framesize = 1;
		tree[CHGRES_SLIDER].ob_spec.obspec.framesize = 1;
		tree[CHGRES_BACK].ob_spec.obspec.interiorcol = 1;
		tree[CHGRES_BACK].ob_spec.obspec.fillpattern = 1;
		tree[CHGRES_UP].ob_y -= 2;
		tree[CHGRES_DOWN].ob_y += 2;
		tree[CHGRES_BACK].ob_x += 2;
		tree[CHGRES_BACK].ob_width -= 4;
		tree[CHGRES_BACK].ob_y -= 5;
		tree[CHGRES_BACK].ob_height += 10;
		tree[CHGRES_SLIDER].ob_x -= 2;
	}
}


#if 0
static int start_shutdown(struct res *cur)
{
	char buf[40];
	int ret = TRUE;
	int i;
	struct res *res;
	
	for (i = 0; i < MAX_DEPTHS; i++)
	{
		for (res = possible_resolutions[i]; res != NULL; res = res->base.next)
		{
			if ((res->base.flags & FLAG_INFO) && res == cur)
			{
				ret = change_nvdivga_inf(res);
			}
		}
	}
	if (ret)
	{
		buf[0] = (char)sprintf(&buf[1], "%d 0 %d", cur->base.rez, cur->base.vmode);
		shel_write(1, 1, 1, "SHUTDOWN.PRG", buf); /* BUG: return value ignored */
	}
	return ret;
}
#endif

static inline short test_pid(short pid) {
	return (pid > 0) ? ((short)trap_1_www(273,pid,0)) : -1;
}

static inline short term_pid(short pid) {
	return (pid > 0) ? ((short)trap_1_www(273,pid,15)) : -1;
}

static inline short kill_pid(short pid) {
	return (pid > 0) ? ((short)trap_1_www(273,pid,9)) : -1;
}

static inline void shutdown(long mode) {
	trap_1_wl(337,mode);
}

static inline short get_pid(short apid)
{
	if (apid > 0) {
		long val = 0xfffe0000L | apid;
		return appl_find((char*)val);
	}
	return -1;
}

static long getms(void)
{
	long ticks = *((volatile long*)0x4baL);
	return ticks * 5;
}

static void warm_reboot(void)
{
	OSHEADER* hdr = *((OSHEADER **)(0x4f2L));
	hdr->reseth();
}

static int apply(void)
{
	if (sys_vdi & F_VDI_FVDI)
	{
		appl_exit();
		if ((sys_aes & F_SYSTEM_TOS) == 0) {
			shutdown(1);
		}
		Supexec(warm_reboot);
	}

	if (sys_aes & F_SYSTEM_GENEVA)
	{
		return -1;
	}
	else if (sys_aes & F_SYSTEM_MAGIC)
	{
		char buf[40];
		buf[0] = (char)sprintf(&buf[1], "%d 0 %d", cur_res->base.rez, cur_res->base.vmode);
		shel_write(1, 1, 1, "SHUTDOWN.PRG", buf); /* BUG: return value ignored */
		return 1;
	}
	else if ((sys_aes & F_SYSTEM_MINT) /*|| (sys_aes == F_SYSTEM_EMUTOS)*/)	
	{
		/* should in theory work on EmuTOS but in practice it's got issues, at least on V4SA */
		short wisgr;
		short wiscr;
		if (cur_res->base.vmode == 0) {
			wisgr = cur_res->base.rez;
			wiscr = 0;
		} else {
			wisgr = cur_res->base.vmode;
			wiscr = 1;
		}

		long startms = Supexec(getms);
		short retries = 1;
		while (retries > 0)
		{
			short apid = 0;
			if (shel_write(SHW_RESCHNG, wisgr, wiscr, (sys_aes & F_SYSTEM_MINT_NAES) ? &apid : NULL, NULL) > 0)
			{
				/* todo: wait until reset */
				sleep(1);
				return 1;
			}
			retries--;
			if ((Supexec(getms) - startms) < (10 * 1000))
			{
				if (apid > 0)
				{
					short pid = get_pid(apid);
					kill_pid(pid);
					retries++;
				}
			}
		}
	}

	/* worst case, warm reboot when SHW_RESCHNG does not work */
	Supexec(warm_reboot);
	return 1;
}

static int save(void)
{
	/* return values:
		-1 = must save but failed
		 0 = prefer save but failed
		 1 = saved
	*/

	/* fvdi saving */
	if (sys_vdi == F_VDI_FVDI)
		return save_fvdi_inf(cur_res);

	/* aes specific saving */
	switch (sys_aes) {
		case F_SYSTEM_TOS:
			return save_newdesk_inf(cur_res) ? 1 : -1;

		case F_SYSTEM_EMUTOS:
			return save_emudesk_inf(cur_res) ? 1 : -1;

		case F_SYSTEM_GENEVA:
			return -1;

		case F_SYSTEM_MAGIC:
			return save_magx_inf(cur_res) ? 1 : 0;

		case F_SYSTEM_MINT_AES:
			return save_newdesk_inf(cur_res) ? 1 : 0;

		case F_SYSTEM_MINT_NAES:
			return save_naes_inf(cur_res) ? 1 : 0;

		case F_SYSTEM_MINT_XAAES:
			return save_xaaes_inf(cur_res) ? 1 : -1;

		case F_SYSTEM_MINT_MYAES:
			return save_myaes_inf(cur_res) ? 1 : 0;

		default:
			return -1;
	}
}

static void setup(void)
{
	/* general cookies */
	vdo = (WORD)(get_cookie('_VDO') >> 16);
	cpu = ((WORD)(get_cookie('_CPU'))) / 10;
	mch = (WORD)(get_cookie('_MCH') >> 16);

	/* tos version */
	OSHEADER* hdr = *((OSHEADER **)(0x4f2L));
	tos_emutos = 0;
	tos_version = hdr->os_version;
	if (*((unsigned long*)(((unsigned long)hdr) + 44)) == 'ETOS') {
		tos_emutos = tos_version;
	}

	/* we cannot rely on cookies for AES because some of them have options to masquerade */
	aes_ver = aes_global[0];
	sys_aes = tos_emutos ? F_SYSTEM_EMUTOS : F_SYSTEM_TOS;
	if (get_cookie('Gnva'))
	{
		sys_aes = F_SYSTEM_GENEVA;
	}
	else if (get_cookie('MiNT'))
	{
		sys_aes = F_SYSTEM_MINT_AES;
		if (get_process_id("myaes") > 0) {
			sys_aes = F_SYSTEM_MINT_MYAES;
		} else if (get_process_id("XaSYS") > 0) {
			sys_aes = F_SYSTEM_MINT_XAAES;
		} else {
			short* naes_hdr = (short*)get_cookie('nAES');
			if(naes_hdr && ((naes_hdr[3] & (1 << 15)) == 0)) {
				sys_aes = F_SYSTEM_MINT_NAES;
			}
		}
	}
	else if (get_cookie('MagX'))
	{
		sys_aes = F_SYSTEM_MAGIC;
	}

	/* special case to deal with EmuTOS + AES4.x */
	if ((sys_aes == F_SYSTEM_EMUTOS) && (aes_ver >= 0x400))
		sys_aes = F_SYSTEM_TOS;

	// vdi
	sys_vdi = F_VDI_ATARI;
	if (get_cookie('fVDI')) {
		sys_vdi = F_VDI_FVDI;
	} else if (get_cookie('NVDI')) {
		sys_vdi = F_VDI_NVDI;
	} else {
		sys_vdi = F_VDI_ATARI;
	}

	/* make sure vampire gets saga modes */
	if ((vdo == 3) && (mch == 6)) {
		vdo = 6;
	}

#if 0
	switch (sys_aes)
	{
		case F_SYSTEM_TOS:			printf("sys: TOS\n\r"); break;
		case F_SYSTEM_EMUTOS:		printf("sys: EmuTOS\n\r"); break;
		case F_SYSTEM_GENEVA:		printf("sys: Geneva\n\r"); break;
		case F_SYSTEM_MAGIC:		printf("sys: MagiC\n\r"); break;
		case F_SYSTEM_MINT_AES:		printf("sys: MiNT\n\r"); break;
		case F_SYSTEM_MINT_NAES:	printf("sys: MiNT + N.AES\n\r"); break;
		case F_SYSTEM_MINT_XAAES:	printf("sys: MiNT + XaAES\n\r"); break;
		case F_SYSTEM_MINT_MYAES:	printf("sys: MiNT + MyAES\n\r"); break;
		default:					printf("sys: unknown\n\r"); break;
	}
	printf("vdo   = %04x\n\r", vdo);
	printf("mch   = %04x\n\r", mch);
	printf("cpu   = %04x\n\r", cpu);
	printf("vdi   = %04x\n\r", sys_vdi);
	printf("sys   = %04x\n\r", sys_aes);
	printf("tos   = %04x\n\r", tos_version);
	printf("etos  = %04x\n\r", tos_emutos);
	printf("aes   = %04x\n\r", aes_ver);
#endif
}

int main(void)
{
	int ret = -1;
	gl_apid = appl_init();
	if (gl_apid != -1)
	{
		aes_handle = graf_handle(&gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox);
		Supexec(setup);
		graf_mouse(ARROW, NULL);
		if (rsrc_load("CHGRES.RSC") != 0)
		{
			fix_rsc();
			init_res();
			if (select_res())
			{
				if (save() < 0)
				{
					printf("save failed\n\r");
					do_dialog(rs_trindex[ERR_RESCHG]);
				}
				else
				{
					if (apply() < 0)
					{
						printf("apply failed\n\r");
						do_dialog(rs_trindex[ERR_RESCHG]);	
					}
				}
			}
			rsrc_free();
			ret = 0;
		}
		appl_exit();
	}

	return ret;
}

