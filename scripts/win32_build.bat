@echo off
setlocal

set application_name=handmade_renderer
set build_options=
set md=..\..
set lib_dir=%md%\lib
set inc_dir=%md%\include

set includes=/I%md%\src /I%inc_dir% /I%inc_dir%\glew\include /I%lib_dir%\glfw\install\include
set libs=/LIBPATH:%lib_dir%\glew\ /LibPath:%lib_dir%\glfw\install\lib

set compiler_flags=/nologo /Zi /FAsc /EHsc /wd4312
set linker_flags=/opt:ref /incremental:no
set platform_linker_flags=%linker_flags% glfw3dll.lib glew32.lib opengl32.lib user32.lib gdi32.lib shell32.lib kernel32.lib

set glfw_dll=%lib_dir%\glfw\install\bin
set glew_dll=%lib_dir%\glew\

if not exist ..\build mkdir ..\build
if not exist ..\build\win32 mkdir ..\build\win32

pushd ..\build\win32

copy %glfw_dll%\glfw3.dll .\
copy %glew_dll%\glew32.dll .\

rem Loop through all arguments
for %%f in (%*) do (
	cl %compiler_flags% %includes% %md%\apps\%%f.c -LD -MT -D_USRDLL /link %linker_flags% -EXPORT:init_app -EXPORT:update_and_render /out:%%f.dll
)

cl %compiler_flags% %includes% %md%\src\glfw\glfw_opengl.c -LD -MT -D_USRDLL /link %libs% %platform_linker_flags% -EXPORT:init_renderer -EXPORT:start_frame -EXPORT:end_frame /out:glfw_opengl.dll

cl %compiler_flags% %includes% %md%\src\glfw\glfw_main.c /link %libs% %platform_linker_flags% /out:%application_name%.exe

popd