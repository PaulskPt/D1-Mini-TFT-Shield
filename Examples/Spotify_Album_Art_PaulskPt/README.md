# SAA - Spotify Album Art

## This is a modified version of SpotifyAlbumArt.ino,
## by Paulus Schulinck (GitHub: @PaulskPt), 2021-10-30.
## The the original version by Brian Lough.
## The following modifications/additions have been realised
## and tested.

Paragraph 1

Added a global boolean variable `my_debug` with which 
easily all debug print statements can be activated or
de-activated.

Paragraph 2

Added a class named `SAA`. SAA stands for `Spotify Album Art`. 
This class has been created to store
various `flags` which respresent the runtime state
of the sketch.
It also stores the `status` received after a sending
a get request to the Spotify API. This class 
consists of functions to set, get and clear 
the flags; set and clear a loop counter;
set and get the Spotify request status.
I created a function `listFlags()` that prints all
the flags (currently only three) in a table style
to the Monitor output, depending.
Below an example of the output of listFlags() in two
moments after reset:

```
    20:22:48.121 -> 
    20:22:48.121 -> Initialisation done.
    20:22:48.822 -> Connecting Wifi: _____________________
    20:22:48.822 -> .......
    20:22:53.043 -> WiFi connected
    20:22:53.043 -> IP address: 192.168.x.xxx
    20:22:53.138 -> Refreshing Access Tokens
    20:22:54.684 -> Refreshed access token
    20:22:54.730 -> 1'
    20:22:54.730 -> WiFi status = 0
    20:22:54.730 -> heap_info(): Free Heap: 11248
    20:22:56.179 -> ---------------------------------------
    20:22:56.179 -> displayCurrentPlayingOnScreen(): 
    20:22:56.179 -> Loop nr: 1
    20:22:56.179 -> Elapsed: 1 Sec
    20:22:56.179 -> Spotify Album Art (SAA) Flags
    20:22:56.179 -> +----------------+---------+
    20:22:56.179 -> |      Flag:     | Status: |
    20:22:56.179 -> +----------------+---------+
    20:22:56.179 -> | IsPlaying      |    1    |
    20:22:56.179 -> | ImageShown     |    0    |
    20:22:56.179 -> | ImageLoadAgain |    0    |
    20:22:56.225 -> +----------------+---------+
    20:22:56.225 -> ----------------------------------------
    20:22:56.225 -> show_new_album_art(): Updating Art
    20:22:56.225 -> Removing existing image
    20:22:58.761 -> displayImage(): Got Image
    20:22:59.184 -> Spotify Album Art (SAA) Flags
    20:22:59.184 -> +----------------+---------+
    20:22:59.184 -> |      Flag:     | Status: |
    20:22:59.184 -> +----------------+---------+
    20:22:59.184 -> | IsPlaying      |    1    |
    20:22:59.184 -> | ImageShown     |    1    |
    20:22:59.184 -> | ImageLoadAgain |    0    |
    20:22:59.184 -> +----------------+---------+
    20:23:18.098 -> heap_info(): Free Heap: 10464
```
Paragraph 3

Added the use of a textfile with the name `secrets.h`.
This file contains WiFi credentials and the 
Spotify Developer app credentials.
The use of an external file with credentials
has been followed to prevent the need
to embed these credentials into the sketch file.
Second advantage to this is that the credentials
can be easily changed by the user without the need 
of recompiling the sketch.

