set cmake_path=%~dp0
set build_path=%cmake_path%build
set ninja_path=%build_path%\ninja
set vsvarsall_path=D:\Programs\VisualStudio\2022\VC\Auxiliary\Build\vcvarsall.bat
set cl_path=D:\Programs\VisualStudio\2022\VC\Tools\MSVC\14.42.34433\bin\Hostx64\x64\cl.exe

cmake -S %cmake_path% -B %build_path%
call %vsvarsall_path% x64
cmake -G "Ninja" -S %cmake_path% -B %ninja_path% -DCMAKE_CXX_COMPILER=%cl_path%