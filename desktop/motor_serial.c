#include "motor_serial.h"

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

  tty.c_cflag |= CS8;      // 8-bit characters

  tty.c_cflag &= ~CSTOPB;   // only need 1 stop bit

  tty.c_cflag |= PARENB;   // Parity
  tty.c_cflag &= ~PARODD;  // Even

  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  tty.c_iflag |= INPCK;

  tty.c_cflag &= ~CRTSCTS;  // no hardware flowcontrol

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // no software flowcontrol

  tty.c_oflag &= ~OPOST; // raw output

  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 0;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    printf("Error from tcsetattr: %s\n", strerror(errno));
    return -1;
  }
  printf("Motor serial opened\n");
  return 0;
}

int motor_serial_write(const uint8_t *data, int length) {
  // fprintf(stdout, "Out: ");
  // int i = 0;
  // for (i = 0; i < length; ++i) fprintf(stdout, "%02X", data[i]);
  // if (i > 0 && data[i - 1] != '\n') fprintf(stdout, "\n");
  int written = write(fd, data, length);
  if (written != length) {
    printf("Error from write: %d, %d\n", length, errno);
  }
  tcdrain(fd);

  return written;
}

int motor_serial_read(uint8_t *buffer, int max_length) {
  int length = read(fd, buffer, max_length);
  // fprintf(stdout, "In : ");
  // if (length != -1) {
  //   for (int i = 0; i < length; ++i) fprintf(stdout, "%02X", buffer[i]);
  // }
  // if (length > 0) fprintf(stdout, "\n");
  return length;
}

int motor_serial_open() {
  const char *portname = "/dev/tty.usbserial-FTHFP6SH";
  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK);
  if (fd < 0) {
    fprintf(stderr, "Error opening %s: %s\n", portname, strerror(errno));
  }

  if (fd >= 0) {
    set_interface_attribs(B9600);
  }
  return fd >= 0;
}
