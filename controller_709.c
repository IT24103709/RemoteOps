#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>

#define PORT 9410
#define BUFFER_SIZE 8192

void send_command(int sock, const char *command)
{
    char buffer[BUFFER_SIZE];

    snprintf(buffer, sizeof(buffer), "%s\n", command);

    send(sock, buffer, strlen(buffer), 0);

    memset(buffer, 0, sizeof(buffer));

    int n = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (n > 0)
    {
        buffer[n] = '\0';
        printf("%s", buffer);
    }
}


void upload_file(int sock, const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");

    if (fp == NULL) {
        perror("File");
        return;
    }

    /* Keep the full path for local file access,
       but send only the filename to the Agent. */
    const char *filename = strrchr(filepath, '/');
    filename = (filename != NULL) ? filename + 1 : filepath;

    if (*filename == '\0' ||
        strcmp(filename, ".") == 0 ||
        strcmp(filename, "..") == 0) {
        printf("Invalid filename\n");
        fclose(fp);
        return;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        perror("fseek");
        fclose(fp);
        return;
    }

    long size = ftell(fp);

    if (size < 0 || fseek(fp, 0, SEEK_SET) != 0) {
        perror("File size");
        fclose(fp);
        return;
    }

    char header[512];

    int header_len = snprintf(
        header, sizeof(header),
        "PUT %s %ld\n", filename, size
    );

    if (header_len < 0 || (size_t)header_len >= sizeof(header)) {
        printf("Upload header too long\n");
        fclose(fp);
        return;
    }

    /* Send all header bytes, handling partial sends. */
    size_t sent = 0;

    while (sent < (size_t)header_len) {
        ssize_t n = send(sock, header + sent,
                         (size_t)header_len - sent, 0);

        if (n < 0 && errno == EINTR)
            continue;

        if (n <= 0) {
            perror("send header");
            fclose(fp);
            return;
        }

        sent += (size_t)n;
    }

    char buffer[4096];
    long remaining = size;

    while (remaining > 0) {
        size_t wanted = remaining > (long)sizeof(buffer)
                      ? sizeof(buffer) : (size_t)remaining;

        size_t nread = fread(buffer, 1, wanted, fp);

        if (nread == 0) {
            printf("Local file read failed\n");
            fclose(fp);
            return;
        }

        size_t offset = 0;

        while (offset < nread) {
            ssize_t n = send(sock, buffer + offset,
                             nread - offset, 0);

            if (n < 0 && errno == EINTR)
                continue;

            if (n <= 0) {
                perror("send file data");
                fclose(fp);
                return;
            }

            offset += (size_t)n;
        }

        remaining -= (long)nread;
    }

    fclose(fp);

    char response[BUFFER_SIZE];
    ssize_t r;

    do {
        r = recv(sock, response, sizeof(response) - 1, 0);
    } while (r < 0 && errno == EINTR);

    if (r > 0) {
        response[r] = '\0';
        printf("%s", response);
    } else {
        printf("No upload confirmation received\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <agent-ip> <port>\n", argv[0]);
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server.sin_addr) <= 0)
    {
        printf("Invalid IP address\n");
        return 1;
    }

    if (connect(sock,
                (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("Connected to RemoteOps Agent\n");

    


    while (1)
    {
        printf("\nRemoteOps Controller\n");
        printf("1. AUTH\n");
        printf("2. SYSINFO\n");
        printf("3. LISTPROC\n");
        printf("4. EXEC\n");
        printf("5. PUT\n");
        printf("6. GET\n");
        printf("7. MONITOR START\n");
        printf("8. MONITOR STOP\n");
        printf("9. QUIT\n");
        printf("Choice: ");

        int choice;

        scanf("%d", &choice);

        getchar();

        if (choice == 1)
        {
            send_command(sock,
                         "AUTH OPS-3709");
        }

        else if (choice == 2)
        {
            send_command(sock,
                         "SYSINFO");
        }

        else if (choice == 3)
        {
            send_command(sock,
                         "LISTPROC");
        }

        else if (choice == 4)
        {
            char command[100];

            printf("Enter command (DATE/UPTIME/DISKFREE/HOSTNAME/WHOAMI): ");

            fgets(command, sizeof(command), stdin);

            command[strcspn(command, "\n")] = '\0';

            char full[150];

            snprintf(full,
                     sizeof(full),
                     "EXEC %s",
                     command);

            send_command(sock, full);
        }

        else if (choice == 5)
        {
            char filename[256];

            printf("Enter local filename: ");

            fgets(filename,
                  sizeof(filename),
                  stdin);

            filename[strcspn(filename, "\n")] = '\0';

            upload_file(sock, filename);
        }

        else if (choice == 6)
        {
            char filename[256];

            printf("Enter filename: ");

            fgets(filename,
                  sizeof(filename),
                  stdin);

            filename[strcspn(filename, "\n")] = '\0';

            char command[300];

            snprintf(command,
                     sizeof(command),
                     "GET %s",
                     filename);

            send_command(sock, command);
        }

        else if (choice == 7)
        {
            send_command(sock,
                         "MONITOR START 9500");
        }

        else if (choice == 8)
        {
            send_command(sock,
                         "MONITOR STOP");
        }

        else if (choice == 9)
        {
            send_command(sock,
                         "QUIT");

            break;
        }

        else
        {
            printf("Invalid choice\n");
        }
    }

    close(sock);

    return 0;
}
