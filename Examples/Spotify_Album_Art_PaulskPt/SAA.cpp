/*
* SAA.cpp
* created by @PaulskPt 2021-10-27
* SAA stands for 'Spotify Album Art'
*
*/

#include "SAA.h"

// Create a class instance to be used by the sketch (defined as extern in header)
SAA SAAhandler;


/***************************************************************************************
** Function name:           SAA
** Description:             Constructor
***************************************************************************************/
SAA::SAA(){
  // Setup a pointer to this class for static functions
  thisPtr = this;
}

/***************************************************************************************
** Function name:           SAA
** Description:             Destructor
***************************************************************************************/
SAA::~SAA(){
  // Bye
}

/***************************************************************************************
** Function name:           clearAll
** Description:             clear all of the SAA_Sketch_Flags
***************************************************************************************/
uint8_t SAA::clrAll(){
	SAA_Sketch_Flags = 0;
	return SAA_Sketch_Flags;
}

/***************************************************************************************
** Function name:           clearSpotifyStatus
** Description:             clear the value of SAA_Spotify_Status
***************************************************************************************/
uint16_t SAA::clrSpotifyStatus(){
	SAA_Spotify_Status = 0;
	return SAA_Spotify_Status;
}

/***************************************************************************************
** Function name:           setFlag
** Description:             set the Flag indicated by parameter bitnr
***************************************************************************************/
uint8_t SAA::setFlag(uint8_t bitnr){
  //SAAstatus |= status;
  SAA_Sketch_Flags |= 1UL << bitnr;
  return SAA_Sketch_Flags;
}

/***************************************************************************************
** Function name:           clrFalg
** Description:             clear the Flag indicated by paramter bitnr
***************************************************************************************/
uint8_t SAA::clrFlag(uint8_t bitnr){
  //SAAstatus &= status;
  SAA_Sketch_Flags &= ~(1UL << bitnr);
  return SAA_Sketch_Flags;
}

/***************************************************************************************
** Function name:           getStatus
** Description:             return the SAA_Sketch_Flags value
***************************************************************************************/
uint8_t SAA::getStatus(){
  return SAA_Sketch_Flags;
}
  
  /***************************************************************************************
** Function name:           getFlag
** Description:             return the status of the Flag requested
***************************************************************************************/
uint8_t SAA::getFlag(uint8_t bitnr){
  // was: return SAAstatus & bitnr;	
  return (SAA_Sketch_Flags >> bitnr) & 1U;
}

/***************************************************************************************
** Function name:           getNrFlags
** Description:             return the number of Flags currently defined
***************************************************************************************/
uint8_t SAA::getNrFlags(){
	return SAA_Nr_Of_Flags;
}

/***************************************************************************************
** Function name:           getFlagName
** Description:             return a text string for the Flag reguested
**                          This is used by the listFlags() function
***************************************************************************************/
String SAA::getFlagName(uint8_t flag){
	switch (flag) {
	  case SAA_ISPLAYING:
	    return "IsPlaying     ";
	    break;
	  case SAA_IMGSHOWN:
	    return "ImageShown    ";
	    break;
	  case SAA_IMGLOADAGAIN:
	    return "ImageLoadAgain";
	    break;
    case SAA_BTNPRESSED:
      return "ButtonPressed ";
      break;
	  default:
	    return "unknown       ";
	    break;
	}
}

/***************************************************************************************
** Function name:           IsPlaying
** Description:             return the status of the flag SAA_ISPLAYING
***************************************************************************************/
bool SAA::IsPlaying(){  
  // was: (SAAstatus & SAA_ISPLAYING > 0)
  if ((SAA_Sketch_Flags >> SAA_ISPLAYING) & 1U > 0)
	  return true;
  return false;
}

/***************************************************************************************
** Function name:           IsImgShown
** Description:             return the status of the flag SAA_IMGSHOWN
***************************************************************************************/
bool SAA::IsImgShown(){
  // was: (SAAstatus & SAA_IMGSHOWN > 0)
  if ((SAA_Sketch_Flags >> SAA_IMGSHOWN) & 1U > 0)
	  return true;
  return false;
}

/***************************************************************************************
** Function name:           IsImgLoadAgain
** Description:             return the status of the flag 	SAA_IMGLOADAGAIN
***************************************************************************************/
bool SAA::IsImgLoadAgain(){
  // was: (SAAstatus & SAA_IMGSHOWN > 0)
  if ((SAA_Sketch_Flags >> SAA_IMGLOADAGAIN) & 1U > 0)
	  return true;
  return false;
}

/***************************************************************************************
** Function name:           setSpotifyStatus
** Description:             store the latest received Spotify Status
***************************************************************************************/
void SAA::setSpotifyStatus(uint16_t value){
	SAA_Spotify_Status = value;
}
	
/***************************************************************************************
** Function name:           getSpotifyStatus
** Description:             get the value of the latest received and store Spotify Status
**                          This function is not used (yet). It is available for future
**                          functionality expansion.
***************************************************************************************/
uint16_t SAA::getSpotifyStatus(){
	return SAA_Spotify_Status;
}

/***************************************************************************************
** Function name:           setCPOS_previous
** Description:             set the value of setCPOS_previous to the time moment of
**                          the current iteration of displayCurrentlyPlayingOnScreen()
***************************************************************************************/
void SAA::setCPOS_previous(long mils){
	dispCPOS_previous = mils;
}

/***************************************************************************************
** Function name:           getCPOS_elapsed
** Description:             calculate and return the time elapsed (in mSeconds)
**                          between two iterations of displayCurrentlyPlayingOnScreen()
***************************************************************************************/
long SAA::getCPOS_elapsed(){
	dispCPOS_elapsed = (millis() - dispCPOS_previous) / 1000;
	return dispCPOS_elapsed;
}

/***************************************************************************************
** Function name:           setCPOS_loopnr
** Description:             function to set the value of the current iteration number
**                          of the displayCurrentlyPlayingOnScreen() function
***************************************************************************************/
void SAA::setCPOS_loopnr(long nr){
  dispCPOS_loopnr = nr;
}

/***************************************************************************************
** Function name:           getCPOS_loopnr
** Description:             return with the value of the loop number
**                          of the displayCurrentlyPlayingOnScreen() iteration
***************************************************************************************/
long SAA::getCPOS_loopnr(){
  return dispCPOS_loopnr;
}
