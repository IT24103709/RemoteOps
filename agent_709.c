/*
 * IE3090 RemoteOps
 * Registration Number: IT24103709
 *
 * Agent / Server
 *
 * Port       : 9410
 * SID        : 9073
 * Auth Token : OPS-3709
 *
 * File       : agent_709.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <limits.h>

#define PORT 9410
#define SID "9073"
#define AUTH_TOKEN "OPS-3709"

#define LOG_FILE "remoteops_IT24103709.log"
#define STORAGE_DIR "./agentfiles/IT24103709"

#define MAX_LINE 1024
#define MAX_FILENAME 256
#define MAX_FILE_SIZE (10 * 1024 * 1024)

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;

    int monitoring;
    int udp_port;

    pthread_t monitor_thread;
    pthread_mutex_t monitor_mutex;
} client_session_t;


/* =========================================================
   Utility functions
   ========================================================= */

void get_timestamp(char *buffer, size_t size)
{
    time_t now = time(NULL);
    struct tm tm_now;

    localtime_r(&now, &tm_now);

    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", &tm_now);
}


void log_event(const char *event)
{
    FILE *fp = fopen(LOG_FILE, "a");

    if (fp == NULL) {
        perror("log file");
        return;
    }

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    fprintf(fp, "[%s] %s\n", timestamp, event);

    fclose(fp);
}


/*
 * Receive exactly one line ending with '\n'.
 */
int recv_line(int fd, char *buffer, size_t size)
{
    size_t pos = 0;

    while (pos < size - 1) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);

        if (n == 0) {
            return 0;
        }

        if (n < 0) {
            if (errno == EINTR)
                continue;

            return -1;
        }

        buffer[pos++] = c;

        if (c == '\n')
            break;
    }

    buffer[pos] = '\0';
    return (int)pos;
}


/*
 * Send all bytes.
 */
