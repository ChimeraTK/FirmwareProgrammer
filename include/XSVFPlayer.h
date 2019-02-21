/*
 * File:   XSVFPlayer.h
 * Author: pperek
 *
 * Created on 23 kwiecień 2015, 15:48
 */

#ifndef XSVFPLAYER_H
#define XSVFPLAYER_H

#include "MtcaProgrammerBase.h"
#include "XSVFPlayerConstants.h"
#include "XSVFPlayerInterface.h"

class XSVFPlayer {
public:
  XSVFPlayer(XSVFPlayerInterface &interface);
  XSVFPlayer(const XSVFPlayer &) = delete;
  XSVFPlayer &operator=(const XSVFPlayer &) = delete;
  virtual ~XSVFPlayer();

  void run(std::string file);

private:
  XSVFPlayerInterface &mInterface;
  FILE *mFile;
  SXsvfInfo sxvfInfo;
  int commandCounter;
  int totalCommandCounter;
  bool dummy_xsvf_player;

  void readByte(unsigned char *data);
  void readVal(lenVal *plv, short sNumBytes);
  short xsvfGetAsNumBytes(long lNumBits);

  int xsvfInitialize(SXsvfInfo *pXsvfInfo);
  int xsvfRun(SXsvfInfo *pXsvfInfo);
  void xsvfCleanup(SXsvfInfo *pXsvfInfo);
  int xsvfInfoInit(SXsvfInfo *pXsvfInfo);
  void xsvfInfoCleanup(SXsvfInfo *pXsvfInfo);
  int xsvfGotoTapState(unsigned char *pucTapState, unsigned char ucTargetState);
  void xsvfTmsTransition(short sTms);
  int xsvfPfDoCmd(unsigned char command, SXsvfInfo *pXsvfInfo);
  void xsvfPrintLenVal(lenVal *plv);
  int xsvfShift(unsigned char *pucTapState, unsigned char ucStartState,
                long lNumBits, lenVal *plvTdi, lenVal *plvTdoCaptured,
                lenVal *plvTdoExpected, lenVal *plvTdoMask,
                unsigned char ucEndState, long lRunTestTime,
                unsigned char ucMaxRepeat);
  void xsvfShiftOnly(long lNumBits, lenVal *plvTdi, lenVal *plvTdoCaptured,
                     int iExitShift);
  int xsvfBasicXSDRTDO(unsigned char *pucTapState, long lShiftLengthBits,
                       short sShiftLengthBytes, lenVal *plvTdi,
                       lenVal *plvTdoCaptured, lenVal *plvTdoExpected,
                       lenVal *plvTdoMask, unsigned char ucEndState,
                       long lRunTestTime, unsigned char ucMaxRepeat);

  // XSVF Function Prototypes
  int xsvfDoIllegalCmd(SXsvfInfo *pXsvfInfo); /* Illegal command function */
  int xsvfDoXCOMPLETE(SXsvfInfo *pXsvfInfo);
  int xsvfDoXTDOMASK(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSIR(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSIR2(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSDR(SXsvfInfo *pXsvfInfo);
  int xsvfDoXRUNTEST(SXsvfInfo *pXsvfInfo);
  int xsvfDoXREPEAT(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSDRSIZE(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSDRTDO(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSETSDRMASKS(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSDRINC(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSDRBCE(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSDRTDOBCE(SXsvfInfo *pXsvfInfo);
  int xsvfDoXSTATE(SXsvfInfo *pXsvfInfo);
  int xsvfDoXENDXR(SXsvfInfo *pXsvfInfo);
  int xsvfDoXCOMMENT(SXsvfInfo *pXsvfInfo);
  int xsvfDoXWAIT(SXsvfInfo *pXsvfInfo);
};

#endif /* XSVFPLAYER_H */
