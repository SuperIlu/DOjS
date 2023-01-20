@echo off
setlocal EnableDelayedExpansion
prompt $P$G

::
:: Download-links for djgpp + OpenWatcom files used by 'build_x' on AppVeyor or
:: for local testing if this .bat-file.
::
:: These can be rather slow:
::
set URL_DJ_WIN_ZIP=http://www.watt-32.net/CI/dj-win.zip
set URL_WATCOM_ZIP=http://www.watt-32.net/CI/watcom20.zip
set URL_BORLAND_ZIP=http://www.watt-32.net/CI/borland.zip
set URL_WINPCAP_EXE=https://www.winpcap.org/install/bin/WinPcap_4_1_3.exe
set URL_NPCAP_EXE=https://github.com/UAVCAN/pyuavcan/blob/master/.test_deps/npcap-0.96.exe
set URL_LLVM_EXE=https://prereleases.llvm.org/win-snapshots/LLVM-10.0.0-e20a1e486e1-win32.exe

::
:: D/L from MS's OneDrive instead with the below cryptic 'URL_x' strings.
::
:: I got these with the help of "CurlWget". An great Chrome extension:
::   https://chrome.google.com/webstore/detail/curlwget/jmocjfidanebdlinpbcdkcmgdifblncg
::
:: set URL_DJ_WIN_ZIP="https://public.am.files.1drv.com/y4mj1hIt_E5LjPPG9V7PywEz9lsuoYtMOEP1-jEcUkZ06M9ZK3kN7SXPw2AOgn9sf2_RbqhRQgvhMOgaJxyIL_O2GTjzp-u9LUB1RtLylxtr-URpATMJDPcPeUmd3gRhCrsXLWMxO8Hh5q7oR1_9V6gMZKB_VWTU6bPZkZ0rc8Tg9YjpzuzX180lWpc2E8w52hpkV0ApaxuVstkq_M6DwOEbssrymQaX30jYgpUw4YCmlA?access_token=EwAIA61DBAAUzl/nWKUlBg14ZGcybuC4/OHFdfEAAcjk8R21S0WRbnPh7yDX/bpeIL1WFpo1loXmt9HWcrm6dtt2uc8AFFghySuz840rgZYMCXSzwFO1lDjQCyFe1ub5GUO%2bvJnqYAbjOOExBSENan77aZ/Y%2bDqxbAU%2bIiSPebGpjrN5KMlszGXV1pJyOLQVRMk3JpQ8MeM5Cfm6bNLCzNCi2IKOrUN0S5MnLypsP0Y55Q728New95Tp6pId3201zbOoh%2byopzvoFWJeGZ1xRV0uqwzHHKw5PnuZrByFPzLI8/XiQ3IUOC/q/oU/L8BAZk2Y8GCg1S41O5Y7%2bQuQXabLxMN8d3QiJV2Ky/SodxpdipT0OfPBMTHDOadiagkDZgAACOiGp0v9%2bTQS2AFKY3E7XSCFUpNnkjnzUXof10is4AuzeF4Dbk0ruj3eM5fQA0JRzTXpbAp6Aojo/1N1SkNpsNNQzK/VClKSRwr7kT98SLgLFtCDxlEioac66eceN3KU6/CHVEWUPHOuiHSjg6OHHwN4/WWsTEDb3%2bRvg6cch2519pkwMveTzPzW5H3gZ5odyKtMhRRJH27B1gzt3jid4yHENEr66uFsx1m6SCsCDEXuZY6KI/M%2bi7xKni/alNjHYJqpi7rmM2fjQsuW5iSaHv39sEeoxPaWINHfAS189iXL67K7V%2br2avekXVz1C0dNoYWu46g27GWjLAMWHJr/K2r2DvTfWebw6F9Ul6JXGrT0atqKlPY4keV6tbltm2g5rlyNvErTaeCM/ZVmaOEZ7sOm8Q1RxnkwE8dkmFP5DVyTgJKMplNyo7U2W1ElwabCaG2NuMxp59avjuHDvewol8WqjLfL4R/cnJR5d3TomX1Llo2YES81cMJPwmj38RnS8jUE6uZ%2bXkziGxiKij%2b6cFBkNUtW9Nt13uSoiHntF5iF0tZ7xFAvJWwlHXSFVxwCNVyrIPDXExasidYdjdNO4Vdmxe4FUuElI7mEZ4BAWZidu2xY6AaAr0fgBS18gukvgzEyAwI%3d"
:: set URL_WATCOM_ZIP="https://public.am.files.1drv.com/y4mBFFcbx7AwhTYCTXPW5j2kRm1_SHzk3g1SJB6uvHub9Y7xIcRVWgPWB1jN5Pbjw71_XgJX_5OVcWY6BTfq6II4PvbQl-5OPe_2sBoTWBK-wlhrucAIjHkBqToE57tuIVDeykEu8xaRBiD9il-XoxbppPrRb6HNXrh2VQU7fdWUx7LqliQU0zLTqIRHWbFHoPNtpfZTba6Yey0ck1RWJaC5oz5kdx1LeVPAOt_NXH11RE?access_token=EwAIA61DBAAUzl/nWKUlBg14ZGcybuC4/OHFdfEAAcjk8R21S0WRbnPh7yDX/bpeIL1WFpo1loXmt9HWcrm6dtt2uc8AFFghySuz840rgZYMCXSzwFO1lDjQCyFe1ub5GUO%2bvJnqYAbjOOExBSENan77aZ/Y%2bDqxbAU%2bIiSPebGpjrN5KMlszGXV1pJyOLQVRMk3JpQ8MeM5Cfm6bNLCzNCi2IKOrUN0S5MnLypsP0Y55Q728New95Tp6pId3201zbOoh%2byopzvoFWJeGZ1xRV0uqwzHHKw5PnuZrByFPzLI8/XiQ3IUOC/q/oU/L8BAZk2Y8GCg1S41O5Y7%2bQuQXabLxMN8d3QiJV2Ky/SodxpdipT0OfPBMTHDOadiagkDZgAACOiGp0v9%2bTQS2AFKY3E7XSCFUpNnkjnzUXof10is4AuzeF4Dbk0ruj3eM5fQA0JRzTXpbAp6Aojo/1N1SkNpsNNQzK/VClKSRwr7kT98SLgLFtCDxlEioac66eceN3KU6/CHVEWUPHOuiHSjg6OHHwN4/WWsTEDb3%2bRvg6cch2519pkwMveTzPzW5H3gZ5odyKtMhRRJH27B1gzt3jid4yHENEr66uFsx1m6SCsCDEXuZY6KI/M%2bi7xKni/alNjHYJqpi7rmM2fjQsuW5iSaHv39sEeoxPaWINHfAS189iXL67K7V%2br2avekXVz1C0dNoYWu46g27GWjLAMWHJr/K2r2DvTfWebw6F9Ul6JXGrT0atqKlPY4keV6tbltm2g5rlyNvErTaeCM/ZVmaOEZ7sOm8Q1RxnkwE8dkmFP5DVyTgJKMplNyo7U2W1ElwabCaG2NuMxp59avjuHDvewol8WqjLfL4R/cnJR5d3TomX1Llo2YES81cMJPwmj38RnS8jUE6uZ%2bXkziGxiKij%2b6cFBkNUtW9Nt13uSoiHntF5iF0tZ7xFAvJWwlHXSFVxwCNVyrIPDXExasidYdjdNO4Vdmxe4FUuElI7mEZ4BAWZidu2xY6AaAr0fgBS18gukvgzEyAwI%3d"
:: set URL_BORLAND_ZIP="https://public.am.files.1drv.com/y4mtgUL21pxqUmKNqXMFqKfyF_Qcfub_7F9y9CW6qzJMK3J3Ty4yibM8PFh8ge1WbLX4GUtWaD2hcCMtBg6IHLG2gOoKUGVyZJsHnuYW453AzpLd4mxz4o5vfHXgbCSdzdzTRGAoRKWLVYKmVdHxkhJvvOnDp6fiM58rAYLsXsSKtN4HeCTrFV0hT6VSXd-6dmdURydrmKnupdDfSVC0iHNg67T7h_d_zQGLe0eoqk4VpU?access_token=EwAIA61DBAAUzl/nWKUlBg14ZGcybuC4/OHFdfEAAZqRQuZdI1RRk31IIQoiWyy8U3d/2GjhJaFhemMlSz0Q/aCpKh%2bE44B0grjZfdU1KIJDAX9hKoIrahYEsENwWQ3OTk8YhQS6JTo7AX5uez6DBYwErJEHiPMNDQEi4LQmeDNW9u5zFHYaxvlDrIHxEzw8qKYOcNV1C/S9Mm76vyZgXqfRbVSKWLP/GAfYhD%2b/H5qLtwf3M80LvwbtCw170ePm3T7qH9UZ35xg81h%2b8H5LnRPqEnTqW146ohFDEQi%2bqajpzfFCSkutXzE7GnaKOwJz0eI0L2E0cHUkO8FhZCvmQ7EjIv7HLD%2bNn9QiFJkl6rxivjMb/wl548ou8zdnv5MDZgAACMd9kYD2MIrm2AF5f1fTR1I0feNZIh9w63GgzyAN92DRw3orkR3NFzpzOCU07wDU6FzFYbwZ9ANFPlgXGoZLo7xPxV%2bo2ENrciUbYqsDe3tNqfcXZwV2wJ7sRbqEyRPglSLz7cEIXxNV7vW/jkNK%2b2VUiXgpZnZm0hFtN6TCa3PwiNj92GoUJKSnxKwiucKyOGFrg0YcoOfvAv4r4Ncq4%2bIADOWBhRHUcURB1hFmjAIJGBet%2beXdNwW/kOpctMSyROf0XO2%2b3W2yKlxS1utzy1aeIb19BzaD4DbLRChFe6u3MpphQDJq/Bcj5oU6iR79wr5Vmu13HR8HUj0RSJ1xamF74t9l84LPJKBnD10PoZ5k2BV7fJiusS7U5%2bqs8F626wNkTq%2b6Bk499Zg0hhgKsedrEq%2bAUvUCMLTl4cTfEBjWO3YlN5Z%2bd%2bpkLB1BV7SpS9A1Y9Cq%2b6HJ3LgRNgRVvdSQS7J7Iclhzq2gTngPZzsR/XlMCa6jIQ3wYKl5z4poMjnWl%2b6nt6VV1LdDMgIlNRvIBpbOgY9OxwomTJCGX9K7kwwZWLljmcn9K%2blgz5Js/6/2AEjrZ2uk6nJIudfhRXf1pba%2b2UrYfi8r5r5VkW/iTPNHYu7tEmQHblEgzHaiyd1NAwI%3d"

