#include <libnotify/notification.h>
#include <libnotify/notify.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EXT_SUCCESS 0
#define EXT_ERROR 1
#define EXT_ERROR_NOTIFY 2
#define EXT_ERROR_WATCH 3

const struct inotify_event *notifyEvent;

int main(int argc, char **argv) {

  const char *NAME = "Watchdog";

  const uint32_t WATCH_MASK = IN_CREATE | IN_DELETE | IN_ACCESS |
                              IN_CLOSE_WRITE | IN_MODIFY | IN_MOVE_SELF;

  // We need the file path
  if (argc < 2) {
    fprintf(stderr, "Missing path to file.");
    exit(EXT_ERROR);
  }

  // Extract file name only
  char *filePath = (char *)malloc(sizeof(argv[1] + 1));
  strcpy(filePath, argv[1]);

  char *token = strtok(filePath, "/");
  while (token != NULL) {
    filePath = token;
    token = strtok(NULL, "/");
  }

  // Init notification for desktop
  if (!notify_init(NAME)) {
    fprintf(stderr, "Failed to init notify");
    exit(EXT_ERROR_NOTIFY);
  }

  // Notifications from kernel
  int inotifyWatch = inotify_init();

  if (inotifyWatch == -1) {
    fprintf(stderr, "Failed to init notify");
    exit(EXT_ERROR_NOTIFY);
  }

  int inotifyStatus = inotify_add_watch(inotifyWatch, argv[1], WATCH_MASK);

  if (inotifyStatus == -1) {
    fprintf(stderr, "Fail to watch file");
    exit(EXT_ERROR_WATCH);
  }

  char buffer[4096];
  char *message = NULL;

  while (1) {
    printf("Waiting for event...\n");

    int readLen = read(inotifyWatch, buffer, sizeof(buffer));

    if (readLen == -1) {
      fprintf(stderr, "Fail to watch file");
      exit(EXT_ERROR_WATCH);
    }

    for (char *buffPointer = buffer; buffPointer < buffer + readLen;
         buffPointer += sizeof(struct inotify_event) + notifyEvent->len) {

      notifyEvent = (const struct inotify_event *)buffPointer;

      message = NULL;

      if (notifyEvent->mask & IN_ACCESS)
        message = "File accessed!";

      if (notifyEvent->mask & IN_MODIFY)
        message = "File modified!";

      if (message == NULL)
        continue;

      printf("%s\n", message);

      notify_notification_show(
          notify_notification_new(filePath, message, "dialog-information"),
          NULL);
    }
  }
}
