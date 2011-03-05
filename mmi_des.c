//#include "simat_include.h"
#include "sci_types.h"
#include "sim_macro_switch.h"
//#include "ctypes.h"
//#include "gsm_gprs.h"
//#include "sim_file_structure.h"
#include "sim_dphone_stk_verify.h"
//#include "nvitem.h"
//#include "mmi_nv.h"
//#include  "mmi_common.h"
//#include "mmiutil.h"
#include "mn_api.h"
//#include "mmiphone.h"
#include "mmi_descontrol.h"

#define D2_DES		/* include double-length support */
#define D3_DES		/* include triple-length support */

#define EN0	0	/* MODE == encrypt */
#define DE1	1	/* MODE == decrypt */

void deskey(unsigned char *, short);
/*		      hexkey[8]     MODE
 * Sets the internal key register according to the hexadecimal
 * key contained in the 8 bytes of hexkey, according to the DES,
 * for encryption or decryption according to MODE.
 */

void usekey(unsigned long *);
/*		    cookedkey[32]
 * Loads the internal key register with the data in cookedkey.
 */

void cpkey(unsigned long *);
/*		   cookedkey[32]
 * Copies the contents of the internal key register into the storage
 * located at &cookedkey[0].
 */

void des(unsigned char *, unsigned char *);
/*		    from[8]	      to[8]
 * Encrypts/Decrypts (according to the key currently loaded in the
 * internal key register) one block of eight bytes at address 'from'
 * into the block at address 'to'.  They can be the same.
 */

#ifdef D2_DES

#define desDkey(a,b)	des2key((a),(b))
void des2key(unsigned char *, short);
/*		      hexkey[16]     MODE
 * Sets the internal key registerS according to the hexadecimal
 * keyS contained in the 16 bytes of hexkey, according to the DES,
 * for DOUBLE encryption or decryption according to MODE.
 * NOTE: this clobbers all three key registers!
 */

void Ddes(unsigned char *, unsigned char *);
/*		    from[8]	      to[8]
 * Encrypts/Decrypts (according to the keyS currently loaded in the
 * internal key registerS) one block of eight bytes at address 'from'
 * into the block at address 'to'.  They can be the same.
 */

void D2des(unsigned char *, unsigned char *);
/*		    from[16]	      to[16]
 * Encrypts/Decrypts (according to the keyS currently loaded in the
 * internal key registerS) one block of SIXTEEN bytes at address 'from'
 * into the block at address 'to'.  They can be the same.
 */

void makekey(char *, unsigned char *);
/*		*password,	single-length key[8]
 * With a double-length default key, this routine hashes a NULL-terminated
 * string into an eight-byte random-looking key, suitable for use with the
 * deskey() routine.
 */

#define makeDkey(a,b)	make2key((a),(b))
void make2key(char *, unsigned char *);
/*		*password,	double-length key[16]
 * With a double-length default key, this routine hashes a NULL-terminated
 * string into a sixteen-byte random-looking key, suitable for use with the
 * des2key() routine.
 */

#ifndef D3_DES	/* D2_DES only */

#define useDkey(a)	use2key((a))
#define cpDkey(a)	cp2key((a))

void use2key(unsigned long *);
/*		    cookedkey[64]
 * Loads the internal key registerS with the data in cookedkey.
 * NOTE: this clobbers all three key registers!
 */

void cp2key(unsigned long *);
/*		   cookedkey[64]
 * Copies the contents of the internal key registerS into the storage
 * located at &cookedkey[0].
 */

#else	/* D3_DES too */

#define useDkey(a)	use3key((a))
#define cpDkey(a)	cp3key((a))

void des3key(unsigned char *, short);
/*		      hexkey[24]     MODE
 * Sets the internal key registerS according to the hexadecimal
 * keyS contained in the 24 bytes of hexkey, according to the DES,
 * for DOUBLE encryption or decryption according to MODE.
 */

void use3key(unsigned long *);
/*		    cookedkey[96]
 * Loads the 3 internal key registerS with the data in cookedkey.
 */

void cp3key(unsigned long *);
/*		   cookedkey[96]
 * Copies the contents of the 3 internal key registerS into the storage
 * located at &cookedkey[0].
 */

void make3key(char *, unsigned char *);
/*		*password,	triple-length key[24]
 * With a triple-length default key, this routine hashes a NULL-terminated
 * string into a twenty-four-byte random-looking key, suitable for use with
 * the des3key() routine.
 */
 #endif	/* D3_DES */
#endif	/* D2_DES */


static void scrunch(unsigned char *, unsigned long *);
static void unscrun(unsigned long *, unsigned char *);
static void desfunc(unsigned long *, unsigned long *);
static void cookey(unsigned long *);
static void my_cookey(unsigned long *,unsigned long *);

static unsigned long KnL[32] = { 0L };
static unsigned long KnR[32] = { 0L };
static unsigned long Kn3[32] = { 0L };
static unsigned char Df_Key[24] = {
	0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,
	0xfe,0xdc,0xba,0x98,0x76,0x54,0x32,0x10,
	0x89,0xab,0xcd,0xef,0x01,0x23,0x45,0x67 };

static unsigned short bytebit[8]	= {
	0200, 0100, 040, 020, 010, 04, 02, 01 };

static unsigned long bigbyte[24] = {
	0x800000L,	0x400000L,	0x200000L,	0x100000L,
	0x80000L,	0x40000L,	0x20000L,	0x10000L,
	0x8000L,	0x4000L,	0x2000L,	0x1000L,
	0x800L, 	0x400L, 	0x200L, 	0x100L,
	0x80L,		0x40L,		0x20L,		0x10L,
	0x8L,		0x4L,		0x2L,		0x1L	};

/* Use the key schedule specified in the Standard (ANSI X3.92-1981). */

static unsigned char pc1[56] = {
	56, 48, 40, 32, 24, 16,  8,	 0, 57, 49, 41, 33, 25, 17,
	 9,  1, 58, 50, 42, 34, 26,	18, 10,  2, 59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14,	 6, 61, 53, 45, 37, 29, 21,
	13,  5, 60, 52, 44, 36, 28,	20, 12,  4, 27, 19, 11,  3 };

static unsigned char totrot[16] = {
	1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28 };

static unsigned char pc2[48] = {
	13, 16, 10, 23,  0,  4,  2, 27, 14,  5, 20,  9,
	22, 18, 11,  3, 25,  7, 15,  6, 26, 19, 12,  1,
	40, 51, 30, 36, 46, 54, 29, 39, 50, 44, 32, 47,
	43, 48, 38, 55, 33, 52, 45, 41, 49, 35, 28, 31 };


const uint8 default_woqi_rand_tag_str[DPHONE_WOQI_RAND_TAG_NUM] = {0x52,0x41,0x4e,0x44};
const uint8 default_woqi_data_tag_str[DPHONE_WOQI_RAND_TAG_NUM] = {0x44,0x41,0x54,0x41};
uint8 g_des_str_buf[DPHONE_DES_RAND_NUM] = {0};

BOOLEAN g_is_process_des=FALSE;
/********************************************************************
//    DESCRIPTION
//    set the flag to judge whether it is a des sim card
//    Global resource dependence :
//    Author: yuhua.shi
//    Note:
***********************************************************************/
void MMI_SetIsDesSimcardFlag(BOOLEAN flag);

/********************************************************************
//    DESCRIPTION
//    get the flag to judge whether it is a des sim card
//    Global resource dependence :
//    Author:
//    Note: yuhua.shi
***********************************************************************/
BOOLEAN MMI_GetIsDesSimcardFlag(void);

/********************************************************************
//    DESCRIPTION
//    read the imei num from nv
//    Global resource dependence :
//    Author:
//    Note:
***********************************************************************/
uint8 MMI_DphoneGetImeiNumber(void);

/********************************************************************
//    DESCRIPTION
//    read the type of the des sim card from nv
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
DPHONE_DES_SIMCARD_TYPE_E MMI_GetDesSimCardType(void);

/********************************************************************
//    DESCRIPTION
//    read the flag to judge whether it is allowed to use the normal sim 
//    card from nv
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
BOOLEAN MMI_GetNormalCardAllowedFlag(void);
/********************************************************************
//    DESCRIPTION
//    read the flag to judge whether it has preocessed the des procedure
//    card from nv
//    Global resource dependence :
//    Author:alexye
//    Note:
***********************************************************************/
BOOLEAN MMI_Des_IsProcessDes(void);
/********************************************************************
//    DESCRIPTION
//    set the flag to judge whether it has preocessed the des procedure
//    card from nv
//    Global resource dependence :
//    Author:alexye
//    Note:
***********************************************************************/
void MMI_Des_SetProcessDes(BOOLEAN is_process);

/********************************************************************
//    DESCRIPTION
//    switch the related des algorithm to produce the cryptograph result 
//    for stk task
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
void MMI_DphoneDes(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult);

/********************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Datang V3 Jiangsu card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
static void DesDatangV2JiangsuCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult);

/********************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Datang V2.1 Liaoning card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
static void DesDatangV21LiaoningCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult);

/********************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Datang V3 Jiangsu card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
static void DesDatangV3JiangsuCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult);

/********************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Woqi card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
static void DesWoqiCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult);

/**********************************************************************************
//    DESCRIPTION
//    des interface for ps project about get input proactive command to mmi
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************************/
BOOLEAN MMI_ProcessDesSimGetInput(SIMAT_GET_INPUT_T* com, SIMAT_DO_TEXT_STRING_T* temp_text_str);


//the different des process function for different des card
static void (*s_des_simcard_process[])(DPHONE_DES_PARAMETER_T* , DPHONE_DES_RESULT_T *) = 
{
	DesDatangV2JiangsuCard,
	DesDatangV21LiaoningCard,
	DesDatangV3JiangsuCard,
	DesWoqiCard
	
	//add the new card function here
};

static BOOLEAN s_is_des_simcard_flag = FALSE;

uint8 km[DPHONE_DES_KEY_ARRAY_NUM][DPHONE_DES_KEY_NUM] = 
{
	{0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x11 , 0x11 , 0x11 , 0x11 , 0x11 , 0x11 , 0x11, 0x11, 0x11 , 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x22 , 0x22 , 0x22 , 0x22 , 0x22 , 0x22 , 0x22, 0x22, 0x22 , 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22},
	{0x33 , 0x33 , 0x33 , 0x33 , 0x33 , 0x33 , 0x33, 0x33, 0x33 , 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33},
	{0x44 , 0x44 , 0x44 , 0x44 , 0x44 , 0x44 , 0x44, 0x44, 0x44 , 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44}
};



