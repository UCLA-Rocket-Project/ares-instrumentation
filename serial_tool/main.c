#include "serial_handler.h"
#include "main.h"

volatile int running = 1; //very much global
volatile int verbose = 1; //all, must be global
//char model[1024] = "{rel_t:1,a0:0,a1:0,a2:0,a3:0,a4:0,a5:0}";
char model[1024] = "rel_t,a0,a1,a2,a3,a4,a5";
char mysql_addr[256] = "127.0.0.1";
char mysql_login[256] = "";
char mysql_pswd[256] = "";
char mysql_table[256] = "";
char mysql_db[256] = "";
char path_to_log_dir[4096]; //used by main and data, could pass, but nah
soc_node* head_soc_list = NULL;
soc_node* tail_soc_list = NULL;
str_node* head_str_list_mysql = NULL;
str_node* tail_str_list_mysql = NULL;
str_node* head_str_list_server = NULL;
str_node* tail_str_list_server = NULL;
str_node* head_str_list_log = NULL;
str_node* tail_str_list_log = NULL;
int32_t str_list_size_mysql = 0;
int32_t str_list_size_server = 0;
int32_t str_list_size_log = 0;
status_struct* status;
char use_log = 1;
char use_mysql = 1;
char use_server = 1;

char serial_path[2048];
int serial_speed;
int use_host = 0;
char host[2048];
int port = 8888;
int server_port = 8888;

graph* main_graph;

