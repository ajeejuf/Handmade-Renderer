@echo off

setlocal enabledelayedexpansion

set md=.
set lib_dir=.\lib
set inc_dir=.\include

set includes=-I%inc_dir% -I.\src
set libs=-L%lib_dir% 

set "pre_files=--preload-file .\data@data"

set main_compile_flags=-Wno-varargs -sMAIN_MODULE=2 -sUSE_WEBGPU=1 -sWASM=1 -sASYNCIFY=1 -sEXPORTED_FUNCTIONS="_main,_realloc,_memset,_free,_sinf,_cosf, _tanf,_emscripten_sleep,  _atanf,_acosf,_memcpy,_strlen,_rand,_time,_getTempRet0,_srand,_stderr,_fprintf, _wgpuCreateInstance,_wgpuInstanceCreateSurface,_wgpuInstanceRequestAdapter,_wgpuAdapterGetInfo, _wgpuAdapterGetLimits,_wgpuAdapterRequestDevice,_wgpuDeviceSetUncapturedErrorCallback, _wgpuDeviceGetLimits,_wgpuDeviceGetQueue,_wgpuSurfaceGetCapabilities,_wgpuSurfaceConfigure, _wgpuSurfaceCapabilitiesFreeMembers,_wgpuAdapterRelease,_wgpuDeviceCreateShaderModule, _wgpuDeviceCreateBindGroupLayout,_wgpuDeviceCreatePipelineLayout,_wgpuDeviceCreateRenderPipeline, _wgpuPipelineLayoutRelease,_wgpuDeviceCreateComputePipeline,_wgpuDeviceCreateBuffer, _wgpuDeviceCreateTexture,_wgpuQueueWriteTexture,_wgpuDeviceCreateSampler,_wgpuDeviceCreateBindGroup, _wgpuSurfaceGetCurrentTexture,_wgpuTextureViewRelease,_wgpuDeviceCreateCommandEncoder, _wgpuQueueWriteBuffer,_wgpuCommandEncoderBeginComputePass,_wgpuComputePassEncoderSetPipeline, _wgpuComputePassEncoderSetBindGroup,_wgpuComputePassEncoderDispatchWorkgroups,_wgpuComputePassEncoderEnd, _wgpuTextureCreateView,_wgpuComputePassEncoderRelease,_wgpuCommandEncoderBeginRenderPass, _wgpuRenderPassEncoderSetPipeline,_wgpuRenderPassEncoderSetVertexBuffer,_wgpuRenderPassEncoderSetIndexBuffer, _wgpuRenderPassEncoderSetBindGroup,_wgpuRenderPassEncoderDrawIndexed,_wgpuRenderPassEncoderEnd, _wgpuRenderPassEncoderRelease,_wgpuCommandEncoderFinish,_wgpuCommandEncoderRelease,_wgpuQueueSubmit, _wgpuCommandBufferRelease,_call_registered_function,_malloc,_calloc,_memcmp" -sEXPORTED_RUNTIME_METHODS="ccall,cwrap,setValue,getValue" -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=1 -sINITIAL_MEMORY=400MB -sSTACK_SIZE=32MB
set side_compile_flags=-Wno-varargs -sSIDE_MODULE=2 -sASYNCIFY=1 -sEXPORTED_FUNCTIONS="_get_callbacks,_load_assets,_init_app,_update_and_render"
set renderer_compile_flags=-Wno-varargs -sSIDE_MODULE=2 -sUSE_WEBGPU=1 -sWASM=1 -sASYNCIFY=1 -sEXPORTED_FUNCTIONS="_init_renderer,_start_frame,_end_frame"

set "template_dir=template"
set "app_dir=apps"
set "out_dir=build\ems"
set "wasm_dir=build\ems"

call emsdk_env.bat

:parse
if "%~1"=="" goto :endparse

if "%~1"=="--templates" (
	set "template_dir=%md%\%~2"

	echo This %~2

	echo Set template_dir: %template_dir%

	shift
	shift
	goto :parse
)

if "%~1"=="--apps" (
	set "app_dir=%md%\%~2"

	echo Set app_dir: %app_dir%

	shift
	shift
	goto :parse
)

if "%~1"=="--output" (
	set "out_dir=%~2"

	echo Set out_dir: %out_dir%

	shift
	shift
	goto :parse
)

if "%~1"=="--wasm" (
	set "wasm_dir=%~2"

	echo Set wasm_dir: %wasm_dir%

	shift
	shift
	goto :parse
)

shift
goto :parse

:endparse


REM Parsing output directory to creating necessary directories

set "temp_list=%out_dir:\= %"
set "temp_dir=..\"

for %%A in (%temp_list%) do (
	set "temp_dir=!temp_dir!%%A\"

	if "%%A" NEQ "." (
		if "%%A" NEQ ".." (
			if not exist "!temp_dir!" (
				echo Creating directory "!temp_dir!"
				mkdir "!temp_dir!"
			)
		)
	)
)

pushd ..\

for %%F in ("%app_dir%\*.c") do (
	echo Compiling %%F with emcc...

	call emcc %%F -o %wasm_dir%\%%~nF.wasm %includes% %side_compile_flags%

	if %ERRORLEVEL% NEQ 0 (
		echo Failed to compile %%F
		exit /b 1
	) else (
		echo Successfully compiled %%F
		set "pre_files=!pre_files! --preload-file .\%wasm_dir%\%%~nF.wasm@%%~nF.wasm"
	)
)

call emcc .\src\emscripten\ems_wgpu.c -o .\%wasm_dir%\ems_wgpu.wasm %includes%  %renderer_compile_flags%
set "pre_files=!pre_files! --preload-file .\%wasm_dir%\ems_wgpu.wasm@ems_wgpu.wasm"

echo !pre_files!


for %%F in ("%template_dir%\*.html") do (
	echo Compiling with %%F template...

	if not exist ".\%out_dir%\%%~nF" mkdir ".\%out_dir%\%%~nF"

	call emcc .\src\emscripten\ems_main.c -o .\%out_dir%\%%~nF\%%~nF.html %includes% %libs% %main_compile_flags% !pre_files! --shell-file .\%%F
)

popd
