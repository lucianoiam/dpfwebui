// some useful code left from the proof of concept
#include <syslog.h>
#include <spawn.h>
extern char **environ;

char strWindowId[sizeof(uintptr_t) + /* 0x + \0 */ 3];
sprintf(strWindowId, "%lx", (long)windowId);
pid_t pid;
const char *argv[] = {"helper", strWindowId, CONTENT_URL, NULL};
const char* fixmeHardcodedPath = "/home/user/src/dpf-webui/bin/d_dpf_webui_helper";
int status = posix_spawn(&pid, fixmeHardcodedPath, NULL, NULL, (char* const*)argv, environ);
syslog(LOG_INFO, "posix_spawn() status %d\n", status);
