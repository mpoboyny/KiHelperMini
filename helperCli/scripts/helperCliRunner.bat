REM set "CUDA_PATH=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.1"
REM set "PATH=%CUDA_PATH%\bin;%PATH%"

REM helperCli.exe %*

helperCli.exe -m "C:\Users\mpobo\LocalKiModels\Qwen_Qwen3.5-0.8B-Q4_K_M.gguf" -todo "C:\Develop\KiHelperMini\helperCli\bin\release\todo.txt"