::
:: 'APPVEYOR_PROJECT_NAME==Watt-32' unless testing this as:
::   c:\net\watt-32\> cmd /c appveyor-script.bat [build_src | build_bin | build_tests]
::
:: locally.
:: Change this for an 'echo.exe' with colour support. Like Cygwin.
::
set _ECHO=%MSYS2_ROOT%\usr\bin\echo.exe -e

if %APPVEYOR_PROJECT_NAME%. == . (
  set LOCAL_TEST=1

  if %BUILDER%. == . (
    echo BUILDER not set!
    exit /b 1
  )
  if %WATT_ROOT%. == . (
    echo WATT_ROOT not set!
    exit /b 1
  )
  set APPVEYOR_BUILD_FOLDER=%WATT_ROOT%
  set APPVEYOR_BUILD_FOLDER_UNIX=e:/net/watt

) else (
  set LOCAL_TEST=0
  set WATT_ROOT=c:\projects\watt-32
  set APPVEYOR_BUILD_FOLDER=c:\projects\watt-32
  set APPVEYOR_BUILD_FOLDER_UNIX=c:/projects/watt-32
  set _ECHO=c:\msys64\usr\bin\echo.exe -e
  set CYGWIN=nodosfilewarning
)

::
:: Download stuff to here:
::
set CI_ROOT=%APPVEYOR_BUILD_FOLDER%\CI-temp
md %CI_ROOT% 2> NUL

