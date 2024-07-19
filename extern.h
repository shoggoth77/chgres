#ifndef _EXTERN_H_
#define _EXTERN_H_


#ifndef WORD
#ifdef __GNUC__
#define WORD short
#endif
#endif
#ifndef _CDECL
#ifdef __GNUC__
#define _CDECL
#endif
#endif

#ifndef FALSE
#  define FALSE 0
#  define TRUE 1
#endif

#ifndef OS_SELECTED
#define OS_SELECTED SELECTED
#define OS_DISABLED DISABLED
#define OS_SHADOWED SHADOWED
#define OS_NORMAL NORMAL
#define OS_CHECKED CHECKED
#define OF_SELECTABLE SELECTABLE
#define OF_DEFAULT DEFAULT
#define OF_HIDETREE HIDETREE
#define OF_FL3DBAK FL3DBAK
#define OF_LASTOB LASTOB
#define OF_NONE   NONE
#endif

extern WORD gl_wchar;
extern WORD gl_hchar;
extern WORD gl_wbox;
extern WORD gl_hbox;

struct resbase {
	struct res *next;
	short selected;
	/* ^^^ above must be same as in LBOX_ITEM */
	char *desc;
	short rez;
	short vmode;
	short flags;
#define FLAG_INFO     0x0001
#define FLAG_DEFMODE  0x0002
	short virt_hres;
	short virt_vres;
	short hres;
	short vres;
};

struct res {
	struct resbase base;
	short planes;
	short freq;
	long mode_offset;
	char descbuf[80];
};

#define MAX_DEPTHS 8
#define NUM_ET4000 6

#define REZ_ST_LOW		(0 + 2)
#define REZ_ST_MED		(1 + 2)
#define REZ_ST_HIGH		(2 + 2)
#define REZ_FALCON		(3 + 2)
#define REZ_TT_MED		(4 + 2)
#define REZ_TT_HIGH		(6 + 2)
#define REZ_TT_LOW		(7 + 2)

#ifndef MON_MONO
#define MON_MONO		0
#define MON_COLOR		1
#define MON_VGA			2
#define MON_TV			3
#endif
#ifndef VGA
#define VGA 0x10
#endif
#ifndef PAL
#define PAL 0x20
#endif
#ifndef NUMCOLS
#define NUMCOLS 7
#endif
#ifndef SVEXT
#define SVEXT 0x4000
#endif

#define F_VDI_ATARI				(1 << 0)
#define F_VDI_NVDI				(1 << 1)
#define F_VDI_NVDIVGA			(1 << 2)
#define F_VDI_FVDI				(1 << 3)

#define F_SYSTEM_TOS			(1 << 0)
#define F_SYSTEM_EMUTOS			((1 << 1) | F_SYSTEM_TOS)
#define F_SYSTEM_MAGIC			(1 << 2)
#define F_SYSTEM_GENEVA			(1 << 3)

#define F_SYSTEM_MINT_AES		(1 << 4)
#define F_SYSTEM_MINT_NAES		(1 << 5)
#define F_SYSTEM_MINT_XAAES		(1 << 6)
#define F_SYSTEM_MINT_MYAES		(1 << 7)
#define F_SYSTEM_MINT			(F_SYSTEM_MINT_AES | F_SYSTEM_MINT_NAES | F_SYSTEM_MINT_XAAES | F_SYSTEM_MINT_MYAES)

extern WORD sys_aes;
extern WORD sys_vdi;

extern WORD tos_version;
extern WORD tos_emutos;
extern WORD aes_ver;
extern char* aes_dir;

/* utils.h */
short get_process_id(const char* processname);
char* get_process_path(const char* processname);
char* load_file(const char *filename, long *size);
char* find_line(char *p, const char *str, long size, long *remain, long *linesize);
long next_line(char *s, long maxlen, long *valid);
char* skip_white(char* str);
char *remove_line(char *p, const char *str, long size, long *newsize);

/* chgres.c */
char* get_system_drive(void);
char* rs_string(long stringid);
short get_process_id(const char* processname);
char* get_process_path(const char* processname);
WORD simple_popup(OBJECT *tree, WORD obj, const char*const *names, WORD num_names, WORD selected);

/* enum.c */
short enumerate();
short enumerate_st(short* outrez, short* outmode);
short enumerate_tt(short* outrez, short* outmode);
short enumerate_falc(short* outrez, short* outmode);
short enumerate_saga(short* outrez, short* outmode);
short enumerate_nvdivga(short* outrez, short* outmode);

/* save.c */
short save_fvdi(struct res* r);
short save_nvdivga(struct res* r);

short save_magx(struct res* r);
short save_emudesk(struct res* r);		/* emudesk.inf */
short save_newdesk(struct res* r);		/* newdesk.inf */
short save_olddesk(struct res* r);		/* desktop.inf */
short save_naes(struct res* r);			/* n_desk.inf  */
short save_xaaes(struct res* r);		/* xaaes.cnf   */
short save_myaes(struct res* r);		/* myaes.cnf   */

/* chgres.c */
struct res* find_mode(short idx, short width, short height, short planes);
struct res* create_mode(short idx, short width, short height, short planes, short rez, short vmode, char* desc);
void add_mode(short idx, struct res* r);
void remove_modes(short idx);

#endif /* _EXTERN_H_ */
