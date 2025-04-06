# Protocol

Name: TBD

The protocol used the both the server and the clients for communication is very loosely based on HTTP, in the sense that we have general commands such as GET, POST, PUT and DELETE. However, we do other steps in a different way.

The general syntax for a message in the protocol is as follows:

\[COMMAND\]:\[ARGUMENT 1\]:\[ARGUMENT 2\]:\[ARGUMENT 3\]:\[...\]

COMMAND is one of the 4 options: GET (to retrieve data), POST (to create data), PUT (to update data), and DELETE (to delete data).

ARGUMENT X, where X is a number >= 1, is one of the arguments for the given command. If an argument uses a semicolon, it must first add a slash ('\') right before it. The client automatically does this for the user.

This is used only for communication and server-side operations. The user does not need to input their request in this format for the client to read. Instead, they can only input the relevant information requested by the client, and the cliend will encode this data. If the data is out of format, the client will refuse to send it altogether. The server also verifies if the data is correct as to ensure no bad actors try to send invalid requests without use of the client.