::
:: Since only 'watcom' has a '%MODEL%' set in 'appveyor.yml'
::
if %BUILDER%. == watcom. (
  %_ECHO% "\e[1;33mDoing '%1' for 'BUILDER=%BUILDER%', 'MODEL=%MODEL%'.\e[0m"

) else (
  %_ECHO% "\e[1;33mDoing '%1' for 'BUILDER=%BUILDER%'.\e[0m"
)

::
:: Stuff common to '[build_src | build_bin | build_tests]'
::
:: mingw64: Stuff for 'util/pkg-conf.mak'.
::
md lib\pkgconfig 2> NUL
set MINGW64=%APPVEYOR_BUILD_FOLDER_UNIX%

::
:: Set the dir for djgpp cross-environment.
:: Use forward slashes for this. Otherwise 'sh' + 'make' will get confused.
:: 7z can create only 1 level of missing directories. So a '%CI_ROOT%\djgpp' will not work
::
set DJGPP=%APPVEYOR_BUILD_FOLDER_UNIX%/CI-temp
set DJGPP_PREFIX=%DJGPP%/bin/i586-pc-msdosdjgpp

::
:: Set env-var for building with Watcom 2.0
::
set WATCOM=%CI_ROOT%
set NT_INCLUDE=%WATCOM%\h;%WATCOM%\h\nt
set DOS_INCLUDE=%WATCOM%\h

