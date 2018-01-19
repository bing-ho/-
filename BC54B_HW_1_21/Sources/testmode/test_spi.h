#ifndef TEST_SPI_INT
#define TESI_SPI_INT



#define SS1_EOF 0x00U                  /* Value of the empty character defined in the Empty character property. */
#ifndef __BWUserType_SS1_TComData
#define __BWUserType_SS1_TComData
  typedef byte SS1_TComData;           /* User type for communication. */
#endif

#ifndef __BWUserType_SS1_TError
#define __BWUserType_SS1_TError
  typedef union {
      byte err;
	  struct {
	  	byte OverRun  : 1;
		  byte RxBufOvf : 1;
		  byte FaultErr : 1;
	    }errName;
  } SS1_TError;
#endif

extern byte SS1_RecvChar(byte *Chr);
extern word SS1_GetCharsInRxBuf(void);
extern byte SS1_GetError(SS1_TError *Err);



extern void SS1_Init(void);

#endif













