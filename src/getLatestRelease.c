#define NOB_IMPLEMENTATION
#include "nob.h"

bool cstrStartsWith(const char *str, const char *pre)
{
  size_t lenpre = strlen(pre);
  size_t lenstr = strlen(str);
  return (lenstr >= lenpre) && (memcmp(pre, str, lenpre) == 0);
}

#if _WIN32
#define where(command) "where /Q "command
#else
#define where(command) "type "command" >/dev/null 2>&1"
#endif

int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);

  if(argc < 3) {
    printf("Usage: %s githubProject fileToInstall\n", argv[0]);
    printf("This program installs the latest release of a project from github\n");
    return 0;
  }

  char *githubProject = argv[1];
  char *fileToInstall = argv[2];
  Nob_Cmd cmd = {0};
  char *downloader[2] = {"wget", "-O"};
  if(system(where("wget"))) {
    // couldn't find wget, try to use curl
    if(system(where("curl"))) {
      fprintf(stderr, "Could not find wget or curl, please install at least one of the two\n");
      nob_cmd_free(cmd);
      return 1;
    }
    downloader[0] = "curl"; downloader[1] = "-o";
  }

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

  return 0;
}
