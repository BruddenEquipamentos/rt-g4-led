#include "ble_serial.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static int fd = 0;

static int set_interface_attribs(int speed) {
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    printf("Error from tcgetattr: %s\n", strerror(errno));
    return -1;
  }

  cfsetospeed(&tty, (speed_t)speed);
  cfsetispeed(&tty, (speed_t)speed);

  tty.c_cflag |= (CLOCAL | CREAD);  // ignore modem controls
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;       // 8-bit characters
  tty.c_cflag &= ~PARENB;   // no parity bit
  tty.c_cflag &= ~CSTOPB;   // only need 1 stop bit
  tty.c_cflag &= ~CRTSCTS;  // no hardware flowcontrol

  tty.c_iflag &=
      ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 1;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    printf("Error from tcsetattr: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int ble_serial_write(const uint8_t *data, int length) {
  printf("out: ");
  for (int i = 0; i < length; ++i) printf("%02X", data[i]);
  printf("\n");

  int written = write(fd, data, length);
  if (written != length) {
    printf("Error from write: %d, %d\n", length, errno);
  }
  tcdrain(fd);
  return written;
}

int ble_serial_read(uint8_t *buffer, int max_length) {
  return read(fd, buffer, max_length);
}

int ble_serial_open() {
  const char *portname = "/dev/tty.usbmodemL10021951";
  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "Error opening %s: %s\n", portname, strerror(errno));
  }

  if (fd >= 0) {
    set_interface_attribs(B115200);
  }

  return fd >= 0;
}
