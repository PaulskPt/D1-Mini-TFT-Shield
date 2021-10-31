# SAA - Spotify Album Art

### This is a modified version of Spotify_Album_Art.ino,
### by Paulus Schulinck (GitHub: @PaulskPt), 2021-10-30.
### The the original version by Brian Lough.
### The following modifications/additions have been realised
### and tested on a D1 Mini (8266EX) with ST7789 TFT hat.

Paragraph 1 - global debug flag

Added a global boolean variable `my_debug` with which 
easily all debug print statements can be activated or
de-activated.

Paragraph 2 - Added SAA class

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
Paragraph 3 - WiFi and Spotify app credentials

Added the use of a textfile with the name `secrets.h`.
This file contains WiFi credentials and the 
Spotify Developer app credentials.
The use of an external file with credentials
has been followed to prevent the need
to embed these credentials into the sketch file.
Second advantage to this is that the credentials
can be easily changed by the user without the need 
of recompiling the sketch. Note that the `SECRET_CLIENT_ID`
and the `SECRET_CLIENT_SECRET` are not your Spotify account
login credentials neither your Spotify Developer credentials,
but the Credentials received when creating your app in your
Developer Dashboard.

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
    ///////please enter your sensitive data in the Secret tab/secrets.h
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
Paragraph 4 - File system change

Changed the use of `SPIFFS` file system to use the
`LittleFS` file system. I made this change because,
every time when building the sketch to flash it to
the 8266EX, I saw compiler messages advising to 
move from SPIFFS to LittleFS. This 'switch' was 
not that easy. For debugging purposes I made changes in
`TFT_eSPI/Extensions/Smooth_font.cpp` and
in the `spotify-api-arduino-main` library,
i.e: to be able to 'follow' what parts or
functions were executed using the LittleFS system.
For this I added in `line 13` of `Smooth_font.cpp` a 
variable `my_debug2`. I could not use the same
name 'my_debug', used in the sketch, because the 
compiler complained.
See also my remarks in paragraph 11).
After this move, the first time I tried to build the
sketch, the compiler 'complained', when
executing `line 75` of the file `elf2bin.py`. The operating
system indicated that a file was missing:
`xtensa-lx106-elf-objdump.exe`. I was able
to download this file from GitHub (I don't recall where).
In my PC running MS Windows 10, these files are located
in the subfolder: 
```
    C:\<Users>\<Username>\AppData\Local\Arduino15\
    packages\esp8266\tools\xtensa-lx106-elf-gcc\
    3.04-gcc10.3-1757bed\bin\xtensa-lx106-elf-objdump.exe.
```

Paragraph 5 - Moved code in sketch

I moved code from `loop()` for a part into the function 
`displayCurrentlyPlayingOnScreen()` and another part into
function `show_new_album_art()`.

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

The following part of the original loop() function (lines 324-328):
```
      SpotifyImage smallestImage = currentlyPlaying.albumImages[1];
      String newAlbum = String(smallestImage.url);
      if (newAlbum != lastAlbumArtUrl) {
        Serial.println("Updating Art");
        int displayImageResult = displayImage(smallestImage.url);
```
caused a compiler error, in the line:

```
    int displayImageResult = displayImage(smallestImage.url);
```
After investigating the error, reading on
fora on internet, finding a candidate solution, inserting the solution
followed by `trial and error` sessions, I managed to create a 
working solution. See below in function:  `show_new_album_art()`,
(lines 602-626). See especially the line:
```
    char* my_url = const_cast<char*>(smallestImage.url);
```
and the modified form of the original line into:

```
    int displayImageResult = displayImage(my_url);
```
Below the function show_new_album_art():

```
void show_new_album_art(SpotifyImage smallestImage, bool load_again = false){
  String newAlbum = String(smallestImage.url);
  String TAG = "show_new_album_art(): ";
  if (load_again || newAlbum != lastAlbumArtUrl) {  // lastAlbumArtUrl is a global var
    disp_line_on_repl();
    Serial.print(TAG);
    Serial.println(F("Updating Art"));
    char* my_url = const_cast<char*>(smallestImage.url);
    // convert from const char* to char* 
    // see: https://stackoverflow.com/questions/833034/how-to-convert-const-char-to-char
    if (load_again)
      Serial.println(F("Forced to download Album Art again"));
    int displayImageResult = displayImage(my_url);
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
```

Paragraph 6 - Displaying error messages
   
In case of error situations, e.g.:

 * Spotify client not playing;
 * httpGetrequest: communication failure.

an error message will be displayed
in the lower part of the tft. Depending on the
state of the global 'my_debug' flag, error messages
will also be printed in the Serial output or REPL.
This is done in the function: `err_msg_to_tft()`.
At the end of this function the flag: `SAA_IMGSHOWN` is cleared
and the flag `SAA_IMGLOADAGAIN` is set. This is done to `force a 
re-download` of the `Spotify Album Art` image after an error message
has been displayed on the tft.

Paragraph 7 - List of artists

Added functions to display the list of
artists in case the track or album has more than
one artist;

Paragraph 8 - Handling Latin-1 group letters

Added handling of letters in the ASCII extended `Latin-1` range.
These letters consist of 2 bytes. The first byte has always
a value of `0xc3` followed by a byte with a value in the
range 0xa0 - 0xff.

I encountered two problems while handling letters in the 
Latin-1 range:

Paragraph 8.1

the `string.length()` function counts a Latin-1 group letter as 2 bytes, 
because it are two bytes, but only one byte is visual on tft, 
the IDE Monitor or Serial output. Displaying strings containing Latin-1
group letters on tft requires correction in the calculation of the length.
Below I explain my workaround to this problem.

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
            Serial.println(F("We have an ASCII Latin-1 supplement character"));
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] value to put into output (io) = 0x"));
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
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] value to put into output (io) = 0x"));
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
            Serial.println(F("We have an ASCII Latin-1 supplement character"));
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] value to put into output (io) = 0x"));
            Serial.println(c, HEX);
        }
        i++;
        c2 = in[i];  // load the 2nd byte
        if (my_debug){  
            Serial.print(F("the value of the 2nd byte = 0x"));
            Serial.println(c2, HEX);
        }
        // perform a bitwise OR between 
        // byte 2 and (0xc0 + 0x20) = 0xe0
        // to get the correct Latin-1 value
        c2 |= 0xe0;  
        io += c2;  // put the byte in output
        if (my_debug){  
            Serial.print(F("in["));
            Serial.print(i);
            Serial.print(F("] value to put into output (io) = = 0x"));
            Serial.println(c2, HEX);
        }