int main(int argc, char* argv[]) {
  status = malloc(sizeof(status_struct));
  /* ------------------------------------------------------------------------*/
  status->mysql_t = millis();
  status->server_t = millis();
  status->log_t = millis();
  status->mysql_nodes = 0;
  status->server_requests = 0;
  status->log_nodes = 0;
  status->mysql_error = 0;
  status->server_error = 0;
  status->log_error = 0;
  status->pipe_error = 0;
  status->values.text[0] = 0;
  status->values.time = millis();
  status->values.next = NULL;
  /* ------------------------------------------------------------------------*/

  /* ------------------------------------------------------------------------*/
  head_str_list_mysql = add_str_node(head_str_list_mysql, "dummy", 0.0);
  tail_str_list_mysql = head_str_list_mysql;
  str_list_size_mysql++;
  head_str_list_server = add_str_node(head_str_list_server, "dummy", 0.0);
  tail_str_list_server = head_str_list_server;
  str_list_size_server++;
  head_str_list_log = add_str_node(head_str_list_log, "dummy", 0.0);
  tail_str_list_log = head_str_list_log;
  str_list_size_log++;
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  strncpy(serial_path, PORT_NAME, sizeof(serial_path));
  serial_path[sizeof(serial_path)-1] = 0;
  serial_speed = PORT_SPEED;
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  strcpy(path_to_log_dir, "./");
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  /* check for help message flag */
  if (argc > 1) {
    int i;
    for (i = 1; i < argc; i++)
      if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
        print_help_message();
        return 0;
      }
  }
  /* ------------------------------------------------------------------------*/

  /* ------------------------------------------------------------------------*/
  char is_terminal = isatty(fileno(stdin));

  char command[256];
  command[0] = 0;
  int command_len = 1;
  printf("serial_tool> ");
  fgets(command, 255, stdin);
  command_len = strlen(command);
  command[command_len-1] = 0;
  command_len--;
  while (!feof(stdin) && strcmp(command, "done") != 0) {
    char* field = command;
    char* value = strchr(command, '=');
    if (value == NULL)
      value = command+254;
    value[0] = 0;
    value += 1;
    value = trim_whitespace(value);
    field = trim_whitespace(field);

    process_command(field, value);

    //printf("You've typed: field=%s and value=%s\n", field, value);
    if (is_terminal)
      printf("serial_tool> ");
    fgets(command, 255, stdin);
    command_len = strlen(command);
    if (command_len > 0)
      command[command_len-1] = 0;
    else
      command[0] = 0;
    command_len--;
  }
  printf("\n");
  print_configuration();

  command[0] = 0;
  while (!feof(stdin) && command[0] != '\n')
    fgets(command, 255, stdin);

  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  /* parse arguments */
  if (argc > 1) {
    int i;
    for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-F") == 0 || strcmp(argv[i], "-l") == 0) {
        if (i + 1 < argc) {
          if (strlen(argv[i+1]) < sizeof(path_to_log_dir)) {
            strncpy(path_to_log_dir, argv[i+1], 4096);
            path_to_log_dir[4096-2] = 0;
            size_t ptl_len = strlen(path_to_log_dir);
            if (path_to_log_dir[ptl_len-1] != '/') {
              path_to_log_dir[ptl_len] = '/';
              path_to_log_dir[ptl_len+1] = 0;
            }
          }
          i++;
        }
      } else if (strcmp(argv[i], "-H") == 0) {
        if (i + 1 < argc) {
          char* port_str = strchr(argv[i+1], ':');
          if (port_str == NULL)
            port = 8888;
          else {
            *(port_str) = 0;
            port = atoi(port_str+1);
            if (port < 0)
              port = 8888;
          }

          if (strlen(argv[i+1]) < sizeof(host)) {
            strcpy(host, argv[i+1]);
            use_host = 1;
          }
          i++;
        }
      } else if (strcmp(argv[i], "-P") == 0) {
        if (i + 1 < argc) {
          if (strlen(argv[i+1]) < sizeof(serial_path))
            strcpy(serial_path, argv[i+1]);
          i++;
        }
      } else if (strcmp(argv[i], "-S") == 0) {
        if (i + 1 < argc) {
          serial_speed = atoi(argv[i+1]);
          if (serial_speed < 0)
            serial_speed = PORT_SPEED;
          i++;
        }
      } else if (strcmp(argv[i], "-m") == 0) {
        if (i + 1 < argc) {
          char** fields;
          int n = parse_CSV(argv[i+1], &fields);
          if (n > 0) {
            strncpy(model, argv[i+1], 1024);
            model[1024-1] = 0;
          }
          free_CSV_fields(fields, n);
          i++;
        }
      } else if (strcmp(argv[i], "--mysql-password") == 0) {
        if (i + 1 < argc) {
          strncpy(mysql_pswd, argv[i+1], 256);
          mysql_pswd[256-1] = 0;
          i++;
        }
      } else if (strcmp(argv[i], "--mysql-login") == 0) {
        if (i + 1 < argc) {
          strncpy(mysql_login, argv[i+1], 256);
          mysql_login[256-1] = 0;
          i++;
        }
      } else if (strcmp(argv[i], "--mysql-database") == 0) {
        if (i + 1 < argc) {
          strncpy(mysql_db, argv[i+1], 256);
          mysql_db[256-1] = 0;
          i++;
        }
      } else if (strcmp(argv[i], "--mysql-table") == 0) {
        if (i + 1 < argc) {
          strncpy(mysql_table, argv[i+1], 256);
          mysql_table[256-1] = 0;
          i++;
        }
      } else if (strcmp(argv[i], "--mysql-address") == 0) {
        if (i + 1 < argc) {
          strncpy(mysql_addr, argv[i+1], 256);
          mysql_addr[256-1] = 0;
          i++;
        }
      } else {
        printf("Unrecognized option: %s\n", argv[i]);
      }
    }
  }
  /* ------------------------------------------------------------------------*/

  /* ------------------------------------------------------------------------*/
  char** model_data_keys;
  int model_data_n = parse_CSV(model, &model_data_keys);
  main_graph = malloc(sizeof(graph));
  main_graph->N = 100;
  main_graph->M = model_data_n - 1;
  main_graph->x = malloc(sizeof(double)*(main_graph->N));
  main_graph->Y = malloc(sizeof(double*)*(main_graph->M));
  int iter, iter2;
  for (iter = 0; iter < main_graph->M; iter++) {
    main_graph->Y[iter] = malloc(sizeof(double)*(main_graph->N));
    for (iter2 = 0; iter2 < main_graph->N; iter2++) {
      main_graph->Y[iter][iter2] = 0.0;
    }
  }
  for (iter2 = 0; iter2 < main_graph->N; iter2++) {
    main_graph->x[iter2] = 0.0;
  }
  main_graph->minx = 0.0;
  main_graph->maxx = 0.0;
  main_graph->miny = 0.0;
  main_graph->maxx = 0.0;
  main_graph->last_updated = (double)millis() / 1000.0;

  free_CSV_fields(model_data_keys, model_data_n);
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  //change_port_permissions(serial_path);
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  /* allow for graceful quitting */
  signal(SIGINT, sighandler);
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  /* create connection to cmd line specified host */
  // struct sockaddr_in servaddr;
  int dummy_port = 8888;
  char dummy_host[] = "dummy";
  tail_soc_list = add_soc_node(tail_soc_list, dummy_host, dummy_port);
  head_soc_list = tail_soc_list;
  if (use_host) {
    //printf("Adding host: %s:%d\n", host, port);
    tail_soc_list = add_soc_node(tail_soc_list, host, port);
  }
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  //if (verbose) printf("Serial pipe \"%s\" will be used with a speed of %i\n", serial_path, serial_speed);
  //if (verbose) printf("Using model: \"%s\"\n", model);
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  pthread_t serial_handler;
  if (pthread_create(&serial_handler, NULL, obtain_data, NULL)) {
    //pthread_detach(serial_handler);
    printf("Error creating the data obtaining thread\n");
    exit(6);
  }
  pthread_t log_handler;
  if (use_log) {
    if (pthread_create(&log_handler, NULL, handle_log_data, NULL)) {
      //pthread_detach(log_handler);
      printf("Error creating the log handling thread\n");
      exit(5);
    }
  }
  pthread_t mysql_handler;
  if (use_mysql) {
#if USE_MYSQL==1
    if (pthread_create(&mysql_handler, NULL, handle_mysql_data, NULL)) {
      //pthread_detach(mysql_handler);
      printf("Error creating the mysql handling thread\n");
      exit(5);
    }
#endif
  }
  pthread_t server_handler;
  if (use_server) {
    if (pthread_create(&server_handler, NULL, server, NULL)) {
      //pthread_detach(server_handler);
      printf("Error creating the server thread\n");
      exit(7);
    }
  }
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  while (running) {
    write_status_page();
#define CRITICAL_NODES_NUMBER 20000
    if (atomicRead32(&str_list_size_mysql) > CRITICAL_NODES_NUMBER) {
      use_mysql = 0;
      free_str_nodes(head_str_list_mysql);
      head_str_list_mysql = NULL;
    }
    if (atomicRead32(&str_list_size_server) > CRITICAL_NODES_NUMBER) {
      use_server = 0;
      free_str_nodes(head_str_list_server);
      head_str_list_server = NULL;
    }
    if (atomicRead32(&str_list_size_log) > CRITICAL_NODES_NUMBER) {
      use_log = 0;
      free_str_nodes(head_str_list_log);
      head_str_list_log = NULL;
    }
    delay(200);
  }
  /* ------------------------------------------------------------------------*/


  /* ------------------------------------------------------------------------*/
  //if (verbose) printf("\nClosing...\n");
  //delay(1000);


  //printf("Closing pipe\n");
  close(fd);

  pthread_join(serial_handler, NULL);
  if (use_log) {
    pthread_join(log_handler, NULL);
  }
  if (use_mysql) {
#if USE_MYSQL==1
    pthread_join(mysql_handler, NULL);
#endif
  }
  if (use_server) {
    pthread_join(server_handler, NULL);
  }

  //printf("Freeing socket nodes\n");
  /*
  free_soc_nodes(head_soc_list);
  head_soc_list = NULL;
  free_str_nodes(head_str_list_mysql);
  head_str_list_mysql = NULL;
  free_str_nodes(head_str_list_server);
  head_str_list_server = NULL;
  free_str_nodes(head_str_list_log);
  head_str_list_log = NULL;
  */
  free(status);
  free(main_graph);
  /* ------------------------------------------------------------------------*/

  return 0;
}