The contents of secrets.h is:
```
    #define SECRET_SSID "..."
    #define SECRET_PASS "..."
    #define SECRET_CLIENT_ID "..."
    #define SECRET_CLIENT_SECRET "..."
```
In the sketch, these are used here (lines 130-145):
```
    #ifdef USE_PSK_SECRETS
    #include "secrets.h"
    #endif
    //------- Replace the following! ------
    #ifdef USE_PSK_SECRETS
    ///////please enter your sensitive data in the Secret tab/arduino_secrets.h
    char ssid[]         = SECRET_SSID;    // Network SSID (name)
    char password[]     = SECRET_PASS;    // Network password (use for WPA, or use as key for WEP)
    char clientId[]     = SECRET_CLIENT_ID;      // Your client ID of your spotify (Developer) APP
    char clientSecret[] = SECRET_CLIENT_SECRET;  // Your client Secret of your spotify APP (Do Not share this!)
    #else
    char ssid[] = "SSID";         // your network SSID (name)
    char password[] = "password"; // your network password
    char clientId[] = "56t4373258u3405u43u543"; // Your client ID of your spotify APP
    char clientSecret[] = "56t4373258u3405u43u543"; // Your client Secret of your spotify APP (Do Not share this!)
    #endif
```
Paragraph 4

Changed the use of `SPIFFS` file system to use the
`LittleFS` file system. I made this change because,
every time when building the sketch to flash it to
the 8266EX, I saw compiler messages advising to 
move from SPIFFS to LittleFS. This 'switch' was 
not that easy. Imade changes in
TFT_eSPI/Extensions/Smooth_font.cpp and
in the spotify-api-arduino-main library,
for debugging purposes, i.e: to be able to 'follow'
the program execution: to 'see' what parts or
functions were executed using the LittleFS system.
For this I added in line 13 of Smooth_font.cpp a 
variable `my_debug2`. I could not use the same
name 'my_debug', used in the sketch, because the 
compiler complained.
See also my remarks in item 11).
After this move, the first time I tried to build the
sketch, the compiler 'complained', when
executing line 75 of the file elf2bin.py. The operating
system indicated that a file was missing:
xtensa-lx106-elf-objdump.exe. I was able
to download this file from:
https://github.com/esp8266/Arduino/releases/tag/3.0.2. 
At the bottom of the page is ready for download the 
file: esp8266-3.0.2.zip.
xtensa-lx106-elf-objdump
In my PC running MS Windows 10, these files are located
in the subfolder: 
```
    C:\<Users>\<Username>\AppData\Local\Arduino15\
    packages\esp8266\tools\xtensa-lx106-elf-gcc\
    3.04-gcc10.3-1757bed\bin\xtensa-lx106-elf-objdump.exe.
```

Paragraph 5

moving some functionality that were in the main 
loop() function into the function 
displayCurrentlyPlayingOnScreen().

The following part of the original loop():
```
    CurrentlyPlaying currentlyPlaying = spotify.getCurrentlyPlaying(SPOTIFY_MARKET);
      if (!currentlyPlaying.error)
      {
      //printCurrentlyPlayingToSerial(currentlyPlaying);
      displayCurrentlyPlayingOnScreen(currentlyPlaying);

      // Smallest (narrowest) image will always be last.
      SpotifyImage smallestImage = currentlyPlaying.albumImages[1];
      String newAlbum = String(smallestImage.url);
      if (newAlbum != lastAlbumArtUrl) {
         Serial.println("Updating Art");
         int displayImageResult = displayImage(smallestImage.url);
         if (displayImageResult == 0) {
            lastAlbumArtUrl = newAlbum;
         } else {
            Serial.print("failed to display image: ");
            Serial.println(displayImageResult);
         }
      }
    }
```
    I changed into:
```
    int status = spotify.getCurrentlyPlaying(displayCurrentlyPlayingOnScreen, SPOTIFY_MARKET);
    if (my_debug){
        Serial.print(F("loop(): result of spotify.getCurrentPlaying(): "));
        Serial.println(status);
    }
    if (!SAAhandler.IsPlaying()){  // Check the flag
        err_msg_to_tft(F("Spotify result: not playing"));
    }
    else if (status != 200){
        SAAhandler.setSpotifyStatus(status); // copy the status into
        err_msg_to_tft(get_status(status));  // put (error) status onto tft
    }
```

Paragraph 6
   
In case of error situations, e.g.:
 * Spotify client not playing;
 * httpGetrequest: communication failure,
