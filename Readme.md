# dlibc

### dependencies
- wget or curl
- msvc on windows, gcc on linux, clang on mac (mac not tested yet)

(msvc is only needed to build the tools, the downloader can be built with any cc)

## Usage:
windows:
```console
build.bat
downloader.exe {info} [lib/tool]
```
linux, mac:
```console
./build.sh
./downloader {info} [lib/tool]
```
where lib/tool could be any of (you may put multiple):

libs: arena, view, nob

tools: 4coder, focus, raddbg, odin

Info is optional, It will tell you a short description of any lib from the list

You may also do:
```console
downloader [alllibs/alltools]
```
To get all libs/tools at once (alltools is specially useful!)

If you want to do it all with a series of commands and skip downloading this repo's zip file:

windows:
```console
wget -O download.bat https://raw.githubusercontent.com/victor-Lopez25/dlibc/refs/heads/main/downloader.c
build.bat
downloder info
```
Or, if you want to use curl:
```console
curl -o download.bat https://raw.githubusercontent.com/victor-Lopez25/dlibc/refs/heads/main/download.bat
build.bat
downloder info
```
linux:
```console
wget -O download.sh https://raw.githubusercontent.com/victor-Lopez25/dlibc/refs/heads/main/download.sh
cc downloader.c -o downloader -O2
chmod +x downloader
./downloder info
```
Or, if you want to use curl:
```console
curl -o download.sh https://raw.githubusercontent.com/victor-Lopez25/dlibc/refs/heads/main/download.sh
cc downloader.c -o downloader -O2
chmod +x downloader
./downloder info
```

### license
[dlibc](https://github.com/victor-Lopez25/dlibc) by [Víctor López](https://github.com/victor-Lopez25) is marked with [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0)
