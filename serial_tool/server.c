#include "server.h"

void* server(void* arg) {
#define BODY_SIZE (1<<20)
  char* body = (char*)malloc(sizeof(char)*BODY_SIZE);

  int servSock;
  int clntSock;
  struct sockaddr_in echoServAddr;
  struct sockaddr_in echoClntAddr;
  unsigned short echoServPort;
  unsigned int clntLen;

  echoServPort = server_port;
  servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (servSock < 0) {
    status->server_error = SERVER_SOCKET_CREATE_ERROR;
    //die("Could not create the socket");
    return NULL;
  }

  memset(&echoServAddr, 0, sizeof(echoServAddr));
  echoServAddr.sin_family = AF_INET;
  echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  echoServAddr.sin_port = htons(echoServPort);

  int True = 1;
  setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(True));
  int b = bind(servSock, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr));
  if (b < 0) {
    status->server_error = SERVER_SOCKET_BIND_ERROR;
    //die("Could not bind to the socket");
    return NULL;
  }

#define MAXPENDING 10
  int l = listen(servSock, MAXPENDING);
  if (l < 0) {
    status->server_error = SERVER_LISTEN_ERROR;
    //die("listen failed");
    return NULL;
  }

  struct pollfd pollServSock;
  memset(&pollServSock, 0, sizeof(pollServSock));
  pollServSock.fd = servSock;
  pollServSock.events = POLLIN | POLLPRI;
  while (running == 1) {
    str_node* last_node;
    str_node* curr_node;
    curr_node = head_str_list_server;

    uint64_t now = millis();
    double recent_limit = ((double)(now - 10000.0))/1000.0;
    while (curr_node->next != NULL) {
      last_node = curr_node; 
      curr_node = curr_node->next;
      head_str_list_server = curr_node;
      free(last_node);
      atomicAdd32(&str_list_size_server, -1);

      if (curr_node->time > recent_limit) break;
    }

    clntLen = sizeof(echoClntAddr);
    
    int p = poll(&pollServSock, 1, 1000);

    if (p > 0) {
      int body_size = build_a_recent_list(body, BODY_SIZE-1, &head_str_list_server, 10000);
      clntSock = accept(servSock, (struct sockaddr*)&echoClntAddr, &clntLen);
      if (clntSock < 0) {
        status->server_error = SERVER_ACCEPT_ERROR;
        //die("accept failed");
        return NULL;
      }

      if (fcntl(clntSock, F_SETFL, fcntl(clntSock, F_GETFL) | O_NONBLOCK) < 0) {
        status->server_error = SERVER_NON_BLOCKING_ERROR;
        //die("Chenaging socket to non-blocking failed");
        return NULL;
      }
      handleClntSock(clntSock, body, body_size);
    }
    //printf("Looping\n");
  }

  //printf("Exiting the SERVER PTHREAD\n");
  free_str_nodes(head_str_list_server);
  head_str_list_server = NULL;
  free(body);
  status->server_error = CLEAN_EXIT_ERROR;
  return NULL;
}

void die(char* s) {
  printf("%s\n", s);
  pthread_exit(0);
}

void handleClntSock(int clntSock, char* body, size_t body_size) {
  int sendbuf_size = body_size;
  setsockopt(clntSock, SOL_SOCKET, SO_SNDBUF, &sendbuf_size, sizeof(sendbuf_size));

#define BUFSIZE (1<<20)
  char* recvBuffer = (char*)malloc(sizeof(char)*BUFSIZE);
  memset(recvBuffer, 0, BUFSIZE);
  int recvMsgSize;

  recvMsgSize = recv(clntSock, recvBuffer, BUFSIZE-1, 0);
  //printf("Recv size is %d\n", recvMsgSize);
  //printf("Body size is %d\n", body_size);
  recvBuffer[recvMsgSize] = 0;
  if (recvMsgSize == 0) {
    status->server_error = SERVER_RECEIVE_ZERO_ERROR;
    //die("message has zero size"); 
    pthread_exit(0);
  }

  while (recvMsgSize > 0) {
    //printf("%s", recvBuffer);
    //int s = send(clntSock, recvBuffer, recvMsgSize, 0);
    //if (s != recvMsgSize) die("sent not the same nb of bytes");

    recvMsgSize = recv(clntSock, recvBuffer, BUFSIZE-1, 0);
    //printf("Recv size is %d\n", recvMsgSize);
    delay(100);
    recvBuffer[recvMsgSize] = 0;
    //if (recvMsgSize < 0) die("recvMsgSize is less than zero");
  }

  //printf("Last characters are:\n %s\n", body+(body_size-50));

  //printf("body_size = %d\n", body_size);
  //int packet_size = 1024;
  //int i = 0;
  //while (body_size > packet_size) {
    //send(clntSock, body + (i * packet_size), packet_size, 0);
    //body_size -= packet_size;
    //i++;
  //} 
  //send(clntSock, body + (i * packet_size), body_size, 0);
  send(clntSock, body, body_size, 0);
  //if (s != recvMsgSize) die("sent not the same nb of bytes");
  //printf("Closing socket\n");
  close(clntSock);
  free(recvBuffer);
  (status->server_requests)++;
}

