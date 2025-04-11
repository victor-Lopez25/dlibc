#define NOB_IMPLEMENTATION
#include "nob.h"

#include <time.h> /* for local time in main.c */

bool cstrStartsWith(const char *str, const char *pre)
{
  size_t lenpre = strlen(pre);
  size_t lenstr = strlen(str);
  return (lenstr >= lenpre) && (memcmp(pre, str, lenpre) == 0);
}

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
# define ARCH_X64 1
#elif defined(__aarch64__)
# define ARCH_ARM64 1
#endif

#if _WIN32
# define where(command) "where /Q "command
# define UnzipToDir(in, out) system("powershell -command \"Expand-Archive -Force -Path "in" -DestinationPath "out)
# define UnzipCmd(in, out) "powershell -command \"Expand-Archive -Force -Path %s -Destination %s", in, out
# define ChangeDirectory(dir) (SetCurrentDirectory(dir))
// for debug
# define PrintWorkingDirectory "cd"
# define asciitime(buf, bufsize, rawtime) asctime_s(buf, bufsize, localtime(&rawtime))
#else
# define where(command) "type "command" >/dev/null 2>&1"
# define UnzipToDir(in, out) system("unzip "in" -d "out" >/dev/null")
# define UnzipCmd(in, out) "unzip %s -d %s >/dev/null", in, out
# define ChangeDirectory(dir) (chdir(dir) == 0)
// for debug
# define PrintWorkingDirectory "pwd"
# define asciitime(buf, bufsize, rawtime) asctime_r(localtime(&rawtime), buf)
#endif

Nob_Cmd cmd = {0};

char *downloader[2] = {"wget", "-O"};
#define DOWNLOADER downloader[0], downloader[1]

bool DownloadGithubLatestRelease(char *githubProject, char *fileToInstall, bool isPrefix, char **fullFileName)
{
  assert(isPrefix == (fullFileName != 0));

  char *projectName = githubProject;
  char *githubLinkStart = "https://github.com/";
  if(cstrStartsWith(githubProject, githubLinkStart)) {
    projectName = githubProject + strlen(githubLinkStart);
  }
  else {
    githubProject = nob_temp_sprintf("%s%s", githubLinkStart, projectName);
  }

  char *temp_file = "project_html";
  char *githubProjectRawFile = temp_file;
  nob_cmd_append(&cmd, DOWNLOADER, githubProjectRawFile, githubProject);
  if(!nob_cmd_run_sync_and_reset(&cmd)) {
    fprintf(stderr, "Could not find github project website\n");
    return false;
  }

  Nob_String_Builder sb = {0};
  if(!nob_read_entire_file(githubProjectRawFile, &sb)) {
    fprintf(stderr, "Could not read '%s' file\n", githubProjectRawFile);
    return false;
  }
  nob_sb_append_null(&sb);

  char *fileData = sb.items;
  char *releaseName = strstr(fileData, "/releases/tag/");
  if(!releaseName) {
    fprintf(stderr, "Could not find a release for the project\n");
    nob_sb_free(sb);
    return false;
  }
  releaseName += strlen("/releases/tag/");
  assert(!strstr(releaseName, "/releases/tag/"));

  char *endStr = releaseName;
  while(*endStr && *endStr != '"') endStr++;
  *endStr = 0; // add a null terminator here so we can use releaseName -> endStr as a cstr

  if(isPrefix) {
    char *latestReleasePage =
      nob_temp_sprintf("%s/releases/expanded_assets/%s", githubProject, releaseName);
    nob_cmd_append(&cmd, DOWNLOADER, "latestReleaseLinks", latestReleasePage);
    if(!nob_cmd_run_sync_and_reset(&cmd)) {
      fprintf(stderr, "Could not get latest release page from github\n");
      nob_sb_free(sb);
      return false;
    }

    Nob_String_Builder lrPageData = {0};
    if(!nob_read_entire_file("latestReleaseLinks", &lrPageData)) {
      fprintf(stderr, "Could not read latestReleaseLinks file\n");
      nob_sb_free(sb);
      return false;
    }
    nob_sb_append_null(&lrPageData);

    char *startStr = strstr(lrPageData.items, fileToInstall);
    if(!startStr) {
      nob_sb_free(sb);
      return false;
    }
    endStr = strstr(startStr, ".zip") + strlen(".zip");
    *endStr = 0;
    fileToInstall = nob_temp_strdup(startStr);
    *fullFileName = fileToInstall;

    nob_sb_free(lrPageData);
    nob_delete_file("latestReleaseLinks");
  }

  char *latestReleasePageFile =
    nob_temp_sprintf("%s/releases/download/%s/%s", githubProject, releaseName, fileToInstall);
  nob_cmd_append(&cmd, DOWNLOADER, fileToInstall, latestReleasePageFile);
  if(!nob_cmd_run_sync_and_reset(&cmd)) {
    fprintf(stderr, "Could not get file from github latest release page\n");
    nob_sb_free(sb);
    return false;
  }

  nob_delete_file(temp_file);
  nob_sb_free(sb);

  return true;
}

