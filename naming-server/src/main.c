#define HASHMAP_IMPL
#include "../inc/base.h"
#include "../inc/server.h"

#include "../inc/hashmap.h"

int main()
{
	ServerData *sData = CreateServer();
	while (1)
	{
		RunServer(sData);
	}
	CloseServer(sData);
	return 0;
}