@echo off

set md=..\..
set lib_dir=%md%\lib
set inc_dir=%md%\include

set includes=-I%inc_dir% -I%md%\src
set libs=-L%lib_dir%
set main_compile_flags=-Wno-varargs -sMAIN_MODULE=2 -sFULL_ES3=1 -sUSE_WEBGL2=1 -sWASM=1 -sGL_ASSERTIONS=1 -sMIN_WEBGL_VERSION=2 -sEXPORTED_FUNCTIONS="_main,_realloc,_memset,_free,_sinf,_cosf,_atanf,_acosf,_memcpy,_strlen,_rand,_time,_getTempRet0,_srand,_stderr" -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=1 -sINITIAL_MEMORY=128MB -sSTACK_SIZE=32MB
set side_compile_flags=-Wno-varargs -sSIDE_MODULE=2 -sEXPORTED_FUNCTIONS="_init_app,_update_and_render"

call emsdk_env.bat

if not exist ..\build mkdir ..\build
if not exist ..\build\ems mkdir ..\build\ems
pushd ..\build\ems

emcc %md%\apps\triangle.c -o triangle.wasm %includes% %side_compile_flags% && ^
emcc %md%\src\emscripten\ems_main.c -o test.html %includes% %libs% %main_compile_flags% --preload-file %md%\data@data --preload-file .\triangle.wasm@triangle.wasm --shell-file %md%\template\shell_minimal.html

popd