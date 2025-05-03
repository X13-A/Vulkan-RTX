@echo off
setlocal

set SLANGC=slangc.exe

%SLANGC% -profile vs_5_0 -target spirv -entry main -o geometry_vert.spv geometry_vert.slang
%SLANGC% -profile ps_5_0 -target spirv -entry main -o geometry_frag.spv geometry_frag.slang

%SLANGC% -profile vs_5_0 -target spirv -entry main -o lighting_vert.spv lighting_vert.slang
%SLANGC% -profile ps_5_0 -target spirv -entry main -o lighting_frag.spv lighting_frag.slang

echo Compilation complete.
pause
