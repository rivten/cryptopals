@echo off

rem --------------------------------------------------------------------------
rem                        COMPILATION
rem --------------------------------------------------------------------------

rem set SDLPath=C:\SDL2-2.0.7\
rem set SDLBinPath=%SDLPath%\lib\x64\
rem set SDLNetPath=C:\SDL2_net-2.0.1\
rem set SDLNetBinPath=%SDLNetPath%\lib\x64\

set UntreatedWarnings=/wd4100 /wd4244 /wd4201 /wd4127 /wd4505 /wd4456 /wd4996 /wd4003 /wd4706
set CommonCompilerDebugFlags=/MT /Od /Oi /fp:fast /fp:except- /Zo /Gm- /GR- /EHa /WX /W4 %UntreatedWarnings% /Z7 /nologo
set CommonLinkerDebugFlags=/incremental:no /opt:ref /subsystem:console

pushd ..\build\
cl %CommonCompilerDebugFlags% ..\code\crypto.cpp /link %CommonLinkerDebugFlags%
popd

rem --------------------------------------------------------------------------
echo Compilation completed...
