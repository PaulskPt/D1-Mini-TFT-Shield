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

Paragraph 8 - Handling Latin-1 and Latin-5 group letters

Added handling of letters in the ASCII extended `Latin-1` range.
These letters consist of 2 bytes. For Latin-1 group of letters,
the first byte has always a value of `0xc3` followed by a byte 
with a value in the range 0xa0 - 0xff. The Latin-5 have a 
lead byte value of `0xc4`or `0xc5`.
To be able to display `Latin-5` group 
letters, I downloaded and converted a font that contains both
Latin-1 and Latin-5 group letters. It is the `RubikReg32.ttf`
font that I converted to `RubikReg3218.vlw`.

I encountered two problems while handling letters in the 
Latin-1 and Latin-5 group blocks:

Paragraph 8.1

the `string.length()` function counts a Latin-1 and Latin-5 group letter as 2 bytes, 
because it are two bytes, but only one byte is visual on tft, 
the IDE Monitor or Serial output. Displaying strings containing Latin-1 or Latin-5
group letters on tft requires correction in the calculation of the length.
Below I explain my workaround to this problem.

Paragraph 8.2

the `TFT_eSPI` library does not display Latin-1  neither Latin-5 group
capital letters. I created a function that converts the capital letters to small letters.

_Workaround:_

Paragraph 8.1.1

In the function `scanExtended()` the input string is scanned 
for occurrances of `lead byte` values of
`Latin-1` and `Latin-5` Extended ASCII group letters (0xc3, 0xc4 and 0xc5).
The function returns the number of 2-byte letters encountered.
In the function displayCurrentlyPlayingOnScreen() are three calls to this function:
`an_Extended = scanExtended(an0, an0.length());`
`tn_Extended = scanExtended(tn0, tn0.length());`
`abn_Extended = scanExtended(abn0, abn0.length());`

To correct the wrong outcome of string.length() with
Latin-1 and Latin-5 group characters I introduced this method:
the values `an_Extended`, `tn_Extended` and `abn_Extended` are use,
used in string length calculations for artist, track and album name, e.g.:

```
    an_le  = an.length() - an_Extended;
```


Paragraph 8.2.1 Handling of Latin-1 group letters

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
            Serial.print(F("] lead byte value to put into output (io) = 0x"));
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
            Serial.print(F("] The value of the converted character  = 0x"));
            Serial.println(c2, HEX);
        }
    }
    else {
        io += c;  // A non Latin-1 letter: put immediately to the output
    }
    [...]
