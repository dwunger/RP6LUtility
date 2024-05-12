
@echo off
setlocal enabledelayedexpansion

set "script_dir=%~dp0"
set "project_dir=%script_dir%..\..\.."
set "build_dir=%project_dir%\build\test\unit"
set "unit_tests_dir=%project_dir%\test\unit"

if not exist "%build_dir%" mkdir "%build_dir%"

if not exist "%build_dir%\CMakeLists.txt" mklink "%build_dir%\CMakeLists.txt" "%script_dir%\CMakeLists.txt"
if not exist "%build_dir%\GoogleTestLists.txt.in" mklink "%build_dir%\GoogleTestLists.txt.in" "%script_dir%\GoogleTestLists.txt.in"

set link_dirs=test include src
for %%d in (%link_dirs%) do (
  set "source_dir=%project_dir%\%%d"
  set "link_dir=%build_dir%\%%d"
  if not exist "!link_dir!" mklink /d "!link_dir!" "!source_dir!"
)

cd "%build_dir%"
cmake .
cmake --build .
::ctest -C Debug

setlocal
set "exec_path=%project_dir%\build\test\unit\Debug"
for %%x in ("%exec_path%\*.exe") do (
    "%%x"
)
endlocal