void UnknownCommand(char *arg)
{
  printf("Ignoring unknown lib/tool '%s'\n", arg);
}

void ShowDetailedInfo()
{
  printf("This is a simple downloader for libraries and tools that I like to use, they are these:\n"
         " Libraries:\n"
         "  Tsoding's arena allocator in C [arena].\n"
         "  Tsoding's String_View implementation in C [view].\n"
         "  Tsoding's nob.h, a library for writing build recipes in C [nob].\n"
         " Tools:\n"
         "  Allen Webster's 4coder, a very good text editor for programming in C/C++ [4coder].\n"
         "  focus editor, the text editor I currently spend the most time in [focus].\n"
         "  Rad game tools' raddebugger, a native, user-mode, multi-process, graphical debugger [raddbg].\n"
         "  Ginger Bill's Odin programming language [odin]. see more at odin-lang.org\n"
         "\n"
         " Usage: downloader {info} [lib/tool]\n"
         " where lib/tool could be any of (you may put multiple):\n"
         " libs: arena, view, nob\n"
         " tools: 4coder, raddbg, odinlang\n"
         " Info is optional, It will tell you a short description of any lib/tool from the list\n"
         "\n"
         "Update: downloader update\n"
         "\n"
         " Tool usage:\n"
         "  Raddebugger is the debugger I use any time I can, unfortunately it is windows only for now.\n"
         "  4coder is a great text editor I used to use all the time, right now I use focus editor more since I don't do only C/C++ coding.\n"
         "  focus editor is the text editor I currently use the most, it has syntax highlighting for any language I'm going to use, from batch to java (added in latest release!).\n"
         "  Odinlang is a programming language I like to use. See more at odin-lang.org/.\n"
         " Library usage:\n"
         "  I mostly only use Tsoding's String_View implementation as startup for any parsing,\n"
         "  but I do use nob.h throughout whole projects and I want to try his arena allocator.\n");
}

enum {
  DOWNLOAD_arena,
  DOWNLOAD_view,
  DOWNLOAD_nob,
  DOWNLOAD_4coder,
  DOWNLOAD_focus,
  DOWNLOAD_raddbg,
  DOWNLOAD_odin,
  DOWNLOAD_count,
};

bool YesNoQuestion(char *msg)
{
  char c;
  do {
    printf("\n%s [Y/n] ", msg);
    c = (char)getchar();
  } while(c != 'y' && c != 'Y' && c != 'n');
  return c != 'n';
}

// for nesting conditions
bool append_null_ret_true(Nob_String_Builder *sb) { nob_sb_append_null(sb); return true; }