an error message will be displayed
in the lower part of the tft. Depending on the
state of the global 'my_debug' flag, error messages
will also be printed in the Serial output or REPL.
This is done in the function: `err_msg_to_tft()`.
At the end of this function the flag: SAA_IMGSHOWN is cleared
and the flag `SAA_IMGLOADAGAIN` is set. This is done to force a 
re-download of the Spotify Album Art after an error message
occurred.

Paragraph 7

Added functions to display the list of
artists in case the track or album has more than
one artist;

Paragraph 8

Added handling of letters in the ASCII extended `Latin-1` range.
These letters consist of 2 bytes. The first byte has always
a value of `0xc3` followed by a byte with a value in the
range 0xa0 - 0xff.

I encountered two problems while handling letters in the 
Latin-1 range:

Paragraph 8.1

the `string.length()` function counts the 'hidden' first
byte of a 2-byte Latin-1 group letter as 2 bytes, but
the tft display or the IDE Monitor or Serial output
only shows the one visual letter.

Paragraph 8.2

the `TFT_eSPI` library does not display Latin-1 group
capital letters. We have to convert them to small letters.

_Workaround:_

Paragraph 8.1.1

In the function `convertUnicode()` :
To correct the wrong outcome of string.length() with
Latin-1 group characters I introduced a global counter variable,
name: `NrOfLatin1Chars`. The value of this counter will later,
in function `displayPlayerToScreen()` be used in string length
calculations for artist, track and album name.

Below the code in function `convertUnicode()` that filters and handles
Latin-1 group letters:
```
   if (iChar == 0xc3){
      out += iChar;
      i++;
      iChar = unicodeStr[i];
      out += iChar;
      NrOfLatin1Chars += 1; // Increase the count for received Latin-1 group characters
   }
   else{
   [...]
```
Later, inside the function: displayCurrentlyPlayingOnScreen(),
the global variable NrOfLatin1Chars is used, e.g.: in these lines of code:
```
    an_NrOfLatin1Chars = NrOfLatin1Chars; // copy the value
    [...]
    an_le  = an.length() - an_NrOfLatin1Chars;   // an_le stands for 'artist name length'
```

Paragraph 8.2.1

Below the part of `ConvUpperToLower()` that filters for and handles
Latin-1 group characters:

Paragraph 8.2.1.1

If it is the first character of the string (usually a capital letter):
```
    [...]
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
        c2 |= 0xe0;  // perform a bitwise OR between byte 2 and (0xc0 + 0x20) = 0xe0 to get the correct Latin-1 small letter value
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
        io += c;  // A non Latin-1 letter: put immediately to the output
    }
    [...]
```

Paragraph 8.2.1.2

   If it is any other letter in the string:
```
    [...]
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
```
    IDE Monitor output below:

