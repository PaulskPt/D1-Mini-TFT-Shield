/*******************************************************************
    Displays Album Art on a ST7789

    This example could easily be adapted to any Adafruit GFX
    based screen.

    NOTE: You need to get a Refresh token to use this example
    Use the getRefreshToken example to get it.

    Parts:
    D1 Mini ESP8266 * - http://s.click.aliexpress.com/e/uzFUnIe
    ST7789 TFT Screen

 *  * = Affilate

    If you find what I do useful and would like to support me,
    please consider becoming a sponsor on Github
    https://github.com/sponsors/witnessmenow/


    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow
 **************************************************************************
 * Note by @PaulskPt on 2021-10-30:
 * This script uses a font for Latin 1 and Latin 1 extended letters,
 * as used in various languages like Turkish, Portuguese, German. The font was
 * selected and downloaded from fonts.google.com/noto as a .zipped .ttf 
 * font family,
 * next converted (see: https://processing.org) to be used in this sketch,
 * using the 'tools/create font.pde sketch from Bodmer's TFT_eSPI
 * repo on GitHub.
 *************************************************************************/
#ifndef USE_PSK_SECRETS
#define USE_PSK_SECRETS (1)   // Comment-out when defining the credentials inside this sketch
#endif

#ifndef SHOW_ASCENT_DESCENT
#define SHOW_ASCENT_DESCENT  (1) // See: /TFT_eSPI/Extensions/Smooth_Fonts/Smooth_font.h
#endif 

#ifndef USE_LITTLEFS
#define USE_LITTLEFS (1)
#endif
/*
 *  created to use in TFT_eSPI/Extensions/Smooth_Fonts.h and Smooth_Fonts.c 
 * for PT market (using special UTF-8 characters which normally 
 * are not isplayed via TFT_eSPI library functions
 */
#ifndef USE_SPOTIFY_MARKET
#define USE_SPOTIFY_MARKET (1)  
#endif
// ----------------------------
// Standard Libraries
// ----------------------------
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define FS_NO_GLOBALS

#ifdef USE_LITTLEFS
#include <LittleFS.h>  // Comment-out if you want to use the deprecated SPIFFS filesystem
#else
#include <FS.h>
#endif

//#include "Free_Fonts.h" // Include the header file attached to this sketch

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------
#include <SPI.h>
//#include <TFT_eSPI.h>
#include <TFT_eSPI.h>
// Graphics and font library for ST7789 driver chip

// Can be installed from the library manager (Search for "eSPI")
// https://github.com/Bodmer/TFT_eSPI

#include <SpotifyArduino.h>
// Library for connecting to the Spotify API

// Install from Github
// https://github.com/witnessmenow/arduino-spotify-api

#define ARDUINOJSON_DECODE_UNICODE 1  // Use Unicode decoding. See: https://arduinojson.org/v6/api/config/decode_unicode/    
// Added by @paulsk 20021-10-23
#include <ArduinoJson.h>
// Library used for parsing Json from the API responses

// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson

#include <TJpg_Decoder.h>
// Library for decoding Jpegs from the API responses

// Search for "tjpg" in the Arduino Library manager
// https://github.com/Bodmer/TJpg_Decoder

bool my_debug = true;  // Set to true if you want debug info to be printed to the Monitor/REPL

/*
 * SAA stands for Spotify Album Art
 * This is a class to hold and manage various flags during the run of this sketch
 * Currently the flags SAA_ISPLAYING, SAA_IMGSHOWN and SAA_IMGLOADAGAIN are defined
 */
#include "SAA.h" 
//SAA SAAhandler(); // The instance of the SAA class is already created in SAA.cpp

// See: convertUnicode()
#define SPOT_ARTIST 0
#define SPOT_TRACK  1
#define SPOT_ALBUM  2

#ifdef USE_PSK_SECRETS
#include "secrets.h"
#endif
//------- Replace the following! ------
#ifdef USE_PSK_SECRETS
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[]         = SECRET_SSID;    // Network SSID (name)
char password[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)
char clientId[]     = SECRET_CLIENT_ID;      // Your client ID of your spotify APP
char clientSecret[] = SECRET_CLIENT_SECRET;    // Your client Secret of your spotify APP (Do Not share this!)
#else
char ssid[] = "SSID";         // your network SSID (name)
char password[] = "password"; // your network password
char clientId[] = "56t4373258u3405u43u543"; // Your client ID of your spotify APP
char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)
#endif
// Country code, including this is advisable
/*
* 2021-10-22_D1_Spotify_:89:24: error: cannot convert 'const char [3]' to 'processCurrentlyPlaying' {aka 'void (*)(CurrentlyPlaying)'}
*    89 | #define SPOTIFY_MARKET "IE"
*      |                        ^~~~
*      |                        |
*      |                        const char [3]
* C:\Users\pauls\Documenten\Arduino\2021-10-22_D1_Spotify_\2021-10-22_D1_Spotify_.ino:328:69: note: in expansion of macro 'SPOTIFY_MARKET'
*  328 |     CurrentlyPlaying currentlyPlaying = spotify.getCurrentlyPlaying(SPOTIFY_MARKET);
*      |                                                                     ^~~~~~~~~~~~~~
*/
//#define SPOTIFY_MARKET "IE"
const char *SPOTIFY_MARKET = "PT"; 