size_t build_a_recent_list(char* body, size_t body_n, str_node** str_list, uint64_t milliseconds) {
  if (*str_list == NULL) return 0;
  if (body_n < 256) return 0;

  memset(body, 0, body_n*sizeof(char));
  size_t body_size = 0;

  sprintf(body, "%s,%s\n", "abs_t", model);
  body_size = strlen(body);

  str_node* last_node;
  str_node* curr_node;
  curr_node = *str_list;

  uint64_t now = millis();
  double recent_limit = ((double)(now - milliseconds))/1000.0;
  while (curr_node->next != NULL) {
    last_node = curr_node; 
    curr_node = curr_node->next;
    *str_list = curr_node;
    free(last_node);
    atomicAdd32(&str_list_size_server, -1);

    if (curr_node->time > recent_limit) break;
  }

  char** model_fields;
  int model_data_n = parse_CSV(model, &model_fields);

  double period = 1.0 / 100.0;
  char temp[512];
  while (curr_node->next != NULL && (body_size + 256) < body_n) {
    curr_node = curr_node->next;
    if (curr_node->time > recent_limit + period) {
      char** line_fields;
      int line_fields_n = parse_CSV(curr_node->text, &line_fields);
      if (line_fields_n == model_data_n) {
        sprintf(temp, "%f,%s\n", curr_node->time, curr_node->text);
        int temp_len = strlen(temp);
        memcpy(body+body_size, temp, temp_len+1);
        body_size += temp_len;
      }
      free_CSV_fields(line_fields, line_fields_n);
      recent_limit += period;
    }
  }
  //printf("%s", body);

  free_CSV_fields(model_fields, model_data_n);
  return body_size;
}

void print_server_time(char* date) {
  time_t t = time(NULL); 
  struct tm tm = *gmtime(&t);
  //char* date = (char*)malloc(sizeof(char)*64);
  char* temp = (char*)malloc(sizeof(char)*64);
  switch (tm.tm_wday) {
    case 0: sprintf(date, "%s, ", "Sun"); break;
    case 1: sprintf(date, "%s, ", "Mon"); break;
    case 2: sprintf(date, "%s, ", "Tue"); break;
    case 3: sprintf(date, "%s, ", "Wed"); break;
    case 4: sprintf(date, "%s, ", "Thu"); break;
    case 5: sprintf(date, "%s, ", "Fri"); break;
    case 6: sprintf(date, "%s, ", "Sat"); break;
  }
  sprintf(temp, "%d ", tm.tm_mday);
  strcat(date, temp);
  switch (tm.tm_mon) {
    case 0: strcpy(temp, "Jan"); break;
    case 1: strcpy(temp, "Feb"); break;
    case 2: strcpy(temp, "Mar"); break;
    case 3: strcpy(temp, "Apr"); break;
    case 4: strcpy(temp, "May"); break;
    case 5: strcpy(temp, "Jun"); break;
    case 6: strcpy(temp, "Jul"); break;
    case 7: strcpy(temp, "Aug"); break;
    case 8: strcpy(temp, "Sep"); break;
    case 9: strcpy(temp, "Oct"); break;
    case 10: strcpy(temp, "Nov"); break;
    case 11: strcpy(temp, "Dec"); break;
  }
  strcat(date, temp);

  sprintf(temp, " %d %d:%d:%d GMT", 1900 + tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
  strcat(date, temp);

  //printf("%s\n", date);
  //free(date);
  free(temp);
}
