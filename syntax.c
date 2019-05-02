/*
MIT License

Copyright (c) 2019 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <conio.h>
#include <stddef.h>

#include "edit.h"

syntax_t edi_wordlist[] = {
    // keywords
    EDI_SYNTAX_EOL(CYAN, "//"),            //
    EDI_SYNTAX(LIGHTMAGENTA, "if"),        //
    EDI_SYNTAX(LIGHTMAGENTA, "do"),        //
    EDI_SYNTAX(YELLOW, "var"),             //
    EDI_SYNTAX(LIGHTMAGENTA, "for"),       //
    EDI_SYNTAX(LIGHTMAGENTA, "try"),       //
    EDI_SYNTAX(LIGHTBLUE, "new"),          //
    EDI_SYNTAX(LIGHTBLUE, "this"),         //
    EDI_SYNTAX(BROWN, "true"),             //
    EDI_SYNTAX(LIGHTMAGENTA, "case"),      //
    EDI_SYNTAX(BROWN, "null"),             //
    EDI_SYNTAX(LIGHTMAGENTA, "else"),      //
    EDI_SYNTAX(BROWN, "false"),            //
    EDI_SYNTAX(LIGHTMAGENTA, "break"),     //
    EDI_SYNTAX(LIGHTMAGENTA, "while"),     //
    EDI_SYNTAX(LIGHTMAGENTA, "catch"),     //
    EDI_SYNTAX(LIGHTMAGENTA, "switch"),    //
    EDI_SYNTAX(LIGHTBLUE, "typeof"),       //
    EDI_SYNTAX(LIGHTMAGENTA, "return"),    //
    EDI_SYNTAX(LIGHTMAGENTA, "finally"),   //
    EDI_SYNTAX(LIGHTMAGENTA, "continue"),  //
    EDI_SYNTAX(LIGHTGREEN, "function"),    //
    EDI_SYNTAX(LIGHTBLUE, "instanceof"),   //
    EDI_SYNTAX(LIGHTBLUE, ".prototype"),   //

    // array methods
    EDI_SYNTAX(LIGHTCYAN, ".lastIndexOf"),  //
    EDI_SYNTAX(LIGHTCYAN, ".reduceRight"),  //
    EDI_SYNTAX(LIGHTCYAN, ".toString"),     //
    EDI_SYNTAX(LIGHTCYAN, ".reverse"),      //
    EDI_SYNTAX(LIGHTCYAN, ".unshift"),      //
    EDI_SYNTAX(LIGHTCYAN, ".indexOf"),      //
    EDI_SYNTAX(LIGHTCYAN, ".forEach"),      //
    EDI_SYNTAX(LIGHTCYAN, ".length"),       //
    EDI_SYNTAX(LIGHTCYAN, ".concat"),       //
    EDI_SYNTAX(LIGHTCYAN, ".filter"),       //
    EDI_SYNTAX(LIGHTCYAN, ".reduce"),       //
    EDI_SYNTAX(LIGHTCYAN, ".splice"),       //
    EDI_SYNTAX(LIGHTCYAN, ".every"),        //
    EDI_SYNTAX(LIGHTCYAN, ".shift"),        //
    EDI_SYNTAX(LIGHTCYAN, ".slice"),        //
    EDI_SYNTAX(LIGHTCYAN, ".some"),         //
    EDI_SYNTAX(LIGHTCYAN, ".push"),         //
    EDI_SYNTAX(LIGHTCYAN, ".sort"),         //
    EDI_SYNTAX(LIGHTCYAN, ".join"),         //
    EDI_SYNTAX(LIGHTCYAN, ".pop"),          //
    EDI_SYNTAX(LIGHTCYAN, ".map"),          //

    // DOjS functions
    EDI_SYNTAX(LIGHTRED, "IpxGetLocalAddress"),  //
    EDI_SYNTAX(LIGHTRED, "MouseSetCursorMode"),  //
    EDI_SYNTAX(LIGHTRED, "CustomCircleArc"),     //
    EDI_SYNTAX(LIGHTRED, "MouseShowCursor"),     //
    EDI_SYNTAX(LIGHTRED, "IpxSocketClose"),      //
    EDI_SYNTAX(LIGHTRED, "MouseSetLimits"),      //
    EDI_SYNTAX(LIGHTRED, "IpxCheckPacket"),      //
    EDI_SYNTAX(LIGHTRED, "GetScreenMode"),       //
    EDI_SYNTAX(LIGHTRED, "MidiIsPlaying"),       //
    EDI_SYNTAX(LIGHTRED, "FilledEllipse"),       //
    EDI_SYNTAX(LIGHTRED, "FilledPolygon"),       //
    EDI_SYNTAX(LIGHTRED, "IpxSocketOpen"),       //
    EDI_SYNTAX(LIGHTRED, "CustomEllipse"),       //
    EDI_SYNTAX(LIGHTRED, "MouseSetSpeed"),       //
    EDI_SYNTAX(LIGHTRED, "SetFramerate"),        //
    EDI_SYNTAX(LIGHTRED, "FilledCircle"),        //
    EDI_SYNTAX(LIGHTRED, "GetFramerate"),        //
    EDI_SYNTAX(LIGHTRED, "CustomCircle"),        //
    EDI_SYNTAX(LIGHTRED, "IpxGetPacket"),        //
    EDI_SYNTAX(LIGHTRED, "SaveBmpImage"),        //
    EDI_SYNTAX(LIGHTRED, "SavePcxImage"),        //
    EDI_SYNTAX(LIGHTRED, "SaveTgaImage"),        //
    EDI_SYNTAX(LIGHTRED, "ClearScreen"),         //
    EDI_SYNTAX(LIGHTRED, "MidiGetTime"),         //
    EDI_SYNTAX(LIGHTRED, "SetExitKey"),          //
    EDI_SYNTAX(LIGHTRED, "CustomLine"),          //
    EDI_SYNTAX(LIGHTRED, "MidiResume"),          //
    EDI_SYNTAX(LIGHTRED, "MemoryInfo"),          //
    EDI_SYNTAX(LIGHTRED, "MidiPause"),           //
    EDI_SYNTAX(LIGHTRED, "CircleArc"),           //
    EDI_SYNTAX(LIGHTRED, "FilledBox"),           //
    EDI_SYNTAX(LIGHTRED, "FloodFill"),           //
    EDI_SYNTAX(LIGHTRED, "MouseWarp"),           //
    EDI_SYNTAX(LIGHTRED, "GetAlpha"),            //
    EDI_SYNTAX(LIGHTRED, "GetGreen"),            //
    EDI_SYNTAX(LIGHTRED, "MidiStop"),            //
    EDI_SYNTAX(LIGHTRED, "GetPixel"),            //
    EDI_SYNTAX(LIGHTRED, "MsecTime"),            //
    EDI_SYNTAX(LIGHTRED, "GetBlue"),             //
    EDI_SYNTAX(LIGHTRED, "Ellipse"),             //
    EDI_SYNTAX(LIGHTRED, "Println"),             //
    EDI_SYNTAX(LIGHTRED, "IpxSend"),             //
    EDI_SYNTAX(LIGHTRED, "MidiOut"),             //
    EDI_SYNTAX(LIGHTRED, "TextXY"),              //
    EDI_SYNTAX(LIGHTRED, "Circle"),              //
    EDI_SYNTAX(LIGHTRED, "GetRed"),              //
    EDI_SYNTAX(LIGHTRED, "Print"),               //
    EDI_SYNTAX(LIGHTRED, "SizeX"),               //
    EDI_SYNTAX(LIGHTRED, "SizeY"),               //
    EDI_SYNTAX(LIGHTRED, "Sleep"),               //
    EDI_SYNTAX(LIGHTRED, "Color"),               //
    EDI_SYNTAX(LIGHTRED, "Line"),                //
    EDI_SYNTAX(LIGHTRED, "List"),                //
    EDI_SYNTAX(LIGHTRED, "Plot"),                //
    EDI_SYNTAX(LIGHTRED, "Stat"),                //
    EDI_SYNTAX(LIGHTRED, "Stop"),                //
    EDI_SYNTAX(LIGHTRED, "Read"),                //
    EDI_SYNTAX(LIGHTRED, "Box"),                 //
    EDI_SYNTAX(LIGHTRED, "Gc"),                  //

    EDI_SYNTAX(RED, ".DrawStringCenter"),  // Font
    EDI_SYNTAX(RED, ".DrawStringRight"),   // Font
    EDI_SYNTAX(RED, ".DrawStringLeft"),    // Font
    EDI_SYNTAX(RED, ".StringHeight"),      // Font
    EDI_SYNTAX(RED, ".WriteString"),       // File
    EDI_SYNTAX(RED, ".StringWidth"),       // Font
    EDI_SYNTAX(RED, ".WriteLine"),         // File
    EDI_SYNTAX(RED, ".WriteByte"),         // File
    EDI_SYNTAX(RED, ".ReadLine"),          // File
    EDI_SYNTAX(RED, ".ReadByte"),          // File
    EDI_SYNTAX(RED, ".GetPixel"),          // Bitmap
    EDI_SYNTAX(RED, ".Close"),             // File
    EDI_SYNTAX(RED, ".Stop"),              // Sample
    EDI_SYNTAX(RED, ".Draw"),              // Bitmap
    EDI_SYNTAX(RED, ".Play"),              // Midi

    EDI_SYNTAX(YELLOW, ".filename"),  // Bitmap/Font/Midi
    EDI_SYNTAX(YELLOW, ".length"),    // Midi
    EDI_SYNTAX(YELLOW, ".height"),    // Bitmap/Font
    EDI_SYNTAX(YELLOW, ".width"),     // Bitmap

    EDI_SYNTAX_END  // end of list
};