// Moved out to a function due to surprisingly complex linux handling
bool DownloadOdinlang()
{
#ifndef _WIN32
  printf("To build from source, install clang and LLVM (the versions we support are 14, 17 and 18) using your package manager\n"
         "It could be that LLVM is split into multiple packages and you also need to install something like llvm-devel\n"
         "Make sure llvm-config, llvm-config-(14|17|18), or llvm-config(14|17|18) and clang are able to be found through your $PATH\n"
         "If you want to specify an explicit LLVM version or path, you can set the LLVM_CONFIG environment variable: LLVM_CONFIG=/path/to/llvm-config make release-native\n");
#endif
  bool buildFromSource = YesNoQuestion("Build odin from source?");
  if(buildFromSource) {
odin_build_from_source:
    nob_cmd_append(&cmd, DOWNLOADER, "odinlang.zip", "https://github.com/odin-lang/Odin/archive/refs/heads/master.zip");
    if(!nob_cmd_run_sync_and_reset(&cmd)) return false;
    UnzipToDir("odinlang.zip", "OdinLang");
    nob_delete_file("odinlang.zip");
    ChangeDirectory("OdinLang");
    nob_minimal_log_level = NOB_WARNING;
    if(!nob_copy_directory_recursively("odin-master", ".")) return false;
    nob_minimal_log_level = NOB_INFO;
#if _WIN32
    system("build release");
#elif linux
    system("make release-native");
    printf("If an atomic.h error occurred, see: https://odin-lang.org/docs/install/#a-note-on-atomich\n");
#endif
  }
  else {
    if(!nob_mkdir_if_not_exists("OdinLang")) return false;
    ChangeDirectory("OdinLang");
    char *fullFileName;
#if linux
    if(!ARCH_X64) {
      fprintf(stderr, "There is no prebuilt Odin zip file for this microarch in linux\n");
      if(YesNoQuestion("Build odin from source?")) goto odin_build_from_source;
      return false;
    }

    char *fileToDownload = "odin-linux-amd64-dev";
    if(!DownloadGithubLatestRelease("odin-lang/Odin", fileToDownload, true, &fullFileName)) {
      fileToDownload = "odin-ubuntu-amd64-dev";
      if(system("cat /proc/version | grep Ubuntu >/dev/null")) {
        fprintf(stderr, "There is no prebuilt Odin zip file for this distribution of linux\n");
        if(YesNoQuestion("Build odin from source?")) goto odin_build_from_source;
        return false;
      }
      if(!DownloadGithubLatestRelease("odin-lang/Odin", fileToDownload, true, &fullFileName)) return false;
      system(nob_temp_sprintf(UnzipCmd(fullFileName, ".")));
      nob_delete_file(fullFileName);
      system("tar -xf dist.tar.gz");
      nob_delete_file("dist.tar.gz");
      Nob_File_Paths filepaths = {0};
      if(!nob_read_entire_dir(".", &filepaths)) return false;
      int pathIdx = -1;
      for(int i = 0; i < filepaths.count; i++)
      {
        if(cstrStartsWith(filepaths.items[i], "odin")) pathIdx = i;
      }
      if(pathIdx == -1) {
        fprintf(stderr, "Could not find odin directory\n"); // shouldn't happen
        return false;
      }
      nob_minimal_log_level = NOB_WARNING;
      nob_copy_directory_recursively(filepaths.items[pathIdx], ".");
      nob_minimal_log_level = NOB_INFO;
    }
    else {
      system(nob_temp_sprintf(UnzipCmd(fullFileName, ".")));
      nob_delete_file(fullFileName);
      fullFileName[strlen(fullFileName)-strlen(".zip")] = 0;
      nob_minimal_log_level = NOB_WARNING;
      nob_copy_directory_recursively(fullFileName, ".");
      nob_minimal_log_level = NOB_INFO;
    }
    ChangeDirectory("..");
#else
#if _WIN32
    char *fileToDownload = "odin-windows-amd64-dev";
#elif ARCH_X64
    char *fileToDownload = "odin-macos-amd64-dev";
#elif ARCH_ARM64
    char *fileToDownload = "odin-macos-arm64-dev";
#else
    fprintf(stderr, "unsupported platform/arch\n");
    return false;
#endif
    if(!DownloadGithubLatestRelease("odin-lang/Odin", fileToDownload, true, &fullFileName)) return false;
    system(nob_temp_sprintf(UnzipCmd(fullFileName, ".")));
    nob_delete_file(fullFileName);
    ChangeDirectory("..");
#endif
  }
  return true;
}

