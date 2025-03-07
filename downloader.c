#define NOB_IMPLEMENTATION
#include "nob.h"

#include <time.h> /* for local time in main.c */

bool cstrStartsWith(const char *str, const char *pre)
{
  size_t lenpre = strlen(pre);
  size_t lenstr = strlen(str);
  return (lenstr >= lenpre) && (memcmp(pre, str, lenpre) == 0);
}

#if _WIN32
#define where(command) "where /Q "command
#define unzip(in, out) "powershell -command \"Expand-Archive -Force -Path "in" -DestinationPath "out
#define ChangeDirectory(dir) (SetCurrentDirectory(dir))
#else
#define where(command) "type "command" >/dev/null 2>&1"
#define unzip(in, out) "unzip "in" >/dev/null"
#define ChangeDirectory(dir) (chdir(dir) == 0)
#endif

Nob_Cmd cmd = {0};

char *downloader[2] = {"wget", "-O"};
#define DOWNLOADER downloader[0], downloader[1]

void DownloadGithubLatestRelease(char *githubProject, char *fileToInstall)
{
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
  nob_cmd_append(&cmd, downloader[0], downloader[1], githubProjectRawFile, githubProject);
  if(!nob_cmd_run_sync_and_reset(&cmd)) {
    fprintf(stderr, "Could not find github project website\n");
    nob_cmd_free(cmd);
    return 2;
  }

  Nob_String_Builder sb = {0};
  if(!nob_read_entire_file(githubProjectRawFile, &sb)) {
    fprintf(stderr, "Could not read '%s' file\n", githubProjectRawFile);
    nob_cmd_free(cmd);
    return 3;
  }
  nob_sb_append_null(&sb);

  char *fileData = sb.items;
  char *releaseName = strstr(fileData, "/releases/tag/");
  if(!releaseName) {
    fprintf(stderr, "Could not find a release for the project\n");
    nob_sb_free(sb);
    nob_cmd_free(cmd);
    return 4;
  }
  releaseName += strlen("/releases/tag/");
  assert(!strstr(releaseName, "/releases/tag/"));

  char *endStr = releaseName;
  while(*endStr && *endStr != '"') endStr++;
  *endStr = 0; // add a null terminator here so we can use releaseName -> endStr as a cstr

  char *latestReleasePageFile =
    nob_temp_sprintf("%s/releases/download/%s/%s", githubProject, releaseName, fileToInstall);
  nob_cmd_append(&cmd, downloader[0], downloader[1], fileToInstall, latestReleasePageFile);
  if(!nob_cmd_run_sync_and_reset(&cmd)) {
    fprintf(stderr, "Could not get file from github latest release page\n");
    nob_sb_free(sb);
    nob_cmd_free(cmd);
    return 5;
  }

  if(!nob_delete_file(temp_file)) {
    fprintf(stderr, "Could not delete temporary file: %s\n", temp_file);
    return 6;
  }
}

void UnknownCommand(char *arg)
{
  printf("Ignoring unknown lib/tool '%s'\n", arg);
}

