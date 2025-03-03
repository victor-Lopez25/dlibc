#!/bin/bash

if [[ "$1" == "" ]]; then
  echo "Usage: download.bat {info} [lib/tool]";
  echo "where lib/tool could be any of (you may put multiple):";
  echo "libs: arena, view, nob";
  echo "tools: 4coder, raddbg, odin";
  echo "Info is optional, It will tell you a short description of any lib from the list";
  echo "You may also do: download.bat [alllibs/alltools]";
  echo "To get all libs/tools at once";
  failed=1;
fi

for arg in "$@"
do
  if [ "$arg" = "4coder" ]; then
    coder4=1;
  else
    eval "$arg=1";
  fi
done

downloader="wget -O";
if ! type wget >/dev/null 2>&1; then
  if ! type curl >/dev/null 2>&1; then
    # curl is likely already installed
    echo "Please install either wget or curl, they are needed for this script.";
    echo "wget: https://www.gnu.org/software/wget/";
    echo "curl: https://curl.se/download.html";
    failed=1;
  else
    downloader="curl -o";
  fi
fi

if [ -n "$alllibs" ]; then
  arena=1
  view=1
  nob=1
fi

if [ -n "$alltools" ]; then
  coder4=1
  #raddbg=1 (not on linux yet)build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev
  odin=1
fi

if [ -n "$arena" ] || [ -n "$view" ] || [ -n "$nob" ]; then
  makemain=1;
fi
if ! [ -n "$nomain" ] && [ -n "$makemain" ]; then
  mkdir -p src;
  pushd src > /dev/null;
  # do I want the date be the same format as the batch date?
  echo "/* date: $(date) */" > main.c;
fi

if [ -n "$failed" ]; then
  echo "Failed somewhere, aborting...";
elif [ -n "$info" ]; then
    if [ -n "$arena" ]; then
      echo "Arena Allocator implementation in pure C as an stb-style single-file library.";
      echo "https://github.com/tsoding/arena";
    fi
    if [ -n "$view" ]; then
      echo "Simple String_View implementation for C programming language.";
      echo "https://github.com/tsoding/sv";
    fi
    if [ -n "$nob" ]; then
      echo "Header only library for writing build recipes in C.";
      echo "https://github.com/tsoding/nob.h";
    fi
    if [ -n "$coder4" ]; then
      echo "4coder is a modern text editor based loosely on Emacs.";
      echo "The primary goal of 4coder is to maximize the power and ease of customization.";
      echo "4coder also places high priority on performance and portability.";
      echo "This is the community version of 4coder (4cc) since I couldn't get 4coder to compile, it's not any worse than 4coder though";
    fi
    if [ -n "$raddbg" ]; then
      echo "raddebugger is a native, user-mode, multi-process, graphical debugger.";
      echo "it doesn't fully support linux yet";
    fi
    if [ -n "$odin" ]; then
      echo "Odin programming language. The Data-Oriented Language for Sane Software Development.";
      echo "see more at odin-lang.org";
    fi
    if [[ "$2"=="" ]]; then
      echo "This is a simple downloader for libraries and tools that I like to use, they are these:";
      echo "Libraries:";
      echo " Tsoding's arena allocator in C [arena].";
      echo " Tsoding's String_View implementation in C [view].";
      echo " Tsoding's nob.h, a library for writing build recipes in C [nob].";
      echo "Tools:";
      echo " Allen Webster's 4coder, a very good text editor for programming in C/C++, this is the community version [4coder].";
      echo " Rad game tools' raddebugger, a native, user-mode, multi-process, graphical debugger [raddbg].";
      echo " Ginger Bill's Odin programming language [odin]. see more at odin-lang.org";
      echo "";
      echo "Usage: download.bat {info} [lib/tool]";
      echo "where lib/tool could be any of (you may put multiple):";
      echo "libs: arena, view, nob";
      echo "tools: 4coder, raddbg, odinlang";
      echo "Info is optional, It will tell you a short description of any lib from the list";
      echo "";
      echo "Tool usage:";
      echo " Raddebugger is the debugger I use any time I can, unfortunately it is windows only for now.";
      echo " 4coder is a great text editor I used to use all the time, right now I use focus editor more since I don't do only C/C++ coding.";
      echo " Odinlang is a programming language I like to use. See more at odin-lang.org/.";
      echo "Library usage:";
      echo " I mostly only use Tsoding's String_View implementation as startup for any parsing,";
      echo " but I do use nob.h throughout whole projects and I want to try his arena allocator.";
    fi
# downloading libs/tools
else
  if [ -n "$arena" ]; then
    echo "downloading arena implementation...";
    $downloader arena.h https://raw.githubusercontent.com/tsoding/arena/refs/heads/master/arena.h
    echo -e "#define ARENA_IMPLEMENTATION
#include \"arena.h\"
" >> main.c
  fi

  if [ -n "$view" ]; then
    echo "downloading string_view implementation...";
    $downloader sv.h https://raw.githubusercontent.com/tsoding/sv/refs/heads/master/sv.h
    echo -e "#define SV_IMPLEMENTATION
#include \"sv.h\"
" >> main.c
  fi

  if [ -n "$nob" ]; then
    echo "downloading nob.h...";
    $downloader nob.h https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h
    echo "making nob.c file...";
    echo "#define NOB_REBUILD_URSELF(binpath, srcpath) \"gcc\", \"-Wall\", \"Wextra\", \"-o\", binpath, srcpath
#define NOB_IMPLEMENTATION
#include \"nob.h\"

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);
  return 0;
}
" > nob.c
  fi
  # this must be the last thing in libs
  if [ -n "$makemain" ]; then
    echo -e "int main()
{
  return 0;
}
" >> main.c
    popd > /dev/null
  fi

  # tools
  if [ -n "$coder4" ]; then
    echo "installing required libraries to compile 4coder...";
    # show the command to install the required libs
    set -x
    sudo apt install build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev
    set +x
    $downloader 4cc.zip https://github.com/4coder-community/4cc/archive/refs/heads/master.zip
    echo "unzipping 4cc.zip..."
    unzip 4cc.zip >/dev/null
    mv 4cc-master 4cc
    cd 4cc/code
    ./bin/package-linux.sh
    echo ""
    echo "4coder will be in current_dist_super_*" # if I understand correctly
    cd ../..
  fi

  if [ -n "$raddbg" ]; then
    echo "Raddebugger is windows only for now";
  fi

  if [ -n "$odin" ]; then
    echo "To be completely honest I don't know if I should install llvm-dev";
    echo "It definitely looks like something that would be pretty big";
  fi
fi
