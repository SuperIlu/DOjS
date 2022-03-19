@echo off
cd VisualStudio
set REBUILD=-target:Rebuild
set MSBUILD_ARGS=-consoleloggerparameters:DisableConsoleColor
set MSBUILD_ARGS=-consoleloggerparameters:DisableConsoleColor

set COMMAND=msbuild Watt-32.sln %MSBUILD_ARGS% /property:Configuration=Debug
echo ---- Running %COMMAND% ---- > ..\make.log
%COMMAND% | tee /a ..\make.log

set COMMAND=msbuild Watt-32.sln %MSBUILD_ARGS% /property:Configuration=Release
echo ---- Running %COMMAND% ---- >> ..\make.log
%COMMAND% | tee /a ..\make.log

cd ..