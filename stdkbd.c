#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

int fd;

void cleanup() {
  printf("Cleaning up...\n");
  ioctl(fd, UI_DEV_DESTROY);
  close(fd);
}

int main() {
  // set up signal handler for cleanup
  struct sigaction new_action;
  new_action.sa_handler = cleanup;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_flags = 0;

  sigaction (SIGINT, &new_action, NULL);
  sigaction (SIGHUP, &new_action, NULL);
  sigaction (SIGTERM, &new_action, NULL);


  /* Set up a fake keyboard device */
  fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK); // or /dev/input/uinput
  ioctl(fd, UI_SET_EVBIT, EV_KEY);
  ioctl(fd, UI_SET_EVBIT, EV_SYN);
  for (int i = KEY_1; i <= KEY_0; i++) {
    ioctl(fd, UI_SET_KEYBIT, i);
  }
  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));
  uidev = (struct uinput_user_dev) {
    .id = {
      .bustype = BUS_USB,
      .vendor = 0x1,
      .product = 0x1,
      .version = 1
    },
  };
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "stdkbd");

  write(fd, &uidev, sizeof(uidev));
  if (ioctl(fd, UI_DEV_CREATE) < 0) {
    printf("Device creation failed.\n");
    return 1;
  }
  printf("Device created.\n");
  sleep(2);
  bool abort = false;

  while (!abort) {
    struct input_event ev;
    memset(&ev, 0, sizeof(ev));
    char c = getchar();
    uint16_t keycode;
    bool ignore = false;

    switch (c) {
      case '0':
        keycode = KEY_0;
        break;
      case '1':
        keycode = KEY_1;
        break;
      case '2':
        keycode = KEY_2;
        break;
      case '3':
        keycode = KEY_3;
        break;
      case '4':
        keycode = KEY_4;
        break;
      case '5':
        keycode = KEY_5;
        break;
      case '6':
        keycode = KEY_6;
        break;
      case '7':
        keycode = KEY_7;
        break;
      case '8':
        keycode = KEY_8;
        break;
      case '9':
        keycode = KEY_9;
        break;
      case '\n':
        ignore = true;
        break;
      default:
        abort = true;
        break;
    }
    if (abort) {
      break;
    }
    if (ignore) {
      continue;
    }


    ev = (struct input_event) {
      .type = EV_KEY,
      .code = keycode,
      .value = 1
    };
    if (write(fd, &ev, sizeof(ev)) < 0) {
      printf("write to device failed.\n");
      return 1;
    };

    ev.value = 0;
    if (write(fd, &ev, sizeof(ev)) < 0) {
      printf("write to device failed.\n");
      return 1;
    }

    // report event packet to "flush" device
    ev = (struct input_event) {
      .type = EV_SYN,
      .code = SYN_REPORT
    };
    if (write(fd, &ev, sizeof(ev)) < 0) {
      printf("write to device failed.\n");
      return 1;
    };
  }
  cleanup();

}