//#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"

//------- ---------------------- ------

// file name for where to save the image.
#define ALBUM_ART "/album.jpg"

// so we can compare and not download the same image if we already have it.
String lastAlbumArtUrl;

WiFiClientSecure client;
SpotifyArduino spotify(client, clientId, clientSecret, SPOTIFY_REFRESH_TOKEN);
// You might want to make this much smaller, so it will update responsively

unsigned long delayBetweenRequests = 30000; // Time between requests (30 seconds)
unsigned long requestDueTime;               //time when request due

TFT_eSPI tft = TFT_eSPI();

#define btn1   D3  // Button for calling spotify.getCurrentlyPlaying()
#define btn2   D8  // idem

/*
* This next function will be called during decoding of the jpeg file to
* render each block to the Display.  If you use a different display
* you will need to adapt this function to suit.
*/
bool displayOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
//bool displayOutput(int8_t x, int8_t y, uint8_t w, uint8_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}

int displayImage(char *albumArtUrl) {
  /*
  * In this example I reuse the same filename
  * over and over, maybe saving the art using
  * the album URI as the name would be better
  * as you could save having to download them each
  * time, but this seems to work fine.
  * tft.fillScreen(TFT_BLACK);
  */
#ifdef USE_LITTLEFS
  if (LittleFS.exists(ALBUM_ART) == true) {
    Serial.println(F("Removing existing image"));
    LittleFS.remove(ALBUM_ART);
  }
  fs::File f = LittleFS.open(ALBUM_ART, "w+");
  if (!f) {
    Serial.println(F("file open failed"));
    return -1;
  }
  bool gotImage = spotify.getImage(albumArtUrl, &f);

  // Make sure to close the file!
  f.close();
#else
  if (SPIFFS.exists(ALBUM_ART) == true) {
    Serial.println(F("Removing existing image"));
    SPIFFS.remove(ALBUM_ART);
  }
  fs::File f = SPIFFS.open(ALBUM_ART, "w+");
  if (!f) {
    Serial.println(F("file open failed"));
    return -1;
  }

  bool gotImage = spotify.getImage(albumArtUrl, &f);

  // Make sure to close the file!
  f.close();
#endif

  if (gotImage) {
    Serial.print(F("displayImage(): "));
    Serial.println(F("Got Image"));
    tft.fillRect(0,0,tft.width(),160, TFT_BLACK); // Clear the upper part of tft 
    delay(1);
    return TJpgDec.drawFsJpg(45, 10, ALBUM_ART);
  } else {
    return -2;
  }
}

/*
*void printCurrentlyPlayingToSerial(CurrentlyPlaying currentlyPlaying)
*{
*  if (!currentlyPlaying.error)
*  {
*    Serial.println("--------- Currently Playing ---------");
*
*
*    Serial.print("Is Playing: ");
*    if (currentlyPlaying.isPlaying)
*    {
*      Serial.println("Yes");
*    } else {
*      Serial.println("No");
*    }
*
*    Serial.print("Track: ");
*    Serial.println(currentlyPlaying.trackName);
*    Serial.print("Track URI: ");
*    Serial.println(currentlyPlaying.trackUri);
*    Serial.println();
*
*    Serial.print("Artist: ");
*    Serial.println(SpotifyArtist.artistName);
*    Serial.print("Artist URI: ");
*    Serial.println(SpotifyArtist.artistUri);
*    Serial.println();
*
*    Serial.print("Album: ");
*    Serial.println(currentlyPlaying.albumName);
*    Serial.print("Album URI: ");
*    Serial.println(currentlyPlaying.albumUri);
*    Serial.println();
*
*    // will be in order of widest to narrowest
*    // currentlyPlaying.numImages is the number of images that
*    // are stored
*    for (int i = 0; i < currentlyPlaying.numImages; i++) {
*      Serial.println("------------------------");
*      Serial.print("Album Image: ");
*      Serial.println(currentlyPlaying.albumImages[i].url);
*      Serial.print("Dimensions: ");
*      Serial.print(currentlyPlaying.albumImages[i].width);
*      Serial.print(" x ");
*      Serial.print(currentlyPlaying.albumImages[i].height);
*      Serial.println();
*    }
*
*    Serial.println("------------------------");
*  }
*}
*/

int NrOfLatin1Chars = 0;

