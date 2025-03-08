@echo off

set cur_path=%~dp0
set build_path=%cur_path%build
set ninja_path=%build_path%\ninja
set vsvarsall_path="D:\Programs\VisualStudio\2022\VC\Auxiliary\Build\vcvarsall.bat"

call %vsvarsall_path% x64
cd %ninja_path%
ninja