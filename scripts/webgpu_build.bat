@echo off

set application_name=wgpu_test
set build_options=
set lib_dir=..\lib
set inc_dir=..\include

set includes=/I%lib_dir%\dawn\install\Release\include /I%inc_dir% /I..\src /I%inc_dir%\glfw\include\
set libs=/LIBPATH:%lib_dir%\dawn\install\Release\lib /LIBPATH:%lib_dir%\glfw

set compiler_flags=/nologo /Zi /FAsc /EHsc /wd4312
set linker_flags=/opt:ref /incremental:no webgpu_dawn.lib

if not exist ..\build mkdir ..\build
pushd ..\build

copy %lib_dir%\glfw\glfw3.dll .\
copy %lib_dir%\dawn\install\Release\bin\webgpu_dawn.dll .\

cl %compiler_flags% %includes% ..\src\webgpu\main.c /link  %libs% %linker_flags% /out:%application_name%.exe

popd