/*
 *  @brief
 *  Function convertUnicode()
 *  Param: 
 *  - String unicodeStr, input string
 *  Return: string
 
 *  Added by @paulsk. Copied from:
 *  https://forum.arduino.cc/t/arduino-and-unicode-strings-like-u00e1/115990/8
 *  the last example in the discussion. Adapted for this sketch by @paulsk 2021-10-24
*/
String convertUnicode(String unicodeStr){
  String out = "";
  int len = unicodeStr.length();

  if (my_debug){
    Serial.print(F("convertUnicode(): length of parameter "));
    Serial.print("\'");
    Serial.print(F("unicodeStr"));
    Serial.print("\' = ");
    Serial.println(len);
    Serial.print(F("parameter contents: "));
    Serial.print("\'");
    Serial.print(unicodeStr);
    Serial.println("\'");
  }
  char iChar, iChar2, iChar3;
  char* error;
  for (int i = 0; i < len; i++){
     iChar = unicodeStr[i];
     if (iChar == 0xc3){
      out += iChar;
      i++;
      iChar = unicodeStr[i];
      out += iChar;
      NrOfLatin1Chars += 1; // Increase the count for received Latin-1 group characters
     }
     else{
       if(iChar == '\\'){ // got escape char     \\u0xe7
         iChar = unicodeStr[++i];
         if(iChar == 'u'){ // got unicode hex
           NrOfLatin1Chars += 1;  // Increase the nr of unicode characters received
           char unicode[6];
           unicode[0] = '0';
           unicode[1] = 'x';
           for (int j = 0; j < 4; j++){
             iChar = unicodeStr[++i];
             unicode[j + 2] = iChar;
           }
           long unicodeVal = strtol(unicode, &error, 16); //convert the string
           out += (char)unicodeVal;
         } else if(iChar == '/'){
           out += iChar;
         } else if(iChar == 'n'){
           out += '\n';
         }
       } else {
        /*
         if (my_debug){
           Serial.print("iChar = 0x");
           Serial.print(iChar, HEX);
           Serial.print(" = ");
           Serial.println(iChar);
         }
         */
         out += iChar;
       }
     }
  }
  if (my_debug){
    Serial.print(F("Returning with: "));
    Serial.println(out);
  }
  return out;
}

/*
 *  @brief
 *  Function ConvUpperToLower()
 *  created by @paulsk 2021-10-23
 *  Params: 
 *  - String in, input string
 *  - int ata, value indicating what the function is to handle: artist, track or album 
 *  - boolean convert_all, if true: convert all 2nd and more letters of each word to lower case. If false don't translate,
 *  e.g.: special writing of artist or band names like 'GOWNboys'
 *  Return: string
 *  We could also use: s1.toLowerCase() but this does not provide what I want to achieve
 */
