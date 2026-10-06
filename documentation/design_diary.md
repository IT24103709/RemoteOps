
# RemoteOps Design Diary

## Project Setup
- Set up the project environment on CentOS 9.
- Created the Agent and Controller source files.
- Configured the personalized TCP port and SID.

## Socket Communication
- Implemented TCP client-server communication using BSD sockets.
- Tested the Controller connection to the Agent on localhost.

## Authentication and Commands
- Implemented token-based authentication.
- Tested system information and process listing.
- Tested the permitted EXEC commands and invalid commands.

## File Transfer
- Tested uploading sample.txt using PUT.
- Tested downloading sample.txt using GET.
- Checked the received file contents and file integrity.

## Monitoring and Concurrency
- Tested UDP monitoring and monitoring stop behaviour.
- Tested multiple Controller connections.
- Recorded any failures and the fixes applied.

## Final Review
- Rebuilt the project using Makefile_709.
- Reviewed the log file and Agent storage directory.
- Prepared the implementation report and submission archive.
