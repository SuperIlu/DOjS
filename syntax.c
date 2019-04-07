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
    EDI_SYNTAX(LIGHTCYAN, ".length"),       //
    EDI_SYNTAX(LIGHTCYAN, ".toString"),     //
    EDI_SYNTAX(LIGHTCYAN, ".concat"),       //
    EDI_SYNTAX(LIGHTCYAN, ".join"),         //
    EDI_SYNTAX(LIGHTCYAN, ".pop"),          //
    EDI_SYNTAX(LIGHTCYAN, ".push"),         //
    EDI_SYNTAX(LIGHTCYAN, ".reverse"),      //
    EDI_SYNTAX(LIGHTCYAN, ".shift"),        //
    EDI_SYNTAX(LIGHTCYAN, ".slice"),        //
    EDI_SYNTAX(LIGHTCYAN, ".sort"),         //
    EDI_SYNTAX(LIGHTCYAN, ".splice"),       //
    EDI_SYNTAX(LIGHTCYAN, ".unshift"),      //
    EDI_SYNTAX(LIGHTCYAN, ".indexOf"),      //
    EDI_SYNTAX(LIGHTCYAN, ".lastIndexOf"),  //
    EDI_SYNTAX(LIGHTCYAN, ".every"),        //
    EDI_SYNTAX(LIGHTCYAN, ".some"),         //
    EDI_SYNTAX(LIGHTCYAN, ".forEach"),      //
    EDI_SYNTAX(LIGHTCYAN, ".map"),          //
    EDI_SYNTAX(LIGHTCYAN, ".filter"),       //
    EDI_SYNTAX(LIGHTCYAN, ".reduce"),       //
    EDI_SYNTAX(LIGHTCYAN, ".reduceRight"),  //

    // DOjS functions
    EDI_SYNTAX(LIGHTRED, "MouseCursorIsDisplayed"),  //
    EDI_SYNTAX(LIGHTRED, "FilledConvexPolygon"),     //
    EDI_SYNTAX(LIGHTRED, "IpxGetLocalAddress"),      //
    EDI_SYNTAX(LIGHTRED, "MouseSetCursorMode"),      //
    EDI_SYNTAX(LIGHTRED, "FilledEllipseArc"),        //
    EDI_SYNTAX(LIGHTRED, "CustomEllipseArc"),        //
    EDI_SYNTAX(LIGHTRED, "FilledCircleArc"),         //
    EDI_SYNTAX(LIGHTRED, "MouseShowCursor"),         //
    EDI_SYNTAX(LIGHTRED, "ModuleIsPlaying"),         //
    EDI_SYNTAX(LIGHTRED, "CustomCircleArc"),         //
    EDI_SYNTAX(LIGHTRED, "MouseSetColors"),          //
    EDI_SYNTAX(LIGHTRED, "IpxSocketClose"),          //
    EDI_SYNTAX(LIGHTRED, "IpxCheckPacket"),          //
    EDI_SYNTAX(LIGHTRED, "MouseSetLimits"),          //
    EDI_SYNTAX(LIGHTRED, "MouseGetLimits"),          //
    EDI_SYNTAX(LIGHTRED, "CustomPolyLine"),          //
    EDI_SYNTAX(LIGHTRED, "IpxSocketOpen"),           //
    EDI_SYNTAX(LIGHTRED, "FilledEllipse"),           //
    EDI_SYNTAX(LIGHTRED, "MidiIsPlaying"),           //
    EDI_SYNTAX(LIGHTRED, "SetInstrument"),           //
    EDI_SYNTAX(LIGHTRED, "NumFreeColors"),           //
    EDI_SYNTAX(LIGHTRED, "CustomEllipse"),           //
    EDI_SYNTAX(LIGHTRED, "CustomPolygon"),           //
    EDI_SYNTAX(LIGHTRED, "MouseSetSpeed"),           //
    EDI_SYNTAX(LIGHTRED, "MouseSetAccel"),           //
    EDI_SYNTAX(LIGHTRED, "FilledPolygon"),           //
    EDI_SYNTAX(LIGHTRED, "SaveBmpImage"),            //
    EDI_SYNTAX(LIGHTRED, "CustomCircle"),            //
    EDI_SYNTAX(LIGHTRED, "IpxGetPacket"),            //
    EDI_SYNTAX(LIGHTRED, "SavePngImage"),            //
    EDI_SYNTAX(LIGHTRED, "FilledCircle"),            //
    EDI_SYNTAX(LIGHTRED, "SetFramerate"),            //
    EDI_SYNTAX(LIGHTRED, "GetFramerate"),            //
    EDI_SYNTAX(LIGHTRED, "ClearScreen"),             //
    EDI_SYNTAX(LIGHTRED, "FloodSpill2"),             //
    EDI_SYNTAX(LIGHTRED, "EllipseArc"),              //
    EDI_SYNTAX(LIGHTRED, "CustomLine"),              //
    EDI_SYNTAX(LIGHTRED, "ModuleStop"),              //
    EDI_SYNTAX(LIGHTRED, "MemoryInfo"),              //
    EDI_SYNTAX(LIGHTRED, "SetExitKey"),              //
    EDI_SYNTAX(LIGHTRED, "FloodSpill"),              //
    EDI_SYNTAX(LIGHTRED, "CustomBox"),               //
    EDI_SYNTAX(LIGHTRED, "FloodFill"),               //
    EDI_SYNTAX(LIGHTRED, "FramedBox"),               //
    EDI_SYNTAX(LIGHTRED, "CircleArc"),               //
    EDI_SYNTAX(LIGHTRED, "FilledBox"),               //
    EDI_SYNTAX(LIGHTRED, "NumColors"),               //
    EDI_SYNTAX(LIGHTRED, "MouseWarp"),               //
    EDI_SYNTAX(LIGHTRED, "MidiStop"),                //
    EDI_SYNTAX(LIGHTRED, "PolyLine"),                //
    EDI_SYNTAX(LIGHTRED, "CharCode"),                //
    EDI_SYNTAX(LIGHTRED, "Ellipse"),                 //
    EDI_SYNTAX(LIGHTRED, "IpxSend"),                 //
    EDI_SYNTAX(LIGHTRED, "Require"),                 //
    EDI_SYNTAX(LIGHTRED, "NoteOff"),                 //
    EDI_SYNTAX(LIGHTRED, "Println"),                 //
    EDI_SYNTAX(LIGHTRED, "Include"),                 //
    EDI_SYNTAX(LIGHTRED, "Polygon"),                 //
    EDI_SYNTAX(LIGHTRED, "NoteOn"),                  //
    EDI_SYNTAX(LIGHTRED, "Circle"),                  //
    EDI_SYNTAX(LIGHTRED, "TextXY"),                  //
    EDI_SYNTAX(LIGHTRED, "Debug"),                   //
    EDI_SYNTAX(LIGHTRED, "Setup"),                   //
    EDI_SYNTAX(LIGHTRED, "Input"),                   //
    EDI_SYNTAX(LIGHTRED, "Print"),                   //
    EDI_SYNTAX(LIGHTRED, "Sleep"),                   //
    EDI_SYNTAX(LIGHTRED, "SizeX"),                   //
    EDI_SYNTAX(LIGHTRED, "SizeY"),                   //
    EDI_SYNTAX(LIGHTRED, "Loop"),                    //
    EDI_SYNTAX(LIGHTRED, "MaxX"),                    //
    EDI_SYNTAX(LIGHTRED, "MaxY"),                    //
    EDI_SYNTAX(LIGHTRED, "Plot"),                    //
    EDI_SYNTAX(LIGHTRED, "Line"),                    //
    EDI_SYNTAX(LIGHTRED, "Read"),                    //
    EDI_SYNTAX(LIGHTRED, "List"),                    //
    EDI_SYNTAX(LIGHTRED, "Stat"),                    //
    EDI_SYNTAX(LIGHTRED, "Stop"),                    //
    EDI_SYNTAX(LIGHTRED, "Box"),                     //
    EDI_SYNTAX(LIGHTRED, "Gc"),                      //

    EDI_SYNTAX(RED, ".Draw"),          //
    EDI_SYNTAX(RED, ".Play"),          //
    EDI_SYNTAX(RED, ".Stop"),          //
    EDI_SYNTAX(RED, ".Close"),         //
    EDI_SYNTAX(RED, ".Resize"),        //
    EDI_SYNTAX(RED, ".GetRed"),        //
    EDI_SYNTAX(RED, ".GetBlue"),       //
    EDI_SYNTAX(RED, ".GetGreen"),      //
    EDI_SYNTAX(RED, ".ReadByte"),      //
    EDI_SYNTAX(RED, ".ReadLine"),      //
    EDI_SYNTAX(RED, ".WriteByte"),     //
    EDI_SYNTAX(RED, ".WriteLine"),     //
    EDI_SYNTAX(RED, ".DrawString"),    //
    EDI_SYNTAX(RED, ".StringWidth"),   //
    EDI_SYNTAX(RED, ".WriteString"),   //
    EDI_SYNTAX(RED, ".StringHeight"),  //

    EDI_SYNTAX_END  // end of list
};
