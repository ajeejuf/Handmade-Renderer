@echo off

set application_name=wgpu_test
set build_options=/D _GLFW_WIN32
set lib_dir=..\lib
set inc_dir=..\include

set includes=/I%lib_dir%\dawn\install\Release\include /I%inc_dir% /I..\src /I%lib_dir%\glfw\install\include\ /I%lib_dir%\glfw3webgpu
set libs=/LIBPATH:%lib_dir%\dawn\install\Release\lib /LIBPATH:%lib_dir%\glfw\install\lib

set compiler_flags=/nologo /Zi /FAsc /EHsc /wd4312
set linker_flags=/opt:ref /incremental:no webgpu_dawn.lib glfw3dll.lib

if not exist ..\build mkdir ..\build
pushd ..\build

copy %lib_dir%\glfw\install\bin\glfw3.dll .\
copy %lib_dir%\dawn\install\Release\bin\webgpu_dawn.dll .\

cl /c %build_options% %compiler_flags% %includes% ..\lib\glfw3webgpu\glfw3webgpu.c

cl /c %build_options% %compiler_flags% %includes% ..\src\webgpu\main.c

cl %build_options% %compiler_flags% %includes% main.obj glfw3webgpu.obj /link %libs% %linker_flags% /out:%application_name%.exe

popd
