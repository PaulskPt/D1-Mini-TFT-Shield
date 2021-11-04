/*
*  SAA.h
*  created by @PaulskPt 2021-10-27
*  SAA stands for 'Spotify Album Art'
* +-------------------+---------+
* | SAA_Sketch_Flags: | value:  |
* +-------------------+---------+
* | isPlaying         | 0x00    |
* +-------------------+---------+  
* | isImgShown        | 0x01    |
* +-------------------+---------+  
* | isImgLoadAgain    | 0x02    |
* +-------------------+---------+
*/
#include "Arduino.h"

#ifndef SAA_Sketch_Flags_H
#define SAA_Sketch_Flags_H

enum {
	SAA_ISPLAYING = 0,
	SAA_IMGSHOWN,
	SAA_IMGLOADAGAIN,
  SAA_BTN1PRESSED,
  SAA_BTN2PRESSED
};
/*
#define SAA_ISPLAYING    0x00
#define SAA_IMGSHOWN     0x01
#define SAA_IMGLOADAGAIN 0x02
#define SAA_BTNPRESSED   0X03
*/

class SAA {

// 'CPOS' stands for 'Currently Playing On Spotify'
private: 
  uint8_t SAA_Sketch_Flags;
  uint16_t SAA_Spotify_Status;
  uint8_t SAA_Nr_Of_Flags = 5;
  long dispCPOS_previous;
  long dispCPOS_elapsed;
  long dispCPOS_loopnr;
  


public:
  SAA();
  ~SAA();

  uint8_t  clrAll();
  uint16_t clrSpotifyStatus();
  
  //uint8_t  setSAAstatusBit(uint8_t status);
  uint8_t  setFlag(uint8_t bitnr);
  //uint8_t  clearSAAstatusBit(uint8_t status);
  uint8_t  clrFlag(uint8_t bitnr);

  uint8_t  getStatus();
  uint8_t  getFlag(uint8_t bitnr);
  uint8_t  getNrFlags();
  String   getFlagName(uint8_t flag);

  bool IsPlaying();
  bool IsImgShown();
  bool IsImgLoadAgain();
  
  void setSpotifyStatus(uint16_t value);
  uint16_t getSpotifyStatus();
  
  void setCPOS_previous(long mils);
  long getCPOS_elapsed();

  void setCPOS_loopnr(long nr);
  long getCPOS_loopnr();

  SAA *thisPtr = nullptr;
};

extern SAA SAAhandler;

#endif // SAAstatus_H