::
:: Set env-var for building with Borland/CBuilder (Win32 only)
::
set BCCDIR=%CI_ROOT%

if %BUILDER%. == borland. set CBUILDER_IS_LLVM_BASED=1
if %BUILDER%. == borland. set INCLUDE=%BCCDIR%\include\windows;%BCCDIR%\include\windows\sdk;%INCLUDE%

::
:: Shit for brains 'cmd' cannot have this inside a 'if x (' block since
:: on a AppVeyor build several "c:\Program Files (x86)\Microsoft xxx" strings
:: are in the 'PATH'.
::
:: This is the PATH to the 64-bit 'clang-cl' already on AppVeyor.
::
set PATH=%PATH%;c:\Program Files\LLVM\bin

::
:: And append the '%WATCOM%\binnt' to the 'PATH' since Watcom has an 'cl.exe'
:: which we do not want to use for 'BUILDER=visualc'.
::
set PATH=%PATH%;%WATCOM%\binnt

::
:: And append the '%BCCDIR%\bin' to the 'PATH' too.
::
set PATH=%PATH%;%BCCDIR%\bin

::
:: In case my curl was built with Wsock-Trace
::
set WSOCK_TRACE_LEVEL=0

::
:: Sanity check:
::
if %BUILDER%. == . (
  %_ECHO% "\e[1;31mBUILDER target not specified!\e[0m"
  exit /b 1
)

::
:: Assume 'CPU=x86'
::
set BITS=32
if %CPU%. == x64. set BITS=64

if %1. == build_src.    goto :build_src
if %1. == build_bin.    goto :build_bin
if %1. == build_tests.  goto :build_tests
if %1. == build_python. goto :build_python
if %1. == run_programs. goto :run_programs

echo Usage: %~dp0%0 ^[build_src ^| build_bin ^| build_tests ^| build_python ^]
exit /b 1

:build_src
cd src

::
:: Local 'cmd' test for '(' in env-vars:
:: This is what AppVeyor have first in their PATH:
::   c:\Program Files (x86)\Microsoft SDKs\Azure\CLI2\wbin
::
:: Hence cannot use a 'if x (what-ever) else (something else)' syntax with that.
::
if %LOCAL_TEST% == 1 (
  echo on
  if not exist "%APPVEYOR_BUILD_FOLDER%" (echo No '%APPVEYOR_BUILD_FOLDER%'. Edit this .bat-file & exit /b 1)
)