```

Paragraph 8.2.1.2

   If it is any other letter in the string than the 1st letter:
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
    ConvUpperToLower(): received to convert: Artist name: 'Sabahat Akkiraz'
    Intl Std Recording Code (first 2 letters only): TR, is country: Turkey
    The length of the Artist name: 15
    Checking need for the conversion of Artist name
    Returnvalue: 'Sabahat Akkiraz'
    Nr of ASCII Extended letters in Artist Name: 0
    value of variable an_le = 16
    ConvUpperToLower(): received to convert: Track name: 'Fenerbahçe Geliyor'
    Intl Std Recording Code (first 2 letters only): TR, is country: Turkey
    The length of the Track name: 19
    Checking need for the conversion of Track name
    We have an ASCII Latin-1 supplement character
    in[8] lead byte value to put into output (io): = 0xC3
    in[9] the value of the 2nd byte                = 0xA7
    in[9] The value of the converted character     = 0xE7
    Returnvalue: 'Fenerbah⸮⸮e Geliyor'
    Nr of ASCII Extended letters in Track Name: 1
    value of variable tn_le = 18
    Intl Std Recording Code (first 2 letters only): TR, is country: Turkey
    The length of the Album name: 19
    Checking need for the conversion of Album name
    We have an ASCII Latin-1 supplement character
    in[8] lead byte value to put into output (io): = 0xC3
    in[9] the value of the 2nd byte                = 0xA7
    in[9] The value of the converted character     = 0xE7
    Returnvalue: 'Fenerbah⸮⸮e Geliyor'
    Nr of ASCII Extended letters in Album Name: 1
    value of variable abn2_le = 19
    Width available to write on tft: 24
    length of album name is: 19
    length of artist #1 name is: 16
    length of track name is: 18
    howmuch_to_loop = 29995
    artist name: 'Sabahat Akkiraz'
    track name: 'Fenerbahçe Geliyor'
    album name: 'Fenerbahçe Geliyor'
    result of spotify.getCurrentPlaying(): 200

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

    `ToDo`: move this option to the .ini file.

Paragraph 8.2.1.3 Handling of Latin-5 letters

The Latin-5 Extended ASCII group of letters are handled by using a font type that 
contains these letters. I became interested in solving the fact that, initially, I was
not able to display Latin-5 group letters on the tft. I became 'triggered'
because I had various Turkish music that I streamed via Spotify. A good explanation
about existing problems of how to handle Turkish letters, I found on this webpage: 
https://meta.wikimedia.org/wiki/Help:Turkish_characters
A complicating fact was that I also struggled with the fact that I didn't understand 
how I could create a font with a corps size large enough to be readable on the small tft. 
After many days of experimenting, reading, google'ing, I discovered that the 'solution' 
lied inside the Create_font.pde, line 137, beside the fact that in the same sketch one has to 
select the blocks of characters that one wants to be contained in the font. 
Ultimately I succeeded to download a suitable font,
convert it with the correct corps size, flash it and used it. More about this below.
   
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
https://fonts.google.com/noto/specimen/Noto+Sans+Display. Days later I decided for the Rubik font,
also from Google Noto.
Within the chosen font family, I chose the 'Regular 400' font. 
A file which I named: 'NotSDispSemiCond-ELight.ttf' (353 kB),
then created an own work folder and edited a copy of the Create_font file 'Create_font.pde'
In that file I decided to use the following blocks of characters:
```
0x0021, 0x007E  // Basic Latin, 128, 128, Latin (52 characters), Common (76 characters);
0x00E0, 0x00FF, // Latin-1 Supplement, 128, 128, Latin (64 characters), Common (64 characters);
0x0100, 0x017F, //Latin Extended-A, 128, 128, Latin  (Also called Latin-5)
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
It is important to note that in line 137 of the sketch Create_font.pde is the definition:
`int  fontSize = 18`; (in my case, that is). Here one decides the font corps size for the
font file to create. This is an important value. In the beginning I did not pay attention
to this and ended up with very tiny letters on the tft, too small to read!
After a successfull conversion I copied the resulting file `RubikReg3218.vlw` (16 kB) to the 
`/data` subfolder of my Arduino sketch folder. Then I successfully flashed this .vlw file
using the Arduino IDE > Tools > `ESP8266 LittleFS Data Upload` function.

In the sketch, function `setup()`, lines 1673 and 1674, is the place where the function is called
that loads the selected font.

```
    1673  fontFileIdx = 1;  // Choose RubikReg3218
    1674  n = openFontFile(fontFileIdx);  // Open the file of the 1st font in the fonts[] array.
    1675  if (n == -1)
    1676  perpetual_loop();

```

I also added a functionality to change the font upon a press of butto #2 (D8). When button #2 is
pressed a call is made to function `chgFontFileIdx`.

Paragraph 10

I added a function called `readIni()`. This function reads the contents of the file `saa.ini`.
In this moment the .ini file contains only one line containing the names of the font files 
flashed to the microcontroller followed by the default font corps size. The current contents
of this .ini file is:

```
    NotoRegular18,RubikReg3218,18
```

The saa.ini file is flashed together with the font files. The contents of this .ini file is read 
after reset.


If the global variable 'my_debug' is true, the following text will be printed to the IDE
Monitor or REPL:

```
    Setup():

    Initialisation done.
    Listing directory: /
    FILE: NotoRegular18.vlw	SIZE: 15748
    FILE: RubikReg3218.vlw	SIZE: 41524
    FILE: album.jpg	SIZE: 29359
    FILE: saa.ini	SIZE: 45
    readIni():
    Nr of font files on disk: 2
    Reading file: '/saa.ini'
    - read from file:
    Size of the fonts array: 2
    fontFiles on disk: 
    [0]	NotoRegular18
    [1]	RubikReg3218
    Font size: 18

```
      
Outside the sketch itself I made some `cosmetic changes` to facilitate or enhance debug output:

Paragraph 11 - Changes to Smooth_font.cpp

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

Paragraph 12 - Changes to SpotifyArduino.cpp

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

Paragraph 13 - Install 8266 LittleFS Data Upload function

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
into this sketch. I added a function `ck_btn()`. In the SAA class I added the flags `SAA_BTN1PRESSED` and `SAA_BTN2PRESSED`. I also adapted
the function getFlagName(). I added calls to ck_btn() inside the functions: loop(), disp_artists() and displayCurrentlyPlayingOnScreen() (the latter in
two places), to increase the number of times the buttons will get polled.