int UpdateSelf()
{  
  if(!nob_file_exists("downloader.c")) {
    fprintf(stderr, "ERROR: This argument can only be given with '"__FILE__"' and 'nob.h' in the same directory\n");
    return 1;
  }

  printf("Checking for updates...\n");
  nob_cmd_append(&cmd, DOWNLOADER, __FILE__".tmp", "https://raw.githubusercontent.com/victor-Lopez25/dlibc/refs/heads/main/downloader.c");
  if(!nob_cmd_run_sync_and_reset(&cmd)) {
    fprintf(stderr, "aborting...\n");
  }

  Nob_String_Builder sb1 = {0};
  Nob_String_Builder sb2 = {0};
  if(!nob_read_entire_file(__FILE__, &sb1) || 
     !nob_read_entire_file(__FILE__".tmp", &sb2)) {
    fprintf(stderr, "aborting...\n");
    nob_delete_file(__FILE__".tmp");
    return 1;
  }

  size_t size = sb1.count > sb2.count ? sb2.count : sb1.count;
  if(memcmp(sb1.items, sb2.items, size) == 0) {
    printf("Already up to date.\n");
    nob_delete_file(__FILE__".tmp");
    return 0;
  }

  if(nob_delete_file(__FILE__) || nob_rename(__FILE__".tmp", __FILE__)) {
    printf("Successfully updated "__FILE__". Please run build.bat to build the new version.\n");
  }

  return 0;
}

