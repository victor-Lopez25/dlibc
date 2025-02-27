@echo off

setlocal

if "%1"=="" (
  echo Usage: download.bat {info} [lib]
  echo where lib could be any of ^(you may put multiple^):
  echo arena, view, nob
  echo Info is optional, It will tell you a short description of any lib from the list
  goto end
)

rem Preambule
set downloader=wget -O
where wget > nul 2> nul
if %errorlevel% equ 1 (
  where curl > nul 2> nul
  if %errorlevel% equ 1 (
    echo could not find curl or wget in the path, please install at least one of them
    goto end
  )
  set downloader=curl -o
)

for %%x in (%*) do set "%%x=1"
::set no_main=0
if "%arena%"=="1" set makemain=1
if "%view%"=="1" set makemain=1
if "%nob%"=="1" set makemain=1
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
  if "%2"=="" (
    echo This is a simple downloader for libraries that I like to use, they are these:
    echo Tsoding's arena allocator in C [arena].
    echo Tsoding's String_View implementation in C [view].
    echo Tsoding's nob.h, a library for writing build recipes in C [nob].
    echo.
    echo Usage: downloader.bat {info} [lib]
    "echo where lib could be any of (you may put multiple):"
    echo "arena, view, nob"
    echo Info is optional, It will tell you a short description of any lib from the list
    echo.
    echo I mostly only use Tsoding's String_View implementation as startup for any parsing,
    echo but I do use nob.h throughout whole projects and I want to try his arena allocator.
  )
) else (
  rem downloading libs
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
)


:end
endlocal