String ConvUpperToLower(String in, int ata, bool convert_all = false){
  int le = in.length();
  if (my_debug){
    Serial.print(F("ConvUpperToLower(): received to convert: "));
    Serial.print("\'");
    Serial.print(in);
    Serial.println("\'");
  }
  String io, io2 = "";
  String s, s2 = "";
  char e = '\0';
  char c, c2, co;
  int n, n2 = 0;

  if (my_debug){
    Serial.print(F("Length of parameter "));
    Serial.print("\'in\': ");
    Serial.println(le);
    Serial.print(F("Checking need for conversion of "));
    if (ata == SPOT_ALBUM)
      Serial.print(F("Album"));
    else if (ata == SPOT_ARTIST)
      Serial.print(F("Artist"));
    else if (ata == SPOT_TRACK)
      Serial.print(F("Track"));
    Serial.println(F(" name"));
  }
  if (le > 0){
    n = in.indexOf(" (feat.");   // look for '<spc>(feat. ...)'
    if (n >= 0){  // we have combo of '<spc>(feat.'
      return in.substring(0,n);  // cut both including rest
    }
    for (int i = 0; i < le; i++){
      c = in[i];
      if (i == 0){
        if (c == 0xc3){ // Do we have a Latin-1 letter?
          io += c;  // yes, copy 1st byte to output 
          if (my_debug){
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] character value to put into io: = 0x"));
            Serial.println(c, HEX);
          }
          i++;
          c2 = in[i];  // load the 2nd byte
          if (my_debug){  
            Serial.print(F("the value of the 2nd byte = 0x"));
            Serial.println(c2, HEX);
          }
          c2 |= 0xe0;  // perform a bitwise OR between byte 2 and (0xc0 + 0x20) = 0xe0 to get the correct Latin-1 value
          io += c2;  // put the byte in output
          if (my_debug){  
            Serial.println(F("We have an ASCII Latin-1 supplement letter"));
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] The value of the character = 0x"));
            Serial.println(c2, HEX);
          }
        }
        else {
          io += c; // A non Latin-1 letter: put immediately to the output
        }
      }
      else{
        if (c == 0x20){ // When encountering a word space 
          io += c;
          if (i+1 < le){
            i++;
            c = in[i];
            if (c >= 0xc0 && c <= 0xff){  // check if we need to convert the first letter of the next word
              i--; // Yes, we need to convert this unicode value. Correct the index value.
              c = in[i]; // get the previous character again.
            }
            else{
              // copy also the first char of the next word which could be an upper case (OK for the 1st letter)
              io += c;
            }
          }
          else {
            break;  // we are done!
          }
        }
        else if (c >= 0x41 && c <= 0x5a){ // A - Z
          if (my_debug){
            Serial.print(F("Char between A and Z: "));
            Serial.println(String(c));
          }
          if (ata == SPOT_ARTIST && !convert_all){
            io += c;  // Add unchanged to output
            if (my_debug){
              Serial.print(F("not converted to lower case because of flag MAKE_ALL_LOWERCASE = "));
              Serial.println(convert_all);
            }
          }
          else{
            c |= 0x20;  // convert to LowerCase
            io += c;
            if (my_debug){
              Serial.print(F("converted to: "));
              Serial.print(String(c));
              Serial.print(F(" or: "));
              Serial.println(c, HEX);
            }
          }
        }
        else if (c >= 0x5b && c <= 0x7f){
          io += c; // add character to output
        }
		/*
		*  Extract of remarks written on:  https://www.ascii-code.com
		*  ................................................................................................
		*  The next else if(...) filter is created with regard to the 
		*  remarks in https://www.ascii-code.com:
		*  <remark> "The extended ASCII codes (character code 128-255).
		*  There are seveeral different variations of the 8-bit ASCII table.
		*  The table is according to Windows-1252 (CP-1252) which is a superset of
		*  a superset of ISO 8859-1, also called ISO Latin-1, in terms of printable characters,
		*  but differs from the IANA's ISO-8859-1 by using displayable characters 
		*  rather than control characters in the 128 to 159 range."</remark>
		*  ................................................................................................
		*  Characters in the range 0xc0 ~ 0xff (192 - 255) = Latin-1 Supplement 128, 128, 
		*/
        else if (c == 0xc3){ // Do we have a Latin-1 letter?
          io += c;  // yes, copy 1st byte to output
          if (my_debug){  
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] character value to put into io: = 0x"));
            Serial.println(c, HEX);
          }
          i++;
          c2 = in[i];  // load the 2nd byte
          if (my_debug){  
            Serial.print(F("the value of the 2nd byte = 0x"));
            Serial.println(c2, HEX);
          }
          c2 |= 0xe0;  // perform a bitwise OR between byte 2 and (0xc0 + 0x20) = 0xe0 to get the correct Latin-1 value
          io += c2;  // put the byte in output
          if (my_debug){  
            Serial.println(F("We have an ASCII Latin-1 supplement character"));
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] The value of the character = 0x"));
            Serial.println(c2, HEX);
          }
        }
        else {
          if (my_debug){
            Serial.print(F("rest character to output. It"));
            Serial.print("\'");
            Serial.print(F("s value = "));
            Serial.println(c, HEX);
          }
          io += c; // accept all other characters
        }
      }
    }
    io += e; // add the end null
    if (my_debug){
      Serial.print("Returnvalue: \'");
      Serial.print(io);
      Serial.println("\'");
    }
  }
  return io;
}

void disp_line_on_repl(int type){
  int n = 10;
  for (int i = 0; i < n; i++){
    if (type == 0)
      Serial.print(F("--------"));
    else
      Serial.print(F("========"));
  }
  Serial.println();
}

/*
 *  @brief
 *  Function disp_artists()
 *  created by @paulsk 2021-10-23
 *  Function displays all artists on tft depending the state of param on_tft.
 *  Secondly, prints to Serial output depending the value of global boolean my_debug
 *  Params: 
 *  - bool on_tft
 *  - CurrentlyPlaying currentlyPlaying 
 *  - int xPos
 *  - int yLst[]
 *  - int yPosLstLe
 *  - int font
 *  Return: void
 * 
 */
void disp_artists(bool on_tft, CurrentlyPlaying currentlyPlaying, int xPos, int yLst[], int yPosLstLe, int font){
  int nr_of_artists = currentlyPlaying.numArtists;
  if (my_debug){
    disp_line_on_repl(0);
    Serial.print(F("disp_artists(): nr of artists = "));
    Serial.println(nr_of_artists);
  }
  ck_btn();  // check for button press
  if (SAAhandler.getFlag(SAA_BTNPRESSED))
    return;  // exit to loop() to handle an adhoc Spotify get data request
  if (nr_of_artists > 1){
    int j = 0; // index for yLst
    if (my_debug){
      Serial.println(F("contents of yLst: "));
    }
    for (int i = 0; i < nr_of_artists; i++){
      if (my_debug){
        Serial.print(F("yLst["));
        Serial.print(i);
        Serial.print(F("] = "));
        Serial.println(yLst[i]);
      }
      if (on_tft){
        tft.drawString(currentlyPlaying.artists[i].artistName, xPos, yLst[j], font);
        j++;
      }
      if (my_debug){
        Serial.print(F("Artist #"));
        Serial.print(i+1);
        Serial.print(F(" name = "));
        Serial.print("\'");
        Serial.print(currentlyPlaying.artists[i].artistName);
        Serial.println("\'");
      }
      if ((j >= 2) && nr_of_artists > yPosLstLe){
        delay(2000);  // Wait a bit
        clr_tft_down_part(xPos, yLst, yPosLstLe, font);
        j = 0; // reset index for yPosLst
      }
    }
  }
}

