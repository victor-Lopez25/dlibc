@echo off

setlocal

if "%1"=="" (
  echo Usage: download.bat {info} [lib/tool]
  echo where lib/tool could be any of ^(you may put multiple^):
  echo libs: arena, view, nob
  echo tools: 4coder, focus, raddbg, odin
  echo Info is optional, It will tell you a short description of any lib from the list
  echo You may also do: download.bat [alllibs/alltools]
  echo To get all libs/tools at once
  goto end
)

rem Preambule
set downloader=wget -O
where /Q wget
if ERRORLEVEL 1 (
  where /Q curl
  if ERRORLEVEL 1 (
    echo could not find curl or wget in the path, please install at least one of them
    goto end
  )
  set downloader=curl -o
)

for %%x in (%*) do (
  if "%%x"=="4coder" (
    set coder4=1
  ) else (
    set "%%x=1"
  )
)
if "%alllibs%"=="1" (
  set arena=1
  set view=1
  set nob=1
)
if "%alltools%"=="1" (
  set coder4=1
  set focus=1
  set raddbg=1
  set odin=1
)

if "%coder4%"=="1" set needmsvc=1
if "%raddbg%"=="1" set needmsvc=1
if "%needmsvc%"=="1" (
  setlocal EnableDelayedExpansion
  where /Q cl.exe || (
    set __VSCMD_ARG_NO_LOGO=1
    for /f "tokens=*" %%i in ('"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
    if "!VS!" equ "" (
      echo ERROR: Visual Studio installation not found
      exit /b 1
    )
    call "!VS!\VC\Auxiliary\Build\vcvarsall.bat" amd64 || exit /b 1
  )
  setlocal DisableDelayedExpansion
)

if "%arena%"=="1" set makemain=1
if "%view%"=="1" set makemain=1
if "%nob%"=="1" set makemain=1
if "%nomain%"=="1" set makemain=0
if "%makemain%"=="1" (
  if not exist src mkdir src
  pushd src
  echo /* date: %date% */ > main.c
)
if "%info%"=="1" (
  if "%arena%"=="1" (
    echo Arena Allocator implementation in pure C as an stb-style single-file library.
    echo https://github.com/tsoding/arena
  )
  if "%view%"=="1" (
    echo Simple String_View implementation for C programming language.
    echo https://github.com/tsoding/sv
  )
  if "%nob%"=="1" (
    echo Header only library for writing build recipes in C.
    echo https://github.com/tsoding/nob.h
  )
  if "%coder4%"=="1" (
    echo 4coder is a modern text editor based loosely on Emacs.
    echo The primary goal of 4coder is to maximize the power and ease of customization.
    echo 4coder also places high priority on performance and portability.
  )
  if "%raddbg%"=="1" (
    echo raddebugger is a native, user-mode, multi-process, graphical debugger.
  )
  if "%odin%"=="1" (
    echo Odin programming language. The Data-Oriented Language for Sane Software Development.
    echo see more at: odin-lang.org
  )
  if "%focus%"=="1" (
    echo A simple editor whose goal is to stand out of your way and let you do work.
    echo It's designed for people who value simplicity, are sensitive to input latency, 
    echo and do not require heavy language support in their editor.
    echo see more at: https://focus-editor.dev/
  )
  if "%2"=="" (
    echo This is a simple downloader for libraries and tools that I like to use, they are these:
    echo Libraries:
    echo  Tsoding's arena allocator in C [arena].
    echo  Tsoding's String_View implementation in C [view].
    echo  Tsoding's nob.h, a library for writing build recipes in C [nob].
    echo Tools:
    echo  Allen Webster's 4coder, a very good text editor for programming in C/C++ [4coder].
    echo  focus editor, the text editor I currently spend the most time in [focus].
    echo  Rad game tools' raddebugger, a native, user-mode, multi-process, graphical debugger [raddbg].
    echo  Ginger Bill's Odin programming language [odin]. see more at odin-lang.org
    echo.
    echo Usage: download.bat {info} [lib/tool]
    echo where lib/tool could be any of ^(you may put multiple^):
    echo libs: arena, view, nob
    echo tools: 4coder, raddbg, odinlang
    echo Info is optional, It will tell you a short description of any lib from the list
    echo.
    echo Tool usage:
    echo  Raddebugger is the debugger I use any time I can, unfortunately it is windows only for now.
    echo  4coder is a great text editor I used to use all the time, right now I use focus editor more since I don't do only C/C++ coding.
    echo  focus editor is the text editor I currently use the most, it has syntax highlighting for any language I'm going to use, from batch to java (added in latest release!).
    echo  Odinlang is a programming language I like to use. See more at odin-lang.org/.
    echo Library usage:
    echo  I mostly only use Tsoding's String_View implementation as startup for any parsing,
    echo  but I do use nob.h throughout whole projects and I want to try his arena allocator.
  )
) else (
  rem downloading libs/tools
  if "%arena%"=="1" (
    echo downloading arena implementation...
    %downloader% arena.h https://raw.githubusercontent.com/tsoding/arena/refs/heads/master/arena.h
    (
      echo #define ARENA_IMPLEMENTATION
      echo #include "arena.h"
      echo.
    ) >> main.c
  )
  if "%view%"=="1" (
    echo downloading string_view implementation...
    %downloader% sv.h https://raw.githubusercontent.com/tsoding/sv/refs/heads/master/sv.h
    (
      echo #define SV_IMPLEMENTATION
      echo #include "sv.h"
      echo.
    ) >> main.c
  )
  if "%nob%"=="1" (
    echo downloading nob.h...
    %downloader% nob.h https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h
    echo making nob.c file...
    (
      echo #define NOB_REBUILD_URSELF^(binpath, srcpath^) "gcc", "-Wall", "Wextra", "-o", binpath, srcpath
      echo #define NOB_IMPLEMENTATION
      echo #include "nob.h"
      echo.
      echo int main^(int argc, char **argv^)
      echo {
      echo   NOB_GO_REBUILD_URSELF^(argc, argv^);
      echo   return 0;
      echo }
    ) > nob.c
  )
  if "%makemain%"=="1" (
    (
      echo int main^(^)
      echo {
      echo   return 0;
      echo }
    ) >> main.c
    popd
  )
  if "%coder4%"=="1" (
    %downloader% 4coder.zip https://github.com/4coder-archive/4coder/archive/refs/heads/master.zip
    %downloader% 4coder-non-source.zip https://github.com/4coder-archive/4coder-non-source/archive/refs/heads/master.zip
    powershell -command "Expand-Archive -Force -Path 4coder.zip -DestinationPath 4coder"
    powershell -command "Expand-Archive -Force -Path 4coder-non-source.zip -DestinationPath 4coder"
    del 4coder.zip
    del 4coder-non-source.zip
    pushd 4coder
    rename 4coder-master code
    rename 4coder-non-source-master 4coder-non-source
    cd code
    rem some directories are created when building? so it doesn't work on the first time but does on the second
    bin\build_optimized.bat
    cd 4coder\code
    bin\build_optimized.bat
    popd
    robocopy 4coder\4coder-non-source\dist_files 4coder\build /E > nul
    robocopy 4coder\code\ship_files 4coder\build /E > nul
  )
  if "%raddbg%"=="1" (
    %downloader% raddebugger.zip https://github.com/EpicGamesExt/raddebugger/archive/refs/heads/master.zip
    powershell -command "Expand-Archive -Force -Path raddebugger.zip -DestinationPath raddebugger"
    del raddebugger.zip
    cd raddebugger
    robocopy raddebugger-master . /E /MOVE > nul
    build release
  )
  if "%odin%"=="1" (
    %downloader% odinlang.zip https://github.com/odin-lang/Odin/archive/refs/heads/master.zip
    powershell -command "Expand-Archive -Force -Path odinlang.zip -DestinationPath odinlang"
    del odinlang.zip
    cd odinlang
    robocopy odin-master . /E /MOVE > nul
    build release
  )
  if "%focus%"=="1" (
    build.bat
    getLatestRelease.exe focus-editor/focus focus.exe
  )
)


:end
endlocal