int send_all(int fd, const void *data, size_t length)
{
    size_t total = 0;
    const char *ptr = (const char *)data;

    while (total < length) {

        ssize_t n = send(fd, ptr + total, length - total, 0);

        if (n < 0) {

            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return -1;

        total += (size_t)n;
    }

    return 0;
}


/*
 * Receive exactly 'length' bytes.
 */
int recv_all(int fd, void *buffer, size_t length)
{
    size_t total = 0;
    char *ptr = (char *)buffer;

    while (total < length) {

        ssize_t n = recv(fd, ptr + total, length - total, 0);

        if (n < 0) {

            if (errno == EINTR)
                continue;

            return -1;
        }

        if (n == 0)
            return -1;

        total += (size_t)n;
    }

    return 0;
}


/*
 * Send a protocol response.
 * Every TCP response ends with SID:9073.
 */
void send_response(int fd, const char *message)
{
    char response[MAX_LINE];

    snprintf(response,
             sizeof(response),
             "%s SID:%s\n",
             message,
             SID);

    send_all(fd, response, strlen(response));
}


/*
 * Prevent path traversal.
 * Only allow a simple filename.
 */
int valid_filename(const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
        return 0;

    if (strlen(filename) >= MAX_FILENAME)
        return 0;

    if (strstr(filename, "..") != NULL)
        return 0;

    if (strchr(filename, '/') != NULL)
        return 0;

    if (strchr(filename, '\\') != NULL)
        return 0;

    return 1;
}


/* =========================================================
   System information
   ========================================================= */

double get_cpu_load(void)
{
    FILE *fp = fopen("/proc/loadavg", "r");

    if (fp == NULL)
        return 0.0;

    double load = 0.0;

    fscanf(fp, "%lf", &load);

    fclose(fp);

    return load;
}


long get_memory_used_mb(void)
{
    FILE *fp = fopen("/proc/meminfo", "r");

    if (fp == NULL)
        return 0;

    char line[256];

    long mem_total = 0;
    long mem_available = 0;

    while (fgets(line, sizeof(line), fp)) {

        if (sscanf(line, "MemTotal: %ld kB", &mem_total) == 1)
            continue;

        if (sscanf(line, "MemAvailable: %ld kB", &mem_available) == 1)
            continue;
    }

    fclose(fp);

    if (mem_total <= 0)
        return 0;

    long used_kb = mem_total - mem_available;

    return used_kb / 1024;
}


long get_uptime_seconds(void)
{
    FILE *fp = fopen("/proc/uptime", "r");

    if (fp == NULL)
        return 0;

    double uptime = 0;

    fscanf(fp, "%lf", &uptime);

    fclose(fp);

    return (long)uptime;
}


/* =========================================================
   SYSINFO
   ========================================================= */

void handle_sysinfo(int fd)
{
    double cpu = get_cpu_load();
    long memory = get_memory_used_mb();
    long uptime = get_uptime_seconds();

    char response[MAX_LINE];

    snprintf(response,
             sizeof(response),
             "OK SYSINFO %.2f %ld %ld",
             cpu,
             memory,
             uptime);

    send_response(fd, response);

    log_event("SYSINFO command executed");
}


/* =========================================================
   LISTPROC
   ========================================================= */

void handle_listproc(int fd)
{
    FILE *fp = popen("ps -eo pid=,comm= --no-headers | head -50", "r");

    if (fp == NULL) {
        send_response(fd, "ERR 003 PROCESS_LIST_FAILED");
        return;
    }

    char result[MAX_LINE];
    result[0] = '\0';

    char line[128];

    while (fgets(line, sizeof(line), fp)) {

        line[strcspn(line, "\n")] = '\0';

        if (strlen(result) + strlen(line) + 2 >= sizeof(result))
            break;

        if (result[0] != '\0')
            strcat(result, ",");

        strcat(result, line);
    }

    pclose(fp);

    char response[MAX_LINE];

    snprintf(response,
         sizeof(response),
         "OK PROCS %.*s",
         (int)(sizeof(response) - strlen("OK PROCS ") - 1),
         result);

    send_response(fd, response);

    log_event("LISTPROC command executed");
}


/* =========================================================
   EXEC whitelist
   ========================================================= */

void handle_exec(int fd, const char *command)
{
    char shell_command[256];
    char result[MAX_LINE];

    result[0] = '\0';

    if (strcmp(command, "DATE") == 0) {

        strcpy(shell_command, "date");

    } else if (strcmp(command, "UPTIME") == 0) {

        strcpy(shell_command, "uptime");

    } else if (strcmp(command, "DISKFREE") == 0) {

        strcpy(shell_command, "df -h /");

    } else if (strcmp(command, "HOSTNAME") == 0) {

        strcpy(shell_command, "hostname");

    } else if (strcmp(command, "WHOAMI") == 0) {

        strcpy(shell_command, "whoami");

    } else {

        send_response(fd,
                      "ERR 002 COMMAND_NOT_ALLOWED");

        log_event("Rejected disallowed EXEC command");

        return;
    }

    FILE *fp = popen(shell_command, "r");

    if (fp == NULL) {

        send_response(fd,
                      "ERR 003 EXEC_FAILED");

        return;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp)) {

        line[strcspn(line, "\r\n")] = '\0';

        if (strlen(result) + strlen(line) + 2 >= sizeof(result))
            break;

        if (result[0] != '\0')
            strcat(result, " ");

        strcat(result, line);
    }

    pclose(fp);

    char response[MAX_LINE];

    snprintf(response,
         sizeof(response),
         "OK EXEC_RESULT %.*s",
         (int)(sizeof(response) - strlen("OK EXEC_RESULT ") - 1),
         result);

    send_response(fd, response);

    char logmsg[512];

    snprintf(logmsg,
             sizeof(logmsg),
             "EXEC %s",
             command);

    log_event(logmsg);
}


/* =========================================================
   PUT
   ========================================================= */

