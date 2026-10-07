
## IE3090 RemoteOps

## Student Details
- Registration Number: IT24103709
- Platform: CentOS 9
- Implementation Language: C
- Network API: BSD Sockets
- TCP Port: 9410
- Session Identifier (SID): 9073

## Project Overview
RemoteOps is a client-server application developed in C. The Controller communicates with the Agent using TCP sockets. The application supports authentication, system information retrieval, process listing, restricted command execution, file upload and download, and monitoring.

## Personalized Configuration
- Authentication token: OPS-3709
- TCP port: 9410
- SID: 9073
- Agent storage directory: ./agentfiles/IT24103709/
- Log file: remoteops_IT24103709.log

## Main Components
- agent_709.c: Agent/server implementation.
- controller_709.c: Controller/client implementation.
- Makefile_709: Build configuration.
- agentfiles/IT24103709/: Directory for files received by the Agent.
- screenshots/: Evidence of implementation and testing.
- documentation/: Design diary, test results, and AI prompt log.

## Compilation
```bash
make -f Makefile_709
```

## Running the Agent
Start the Agent in one terminal:
```bash
./agent_709
```

## Running the Controller
In another terminal on the same machine:
```bash
./controller_709 127.0.0.1 9410
```

Authenticate through the Controller menu before running protected commands.

## Testing
Record the actual results of authentication, SYSINFO, LISTPROC, permitted EXEC commands, PUT, GET, UDP monitoring, and concurrent Controller connections in the implementation report.

## Security
The Agent restricts EXEC commands to the commands implemented in the program. File transfers use the Agent's configured storage directory and filename validation.

## Limitations
Document any known limitations discovered during testing.

## Screenshots
Screenshots of the source code, runtime output, file transfers, monitoring, logs, and concurrent connections are stored in the screenshots directory.
## Features
- TCP client-server communication
- Authentication
- System information
- Process listing
- Restricted command execution
- File upload and download
- UDP monitoring
- Event logging
