#ifndef __ADX_TLK2_H__
#define __ADX_TLK2_H__

/* ADX Talk (ADXT_CALC_WORK) */
#define ADXT_MAX_NCH    (2)     /* Maximum number of play channel */
#define ADXT_IBUF_XLEN  (36)    /* Size of Extra area in input buffer */

/* Output buffer size of ADX Talk (unit:sample) */
#define ADXT_OBUF_SIZE  (0x2000)
#define ADXT_OBUF_DIST  (0x2020)

/* Play method of ADX Talk (used 'ADXT_CALC_WORK' macro) */
#define ADXT_PLY_MEM    (0)     /* Play memory data */
#define ADXT_PLY_STM    (1)     /* Stream play from CD-ROM */

#define ADXT_CALC_IBUFSIZE0(nch, sfreq) (25000*(nch)*((sfreq)/1000)/44)

#define ADXT_CALC_IBUFSIZE(nch, nstm, sfreq)        \
    ((ADXT_CALC_IBUFSIZE0(nch, sfreq)*((nstm)+1)+2048)/2048*2048+ADXT_IBUF_XLEN)
#define ADXT_CALC_OBUFSIZE(nch)                     \
    (ADXT_OBUF_DIST*(nch)*sizeof(short))
#define ADXT_CALC_WORK(nch, stmflg, nstm, sfreq)    \
    (ADXT_CALC_IBUFSIZE(nch, nstm, sfreq)*(stmflg) + ADXT_CALC_OBUFSIZE(nch) + 32)

#define ADXT_USE_NCH    (1)
#define ADXT_WORKSIZE   (ADXT_CALC_WORK(ADXT_USE_NCH, ADXT_PLY_STM, 1, 22050))

/* ADX Talk */
/* Structure of ADX Talk object */
typedef struct _adx_talk
{
    char    used;
    char    stat;
    char    pmode;
    char    maxnch;
    void *  sjd;
    void *  stm;
    void *  rna;
    void *  sjf;
    void *  sji;
    void *  sjo[ADXT_MAX_NCH];
    char    ibuf;
    long    ibuflen;
    long    ibufxlen;
    short   obuf;
    long    obufsize;
    long    obufdist;
    long    svrfreq;
    short   maxsct;
    short   minsct;
    short   outvol;
    short   outpan[ADXT_MAX_NCH];
    long    maxdecsmpl;
    long    lpcnt;
    long    lp_skiplen;
    long    trp;
    long    wpos;
    long    mofst;
    short   ercode;
    long    edecpos;
    short   edeccnt;
    short   eshrtcnt;
    char    lpflg;
    char    autorcvr;
    char    fltmode;
    char    execflag;
    char    pstwait_flag;
    char    pstready_flag;
    char    pause_flag;
    void    *amp;
    void *  ampsji[ADXT_MAX_NCH];
    void *  ampsjo[ADXT_MAX_NCH];
    long    time_ofst;
} ADX_TALK;

typedef ADX_TALK *ADXT;

void ADXT_Init(void);
ADXT ADXT_Create(long maxnch, void *work, long worksize);
void ADXT_Destroy(ADXT adxt);
void ADXT_StartFname(ADXT adxt, char *fname);
void ADXT_Stop(ADXT adxt);
void ADXT_ExecServer(void);
void ADXT_EntryErrFunc(void (*func)(void *obj, char *msg), void *obj);

#endif
