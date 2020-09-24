#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef WINDOWS
  #include <process.h>
  #include <windows.h>
  #include <shlwapi.h>

  // Use POSIX helpers function on Windows
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/putenv-wputenv?view=vs-2019
  #define putenv _putenv
#else
  // execv and friends on POSIX
  #include <unistd.h>
#endif

int main(int argc, char **argv)
{
  char *desktopPath;
  char *desktopAskPassScriptPath;
  int err = 0;

  if (argc < 2)
  {
    fprintf(stderr, "USAGE: ask-pass-trampoline PROMPT\n");
    return 1;
  }

  desktopPath = getenv("DESKTOP_PATH");
  desktopAskPassScriptPath = getenv("DESKTOP_ASKPASS_SCRIPT");

  if (desktopPath == NULL || desktopAskPassScriptPath == NULL)
  {
    fprintf(stderr, "ERROR: Missing DESKTOP_PATH or DESKTOP_ASKPASS_SCRIPT environment variables\n");
    return 1;
  }

  putenv("ELECTRON_RUN_AS_NODE=1");
  putenv("ELECTRON_NO_ATTACH_CONSOLE=1");

  #if WINDOWS
    // I don't know what I'm doing!
    char quotedDesktopPath[MAX_PATH];
    strcpy(quotedDesktopPath, desktopPath);
    PathQuoteSpaces(quotedDesktopPath);

    char quotedDesktopAskPassScriptPath[MAX_PATH];
    strcpy(quotedDesktopAskPassScriptPath, desktopAskPassScriptPath);
    PathQuoteSpaces(quotedDesktopAskPassScriptPath);

    err = _spawnl(_P_WAIT, desktopPath, quotedDesktopPath, desktopAskPassScriptPath, argv[1], NULL);
  #else
    err = execl(desktopPath, desktopPath, desktopAskPassScriptPath, argv[1], NULL);
  #endif

  if (err != 0) {
    fprintf(stderr, "ERROR: Failed to launch \"%s\": %s\n", desktopPath, strerror(errno));
    return 1;
  }
}