::
:: Generate a 'src/oui-generated.c' file from 'src/oui.txt (do not download it every time).
:: This is needed for many Win32 targets since it's mentioned in the 'build/*/watt32.dep' file.
:: Otherwise some Make programs (Cbuilder make) exits.
::
set CL=
%_ECHO% "\e[1;33mGenerating 'src/oui-generated.c'.\e[0m"
python.exe make-oui.py > oui-generated.c
if errorlevel 0 set CL=-DHAVE_OUI_GENERATED_C
%_ECHO% "\e[1;33m--------------------------------------------------------------------------------------------------\e[0m"

if %BUILDER%. == visualc. (
  call configur.bat visualc
  %_ECHO% "\e[1;33m[%CPU%]: Building release:\e[0m"
  nmake -nologo -f visualc-release_%BITS%.mak
  exit /b
)

%_ECHO% "\e[1;33m[%CPU%]: call configur.bat %BUILDER%:\e[0m"

::
:: Need to do 'call :install_LLVM' here to set the PATH for 'clang-cl.exe'!
::
if %BUILDER%. == clang. (
  call :install_LLVM
  call configur.bat clang
  %_ECHO% "\e[1;33m[%CPU%]: Building release:\e[0m"
  make -f clang-release_%BITS%.mak
  exit /b
)

if %BUILDER%. == mingw64. (
  call configur.bat mingw64
  %_ECHO% "\e[1;33m[%CPU%]: Building:\e[0m"
  make -f MinGW64_%BITS%.mak
  exit /b
)

if %BUILDER%. == cygwin. (
  call configur.bat cygwin
  %_ECHO% "\e[1;33m[%CPU%]: Building:\e[0m"
  make -f cygwin_%BITS%.mak
  exit /b
)

if %BUILDER%. == djgpp. (
  call :install_djgpp
  call configur.bat djgpp
  %_ECHO% "\e[1;33m[%CPU%]: Building:\e[0m"
  make -f djgpp.mak
  exit /b
)

if %BUILDER%. == borland. (
  call :install_borland
  call configur.bat borland
  %_ECHO% "\e[1;33m[%CPU%]: Building:\e[0m"
  %BCCDIR%\bin\make -f bcc_w.mak
  exit /b
)

if %BUILDER%. == watcom. (
  call :install_watcom
  call configur.bat watcom

  if %MODEL%. == win32. (
    %_ECHO% "\e[1;33m[%CPU%]: Building for Watcom/Win32:\e[0m"
    wmake -h -f watcom_w.mak

  ) else if %MODEL%. == flat. (
    %_ECHO% "\e[1;33m[%CPU%]: Building for Watcom/flat:\e[0m"
    wmake -h -f watcom_f.mak

  ) else if %MODEL%. == large. (
    %_ECHO% "\e[1;33m[%CPU%]: Building for Watcom/large:\e[0m"
    wmake -h -f watcom_l.mak

  ) else (
    %_ECHO% "\e[1;31m[%CPU%]: BUILDER 'watcom' needs a 'MODEL'!\e[0m"
     exit /b 1
  )
  exit /b
)

%_ECHO% "\e[1;31mIllegal BUILDER / CPU (BUILDER=%BUILDER%, CPU=%CPU%) values! Remember cmd.exe is case-sensitive.\e[0m"
exit /b 1

::
:: './bin/' programs to build for djgpp, Visual-C, MinGW-w64, Cygwin, clang-cl and Borland:
::
:build_bin

set PROGS_DJ=bping.exe ping.exe finger.exe ident.exe htget.exe tcpinfo.exe tracert.exe country.exe
set PROGS_VC=ping.exe finger.exe tcpinfo.exe host.exe htget.exe tracert.exe con-test.exe gui-test.exe lpq.exe lpr.exe ntime.exe whois.exe ident.exe country.exe
set PROGS_CYG=ping.exe finger.exe tcpinfo.exe host.exe htget.exe tracert.exe lpq.exe lpr.exe ntime.exe whois.exe ident.exe country.exe
set PROGS_MW=%PROGS_VC%
set PROGS_CL=%PROGS_VC%
set PROGS_BC=%PROGS_VC%

::
:: './bin/' programs to build for Watcom (Win32 + large + flat):
::
set PROGS_WC_WIN=ping.exe htget.exe finger.exe tcpinfo.exe con-test.exe gui-test.exe htget.exe tracert.exe whois.exe
set PROGS_WC_LARGE=ping.exe htget.exe finger.exe tcpinfo.exe htget.exe whois.exe
set PROGS_WC_FLAT=%PROGS_WC_LARGE%
set PROGS_WC_SMALL32=%PROGS_WC_LARGE%

cd bin

if %BUILDER%. == djgpp. (
  %_ECHO% "\e[1;33m[%CPU%]: Building PROGS_DJ=%PROGS_DJ%:\e[0m"
  make -f djgpp_win.mak DPMI_STUB=0 %PROGS_DJ%
  exit /b
)

if %BUILDER%. == visualc. (
  %_ECHO% "\e[1;33m[%CPU%]: Building PROGS_VC=%PROGS_VC%:\e[0m"
  nmake -nologo -f visualc.mak %PROGS_VC%
  exit /b
)

if %BUILDER%. == mingw64. (
  %_ECHO% "\e[1;33m[%CPU%]: Building PROGS_MW=%PROGS_MW%:\e[0m"
  make -f mingw64.mak %PROGS_MW%
  exit /b
)

if %BUILDER%. == cygwin. (
  %_ECHO% "\e[1;33m[%CPU%]: Building PROGS_CYG=%PROGS_CYG%:\e[0m"
  make -f cygwin.mak %PROGS_CYG%
  exit /b
)

::
:: Need to do 'call :install_LLVM' here to set the PATH for 'clang-cl.exe' again.
::
if %BUILDER%. == clang. (
  call :install_LLVM
  %_ECHO% "\e[1;33m[%CPU%]: Building PROGS_CL=%PROGS_CL%:\e[0m"
  make -f clang.mak check_CPU %PROGS_CL%
  exit /b
)

if %BUILDER%. == borland. (
  %_ECHO% "\e[1;33m[%CPU%]: Building PROGS_BC=%PROGS_BC%:\e[0m"
  %BCCDIR%\bin\make -f bcc_win.mak %PROGS_BC%
  exit /b
)

if %BUILDER%. == watcom. (
  if %MODEL%. == win32. (
    %_ECHO% "\e[1;33mwatcom/Win32: Building PROGS_WC_WIN=%PROGS_WC_WIN%:\e[0m"
    rm -f %PROGS_WC_WIN%
    wmake -h -f wc_win.mak %PROGS_WC_WIN%

  ) else if %MODEL%. == flat. (
    %_ECHO% "\e[1;33mwatcom/flat: Building PROGS_WC_FLAT=%PROGS_WC_FLAT%:\e[0m"
    rm -f %PROGS_WC_LARGE%
    wmake -h -f causeway.mak %PROGS_WC_FLAT%

  ) else if %MODEL%. == large. (
    %_ECHO% "\e[1;33mwatcom/large: Building PROGS_WC_LARGE=%PROGS_WC_LARGE%:\e[0m"
    rm -f %PROGS_WC_LARGE%
    wmake -h -f watcom.mak %PROGS_WC_LARGE%

  ) else (
    %_ECHO% "\e[1;31mThe 'watcom' needs a 'MODEL'!\e[0m"
     exit /b 1
  )
  exit /b
)

%_ECHO% "\e[1;31m[%CPU%]: No 'build_bin' for 'BUILDER=%BUILDER%' yet.\e[0m"
exit /b 0

::
:: Run the './bin/tcpinfo.exe' program if it exists.
:: But try to install NPcap first.
::
:run_programs
  call :install_npcap
  cd bin
  if exist tcpinfo.exe (
    set WATTCP_CFG=c:/projects/watt-32
    %_ECHO% "\e[1;33mRunning test 'tcpinfo.exe -d' ---------------------------------------------------------------\e[0m"
    tcpinfo.exe -d
  )
  exit /b

::
:: Build and run some test programs in './src/tests'.
:: (But djgpp, small, large and flat programs cannot run on AppVeyor).
::
:: Build all except for the 'watcom' MODELS 'small' and 'small32' (DOS).
:: All these generated makefiles requires GNU-make (a 'make' should already be on 'PATH').
::
:build_tests
  if %LOCAL_TEST%. == 0. (
    set WATTCP_CFG=c:/projects/watt-32
    %_ECHO% "\e[1;33mGenerating '%%WATTCP_CFG%%/wattcp.cfg'.\e[0m"
    call :generate_wattcp_cfg
  )

  cd src\tests

  set USE_WSOCK_TRACE=0

  if %CPU%. == x86. set PATH=c:\Program Files (x86)\LLVM\bin;%PATH%

  %_ECHO% "\e[1;33m[%CPU%]Configuring 'build_tests' for 'BUILDER=%BUILDER%'.\e[0m"

  call configur.bat %BUILDER%
  if %BUILDER%. == borland.  make -f bcc_w.mak
  if %BUILDER%. == djgpp.    make -f djgpp.mak
  if %BUILDER%. == clang.    make -f clang_%BITS%.mak
  if %BUILDER%. == mingw64.  make -f MinGW64_%BITS%.mak
  if %BUILDER%. == cygwin.   make -f Cygwin_%BITS%.mak
  if %BUILDER%. == visualc.  make -f visualc_%BITS%.mak
  if %BUILDER%. == watcom. (
     if %MODEL%. == large. make -f watcom_l.mak
     if %MODEL%. == flat.  make -f watcom_f.mak
     if %MODEL%. == win32. make -f watcom_w.mak
  )

  %_ECHO% "\e[1;33mRunning test 'cpu.exe' ---------------------------------------------------------------\e[0m"
  cpu.exe
  %_ECHO% "\e[1;33mRunning test 'cpuspeed.exe 1 1' ------------------------------------------------------\e[0m"
  cpuspeed.exe 1 1
  %_ECHO% "\e[1;33mRunning test 'swap.exe' --------------------------------------------------------------\e[0m"
  swap.exe
  %_ECHO% "\e[1;33mRunning test 'chksum.exe -s' ---------------------------------------------------------\e[0m"
  chksum.exe -s

:no_tests
  exit /b 0

::
:: Try to build the '_watt32.pyd' module for 32-bit
::
:build_python
  cd src\Python

  if %BUILDER%-%CPU%. == visualc-x86.  (
    %_ECHO% "\e[1;33m[%CPU%]: Building 'build_python' for 'BUILDER=visualc'.\e[0m"
    make PYTHON="py -3" CC=cl

  ) else if %BUILDER%-%CPU%. == clang-x86. (
    %_ECHO% "\e[1;33m[%CPU%]: Building 'build_python' for 'BUILDER=clang-cl'.\e[0m"
    make PYTHON="py -3" CC=clang-cl (

  ) else if %BUILDER%-%CPU%. == mingw64-x86. (
    %_ECHO% "\e[1;33m[%CPU%]: Building 'build_python' for 'BUILDER=MinGW64'.\e[0m"
    make PYTHON="py -3" CC=gcc

  ) else (
    %_ECHO% "\e[1;33m[%CPU%]Not doing 'build_python' for 'BUILDER=%BUILDER%'.\e[0m"
  )

  exit /b 0

::
:: Download the '%CI_ROOT%\llvm-installer.exe' for 32-bit 'clang-cl'.
:: A 200 MByte download which installs to "c:\Program Files (x86)\LLVM"
::
:: And it's PATH must be prepended to the normal PATH.
::
:install_LLVM
  if %CPU%. == x64. exit /b
  set PATH=c:\Program Files (x86)\LLVM\bin;%PATH%
  if exist "c:\Program Files (x86)\LLVM\bin\clang-cl.exe" exit /b
  if not exist %CI_ROOT%\llvm-installer.exe call :download_LLVM

  %_ECHO% "\e[1;33mInstalling 32-bit LLVM...'.\e[0m"
  start /wait %CI_ROOT%\llvm-installer.exe /S
  clang-cl -v
  %_ECHO% "\e[1;33mDone\n--------------------------------------------------------\e[0m"
  exit /b

:download_LLVM
  %_ECHO% "\e[1;33mDownloading 32-bit LLVM...'.\e[0m"
  curl -# -o %CI_ROOT%\llvm-installer.exe %URL_LLVM_EXE%
  if not errorlevel == 0 (
    %_ECHO% "\e[1;31mThe curl download failed!\e[0m"
    exit /b 1
  )
  exit /b

::
:: Download and install cross compiler for djgpp
::
:install_djgpp
  if exist %DJGPP%\bin\i586-pc-msdosdjgpp-gcc.exe exit /b
  call :download_djgpp
  7z x -y -o%DJGPP% %CI_ROOT%\dj-win.zip > NUL
  %_ECHO% "\e[1;33mDone\n--------------------------------------------------------\e[0m"
  exit /b

:download_djgpp
  %_ECHO% "\e[1;33mDownloading Andrew Wu's DJGPP cross compiler:\e[0m"
  curl -# -o %CI_ROOT%\dj-win.zip %URL_DJ_WIN_ZIP%
  if not errorlevel == 0 (
    %_ECHO% "\e[1;31mThe curl download failed!\e[0m"
    exit /b 1
  )
  exit /b

::
:: Download and install Borland/CBuilder
::
:install_borland
  if exist %BCCDIR%\bin\make.exe exit /b
  %_ECHO% "\e[1;33mDownloading Borland:\e[0m"
  curl -# -o %CI_ROOT%\borland.zip %URL_BORLAND_ZIP%
  if not errorlevel == 0 (
    %_ECHO% "\e[1;31mThe curl download failed!\e[0m"
    exit /b
  )
  7z x -y -o%BCCDIR% %CI_ROOT%\borland.zip > NUL
  exit /b

::
:: Download and install OpenWatcom
::
:install_watcom
  if exist %WATCOM%\binnt\wmake.exe exit /b
  %_ECHO% "\e[1;33mDownloading OpenWatcom 2.0:\e[0m"
  curl -# -o %CI_ROOT%\watcom20.zip %URL_WATCOM_ZIP%
  if not errorlevel == 0 (
    %_ECHO% "\e[1;31mThe curl download failed!\e[0m"
    exit /b
  )
  7z x -y -o%WATCOM% %CI_ROOT%\watcom20.zip > NUL
  exit /b


::
:: Download and install WinPcap
::
:install_winpcap
  if exist %CI_ROOT%\WinPcap\Uninstall.exe exit /b
  %_ECHO% "\e[1;33mDownloading WinPcap 4.1.3:\e[0m"
  curl -# -o %CI_ROOT%\WinPcap_4_1_3.exe %URL_WINPCAP_EXE%
  if not errorlevel == 0 (
    %_ECHO% "\e[1;31mThe curl download failed!\e[0m"
    exit /b
  )
  %CI_ROOT%\WinPcap_4_1_3.exe
  exit /b

::
:: Download and install the NPcap silent installer.
::
:install_npcap
  if exist "C:\Program Files\NPcap\Uninstall.exe" exit /b
  %_ECHO% "\e[1;33mDownloading NPcap 0.96:\e[0m"
  curl -# -o %CI_ROOT%\npcap-0.96.exe %URL_NPCAP_EXE%
  if not errorlevel == 0 (
    %_ECHO% "\e[1;31mThe curl download failed!\e[0m"
    exit /b
  )
  %CI_ROOT%\npcap-0.96.exe /loopback_support=yes /winpcap_mode=yes /S
  exit /b

::
:: Generate a 'c:\projects\watt-32\wattcp.cfg' for AppVeyor only.
::
:generate_wattcp_cfg
  echo # A generated %WATTCP_CFG%/wattcp.cfg for AppVeyor > %WATTCP_CFG%/wattcp.cfg
  echo debug           = 2                               >> %WATTCP_CFG%/wattcp.cfg
  echo nameserver      = 8.8.8.8                         >> %WATTCP_CFG%/wattcp.cfg
  echo winpkt.device   =                                 >> %WATTCP_CFG%/wattcp.cfg
  echo winpkt.dumpfile = $(WATT_ROOT)\winpkt_dump.txt    >> %WATTCP_CFG%/wattcp.cfg
  echo winpkt.trace    = 2                               >> %WATTCP_CFG%/wattcp.cfg
  echo winpkt.rxmode   = 0x20                            >> %WATTCP_CFG%/wattcp.cfg
  echo my_ip           = 10.0.0.2                        >> %WATTCP_CFG%/wattcp.cfg
  echo gateway         = 10.0.0.1                        >> %WATTCP_CFG%/wattcp.cfg
  echo netmask         = 255.255.255.0                   >> %WATTCP_CFG%/wattcp.cfg
  echo hosts           = $(WATT_ROOT)\bin\hosts          >> %WATTCP_CFG%/wattcp.cfg
  echo hosts6          = $(WATT_ROOT)\bin\hosts6         >> %WATTCP_CFG%/wattcp.cfg
  echo services        = $(WATT_ROOT)\bin\services       >> %WATTCP_CFG%/wattcp.cfg
  echo protocols       = $(WATT_ROOT)\bin\protocol       >> %WATTCP_CFG%/wattcp.cfg
  echo networks        = $(WATT_ROOT)\bin\networks       >> %WATTCP_CFG%/wattcp.cfg
  echo ethers          = $(WATT_ROOT)\bin\ethers         >> %WATTCP_CFG%/wattcp.cfg
  type "%WATTCP_CFG%\wattcp.cfg"
  exit /b