void ShowDetailedInfo()
{
  printf("This is a simple downloader for libraries and tools that I like to use, they are these:\n");
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
         " Usage: download.bat {info} [lib/tool]\n"
         " where lib/tool could be any of (you may put multiple):\n"
         " libs: arena, view, nob\n"
         " tools: 4coder, raddbg, odinlang\n"
         " Info is optional, It will tell you a short description of any lib from the list\n"
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

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);

  if(argc < 2) {
    printf("Usage: %s {info} [lib/tool]\n", argv[0]);
    printf("where lib/tool could be any of (you may put multiple):\n");
           "libs: arena, view, nob\n");
           "tools: 4coder, focus, raddbg, odin\n");
           "Info is optional, It will tell you a short description of any lib from the list\n");
           "You may also do: download.bat [alllibs/alltools]\n");
           "To get all libs/tools at once\n");
    return 0;
  }

  if(system(where("wget"))) {
    // couldn't find wget, try to use curl
    if(system(where("curl"))) {
      fprintf(stderr, "Could not find wget or curl, please install at least one of the two\n");
      nob_cmd_free(cmd);
      return 1;
    }
    downloader[0] = "curl"; downloader[1] = "-o";
  }

  bool toDownload[DOWNLOAD_count] = {0};
  bool showInfo = true;
  bool noMakeMain = false;

  for(int i = 1; i < argc; i++)
  {
    char *arg = argv[i];
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
  for(int i = 0; i < ARRAY_LEN(toDownload); i++) if(toDownload[i]) onlyInfo = false;

  if(onlyInfo) {
    ShowDetailedInfo();
    return 0;
  }

  if(showInfo) {
    if(toDownload[DOWNLAD_arena]) {
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
    bool makeMain = (toDownload[DOWNLOAD_arena]|toDownlaod[DOWNLOAD_view]|
                     toDownload[DOWNLOAD_nob]) && !noMakeMain;
    Nob_String_Builder mainBuilder = {0};
    if(makeMain) {
      if(!nob_mkdir_if_not_exists("src")) return 1;
      time_t rawtime;
      time(&rawtime);
      nob_sb_append_cstr(&mainBuilder, "/* ");
      nob_sb_append_cstr(&mainBuilder, asctime(localtime(&rawtime)));
      nob_sb_append_cstr(&mainBuilder, " */\n");
    }

    if(toDownload[DOWNLOAD_arena]) {
      printf("downloading arena implementation...\n");
      nob_cmd_append(&cmd, DOWNLOADER, "src/arena.h", "https://raw.githubusercontent.com/tsoding/arena/refs/heads/master/arena.h");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      nob_sb_append_cstr(&mainBuilder, "#define ARENA_IMPLEMENTATION\n"
                         "#include \"arena.h\"\n");
    }
    if(toDownload[DOWNLOAD_nob]) {
      printf("downloading nob.h...\n");
      nob_cmd_append(&cmd, DOWNLOADER, "src/nob.h", "https://raw.githubusercontent.com/tsoding/nob.h/refs/heads/main/nob.h");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      printf("making nob.c file...\n");
      char *nobCFileData =
        "#define NOB_REBUILD_URSELF(binpath, srcpath) \"gcc\", \"-Wall\", \"-Wextra\", \"-o\", binpath, srcpath\n"
        "#define NOB_IMPLEMENTATION\n"
        "\n"
        "int main(int argc, char **argv)\n"
        "{\n"
        "  NOB_GO_REBUILD_URSELF(argc, argv)\n"
        "  return 0;\n"
        "}\n";
      if(!nob_write_entire_file("src/nob.c", nobCFileData, strlen(nobCFileData))) return 1;
    }
    if(toDownload[DOWNLOAD_view]) {
      printf("downloading String_View implementation...\n");
      nob_cmd_append(&cmd, DOWNLOADER, "src/view.h", "https://raw.githubusercontent.com/tsoding/sv/refs/heads/master/sv.h");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      nob_sb_append_cstr(&mainBuilder, "#define SV_IMPLEMENTATION\n"
                         "#include \"sv.h\"\n");
    }

    if(makeMain) {
      nob_sb_append_cstr(&mainBuilder, "int main()\n{\n  return 0;\n}\n");
      if(!nob_write_entire_file("src/main.c", sb.items, sb.count)) return 1;
    }

    assert(!"WARNING: probably very buggy, do not use, I need to test this.");
    // This commit is to be able to test stuff on windows

    if(toDownload[DOWNLOAD_4coder]) {
#ifndef _WIN32
      printf("You need a couple of libs to build 4coder, if you're not sure you have them, Ctrl-C out and try to install them\n"
             "make sure to have them installed before rerunning this program:\n"
             "build-essential libx11-dev libxfixes-dev libglx-dev mesa-common-dev libasound2-dev libfreetype-dev libfontconfig-dev\n");
             "You can also download 4coder prebuilt, from: https://mr-4th.itch.io/4coder\n");
      char c;
      do {
        printf("\nDo you want to continue? [Y/n]");
        c = getchar();
      } while(c != 'y' && c != 'Y' && c != 'n');
      if(c == 'n') return 0;
#endif

      nob_cmd_append(&cmd, DOWNLOADER, "4coder.zip", "https://github.com/4coder-archive/4coder/archive/refs/heads/master.zip");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      nob_cmd_append(&cmd, DOWNLOADER, "4coder-non-source.zip", "https://github.com/4coder-archive/4coder-non-source/archive/refs/heads/master.zip");
      if(!nob_cmd_run_sync_and_reset(&cmd)) return 1;
      system(unzip("4coder.zip", "4coder"));
      system(unzip("4coder-non.source.zip", "4coder"));
      nob_delete_file("4coder.zip");
      nob_delete_file("4coder-non-source.zip");
      ChangeDirectory("4coder"));
      nob_rename("4coder-master", "code");
      nob_rename("4coder-non-source-master", "4coder-non-source");
      ChangeDirectory("code");
#if _WIN32
      system("bin/build_optimized.bat");
      ChangeDirectory("4coder/code");
      system("bin/build_optimized.bat");
#else
      system("bin/build-linux.sh"); // todo: only do this when linux, call mac build otherwise
#endif
      ChangeDirectory("../..");
      if(!nob_copy_directory_recursively("4coder/4coder-non-source/dist_files", "4coder/build")) return 1;
      if(!nob_copy_directory_recursively("4coder/code/ship_files", "4coder/build")) return 1;
    }


  }

  return 0;
}