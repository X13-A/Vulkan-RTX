@echo off
setlocal

set SLANGC=slangc.exe
%SLANGC% -profile vs_5_0 -target spirv -entry main -o vert.spv vert.slang
%SLANGC% -profile ps_5_0 -target spirv -entry main -o frag.spv frag.slang

echo Compilation complete.
pause