```
    IDE Monitor output below:

```
    18:35:46.177 -> ConvUpperToLower(): received to convert: 'Çökertme'
    18:35:46.224 -> Length of parameter 'in': 10
    18:35:46.271 -> Checking need for conversion of Track name

    // First Latin-1 group letter received:
    18:35:46.318 -> We have an ASCII Latin-1 supplement character
    18:35:46.318 -> in[0] value to put into output (io) = 0xC3

    // TDT_eSPI cannot display this Latin-1 capital letter
    // C with cedille, we have to convert it to small letter

    18:35:46.364 -> the value of the 2nd byte = 0x87     

    // Converted to Latin-1 small letter c with cedille
    18:35:46.460 -> in[1] value to put into output (io) = 0xE7

    // a second Latin-1 group letter received:
    18:35:46.460 -> in[2] value to put into output (io) = 0xC3

    18:35:46.506 -> the value of the 2nd byte = 0xB6

    // Latin-1 small letter o with trema
    // after a bitwise OR of 0xB6 with 0xE0, the value = 0xF6

    18:35:46.599 -> in[3] value to put into output (io) = 0xF6
    18:35:46.646 -> Returnvalue: '⸮⸮⸮⸮kertme   = 'çökertme'
```
    `Note` that one can convert capital letters to small letters using the
    function string.toLowerCase() but this function does not solve the
    problems explained above: 
    *  the ASCII extended Latin-1 group of letters; 
    *  the handling of some long album names like:
        'Cross Me (feat. Chance the Rapper & PnB Rock)' or:
        'Remember The Name (feat. Eminem & 50 Cent)'

    In the function `ConvUpperToLower()` I added a functionality to search a string
    for the occurrance of `'<space>(feat.'`. If such a substring is encountered,
    the part of the track title including '<space>(feat.' until the string end,
    are sliced off. I did this because limited space on a 240x240px tft display.
    In the given examples the function ConvUpperToLower() will return a track title
    'Cross Me' or, as in the second example, return: 'Remember The Name'
    to be displayed on the tft.

    The function `ConvUpperToLower()` also contains an option to convert all characters to
    small letters, depending on the value of the parameter `convert_all` which defaults to
    false. Example:
    if parameter 'convert_all' is true, all characters, except the first character 
    of each word, will be converted to a small letter.
    If 'convert_all' is false, characters will not be converted. This option can be used
    is we don't want to 'touch' specific artist, track or band names. Many artists
    and bands identify themselves by using a special branding their name and/or 
    the name of a track. Example: track name: 'GOD MODE'. Artist name: ', 'LON3R', 
    'Bispo D'ay', 'JOHNY'.

    `ToDo`: move this option to an .ini file or use the Secrets.h file to store this option.
   
Paragraph 9 - Creating our own font

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
```
0x0021, 0x007E  // Basic Latin, 128, 128, Latin (52 characters), Common (76 characters);
0x00E0, 0x00FF, // Latin-1 Supplement, 128, 128, Latin (64 characters), Common (64 characters);
// Commonly used codes:
0x00A3, 0x00B0, 0x00B5, 0x03A9, 0x20AC, // = Pound Sterling, degrees, micro (mu) omega and Euro symbol.
```
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

Using the `Processing` application I was able to convert the `.ttf` font file 
into a `.vlw` font file, using the choices I made in the file: `Create_font.pde`.
After a successfull conversion I copied the resulting file `NotoRegular18.vlw` (16 kB) to the 
`/data` subfolder of my Arduino sketch folder. Then I successfully flashed this .vlw file
using the Arduino IDE > Tools > `8266 LittleFS Data Upload` function.

In the sketch, function `setup()`, line 1004, there is the command to use the font file:
`tft.loadFont("NotoRegular18", LittleFS)`;
      
Outside the sketch itself I made some `cosmetic changes` to facilitate or enhance debug output:

Paragraph 10 - Changes to Smotth_font.cpp

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

Paragraph 11 - Changes to SpotifyArduino.cpp

Changes made to files of the Arduino library: spotify-api-arduino-main/src/ :
In the file: SpotifyArduino.cpp
I modified some debug output to show which library and which function inside that library is
printing the debug output. E.g.: line 54:
```
from: 'Serial.println(F("Connection failed"));'
to:   'Serial.println(F("SpotifyArduino.makeRequestWithBody(): Connection failed"));'
```
I also decided to comment-out some precompiler conditions, e.g.:
```
    51 // #ifdef SPOTIFY_SERIAL_OUTPUT
    52 // addition of class.function info by @paulsk to see in output which 
    53 // function brought the message to REPL/Monitor window
    54 Serial.println(F("SpotifyArduino.makeRequestWithBody(): Connection failed"));
    55 // #endif