void show_new_album_art(SpotifyImage smallestImage, bool load_again = false){
  String newAlbum = String(smallestImage.url);
  String TAG = "show_new_album_art(): ";
  if (load_again || newAlbum != lastAlbumArtUrl) {  // lastAlbumArtUrl is a global var
    disp_line_on_repl(0);
    Serial.print(TAG);
    Serial.println(F("Updating Art"));
    char* my_url = const_cast<char*>(smallestImage.url);  // Modification by @paulsk 2021-10-20
    // convert from const char* to char* // see: https://stackoverflow.com/questions/833034/how-to-convert-const-char-to-char
    if (load_again)
      Serial.println(F("Forced to download Album Art again"));
    int displayImageResult = displayImage(my_url);  // was: int displayImageResult =displayImage(smallestImage.url);
    if (displayImageResult == 0) {
      lastAlbumArtUrl = newAlbum;
      SAAhandler.clrFlag(SAA_IMGLOADAGAIN);
      SAAhandler.setFlag(SAA_IMGSHOWN);
      listFlags();
    } else {
      Serial.print(TAG);
      Serial.print(F("failed to display image: "));
      Serial.println(displayImageResult);
    }
  }
}

void clr_tft_down_part(int xPos, int yPosLst[], int yPosLstLe, int font){
  // String blank = "                                   "; // 35 * <spc>
  int vt = (yPosLst[yPosLstLe-1] - yPosLst[0] -1);  // vt = 55
  if (my_debug){
    Serial.print(F("clr_tft_down_part(): vt = "));
    Serial.println(vt);
  }
  tft.fillRect(0, yPosLst[0], tft.width(), tft.height(), TFT_BLACK);
  //for (int i = 0; i < yPosLstLe; i++){
  //  tft.drawString(blank, xPos, yPosLst[i], font);
  //}
}

void listFlags(){
  Serial.println(F("Spotify Album Art (SAA) Flags"));
  Serial.println(F("+----------------+---------+"));
  Serial.println(F("|      Flag:     | Status: |"));
  Serial.println(F("+----------------+---------+"));
  for (uint8_t i = 0; i < SAAhandler.getNrFlags(); i++){
    Serial.print(F("| "));
    Serial.print(SAAhandler.getFlagName(i));
    Serial.print(F(" |    "));
    Serial.print(SAAhandler.getFlag(i));
    Serial.println(F("    |"));
  }
  Serial.println(F("+----------------+---------+"));
  
}

void ck_btn(void) {
  String btn = "";
  int btn_nr = 0;
  // Only check if flag not set yet
  if (!SAAhandler.getFlag(SAA_BTNPRESSED)){ 
    // If D3 is LOW, button is pressed
    if (digitalRead(D3) == LOW){  
      btn = "D3";
      btn_nr = 1;
    }
    // If D8 is HIGH, button is pressed
    if (digitalRead(D8) == HIGH){
      btn = "D8";
      btn_nr = 2;
    }
    if (btn.length() > 0){
      SAAhandler.setFlag(SAA_BTNPRESSED);  
      msg_to_tft("Button pressed. Ad hoc          data request on it\'s way");
      if (my_debug){
        Serial.print(F("<<<=== Button "));
        Serial.print(btn);
        Serial.print(F(" (= button "));
        Serial.print(btn_nr);
        Serial.println(F(") pressed. ===>>>")); 
        listFlags();
      }
    }
  }
}

