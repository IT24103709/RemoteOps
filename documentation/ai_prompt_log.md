1. Purpose of AI Assistance

AI assistance was used during the development of the RemoteOps assignment to understand BSD socket programming, configure the development environment, troubleshoot implementation errors, test client-server communication, and prepare technical documentation. AI-generated suggestions were reviewed and tested on the CentOS 9 environment before being used.

2. Prompts Used and Their Applications
Prompt 1: Understanding the Assignment

Prompt used:
"Explain the IE3090 RemoteOps assignment requirements and help me implement the project step by step on CentOS 9 using C and BSD sockets. The Agent and Controller will run on the same machine."

Tool used: ChatGPT

How the output was used:
The explanation helped me organize the project into the Agent, Controller, Makefile, file storage, logging, testing, and documentation components.

Verification and changes:
I followed the steps in my CentOS environment and checked the created project files and build configuration.

Prompt 2: Personalized Configuration

Prompt used:
"Help me determine and configure the personalized TCP port, session identifier, authentication token, source filenames, storage directory, and log filename for registration number IT24103709."

Tool used: ChatGPT

How the output was used:
The personalized configuration was used for the project files and runtime settings:

TCP port: 9410
Session Identifier (SID): 9073
Authentication token: OPS-3709
Agent source file: agent_709.c
Controller source file: controller_709.c
Makefile: Makefile_709
Storage directory: agentfiles/IT24103709/
Log file: remoteops_IT24103709.log

Verification and changes:
I checked the Agent's displayed configuration and confirmed that the service was listening on TCP port 9410.

Prompt 3: TCP Socket Communication

Prompt used:
"Explain how to compile and run a C-based TCP Agent and Controller using BSD sockets on CentOS 9, and how to troubleshoot connection errors."

Tool used: ChatGPT

How the output was used:
The guidance helped me compile the C programs, start the Agent, connect the Controller to 127.0.0.1 on port 9410, and inspect the listening socket.

Verification and changes:
I used the terminal output and the ss -tlnp command to verify the listening service. When an address-in-use error occurred, I checked the existing Agent process rather than starting another instance on the same port.

Prompt 4: Authentication and TCP Commands

Prompt used:
"Help me troubleshoot authentication and test the RemoteOps commands SYSINFO, LISTPROC, restricted EXEC, and QUIT. The protocol responses should include SID:9073."

Tool used: ChatGPT

How the output was used:
The guidance helped me test authentication and the Controller menu, including system information, process listing, permitted command execution, and session termination.

Verification and changes:
I tested the Controller against the running Agent and checked the returned responses. Authentication was required before protected operations could be performed.

Prompt 5: PUT and GET File Transfer

Prompt used:
"Help me debug PUT and GET file transfer in my C client-server application. The Agent accepts only simple filenames, and I need to upload and download a file without changing its contents."

Tool used: ChatGPT

How the output was used:
The guidance helped me investigate filename validation and the handling of local file paths in the Controller. I modified the upload logic so that the local file could be opened using its path while only the filename was sent to the Agent.

Verification and changes:
I successfully uploaded sample.txt and received the response OK FILE_RECEIVED sample.txt SID:9073. I then downloaded the file and received OK FILE_SEND sample.txt 35 SID:9073. The displayed file content was checked after the transfer.

Prompt 6: UDP Monitoring and Concurrent Controllers

Prompt used:
"Explain how to test UDP monitoring and multiple simultaneous Controller connections for my RemoteOps assignment, including the use of threads and SID-tagged monitoring messages."

Tool used: ChatGPT

How the output was used:
The guidance provided a testing procedure for monitoring, concurrent connections, and checking established TCP sessions.

Verification and changes:
Runtime tests and screenshots are used to document the features that have actually been verified. Any feature not successfully tested must be recorded as incomplete or requiring further investigation.

Prompt 7: Logging and Submission Documentation

Prompt used:
"Help me prepare a README, design diary, test results, AI prompt log, implementation report, and final submission checklist for the IE3090 RemoteOps assignment."

Tool used: ChatGPT

How the output was used:
The guidance helped me organize the project documentation and identify the runtime and source-code screenshots needed for the implementation report.

Verification and changes:
I reviewed the documentation against my actual project files and test results. Results that were not verified were not treated as successful tests.

3. Limitations of AI Assistance

AI-generated suggestions did not always work correctly with the existing source code or protocol implementation. For example, filename validation rejected a full local file path, and authentication had to succeed before file transfer commands could be used. These problems required checking the source code, correcting the Controller input or logic, recompiling, and testing again.

AI suggestions were treated as guidance rather than proof that a feature was implemented correctly. Runtime output and source-code inspection were used to verify the behaviour of the application.

4. Learning Outcomes

Using AI assistance helped me understand the relationship between TCP sockets, client-server communication, authentication, file transfer, and error handling. I also learned the importance of testing changes in the actual environment, checking logs and socket states, validating filenames, and documenting the results of each test. I remain responsible for understanding the code and explaining the final implementation.
