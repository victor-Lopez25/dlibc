@echo off

setlocal

if "%1"=="" (
  echo Usage: downloader.bat {info} [lib]
  echo where lib could be any of ^(you may put multiple^):
  echo arena, view, nob
  echo Info is optional, It will tell you a short description of any lib from the list
  goto end
)

rem Preambule
set downloader=curl -o
where curl > nul 2> nul
if %errorlevel% equ 1 (
  where wget > nul 2> nul
  if %errorlevel% equ 1 (
    echo could not find curl or wget in the path, please install at least one of them
    goto end
  )
  set downloader=wget -O
)

for %%x in (%*) do set "%%x=1"
::set no_main=0
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
  if not exist src mkdir src
  pushd src
  rem downloading libs
  if "%arena%"=="1" (
    echo downloading arena implementation...
    %downloader% arena.h https://raw.githubusercontent.com/tsoding/arena/refs/heads/master/arena.h
  )
  if "%view%"=="1" (
    echo downloading string_view implementation...
    %downloader% sv.h https://raw.githubusercontent.com/tsoding/sv/refs/heads/master/sv.h
  )
  if "%nob%"=="1" (
    echo downloading nob.h...
    %downloader% nob.h https://raw.githubusercontent.com/tsoding/nob/refs/heads/master/nob.h
  )
  popd
)

:end
endlocal
