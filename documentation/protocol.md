# RemoteOps Communication Protocol

## TCP Commands
- AUTH: Authenticate a controller.
- SYSINFO: Request system information.
- LISTPROC: Request a process listing.
- EXEC: Execute an allowed command.
- PUT: Upload a file to the Agent.
- GET: Download a file from the Agent.
- QUIT: Close the connection.

## Personalization
- TCP Port: 9410
- SID: 9073

## File Transfer
PUT and GET transfer file data over TCP.
Only simple filenames are accepted by the Agent.
