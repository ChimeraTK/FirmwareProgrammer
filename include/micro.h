/*****************************************************************************
* File:         micro.h
* Description:  This header file contains the function prototype to the
*               primary interface function for the XSVF player.
* Usage:        FIRST - PORTS.C
*               Customize the ports.c function implementations to establish
*               the correct protocol for communicating with your JTAG ports
*               (setPort() and readTDOBit()) and tune the waitTime() delay
*               function.  Also, establish access to the XSVF data source
*               in the readByte() function.
*               FINALLY - Call xsvfExecute().
*****************************************************************************/
#include "lenval.h"
#include <stdio.h>

#ifndef XSVF_MICRO_H
#define XSVF_MICRO_H

/* Legacy error codes for xsvfExecute from original XSVF player v2.0 */
#define XSVF_LEGACY_SUCCESS 1
#define XSVF_LEGACY_ERROR   0

/* 4.04 [NEW] Error codes for xsvfExecute. */
/* Must #define XSVF_SUPPORT_ERRORCODES in micro.c to get these codes */
#define XSVF_ERROR_NONE         0
#define XSVF_ERROR_UNKNOWN      1
#define XSVF_ERROR_TDOMISMATCH  2
#define XSVF_ERROR_MAXRETRIES   3   /* TDO mismatch after max retries */
#define XSVF_ERROR_ILLEGALCMD   4
#define XSVF_ERROR_ILLEGALSTATE 5
#define XSVF_ERROR_DATAOVERFLOW 6   /* Data > lenVal MAX_LEN buffer size*/
/* Insert new errors here */
#define XSVF_ERROR_LAST         7


/*****************************************************************************
* Define:       XSVF_SUPPORT_COMPRESSION
* Description:  Define this to support the XC9500/XL XSVF data compression
*               scheme.
*               Code size can be reduced by NOT supporting this feature.
*               However, you must use the -nc (no compress) option when
*               translating SVF to XSVF using the SVF2XSVF translator.
*               Corresponding, uncompressed XSVF may be larger.
*****************************************************************************/
#ifndef XSVF_SUPPORT_COMPRESSION
    #define XSVF_SUPPORT_COMPRESSION    1
#endif


typedef struct tagSXsvfInfo
{
    unsigned int readPointer;
    
    //int devDescriptor;

	/* XSVF status information */
    unsigned char   ucComplete;         /* 0 = running; 1 = complete */
    unsigned char   ucCommand;          /* Current XSVF command byte */
    long            lCommandCount;      /* Number of commands processed */
    int             iErrorCode;         /* An error code. 0 = no error. */

    /* TAP state/sequencing information */
    unsigned char   ucTapState;         /* Current TAP state */
    unsigned char   ucEndIR;            /* ENDIR TAP state (See SVF) */
    unsigned char   ucEndDR;            /* ENDDR TAP state (See SVF) */

    /* RUNTEST information */
    unsigned char   ucMaxRepeat;        /* Max repeat loops (for xc9500/xl) */
    long            lRunTestTime;       /* Pre-specified RUNTEST time (usec) */

    /* Shift Data Info and Buffers */
    long            lShiftLengthBits;   /* Len. current shift data in bits */
    short           sShiftLengthBytes;  /* Len. current shift data in bytes */

    lenVal          lvTdi;              /* Current TDI shift data */
    lenVal          lvTdoExpected;      /* Expected TDO shift data */
    lenVal          lvTdoCaptured;      /* Captured TDO shift data */
    lenVal          lvTdoMask;          /* TDO mask: 0=dontcare; 1=compare */

#ifdef  XSVF_SUPPORT_COMPRESSION
    /* XSDRINC Data Buffers */
    lenVal          lvAddressMask;      /* Address mask for XSDRINC */
    lenVal          lvDataMask;         /* Data mask for XSDRINC */
    lenVal          lvNextData;         /* Next data for XSDRINC */
#endif  /* XSVF_SUPPORT_COMPRESSION */
} SXsvfInfo;

extern int xsvfInitialize( SXsvfInfo* pXsvfInfo );
extern int xsvfRun( SXsvfInfo* pXsvfInfo );
/*****************************************************************************
* Function:     xsvfExecute
* Description:  Process, interpret, and apply the XSVF commands.
*               See port.c:readByte for source of XSVF data.
* Parameters:   none.
* Returns:      int - For error codes see above.
*****************************************************************************/
//extern int xsvfExecute();

#endif  /* XSVF_MICRO_H */