void displayCurrentlyPlayingOnScreen(CurrentlyPlaying currentlyPlaying)
{
  int an_NrOfLatin1Chars  = 0;
  int tn_NrOfLatin1Chars  = 0;
  int abn_NrOfLatin1Chars = 0;
  String an, an0, tn, tn0, tn2, abn, abn0, abn2;
  int xPos, width_to_write, font, nr_of_artists = 0;  
  int an_le, tn_le, abn_le = 0;
  int yPosLst[] = {164, 190, 220};
  int yPosLstLe = 3; 
  String TAG = "displayCurrentPlayingOnScreen(): ";
  SpotifyImage smallestImage; // create an instance of the SpotifyImage object

  disp_line_on_repl(0);
  Serial.println(TAG);
  SAAhandler.setCPOS_loopnr(SAAhandler.getCPOS_loopnr() + 1); // increase loopnr
  Serial.print(F("Loop nr: "));
  Serial.println(SAAhandler.getCPOS_loopnr());
  Serial.print(F("Elapsed: "));
  Serial.print(SAAhandler.getCPOS_elapsed());  // Elapsed in seconds
  Serial.println(F(" Sec"));
  SAAhandler.setCPOS_previous(millis());
  tft.setRotation(3); // Needs to be rotated 3 to look correct on the shield
  tft.fillRect(0, 160, 240, 240, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, tft.color565(25,25,25));
  font = 12;
  NrOfLatin1Chars = 0;  // global value (defined in line above convertUnicode()
  ck_btn();  // Check if button D8 (btn2) or button 3 (btn1) is pressed.
  if (SAAhandler.getFlag(SAA_BTNPRESSED))
    return;  // exit to loop() to handle an adhoc Spotify get data request
  if (!currentlyPlaying.isPlaying){
    Serial.print(TAG);
    Serial.println(F("Spotify player is not playing track"));
    SAAhandler.clrFlag(SAA_ISPLAYING); // indicate not playing
    listFlags();
    return;
  }
  else{
    SAAhandler.setFlag(SAA_ISPLAYING);  // idicate is playing
    listFlags();
    // First show the image if new Album
    if (!SAAhandler.IsImgShown())
      tft.fillRect(0,0,240,160,TFT_BLACK); // Clear upper part of screen (to clear earlier error messsage(s)
     smallestImage = currentlyPlaying.albumImages[1];  // save this image to the global var to use in loop()

    if (SAAhandler.IsImgLoadAgain()){
      lastAlbumArtUrl = ""; 
      show_new_album_art(smallestImage, true);  // force a new download of the Album Art.
    }
    else{
      show_new_album_art(smallestImage);  // handle normal call (without forcing re-download)
    }

    // Handle artist name
  
    nr_of_artists = currentlyPlaying.numArtists;
  
    an0 = String(currentlyPlaying.artists[0].artistName); // an0 = 1st Artist Name
    an = convertUnicode(an0);  // try to convert unicode 
    an_NrOfLatin1Chars = NrOfLatin1Chars; // copy the value
    NrOfLatin1Chars = 0;
    an = ConvUpperToLower(an, SPOT_ARTIST, false);
    an_le  = an.length() - an_NrOfLatin1Chars;  
    if (my_debug){
      Serial.print(F("ConvUpperToLower() Returnvalue: "));
      Serial.print("\'");
      Serial.print(an);
      Serial.println("\'");
      Serial.print(F("an NrOfLatin1Chars = "));
      Serial.println(an_NrOfLatin1Chars);
      Serial.print(F("value an_le = "));
      Serial.println(an_le);
    }
  
    // Handle track name
    tn0 = String(currentlyPlaying.trackName);
    tn = convertUnicode(tn0);  // try to convert unicode 
    tn_NrOfLatin1Chars = NrOfLatin1Chars; // copy the value
    NrOfLatin1Chars = 0;
    tn_le  = tn.length() - tn_NrOfLatin1Chars;  // tn = Track Name
    tn = ConvUpperToLower(tn, SPOT_TRACK, false);
    tn2 = "";
    if (my_debug){
      Serial.print(F("ConvUpperToLower() Returnvalue: "));
      Serial.print("\'");
      Serial.print(tn);
      Serial.println("\'");
      Serial.print(F("tn NrOfLatin1Chars = "));
      Serial.println(tn_NrOfLatin1Chars);
      Serial.print(F("value tn_le = "));
      Serial.println(tn_le);
    }
    
    // handle album name
    abn0 = String(currentlyPlaying.albumName);
    abn2 = "";
    abn = convertUnicode(abn0);  // try to convert unicode 
    abn_NrOfLatin1Chars = NrOfLatin1Chars;  // copy the value
    NrOfLatin1Chars = 0;
    abn = ConvUpperToLower(abn, SPOT_ALBUM, false);
    abn_le  = abn.length() - abn_NrOfLatin1Chars;  // tn = Track Name
    if (my_debug){
      Serial.print(F("ConvUpperToLower() Returnvalue: "));
      Serial.print("\'");
      Serial.print(abn);
      Serial.println("\'");
      Serial.print(F("abn NrOfLatin1Chars = "));
      Serial.println(abn_NrOfLatin1Chars);
      Serial.print(F("value abn_le = "));
      Serial.println(abn_le);
    }
    
    xPos = 15;
    width_to_write = 24;
    if (tn_le > width_to_write)
      tn2 = tn.substring(0,width_to_write) + "...";
    else
      tn2 = tn;
   
    if (abn_le > width_to_write)
      abn2 = abn.substring(0,width_to_write) + "...";
    else
      abn2 = abn;
    if (my_debug){
      Serial.print(F("Width available to write on tft: "));
      Serial.println(width_to_write);
      Serial.print(F("length of album name is: "));
      Serial.println(abn_le);
      Serial.print(F("length of artist #1 name is: "));
      Serial.println(an_le);
      Serial.print(F("length of track name is: "));
      Serial.println(tn_le);
    }
  
    long run_init = millis();
    bool show_all_artists = true;
    bool show_artists = false;
    long loop_time = 0L;
    bool shown_single = false;
    unsigned long howmuch_to_loop = delayBetweenRequests - 5;  
    int elapsed = 0;
    if (my_debug){
      Serial.print(F("howmuch_to_loop = "));
      Serial.println(howmuch_to_loop);
    }
    if (nr_of_artists == 1)
      show_all_artists = false;
    else
      show_all_artists = true;
    
    if (!show_all_artists){
      if (!shown_single){
        shown_single = true;
        // We're only going to display ata (1st artist, track, album)
        tft.drawString(an ,  xPos, yPosLst[0], font);
        tft.drawString(tn2,  xPos, yPosLst[1], font);
		// Only show album name if it is different from the track name (check first 5 chars)
        if (abn2.substring(0,5) != tn2.substring(0,5))  
          tft.drawString(abn2, xPos, yPosLst[2], font);
      }
    }
    else{
      if (my_debug){
        Serial.println(F("Going to show all artists"));
      }
      // We're going to display in an alternate cycle:
      // a) track and album; b) all artists 
      while (true){
        if (my_debug){
          Serial.print(F("value flag show_artists = "));
          Serial.println(show_artists);
        }
        if (!show_artists){
          tft.drawString(tn2,  xPos, yPosLst[1], font);
          if (abn2 != tn2)  // Only show album name if it is different from the track name
            tft.drawString(abn2, xPos, yPosLst[2], font);
        }
        else{
          disp_artists(show_all_artists, currentlyPlaying, xPos, yPosLst, yPosLstLe, font);
        }
        delay(3000);
        loop_time = millis();
        elapsed = (loop_time - run_init);
        if (my_debug){
          Serial.print(F("elapsed = "));
          Serial.println(elapsed);
        }
        if ( elapsed >=  howmuch_to_loop )  // exit the outer loop after one minute
          break;
        else{
          ck_btn();
          if (SAAhandler.getFlag(SAA_BTNPRESSED))
            return;  // exit to loop() to handle an adhoc Spotify get data request
          show_artists = !show_artists; // flip bool
          if (my_debug){
            Serial.print(F("flag flipped. Value flag show_artists = "));
            Serial.println(show_artists);
          }
          clr_tft_down_part(xPos, yPosLst, yPosLstLe, font);
        }
      }
    }
  
    if (my_debug){
      Serial.print(F("artist name: "));
      Serial.print("\'");
      Serial.print(currentlyPlaying.artists[0].artistName);
      Serial.println("\'");
      Serial.print(F("track name: "));
      Serial.print("\'");
      Serial.print(currentlyPlaying.trackName);
      Serial.println("\'");
      Serial.print(F("album name: "));
      Serial.print("\'");
      Serial.print(currentlyPlaying.albumName);
      Serial.println("\'");
    }
  } // end-of if(CurrentlyPlaying.isPlaying)
  
  //    Serial.println(currentlyPlaying.trackName);
  //    Serial.print("Track URI: ");
  //    Serial.println(currentlyPlaying.trackUri);
  //    Serial.println();
  //
  //    Serial.print("Artist: ");
  //    Serial.println(SpotifyArtist.artists[0].artistName);
  //    Serial.print("Artist URI: ");
  //    Serial.println(SpotifyArtist.artists[0].artistUri);
  //    Serial.println();
  //
  //    Serial.print("Album: ");
  //    Serial.println(currentlyPlaying.albumName);
  //    Serial.print("Album URI: ");
  //    Serial.println(currentlyPlaying.albumUri);
  //    Serial.println();
  //
  //    // will be in order of widest to narrowest
  //    // currentlyPlaying.numImages is the number of images that
  //    // are stored
  //    for (int i = 0; i < currentlyPlaying.numImages; i++) {
  //      Serial.println("------------------------");
  //      Serial.print("Album Image: ");
  //      Serial.println(currentlyPlaying.albumImages[i].url);
  //      Serial.print("Dimensions: ");
  //      Serial.print(currentlyPlaying.albumImages[i].width);
  //      Serial.print(" x ");
  //      Serial.print(currentlyPlaying.albumImages[i].height);
  //      Serial.println();
  //    }
  //
  //    Serial.println("------------------------");

  //smallestImage = currentlyPlaying.albumImages[1];  // save this image to the global var to use in loop()
}

