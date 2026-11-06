@echo off

@set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1"@
@set "PATH=%CUDA_PATH%\bin;%PATH%"

@rem helperCli.exe -m "C:\Users\poboynyy\LocalKiModels\Qwen_Qwen3.5-0.8B-Q4_K_M.gguf"
helperCli.exe -m "C:\Users\poboynyy\LocalKiModels\Qwen_Qwen3.5-9B-Q4_K_M.gguf"