//用法：调用deskey设置密钥,key为64位密钥，edf位模式选择
//		edf=DE1，解密模式；edf=EN0，加密模式
//		调用des加密/解密数据
//		inblock是输入的明文，outblock是输出的密文
//如果需要用更长的密钥，如2des或者3des模式的话，可以用下面的		
//des2key 和 Ddes 之类的函数
void deskey(unsigned char *key, short edf)	/* Thanks to James Gillogly & Phil Karn! */
{
	register int i, j, l, m, n;
	unsigned char pc1m[56], pcr[56];
	unsigned long kn[32];

	for ( j = 0; j < 56; j++ ) {
		l = pc1[j];
		m = l & 07;
		pc1m[j] = (key[l >> 3] & bytebit[m]) ? 1 : 0;
		}
	for( i = 0; i < 16; i++ ) {
		if( edf == DE1 ) m = (15 - i) << 1;
		else m = i << 1;
		n = m + 1;
		kn[m] = kn[n] = 0L;
		for( j = 0; j < 28; j++ ) {
			l = j + totrot[i];
			if( l < 28 ) pcr[j] = pc1m[l];
			else pcr[j] = pc1m[l - 28];
			}
		for( j = 28; j < 56; j++ ) {
		    l = j + totrot[i];
		    if( l < 56 ) pcr[j] = pc1m[l];
		    else pcr[j] = pc1m[l - 28];
		    }
		for( j = 0; j < 24; j++ ) {
			if( pcr[pc2[j]] ) kn[m] |= bigbyte[j];
			if( pcr[pc2[j+24]] ) kn[n] |= bigbyte[j];
			}
		}
	cookey(kn);
	return;
	}

static void cookey(register unsigned long *raw1)
{
	register unsigned long *cook, *raw0;
	unsigned long dough[32];
	register int i;

	cook = dough;
	for( i = 0; i < 16; i++, raw1++ ) {
		raw0 = raw1++;
		*cook	 = (*raw0 & 0x00fc0000L) << 6;
		*cook	|= (*raw0 & 0x00000fc0L) << 10;
		*cook	|= (*raw1 & 0x00fc0000L) >> 10;
		*cook++ |= (*raw1 & 0x00000fc0L) >> 6;
		*cook	 = (*raw0 & 0x0003f000L) << 12;
		*cook	|= (*raw0 & 0x0000003fL) << 16;
		*cook	|= (*raw1 & 0x0003f000L) >> 4;
		*cook++ |= (*raw1 & 0x0000003fL);
		}
	usekey(dough);
	return;
	}

void cpkey(register unsigned long *into)
{
	register unsigned long *from, *endp;

	from = KnL, endp = &KnL[32];
	while( from < endp ) *into++ = *from++;
	return;
	}

void usekey(register unsigned long *from)
{
	register unsigned long *to, *endp;

	to = KnL, endp = &KnL[32];
	while( to < endp ) *to++ = *from++;
	return;
	}

void des(unsigned char * inblock, unsigned char * outblock)
{
	unsigned long work[2];

	scrunch(inblock, work);
	desfunc(work, KnL);
	unscrun(work, outblock);
	return;
	}

static void scrunch(register unsigned char *outof, register unsigned long *into)
{
	*into	 = (*outof++ & 0xffL) << 24;
	*into	|= (*outof++ & 0xffL) << 16;
	*into	|= (*outof++ & 0xffL) << 8;
	*into++ |= (*outof++ & 0xffL);
	*into	 = (*outof++ & 0xffL) << 24;
	*into	|= (*outof++ & 0xffL) << 16;
	*into	|= (*outof++ & 0xffL) << 8;
	*into	|= (*outof   & 0xffL);
	return;
	}

static void unscrun(register unsigned long *outof, register unsigned char *into)
{
	*into++ = (*outof >> 24) & 0xffL;
	*into++ = (*outof >> 16) & 0xffL;
	*into++ = (*outof >>  8) & 0xffL;
	*into++ =  *outof++	 & 0xffL;
	*into++ = (*outof >> 24) & 0xffL;
	*into++ = (*outof >> 16) & 0xffL;
	*into++ = (*outof >>  8) & 0xffL;
	*into	=  *outof	 & 0xffL;
	return;
	}

static unsigned long SP1[64] = {
	0x01010400L, 0x00000000L, 0x00010000L, 0x01010404L,
	0x01010004L, 0x00010404L, 0x00000004L, 0x00010000L,
	0x00000400L, 0x01010400L, 0x01010404L, 0x00000400L,
	0x01000404L, 0x01010004L, 0x01000000L, 0x00000004L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00010400L,
	0x00010400L, 0x01010000L, 0x01010000L, 0x01000404L,
	0x00010004L, 0x01000004L, 0x01000004L, 0x00010004L,
	0x00000000L, 0x00000404L, 0x00010404L, 0x01000000L,
	0x00010000L, 0x01010404L, 0x00000004L, 0x01010000L,
	0x01010400L, 0x01000000L, 0x01000000L, 0x00000400L,
	0x01010004L, 0x00010000L, 0x00010400L, 0x01000004L,
	0x00000400L, 0x00000004L, 0x01000404L, 0x00010404L,
	0x01010404L, 0x00010004L, 0x01010000L, 0x01000404L,
	0x01000004L, 0x00000404L, 0x00010404L, 0x01010400L,
	0x00000404L, 0x01000400L, 0x01000400L, 0x00000000L,
	0x00010004L, 0x00010400L, 0x00000000L, 0x01010004L };

static unsigned long SP2[64] = {
	0x80108020L, 0x80008000L, 0x00008000L, 0x00108020L,
	0x00100000L, 0x00000020L, 0x80100020L, 0x80008020L,
	0x80000020L, 0x80108020L, 0x80108000L, 0x80000000L,
	0x80008000L, 0x00100000L, 0x00000020L, 0x80100020L,
	0x00108000L, 0x00100020L, 0x80008020L, 0x00000000L,
	0x80000000L, 0x00008000L, 0x00108020L, 0x80100000L,
	0x00100020L, 0x80000020L, 0x00000000L, 0x00108000L,
	0x00008020L, 0x80108000L, 0x80100000L, 0x00008020L,
	0x00000000L, 0x00108020L, 0x80100020L, 0x00100000L,
	0x80008020L, 0x80100000L, 0x80108000L, 0x00008000L,
	0x80100000L, 0x80008000L, 0x00000020L, 0x80108020L,
	0x00108020L, 0x00000020L, 0x00008000L, 0x80000000L,
	0x00008020L, 0x80108000L, 0x00100000L, 0x80000020L,
	0x00100020L, 0x80008020L, 0x80000020L, 0x00100020L,
	0x00108000L, 0x00000000L, 0x80008000L, 0x00008020L,
	0x80000000L, 0x80100020L, 0x80108020L, 0x00108000L };

static unsigned long SP3[64] = {
	0x00000208L, 0x08020200L, 0x00000000L, 0x08020008L,
	0x08000200L, 0x00000000L, 0x00020208L, 0x08000200L,
	0x00020008L, 0x08000008L, 0x08000008L, 0x00020000L,
	0x08020208L, 0x00020008L, 0x08020000L, 0x00000208L,
	0x08000000L, 0x00000008L, 0x08020200L, 0x00000200L,
	0x00020200L, 0x08020000L, 0x08020008L, 0x00020208L,
	0x08000208L, 0x00020200L, 0x00020000L, 0x08000208L,
	0x00000008L, 0x08020208L, 0x00000200L, 0x08000000L,
	0x08020200L, 0x08000000L, 0x00020008L, 0x00000208L,
	0x00020000L, 0x08020200L, 0x08000200L, 0x00000000L,
	0x00000200L, 0x00020008L, 0x08020208L, 0x08000200L,
	0x08000008L, 0x00000200L, 0x00000000L, 0x08020008L,
	0x08000208L, 0x00020000L, 0x08000000L, 0x08020208L,
	0x00000008L, 0x00020208L, 0x00020200L, 0x08000008L,
	0x08020000L, 0x08000208L, 0x00000208L, 0x08020000L,
	0x00020208L, 0x00000008L, 0x08020008L, 0x00020200L };

static unsigned long SP4[64] = {
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802080L, 0x00800081L, 0x00800001L, 0x00002001L,
	0x00000000L, 0x00802000L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00800080L, 0x00800001L,
	0x00000001L, 0x00002000L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002001L, 0x00002080L,
	0x00800081L, 0x00000001L, 0x00002080L, 0x00800080L,
	0x00002000L, 0x00802080L, 0x00802081L, 0x00000081L,
	0x00800080L, 0x00800001L, 0x00802000L, 0x00802081L,
	0x00000081L, 0x00000000L, 0x00000000L, 0x00802000L,
	0x00002080L, 0x00800080L, 0x00800081L, 0x00000001L,
	0x00802001L, 0x00002081L, 0x00002081L, 0x00000080L,
	0x00802081L, 0x00000081L, 0x00000001L, 0x00002000L,
	0x00800001L, 0x00002001L, 0x00802080L, 0x00800081L,
	0x00002001L, 0x00002080L, 0x00800000L, 0x00802001L,
	0x00000080L, 0x00800000L, 0x00002000L, 0x00802080L };

static unsigned long SP5[64] = {
	0x00000100L, 0x02080100L, 0x02080000L, 0x42000100L,
	0x00080000L, 0x00000100L, 0x40000000L, 0x02080000L,
	0x40080100L, 0x00080000L, 0x02000100L, 0x40080100L,
	0x42000100L, 0x42080000L, 0x00080100L, 0x40000000L,
	0x02000000L, 0x40080000L, 0x40080000L, 0x00000000L,
	0x40000100L, 0x42080100L, 0x42080100L, 0x02000100L,
	0x42080000L, 0x40000100L, 0x00000000L, 0x42000000L,
	0x02080100L, 0x02000000L, 0x42000000L, 0x00080100L,
	0x00080000L, 0x42000100L, 0x00000100L, 0x02000000L,
	0x40000000L, 0x02080000L, 0x42000100L, 0x40080100L,
	0x02000100L, 0x40000000L, 0x42080000L, 0x02080100L,
	0x40080100L, 0x00000100L, 0x02000000L, 0x42080000L,
	0x42080100L, 0x00080100L, 0x42000000L, 0x42080100L,
	0x02080000L, 0x00000000L, 0x40080000L, 0x42000000L,
	0x00080100L, 0x02000100L, 0x40000100L, 0x00080000L,
	0x00000000L, 0x40080000L, 0x02080100L, 0x40000100L };