void handle_put(int fd,
                const char *filename,
                long filesize)
{
    if (!valid_filename(filename)) {

        send_response(fd,
                      "ERR 006 INVALID_FILENAME");

        return;
    }

    if (filesize < 0 || filesize > MAX_FILE_SIZE) {

        send_response(fd,
                      "ERR 004 FILE_TOO_LARGE");

        return;
    }

    char path[PATH_MAX];

    snprintf(path,
             sizeof(path),
             "%s/%s",
             STORAGE_DIR,
             filename);

    FILE *fp = fopen(path, "wb");

    if (fp == NULL) {

        send_response(fd,
                      "ERR 003 FILE_OPEN_FAILED");

        return;
    }

    char buffer[4096];

    long remaining = filesize;

    while (remaining > 0) {

        size_t chunk =
            remaining > (long)sizeof(buffer)
            ? sizeof(buffer)
            : (size_t)remaining;

        if (recv_all(fd, buffer, chunk) < 0) {

            fclose(fp);

            remove(path);

            return;
        }

        fwrite(buffer, 1, chunk, fp);

        remaining -= (long)chunk;
    }

    fclose(fp);

    char response[MAX_LINE];

    snprintf(response,
             sizeof(response),
             "OK FILE_RECEIVED %s",
             filename);

    send_response(fd, response);

    char logmsg[512];

    snprintf(logmsg,
             sizeof(logmsg),
             "PUT %s (%ld bytes)",
             filename,
             filesize);

    log_event(logmsg);
}


/* =========================================================
   GET
   ========================================================= */

void handle_get(int fd, const char *filename)
{
    if (!valid_filename(filename)) {

        send_response(fd,
                      "ERR 006 INVALID_FILENAME");

        return;
    }

    char path[PATH_MAX];

    snprintf(path,
             sizeof(path),
             "%s/%s",
             STORAGE_DIR,
             filename);

    FILE *fp = fopen(path, "rb");

    if (fp == NULL) {

        send_response(fd,
                      "ERR 005 FILE_NOT_FOUND");

        log_event("GET file not found");

        return;
    }

    fseek(fp, 0, SEEK_END);

    long filesize = ftell(fp);

    fseek(fp, 0, SEEK_SET);

    char header[MAX_LINE];

    snprintf(header,
             sizeof(header),
             "OK FILE_SEND %s %ld SID:%s\n",
             filename,
             filesize,
             SID);

    if (send_all(fd, header, strlen(header)) < 0) {

        fclose(fp);
        return;
    }

    char buffer[4096];

    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {

        if (send_all(fd, buffer, n) < 0)
            break;
    }

    fclose(fp);

    char logmsg[512];

    snprintf(logmsg,
             sizeof(logmsg),
             "GET %s (%ld bytes)",
             filename,
             filesize);

    log_event(logmsg);
}


/* =========================================================
   UDP Monitoring
   ========================================================= */

void *monitor_thread_function(void *arg)
{
    client_session_t *session =
        (client_session_t *)arg;

    int udp_fd = socket(AF_INET,
                        SOCK_DGRAM,
                        0);

    if (udp_fd < 0)
        return NULL;

    struct sockaddr_in udp_addr;

    memset(&udp_addr, 0, sizeof(udp_addr));

    udp_addr.sin_family = AF_INET;

    udp_addr.sin_port =
        htons(session->udp_port);

    udp_addr.sin_addr =
        session->client_addr.sin_addr;

    while (1) {

        pthread_mutex_lock(&session->monitor_mutex);

        int active = session->monitoring;

        pthread_mutex_unlock(&session->monitor_mutex);

        if (!active)
            break;

        double cpu = get_cpu_load();

        long memory = get_memory_used_mb();

        long uptime = get_uptime_seconds();

        char message[512];

        snprintf(message,
                 sizeof(message),
                 "SYSINFO %.2f %ld %ld SID:%s",
                 cpu,
                 memory,
                 uptime,
                 SID);

        sendto(udp_fd,
               message,
               strlen(message),
               0,
               (struct sockaddr *)&udp_addr,
               sizeof(udp_addr));

        sleep(3);
    }

    close(udp_fd);

    return NULL;
}


