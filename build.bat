@echo off

setlocal EnableDelayedExpansion

set cc=gcc -o downloader.exe -O2
where /Q gcc || (
  where /Q cl.exe || (
    set __VSCMD_ARG_NO_LOGO=1
    for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
    if "!VS!" equ "" (
      echo ERROR: Could not find gcc or msvc installation
      exit /b 1
    )
    call "!VS!\VC\Auxiliary\Build\vcvarsall.bat" amd64 || exit /b 1
    set cc=cl -FC -GR- -EHa- -nologo -O2 -Fedownloader.exe -D_CRT_SECURE_NO_WARNINGS
  )
)
setlocal DisableDelayedExpansion

%cc% downloader.c