static unsigned long SP6[64] = {
	0x20000010L, 0x20400000L, 0x00004000L, 0x20404010L,
	0x20400000L, 0x00000010L, 0x20404010L, 0x00400000L,
	0x20004000L, 0x00404010L, 0x00400000L, 0x20000010L,
	0x00400010L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x00000000L, 0x00400010L, 0x20004010L, 0x00004000L,
	0x00404000L, 0x20004010L, 0x00000010L, 0x20400010L,
	0x20400010L, 0x00000000L, 0x00404010L, 0x20404000L,
	0x00004010L, 0x00404000L, 0x20404000L, 0x20000000L,
	0x20004000L, 0x00000010L, 0x20400010L, 0x00404000L,
	0x20404010L, 0x00400000L, 0x00004010L, 0x20000010L,
	0x00400000L, 0x20004000L, 0x20000000L, 0x00004010L,
	0x20000010L, 0x20404010L, 0x00404000L, 0x20400000L,
	0x00404010L, 0x20404000L, 0x00000000L, 0x20400010L,
	0x00000010L, 0x00004000L, 0x20400000L, 0x00404010L,
	0x00004000L, 0x00400010L, 0x20004010L, 0x00000000L,
	0x20404000L, 0x20000000L, 0x00400010L, 0x20004010L };

static unsigned long SP7[64] = {
	0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
	0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
	0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
	0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
	0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
	0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
	0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
	0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
	0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
	0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
	0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
	0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
	0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
	0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
	0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
	0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L };

static unsigned long SP8[64] = {
	0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
	0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
	0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
	0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
	0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
	0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
	0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
	0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
	0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
	0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
	0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
	0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
	0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
	0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
	0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
	0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L };

