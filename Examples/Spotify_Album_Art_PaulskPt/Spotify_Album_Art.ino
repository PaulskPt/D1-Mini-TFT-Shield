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
 * This script uses a font for Latin 1 and Latin 5 extended letters,
 * as used in various languages like Turkish, Portuguese, German. The font was
 * selected and downloaded from fonts.google.com/noto as a .zipped .ttf 
 * font family,
 * next converted (see: https://processing.org) to be used in this sketch,
 * using the 'tools/create font.pde sketch from Bodmer's TFT_eSPI
 * repo on GitHub.
 * This version has 26 functions.
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

#ifdef USE_FREE_FONTS
#undef USE_FREE_FONTS
#endif

#ifdef USE_FREE_FONTS
#include <Adafruit_GFX.h>
//#include "Free_Fonts.h" // Include the header file attached to this sketch
#include "FreeSans12pt7b.h"
#include "FreeSans18pt7b.h"
#endif

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
#define SPOTIFY_REFRESH_TOKEN "AQDCTOsj3YzLWnoDmHlTxU_qcqIAy9JuWPTz1B0YOM2TVSlKKwqAAIMs_9t8RXvRma3ijvVb8ockhpWM9kZjFNYFRtEuwQXeQFCrHz3HhepM1wiMq-S2sQERPfALG65s-qY"

//------- ---------------------- ------

// file name for where to save the image.
#define ALBUM_ART "/album.jpg"

int defaultFontSize = 18;

#ifndef USE_FREE_FONTS
String fontFile;
int  fontSize;
String defaultFontFile = "";
String fonts[3] = {{""},{""},{""}};
int NrOfFontFilesOnDisk = 0;
int fontFileIdx = 0;
#endif

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

#ifndef USE_FREE_FONTS
void listDir(fs::File &fs, const char * dirname, uint8_t levels){
  String fn;
  int n;
  Serial.printf("Listing directory: %s\r\n", dirname);

  fs::File root = LittleFS.open(dirname, "r");
  if(!root){
    Serial.println("- failed to open directory");
    return;
  }
  if(!root.isDirectory()){
    Serial.println(" - not a directory");
    return;
  }

  fs::File file = root.openNextFile();
  while(file){
    if(file.isDirectory()){
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if(levels){
          listDir(fs, file.name(), levels -1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      fn = String(file.name());
      n = fn.indexOf(".vlw");
      if (n >= 0)
        NrOfFontFilesOnDisk++;  // global int
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

int readIni(){
  String fof = " from .ini file";
  String f2, f3, f4;
  String s;
  int le, fs_le, n, n2, n3 = 0;
  long fntSz;
  int fntarrSz = 0;
  String path = "/saa.ini";
  String sErr = "Error: ";
  
  fs::File root = LittleFS.open("/", "r");
  if (!root || root.isDirectory()){
    listDir(root, "/", 1);
  }
  root.close();

  Serial.println(F("readIni():"));
  if (NrOfFontFilesOnDisk == 0){
    Serial.print(sErr);
    Serial.println(F("No font files on flash memory disk"));
    return -1;
  }
  else {
    Serial.print(F("Nr of font files on disk: "));
    Serial.println(NrOfFontFilesOnDisk);
  }
  Serial.print(F("Reading file: \'"));
  Serial.print(path);
  Serial.println(F("\'"));

  fs::File f = LittleFS.open(path, "r");
  if (!f || f.isDirectory()){
    Serial.println(F("failed to open file \'"));
    Serial.print(path);
    Serial.println(F("\' for reading"));
    return -1;
  }
  Serial.println("- read from file:");
  while(f.available()){
    f2 = f.readString();
  }
  f.close();
  le = f2.length();
  if (le == 0){
    Serial.print(sErr);
    Serial.print(F("failed to read fontFile name"));
    Serial.println(fof);
    fontFile = defaultFontFile;
    fontSize = defaultFontSize;
    return -1;
  }
  
  // Read the fontfile names into the fonts[] array.
  // The read string is of structure:
  // "<fontFile #1 name> ,<fontFile #1 name>, <fontFile #x name>, fontSize"
  f3 = f2; // make a copy so we leave f2 untouched
  for (int i = 0; i < NrOfFontFilesOnDisk; i++){
    f4 = f3;
    n = f4.indexOf(",");
    if (n > 0){
      // extract the fontFile name and put into fonts array
      fonts[i] = f4.substring(0,n);
      f3 = f4.substring(n+1);
    }
    else{
      // No more delimiters found
      break;
    }
  }
  // now extract the fontSize
  fntSz = f3.toInt();
  if (fntSz > 0){
    fontSize = int(fntSz);
  }
  else{
    fontSize = defaultFontSize;
  }
  fntarrSz = sizeof(fonts)/sizeof(fonts[0]);
  fontFile = fonts[0];
  if (my_debug){
    if (fntarrSz > 0){  
      Serial.print(F("Size of the fonts array: "));
      Serial.println(fntarrSz);
      Serial.println(F("fontFiles on disk: "));
      for (int i = 0; i < fntarrSz; i++){
        Serial.print(F("["));
        Serial.print(i);
        Serial.print(F("]"));
        Serial.print("\t");
        Serial.println(fonts[i]);
      }
    }
    else{
      Serial.print(sErr);
      Serial.println(F("fonts[] array empty. Exit"));
      return -1;
    }
    if (my_debug){
      Serial.print(F("Font size: "));
      Serial.println(fontSize);
      Serial.println();
    }
  }
  return 1;  
}

int chgFontFileIdx(int idx){
  int n;
  SAAhandler.clrFlag(SAA_BTN2PRESSED);
  if (idx >= 0 && idx <= NrOfFontFilesOnDisk){
    if (my_debug){
      Serial.print(F("Current font file index: "));
      Serial.println(fontFileIdx);
    }
    if (idx != fontFileIdx){
      tft.unloadFont();
      Serial.print(F("font file: "));
      Serial.print(fonts[fontFileIdx]);
      Serial.println(F(" unloaded"));
      fontFileIdx = idx;
      if (my_debug){
        Serial.print(F("font file index changed to: "));
        Serial.println(fontFileIdx);
      }
      n = openFontFile(idx);
      if (n > 0){
        fontFileIdx = idx; // set global var
        /*
        if (my_debug){
          Serial.print(F("Font file changed to: "));
          Serial.println(fonts[idx]);
        }
        */
      }
    }
  }
  return n;
}

int openFontFile(int idx){
  int n;
  if (idx >= 0 && idx <= NrOfFontFilesOnDisk){
    n = tft.loadFont(fonts[idx], LittleFS); 
  }
  if (n >= 0){
    fontFile = fonts[idx];  // set global var
    if (my_debug){
      Serial.print(F("Loading of fontfile: "));
      Serial.print("\'");
      Serial.print(fonts[idx]);
      Serial.print("\'");
      Serial.println(F(" successful"));
    }
    return 1;
  }
  else{  // Always print error state
    Serial.print(F("failed loading of fontfile: "));
    Serial.print("\'");
    Serial.print(fonts[idx]);
    Serial.println("\'");
    return -1;
  }
}
#endif

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
      NrOfLatin1Chars += 1; // Increase the count for received Latin-1 group letters
      // Not for the Latin-5 group letters, however they also come as a 2-byte pair,
      // but we put only one (morphed) byte in the output stream.
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

void testTurk(){
  static int specificUnicodes[] = {
  // Some Turkish codes, added by @PaulskPt on 2021-11-02
  0x011E, 0x0130, 0x015E, 0x011f, 0x131, 0x015F}; // Ğ, İ, Ş, ğ, ı, ş
  int xPos = 15;
  int yPosLst[] = {164, 190, 220};
  int yPosLstLe = 3;
  int font = 32;
  String s = "\xc4\x9E\x20\xc4\xB0\x20\xc5\x9E\x20\xc4\x9f\x20\xc4\xb1\x20\xc5\x9f"; // Works OK
  //                                                                    the code values are OK
  //String s = "Ğ İ Ş ğ ı ş";  // Works OK
  //String s =  "\\u0286\\u0304\\u0350\\u0287\\u0305\\u0351";  // Displays "\u286" literally !!!
  
  clr_tft_down_part(xPos, yPosLst, yPosLstLe);
  /*
  Serial.print(F("testTurk(): "));
  Serial.println(s);
  for (int i=0; i < s.length(); i++){
    Serial.print(F("char[ "));
    Serial.print(i);
    Serial.print(F("] = 0x"));
    Serial.println((int)s[i], HEX);
  }
  */
  Serial.println(F("see also on tft"));

  tft.drawString(s, xPos, yPosLst[1], 4);
  /*
  for (int i = 0; i < 6; i++){
    tft.drawChar(xPos+i, yPosLst[1], specificUnicodes[i], TFT_WHITE, TFT_BLACK, font);
  }
  */
  //tft.drawString(s, xPos, yPosLst[1]);
  delay(5000);
}

uint16_t handleTurk(String in, int i, char c){
  int j = i;
  char c2 = 0;
  Serial.print(F("handleTurk(): "));
  // Do we have a Latin-5 (Turkish) letter? (this is the 'lead byte')
  // The first byte comes from a range of codes reserved for use as lead bytes
  if (c == 0xc4 || c == 0xc5){
    Serial.println(F("We have an ASCII Latin-5 supplement letter"));
    if (my_debug){
      Serial.print(F("in["));
      Serial.print(i);
      Serial.print(F("] lead byte value = 0x"));
      Serial.println(c, HEX);
    }
    j++;
    c2 = in[j];
    if (my_debug){  
      Serial.print(F("in["));
      Serial.print(j);
      Serial.print(F("] the value of the 2nd byte = 0x"));
      Serial.println(c2, HEX);
    }
  }  //0x011E, 0x0130, 0x015E, 0x011f, 0x131, 0x015F}; // Ğ, İ, Ş, ğ, ı, ş
 

  if (c == 0xc4){
    switch (c2){
      case 0x9E:
        c2 |= 0x5F;  // making it 0xF.. (..) (letter ..)
        break;
      case 0x9F:
        c2 |= 0x51;  // Making it 0xF0 (ğ) (small letter g with breve accent) 0x011F
        //c2 = 0x11F;
        break;
      case 0xB0:        
        c2 |= 0x2D;  // Making it 0xDD  (İ) (capital letter i) 0x0130
        //c2 = 0x0130;
        break;
      case 0xB1:
        c2 |= 0x4C;  // making it 0xFD  (ı) (letter i without a dot on top) 0x0131
        //c2 = 0x0131; 
        break;
      default:
        break;
    }
  }
  else if (c == 0xc5){
    switch (c2){
      case 0x9E:
        c2 |= 0x40;  // making it 0xDE (Ş) (capital S with cedille) 0x015E
        //c2 = 0x015E;
        break;
      case 0x9F:
        c2 |= 0x5F;  // making it 0xFE (ş) (small letter s with vertical line at bottom) 0x015F
        //c2 = 0x015F;
        break;
      default:
        break;
    }
  }
  if (c2 >= 0xE0){
    if (my_debug){  
      Serial.print(F("in["));
      Serial.print(j);
      Serial.print(F("] The value of the converted character = 0x"));
      Serial.println(c2, HEX);
    }
  }
  return c2;
}

int scanExtended(String in, int le){
  char c, c2;
  int  n, n2, NrOfExtended = 0; 
  // Scan in for Latin-5 letters and set the counter for them
  for (int i = 0; i < le; i++){
    c = in[i];
    if (c == 0xc3 || c == 0xc4 || c == 0xc5){
      NrOfExtended += 1;
    }
  }
  if (NrOfExtended == 0) 
    return 0;
  else{
    /*
    disp_line_on_repl(0);
    Serial.println(F("scanExtended(): List of  Latin-5 2-byte letters rcvd:"));
    for (int i = 0; i < le; i++){
      n = i;
      c = in[i];
      if (c == 0xc3 || c == 0xc4 || c == 0xc5){
        i++;
        n2 = i;
        c2 = in[i];
        // Print only the 2-byte Latin-5 group letter bytes
        for (int j = 0; j < 2; j++){
          if (j == 0)
            Serial.print(F("Lead "));
          else if (j == 1)
            Serial.print(F("2nd  "));
          Serial.print(F("byte char["));
          if ((j == 0 && n < 10) || (j == 1 && n2 < 10))
            Serial.print("0");
          if (j == 0)
            Serial.print(n);
          else if (j == 1)
            Serial.print(n2);
          Serial.print(F("] value 0x"));
          if (j == 0)
            Serial.println(c, HEX);
          else if (j == 1)
            Serial.println(c2, HEX);
        }  
      }
    }
    */
    disp_line_on_repl(0);  // Print only at the end of the list
  }
  return NrOfExtended;
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
String ConvUpperToLower(String in, int ata, String isrc_id, bool convert_all = false){
  int le = in.length();
  int le_id = isrc_id.length();
  String isrc_id_short, tpATA;
  String io, io2 = "";
  String s, s2 = "";
  char e = '\0';
  char c, c2, co;
  char16_t c16;
  int n, n2, NrOfExtended = 0;
  bool isTurk = false;
  int tlst_le = 0;

  if (ata == SPOT_ALBUM)
    tpATA = "Album";
  else if (ata == SPOT_ARTIST)
    tpATA = "Artist";
  else if (ata == SPOT_TRACK)
    tpATA = "Track";

  if (le_id > 0)
    isrc_id_short = isrc_id.substring(0,2);  // take the first two characters, e.g.: "TR" which stands for "Turkey"
  else
    isrc_id_short = "";
    
  if (my_debug){
    Serial.print(F("ConvUpperToLower(): received to convert: "));
    Serial.print(tpATA);
    Serial.print(" name: \'");
    Serial.print(in);
    Serial.println("\'");

    if (le_id > 0){
      Serial.print(F("Intl Std Recording Code (first 2 letters only): "));
      Serial.print(isrc_id_short);
      Serial.print(F(", is country: "));
      if (isrc_id_short == "TR")
        Serial.println("Turkey");
      else
        Serial.println("is unknown");
    }
  }
  if (isrc_id_short == "TR")
    isTurk = true;
 
  if (my_debug){
    Serial.print(F("The length of the "));
    Serial.print(tpATA);
    Serial.print(" name: ");
    Serial.println(le);
    Serial.print(F("Checking need for the conversion of "));
    Serial.print(tpATA);
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
        if (c == 0xc3){ // Do we have a Latin-1 letter? (this is the 'lead byte')
          Serial.println(F("We have an ASCII Latin-1 supplement letter"));
          // see: https://docs.microsoft.com/en-us/cpp/text/support-for-multibyte-character-sets-mbcss?view=msvc-160
          // The first byte comes from a range of codes reserved for use as lead bytes
          // _ismbblead tells you whether a specific byte in a string is a lead byte.
          io += c;  // yes, copy lead byte to output 
          if (my_debug){
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] lead byte value to put into output (io): = 0x"));
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
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] The value of the converted character = 0x"));
            Serial.println(c2, HEX);
          }
        }
        /*
        else if (c == 0xc4 || c == 0xc5){
          // Do we have a Latin-5 (Turkish) letter? (this is the 'lead byte')
          // The first byte comes from a range of codes reserved for use as lead bytes
          //io += c; // put lead byte into output
          if (isTurk){
            c2 = handleTurk(in, i, c);
            io += c2;
          }
          i++;
        }
        */
        else {
          io += c; // A non Latin-1 and non Latin-5 letter: put immediately to the output
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
          Serial.println(F("We have an ASCII Latin-1 supplement character"));
          io += c;  // yes, copy lead byte to output
          if (my_debug){  
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] lead byte value to put into output (io): = 0x"));
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

            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] The value of the converted character = 0x"));
            Serial.println(c2, HEX);
          }
        }
        /*
        else if (c == 0xc4 || c == 0xc5){
          // Do we have a Latin-5 (Turkish) letter? (this is the 'lead byte')
          // The first byte comes from a range of codes reserved for use as lead bytes
          //io += c;
          if (isTurk){
            c2 = handleTurk(in, i, c);
            io += c2;
          }
          i++;
        }
        */
        else {
          // No need to show Turkish letters here (they result in a question mark)
          if (my_debug && !isTurk){
            Serial.print(F("rest char["));
            Serial.print(i);
            Serial.print(F("] to output. It"));
            Serial.print("\'");
            Serial.print(F("s value = "));
            Serial.print(c, HEX);
            Serial.print(F(" = \'"));
            Serial.print( String(c) );
            Serial.println("\'");
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
void disp_artists(bool on_tft, CurrentlyPlaying currentlyPlaying, int xPos, int yLst[], int yPosLstLe){
  int nr_of_artists = currentlyPlaying.numArtists;
  if (my_debug){
    disp_line_on_repl(0);
    Serial.print(F("disp_artists(): nr of artists = "));
    Serial.println(nr_of_artists);
  }
  ck_btn();  // check for button press
  if (SAAhandler.getFlag(SAA_BTN1PRESSED) || SAAhandler.getFlag(SAA_BTN2PRESSED))
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
        tft.drawString(currentlyPlaying.artists[i].artistName, xPos, yLst[j]);
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
        clr_tft_down_part(xPos, yLst, yPosLstLe);
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

void clr_tft_down_part(int xPos, int yPosLst[], int yPosLstLe){
  // String blank = "                                   "; // 35 * <spc>
  int vt = (yPosLst[yPosLstLe-1] - yPosLst[0] -1);  // vt = 55
  if (my_debug){
    Serial.print(F("clr_tft_down_part(): vt = "));
    Serial.println(vt);
  }
  tft.fillRect(0, yPosLst[0], tft.width(), tft.height(), TFT_BLACK);
  //for (int i = 0; i < yPosLstLe; i++){
  //  tft.drawString(blank, xPos, yPosLst[i]);
  //}
}

void listFlags(){
  Serial.println(F("Spotify Album Art (SAA) Flags"));
  Serial.println(F("+-----------------+---------+"));
  Serial.println(F("|      Flag:      | Status: |"));
  Serial.println(F("+-----------------+---------+"));
  for (uint8_t i = 0; i < SAAhandler.getNrFlags(); i++){
    Serial.print(F("| "));
    Serial.print(SAAhandler.getFlagName(i));
    Serial.print(F(" |    "));
    Serial.print(SAAhandler.getFlag(i));
    Serial.println(F("    |"));
  }
  Serial.println(F("+-----------------+---------+"));
  
}

void listFontsLoaded(uint16_t fnt_ld){
  const char* fntLst[] = { " ", "2", "4", "6", "7", "8", "8N", " "};
  int bIdxLst[] = { 1, 2, 4, 6, 7, 8, 9, 15};
  uint8_t n;

  Serial.println(F("Spotify Album Art (SAA) Fonts"));
  Serial.println(F("+-----------------+---------+"));
  Serial.println(F("|      Font:      | Loaded: |"));
  Serial.println(F("+-----------------+---------+"));
  for (int i = 0; i < 8; i++){
    if (i >= 0 && i < 6)
      Serial.print(F("|   "));
    if (i == 6 || i == 7)
      Serial.print(F("|  "));
    if (i >= 0 && i < 7)
      Serial.print(F("LOAD_"));
    if (i == 0)
      Serial.print(F("GLCD "));
    if (i >= 1 && i < 7){
      Serial.print(F("FONT"));
      Serial.print(fntLst[i]);  
    }
    if (i == 7)
      Serial.print(F("SMOOTH_FONT"));
    Serial.print(F("    |   "));
    n = fnt_ld >> bIdxLst[i] & 1U;
    Serial.print(n);
    Serial.println(F("     |"));
  }
  Serial.println(F("+-----------------+---------+"));
}

void ck_btn(void) {
  String btn = "";
  int btn_nr = 0;
  int fidx = 0;
  // Only check if flag not set yet
  if (!SAAhandler.getFlag(SAA_BTN1PRESSED)){ 
    // If D3 is LOW, button is pressed
    if (digitalRead(D3) == LOW){  
      btn = "D3";
      btn_nr = 1;
      SAAhandler.setFlag(SAA_BTN1PRESSED);
      msg_to_tft("Button 1 pressed. Ad hoc          data request on it\'s way");
    }
  }
  if (!SAAhandler.getFlag(SAA_BTN2PRESSED)){
    // If D8 is HIGH, button is pressed
    if (digitalRead(D8) == HIGH){
      btn = "D8";
      btn_nr = 2;
      SAAhandler.setFlag(SAA_BTN2PRESSED);
      msg_to_tft("Button 2 pressed.                 font change...");
#ifndef USE_FREE_FONTS
      fidx = fontFileIdx + 1;
      if (fidx >= NrOfFontFilesOnDisk)
        fidx = 0;
#endif
    }
  }

  if (btn_nr > 0){
    if (my_debug){
      Serial.print(F("<<<=== Button "));
      Serial.print(btn);
      Serial.print(F(" (= button "));
      Serial.print(btn_nr);
      Serial.println(F(") pressed. ===>>>")); 
      listFlags();
    }
#ifndef USE_FREE_FONTS
    if (btn_nr == 2)
      chgFontFileIdx(fidx); // Put this here and not above
      // We first need to pass the call to listFlags() to see
      // that btn2 has been pressed
#endif
  }
}

void displayCurrentlyPlayingOnScreen(CurrentlyPlaying currentlyPlaying)
{
  int an_Extended = 0;
  int tn_Extended = 0;
  int abn_Extended = 0;
  String an, an0, tn, tn0, tn2, abn, abn0, abn2, abn3, s;
  int n, n2, xPos, width_to_write, font, nr_of_artists = 0;  
  int an_le, tn_le, abn2_le = 0;
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
#ifdef USE_FREE_FONTS
  font = 2;
  tft.setFreeFont(&FreeSans12pt7b);
  tft.setTextSize(font);  // Possible value: 1 to 7
#else
  tft.setTextSize(7);  // Possible value: 1 to 7
  tft.setTextColor(TFT_WHITE, tft.color565(25,25,25));
  if (fontFileIdx == 1)  // the font that displays very little letters
    font = 4;
  else
    font = fontSize;  // taken from SAA.ini
#endif
  ck_btn();  // Check if button D8 (btn2) or button 3 (btn1) is pressed.
  if (SAAhandler.getFlag(SAA_BTN1PRESSED))
    return;  // exit to loop() to handle an adhoc Spotify get data request

  String isrc = String(currentlyPlaying.isrcId);
  int isrc_le = isrc.length();
  if (isrc_le > 0){
    if (my_debug){
      Serial.print(F("ISRC code = "));
      Serial.println(isrc);
    }
  }
  else
     Serial.println(F("isrc ID unknown"));
  
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
  
    an0 = String(currentlyPlaying.artists[0].artistName); // 1st Artist Name
    an_Extended = scanExtended(an0, an0.length());
    NrOfLatin1Chars = 0;
    an = ConvUpperToLower(an0, SPOT_ARTIST, isrc, false);
    an_le  = an.length() - an_Extended;  
    if (!my_debug){
      Serial.print(F("ConvUpperToLower() Returnvalue: "));
      Serial.print("\'");
      Serial.print(an);
      Serial.println("\'");
      Serial.print(F("an_Extended = "));
      Serial.println(an_Extended);
      Serial.print(F("value an_le = "));
      Serial.println(an_le);
    }
  
    // Handle track name

    tn0 = String(currentlyPlaying.trackName);
    tn_Extended = scanExtended(tn0, tn0.length());
    tn_le  = tn0.length() - tn_Extended;  // tn = Track Name    
    tn = ConvUpperToLower(tn0, SPOT_TRACK, isrc, false);
    tn2 = "";
    if (!my_debug){
      Serial.print(F("ConvUpperToLower() Returnvalue: "));
      Serial.print("\'");
      Serial.print(tn);
      Serial.println("\'");
      Serial.print(F("tn_Extended = "));
      Serial.println(tn_Extended);
      Serial.print(F("value tn_le = "));
      Serial.println(tn_le);
    }
    
    // handle album name

    abn0 = String(currentlyPlaying.albumName);
    abn2 = "";
    abn_Extended = scanExtended(abn0, abn0.length());
    abn = ConvUpperToLower(abn0, SPOT_ALBUM, isrc, false);
    if (my_debug){
      Serial.print(F("ConvUpperToLower() Returnvalue: "));
      Serial.print("\'");
      Serial.print(abn);
      Serial.println("\'");
    }

    n = abn.indexOf(an); // see if Artist Name is also in Album Name
    if (n >= 0){
      abn2 = abn.substring(n+1); // slice off Artists Name off Album Name
      n2 = abn.indexOf(",");
      if (n2 >= 0){
        abn2 = abn2.substring(n2+1); // slice off the part until and including the comma
      }
      abn2.trim(); // remove preceding and trailing white spaces     
    }
    else
      abn2 = abn;

    abn2_le  = abn2.length() - abn_Extended;  // abn = Album Name
    if (my_debug){
      Serial.print(F("abn_Extended = "));
      Serial.println(abn_Extended); 
      Serial.print(F("value abn2_le = "));
      Serial.println(abn2_le);
    }
    
    xPos = 5;
#ifdef USE_FREE_FONTS
    width_to_write = 19;
#else
    width_to_write = 24;
#endif
    if (tn_le > width_to_write)
      tn2 = tn.substring(0,width_to_write); // + "...";
    else
      tn2 = tn;

    if (abn2_le > width_to_write)
      abn3 = abn2.substring(0,width_to_write); // + "...";
    else
      abn3 = abn2;
    if (my_debug){
      Serial.print(F("Width available to write on tft: "));
      Serial.println(width_to_write);
      Serial.print(F("length of album name is: "));
      Serial.println(abn2_le);
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
#ifdef USE_FREE_FONTS
    tft.setTextColor(TFT_YELLOW); // Background color is ignored if callback is set
    if (!show_all_artists){
      if (!shown_single){
        shown_single = true;
        // We're only going to display ata (1st artist, track, album)
        tft.setCursor(xPos, yPosLst[0]);
        tft.print(an);
        tft.setCursor(xPos, yPosLst[1]);
        tft.print(tn2);

    // Only show album name if it is different from the track name (check first 5 chars)
        if (abn3.substring(0,5) != tn2.substring(0,5))  
          tft.setCursor(xPos, yPosLst[2]);
          tft.print(abn3);
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
          tft.setCursor(xPos, yPosLst[1]);
          tft.print(tn2); 
          if (abn3 != tn2)  // Only show album name if it is different from the track name
            tft.setCursor(xPos, yPosLst[2]);
            tft.print(abn3);
        }
        else{
          disp_artists(show_all_artists, currentlyPlaying, xPos, yPosLst, yPosLstLe);
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
          if (SAAhandler.getFlag(SAA_BTN1PRESSED))
            return;  // exit to loop() to handle an adhoc Spotify get data request
          show_artists = !show_artists; // flip bool
          if (my_debug){
            Serial.print(F("flag flipped. Value flag show_artists = "));
            Serial.println(show_artists);
          }
          clr_tft_down_part(xPos, yPosLst, yPosLstLe);
        }
      }
    }
#else
    if (!show_all_artists){
      if (!shown_single){
        shown_single = true;
        // We're only going to display ata (1st artist, track, album)
        tft.drawString(an ,  xPos, yPosLst[0]);
        tft.drawString(tn2,  xPos, yPosLst[1]);
        // Only show album name if it is different from the track name (check first 5 chars)
        if (abn3.substring(0,5) != tn2.substring(0,5))  
          tft.drawString(abn3, xPos, yPosLst[2]);
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
          tft.drawString(tn2,  xPos, yPosLst[1]);
          // Only show album name if it is different from the track name
          if (abn3 != tn2)
            tft.drawString(abn3, xPos, yPosLst[2]);
        }
        else{
          disp_artists(show_all_artists, currentlyPlaying, xPos, yPosLst, yPosLstLe);
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
          if (SAAhandler.getFlag(SAA_BTN1PRESSED))
            return;  // exit to loop() to handle an adhoc Spotify get data request
          show_artists = !show_artists; // flip bool
          if (my_debug){
            Serial.print(F("flag flipped. Value flag show_artists = "));
            Serial.println(show_artists);
          }
          clr_tft_down_part(xPos, yPosLst, yPosLstLe);
        }
      }
    }
#endif
  
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
#ifdef USE_FREE_FONTS
  int xPos = 5;
  int yPosLst[] = {164, 190, 220};
  //int yPosLstLe = 3;
  tft.setCursor(xPos, yPosLst[1]);
  int n = msg.length();
  if (n > 25){
    tft.print(msg.substring(0,25));
    tft.setCursor(xPos, yPosLst[2]);
    tft.print(msg.substring(25));
  }
  else
    tft.print(msg);
#else
  tft.drawString(msg,10,200);
#endif
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
  int n;
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

#ifndef USE_FREE_FONTS
  n = readIni();
  if (n == -1)
    perpetual_loop();
#endif
  // Initialise the TFT
  tft.begin();
  tft.setRotation(3); // Needs to be rotated 3 to look correct on the shield
  tft.setTextColor(0xFFFF, 0x0000);
#ifdef USE_FREE_FONTS
   tft.setFreeFont(&FreeSans12pt7b);
#endif
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

#ifndef USE_FREE_FONTS
  // Load a smooth font from Flash FS
  //tft.loadFont("NotoRegular18", LittleFS);  // Original font file: NotoSansDisplay_Regular.ttf 
  //tft.loadFont(fontFile, LittleFS);  // Original font file: NotoSansDisplay_Regular.ttf Taken from secrets.h
  fontFileIdx = 1;  // Choose RubikReg3218
  n = openFontFile(fontFileIdx);  // Open the file of the 1st font in the fonts[] array.
  if (n == -1)
    perpetual_loop();
#endif

  // (Downloaded from: https://fonts.google.com/noto/specimen/Noto+Sans+Display);
  //  NotoSansDisplay_SemiCondensed-Regular.ttf     -- chosen corps 12
  
  // D8 mode needs to be set after tft.init
  pinMode(btn1, INPUT); // is pulled low, will be low when pressed
  pinMode(btn2, INPUT); // is pulled down, will be high when pressed
  
  SAAhandler.clrAll();  // clear all SAA defined flags
  SAAhandler.clrSpotifyStatus(); // clear the Spotify Status register
  SAAhandler.setCPOS_previous(millis()); // set the start time in 'previous'
  SAAhandler.setCPOS_loopnr(0);
  SAAhandler.clrFlag(SAA_BTN1PRESSED);
  SAAhandler.clrFlag(SAA_BTN2PRESSED);
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
  if (lStart || SAAhandler.getFlag(SAA_BTN1PRESSED) || loop_elapsed > delayBetweenRequests)  // was: millis() > requestDueTime)
  {
    if (my_debug){
      if (lStart)
        testTurk();  // Temporary test
      disp_line_on_repl(1);
#ifndef USE_FREE_FONTS
      if (lStart){
        uint16_t fontsld = tft.fontsLoaded();
        Serial.print(F("tft.fontsLoaded = 0b"));
        Serial.println(fontsld, BIN);
        listFontsLoaded(fontsld);
      }
      Serial.print(F("Font loaded: "));
      Serial.println(fontFile);
      Serial.print(F("Font size: "));
      Serial.println(fontSize);
      int16_t fonthght = tft.fontHeight(fontFileIdx);  // Just to test. I don't know if value is OK
      Serial.print(F("fontHeight = "));
      Serial.println(fonthght);
      uint8_t utf8_sw = tft.getAttribute(2); // Check state of UTF8_SWITCH
      Serial.print(F("UTF8_SWITCH state = "));
      Serial.println(utf8_sw);
#endif
      listFlags();
      Serial.println(F("loop(): getting info on currently playing song:"));
    }
    if (SAAhandler.getFlag(SAA_BTN1PRESSED)){
      SAAhandler.clrFlag(SAA_BTN1PRESSED);  // reset flag
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

void perpetual_loop(){
  delay(0);  // feed the WDC
}