```
to have the command in line 54 be active all the time. I want to be informed when there is a
communication failure.

Paragraph 12 - Install 8266 LittleFS Data Upload function

In the Arduino IDE I installed the functionality to upload font file data to the ESP8266
using LittleFS. The software and installation instructions you can find via:
(https://github.com/earlephilhower/arduino-esp8266littlefs.plugin) . Following the instructions,
I created a /data subfolder in my Arduino sketchfolder. In this folder, for test,
I copied a thumbnail .bmp file. I uploaded it using the new installed LittleFS plugin.
As described above, later I uploaded the font file in this way.

Paragraph 13 - data request interval

The interval of requests for Spotify Player data currently is set in this sketch
to 30 seconds. See line 162 below:

```
   unsigned long delayBetweenRequests = 30000; // Time between requests (30 seconds)
```

To my perception this is quite long. When the Spotify Player on your device
is running and when there is a change in track, we have to wait 30 seconds (worst case scenario) to receive an update
visual on the tft display and/or other output selected. I did not (yet) read guidelines for the refresh
requests, but I expect that one cannot push too many requests in a short period of time. There will be a chance of getting
a kind of refusal or blocking. To compensate this I added button press functionality. See the next paragraph.

Paragraph 14 - Implementation of button press awareness

Building on earlier experience built up using a D1 Mini and the two buttons on the tft hat, I implemented a `buttonpress awareness` 
into this sketch. I added a function `ck_btn()`. In the SAA class I added the flag `SAA_BTNPRESSED`. I also adapted
the function getFlagName(). I added calls to ck_btn() inside the functions: loop(), disp_artists() and displayCurrentlyPlayingOnScreen() (the latter in
two places), to increase the number of times the buttons will get polled. As soon as the flag SAA_BTNPRESSED is active, the current function,
e.g.: displayCurrentlyPlayingOnScreen() will be left and control returns to loop() where the flag SAA_BTNPRESSED will be honored with sending a
Spotify get player data request. The 'sensitivity' for button presses is not optimum because we use a polling method and not an interrupt method, but the added functionality is working. See an example of the Monitor output:

```
    Note: sketch execution inside 
    function displayCurrentlyPlayingOnScreen().
    Button D8 (button 2) was pressed. This event is registrated and handled:

    18:29:39.595 -> howmuch_to_loop = 29995
    18:29:39.643 -> artist name: 'Mundo Segundo'
    18:29:39.643 -> track name: 'Sempre Grato'
    18:29:39.690 -> album name: 'Sempre Grato'
    18:29:39.738 -> result of spotify.getCurrentPlaying(): 200
    18:29:45.958 -> <<<=== Button D8 (= button 2) pressed. ===>>>
    18:29:46.005 -> Spotify Album Art (SAA) Flags
    18:29:46.005 -> +----------------+---------+
    18:29:46.051 -> |      Flag:     | Status: |
    18:29:46.098 -> +----------------+---------+
    18:29:46.098 -> | IsPlaying      |    1    |
    18:29:46.145 -> | ImageShown     |    1    |
    18:29:46.191 -> | ImageLoadAgain |    0    |
    18:29:46.238 -> | ButtonPressed  |    1    |
    18:29:46.238 -> +----------------+---------+
    18:29:46.285 -> ================================================================================
    18:29:46.333 -> Spotify Album Art (SAA) Flags
    18:29:46.380 -> +----------------+---------+
    18:29:46.426 -> |      Flag:     | Status: |
    18:29:46.473 -> +----------------+---------+
    18:29:46.473 -> | IsPlaying      |    1    |
    18:29:46.521 -> | ImageShown     |    1    |
    18:29:46.567 -> | ImageLoadAgain |    0    |
    18:29:46.567 -> | ButtonPressed  |    1    |
    18:29:46.612 -> +----------------+---------+
    18:29:46.661 -> loop(): getting info on currently playing song:
    18:29:46.661 -> A button has been pressed. Going to send a get Playing data request
    18:29:46.755 -> Elapsed: 8 Sec
    18:29:46.802 -> heap_info(): Free Heap: 10072
    18:29:46.802 -> /v1/me/player/currently-playing?market=PT
    18:29:46.849 -> SpotifyArduino.printstack(): stack size 0xC0000410
    18:29:48.622 -> SpotifyArduino.getHttpStatusCode(): Status: HTTP/1.0 200 OK
    18:29:48.669 -> HTTP Version: HTTP/1.0
    18:29:48.717 -> SpotifyArduino.getHttpStatusCode(): Status Code: 200
    18:29:48.764 -> SpotifyArduino.getCurrentlyPlaying(): Status Code: 200
    18:29:48.859 -> SpotifyArduino.printstack(): stack size 0xC0000410
    18:29:48.859 -> {
    18:29:48.859 ->   "timestamp" : 1635704831352,
    18:29:48.904 ->   "context" : {
```

# Final notes:

To be able to use the Spotify Album Art sketch one needs to have at least a `Spotify Premium account`.
Then one has to create a `Spotify Developer account`. 
After this account is created one has to create at least one app in the Developer website `personal Dashboard`.
Then, in the `Settings` one has to add the `return address`, e.g.: `http:192.168.0.120/callback/` to whitelist
this address to redirect to after authentication success OR failure. This is needed during the creation
of the first RefreshToken.
Next one has to build, flash and run the sketch `getRefreshToken` (from the examples of the `spotify-api-arduino`
repo of Brian Lough). If this run proceeds successful, the obtained `RefreshToken` has to be inserted into
the `Spotify_Album_Art.ino` sketch, where is the line:
```
    //#define SPOTIFY_REFRESH_TOKEN "AAAAAAAAAABBBBBBBBBBBCCCCCCCCCCCDDDDDDDDDDD"
```
If I counted well, the RefreshToken that I received was 132 characters long.
This RefreshToken will be renewed automatically during the use of the sketch.


That's all Folks!

Feedback welcome. 

I am just an imperfect hobbyist.

Paulus Schulinck