```
    18:35:46.177 -> ConvUpperToLower(): received to convert: 'Çökertme'
    18:35:46.224 -> Length of parameter 'in': 10
    18:35:46.271 -> Checking need for conversion of Track name
    18:35:46.318 -> in[0] character value to put into io: = 0xC3    Note: this is the first character of the input string.
    18:35:46.364 -> the value of the 2nd byte = 0x87                      Since TDT_eSPI cannot display a Latin-1 capital letter
    18:35:46.364 -> We have an ASCII Latin-1 supplement character         C with cedille, we have to convert it to small letter
    18:35:46.460 -> in[1] The value of the character = 0xE7         <<<=== Latin-1 small letter c with cedille
    18:35:46.460 -> in[2] character value to put into io: = 0xC3
    18:35:46.506 -> the value of the 2nd byte = 0xB6
    18:35:46.599 -> We have an ASCII Latin-1 supplement character
    18:35:46.599 -> in[3] The value of the character = 0xF6         <<<=== Latin-1 small letter o with trema
    18:35:46.646 -> Returnvalue: '⸮⸮⸮⸮kertme   = 'çökertme'
```
    `Note` that one can convert capital letters to small letters using the
    function string.toLowerCase() but this function does not solve the
    problems explained above: 
    *  the ASCII extended Latin-1 group of letters; 
    *  the handling of some long album names like:
        'Cross Me (feat. Chance the Rapper & PnB Rock)' or:
        'Remember The Name (feat. Eminem & 50 Cent)'

    In the function ConvUpperToLower() I added a functionality to search a string
    for the occurrance of '<space>(feat.'. If such a substring is encountered,
    the part of the track title including '<space>(feat.' and the subsequent text,
    are sliced off. I did this because limited space on a 240x240px tft display.
    In the given examples the function ConvUpperToLower() will return a track title
    'Cross Me' or, as in the second example, return: 'Remember The Name'
    to be displayed on the tft.

    The function ConvUpperToLower() also contains an option to convert all characters to
    small letters, depending on the value of the parameter 'convert_all' which defaults to
    false. Example:
    if parameter 'convert_all' is true, all characters, except the first character 
    of each word, will be converted to a small letter.
    If 'convert_all' is false, characters will not be converted. This option can be used
    is we don't want to 'touch' specific artist, track or band names. We know that many artists 
    and bands identify themselves by using a special way of writing their name and/or 
    the name of a track. Example: track name: 'GOD MODE'. Artist name: ', 'LON3R', 
    'Bispo D'ay', 'JOHNY'.

    ToDo: move this option to an .ini file or use the Secrets.h file to store this option.
   
Paragraph 9

Using another font by downloading, converting and flashing a font file.
After performing various experiments using different fonts using the TFT_eSPI library,
I decided to choose a different font file that suited better my needs. I live in Portugal,
I listen to Portuguese music. Names of Portuguese singers, bands, names of album and track titles
consist of letters with accents, listed in the ASCII extended Latin-1 group of characters.
Example of some received track name: 'Não Posso Ficar (feat. Sir Scratch)'
This is why I wanted a font file that contains those characters. Reading the instructions in the
TFT_eSPI/Tools/Create_Smooth_Font/Create_font there is a 'Create_font.pde'. After studying
various documents I decided to download a .ttf font file from the Google Fonts website.
I downloaded a font file from the 'Noto Sans Display' font family. See:
https://fonts.google.com/noto/specimen/Noto+Sans+Display. Within that font family,
I choosed the 'Regular 400' font. A file which I named: 'NotSDispSemiCond-ELight.ttf' (353 kB)
then created an own work folder and edited a copy of the Create_font file 'Create_font.pde'
In that file I decided to use the following blocks of characters:
0x0021, 0x007E  // Basic Latin, 128, 128, Latin (52 characters), Common (76 characters);
0x00E0, 0x00FF, // Latin-1 Supplement, 128, 128, Latin (64 characters), Common (64 characters);
// Commonly used codes:
0x00A3, 0x00B0, 0x00B5, 0x03A9, 0x20AC, // = Pound Sterling, degrees, micro (mu) omega and Euro symbol.

I had to download and install the Windows application: 'Processing' (version 4.0 beta 2 (Oct 4, 2021))
See: https://processing.org/. I downloaded the file: processing-4.0b2-windows64.zip. Unpacking this,
creates a folder with the name 'processing-4.0b2'. In this folder are various subfolders. In the main
folder a file with the name 'processing.exe'. Running this executable one can open the 'Create_font.pde'.
I learned that 'Processing.exe' requires some form of folder and subfolder structure. Through a 'cut and
try' method I came to created the following temporary file/folder structure that was 'excepted'
by the Processing application:
    
    c:\<Users>\<User>\Documents\Arduino\libraries\TFT_eSPI\Tools\Create_Smoot_Font\ 
    In this 'root' I created the following:
```
    \Create_font_paulsk
        \Create_font
            \data
                NotoSDispSemiCond-ELisght.ttf
            \FontFiles
                System_Font_List.txt     (created during the conversion process)
            Create_font.pde
        \FontFiles
            (empty subfolder)
```

Using the Processing application I was able to convert the .ttf font file 
into a .vlw font file, using the choices I made in the file: Create_font.pde.
After a successfull conversion I copied the resulting file 'NotoRegular18.vlw" (16 kB) to the 
/data subfolder of my Arduino sketch folder. Then I successfully flashed this .vlw file
using the Arduino IDE > Tools > 8266 LittleFS Data Upload function.

In the sketch, function setup(), line 1004, there is the command to use the font file:
1004   tft.loadFont("NotoRegular18", LittleFS);
      
Outside the sketch itself I made some 'cosmetic' changes to facilitate or enhance debug output:

Paragraph 10

In the file: TFT_eSPI/Extensions/Smooth_font.cpp (line 238-242 and 280-312
(line numbers after my alterations)) I modified the print output to show the loadFont()
list in a table view. Here the part of the start of this output:
```
    17:16:36.848 -> Smooth_font.cpp, line 130: we are using LittleFS! We are not using SPIFFS
    17:16:36.848 -> TFT_eSPI.loadFont(): fontFile.open result = '1'
    17:16:36.848 -> +---------+---------+--------+-----------+-----+
    17:16:36.848 -> | Unicode | gHeight | gWidth | gxAdvance | gdY |
    17:16:36.894 -> +---------+---------+--------+-----------+-----+
    17:16:36.894 -> |     21  |   13    |    2   |     4     |  13 |
    17:16:36.894 -> +---------+---------+--------+-----------+-----+
    17:16:36.894 -> |     22  |    5    |    5   |     7     |  13 |
    17:16:36.894 -> +---------+---------+--------+-----------+-----+
    [...]
```

Paragraph 11 

Changes made to files of the Arduino library: spotify-api-arduino-main/src/ :
In the file: SpotifyArduino.cpp
I modified some debug output to show which library and which function inside that library is
printing the debug output. E.g.: line 54:
from: 'Serial.println(F("Connection failed"));'
to:   'Serial.println(F("SpotifyArduino.makeRequestWithBody(): Connection failed"));'
I also decided to comment-out some precompiler conditions, e.g.:
```
    51 // #ifdef SPOTIFY_SERIAL_OUTPUT
    52    // addition of class.function info by @paulsk to see in output which 
    53 	// function brought the message to REPL/Monitor window
    54    Serial.println(F("SpotifyArduino.makeRequestWithBody(): Connection failed"));
    55 // #endif
```
to have the command in line 54 be active all the time. I want to be informed when there is a
communication failure.

Paragraph 12

In the Arduino IDE I installed the functionality to upload font file data to the ESP8266
using LittleFS. The software and installation instructions you can find via:
(https://github.com/earlephilhower/arduino-esp8266littlefs.plugin) . Following the instructions,
I created a /data subfolder in my Arduino sketchfolder. In this folder, for test,
I copied a thumbnail .bmp file. I uploaded it using the new installed LittleFS plugin.
As described above, later I uploaded the font file in this way.


# Final notes:

To be able to use the Spotify Album Art sketch one needs to have at least a Spotify Premium account.
Then one has to create a Spotify Developer Account. 
After this account is created one has to create at least one app in the Developer website personal Dashboard.
Then, in the Settings one has to add the return address, e.g.: http:192.168.0.120/callback/ to whitelist
this address to redirect to after authentication success OR failure. This is needed during the creation
of the first RefreshToken.
Next one has to build, flash and run the sketch getRefreshToken (from the examples of the spotify-api-arduino
repo of Brian Lough). If this run proceeds successful, the obtained RefreshToken has to be inserted into
the SpotifyAlbumArt.ino sketch, where is the line: 
    //#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"
If I counted well, the RefreshToken that I received was 132 characters long.
This RefreshToken will be renewed automatically during the use of the sketch.