/* =========================================================
   MONITOR START
   ========================================================= */

void handle_monitor_start(client_session_t *session,
                          int udp_port)
{
    pthread_mutex_lock(&session->monitor_mutex);

    if (session->monitoring) {

        pthread_mutex_unlock(&session->monitor_mutex);

        send_response(session->client_fd,
                      "OK MONITOR_STARTED");

        return;
    }

    session->udp_port = udp_port;

    session->monitoring = 1;

    pthread_mutex_unlock(&session->monitor_mutex);

    pthread_create(&session->monitor_thread,
                   NULL,
                   monitor_thread_function,
                   session);

    send_response(session->client_fd,
                  "OK MONITOR_STARTED");

    log_event("MONITOR START");
}


/* =========================================================
   MONITOR STOP
   ========================================================= */

void handle_monitor_stop(client_session_t *session)
{
    pthread_mutex_lock(&session->monitor_mutex);

    int was_active = session->monitoring;

    session->monitoring = 0;

    pthread_mutex_unlock(&session->monitor_mutex);

    if (was_active)
        pthread_join(session->monitor_thread, NULL);

    send_response(session->client_fd,
                  "OK MONITOR_STOPPED");

    log_event("MONITOR STOP");
}


/* =========================================================
   Client handler
   ========================================================= */

void *client_handler(void *arg)
{
    client_session_t *session =
        (client_session_t *)arg;

    int fd = session->client_fd;

    char client_ip[INET_ADDRSTRLEN];

    inet_ntop(AF_INET,
              &session->client_addr.sin_addr,
              client_ip,
              sizeof(client_ip));

    char logmsg[512];

    snprintf(logmsg,
             sizeof(logmsg),
             "Connection accepted from %s:%d",
             client_ip,
             ntohs(session->client_addr.sin_port));

    log_event(logmsg);

    int authenticated = 0;

    char line[MAX_LINE];

    while (1) {

        int n = recv_line(fd,
                          line,
                          sizeof(line));

        if (n <= 0)
            break;

        line[strcspn(line, "\r\n")] = '\0';

        if (strncmp(line, "AUTH ", 5) == 0) {

            char token[256];

            sscanf(line + 5,
                   "%255s",
                   token);

            if (strcmp(token, AUTH_TOKEN) == 0) {

                authenticated = 1;

                send_response(fd,
                              "OK AUTHENTICATED");

                log_event("AUTH successful");

            } else {

                send_response(fd,
                              "ERR 001 AUTH_FAILED");

                log_event("AUTH failed");
            }

            continue;
        }


        /*
         * All commands except AUTH require authentication.
         */

        if (!authenticated) {

            send_response(fd,
                          "ERR 001 AUTH_FAILED");

            log_event("Command rejected before authentication");

            continue;
        }


        if (strcmp(line, "SYSINFO") == 0) {

            handle_sysinfo(fd);

        } else if (strcmp(line, "LISTPROC") == 0) {

            handle_listproc(fd);

        } else if (strncmp(line, "EXEC ", 5) == 0) {

            handle_exec(fd,
                        line + 5);

        } else if (strncmp(line, "PUT ", 4) == 0) {

            char filename[MAX_FILENAME];

            long filesize;

            if (sscanf(line + 4,
                       "%255s %ld",
                       filename,
                       &filesize) == 2) {

                handle_put(fd,
                           filename,
                           filesize);

            } else {

                send_response(fd,
                              "ERR 007 BAD_PUT_FORMAT");
            }

        } else if (strncmp(line, "GET ", 4) == 0) {

            char filename[MAX_FILENAME];

            if (sscanf(line + 4,
                       "%255s",
                       filename) == 1) {

                handle_get(fd,
                           filename);

            } else {

                send_response(fd,
                              "ERR 007 BAD_GET_FORMAT");
            }

        } else if (strncmp(line, "MONITOR START ", 14) == 0) {

            int udp_port =
                atoi(line + 14);

            if (udp_port < 1 || udp_port > 65535) {

                send_response(fd,
                              "ERR 008 INVALID_UDP_PORT");

            } else {

                handle_monitor_start(session,
                                     udp_port);
            }

        } else if (strcmp(line, "MONITOR STOP") == 0) {

            handle_monitor_stop(session);

        } else if (strcmp(line, "QUIT") == 0) {

            pthread_mutex_lock(&session->monitor_mutex);

            int active = session->monitoring;

            session->monitoring = 0;

            pthread_mutex_unlock(&session->monitor_mutex);

            if (active)
                pthread_join(session->monitor_thread,
                             NULL);

            send_response(fd,
                          "OK BYE");

            log_event("Client disconnected with QUIT");

            break;

        } else {

            send_response(fd,
                          "ERR 009 UNKNOWN_COMMAND");

            log_event("Unknown command");
        }
    }


    /*
     * Make sure monitoring stops when client disconnects.
     */

    pthread_mutex_lock(&session->monitor_mutex);

    int active = session->monitoring;

    session->monitoring = 0;

    pthread_mutex_unlock(&session->monitor_mutex);

    if (active)
        pthread_join(session->monitor_thread,
                     NULL);


    close(fd);

    log_event("Connection closed");

    pthread_mutex_destroy(&session->monitor_mutex);

    free(session);

    return NULL;
}


