@echo off
setlocal

set application_name=handmade_renderer
set build_options=
set lib_dir=..\lib
set inc_dir=..\include

set includes=/I..\src /I%inc_dir% /I%inc_dir%\glew\include /I%inc_dir%\glfw\include
set libs=/LIBPATH:%lib_dir%\glew\ /LibPath:%lib_dir%\glfw\

set compiler_flags=/nologo /Zi /FAsc /EHsc /wd4312
set linker_flags=/opt:ref /incremental:no
set platform_linker_flags=%linker_flags% glfw3dll.lib glew32.lib opengl32.lib user32.lib gdi32.lib shell32.lib kernel32.lib

set glfw_dll=%lib_dir%\glfw\
set glew_dll=%lib_dir%\glew\

if not exist .\build mkdir .\build
pushd .\build

copy %glfw_dll%\glfw3.dll .\
copy %glew_dll%\glew32.dll .\

rem Loop through all arguments
for %%f in (%*) do (
	cl %compiler_flags% %includes% ..\apps\%%f.c -LD -MT -D_USRDLL /link %linker_flags% -EXPORT:init_app -EXPORT:update_and_render /out:%%f.dll
)

cl %compiler_flags% %includes% ..\src\glfw\glfw_main.c /link %libs% %platform_linker_flags% /out:%application_name%.exe

popd