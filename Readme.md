# dlibc

Tools are not supported on linux yet, will get to it soon

### dependencies
The downloader requires wget or curl and to build the tools, msvc

## Usage:
```console
download.bat {info} [lib/tool]
```
where lib/tool could be any of (you may put multiple):

libs: arena, view, nob

tools: 4coder, raddbg

Info is optional, It will tell you a short description of any lib from the list

You may also do:
```console
download.bat [alllibs/alltools]
```
To get all libs/tools at once

You may want to only download the downloader file like so:
```console
wget -O download.bat https://raw.githubusercontent.com/victor-Lopez25/dlibc/refs/heads/main/download.bat
```
Or, if you want to use curl:
```console
curl -o download.bat https://raw.githubusercontent.com/victor-Lopez25/dlibc/refs/heads/main/download.bat
```

### license

[dlibc](https://github.com/victor-Lopez25/dlibc) by [Víctor López](https://github.com/victor-Lopez25) is marked with [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0)