int main(int argc, char **argv)
{
  //NOB_GO_REBUILD_URSELF(argc, argv);

  if(argc < 2) {
    printf("Usage: %s {info} [lib/tool]\n", argv[0]);
    printf("where lib/tool could be any of (you may put multiple):\n"
           "libs: arena, view, nob\n"
           "tools: 4coder, focus, raddbg, odin\n"
           "Info is optional, It will tell you a short description of any lib from the list\n"
           "You may also do: 'downloader [alllibs/alltools]' To get all libs/tools at once\n"
           "To update: downlaoder update\n");
    return 1;
  }

  if(system(where("wget"))) {
    // couldn't find wget, try to use curl
    if(system(where("curl"))) {
      fprintf(stderr, "Could not find wget or curl, please install at least one of the two\n");
      return 1;
    }
    downloader[0] = "curl"; downloader[1] = "-o";
  }

  bool toDownload[DOWNLOAD_count] = {0};
  bool showInfo = false;
  bool noMakeMain = false;

  for(int i = 1; i < argc; i++)
  {
    char *arg = argv[i];
    if(memcmp(arg, "update", strlen("update")) == 0) {
      return UpdateSelf();
    } 

    switch(arg[0]) {
      case '4': {
        if(memcmp(arg, "4coder", strlen("4coder")) == 0) {
          toDownload[DOWNLOAD_4coder] = true;
        }
        else UnknownCommand(arg);
      } break;

      case 'a': {
        if(memcmp(arg, "alllibs", strlen("alllibs")) == 0) {
          toDownload[DOWNLOAD_arena] = true;
          toDownload[DOWNLOAD_view] = true;
          toDownload[DOWNLOAD_nob] = true;
        }
        else if(memcmp(arg, "alltools", strlen("alltools")) == 0) {
          toDownload[DOWNLOAD_4coder] = true;
          toDownload[DOWNLOAD_focus] = true;
          toDownload[DOWNLOAD_raddbg] = true;
          toDownload[DOWNLOAD_odin] = true;
        }
        else if(memcmp(arg, "arena", strlen("arena")) == 0) {
            toDownload[DOWNLOAD_arena] = true;
        }
        else UnknownCommand(arg);
      } break;

      case 'f': {
        if(memcmp(arg, "focus", strlen("focus")) == 0) {
          toDownload[DOWNLOAD_focus] = true;
        }
        else UnknownCommand(arg);
      } break;

      case 'i': {
        if(memcmp(arg, "info", strlen("info")) == 0) {
          showInfo = true;
        }
        else UnknownCommand(arg);
      } break;

      case 'n': {
        if(memcmp(arg, "nob", strlen("nob")) == 0) {
          toDownload[DOWNLOAD_nob] = true;
        }
        else if(memcmp(arg, "nomain", strlen("nomain")) == 0) {
          noMakeMain = true;
        }
        else UnknownCommand(arg);
      } break;

      case 'o': {
        if(memcmp(arg, "odin", strlen("odin")) == 0) {
          toDownload[DOWNLOAD_odin] = true;
        }
        else UnknownCommand(arg);
      } break;

      case 'r': {
        if(memcmp(arg, "raddbg", strlen("raddbg")) == 0) {
          toDownload[DOWNLOAD_raddbg] = true;
        }
        else UnknownCommand(arg);
      } break;

      case 'v': {
        if(memcmp(arg, "view", strlen("view")) == 0) {
          toDownload[DOWNLOAD_view] = true;
        }
        else UnknownCommand(arg);
      } break;

      default: { UnknownCommand(arg); } break;
    }
  }

  bool onlyInfo = true;
  for(int i = 0; i < (int)NOB_ARRAY_LEN(toDownload); i++) if(toDownload[i]) onlyInfo = false;

  if(onlyInfo) {
    ShowDetailedInfo();
    return 0;
  }

  if(showInfo) {
    if(toDownload[DOWNLOAD_arena]) {
      printf("Arena Allocator implementation in pure C as an stb-style single-file library.\n"
             "https://github.com/tsoding/arena\n");
    }
    if(toDownload[DOWNLOAD_view]) {
      printf("Simple String_View implementation for C programming language.\n"
             "https://github.com/tsoding/sv\n");
    }
    if(toDownload[DOWNLOAD_nob]) {
      printf("Header only library for writing build recipes in C.\n"
             "https://github.com/tsoding/nob.h\n");
    }
    if(toDownload[DOWNLOAD_4coder]) {
      printf("4coder is a modern text editor based loosely on Emacs.\n"
             "The primary goal of 4coder is to maximize the power and ease of customization.\n"
             "4coder also places high priority on performance and portability.\n");
    }
    if(toDownload[DOWNLOAD_raddbg]) {
      printf("raddebugger is a native, user-mode, multi-process, graphical debugger.\n");
    }
    if(toDownload[DOWNLOAD_odin]) {
      printf("Odin programming language. The Data-Oriented Language for Sane Software Development.\n"
             "see more at: odin-lang.org\n");
    }
    if(toDownload[DOWNLOAD_focus]) {
      printf("A simple editor whose goal is to stand out of your way and let you do work.\n"
             "It's designed for people who value simplicity, are sensitive to input latency,\n"
             "and do not require heavy language support in their editor.\n"
             "see more at: https://focus-editor.dev/\n");
    }
  }
  else {
    bool makeMain = (toDownload[DOWNLOAD_arena]|toDownload[DOWNLOAD_view]|
                     toDownload[DOWNLOAD_nob]) && !noMakeMain;
    Nob_String_Builder mainBuilder = {0};
    if(makeMain) {
      if(!nob_mkdir_if_not_exists("src")) return 1;
      time_t rawtime;
      time(&rawtime);
      char timebuf[26];
      asciitime(timebuf, 26, rawtime);
      timebuf[strlen(timebuf)-1] = 0; // I don't want the trailing '\n'
      nob_sb_append_cstr(&mainBuilder, "/* ");
      nob_sb_append_cstr(&mainBuilder, timebuf);
      nob_sb_append_cstr(&mainBuilder, " */\n");
    }

    if(toDownload[DOWNLOAD_arena]) {
      printf("downloading arena implementation...\n");
      nob_cmd_append(&cmd, DOWNLOADER, "src/arena.h", "https://raw.githubusercontent.com/tsoding/arena/refs/heads/master/arena.h");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      nob_sb_append_cstr(&mainBuilder, "#define ARENA_IMPLEMENTATION\n"
                         "#include \"arena.h\"\n\n");
    }
    if(toDownload[DOWNLOAD_nob]) {
      printf("downloading nob.h...\n");
      nob_cmd_append(&cmd, DOWNLOADER, "src/nob.h", "https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      printf("making nob.c file...\n");
      char *nobCFileData =
        "#define NOB_REBUILD_URSELF(binpath, srcpath) \"gcc\", \"-Wall\", \"-Wextra\", \"-o\", binpath, srcpath\n"
        "#define NOB_IMPLEMENTATION\n"
        "#include \"nob.h\"\n"
        "\n"
        "int main(int argc, char **argv)\n"
        "{\n"
        "  NOB_GO_REBUILD_URSELF(argc, argv);\n"
        "  return 0;\n"
        "}\n";
      if(!nob_write_entire_file("src/nob.c", nobCFileData, strlen(nobCFileData))) return 1;
    }
    if(toDownload[DOWNLOAD_view]) {
      printf("downloading String_View implementation...\n");
      nob_cmd_append(&cmd, DOWNLOADER, "src/sv.h", "https://raw.githubusercontent.com/tsoding/sv/refs/heads/master/sv.h");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      nob_sb_append_cstr(&mainBuilder, "#define SV_IMPLEMENTATION\n"
                         "#include \"sv.h\"\n\n");
    }

    if(makeMain) {
      nob_sb_append_cstr(&mainBuilder, "int main()\n{\n  return 0;\n}\n");
      if(!nob_write_entire_file("src/main.c", mainBuilder.items, mainBuilder.count)) return 1;
      nob_sb_free(mainBuilder);
    }

#if _WIN32
    if(toDownload[DOWNLOAD_4coder] || toDownload[DOWNLOAD_raddbg]) {
      printf("Make sure you have initialized msvc by calling \"vcvarsall.bat x64\"\n"
             "This script is automatically called by the 'x64 Native Tools Command Prompt for VS <year>' variant of the vanilla cmd.exe\n"
             "If you've installed the build tools, this command prompt may be easily located by searching for Native from the Windows Start Menu search.\n"
             "Now is the chance to exit using Ctrl-C if you haven't called vcvarsall.bat\n");
      if(!YesNoQuestion("Do you want to continue?")) return 0;
    }
#endif

    if(toDownload[DOWNLOAD_4coder]) {
#ifndef _WIN32
      printf("You need a couple of libs to build 4coder, if you're not sure you have them, Ctrl-C out and try to install them\n"
             "make sure to have them installed before rerunning this program:\n"
             "build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev\n"
             "You can also download 4coder prebuilt, from: https://mr-4th.itch.io/4coder\n\n");
      if(!YesNoQuestion("Do you want to continue?")) return 0;
#endif

      nob_cmd_append(&cmd, DOWNLOADER, "4coder.zip", "https://github.com/4coder-archive/4coder/archive/refs/heads/master.zip");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      nob_cmd_append(&cmd, DOWNLOADER, "4coder-non-source.zip", "https://github.com/4coder-archive/4coder-non-source/archive/refs/heads/master.zip");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      UnzipToDir("4coder.zip", "4coder");
      UnzipToDir("4coder-non-source.zip", "4coder");
      nob_delete_file("4coder.zip");
      nob_delete_file("4coder-non-source.zip");
      ChangeDirectory("4coder");
      nob_rename("4coder-master", "code");
      nob_rename("4coder-non-source-master", "4coder-non-source");
      ChangeDirectory("code");
#if _WIN32
      system("bin\\build_optimized.bat");
      ChangeDirectory("4coder/code");
      system("bin\\build_optimized.bat");
#elif linux
      system("./bin/build-linux.sh");
#else
      system("./bin/build-mac.sh"); // untested, but no reason it shouldn't work?
#endif
      ChangeDirectory("../..");
      nob_minimal_log_level = NOB_WARNING; // a bunch of files, you probably don't want INFO here
      if(!nob_copy_directory_recursively("4coder/4coder-non-source/dist_files", "4coder/build")) return 1;
      if(!nob_copy_directory_recursively("4coder/code/ship_files", "4coder/build")) return 1;
      nob_minimal_log_level = NOB_INFO;
    }

    if(toDownload[DOWNLOAD_raddbg]) {
#if _WIN32
      nob_cmd_append(&cmd, DOWNLOADER, "raddebugger.zip", "https://github.com/EpicGamesExt/raddebugger/archive/refs/heads/master.zip");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      UnzipToDir("raddebugger.zip", "raddebugger");
      nob_delete_file("raddebugger.zip");
      ChangeDirectory("raddebugger");
      nob_minimal_log_level = NOB_WARNING;
      if(!nob_copy_directory_recursively("raddebugger-master", ".")) return 1;
      nob_minimal_log_level = NOB_INFO;
      system("build.bat release");
#else
      printf("Raddebugger is only supported on windows for now\n");
#endif
    }

    if(toDownload[DOWNLOAD_odin]) {
      DownloadOdinlang();
    }

    if(toDownload[DOWNLOAD_focus]) {
#if _WIN32
      char *fileToDownload = "focus.exe";
#elif linux
      char *fileToDownload = "focus-linux";
#else
      char *fileToDownload = "focus-macOS.dmg";
#endif
      if(!nob_mkdir_if_not_exists("focus")) return 1;
      ChangeDirectory("focus");
      if(!DownloadGithubLatestRelease("focus-editor/focus", fileToDownload, false, 0)) return 1;
      ChangeDirectory("..");
    }
  }

  nob_cmd_free(cmd);

  return 0;
}