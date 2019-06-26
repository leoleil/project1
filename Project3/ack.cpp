#include "ack.h"

DWORD ack_service(LPVOID lpParameter)
{
	while (1) {
		AckSocket socket;
		socket.createReceiveServer(5000);
	}
	return 0;
}
