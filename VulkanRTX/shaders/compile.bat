@echo off
setlocal

::exit /b 0

set SLANGC=slangc.exe

echo Compiling shaders...

%SLANGC% -profile vs_5_0 -target spirv -entry main -o geometry_vert.spv geometry_vert.slang
%SLANGC% -profile ps_5_0 -target spirv -entry main -o geometry_frag.spv geometry_frag.slang

%SLANGC% -profile vs_5_0 -target spirv -entry main -o lighting_vert.spv lighting_vert.slang

%SLANGC% -profile ps_5_0 -target spirv -entry main -o lighting_frag.spv lighting_frag.slang

%SLANGC% -target spirv -stage raygeneration -entry main -o ray_gen.spv ray_gen.slang
%SLANGC% -target spirv -stage miss -entry main -o ray_miss.spv ray_miss.slang
%SLANGC% -target spirv -stage closesthit -entry main -o ray_closesthit.spv ray_closesthit.slang

echo Compilation complete.

pause

exit /b 0