soc_node* add_soc_node(soc_node* soc, char* ip, int port) {
  soc_node* soc_curr = (soc_node*)malloc(sizeof(soc_node)); 
  soc_curr->next = NULL;
  if (soc != NULL)
    soc->next = soc_curr;

  //if (verbose && soc != NULL) printf("Sending UDP packets will occur to %s on port %i\n", ip, port);
  struct hostent* hp; 

  if (strcmp(ip, "dummy") == 0) return soc_curr;

  memset((char*)&(soc_curr->servaddr), 0, sizeof(soc_curr->servaddr));
  (soc_curr->servaddr).sin_family = AF_INET;
  (soc_curr->servaddr).sin_port = htons(port);
  hp = gethostbyname(ip);
  if (!hp) {
    //if (verbose) printf("Could not have obtained host address of %s:%i!\n", ip, port);
    if (soc != NULL)
      soc->next = NULL;
    free(soc_curr);
    return NULL; 
  }
  memcpy((void*)&(soc_curr->servaddr).sin_addr, hp->h_addr_list[0], hp->h_length);
  soc_curr->fd = socket(AF_INET, SOCK_DGRAM, 0);

  return soc_curr;
}

void free_soc_nodes(soc_node* soc) {
  soc_node* last_node;
  soc_node* curr_node;
  curr_node = soc;
  while (curr_node != NULL) {
    last_node = curr_node; 
    curr_node = curr_node->next;
    close(last_node->fd);
    free(last_node);
  }
}