/* =========================================================
   Main
   ========================================================= */

int main(void)
{
    signal(SIGPIPE, SIG_IGN);

    /*
     * Create storage directory.
     */

    mkdir("./agentfiles", 0755);

    mkdir(STORAGE_DIR, 0755);


    /*
     * Create TCP socket.
     */

    int server_fd =
        socket(AF_INET,
               SOCK_STREAM,
               0);

    if (server_fd < 0) {

        perror("socket");

        return 1;
    }


    int opt = 1;

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));


    /*
     * Bind to port 9410.
     */

    struct sockaddr_in server_addr;

    memset(&server_addr,
           0,
           sizeof(server_addr));

    server_addr.sin_family =
        AF_INET;

    server_addr.sin_addr.s_addr =
        INADDR_ANY;

    server_addr.sin_port =
        htons(PORT);


    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {

        perror("bind");

        close(server_fd);

        return 1;
    }


    /*
     * Listen.
     */

    if (listen(server_fd, 10) < 0) {

        perror("listen");

        close(server_fd);

        return 1;
    }


    printf("=====================================\n");
    printf("        RemoteOps Agent\n");
    printf("=====================================\n");
    printf("Registration : IT24103709\n");
    printf("Port         : %d\n", PORT);
    printf("SID          : %s\n", SID);
    printf("Storage      : %s\n", STORAGE_DIR);
    printf("Status       : Listening...\n");
    printf("=====================================\n");


    log_event("RemoteOps Agent started");


    /*
     * Accept clients forever.
     */

    while (1) {

        struct sockaddr_in client_addr;

        socklen_t client_len =
            sizeof(client_addr);

        int client_fd =
            accept(server_fd,
                   (struct sockaddr *)&client_addr,
                   &client_len);

        if (client_fd < 0) {

            if (errno == EINTR)
                continue;

            perror("accept");

            continue;
        }


        client_session_t *session =
            malloc(sizeof(client_session_t));

        if (session == NULL) {

            close(client_fd);

            continue;
        }


        session->client_fd =
            client_fd;

        session->client_addr =
            client_addr;

        session->monitoring = 0;

        session->udp_port = 0;

        pthread_mutex_init(&session->monitor_mutex,
                           NULL);


        pthread_t thread;

        if (pthread_create(&thread,
                           NULL,
                           client_handler,
                           session) != 0) {

            close(client_fd);

            pthread_mutex_destroy(
                &session->monitor_mutex);

            free(session);

            continue;
        }


        /*
         * Client thread resources are automatically
         * released after termination.
         */

        pthread_detach(thread);
    }


    close(server_fd);

    return 0;
}
