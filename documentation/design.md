# System Design

## Agent
The Agent listens for incoming TCP connections and
processes authenticated client requests.

## Controller
The Controller connects to the Agent and sends commands.

## TCP
TCP is used for reliable command communication
and file transfer.

## UDP
UDP is used for monitoring messages.

## Concurrency
The implementation should support multiple controller
connections using concurrent client handling.
