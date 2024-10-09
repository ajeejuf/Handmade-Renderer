@echo off

setlocal enabledelayedexpansion

set application_name=wgpu_test
set build_options=
set lib_dir=..\lib
set inc_dir=..\include

set includes=/I%lib_dir%\dawn\include /I%lib_dir%\dawn\out\Debug\gen\include
set libs=/LIBPATH:%lib_dir%\dawn\out\Debug

set compiler_flags=/nologo /Zi /FAsc /EHsc /wd4312
set linker_flags=/opt:ref /incremental:no dawn_proc.dll.lib dawn_native.dll.lib dawn_wire.dll.lib webgpu_dawn.dll.lib

if not exist ..\build mkdir ..\build
pushd ..\build

set dawn_dll_dir=%lib_dir%\dawn\out\Debug
for %%f in ("%dawn_dll_dir%\*.dll") do (
	copy "%%f" .\
)

for %%f in ("%dawn_dll_dir%\*.lib") do (
	set linker_flags=!linker_flags! "%%~nxf"
	echo "!linker_flags!"
)

cl %compiler_flags% %includes% ..\src\webgpu\main.c /link  %libs% !linker_flags! /out:%application_name%.exe

popd