void print_help_message() {
  char message[] = "\nThe following options are supported:\n" \
"-h,--help          prints this help message\n" \
"-H                 specify host to which logged data is sent using UDP\n" \
"                   UDP transmissions only occur when host is specified\n" \
"                   host defaults to \"localhost:8888\"\n" \
"                   use format IP:PORT as in 192.168.1.1:8888\n" \
"-F,-l              specifies the file to which logging should occur\n" \
"-s                 silent mode, no output is produced\n" \
"-P                 specify serial pipe to be used\n" \
"                   only one pipe per process is currently supported\n" \
"-S                 specify the speed of serial\n" \
"-m                 specify model JSON string for MySql parsing\n" \
"--mysql-login      specify mysql login for the server\n" \
"--mysql-password   specify mysql password for the server\n" \
"--mysql-database   specify mysql database\n" \
"--mysql-table      specify mysql table for data\n" \
"--mysql-address    specify mysql addr for the server\n\n" \
"Notes:\n" \
"Serial pipes of USB ports are only readable on root by default, make sure" \
"to change their permissions first or execute this program as root\n\n"
"Examples:\n" \
"sudo ./exec -P \"/dev/ttyACM0\" -l \"$HOME/Desktop\" --mysql-database" \
" \"testing\"\n\n" \
"Press Ctrl + C to terminate this program safely\n";
  puts(message);
}

void sighandler(int signum) {
  running = 0;
}

void change_port_permissions(char* serial_path) {
  if (verbose) {
    printf("Changing permissions on port %s to 766.\n", serial_path);
    printf("Get ready to input your password\n");
  }
  char* cmd = (char*)malloc(sizeof(char)*(strlen(serial_path)+64));
  sprintf(cmd, "sudo chmod 766 %s", serial_path);
  system(cmd);
  free(cmd);
}

char* trim_whitespace(char* str) {
  size_t len = strlen(str);
  char* sub = str;
  int i = 0;
  while (i < len && my_isspace(sub)) {
    sub++;
    i++;
    len--;
  }
  i = len-1;
  while (i > 0 && my_isspace(sub+i)) {
    sub[i] = 0;
    i--;
  }
  return sub;
}

int my_isspace(char* c) {
  if (c == NULL)
    return 1;
  if (c[0] == '\n' || c[0] == '\t' || c[0] == ' ' || c[0] == '\r' || c[0] == '\v' || c[0] == '\f') {
    return 1;
  } else {
    return 0;
  }
}