The functions of the buttons is as follows:
```
    +-------------+-----------------------------------------+
    | Button:     | Funmction:                              |
    +-------------+-----------------------------------------+
    | Button 1    | send an ad-hoc data request to Spotify  |
    +-------------+-----------------------------------------+  
    | Button 2    | load the next available font            |
    +-------------+-----------------------------------------+
```

As soon as button 1 is pressed, the current function, e.g.: displayCurrentlyPlayingOnScreen() will be left and control returns to loop()
where the flag SAA_BTN1PRESSED will be honored with sending a Spotify get player data request. The 'sensitivity' 
for button presses is not optimum because we use a polling method and not an interrupt method, but the added functionality is working. 
Button 2 press will be followed-up from inside the ck_btn() function.
See an example of the Monitor output:

```
    Note: sketch execution inside 
    function displayCurrentlyPlayingOnScreen().
    Button D3 (button 1) was pressed. This event is registrated and handled:

    howmuch_to_loop = 29995
    artist name: 'Mundo Segundo'
    track name: 'Sempre Grato'
    album name: 'Sempre Grato'
    result of spotify.getCurrentPlaying(): 200
    <<<=== Button D3 (= button 1) pressed. ===>>>
    Spotify Album Art (SAA) Flags
    +-----------------+---------+
    |      Flag:      | Status: |
    +-----------------+---------+
    | IsPlaying       |    1    |
    | ImageShown      |    1    |
    | ImageLoadAgain  |    0    |
    | Button1 Pressed |    1    |
    | Button2 Pressed |    0    |
    +-----------------+---------+
    ================================================================================
    Spotify Album Art (SAA) Flags
    +-----------------+---------+
    |      Flag:      | Status: |
    +-----------------+---------+
    | IsPlaying       |    1    |
    | ImageShown      |    1    |
    | ImageLoadAgain  |    0    |
    | Button1 Pressed |    0    |
    | Button2 Pressed |    0    |
    +-----------------+---------+
    loop(): getting info on currently playing song:
    Button 1 has been pressed. Going to send a get Playing data request
    Elapsed: 8 Sec
    heap_info(): Free Heap: 10072
    /v1/me/player/currently-playing?market=PT
    SpotifyArduino.printstack(): stack size 0xC0000410
    SpotifyArduino.getHttpStatusCode(): Status: HTTP/1.0 200 OK
    HTTP Version: HTTP/1.0
    SpotifyArduino.getHttpStatusCode(): Status Code: 200
    SpotifyArduino.getCurrentlyPlaying(): Status Code: 200
    SpotifyArduino.printstack(): stack size 0xC0000410
    {
    "timestamp" : 1635704831352,
    "context" : {
```

Next the following text will be printed to the Monitor output, depended
on the settings in TFT_eSPI/User_setup.h: (or, as I decided: in the folder:
C:\Users\<User>\Documents\Arduino\libraries\TFT_eSPI_Setups\User_setup.h.
This information is retrieved through a call to `tft.fontsLoaded()`. The 
table below is printed by the sketch function `listFontsLoaded(fontsld)`.
The UTF_SWITCH state info (below) is retrieved from sketch function loop() 
through the call: `tft.getAttribute(2);`

```
    Loading of fontfile: 'RubikReg3218' successful
    clr_tft_down_part(): vt = 55
    see also on tft
    ================================================================================
    tft.fontsLoaded = 0b1000000111010110
    Spotify Album Art (SAA) Fonts
    +-----------------+---------+
    |      Font:      | Loaded: |
    +-----------------+---------+
    |   LOAD_GLCD     |   1     |
    |   LOAD_FONT2    |   1     |
    |   LOAD_FONT4    |   1     |
    |   LOAD_FONT6    |   1     |
    |   LOAD_FONT7    |   1     |
    |   LOAD_FONT8    |   1     |
    |  LOAD_FONT8N    |   0     |
    |  SMOOTH_FONT    |   1     |
    +-----------------+---------+
    Font loaded: RubikReg3218
    Font size: 18
    fontHeight = 22
    UTF8_SWITCH state = 1

```

Paragraph 14 - Sketch divided for the use of Free Fonts or own flashed font (.vlw)

After experimenting with both  `Free Fonts` and `flashed own font` (.vlw) I decided for the latter.
I created the pre-compiler define USE_FREE_FONTS to devide the functionality of the sketch in two parts.

```
    #ifdef USE_FREE_FONTS
    ... code concerning the use of free fonts...
    #else
    ... code to use the own .vlw type of font
    #endif
```

This type of division occurs in several sections of the script.
Because, after my decision to use a .vlw font, that part of the script has gotten most of the attention 
in the final days of the development of this sketch.


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
