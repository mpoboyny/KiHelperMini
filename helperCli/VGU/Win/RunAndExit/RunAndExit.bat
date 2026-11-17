@echo off

@set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1"@
@set "PATH=%CUDA_PATH%\bin;%PATH%"

@set VGU_INC=C:\Develop\misc\LlamaCliEx\KiHelperMini\KiHelperMini\helperCli\VGU\Win\VguInclude\
@set VGU_PATH=C:\VGU\virtualGUser-1.3.6\bin\bin_trace_windows_x86_vs\

"%VGU_PATH%virtualGUser-1.3.6.exe" C:\Develop\misc\LlamaCliEx\KiHelperMini\KiHelperMini\helperCli\VGU\Win\RunAndExit\RunAndExit.lua "C:\Develop\misc\LlamaCliEx\KiHelperMini\KiHelperMini\helperCli\bin\release\helperCli.exe" "C:\Users\poboynyy\LocalKiModels\Qwen_Qwen3.5-0.8B-Q4_K_M.gguf"