void write_status_page() {
  char* temp = malloc(sizeof(char)*1024);
  printf("\e[1;1H\e[2J"); // clear screen
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

  switch (status->mysql_error) {
    case MYSQL_INIT_ERROR:
      sprintf(temp, RED"Error initializing"RESET);
      break;
    case MYSQL_TIMEOUT_SET_ERROR:
      sprintf(temp, RED"Error setting timeout on mysql socket"RESET);
      break;
    case MYSQL_CONNECT_ERROR:
      sprintf(temp, RED"Error connecting"RESET);
      break;
    case MYSQL_CREATE_DB_ERROR:
      sprintf(temp, RED"Error creating database"RESET);
      break;
    case MYSQL_USE_DB_ERROR:
      sprintf(temp, RED"Error using database"RESET);
      break;
    case MYSQL_CREATE_TABLE_ERROR:
      sprintf(temp, RED"Error creating table"RESET);
      break;
    case LIST_NULL_ERROR:
      sprintf(temp, RED"Error, list passed was NULL"RESET);
      break;
    case CLEAN_EXIT_ERROR:
      sprintf(temp, YELLOW"Exited cleanly"RESET);
      break;
    case MYSQL_INSERTION_ERROR:
      sprintf(temp, RED"Error inserting data"RESET);
      break;
    default:
      sprintf(temp, GREEN"No problem"RESET);
      break;
  }
#if USE_MYSQL==1
  if (use_mysql) {
    printf("MYSQL status:         %s\n", temp);
    printf("MYSQL last update:    %f\n", (double)(millis() - status->mysql_t) / 1000.0);
    printf("MYSQL analyzed nodes: %d\n", status->mysql_nodes);
    printf("MYSQL list size:      %d\n", atomicRead32(&str_list_size_mysql));
    printf("\n");
  } else {
    printf("MYSQL status:         %s\n", YELLOW"Not using MYSQL"RESET);
    printf("\n");
    printf("\n");
    printf("\n");
  }
#endif

  switch (status->log_error) {
    case LOG_LOG_FD_ERROR:
      sprintf(temp, RED"Error opening log, file closed"RESET);
      break;
    case LIST_NULL_ERROR:
      sprintf(temp, RED"Error, list passed was NULL"RESET);
      break;
    case CLEAN_EXIT_ERROR:
      sprintf(temp, YELLOW"Exited cleanly"RESET);
      break;
    default:
      sprintf(temp, GREEN"No problem"RESET);
      break;
  }
  if (use_log) {
    printf("Log status:           %s\n", temp);
    printf("Log last update:      %f\n", (double)(millis() - status->log_t) / 1000.0);
    printf("Log analyzed nodes:   %d\n", status->log_nodes);
    printf("Log list size:        %d\n", atomicRead32(&str_list_size_log));
    printf("\n");
  } else {
    printf("Log status:           %s\n", YELLOW"Not using log"RESET);
    printf("\n");
    printf("\n");
    printf("\n");
  }

  switch (status->pipe_error) {
    case PIPE_COULD_NOT_OPEN_ERROR:
      sprintf(temp, RED"Error opening serial source"RESET);
      break;
    case PIPE_CLOSED_ERROR:
      sprintf(temp, RED"Error, serial source is closed"RESET);
      break;
    default:
      sprintf(temp, GREEN"No problem"RESET);
      break;
  }
  printf("Serial status:        %s\n", temp);
  printf("\n");

  switch (status->server_error) {
    case SERVER_SOCKET_CREATE_ERROR:
      sprintf(temp, RED"Error creating server socket"RESET);
      break;
    case SERVER_SOCKET_BIND_ERROR:
      sprintf(temp, RED"Error binding to the socket"RESET);
      break;
    case SERVER_LISTEN_ERROR:
      sprintf(temp, RED"Error listening to the socket"RESET);
      break;
    case SERVER_ACCEPT_ERROR:
      sprintf(temp, RED"Error accepting request"RESET);
      break;
    case SERVER_NON_BLOCKING_ERROR:
      sprintf(temp, RED"Error setting socket to non-blocking mode"RESET);
      break;
    case SERVER_RECEIVE_ZERO_ERROR:
      sprintf(temp, RED"Error receiving, message length = 0"RESET);
      break;
    case CLEAN_EXIT_ERROR:
      sprintf(temp, YELLOW"Exited cleanly"RESET);
      break;
    default:
      sprintf(temp, GREEN"No problem"RESET);
      break;
  }
  if (use_server) {
    printf("Server status:        %s\n", temp);
    printf("Server requests:      %lld\n", status->server_requests);
    printf("Server list size:     %d\n", atomicRead32(&str_list_size_server));
    printf("\n");
  } else {
    printf("Server status:        %s\n", YELLOW"Not using the server"RESET);
    printf("\n");
    printf("\n");
  }
  char** model_data_keys;
  char** data_keys;
  int model_data_n = parse_CSV(model, &model_data_keys);
  int data_n = parse_CSV(status->values.text, &data_keys);

  int iter, iter2;
  if (data_n == model_data_n) {
    double* values = malloc(sizeof(double)*data_n);
    int i;
    for (i = 0; i < data_n; i++)
      sscanf(data_keys[i], "%lf", values+i);
    printf("+------------+--------------+\n");
    printf("| %10s | %+1.5e |\n", "abs_t", status->values.time);
    for (i = 0; i < data_n; i++) {
      printf("| %10s | %+1.5e |\n", model_data_keys[i], values[i]);
    }
    printf("+------------+--------------+\n");
    //printf("abs_t,%s\n", model);
    //printf("%f,%s\n", status->values.time, status->values.text);

    //if ((main_graph->last_updated - ((double)millis() / 1000.0)) > (2000.0 / (double)(main_graph->N))) {
    main_graph->maxy = main_graph->Y[0][1];
    main_graph->miny = main_graph->Y[0][1];
    for (iter = 0; iter < main_graph->M; iter++) {
      for (iter2 = 1; iter2 < (main_graph->N); iter2++) {
        main_graph->Y[iter][iter2-1] = main_graph->Y[iter][iter2];
        main_graph->maxy = main_graph->maxy > main_graph->Y[iter][iter2] ? main_graph->maxy : main_graph->Y[iter][iter2];
        main_graph->miny = main_graph->miny < main_graph->Y[iter][iter2] ? main_graph->miny : main_graph->Y[iter][iter2];
      }
      main_graph->Y[iter][main_graph->N - 1] = values[iter+1];
      main_graph->maxy = main_graph->maxy > values[iter+1] ? main_graph->maxy : values[iter+1];
      main_graph->miny = main_graph->miny < values[iter+1] ? main_graph->miny : values[iter+1];
    }
    for (iter2 = 1; iter2 < (main_graph->N); iter2++)
      main_graph->x[iter2-1] = main_graph->x[iter2];
    main_graph->x[main_graph->N - 1] = status->values.time - main_graph->last_updated;
    main_graph->maxx = main_graph->x[main_graph->N - 1];
    main_graph->minx = main_graph->x[0];
   
    //main_graph->last_updated = (double)millis() / 1000.0;
    //}
    free(values);
  }
  char markers[] = "x+o-*#.•xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  text_plot_config* config = malloc(sizeof(text_plot_config));
  config->N = main_graph->N;
  config->x = main_graph->x;
  config->minx = main_graph->minx;
  config->maxx = main_graph->maxx;
  config->miny = main_graph->miny;
  config->maxy = main_graph->maxy;
  config->draw = 0;
  config->clear = 1;
  config->canvas = NULL;
  config->free = 0;
  config->n = 70;
  config->m = 20;
  config->marker = markers[0];
  for (iter = 0; iter < main_graph->M - 1; iter++) {
    config->y = main_graph->Y[iter];
    text_plot(config);
    config->clear = 0;
    config->marker = markers[iter+1];
  }
  config->draw = 1;
  config->free = 1;
  config->y = main_graph->Y[main_graph->M - 1];
  text_plot(config);

  free_CSV_fields(model_data_keys, model_data_n);
  free_CSV_fields(data_keys, data_n);
  free(temp);

  free(config);
}