static void desfunc(register unsigned long * block, register unsigned long *keys)
{
	register unsigned long fval, work, right, leftt;
	register int round;

	leftt = block[0];
	right = block[1];
	work = ((leftt >> 4) ^ right) & 0x0f0f0f0fL;
	right ^= work;
	leftt ^= (work << 4);
	work = ((leftt >> 16) ^ right) & 0x0000ffffL;
	right ^= work;
	leftt ^= (work << 16);
	work = ((right >> 2) ^ leftt) & 0x33333333L;
	leftt ^= work;
	right ^= (work << 2);
	work = ((right >> 8) ^ leftt) & 0x00ff00ffL;
	leftt ^= work;
	right ^= (work << 8);
	right = ((right << 1) | ((right >> 31) & 1L)) & 0xffffffffL;
	work = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;
	right ^= work;
	leftt = ((leftt << 1) | ((leftt >> 31) & 1L)) & 0xffffffffL;

	for( round = 0; round < 8; round++ ) {
		work  = (right << 28) | (right >> 4);
		work ^= *keys++;
		fval  = SP7[ work		 & 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work  = right ^ *keys++;
		fval |= SP8[ work		 & 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		leftt ^= fval;
		work  = (leftt << 28) | (leftt >> 4);
		work ^= *keys++;
		fval  = SP7[ work		 & 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work  = leftt ^ *keys++;
		fval |= SP8[ work		 & 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		right ^= fval;
		}

	right = (right << 31) | (right >> 1);
	work = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;
	right ^= work;
	leftt = (leftt << 31) | (leftt >> 1);
	work = ((leftt >> 8) ^ right) & 0x00ff00ffL;
	right ^= work;
	leftt ^= (work << 8);
	work = ((leftt >> 2) ^ right) & 0x33333333L;
	right ^= work;
	leftt ^= (work << 2);
	work = ((right >> 16) ^ leftt) & 0x0000ffffL;
	leftt ^= work;
	right ^= (work << 16);
	work = ((right >> 4) ^ leftt) & 0x0f0f0f0fL;
	leftt ^= work;
	right ^= (work << 4);
	*block++ = right;
	*block = leftt;
	return;
	}

#ifdef D2_DES

void des2key(unsigned char *hexkey, short mode)		/* stomps on Kn3 too */
{
	short revmod;

	revmod = (mode == EN0) ? DE1 : EN0;
	deskey(&hexkey[8], revmod);
	cpkey(KnR);
	deskey(hexkey, mode);
	cpkey(Kn3);					/* Kn3 = KnL */
	return;
	}

void Ddes(unsigned char *from, unsigned char *into)
{
	unsigned long work[2];

	scrunch(from, work);
	desfunc(work, KnL);
	desfunc(work, KnR);
	desfunc(work, Kn3);
	unscrun(work, into);
	return;
	}

void D2des(unsigned char *from, unsigned char *into)
{
	unsigned long *right, *l1, swap;
	unsigned long leftt[2], bufR[2];

	right = bufR;
	l1 = &leftt[1];
	scrunch(from, leftt);
	scrunch(&from[8], right);
	desfunc(leftt, KnL);
	desfunc(right, KnL);
	swap = *l1;
	*l1 = *right;
	*right = swap;
	desfunc(leftt, KnR);
	desfunc(right, KnR);
	swap = *l1;
	*l1 = *right;
	*right = swap;
	desfunc(leftt, Kn3);
	desfunc(right, Kn3);
	unscrun(leftt, into);
	unscrun(right, &into[8]);
	return;
	}

void makekey(register char *aptr, register unsigned char *kptr)
				/* NULL-terminated  */
		/* unsigned char[8] */
{
	register unsigned char *store;
	register int first, i;
	unsigned long savek[96];

	cpDkey(savek);
	des2key(Df_Key, EN0);
	for( i = 0; i < 8; i++ ) kptr[i] = Df_Key[i];
	first = 1;
	while( (*aptr != '\0') || first ) {
		store = kptr;
		for( i = 0; i < 8 && (*aptr != '\0'); i++ ) {
			*store++ ^= *aptr & 0x7f;
			*aptr++ = '\0';
			}
		Ddes(kptr, kptr);
		first = 0;
		}
	useDkey(savek);
	return;
	}

void make2key(register char *aptr, register unsigned char *kptr)
				/* NULL-terminated   */
		/* unsigned char[16] */
{
	register unsigned char *store;
	register int first, i;
	unsigned long savek[96];

	cpDkey(savek);
	des2key(Df_Key, EN0);
	for( i = 0; i < 16; i++ ) kptr[i] = Df_Key[i];
	first = 1;
	while( (*aptr != '\0') || first ) {
		store = kptr;
		for( i = 0; i < 16 && (*aptr != '\0'); i++ ) {
			*store++ ^= *aptr & 0x7f;
			*aptr++ = '\0';
			}
		D2des(kptr, kptr);
		first = 0;
		}
	useDkey(savek);
	return;
	}

#ifndef D3_DES	/* D2_DES only */
#ifdef	D2_DES	/* iff D2_DES! */

void cp2key(register unsigned long *into)
	/* unsigned long[64] */
{
	register unsigned long *from, *endp;

	cpkey(into);
	into = &into[32];
	from = KnR, endp = &KnR[32];
	while( from < endp ) *into++ = *from++;
	return;
	}

void use2key(register unsigned long *from)				/* stomps on Kn3 too */
	/* unsigned long[64] */
{
	register unsigned long *to, *endp;

	usekey(from);
	from = &from[32];
	to = KnR, endp = &KnR[32];
	while( to < endp ) *to++ = *from++;
	cpkey(Kn3);					/* Kn3 = KnL */
	return;
	}

#endif	/* iff D2_DES */
#else	/* D3_DES too */

static void D3des(unsigned char *, unsigned char *);

void des3key(unsigned char *hexkey, short mode)
			/* unsigned char[24] */

{
	unsigned char *first, *third;
	short revmod;

	if( mode == EN0 ) {
		revmod = DE1;
		first = hexkey;
		third = &hexkey[16];
		}
	else {
		revmod = EN0;
		first = &hexkey[16];
		third = hexkey;
		}
	deskey(&hexkey[8], revmod);
	cpkey(KnR);
	deskey(third, mode);
	cpkey(Kn3);
	deskey(first, mode);
	return;
	}

void cp3key(register unsigned long *into)
	/* unsigned long[96] */
{
	register unsigned long *from, *endp;

	cpkey(into);
	into = &into[32];
	from = KnR, endp = &KnR[32];
	while( from < endp ) *into++ = *from++;
	from = Kn3, endp = &Kn3[32];
	while( from < endp ) *into++ = *from++;
	return;
	}

void use3key(register unsigned long *from)
	/* unsigned long[96] */
{
	register unsigned long *to, *endp;

	usekey(from);
	from = &from[32];
	to = KnR, endp = &KnR[32];
	while( to < endp ) *to++ = *from++;
	to = Kn3, endp = &Kn3[32];
	while( to < endp ) *to++ = *from++;
	return;
	}

static void D3des(unsigned char *from, unsigned char *into)	/* amateur theatrics */
			/* unsigned char[24] */
			/* unsigned char[24] */
{
	unsigned long swap, leftt[2], middl[2], right[2];

	scrunch(from, leftt);
	scrunch(&from[8], middl);
	scrunch(&from[16], right);
	desfunc(leftt, KnL);
	desfunc(middl, KnL);
	desfunc(right, KnL);
	swap = leftt[1];
	leftt[1] = middl[0];
	middl[0] = swap;
	swap = middl[1];
	middl[1] = right[0];
	right[0] = swap;
	desfunc(leftt, KnR);
	desfunc(middl, KnR);
	desfunc(right, KnR);
	swap = leftt[1];
	leftt[1] = middl[0];
	middl[0] = swap;
	swap = middl[1];
	middl[1] = right[0];
	right[0] = swap;
	desfunc(leftt, Kn3);
	desfunc(middl, Kn3);
	desfunc(right, Kn3);
	unscrun(leftt, into);
	unscrun(middl, &into[8]);
	unscrun(right, &into[16]);
	return;
	}

void make3key(register char *aptr, register unsigned char *kptr)
				/* NULL-terminated   */
		/* unsigned char[24] */
{
	register unsigned char *store;
	register int first, i;
	unsigned long savek[96];

	cp3key(savek);
	des3key(Df_Key, EN0);
	for( i = 0; i < 24; i++ ) kptr[i] = Df_Key[i];
	first = 1;
	while( (*aptr != '\0') || first ) {
		store = kptr;
		for( i = 0; i < 24 && (*aptr != '\0'); i++ ) {
			*store++ ^= *aptr & 0x7f;
			*aptr++ = '\0';
			}
		D3des(kptr, kptr);
		first = 0;
		}
	use3key(savek);
	return;
	}

#endif	/* D3_DES */
#endif	/* D2_DES */

/* Validation sets:
 *
 * Single-length key, single-length plaintext -
 * Key	  : 0123 4567 89ab cdef
 * Plain  : 0123 4567 89ab cde7
 * Cipher : c957 4425 6a5e d31d
 *
 * Double-length key, single-length plaintext -
 * Key	  : 0123 4567 89ab cdef fedc ba98 7654 3210
 * Plain  : 0123 4567 89ab cde7
 * Cipher : 7f1d 0a77 826b 8aff
 *
 * Double-length key, double-length plaintext -
 * Key	  : 0123 4567 89ab cdef fedc ba98 7654 3210
 * Plain  : 0123 4567 89ab cdef 0123 4567 89ab cdff
 * Cipher : 27a0 8440 406a df60 278f 47cf 42d6 15d7
 *
 * Triple-length key, single-length plaintext -
 * Key	  : 0123 4567 89ab cdef fedc ba98 7654 3210 89ab cdef 0123 4567
 * Plain  : 0123 4567 89ab cde7
 * Cipher : de0b 7c06 ae5e 0ed5
 *
 * Triple-length key, double-length plaintext -
 * Key	  : 0123 4567 89ab cdef fedc ba98 7654 3210 89ab cdef 0123 4567
 * Plain  : 0123 4567 89ab cdef 0123 4567 89ab cdff
 * Cipher : ad0d 1b30 ac17 cf07 0ed1 1c63 81e4 4de5
 *
 * d3des V5.0a rwo 9208.07 18:44 Graven Imagery
 **********************************************************************/
void my_deskey(unsigned char *keyin,unsigned long *keyout,short edf)

//keyout是32字节unsigned long *
/////////////8字节key，输出也是8字节
{
register int i, j, l, m, n;
	unsigned char pc1m[56], pcr[56];
	unsigned long kn[32];

	for ( j = 0; j < 56; j++ ) {
		l = pc1[j];
		m = l & 07;
		pc1m[j] = (keyin[l >> 3] & bytebit[m]) ? 1 : 0;
		//此处把key变为keyin
		}
	for( i = 0; i < 16; i++ ) {
		if( edf == DE1 ) m = (15 - i) << 1;
		else m = i << 1;
		n = m + 1;
		kn[m] = kn[n] = 0L;
		for( j = 0; j < 28; j++ ) {
			l = j + totrot[i];
			if( l < 28 ) pcr[j] = pc1m[l];
			else pcr[j] = pc1m[l - 28];
			}
		for( j = 28; j < 56; j++ ) {
		    l = j + totrot[i];
		    if( l < 56 ) pcr[j] = pc1m[l];
		    else pcr[j] = pc1m[l - 28];
		    }
		for( j = 0; j < 24; j++ ) {
			if( pcr[pc2[j]] ) kn[m] |= bigbyte[j];
			if( pcr[pc2[j+24]] ) kn[n] |= bigbyte[j];
			}
		}
my_cookey(kn,keyout);

	return;
}



void my_des2key(unsigned char *hexkey,unsigned long *my_KnR,unsigned long *my_Kn3,short mode)
			/* unsigned char[16] */
			/* unsigned char[32] */
			/* unsigned char[32] */
/*		      hexkey[16]     MODE*/
{
short revmod;
	revmod = (mode == EN0) ? DE1 : EN0;
	my_deskey(&hexkey[8],my_KnR, revmod);
//	cpkey(KnR);
	my_deskey(hexkey, my_Kn3,mode);
//	cpkey(Kn3);					/* Kn3 = KnL */
	return;
}

void my_des(unsigned char *inblock, 
			unsigned char *outblock,
			unsigned char *my_key,
			short edf
			)

//8byte
//from[8]	      to[8]   8字节的key
{
	unsigned long keyout[32]={0L};
	unsigned long work[2];
    my_deskey(my_key,keyout,edf);

	scrunch(inblock, work);
	desfunc(work, keyout);//把KnL改为了keyout
	unscrun(work, outblock);
	return;

}
void my_Ddes(unsigned char *from,
			 unsigned char *into,
			 unsigned char *my_key,
			 short edf)
//8byte
//16字节

///*		    from[8]	      to[8]   16字节的key
{
	unsigned long my_KnR[32]={0L};
	unsigned long my_Kn3[32]={0L};
    unsigned long work[2];
	my_des2key(my_key,my_KnR,my_Kn3, edf);
	scrunch(from, work);
	desfunc(work, my_Kn3);//我认为此时KnL==Kn3
	desfunc(work, my_KnR);
	desfunc(work, my_Kn3);
	unscrun(work, into);
	return;
}

void my_D2des(unsigned char * from,
			  unsigned char * into,
			  unsigned char * my_key,
			  short edf)
//16byte
//16字节

///*		    from[16]	      to[16]
{
	unsigned long *right, *l1, swap;
	unsigned long leftt[2], bufR[2];
	unsigned long my_KnR[32]={0L};
	unsigned long my_Kn3[32]={0L};
    
	my_des2key(my_key,my_KnR,my_Kn3,edf);
    ///////////////////d2des/////
	//我认为此时KnL==Kn3
	right = bufR;
	l1 = &leftt[1];
	scrunch(from, leftt);
	scrunch(&from[8], right);
	desfunc(leftt, my_Kn3);//KnL);
	desfunc(right, my_Kn3);//KnL);
	swap = *l1;
	*l1 = *right;
	*right = swap;
	desfunc(leftt,my_KnR);// KnR);
	desfunc(right, my_KnR);//KnR);
	swap = *l1;
	*l1 = *right;
	*right = swap;
	desfunc(leftt, my_Kn3);//Kn3);
	desfunc(right, my_Kn3);//Kn3);
	unscrun(leftt, into);
	unscrun(right, &into[8]);
	return;
}

///////////////
static void my_cookey(register unsigned long *raw1,
					  unsigned long *m_key)

//32字节unsigned long*
{
	register unsigned long *cook, *raw0;
	unsigned long dough[32];
	register int i;
	register unsigned long *to, *endp;

	cook = dough;
	for( i = 0; i < 16; i++, raw1++ ) {
		raw0 = raw1++;
		*cook	 = (*raw0 & 0x00fc0000L) << 6;
		*cook	|= (*raw0 & 0x00000fc0L) << 10;
		*cook	|= (*raw1 & 0x00fc0000L) >> 10;
		*cook++ |= (*raw1 & 0x00000fc0L) >> 6;
		*cook	 = (*raw0 & 0x0003f000L) << 12;
		*cook	|= (*raw0 & 0x0000003fL) << 16;
		*cook	|= (*raw1 & 0x0003f000L) >> 4;
		*cook++ |= (*raw1 & 0x0000003fL);
		}
	///////此处//////////////给输出
    cook=dough;
	to = m_key, endp = &m_key[32];
	while( to < endp ) *to++ = *cook++;
	return;

//		return;
	}
	
	
/********************************************************************
//    DESCRIPTION
//    set the flag to judge whether it is a des sim card
//    Global resource dependence :
//    Author: yuhua.shi
//    Note:
***********************************************************************/
void MMI_SetIsDesSimcardFlag(BOOLEAN flag)
{
	s_is_des_simcard_flag = flag;
} 

/********************************************************************
//    DESCRIPTION
//    get the flag to judge whether it is a des sim card
//    Global resource dependence :
//    Author:
//    Note: yuhua.shi
***********************************************************************/
BOOLEAN MMI_GetIsDesSimcardFlag(void)
{
	return	s_is_des_simcard_flag;
}

/********************************************************************
//    DESCRIPTION
//    read the imei num from nv
//    Global resource dependence :
//    Author:
//    Note:
***********************************************************************/
uint8 MMI_DphoneGetImeiNumber(void)
{
	uint8 imei_num=0x00;
	MNNV_ReadItem(MMINV_IMEI_NUM,sizeof(uint8),&imei_num);
	SCI_TRACE_LOW("MMI_DphoneGetImeiNumber imei=%d",imei_num);
	return imei_num;
}



/********************************************************************
//    DESCRIPTION
//    read the flag to judge whether it is allowed to use the normal sim 
//    card from nv
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
BOOLEAN MMI_GetNormalCardAllowedFlag(void)
{
	MN_RETURN_RESULT_E   return_val = MN_RETURN_FAILURE;
	BOOLEAN    normalcard_allowed = TRUE;
	
	return_val = MNNV_ReadItem(MMINV_DES_NORMALCARD_ALLOWED_FLAG,sizeof(uint8),&normalcard_allowed);
	if(MN_RETURN_FAILURE == return_val)
	{
		normalcard_allowed = TRUE;
		MNNV_WriteItem(MMINV_DES_NORMALCARD_ALLOWED_FLAG, sizeof(uint8), &normalcard_allowed);
	}
	SCI_TRACE_LOW("MMI::get normal card allowed is %d", normalcard_allowed);
	
	return normalcard_allowed;
}

/**********************************************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Datang V3 Jiangsu card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************************************/
static void DesDatangV2JiangsuCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult)
{
	uint8 index ,i;
	uint8 *p_key;
	uint8 input[DPHONE_DES_KEY_NUM/2];
	uint8 result[DPHONE_DES_KEY_NUM/2];
	uint8 kc1[DPHONE_DES_KEY_NUM];
	uint8 kc[DPHONE_DES_KEY_NUM];
	/*
	//for test
	uint8 km[DPHONE_DES_KEY_ARRAY_NUM][DPHONE_DES_KEY_NUM] = 
	{
		{0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x11 , 0x11 , 0x11 , 0x11 , 0x11 , 0x11 , 0x11, 0x11, 0x11 , 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
		{0x22 , 0x22 , 0x22 , 0x22 , 0x22 , 0x22 , 0x22, 0x22, 0x22 , 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22},
		{0x33 , 0x33 , 0x33 , 0x33 , 0x33 , 0x33 , 0x33, 0x33, 0x33 , 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33},
		{0x44 , 0x44 , 0x44 , 0x44 , 0x44 , 0x44 , 0x44, 0x44, 0x44 , 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44}
	};
	*/

	if (EFS_NvitemRead(MMINV_3DES_KM,DPHONE_DES_KEY_ARRAY_NUM*DPHONE_DES_KEY_NUM,(uint8*)km)==0)
    {
		SCI_TRACE_LOW("DesDatangV2JiangsuCard , GET KM SUCCESS");
    }
	else
    {
        SCI_TRACE_LOW("DesDatangV2JiangsuCard , GET KM FAIL");
		
    }
	
	index = (pdes->rand[DPHONE_DATANG_V2_RAND1_NUM-1] & DPHONE_DES_KEY_MASK)%DPHONE_DES_KEY_ARRAY_NUM;  //第 4 字节 RANDOM 先 与 7 再 模 5 ， 获得 密钥索引
	p_key = (uint8*)km + index*DPHONE_DES_KEY_NUM;
	
	
	SCI_TRACE_BUF("INPUT RAND : " , pdes->rand , DPHONE_DATANG_V2_RAND_NUM);
	SCI_TRACE_BUF("INPUT ICCID: " , pdes->iccid , DPHONE_DES_ICCID_NUM);
	
	
	
	//一级分散
	SCI_MEMCPY(input , pdes->iccid , DPHONE_DES_ICCID_NUM);
	
	my_Ddes(input , result , p_key ,EN0);  // kc1(upper 8 bytes) = 3des(iccid , km)
	
	SCI_MEMCPY(kc1 , result , DPHONE_DES_KEY_NUM/2);
	
	for(i = 0 ; i < DPHONE_DES_ICCID_NUM ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input , result , p_key , EN0);  //kc1(lower 8 bytes) = 3des(NOT(iccid) , km)
	SCI_MEMCPY( (uint8*)kc1 + DPHONE_DES_KEY_NUM/2 , result ,DPHONE_DES_KEY_NUM/2);
	
	SCI_TRACE_BUF("KC 1 (0-7):" , kc1 , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF("KC 1 (8-15):" , &(kc1[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	
	//二级分散
	SCI_MEMSET(input , 0x20 , DPHONE_DES_KEY_NUM/2);
	SCI_MEMCPY(input , pdes->rand , DPHONE_DATANG_V2_RAND1_NUM);
	
	my_Ddes(input , result , kc1 ,EN0);  //kc(upper 8 bytes) = 3des(rand1(低位补4个x020) , kc1)
	SCI_MEMCPY( kc , result , DPHONE_DES_KEY_NUM/2);
	
	for(i = 0 ; i < DPHONE_DES_KEY_NUM/2 ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input, result , kc1, EN0);  //kc(lower 8 bytes) = 3des(NOT(rand1(低位补4个x020)) , kc1)
	
	SCI_MEMCPY((uint8*)kc + DPHONE_DES_KEY_NUM/2 , result , DPHONE_DES_KEY_NUM/2);
	
	SCI_TRACE_BUF(" KC (0-7):" , kc , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF(" KC (8-15):" , &(kc[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	
	//计算输出结果
	my_Ddes(&(pdes->rand[DPHONE_DATANG_V2_RAND1_NUM]) , presult->cryp_array , kc , EN0);
	
	my_Ddes(&(pdes->rand[DPHONE_DATANG_V2_RAND1_NUM+(DPHONE_DES_KEY_NUM/2)]) , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , kc , EN0);
	
	SCI_TRACE_BUF(" CRYP RESULT(0-7) :" , presult->cryp_array , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF(" CRYP RESULT(8-15) :" , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	presult->cryp_len = DPHONE_DES_CRYP_NUM;
	
	return;
}

/*************************************************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Datang V2.1 Liaoning card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
**************************************************************************************************/
static void DesDatangV21LiaoningCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult)
{
	uint8 index ,i;
	uint8 *p_key;
	uint8 input[DPHONE_DES_KEY_NUM/2];
	uint8 result[DPHONE_DES_KEY_NUM/2];
	uint8 kc1[DPHONE_DES_KEY_NUM];
	uint8 kc[DPHONE_DES_KEY_NUM];
	uint8 imei[MN_MAX_MOBILE_ID_LENGTH];
    char  imeiStr[MN_MAX_MOBILE_ID_LENGTH*2 + 1];
    
    /*
    //for test
	uint8 km[DPHONE_DES_KEY_ARRAY_NUM][DPHONE_DES_KEY_NUM] = 
	{
		{0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x11 , 0x11 , 0x11 , 0x11 , 0x11 , 0x11 , 0x11, 0x11, 0x11 , 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
		{0x22 , 0x22 , 0x22 , 0x22 , 0x22 , 0x22 , 0x22, 0x22, 0x22 , 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22},
		{0x33 , 0x33 , 0x33 , 0x33 , 0x33 , 0x33 , 0x33, 0x33, 0x33 , 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33},
		{0x44 , 0x44 , 0x44 , 0x44 , 0x44 , 0x44 , 0x44, 0x44, 0x44 , 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44}
	};
	*/

    //获得IMEI
    SCI_MEMSET(imeiStr,0x20,strlen(imeiStr));
	 if(MNNV_GetIMEI(imei) )
	 {
      	if((imei[0] & 0x08) != 0) //odd
		{
			MMI_BcdToStr(PACKED_LSB_FIRST,
	            imei,
	            MN_MAX_MOBILE_ID_LENGTH*2,
	            (char*)imeiStr);
		}
		else  //even
		{
			//如果是偶数，第一个BYTE是  0xfa , 跳过
			MMI_BcdToStr(PACKED_LSB_FIRST,
	            imei+1,
	            (MN_MAX_MOBILE_ID_LENGTH-1)*2,
	            (char*)imeiStr);
		}
	  }
    SCI_TRACE_BUF("DES IMEI: " , imeiStr , MN_MAX_MOBILE_ID_LENGTH*2);	
   
	if (EFS_NvitemRead(MMINV_3DES_KM,DPHONE_DES_KEY_ARRAY_NUM*DPHONE_DES_KEY_NUM,(uint8*)km)==0)
    {
		SCI_TRACE_LOW("DesDatangV21LiaoningCard , GET KM SUCCESS");
    }
	else
    {
        SCI_TRACE_LOW("DesDatangV21LiaoningCard , GET KM FAIL");
		
    }
	
	//index = (pdes->rand[3] & 0x07)%5;  //第 4 字节 RANDOM 先 与 7 再 模 5 ， 获得 密钥索引
	//辽宁卡需要8位rand
	index = (pdes->rand[DPHONE_DATANG_V21_RAND_NUM-1] & DPHONE_DES_KEY_MASK)%DPHONE_DES_KEY_ARRAY_NUM;  //第 8 字节 RANDOM 先 与 7 再 模 5 ， 获得 密钥索引
	p_key = (uint8*)km + index*DPHONE_DES_KEY_NUM;
	
	
	SCI_TRACE_BUF("INPUT RAND : " , pdes->rand , DPHONE_DATANG_V21_RAND_NUM);
	SCI_TRACE_BUF("INPUT ICCID: " , pdes->iccid , DPHONE_DES_ICCID_NUM);
	
	
	
	//一级分散
	SCI_MEMCPY(input , pdes->iccid , DPHONE_DES_ICCID_NUM);
	
	my_Ddes(input , result , p_key ,EN0);  // kc1(upper 8 bytes) = 3des(iccid , km)
	
	SCI_MEMCPY(kc1 , result , DPHONE_DES_KEY_NUM/2);
	
	for(i = 0 ; i < DPHONE_DES_ICCID_NUM ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input , result , p_key , EN0);  //kc1(lower 8 bytes) = 3des(NOT(iccid) , km)
	SCI_MEMCPY( (uint8*)kc1 + DPHONE_DES_KEY_NUM/2 , result ,DPHONE_DES_KEY_NUM/2);
	
	SCI_TRACE_BUF("KC 1 (0-7):" , kc1 , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF("KC 1 (8-15):" , &(kc1[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	
	//二级分散
	SCI_MEMSET(input , 0x20 , DPHONE_DES_KEY_NUM/2);
	//辽宁卡需要8位and
	SCI_MEMCPY(input , pdes->rand , DPHONE_DES_KEY_NUM/2);
	
	my_Ddes(input , result , kc1 ,EN0);  //kc(upper 8 bytes) = 3des(rand1 , kc1)
	SCI_MEMCPY( kc , result , DPHONE_DES_KEY_NUM/2);
	
	for(i = 0 ; i < DPHONE_DES_KEY_NUM/2 ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input, result , kc1, EN0);  //kc(lower 8 bytes) = 3des(NOT(rand1) , kc1)
	
	SCI_MEMCPY((uint8*)kc + DPHONE_DES_KEY_NUM/2 , result , DPHONE_DES_KEY_NUM/2);
	
	SCI_TRACE_BUF(" KC (0-7):" , kc , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF(" KC (8-15):" , &(kc[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	//计算输出结果
	//辽宁卡用IMEI
	my_Ddes((uint8*)imeiStr , presult->cryp_array , kc , EN0);
	
	my_Ddes((uint8*)(&(imeiStr[DPHONE_DES_KEY_NUM/2])) , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , kc , EN0);
	
	SCI_TRACE_BUF(" CRYP RESULT(0-7) :" , presult->cryp_array , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF(" CRYP RESULT(8-15) :" , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	presult->cryp_len = DPHONE_DES_CRYP_NUM;
	
	return;
}

/***********************************************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Datang V3 Jiangsu card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
************************************************************************************************/
static void DesDatangV3JiangsuCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult)
{
	uint8 index ,i;
	uint8 *p_key;
	uint8 input[DPHONE_DES_KEY_NUM/2];
	uint8 result[DPHONE_DES_KEY_NUM/2];
	uint8 kc1[DPHONE_DES_KEY_NUM];
	uint8 kc[DPHONE_DES_KEY_NUM];
	
	/*
	Key1=0xA1,0x6A,0xB0,0x8A,0x45,0x2D,0xB4,0xF0,0x65,0xE3,0x42,0x81,0xA4,0x1A,0x09,0xD3
	Key2=0xA5,0x10,0xAA,0x21,0x09,0xD8,0x90,0xAC,0x70,0x4E,0xF3,0xE6,0x20,0x4E,0xAC,0x4F
	Key3=0x3D,0x9B,0x87,0x73,0x93,0x16,0x74,0x33,0x4D,0xF6,0xED,0xFC,0xB9,0xD7,0x59,0x64
	Key4=0x6C,0xA9,0x1C,0x6A,0x80,0x5B,0x55,0x13,0x7F,0x00,0x5B,0x9B,0x05,0x9F,0x8C,0x5B
	Key5=0x31,0x69,0x90,0x33,0xBD,0x65,0x99,0x59,0x65,0x2E,0x33,0x81,0xF8,0x3B,0x85,0x5B
	*/
	//for test
	uint8 km[DPHONE_DES_KEY_ARRAY_NUM][DPHONE_DES_KEY_NUM] =  //imei = 0x06
	{
/*		//wintk key
		{0x35,0xBD,0xEF,0x69,0x14,0x90,0x63,0x0A,0x23,0x38,0x15,0x49,0xF7,0xB1,0x16,0xF2},
		{0xD3,0xA5,0x67,0x73,0x17,0x83,0x38,0x12,0x49,0x7B,0x1C,0x6F,0x7B,0x1E,0x6F,0x67},
		{0x9B,0x55,0x51,0x8E,0xF4,0x40,0x12,0x4D,0x63,0x01,0xA2,0xB1,0x1C,0x2D,0xA2,0x32},
		{0x04,0x51,0x7D,0xEF,0x04,0x35,0x9F,0x7B,0x1E,0x6B,0x4D,0xB6,0x30,0x12,0x18,0x16},
		{0x02,0xF3,0x7F,0xAB,0xB6,0x11,0x68,0x12,0x49,0xF7,0xB1,0xE6,0xAF,0x33,0x0A,0x29},

		//xinuo key
		{0x35,0xBD,0xEF,0x69,0x14,0x90,0x3A,0x3C,0x37,0xBE,0xE7,0xB4,0x16,0x9C,0xFA,0x6E},
		{0xD3,0xA5,0x67,0x73,0x17,0x83,0x18,0x9D,0xA8,0xDC,0x93,0x92,0xEF,0x2F,0x58,0x1E},
		{0x9B,0x55,0x51,0x8E,0xF4,0x40,0xB0,0xC9,0xE5,0x66,0x30,0xED,0xF1,0xE1,0x77,0x02},
		{0x04,0x51,0x7D,0xEF,0x04,0x35,0x1E,0xE5,0xC9,0x06,0xC1,0x46,0x6C,0x74,0xFB,0x9E},
		{0x02,0xF3,0x7F,0xAB,0xB6,0x11,0x75,0xFB,0xBC,0x1D,0x9A,0x1F,0x8F,0xF8,0x1B,0x03},
*/		//star key
		{0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22},
		{0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33},
		{0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44},
		{0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55},
		{0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66,0x66}
	};

	/*
	if (EFS_NvitemRead(MMINV_3DES_KM,DPHONE_DES_KEY_ARRAY_NUM*DPHONE_DES_KEY_NUM,(uint8*)km)==0)
    {
		SCI_TRACE_LOW("DesDatangV3JiangsuCard , GET KM SUCCESS");
    }
	else
    {
        SCI_TRACE_LOW("DesDatangV3JiangsuCard , GET KM FAIL");
		
    }
    */
	
	index = (pdes->rand[DPHONE_DATANG_V3_RAND1_NUM-1] & DPHONE_DES_KEY_MASK)%DPHONE_DES_KEY_ARRAY_NUM;  //第 4 字节 RANDOM 先 与 7 再 模 5 ， 获得 密钥索引
	p_key = (uint8*)km + index*DPHONE_DES_KEY_NUM;
	
	
	SCI_TRACE_BUF("INPUT RAND : " , pdes->rand , DPHONE_DATANG_V3_RAND_NUM);
	SCI_TRACE_BUF("INPUT ICCID: " , pdes->iccid , DPHONE_DES_ICCID_NUM);
	
	
	
	//一级分散
	SCI_MEMCPY(input , pdes->iccid , DPHONE_DES_ICCID_NUM);
	
	my_Ddes(input , result , p_key ,EN0);  // kc1(upper 8 bytes) = 3des(iccid , km)
	
	SCI_MEMCPY(kc1 , result , DPHONE_DES_KEY_NUM/2);
	
	for(i = 0 ; i < DPHONE_DES_ICCID_NUM ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input , result , p_key , EN0);  //kc1(lower 8 bytes) = 3des(NOT(iccid) , km)
	SCI_MEMCPY( (uint8*)kc1 + DPHONE_DES_KEY_NUM/2 , result ,DPHONE_DES_KEY_NUM/2);
	
	SCI_TRACE_BUF("KC 1 (0-7):" , kc1 , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF("KC 1 (8-15):" , &(kc1[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	
	//二级分散
	SCI_MEMSET(input , 0x20 , DPHONE_DES_KEY_NUM/2);
	SCI_MEMCPY(input , pdes->rand , DPHONE_DATANG_V3_RAND1_NUM);
	
	my_Ddes(input , result , kc1 ,EN0);  //kc(upper 8 bytes) = 3des(rand1(低位补4个x020) , kc1)
	SCI_MEMCPY( kc , result , DPHONE_DES_KEY_NUM/2);
	
	for(i = 0 ; i < DPHONE_DES_KEY_NUM/2 ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input, result , kc1, EN0);  //kc(lower 8 bytes) = 3des(NOT(rand1(低位补4个x020)) , kc1)
	
	SCI_MEMCPY((uint8*)kc + DPHONE_DES_KEY_NUM/2 , result , DPHONE_DES_KEY_NUM/2);
	
	SCI_TRACE_BUF(" KC (0-7):" , kc , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF(" KC (8-15):" , &(kc[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	
	//计算输出结果
	my_Ddes(&(pdes->rand[DPHONE_DATANG_V3_RAND1_NUM]) , presult->cryp_array , kc , EN0);
	
	my_Ddes(&(pdes->rand[DPHONE_DATANG_V3_RAND1_NUM+DPHONE_DES_KEY_NUM/2]) , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , kc , EN0);
	
	SCI_TRACE_BUF(" CRYP RESULT(0-7) :" , presult->cryp_array , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF(" CRYP RESULT(8-15) :" , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	presult->cryp_len = DPHONE_DES_CRYP_NUM;
	
	return;
}

/**************************************************************************************
//    DESCRIPTION
//    do the des algorithm to produce the cryptograph result for Woqi card
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***************************************************************************************/
static void DesWoqiCard(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult)
{
	uint8 index ,i;
	uint8 *p_key;
	uint8 input[DPHONE_DES_KEY_NUM/2];
	uint8 result[DPHONE_DES_KEY_NUM/2];
	uint8 kc1[DPHONE_DES_KEY_NUM];
	uint8 kc[DPHONE_DES_KEY_NUM];
	uint8 km[DPHONE_DES_KEY_NUM];
	

	SCI_MEMCPY(km , pdes->key , DPHONE_DES_KEY_NUM);//获得主密钥
	p_key = (uint8*)km;
	
	
	//分散
	SCI_MEMCPY(input , pdes->rand , DPHONE_DES_KEY_NUM/2);
	
	my_Ddes(input , result , p_key ,EN0);  // kc(upper 8 bytes) = 3des(rand , km)
	
	SCI_MEMCPY(kc , result , DPHONE_DES_KEY_NUM/2);
	
	for(i = 0 ; i < DPHONE_DES_KEY_NUM/2 ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input , result , p_key , EN0);  // kc(lower 8 bytes) = 3des(~rand , km)
	SCI_MEMCPY( (uint8*)kc + DPHONE_DES_KEY_NUM/2 , result ,DPHONE_DES_KEY_NUM/2);
	
	SCI_TRACE_BUF("KC  (0-7):" , kc , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF("KC  (8-15):" , &(kc[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	
	
	//加密,计算输出结果
	SCI_MEMCPY(input , pdes->rand , DPHONE_DES_KEY_NUM/2);
	
	my_Ddes(input , presult->cryp_array , kc , EN0);// cryp_result(upper 8 bytes) = 3des(rand , kc)
	
	for(i = 0 ; i < DPHONE_DES_KEY_NUM/2 ; i++)
	{
		input[i] = ~input[i];
	}
	
	my_Ddes(input , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , kc , EN0);// cryp_result(lower 8 bytes) = 3des(~rand , kc)
	
	SCI_TRACE_BUF(" CRYP RESULT(0-7) :" , presult->cryp_array , DPHONE_DES_KEY_NUM/2);
	SCI_TRACE_BUF(" CRYP RESULT(8-15) :" , &(presult->cryp_array[DPHONE_DES_KEY_NUM/2]) , DPHONE_DES_KEY_NUM/2);
	
	presult->cryp_len = DPHONE_DES_CRYP_NUM;
	
	return;
}

/**********************************************************************************
//    DESCRIPTION
//    switch the related des algorithm to produce the cryptograph result 
//    for stk task
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************************/
void MMI_DphoneDes(DPHONE_DES_PARAMETER_T* pdes,DPHONE_DES_RESULT_T * presult)
{
	DPHONE_DES_SIMCARD_TYPE_E card_type;
    SCI_TRACE_LOW("MMI_DphoneDes1111111111");	
    MMI_Des_SetProcessDes(TRUE);
	card_type = pdes->card_type;

	SCI_ASSERT(card_type > NORMAL_CARD && card_type < MAX_CARD_NUM);

	MMI_SetIsDesSimcardFlag(TRUE);
	s_des_simcard_process[card_type - 1](pdes, presult);
	
	return;
}


/**********************************************************************************
//    DESCRIPTION
//    des interface for ps project about get input proactive command to mmi
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************************/
BOOLEAN MMI_ProcessDesSimGetInput(SIMAT_GET_INPUT_T* com, SIMAT_DO_TEXT_STRING_T* temp_text_str)
{
	//extern void SIM_GetIccid(uint8 *piccid);

	DPHONE_DES_PARAMETER_T des_parameter;
	DPHONE_DES_RESULT_T des_result;
    uint8 i,des_offset;
	DPHONE_DES_SIMCARD_TYPE_E card_type;

#ifdef WIN32
	 return;
#endif

	 
    SCI_MEMSET(&des_parameter, 0, sizeof(DPHONE_DES_PARAMETER_T));
    
    card_type = MMI_GetDesSimCardType();
    SCI_TRACE_LOW("SIMAT:: des card type is %d", card_type);
    SCI_TRACE_LOW("SIMAT:: get input command len is %d", com->text_str.len);
    
	switch(card_type)
	{
	case NORMAL_CARD:
		//STK task not process the pc signal, send it to other task which need it
		//SCI_SendEventToClient(MN_APP_SIMAT_SERVICE,SIMAT_EVENT_GET_INPUT,com);
        SCI_ASSERT(FALSE);
        return;
		break;
		
	case DATANG_V2_JIANGSU_CARD:
		if(DPHONE_DATANG_V2_RAND_NUM == com->text_str.len)
		{
			//uint8 iccid_array[DPHONE_SIM_ICCID_NUM_LEN];
			MNSIM_ICCID_T iccid;
			//modified 
	        MNSIM_GetICCID((MNSIM_ICCID_T *)&iccid); //modified by alex li on 2005 10 17
	        //end
	        
	        //modified by alex li, on 2005 10 28 , if the card type is encrypted sim card, then set it.
	        MMI_SetNormalCardFlag(FALSE); 
	        //end
	        
	        SCI_TRACE_LOW("SIMAT::iccid(1-10):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
			iccid.id_num[0],iccid.id_num[1],iccid.id_num[2],iccid.id_num[3],
			iccid.id_num[4],iccid.id_num[5],iccid.id_num[6],iccid.id_num[7],
			iccid.id_num[8],iccid.id_num[9]);
			
			SCI_MEMCPY(des_parameter.iccid,iccid.id_num + (DPHONE_SIM_ICCID_NUM_LEN-DPHONE_DES_ICCID_NUM),DPHONE_DES_ICCID_NUM);
	        
	        SCI_MEMCPY(des_parameter.rand,com->text_str.text_ptr,DPHONE_DATANG_V2_RAND_NUM);
	        des_parameter.card_type = DATANG_V2_JIANGSU_CARD;
	        
	        SCI_TRACE_LOW("SIMAT::it is the datang v2 card.");
	        SCI_TRACE_LOW("SIMAT::Rand(1-20):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x \n",
				com->text_str.text_ptr[0],com->text_str.text_ptr[1],com->text_str.text_ptr[2],com->text_str.text_ptr[3],
				com->text_str.text_ptr[4],com->text_str.text_ptr[5],com->text_str.text_ptr[6],com->text_str.text_ptr[7],
				com->text_str.text_ptr[8],com->text_str.text_ptr[9],com->text_str.text_ptr[10],com->text_str.text_ptr[11],
				com->text_str.text_ptr[12],com->text_str.text_ptr[13],com->text_str.text_ptr[14],com->text_str.text_ptr[15],
				com->text_str.text_ptr[16],com->text_str.text_ptr[17],com->text_str.text_ptr[18],com->text_str.text_ptr[19]);
				
			/*
	        SCI_TRACE_LOW("SIMAT::iccid(1-10):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
			iccid_array[0],iccid_array[1],iccid_array[2],iccid_array[3],
			iccid_array[4],iccid_array[5],iccid_array[6],iccid_array[7],
			iccid_array[8],iccid_array[9]);
	 
	        SCI_MEMCPY(des_parameter.iccid,iccid_array + (DPHONE_SIM_ICCID_NUM_LEN-DPHONE_DES_ICCID_NUM),DPHONE_DES_ICCID_NUM);
	        SCI_MEMCPY(des_parameter.rand,com->text_str.text_ptr,DPHONE_DATANG_V2_RAND_NUM);
	        des_parameter.card_type = DATANG_V2_JIANGSU_CARD;
	        
	        SCI_TRACE_LOW("SIMAT::it is the datang v2 card.");
	        SCI_TRACE_LOW("SIMAT::Rand(1-20):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x \n",
				com->text_str.text_ptr[0],com->text_str.text_ptr[1],com->text_str.text_ptr[2],com->text_str.text_ptr[3],
				com->text_str.text_ptr[4],com->text_str.text_ptr[5],com->text_str.text_ptr[6],com->text_str.text_ptr[7],
				com->text_str.text_ptr[8],com->text_str.text_ptr[9],com->text_str.text_ptr[10],com->text_str.text_ptr[11],
				com->text_str.text_ptr[12],com->text_str.text_ptr[13],com->text_str.text_ptr[14],com->text_str.text_ptr[15],
				com->text_str.text_ptr[16],com->text_str.text_ptr[17],com->text_str.text_ptr[18],com->text_str.text_ptr[19]);
			*/
			
		}
		else
		{
			//SCI_SendEventToClient(MN_APP_SIMAT_SERVICE,SIMAT_EVENT_GET_INPUT,com);
       		return FALSE;
		}
		break;

	case DATANG_V21_LIAONING_CARD:
		if(DPHONE_DATANG_V21_RAND_NUM == com->text_str.len)
		{
			MNSIM_ICCID_T iccid;
			//modified 
	        MNSIM_GetICCID((MNSIM_ICCID_T *)&iccid); //modified by alex li on 2005 10 17
	        //end
	        
	        //modified by alex li, on 2005 10 28 , if the card type is encrypted sim card, then set it.
	        MMI_SetNormalCardFlag(FALSE); 
	        //end
	        
	        SCI_TRACE_LOW("SIMAT::iccid(1-10):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
			iccid.id_num[0],iccid.id_num[1],iccid.id_num[2],iccid.id_num[3],
			iccid.id_num[4],iccid.id_num[5],iccid.id_num[6],iccid.id_num[7],
			iccid.id_num[8],iccid.id_num[9]);
			
			SCI_MEMCPY(des_parameter.iccid,iccid.id_num + (DPHONE_SIM_ICCID_NUM_LEN-DPHONE_DES_ICCID_NUM),DPHONE_DES_ICCID_NUM);
	        
			/*
			uint8 iccid_array[DPHONE_SIM_ICCID_NUM_LEN];
			
	        //SIM_GetIccid(iccid_array);
	        //modified 
	        MNSIM_GetICCID(iccid_array); //modified by alex li on 2005 10 17
	        //end
	        
	        SCI_TRACE_LOW("SIMAT::iccid(1-10):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
			iccid_array[0],iccid_array[1],iccid_array[2],iccid_array[3],
			iccid_array[4],iccid_array[5],iccid_array[6],iccid_array[7],
			iccid_array[8],iccid_array[9]);
	 
	        SCI_MEMCPY(des_parameter.iccid,iccid_array + (DPHONE_SIM_ICCID_NUM_LEN-DPHONE_DES_ICCID_NUM),DPHONE_DES_ICCID_NUM);
	        */
	        
	        SCI_MEMCPY(des_parameter.rand,com->text_str.text_ptr,DPHONE_DATANG_V21_RAND_NUM);
	        des_parameter.card_type = DATANG_V21_LIAONING_CARD;
	        
	        SCI_TRACE_LOW("SIMAT::it is the datang v21 card.");
	        SCI_TRACE_LOW("SIMAT::Rand(1-8):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x \n",
				com->text_str.text_ptr[0],com->text_str.text_ptr[1],com->text_str.text_ptr[2],com->text_str.text_ptr[3],
				com->text_str.text_ptr[4],com->text_str.text_ptr[5],com->text_str.text_ptr[6],com->text_str.text_ptr[7]);
		}
		else
		{
			//SCI_SendEventToClient(MN_APP_SIMAT_SERVICE,SIMAT_EVENT_GET_INPUT,com);
       		return FALSE;
		}
		break;
		
	case DATANG_V3_JIANGSU_CARD:
		if(DPHONE_DATANG_V3_RAND_NUM == com->text_str.len)
		{	
			MNSIM_ICCID_T iccid;
			//modified 
			SCI_TRACE_LOW("DATANG_V3_JIANGSU_CARD");
	        MNSIM_GetICCID((MNSIM_ICCID_T *)&iccid); //modified by alex li on 2005 10 17
			//SIM_GetIccid((uint8 *) iccid.id_num);			
	        //end
	        
	        //modified by alex li, on 2005 10 28 , if the card type is encrypted sim card, then set it.
	        MMI_SetNormalCardFlag(FALSE); 
	        //end
/*			iccid.id_num[0]=0xff;iccid.id_num[1]=0xff;iccid.id_num[2]=0xff;iccid.id_num[3]=0xff;
			iccid.id_num[4]=0xff;iccid.id_num[5]=0xff;iccid.id_num[6]=0xff;iccid.id_num[7]=0xff;
			iccid.id_num[8]=0xff;iccid.id_num[9]=0xff;

			iccid.id_num[0]=0xff;iccid.id_num[1]=0xff;iccid.id_num[2]=g_sim_status.imsi.imsi_val[0];
			iccid.id_num[3]=g_sim_status.imsi.imsi_val[1];
			iccid.id_num[4]=g_sim_status.imsi.imsi_val[2];iccid.id_num[5]=g_sim_status.imsi.imsi_val[3];
			iccid.id_num[6]=g_sim_status.imsi.imsi_val[4];iccid.id_num[7]=g_sim_status.imsi.imsi_val[5];
			iccid.id_num[8]=g_sim_status.imsi.imsi_val[6];iccid.id_num[9]=g_sim_status.imsi.imsi_val[7];

*/
	        SCI_TRACE_LOW("SIMAT::iccid(1-10):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
			iccid.id_num[0],iccid.id_num[1],iccid.id_num[2],iccid.id_num[3],
			iccid.id_num[4],iccid.id_num[5],iccid.id_num[6],iccid.id_num[7],
			iccid.id_num[8],iccid.id_num[9]);
			
			SCI_MEMCPY(des_parameter.iccid,iccid.id_num + (DPHONE_SIM_ICCID_NUM_LEN-DPHONE_DES_ICCID_NUM),DPHONE_DES_ICCID_NUM);
	        
	        /*
			uint8 iccid_array[DPHONE_SIM_ICCID_NUM_LEN];
			
	        //SIM_GetIccid(iccid_array);
	        //modified 
	        MNSIM_GetICCID(iccid_array); //modified by alex li on 2005 10 17
	        //end
	        
	        SCI_TRACE_LOW("SIMAT::iccid(1-10):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x\n",
			iccid_array[0],iccid_array[1],iccid_array[2],iccid_array[3],
			iccid_array[4],iccid_array[5],iccid_array[6],iccid_array[7],
			iccid_array[8],iccid_array[9]);
	 
	        SCI_MEMCPY(des_parameter.iccid,iccid_array + (DPHONE_SIM_ICCID_NUM_LEN-DPHONE_DES_ICCID_NUM),DPHONE_DES_ICCID_NUM);
	        */
	        
	        SCI_MEMCPY(des_parameter.rand,com->text_str.text_ptr,DPHONE_DATANG_V3_RAND_NUM);

	        des_parameter.card_type = DATANG_V3_JIANGSU_CARD;
	        
	        SCI_TRACE_LOW("SIMAT::it is the datang v3 card.");
	        SCI_TRACE_LOW("SIMAT::Rand(1-20):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x \n",
				com->text_str.text_ptr[0],com->text_str.text_ptr[1],com->text_str.text_ptr[2],com->text_str.text_ptr[3],
				com->text_str.text_ptr[4],com->text_str.text_ptr[5],com->text_str.text_ptr[6],com->text_str.text_ptr[7],
				com->text_str.text_ptr[8],com->text_str.text_ptr[9],com->text_str.text_ptr[10],com->text_str.text_ptr[11],
				com->text_str.text_ptr[12],com->text_str.text_ptr[13],com->text_str.text_ptr[14],com->text_str.text_ptr[15],
				com->text_str.text_ptr[16],com->text_str.text_ptr[17],com->text_str.text_ptr[18],com->text_str.text_ptr[19]);
		}
		else
		{
			//SCI_SendEventToClient(MN_APP_SIMAT_SERVICE,SIMAT_EVENT_GET_INPUT,com);
       		return FALSE;
		}
		break;

	case WOQI_CARD:
			SCI_TRACE_LOW("woqi_card,text_str[0-4] len= %d,%2x,%2x,%2x,%2x,%2x",com->text_str.len,com->text_str.text_ptr[0],com->text_str.text_ptr[1],
			com->text_str.text_ptr[2],com->text_str.text_ptr[3],com->text_str.text_ptr[4]);
			
			SCI_TRACE_LOW("woqi_card,default_text[0-4] len=%d,%2x,%2x,%2x,%2x,%2x",com->default_text.len,com->default_text.text_ptr[0],com->default_text.text_ptr[1],
			com->default_text.text_ptr[2],com->default_text.text_ptr[3],com->default_text.text_ptr[4]);
		if(memcmp(com->default_text.text_ptr, default_woqi_rand_tag_str, DPHONE_WOQI_DATA_TAG_NUM) == 0)
		//if(DPHONE_WOQI_DEFAULT_TEXT_LENGTH == com->text_str.len)
		{
			//modified by alex li, on 2005 10 28 , if the card type is encrypted sim card, then set it.
	        MMI_SetNormalCardFlag(FALSE); 
	        //end
	        
	        
		    SCI_MEMCPY(des_parameter.rand,com->default_text.text_ptr + 4,DPHONE_WOQI_DES_RAND_NUM);
		    SCI_MEMCPY(des_parameter.key,com->default_text.text_ptr + 12,DPHONE_DES_KEY_NUM);
			des_parameter.card_type = WOQI_CARD;
			
		    SCI_TRACE_LOW("SIMAT::it is the woqi card.");
		    SCI_TRACE_LOW("SIMAT::Text(1-28):0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x 0x%2x \n",
				com->text_str.text_ptr[0],com->text_str.text_ptr[1],com->text_str.text_ptr[2],com->text_str.text_ptr[3],
				com->text_str.text_ptr[4],com->text_str.text_ptr[5],com->text_str.text_ptr[6],com->text_str.text_ptr[7],
				com->text_str.text_ptr[8],com->text_str.text_ptr[9],com->text_str.text_ptr[10],com->text_str.text_ptr[11],
				com->text_str.text_ptr[12],com->text_str.text_ptr[13],com->text_str.text_ptr[14],com->text_str.text_ptr[15],
				com->text_str.text_ptr[16],com->text_str.text_ptr[17],com->text_str.text_ptr[18],com->text_str.text_ptr[19],
				com->text_str.text_ptr[20],com->text_str.text_ptr[21],com->text_str.text_ptr[22],com->text_str.text_ptr[23],
				com->text_str.text_ptr[24],com->text_str.text_ptr[25],com->text_str.text_ptr[26],com->text_str.text_ptr[27]);  
		}
		else
		{
			//SCI_SendEventToClient(MN_APP_SIMAT_SERVICE,SIMAT_EVENT_GET_INPUT,com);
       		return FALSE;
		}
		break;
	
	default:
		//SCI_SendEventToClient(MN_APP_SIMAT_SERVICE,SIMAT_EVENT_GET_INPUT,com);
        return FALSE;
		break;
	}
    
    //call the des function from mmi to get the cryptograph result
    MMI_DphoneDes(&des_parameter,&des_result);
    //by far, the length of cryptograph result of all kinds of cards is 16 bytes
    SCI_ASSERT(des_result.cryp_len == DPHONE_DES_CRYP_NUM);
    
    SCI_TRACE_LOW("SIMAT::has get the cryptograph result.");
    
    switch(MMI_GetDesSimCardType())
	{
	case DATANG_V2_JIANGSU_CARD:
        temp_text_str->len = des_result.cryp_len; //cryp result
        temp_text_str->dcs = com->text_str.dcs;
        SCI_MEMCPY(g_des_str_buf,des_result.cryp_array,DPHONE_DES_CRYP_NUM);
        SCI_MEMCPY(temp_text_str->text_ptr,g_des_str_buf,DPHONE_DES_RAND_NUM);
		//temp_text_str->text_ptr = g_des_str_buf;
		break;

	case DATANG_V21_LIAONING_CARD:
        temp_text_str->len = des_result.cryp_len; //cryp result
        temp_text_str->dcs = com->text_str.dcs;
        SCI_MEMCPY(g_des_str_buf,des_result.cryp_array,DPHONE_DES_CRYP_NUM);
        SCI_MEMCPY(temp_text_str->text_ptr,g_des_str_buf,DPHONE_DES_RAND_NUM);
		//temp_text_str->text_ptr = g_des_str_buf;
		break;
		
	case DATANG_V3_JIANGSU_CARD:
        temp_text_str->len = des_result.cryp_len + 1; //cryp result add IMEI ID
        temp_text_str->dcs = com->text_str.dcs;
        //the IMEI ID is put in the head of the buffer followed by the cryp result
        g_des_str_buf[0] = MMI_DphoneGetImeiNumber(); 
        
        SCI_TRACE_LOW("SIMAT::get the IMEI ID is %d.",g_des_str_buf[0]);
        SCI_MEMCPY(g_des_str_buf+1,des_result.cryp_array,DPHONE_DES_CRYP_NUM);
        SCI_MEMCPY(temp_text_str->text_ptr,g_des_str_buf,DPHONE_DES_RAND_NUM);
		
		break;

	case WOQI_CARD:
		SCI_MEMCPY(g_des_str_buf,default_woqi_data_tag_str,DPHONE_WOQI_DATA_TAG_NUM);
    	SCI_MEMCPY(g_des_str_buf+DPHONE_WOQI_DATA_TAG_NUM,des_result.cryp_array,DPHONE_DES_CRYP_NUM);
    	temp_text_str->len = des_result.cryp_len+DPHONE_WOQI_DATA_TAG_NUM;
    	temp_text_str->dcs = com->text_str.dcs;
    	SCI_MEMCPY(temp_text_str->text_ptr,g_des_str_buf,DPHONE_DES_RAND_NUM);
		//temp_text_str->text_ptr = g_des_str_buf;
		break;
	
	default:
		SCI_ASSERT(FALSE);
		break;
	}
    
    SCI_ASSERT(temp_text_str->text_ptr != NULL);
    SCI_TRACE_LOW("MMI_ProcessDesSimGetInput()::get des crypto result.");
    
    return TRUE;
}

/********************************************************************
//    DESCRIPTION
//    read the type of the des sim card from nv
//    Global resource dependence :
//    Author:yuhua.shi
//    Note:
***********************************************************************/
DPHONE_DES_SIMCARD_TYPE_E MMI_GetDesSimCardType(void)
{
	MN_RETURN_RESULT_E   return_val = MN_RETURN_FAILURE;
	
	uint8 card_type = NORMAL_CARD;
	
	return_val = MNNV_ReadItem(MMINV_DES_CARD_TYPE,sizeof(uint8),&card_type);
	
	if(   (MN_RETURN_FAILURE == return_val)
		|| (card_type >= MAX_CARD_NUM))
	{
		card_type = NORMAL_CARD;
		MNNV_WriteItem(MMINV_DES_CARD_TYPE, sizeof(uint8), &card_type);
	}
	SCI_TRACE_LOW("MMI::get des card type is %d", card_type);
	return (DPHONE_DES_SIMCARD_TYPE_E)card_type;
}


//如果SIM要PUK2会传这样的字符串上来
const STATIC uint8 get_puk2_text_info[DPHONE_IND_PUK2_TEXT_LEN] = 
			{0x8b,0xf7,0x8f,0x93,0x51,0x65,0x0,0x50,0x0,0x55,0x0,0x4b,0x0,0x32,0x5b,0xc6,0x78,0x1,0xff,0x1a};

SIMAT_GET_INPUT_CNF_T g_3ses_get_input_cnf;
			
//从用户输入获得的字符串判断是否正确
BOOLEAN   MMI_CheckIfGetPuk2(SIMAT_GET_INPUT_T*  p_get_input)
{
	uint8 buf_id=0;
	uint8 *share_buf=NULL;
	SCI_ASSERT(p_get_input != NULL);
	
	SCI_TRACE_LOW("MMI , DES , ENTER MMI_CheckIfGetPuk2");
	
	
	if(memcmp(p_get_input->text_str.text_ptr , get_puk2_text_info , 20) == 0)
	{
	   //字符串匹配
	   if (!SIMAT_OpenShareBuf(&buf_id))
		{
    		SCI_ASSERT(FALSE);
    		return FALSE;
		}
		
		g_3ses_get_input_cnf.text_str.dcs = p_get_input->text_str.dcs;
		
		g_3ses_get_input_cnf.result = SIMAT_TR_SUCCESS;
		share_buf=(uint8*)SIMAT_AllocFromShareBuf(buf_id,8);
	//	g_3ses_get_input_cnf.text_str.text_ptr = (uint8*)SIMAT_AllocFromShareBuf(buf_id,8);
	    SCI_MEMCPY(g_3ses_get_input_cnf.text_str.text_ptr,share_buf,strlen((char*)(share_buf)));
		
		SCI_TRACE_LOW("MMI , DES , ENTER MMI_CheckIfGetPuk2 , MATCHED ");
		
		return TRUE;
	}
	
	return FALSE;
	
}

//回复给STK层的函数
void  MMI_CnfPUK2( char* puk2)
{
	
	uint8 len ;
	
	SCI_ASSERT(puk2 != NULL);
	
	len = strlen(puk2);
	
	
	SCI_ASSERT(len >= 4  && len  <= 8) ;
	
	g_3ses_get_input_cnf.text_str.len = len;
	
	SCI_MEMCPY(g_3ses_get_input_cnf.text_str.text_ptr , (uint8*)puk2 , len );
	
	SIMAT_CnfGetInput(&g_3ses_get_input_cnf);
	
}

//判断是否是在加密过程中，该函数会在开始的时候，如果判断是加密卡，会调用
BOOLEAN MMI_Des_IsProcessDes(void)
{
   return g_is_process_des;
}

//设置是否还在加密过程中
void MMI_Des_SetProcessDes(BOOLEAN is_process)
{
	g_is_process_des=is_process;
}
