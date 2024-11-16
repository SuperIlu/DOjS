echo on

set source_dir=%APPVEYOR_BUILD_FOLDER%

set build_dir_shared=c:\tmp\jasper\build\shared
set build_dir_static=c:\tmp\jasper\build\static
set install_dir_shared=c:\tmp\jasper\install\shared
set install_dir_static=c:\tmp\jasper\install\static
rem set allow_in_source_build=-DALLOW_IN_SOURCE_BUILD=true
set allow_in_source_build=

@echo ############################################################
@echo Static Library
@echo ############################################################

rem cmake -G "Visual Studio 12 2013 Win64" ^
cmake ^
  -H%source_dir% -B%build_dir_static% %allow_in_source_build% ^
  -DCMAKE_INSTALL_PREFIX=%install_dir_static% ^
  -DJAS_ENABLE_LIBJPEG=false ^
  -DJAS_ENABLE_OPENGL=false ^
  -DJAS_ENABLE_DOC=false ^
  -DJAS_ENABLE_SHARED=false || exit /B 1

rem list targets
rem pushd %build_dir_static% & msbuild /pp | find "<Target " & popd

rem msbuild %build_dir_static%\INSTALL.vcxproj || exit /B 1
cmake --build %build_dir_static% --clean-first || exit /B 1
cmake --build %build_dir_static% --target install || exit /B 1

dir %install_dir_static%\include\jasper || exit /B 1
dir %install_dir_static%\bin || exit /B 1
dir %install_dir_static%\lib || exit /B 1

@echo ############################################################
@echo Shared Library
@echo ############################################################

rem cmake -G "Visual Studio 12 2013 Win64" ^
cmake ^
  -H%source_dir% -B%build_dir_shared% %allow_in_source_build% ^
  -DCMAKE_INSTALL_PREFIX=%install_dir_shared% ^
  -DJAS_ENABLE_SHARED=true ^
  -DJAS_ENABLE_DOC=false || exit /B 1

rem msbuild %build_dir_shared%\INSTALL.vcxproj || exit /B 1
cmake --build %build_dir_shared% --clean-first || exit /B 1
cmake --build %build_dir_shared% --target install || exit /B 1

dir %install_dir_shared%\include\jasper || exit /B 1
dir %install_dir_shared%\bin || exit /B 1
dir %install_dir_shared%\lib || exit /B 1
