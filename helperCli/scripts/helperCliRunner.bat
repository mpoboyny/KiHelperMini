@echo off

@set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1"
@set "PATH=%CUDA_PATH%\bin;%PATH%"

@rem helperCli.exe -m "%USERPROFILE%\LocalKiModels\Qwen_Qwen3.5-0.8B-Q4_K_M.gguf" --log-file helperCli.log -v
helperCli.exe -m "%USERPROFILE%\LocalKiModels\Qwen_Qwen3.5-0.8B-Q4_K_M.gguf"
