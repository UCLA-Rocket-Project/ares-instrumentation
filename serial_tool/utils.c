#include "serial_handler.h"
#include "utils.h"

int fd = -1;

int open_port(char* serial_path, int speed) {
  fd = open(serial_path, O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1) {
    //if (verbose) printf("Could not have opened the port: %s!\n", serial_path);
    status->pipe_error = PIPE_COULD_NOT_OPEN_ERROR;
    return -1;
  } 
  fcntl(fd, F_SETFL, FNDELAY);

  struct termios options;
  tcgetattr(fd, &options);
  if (speed > 0) {
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);
  }

  options.c_cflag |= (CLOCAL | CREAD);
  tcsetattr(fd, TCSANOW, &options);

  //tcgetattr(fd, &options);
  status->pipe_error = 0; // indicate success
  return fd;
}

int wait_on_data(int fd, uint64_t millisec) {
  struct pollfd fds;
  int retval;

  fds.fd = fd;
  fds.events = POLLIN | POLLRDNORM | POLLPRI;

  //uint64_t t1 = millis();
  retval = poll(&fds, 1, millisec);
  //printf("%lu millisec elapsed\n", millis() - t1);
  if (retval == -1) {
    //printf("An error occurred with select()\n");
  }
  return retval;
}

void imprint_current_time(char* s) {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  sprintf(s, "%d/%d/%d UTC %02d:%02d:%02d:%03d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (int)(millis() % 1000));
}

size_t parse(char* buffer, size_t buff_len) {
  size_t i, j;
  char line[M];
  for (i = 0, j = 0; i < buff_len; i++) {
    if (buffer[i] == '\n') {
      memcpy(line, buffer+j, i-j);
      line[i-j] = 0;
      if (i-j > 0) {
        double curr_time = ((double)micros())/1000000.0;
        if (use_mysql) {
#if USE_MYSQL==1
          tail_str_list_mysql = add_str_node(tail_str_list_mysql, line, curr_time);
          atomicAdd32(&str_list_size_mysql, 1);
#endif
        }

        if (use_server) {
          tail_str_list_server = add_str_node(tail_str_list_server, line, curr_time);
          atomicAdd32(&str_list_size_server, 1);
        }

        if (use_log) {
          tail_str_list_log = add_str_node(tail_str_list_log, line, curr_time);
          atomicAdd32(&str_list_size_log, 1);
        }
        strncpy(status->values.text, line, sizeof(status->values.text));
        status->values.text[sizeof(status->values.text)-1] = 0;
        status->values.time = curr_time;
      }
      j = i + 1;
    }
  }
  return j;
}

void* obtain_data(void* arg) {

  /* start serial reception */
  fd = open_port(serial_path, serial_speed);

  char line[M];
  char buffer[N<<2];
  size_t buff_len = 0;
  line[0] = 0;
  uint64_t bytes_written = 0;
  //unsigned writing_occurred = 0;
  while (running) {
    /* attempt to open socket when closed */
    //char socket_neg = ((fd == -1) ? 1 : 0);
    //char write_wrong = ((write(fd, "", 0) < 0) ? 1 : 0);
    char socket_neg = 0;
    //char write_wrong = 0;
    if (fd == -1) socket_neg = 1;
    else socket_neg = 0;
    //if (write(fd, "", 0) < 0) write_wrong = 1;
    //else write_wrong = 0;

    if (socket_neg) { //socket is closed 
      close(fd);
      fd = open_port(serial_path, serial_speed);
      if (verbose) {
        //if (socket_neg) printf("Socket positive test failed\n");
        //if (write_wrong) printf("Zero write test failed\n");
        //printf("##Re-opening port\n");
        status->pipe_error = PIPE_CLOSED_ERROR;
      }   
      delay(100);
      /* read data from open socket */
    } else {
      wait_on_data(fd, 1000);
      int read_bytes = read(fd, line, M-1);
      if (read_bytes > 0) {
        memcpy(buffer+buff_len, line, read_bytes);
        buff_len += read_bytes;
        if (buff_len > N<<2)
          buff_len = N<<2;
        buffer[buff_len] = 0;
        int j = parse(buffer, buff_len);
        memmove(buffer, buffer+j, buff_len - j);
        buff_len = buff_len - j;
        buffer[buff_len] = 0;
        bytes_written += j;

        line[read_bytes] = 0;
        //if (verbose) printf("%s", line);
      } else if (read_bytes == 0) {
        // WEIRD CASE
      } else {
        // NO DATA CASE 
      }
    }
  }
  //printf("EXITING SERIAL OBTAINING THREAD\n");
  return NULL;
}