void msg_to_tft(String msg){
  int font = 12;
  tft.setRotation(3); // Needs to be rotated 3 to look correct on the shield
  tft.fillRect(0, 160, 240, 240, TFT_BLACK);  // was: tft.fillRect(0, 160, 240, 80, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, tft.color565(25,25,25));
  tft.drawString(msg,20,200, font);
  //delay(3000);
  // set SAA flag to load album image after calling this function
  SAAhandler.clrFlag(SAA_IMGSHOWN);
  SAAhandler.setFlag(SAA_IMGLOADAGAIN);
}

void heap_info(){
  Serial.print(F("heap_info(): Free Heap: "));
  Serial.println(ESP.getFreeHeap());
}

void setup() {

  Serial.begin(9600);  // was: 115200
  
  Serial.println(F("Setup():"));
  /*
  * Initialise LittleFS, if this fails try .begin(true)
  * NOTE: I believe this formats it though it will erase everything on
  * LittleFS already! In this example that is not a problem.
  * I have found once I used the true flag once, I could use it
  * without the true flag after that.
  */
#ifdef USE_LITTLEFS
  if (!LittleFS.begin()) {
    Serial.println(F("LittleFS initialisation failed!"));
#else
  if (!SPIFFS.begin()) {
    Serial.println(F("SPIFFS initialisation failed!"));
  }
#endif
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.print("\r\n");
  Serial.println(F("Initialisation done."));

  // Initialise the TFT
  tft.begin();
  tft.setRotation(3); // Needs to be rotated 3 to look correct on the shield
  tft.setTextColor(0xFFFF, 0x0000);
  //tft.setFreeFont(FSB9);   // Select Free Serif 9 point font, could use: tft.setFreeFont(&FreeSerif9pt7b);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true); // We need to swap the colour bytes (endianess)

  // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
  TJpgDec.setJpgScale(2);

  TJpgDec.setCallback(displayOutput);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Attempt to connect to Wifi network:
  Serial.print(F("Connecting Wifi: "));
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println(F("WiFi connected"));
  Serial.print(F("IP address: "));
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);

  // Only avaible in ESP8266 V2.5 RC1 and above
  //client.setFingerprint(SPOTIFY_FINGERPRINT);
  client.setInsecure();

  // If you want to enable some extra debugging
  // uncomment the "#define SPOTIFY_DEBUG" in ArduinoSpotify.h
  delay(100);
  if (my_debug){
    heap_info();
  }
  Serial.println(F("Refreshing Access Tokens"));
  if (!spotify.refreshAccessToken()) 
    Serial.println(F("Failed to get access tokens"));
  else
    Serial.println(F("Refreshed access token"));

  // Load a smooth font from Flash FS
  if (my_debug)
    Serial.println(F("loading font NotoRegular18 with LittleFS"));
  tft.loadFont("NotoRegular18", LittleFS);  // Original font file: NotoSansDisplay_Regular.ttf 
  // (Downloaded from: https://fonts.google.com/noto/specimen/Noto+Sans+Display);
  //  NotoSansDisplay_SemiCondensed-Regular.ttf     -- chosen corps 12
  
  // D8 mode needs to be set after tft.init
  pinMode(btn1, INPUT); // is pulled low, will be low when pressed
  pinMode(btn2, INPUT); // is pulled down, will be high when pressed
  
  SAAhandler.clrAll();  // clear all SAA defined flags
  SAAhandler.clrSpotifyStatus(); // clear the Spotify Status register
  SAAhandler.setCPOS_previous(millis()); // set the start time in 'previous'
  SAAhandler.setCPOS_loopnr(0);
  SAAhandler.clrFlag(SAA_BTNPRESSED);
}

