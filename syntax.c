/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

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
#include "syntax.h"

#include <conio.h>
#include <stddef.h>

// enum COLORS {
//     BLACK,  // background
//     BLUE,
//     GREEN,    // strings
//     CYAN,     // comments
//     RED,      // DOjS methods
//     MAGENTA,  // Setup, Loop and Input
//     BROWN,    // keywords: true, false, null
//     LIGHTGRAY,
//     DARKGRAY,
//     LIGHTBLUE,     // keywords: instanceof .prototype
//     LIGHTGREEN,    // classes
//     LIGHTCYAN,     // array methods
//     LIGHTRED,      // DOjS functions
//     LIGHTMAGENTA,  // keywords: flow control
//     YELLOW,        // keywords: var / members
//     WHITE          // normal text
// };

const syntax_t edi_wordlist[] = {
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
    EDI_SYNTAX(LIGHTMAGENTA, "throw"),     //
    EDI_SYNTAX(LIGHTMAGENTA, "switch"),    //
    EDI_SYNTAX(LIGHTBLUE, "typeof"),       //
    EDI_SYNTAX(LIGHTMAGENTA, "return"),    //
    EDI_SYNTAX(LIGHTMAGENTA, "finally"),   //
    EDI_SYNTAX(LIGHTMAGENTA, "continue"),  //
    EDI_SYNTAX(LIGHTGREEN, "function"),    //
    EDI_SYNTAX(LIGHTBLUE, "instanceof"),   //
    EDI_SYNTAX(LIGHTBLUE, ".prototype"),   //

    // DOjS functions
    EDI_SYNTAX(MAGENTA, "Setup"),  //
    EDI_SYNTAX(MAGENTA, "Loop"),   //
    EDI_SYNTAX(MAGENTA, "Input"),  //

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
    EDI_SYNTAX(LIGHTRED, "fxAlphaControlsITRGBLighting"),  //
    EDI_SYNTAX(LIGHTRED, "fxGetNumPendingBufferSwaps"),    //
    EDI_SYNTAX(LIGHTRED, "fxGetMaxTextureAspectRatio"),    //
    EDI_SYNTAX(LIGHTRED, "fxAlphaTestReferenceValue"),     //
    EDI_SYNTAX(LIGHTRED, "NGetTransformationMatrix"),      //
    EDI_SYNTAX(LIGHTRED, "GetVectorRotationMatrix"),       //
    EDI_SYNTAX(LIGHTRED, "GetTransformationMatrix"),       //
    EDI_SYNTAX(LIGHTRED, "FxTexMemGetStartAddress"),       //
    EDI_SYNTAX(LIGHTRED, "fxGetGammaTableEntries"),        //
    EDI_SYNTAX(LIGHTRED, "fxDepthBufferFunction"),         //
    EDI_SYNTAX(LIGHTRED, "SetProjectionViewport"),         //
    EDI_SYNTAX(LIGHTRED, "JoystickCalibrateName"),         //
    EDI_SYNTAX(LIGHTRED, "fxTexCalcMemRequired"),          //
    EDI_SYNTAX(LIGHTRED, "fxGetFogTableEntries"),          //
    EDI_SYNTAX(LIGHTRED, "fxGammaCorrectionRGB"),          //
    EDI_SYNTAX(LIGHTRED, "fxConstantColorValue"),          //
    EDI_SYNTAX(LIGHTRED, "fxAlphaBlendFunction"),          //
    EDI_SYNTAX(LIGHTRED, "GetTranslationMatrix"),          //
    EDI_SYNTAX(LIGHTRED, "fxResetVertexLayout"),           //
    EDI_SYNTAX(LIGHTRED, "fxGetMaxTextureSize"),           //
    EDI_SYNTAX(LIGHTRED, "fxFogGenerateLinear"),           //
    EDI_SYNTAX(LIGHTRED, "fxDisableAllEffects"),           //
    EDI_SYNTAX(LIGHTRED, "fxAlphaTestFunction"),           //
    EDI_SYNTAX(LIGHTRED, "TransparencyEnabled"),           //
    EDI_SYNTAX(LIGHTRED, "fxTexDetailControl"),            //
    EDI_SYNTAX(LIGHTRED, "fxLfbConstantDepth"),            //
    EDI_SYNTAX(LIGHTRED, "fxLfbConstantAlpha"),            //
    EDI_SYNTAX(LIGHTRED, "fxFogTableIndexToW"),            //
    EDI_SYNTAX(LIGHTRED, "ReadSoundInputInts"),            //
    EDI_SYNTAX(LIGHTRED, "NGetRotationMatrix"),            //
    EDI_SYNTAX(LIGHTRED, "MouseSetCursorMode"),            //
    EDI_SYNTAX(LIGHTRED, "IpxStringToAddress"),            //
    EDI_SYNTAX(LIGHTRED, "IpxGetLocalAddress"),            //
    EDI_SYNTAX(LIGHTRED, "IpxAddressToString"),            //
    EDI_SYNTAX(LIGHTRED, "GetLoadedLibraries"),            //
    EDI_SYNTAX(LIGHTRED, "fxTexLodBiasValue"),             //
    EDI_SYNTAX(LIGHTRED, "fxGetZDepthMinMax"),             //
    EDI_SYNTAX(LIGHTRED, "fxGetWDepthMinMax"),             //
    EDI_SYNTAX(LIGHTRED, "fxFogGenerateExp2"),             //
    EDI_SYNTAX(LIGHTRED, "fxDrawVertexArray"),             //
    EDI_SYNTAX(LIGHTRED, "fxDepthBufferMode"),             //
    EDI_SYNTAX(LIGHTRED, "NGetZRotateMatrix"),             //
    EDI_SYNTAX(LIGHTRED, "NGetYRotateMatrix"),             //
    EDI_SYNTAX(LIGHTRED, "NGetXRotateMatrix"),             //
    EDI_SYNTAX(LIGHTRED, "JoystickCalibrate"),             //
    EDI_SYNTAX(LIGHTRED, "GetRotationMatrix"),             //
    EDI_SYNTAX(LIGHTRED, "GetLocalIpAddress"),             //
    EDI_SYNTAX(LIGHTRED, "GetIdentityMatrix"),             //
    EDI_SYNTAX(LIGHTRED, "fxGetRevisionTmu"),              //
    EDI_SYNTAX(LIGHTRED, "fxFogGenerateExp"),              //
    EDI_SYNTAX(LIGHTRED, "fxDepthBiasLevel"),              //
    EDI_SYNTAX(LIGHTRED, "fxChromakeyValue"),              //
    EDI_SYNTAX(LIGHTRED, "fxAADrawTriangle"),              //
    EDI_SYNTAX(LIGHTRED, "VoiceGetPosition"),              //
    EDI_SYNTAX(LIGHTRED, "SoundInputSource"),              //
    EDI_SYNTAX(LIGHTRED, "QTranslateMatrix"),              //
    EDI_SYNTAX(LIGHTRED, "JoystickSaveData"),              //
    EDI_SYNTAX(LIGHTRED, "JoystickLoadData"),              //
    EDI_SYNTAX(LIGHTRED, "GetZRotateMatrix"),              //
    EDI_SYNTAX(LIGHTRED, "GetYRotateMatrix"),              //
    EDI_SYNTAX(LIGHTRED, "GetXRotateMatrix"),              //
    EDI_SYNTAX(LIGHTRED, "GetScalingMatrix"),              //
    EDI_SYNTAX(LIGHTRED, "GetRawSectorSize"),              //
    EDI_SYNTAX(LIGHTRED, "GetParallelPorts"),              //
    EDI_SYNTAX(LIGHTRED, "fxTexMipMapMode"),               //
    EDI_SYNTAX(LIGHTRED, "fxTexMinAddress"),               //
    EDI_SYNTAX(LIGHTRED, "fxTexMaxAddress"),               //
    EDI_SYNTAX(LIGHTRED, "fxTexFilterMode"),               //
    EDI_SYNTAX(LIGHTRED, "fxGetVertexSize"),               //
    EDI_SYNTAX(LIGHTRED, "fxGetRevisionFb"),               //
    EDI_SYNTAX(LIGHTRED, "fxFogColorValue"),               //
    EDI_SYNTAX(LIGHTRED, "fxChromakeyMode"),               //
    EDI_SYNTAX(LIGHTRED, "SoundStartInput"),               //
    EDI_SYNTAX(LIGHTRED, "SetRenderBitmap"),               //
    EDI_SYNTAX(LIGHTRED, "NormalizeVector"),               //
    EDI_SYNTAX(LIGHTRED, "NPolygonZNormal"),               //
    EDI_SYNTAX(LIGHTRED, "MouseShowCursor"),               //
    EDI_SYNTAX(LIGHTRED, "GetCameraMatrix"),               //
    EDI_SYNTAX(LIGHTRED, "CustomCircleArc"),               //
    EDI_SYNTAX(LIGHTRED, "fxVertexLayout"),                //
    EDI_SYNTAX(LIGHTRED, "fxTexClampMode"),                //
    EDI_SYNTAX(LIGHTRED, "fxRenderBuffer"),                //
    EDI_SYNTAX(LIGHTRED, "fxGetNumBoards"),                //
    EDI_SYNTAX(LIGHTRED, "fxGetMemoryUma"),                //
    EDI_SYNTAX(LIGHTRED, "fxGetMemoryTmu"),                //
    EDI_SYNTAX(LIGHTRED, "fxGetBitsDepth"),                //
    EDI_SYNTAX(LIGHTRED, "fxDrawTriangle"),                //
    EDI_SYNTAX(LIGHTRED, "fxColorCombine"),                //
    EDI_SYNTAX(LIGHTRED, "fxAlphaCombine"),                //
    EDI_SYNTAX(LIGHTRED, "SoundStopInput"),                //
    EDI_SYNTAX(LIGHTRED, "ScenePolygon3D"),                //
    EDI_SYNTAX(LIGHTRED, "ReadSoundInput"),                //
    EDI_SYNTAX(LIGHTRED, "PolygonZNormal"),                //
    EDI_SYNTAX(LIGHTRED, "MouseSetLimits"),                //
    EDI_SYNTAX(LIGHTRED, "IpxSocketClose"),                //
    EDI_SYNTAX(LIGHTRED, "IpxCheckPacket"),                //
    EDI_SYNTAX(LIGHTRED, "GetSerialPorts"),                //
    EDI_SYNTAX(LIGHTRED, "GetNumberOfHDD"),                //
    EDI_SYNTAX(LIGHTRED, "GetNumberOfFDD"),                //
    EDI_SYNTAX(LIGHTRED, "GetNetworkMask"),                //
    EDI_SYNTAX(LIGHTRED, "GetEmptyMatrix"),                //
    EDI_SYNTAX(LIGHTRED, "GetAlignMatrix"),                //
    EDI_SYNTAX(LIGHTRED, "fxTexNCCTable"),                 //
    EDI_SYNTAX(LIGHTRED, "fxGetMemoryFb"),                 //
    EDI_SYNTAX(LIGHTRED, "fxBufferClear"),                 //
    EDI_SYNTAX(LIGHTRED, "StringToBytes"),                 //
    EDI_SYNTAX(LIGHTRED, "NamedFunction"),                 //
    EDI_SYNTAX(LIGHTRED, "NPerspProject"),                 //
    EDI_SYNTAX(LIGHTRED, "MouseSetSpeed"),                 //
    EDI_SYNTAX(LIGHTRED, "MidiIsPlaying"),                 //
    EDI_SYNTAX(LIGHTRED, "LPTRawControl"),                 //
    EDI_SYNTAX(LIGHTRED, "IpxSocketOpen"),                 //
    EDI_SYNTAX(LIGHTRED, "GetScreenMode"),                 //
    EDI_SYNTAX(LIGHTRED, "GetDomainname"),                 //
    EDI_SYNTAX(LIGHTRED, "GetDiskStatus"),                 //
    EDI_SYNTAX(LIGHTRED, "FxEmptyVertex"),                 //
    EDI_SYNTAX(LIGHTRED, "FilledPolygon"),                 //
    EDI_SYNTAX(LIGHTRED, "FilledEllipse"),                 //
    EDI_SYNTAX(LIGHTRED, "CustomEllipse"),                 //
    EDI_SYNTAX(LIGHTRED, "BytesToString"),                 //
    EDI_SYNTAX(LIGHTRED, "fxTexCombine"),                  //
    EDI_SYNTAX(LIGHTRED, "fxDitherMode"),                  //
    EDI_SYNTAX(LIGHTRED, "fxDepthRange"),                  //
    EDI_SYNTAX(LIGHTRED, "fxClipWindow"),                  //
    EDI_SYNTAX(LIGHTRED, "fxBufferSwap"),                  //
    EDI_SYNTAX(LIGHTRED, "VectorLength"),                  //
    EDI_SYNTAX(LIGHTRED, "SetFramerate"),                  //
    EDI_SYNTAX(LIGHTRED, "SaveTgaImage"),                  //
    EDI_SYNTAX(LIGHTRED, "SavePngImage"),                  //
    EDI_SYNTAX(LIGHTRED, "SavePcxImage"),                  //
    EDI_SYNTAX(LIGHTRED, "SaveBmpImage"),                  //
    EDI_SYNTAX(LIGHTRED, "QScaleMatrix"),                  //
    EDI_SYNTAX(LIGHTRED, "PerspProject"),                  //
    EDI_SYNTAX(LIGHTRED, "NApplyMatrix"),                  //
    EDI_SYNTAX(LIGHTRED, "LPTRawStatus"),                  //
    EDI_SYNTAX(LIGHTRED, "JoystickPoll"),                  //
    EDI_SYNTAX(LIGHTRED, "IpxGetPacket"),                  //
    EDI_SYNTAX(LIGHTRED, "IpxFindNodes"),                  //
    EDI_SYNTAX(LIGHTRED, "GetRandomInt"),                  //
    EDI_SYNTAX(LIGHTRED, "GetFramerate"),                  //
    EDI_SYNTAX(LIGHTRED, "FxTexMemInit"),                  //
    EDI_SYNTAX(LIGHTRED, "FxRGB2Vertex"),                  //
    EDI_SYNTAX(LIGHTRED, "FilledCircle"),                  //
    EDI_SYNTAX(LIGHTRED, "DestroyScene"),                  //
    EDI_SYNTAX(LIGHTRED, "CustomCircle"),                  //
    EDI_SYNTAX(LIGHTRED, "CrossProduct"),                  //
    EDI_SYNTAX(LIGHTRED, "fxGetNumTmu"),                   //
    EDI_SYNTAX(LIGHTRED, "fxDrawPoint"),                   //
    EDI_SYNTAX(LIGHTRED, "fxDepthMask"),                   //
    EDI_SYNTAX(LIGHTRED, "fxColorMask"),                   //
    EDI_SYNTAX(LIGHTRED, "StartupInfo"),                   //
    EDI_SYNTAX(LIGHTRED, "SetSceneGap"),                   //
    EDI_SYNTAX(LIGHTRED, "RequireFile"),                   //
    EDI_SYNTAX(LIGHTRED, "RenderScene"),                   //
    EDI_SYNTAX(LIGHTRED, "OutPortWord"),                   //
    EDI_SYNTAX(LIGHTRED, "OutPortLong"),                   //
    EDI_SYNTAX(LIGHTRED, "OutPortByte"),                   //
    EDI_SYNTAX(LIGHTRED, "MidiGetTime"),                   //
    EDI_SYNTAX(LIGHTRED, "LoadLibrary"),                   //
    EDI_SYNTAX(LIGHTRED, "IpxAllNodes"),                   //
    EDI_SYNTAX(LIGHTRED, "GetHostname"),                   //
    EDI_SYNTAX(LIGHTRED, "CreateScene"),                   //
    EDI_SYNTAX(LIGHTRED, "ClearScreen"),                   //
    EDI_SYNTAX(LIGHTRED, "ApplyMatrix"),                   //
    EDI_SYNTAX(LIGHTRED, "fxViewport"),                    //
    EDI_SYNTAX(LIGHTRED, "fxShutdown"),                    //
    EDI_SYNTAX(LIGHTRED, "fxGetNumFb"),                    //
    EDI_SYNTAX(LIGHTRED, "fxFogTable"),                    //
    EDI_SYNTAX(LIGHTRED, "fxDrawLine"),                    //
    EDI_SYNTAX(LIGHTRED, "fxCullMode"),                    //
    EDI_SYNTAX(LIGHTRED, "Triangle3D"),                    //
    EDI_SYNTAX(LIGHTRED, "SetExitKey"),                    //
    EDI_SYNTAX(LIGHTRED, "NMatrixMul"),                    //
    EDI_SYNTAX(LIGHTRED, "MidiResume"),                    //
    EDI_SYNTAX(LIGHTRED, "MidiGetPos"),                    //
    EDI_SYNTAX(LIGHTRED, "MemoryInfo"),                    //
    EDI_SYNTAX(LIGHTRED, "MakeString"),                    //
    EDI_SYNTAX(LIGHTRED, "LoadModule"),                    //
    EDI_SYNTAX(LIGHTRED, "LPTRawData"),                    //
    EDI_SYNTAX(LIGHTRED, "InPortWord"),                    //
    EDI_SYNTAX(LIGHTRED, "InPortLong"),                    //
    EDI_SYNTAX(LIGHTRED, "InPortByte"),                    //
    EDI_SYNTAX(LIGHTRED, "HelloWorld"),                    //
    EDI_SYNTAX(LIGHTRED, "DotProduct"),                    //
    EDI_SYNTAX(LIGHTRED, "CustomLine"),                    //
    EDI_SYNTAX(LIGHTRED, "CompareKey"),                    //
    EDI_SYNTAX(LIGHTRED, "ClearScene"),                    //
    EDI_SYNTAX(LIGHTRED, "fxFogMode"),                     //
    EDI_SYNTAX(LIGHTRED, "fxDisable"),                     //
    EDI_SYNTAX(LIGHTRED, "ZipPrefix"),                     //
    EDI_SYNTAX(LIGHTRED, "StopWatch"),                     //
    EDI_SYNTAX(LIGHTRED, "ResolveIp"),                     //
    EDI_SYNTAX(LIGHTRED, "RandomInt"),                     //
    EDI_SYNTAX(LIGHTRED, "Polygon3D"),                     //
    EDI_SYNTAX(LIGHTRED, "MouseWarp"),                     //
    EDI_SYNTAX(LIGHTRED, "MidiPause"),                     //
    EDI_SYNTAX(LIGHTRED, "MatrixMul"),                     //
    EDI_SYNTAX(LIGHTRED, "LPTStatus"),                     //
    EDI_SYNTAX(LIGHTRED, "FloodFill"),                     //
    EDI_SYNTAX(LIGHTRED, "FilledBox"),                     //
    EDI_SYNTAX(LIGHTRED, "DrawArray"),                     //
    EDI_SYNTAX(LIGHTRED, "CircleArc"),                     //
    EDI_SYNTAX(LIGHTRED, "fxSplash"),                      //
    EDI_SYNTAX(LIGHTRED, "fxOrigin"),                      //
    EDI_SYNTAX(LIGHTRED, "fxIsBusy"),                      //
    EDI_SYNTAX(LIGHTRED, "fxFinish"),                      //
    EDI_SYNTAX(LIGHTRED, "fxEnable"),                      //
    EDI_SYNTAX(LIGHTRED, "RawWrite"),                      //
    EDI_SYNTAX(LIGHTRED, "MsecTime"),                      //
    EDI_SYNTAX(LIGHTRED, "MidiStop"),                      //
    EDI_SYNTAX(LIGHTRED, "LPTReset"),                      //
    EDI_SYNTAX(LIGHTRED, "IpxDebug"),                      //
    EDI_SYNTAX(LIGHTRED, "HasCpuId"),                      //
    EDI_SYNTAX(LIGHTRED, "HSBColor"),                      //
    EDI_SYNTAX(LIGHTRED, "GetPixel"),                      //
    EDI_SYNTAX(LIGHTRED, "GetGreen"),                      //
    EDI_SYNTAX(LIGHTRED, "GetAlpha"),                      //
    EDI_SYNTAX(LIGHTRED, "CharCode"),                      //
    EDI_SYNTAX(LIGHTRED, "fxFlush"),                       //
    EDI_SYNTAX(LIGHTRED, "Resolve"),                       //
    EDI_SYNTAX(LIGHTRED, "Require"),                       //
    EDI_SYNTAX(LIGHTRED, "ReadZIP"),                       //
    EDI_SYNTAX(LIGHTRED, "RawRead"),                       //
    EDI_SYNTAX(LIGHTRED, "Println"),                       //
    EDI_SYNTAX(LIGHTRED, "NumCpus"),                       //
    EDI_SYNTAX(LIGHTRED, "MidiOut"),                       //
    EDI_SYNTAX(LIGHTRED, "MakeDir"),                       //
    EDI_SYNTAX(LIGHTRED, "LPTSend"),                       //
    EDI_SYNTAX(LIGHTRED, "IpxSend"),                       //
    EDI_SYNTAX(LIGHTRED, "IpDebug"),                       //
    EDI_SYNTAX(LIGHTRED, "Include"),                       //
    EDI_SYNTAX(LIGHTRED, "GetBlue"),                       //
    EDI_SYNTAX(LIGHTRED, "Ellipse"),                       //
    EDI_SYNTAX(LIGHTRED, "fxInit"),                        //
    EDI_SYNTAX(LIGHTRED, "VDebug"),                        //
    EDI_SYNTAX(LIGHTRED, "TextXY"),                        //
    EDI_SYNTAX(LIGHTRED, "System"),                        //
    EDI_SYNTAX(LIGHTRED, "SNoise"),                        //
    EDI_SYNTAX(LIGHTRED, "RmFile"),                        //
    EDI_SYNTAX(LIGHTRED, "Rename"),                        //
    EDI_SYNTAX(LIGHTRED, "Quad3D"),                        //
    EDI_SYNTAX(LIGHTRED, "PNoise"),                        //
    EDI_SYNTAX(LIGHTRED, "GetRed"),                        //
    EDI_SYNTAX(LIGHTRED, "Clip3D"),                        //
    EDI_SYNTAX(LIGHTRED, "Circle"),                        //
    EDI_SYNTAX(LIGHTRED, "Sleep"),                         //
    EDI_SYNTAX(LIGHTRED, "SizeY"),                         //
    EDI_SYNTAX(LIGHTRED, "SizeX"),                         //
    EDI_SYNTAX(LIGHTRED, "RmDir"),                         //
    EDI_SYNTAX(LIGHTRED, "Print"),                         //
    EDI_SYNTAX(LIGHTRED, "Noise"),                         //
    EDI_SYNTAX(LIGHTRED, "FXBIT"),                         //
    EDI_SYNTAX(LIGHTRED, "Debug"),                         //
    EDI_SYNTAX(LIGHTRED, "CpuId"),                         //
    EDI_SYNTAX(LIGHTRED, "Color"),                         //
    EDI_SYNTAX(LIGHTRED, "Stop"),                          //
    EDI_SYNTAX(LIGHTRED, "Stat"),                          //
    EDI_SYNTAX(LIGHTRED, "Read"),                          //
    EDI_SYNTAX(LIGHTRED, "Plot"),                          //
    EDI_SYNTAX(LIGHTRED, "POST"),                          //
    EDI_SYNTAX(LIGHTRED, "List"),                          //
    EDI_SYNTAX(LIGHTRED, "Line"),                          //
    EDI_SYNTAX(LIGHTRED, "Info"),                          //
    EDI_SYNTAX(LIGHTRED, "Dump"),                          //
    EDI_SYNTAX(LIGHTRED, "Box"),                           //
    EDI_SYNTAX(LIGHTRED, "Gc"),                            //

    // Classes
    EDI_SYNTAX(LIGHTGREEN, "TexInfo"),  // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "COMPort"),  // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "ZBuffer"),  // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "FxState"),  // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "Sample"),   // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "Socket"),   // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "Bitmap"),   // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "Font"),     // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "File"),     // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "Midi"),     // .ctor()
    EDI_SYNTAX(LIGHTGREEN, "Zip"),      // .ctor()

    // Methods
    EDI_SYNTAX(RED, "SetCertificatePassword"),  //
    EDI_SYNTAX(RED, "SetUnrestrictedAuth"),     //
    EDI_SYNTAX(RED, "SetFollowLocation"),       //
    EDI_SYNTAX(RED, "SetConnectTimeout"),       //
    EDI_SYNTAX(RED, "DrawStringCenter"),        //
    EDI_SYNTAX(RED, "GetResponseCode"),         //
    EDI_SYNTAX(RED, "DrawStringRight"),         //
    EDI_SYNTAX(RED, "SetKeyPassword"),          //
    EDI_SYNTAX(RED, "SetCertificate"),          //
    EDI_SYNTAX(RED, "DrawStringLeft"),          //
    EDI_SYNTAX(RED, "DownloadMipMap"),          //
    EDI_SYNTAX(RED, "SetSocksProxy"),           //
    EDI_SYNTAX(RED, "IsOutputEmpty"),           //
    EDI_SYNTAX(RED, "GetRemotePort"),           //
    EDI_SYNTAX(RED, "GetRemoteHost"),           //
    EDI_SYNTAX(RED, "StringHeight"),            //
    EDI_SYNTAX(RED, "SetUserAgent"),            //
    EDI_SYNTAX(RED, "SetSslVerify"),            //
    EDI_SYNTAX(RED, "SetProxyUser"),            //
    EDI_SYNTAX(RED, "SetProxyPort"),            //
    EDI_SYNTAX(RED, "SetMaxRedirs"),            //
    EDI_SYNTAX(RED, "SaveTgaImage"),            //
    EDI_SYNTAX(RED, "SavePngImage"),            //
    EDI_SYNTAX(RED, "SavePcxImage"),            //
    EDI_SYNTAX(RED, "SaveBmpImage"),            //
    EDI_SYNTAX(RED, "IsOutputFull"),            //
    EDI_SYNTAX(RED, "IsInputEmpty"),            //
    EDI_SYNTAX(RED, "GetLocalPort"),            //
    EDI_SYNTAX(RED, "DrawAdvanced"),            //
    EDI_SYNTAX(RED, "ClearHeaders"),            //
    EDI_SYNTAX(RED, "WriteString"),             //
    EDI_SYNTAX(RED, "StringWidth"),             //
    EDI_SYNTAX(RED, "MemRequired"),             //
    EDI_SYNTAX(RED, "IsInputFull"),             //
    EDI_SYNTAX(RED, "FlushOutput"),             //
    EDI_SYNTAX(RED, "ExtractFile"),             //
    EDI_SYNTAX(RED, "Established"),             //
    EDI_SYNTAX(RED, "WriteBytes"),              //
    EDI_SYNTAX(RED, "SetTimeout"),              //
    EDI_SYNTAX(RED, "SetReferer"),              //
    EDI_SYNTAX(RED, "SetCookies"),              //
    EDI_SYNTAX(RED, "ReadString"),              //
    EDI_SYNTAX(RED, "ReadBuffer"),              //
    EDI_SYNTAX(RED, "NumEntries"),              //
    EDI_SYNTAX(RED, "MarkUnused"),              //
    EDI_SYNTAX(RED, "GetLastUrl"),              //
    EDI_SYNTAX(RED, "GetEntries"),              //
    EDI_SYNTAX(RED, "GetComment"),              //
    EDI_SYNTAX(RED, "GetAllData"),              //
    EDI_SYNTAX(RED, "FlushInput"),              //
    EDI_SYNTAX(RED, "WriteLine"),               //
    EDI_SYNTAX(RED, "WriteInts"),               //
    EDI_SYNTAX(RED, "WriteByte"),               //
    EDI_SYNTAX(RED, "WaitInput"),               //
    EDI_SYNTAX(RED, "WaitFlush"),               //
    EDI_SYNTAX(RED, "SkipFrame"),               //
    EDI_SYNTAX(RED, "SetUserPw"),               //
    EDI_SYNTAX(RED, "SetCaFile"),               //
    EDI_SYNTAX(RED, "ReadBytes"),               //
    EDI_SYNTAX(RED, "PlayFrame"),               //
    EDI_SYNTAX(RED, "FxDrawLfb"),               //
    EDI_SYNTAX(RED, "FlushNext"),               //
    EDI_SYNTAX(RED, "DrawTrans"),               //
    EDI_SYNTAX(RED, "DoRequest"),               //
    EDI_SYNTAX(RED, "DataReady"),               //
    EDI_SYNTAX(RED, "AddHeader"),               //
    EDI_SYNTAX(RED, "ToString"),                //
    EDI_SYNTAX(RED, "SetProxy"),                //
    EDI_SYNTAX(RED, "ReadLine"),                //
    EDI_SYNTAX(RED, "ReadInts"),                //
    EDI_SYNTAX(RED, "ReadByte"),                //
    EDI_SYNTAX(RED, "GetPixel"),                //
    EDI_SYNTAX(RED, "ToArray"),                 //
    EDI_SYNTAX(RED, "SetPost"),                 //
    EDI_SYNTAX(RED, "NoFlush"),                 //
    EDI_SYNTAX(RED, "GetSize"),                 //
    EDI_SYNTAX(RED, "AddFile"),                 //
    EDI_SYNTAX(RED, "Source"),                  //
    EDI_SYNTAX(RED, "SetPut"),                  //
    EDI_SYNTAX(RED, "SetKey"),                  //
    EDI_SYNTAX(RED, "SetGet"),                  //
    EDI_SYNTAX(RED, "Append"),                  //
    EDI_SYNTAX(RED, "Train"),                   //
    EDI_SYNTAX(RED, "Shift"),                   //
    EDI_SYNTAX(RED, "Flush"),                   //
    EDI_SYNTAX(RED, "Close"),                   //
    EDI_SYNTAX(RED, "Clear"),                   //
    EDI_SYNTAX(RED, "Stop"),                    //
    EDI_SYNTAX(RED, "Save"),                    //
    EDI_SYNTAX(RED, "Push"),                    //
    EDI_SYNTAX(RED, "Play"),                    //
    EDI_SYNTAX(RED, "Mode"),                    //
    EDI_SYNTAX(RED, "Exec"),                    //
    EDI_SYNTAX(RED, "Draw"),                    //
    EDI_SYNTAX(RED, "Set"),                     //
    EDI_SYNTAX(RED, "Run"),                     //
    EDI_SYNTAX(RED, "Pop"),                     //
    EDI_SYNTAX(RED, "Get"),                     //

    // members
    EDI_SYNTAX(YELLOW, ".hidden_layers"),  //
    EDI_SYNTAX(YELLOW, ".SavePngImage"),   //
    EDI_SYNTAX(YELLOW, ".textureSize"),    //
    EDI_SYNTAX(YELLOW, ".aspectRatio"),    //
    EDI_SYNTAX(YELLOW, ".frameCount"),     //
    EDI_SYNTAX(YELLOW, ".alloc_size"),     //
    EDI_SYNTAX(YELLOW, ".tableType"),      //
    EDI_SYNTAX(YELLOW, ".frequency"),      //
    EDI_SYNTAX(YELLOW, ".RenderSVG"),      //
    EDI_SYNTAX(YELLOW, ".smallLod"),       //
    EDI_SYNTAX(YELLOW, ".minDelay"),       //
    EDI_SYNTAX(YELLOW, ".maxDelay"),       //
    EDI_SYNTAX(YELLOW, ".largeLod"),       //
    EDI_SYNTAX(YELLOW, ".filename"),       //
    EDI_SYNTAX(YELLOW, ".duration"),       //
    EDI_SYNTAX(YELLOW, ".outputs"),        //
    EDI_SYNTAX(YELLOW, ".address"),        //
    EDI_SYNTAX(YELLOW, ".stereo"),         //
    EDI_SYNTAX(YELLOW, ".server"),         //
    EDI_SYNTAX(YELLOW, ".length"),         //
    EDI_SYNTAX(YELLOW, ".inputs"),         //
    EDI_SYNTAX(YELLOW, ".hidden"),         //
    EDI_SYNTAX(YELLOW, ".height"),         //
    EDI_SYNTAX(YELLOW, ".format"),         //
    EDI_SYNTAX(YELLOW, ".width"),          //
    EDI_SYNTAX(YELLOW, ".mode"),           //
    EDI_SYNTAX(YELLOW, ".bits"),           //
    EDI_SYNTAX(YELLOW, ".udp"),            //
    EDI_SYNTAX(YELLOW, ".tmu"),            //

    EDI_SYNTAX_END  // end of list
};