void print_configuration() {
  printf("+----------------+---------------------------------------+\n");
  printf("Serial path =    |   %s\n", serial_path);
  printf("Serial speed =   |   %d\n", serial_speed);
  printf("                 |   \n");
  printf("Model =          |   %s\n", model);
  printf("                 |   \n");

  printf("Use log =        |   %d\n", use_log);
  printf("Log directory =  |   %s\n", path_to_log_dir);
  printf("                 |   \n");

#if USE_MYSQL==1
  printf("Use MYSQL =      |   %d\n", use_mysql);
  printf("MYSQL address =  |   %s\n", mysql_addr);
  printf("MYSQL login =    |   %s\n", mysql_login);
  printf("MYSQL password = |   %s\n", mysql_pswd);
  printf("MYSQL database = |   %s\n", mysql_db);
  printf("MYSQL table =    |   %s\n", mysql_table);
  printf("                 |   \n");
#endif

  printf("Use host =       |   %d\n", use_host);
  printf("host =           |   %s:%d\n", host, port);
  printf("                 |   \n");

  printf("Use server =     |   %d\n", use_server);
  printf("Server port =    |   %d\n", server_port);
  printf("+----------------+---------------------------------------+\n");
}

void process_command(char* field, char* value) {
  int i;
  size_t flen = strlen(field);

  for (i = 0; i < flen; i++)
    field[i] = tolower(field[i]);

  if (strcmp(field, "use log") == 0) {
    if (strcmp(value, "1") == 0)
      use_log = 1;
    else
      use_log = 0;
  } else if (strcmp(field, "use server") == 0) {
    if (strcmp(value, "1") == 0)
      use_server = 1;
    else
      use_server = 0;
  } else if (strcmp(field, "use mysql") == 0) {
    if (strcmp(value, "1") == 0)
      use_mysql = 1;
    else
      use_mysql = 0;
  } else if (strcmp(field, "use host") == 0) {
    if (strcmp(value, "1") == 0)
      use_host = 1;
    else
      use_host = 0;
  } else if (strcmp(field, "host") == 0) {
    char* port_str = strchr(value, ':');
    if (port_str == NULL)
      port = 8888;
    else {
      *(port_str) = 0;
      port = atoi(port_str+1);
      if (port < 0)
        port = 8888;
    }

    if (strlen(value) < sizeof(host)) {
      strcpy(host, value);
      use_host = 1;
    }
  } else if (strcmp(field, "server port") == 0) {
    int p = atoi(value);
    if (p < 0) {
      p = 8888;
    } else {
      server_port = p; 
      use_server = 1;
    }
  } else if (strcmp(field, "mysql address") == 0) {
    sprintf(mysql_addr, "%s", value);
  } else if (strcmp(field, "mysql login") == 0) {
    sprintf(mysql_login, "%s", value);
  } else if (strcmp(field, "mysql password") == 0) {
    sprintf(mysql_pswd, "%s", value);
  } else if (strcmp(field, "mysql database") == 0) {
    sprintf(mysql_db, "%s", value);
  } else if (strcmp(field, "mysql table") == 0) {
    sprintf(mysql_table, "%s", value);
  } else if (strcmp(field, "log directory") == 0) {
    sprintf(path_to_log_dir, "%s", value);
    size_t ptl_len = strlen(path_to_log_dir);
    if (path_to_log_dir[ptl_len-1] != '/') {
      path_to_log_dir[ptl_len] = '/';
      path_to_log_dir[ptl_len+1] = 0;
    }
  } else if (strcmp(field, "model") == 0) {
    char** fields;
    int n = parse_CSV(value, &fields);
    if (n > 0)
      sprintf(model, "%s", value);
    else
      printf("Model is ill-formatted\n");
    free_CSV_fields(fields, n);
  } else if (strcmp(field, "serial path") == 0) {
    sprintf(serial_path, "%s", value);
  } else if (strcmp(field, "serial speed") == 0) {
    serial_speed = atoi(value);
    if (serial_speed < 0)
      serial_speed = PORT_SPEED;
  } else if (strcmp(field, "print") == 0) {
    print_configuration();
  } else {
    printf("Unrecognized option: %s = %s\n", field, value);
  }
}