String get_status(int status){
  if (status == -1)
    return "Connection failed";
  else if (status == 200)
    return "OK";
  else if (status == 204)
    return "Command sent";
  else if (status == 401)
    return "Bad or expired token";
  else if (status == 403)
    return "Bad OAuth request";
  else if (status == 429)
    return "This app has exceede its rate limits";
  else if (status == 500)
    return "Server error";
  else
    return "unknown status: "+String(status);
}

bool lStart = true;
unsigned long loop_start = millis();
unsigned long loop_current = 0;
unsigned long loop_elapsed = 0;

void loop() {
  uint8_t wifiStatus = client.status();  // client.connected();
  ck_btn();
  if (lStart || SAAhandler.getFlag(SAA_BTNPRESSED) || loop_elapsed > delayBetweenRequests)  // was: millis() > requestDueTime)
  {
    if (my_debug){
      disp_line_on_repl(1);
      listFlags();
      Serial.println(F("loop(): getting info on currently playing song:"));
    }
	  if (SAAhandler.getFlag(SAA_BTNPRESSED)){
  	  SAAhandler.clrFlag(SAA_BTNPRESSED);  // reset flag
      if (my_debug)
        Serial.println(F("A button has been pressed. Going to send a get Playing data request"));
        Serial.print(F("Elapsed: "));
        Serial.print(SAAhandler.getCPOS_elapsed());  // Elapsed in seconds
        Serial.println(F(" Sec"));
	  }
    if (lStart){
      Serial.print(F("WiFi status = "));
      Serial.println(wifiStatus);
    }
    lStart = false;  // lStart is needed to force a run of getCurrentlyPlaying at boot time
    heap_info();
    // Market can be excluded if you want e.g. spotify.getCurrentlyPlaying()
    int status = spotify.getCurrentlyPlaying(displayCurrentlyPlayingOnScreen, SPOTIFY_MARKET);
    if (my_debug){
      Serial.print(F("result of spotify.getCurrentPlaying(): "));
      Serial.println(status);
    }
    if (!SAAhandler.IsPlaying()){  // Check the flag
      msg_to_tft(F("Spotify result: not playing"));
    }
    else if (status != 200){
      SAAhandler.setSpotifyStatus(status); // copy the status into
      msg_to_tft(get_status(status));  // put (error) status onto tft
    }
  }
  loop_current = millis();  // was: requestDueTime = millis() + delayBetweenRequests;
  loop_elapsed = loop_current - loop_start;
  if (loop_elapsed > delayBetweenRequests)
    loop_start = millis();
}
