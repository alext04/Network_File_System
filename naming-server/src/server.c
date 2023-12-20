#include "../inc/base.h"
#include "../inc/server.h"

ServerData *CreateServer()
{
	ServerData *sData = malloc(sizeof(ServerData));

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		sData->clientData[i] = NULL;
		sData->ss_List[i] = NULL;
	}

	sData->file_ht = CreateHashTable();
	sData->client_ht = CreateHashTable();

	sData->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sData->sockfd < 0)
	{
		printf("[-]Error in connection.\n");
		exit(1);
	}
	sData->numServers = 0;
	// setsockopt(sData->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	// setsockopt(sData->sockfd, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));
	printf("[+]Server Socket is created. %d \n", sData->sockfd);

	memset(&sData->serverAddr, '\0', sizeof(sData->serverAddr));
	sData->serverAddr.sin_family = AF_INET;
	sData->serverAddr.sin_port = PORT;
	sData->serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	for (int i = 0; i < MAX_CLIENTS; i++) // Initialise the client sockets to 0
	{
		sData->clientData[i] = NULL;
	}
	int ret = bind(sData->sockfd, (struct sockaddr *)&sData->serverAddr, sizeof(sData->serverAddr));
	if (ret < 0)
	{
		perror("[-]Error in binding ");
		exit(1);
	}
	printf("[+]Bind to port %d\n", PORT);

	if (listen(sData->sockfd, MAX_CLIENTS) == 0)
	{
		printf("[+]Listening....\n");
	}
	else
	{
		printf("[-]Error in listening.\n");
	}
	return sData;
}

void RunServer(ServerData *sData)
{
	fd_set readfds;	   // File descriptor set to handle all fds.
	FD_ZERO(&readfds); // Clear the fd_set

	uint8_t buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);
	// printf("Setting %d\n", sData->sockfd);
	FD_SET(sData->sockfd, &readfds); // Insert the master socket into the fd-set
	int maxsd = sData->sockfd;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (sData->clientData[i] == NULL)
			continue;

		int sd = sData->clientData[i]->sockfd;
		if (sd > 0)
			FD_SET(sData->clientData[i]->sockfd, &readfds); // Insert the client to fd-set if a valid socket and not 0
		if (sd > maxsd)
		{ // get the highest descriptor number for first argument of select function
			maxsd = sd;
		}
	}

	int socketcount = select(maxsd + 1, &readfds, NULL, NULL, NULL);

	if (socketcount < 1)
	{
		printf("[-]Error in select");
	}
	if (FD_ISSET(sData->sockfd, &readfds))
	{ // Check if the activity is on master socket for incoming connection
		printf("Helllo\n");
		struct sockaddr_in newAddr;
		socklen_t addr_size;
		int newSocket = accept(sData->sockfd, (struct sockaddr *)&newAddr, &addr_size);
		if (newSocket < 0)
		{
			perror("[-]Error in accepting ");
			exit(1);
		}
		printf("[+]Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (sData->clientData[i] == NULL)
			{
				sData->clientData[i] = malloc(sizeof(ClientData));
				sData->clientData[i]->sockfd = newSocket;
				memcpy(&sData->clientData[i]->addr, &newAddr, addr_size);
				break;
			}
		}
	}
	else
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (sData->clientData[i] == NULL)
				continue;
			int sd = sData->clientData[i]->sockfd;
			if (FD_ISSET(sd, &readfds))
			{
				int bytesize = recv(sd, buffer, BUFFER_SIZE, 0);
				HandleMessage(sData, sData->clientData[i], buffer, bytesize);
				if (bytesize == 0)
				{
					close(sd);
					// free(sData->clientData[i]);
					sData->clientData[i] = NULL;
				}
			}
		}
	}
}

ClientData *getSS(ServerData *sData, ClientData *cData)
{
	int ss_id = -1;
	HM_Node *node = sData->client_ht->arr[cData->id % MOD];
	int j = 0;
	while (node != NULL)
	{
		// printf("\n%d\n",j++);
		if (node->hash == cData->id)
		{
			ss_id = node->idx;
			break;
		}
		node = node->next;
	}
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (sData->clientData[i] && sData->clientData[i]->cType == STORAGE && sData->clientData[i]->id == ss_id)
		{

			return sData->clientData[i];
		}
	}
	return NULL;
	// return sData->clientData[0];
}

ClientData *getSS2(ServerData *sData, ClientData *cData)
{
	int ss_id = -1;
	HM_Node *node = sData->client_ht->arr[cData->id % MOD];
	int j = 0;
	int flag = 0;
	while (node != NULL)
	{
		// printf("\n%d\n",j++);
		if (node->hash == cData->id)
		{
			ss_id = node->idx;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (sData->clientData[i] && sData->clientData[i]->cType == STORAGE && sData->clientData[i]->id == ss_id)
				{
					if (!flag)
					{
						flag = 1;
						break;
					}
					else
					{

						return sData->clientData[i];
					}
				}
			}
		}
		node = node->next;
	}
	return NULL;
	// return sData->clientData[0];
}

void HandleMessage(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	printf("Handling message : %d\n", buffer[0]);
	switch (buffer[0])
	{
	case 0x01:
		MH_InitConn(sData, cData, buffer, buflen);
		break;
	case 0x02:
		MH_ListDir(sData, cData, buffer, buflen);
		break;
	case 0x09:
		MH_GetFileLoc(sData, cData, buffer, buflen);
		break;
	case 0x13:
		MH_CreateFile(sData, cData, buffer, buflen);
		break;
	case 0x14:
		MH_CreateDir(sData, cData, buffer, buflen);
		break;
	case 0x15:
		MH_DeleteFile(sData, cData, buffer, buflen);
		break;
	case 0x16:
		MH_DeleteDir(sData, cData, buffer, buflen);
		break;
	case 0x17:
		MH_CopyFile(sData, cData, buffer, buflen);
		break;
	case 0x18:
		MH_CopyDir(sData, cData, buffer, buflen);
		break;
	default:
		break;
	}
}

int Ping(ServerData *sData, ClientData *cData)
{
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 250000;
	int cSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (cSocket < 0)
	{
		perror("[-]Ping error, creating socket");
		return 0;
	}
	if (setsockopt(cSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout,
				   sizeof(timeout)) < 0)
		perror("setsockopt failed\n");

	if (setsockopt(cSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout,
				   sizeof(timeout)) < 0)
		perror("setsockopt failed\n");

	if (connect(cSocket, (struct sockaddr *)&cData->addr, sizeof(cData->addr)) < 0)
	{
		perror("[-]Ping error, connecting");
		return 0;
	}
	int pingBuffer[2];
	pingBuffer[0] = 0x1b;

	if (send(cSocket, pingBuffer, 1, 0) < 0)
	{
		perror("[-]Error Sending ping ");
		return 0;
	}
	if (recv(cSocket, pingBuffer, 1, 0) < 0)
	{
		perror("[-]Error Receiving ping");
		return 0;
	}
	if (pingBuffer[0] != 0x9b)
	{
		perror("[-]Error Receiving ping, wrong code");
		return 0;
	}
	return 1;
}

void SendMessage(ClientData *cData, uint8_t *buffer, int buflen)
{
	send(cData->sockfd, buffer, buflen, 0);
}

void CloseServer(ServerData *sData)
{
	close(sData->sockfd);
	free(sData);
}

// Message Handlers
void MH_InitConn(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	if (buffer[1] == 0)
	{
		cData->cType = USER;
		int ss_count = 0;
		cData->id = *((int *)(buffer + 2));
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (sData->clientData[i] != NULL && sData->clientData[i]->cType == STORAGE)
				ss_count++;
		}

		int final_i = -1;
		if (ss_count == 0)
		{
			printf("[-] No storage server initialized\n");
			// Error
			return;
		}
		int ss_idx = cData->id % ss_count;
		int ss_idx2 = (ss_idx + 1) % ss_count;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (sData->clientData[i] != NULL && sData->clientData[i]->cType == STORAGE)
			{
				if (ss_idx == 0)
				{
					final_i = i;
					break;
				}
				ss_idx--;
			}
		}
		InsertNodeDirect(sData->client_ht, cData->id, sData->clientData[final_i]->id);

		if (ss_count >= REDUNDANCY_CUTOFF)
		{

			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (sData->clientData[i] != NULL && sData->clientData[i]->cType == STORAGE)
				{
					if (ss_idx2 == 0)
					{
						final_i = i;
						break;
					}
					ss_idx2--;
				}
			}
			InsertNodeDirect(sData->client_ht, cData->id, sData->clientData[final_i]->id);
		}
	}
	else if (buffer[1] == 1)
	{
		cData->cType = STORAGE;
		cData->port_1 = *((int *)(buffer + 2));
		cData->port_2 = *((int *)(buffer + 6));
		cData->id = hash(inet_ntoa(cData->addr.sin_addr)) + ntohs(cData->addr.sin_port);

		int j = 0;
		while (sData->ss_List[j] != NULL && sData->ss_List[j]->id != cData->id)
		{
			j++;
		}
		sData->ss_List[j] = cData;

		j = 0;
		while (sData->ss_List[j] != NULL)
		{
			j++;
		}
		sData->numServers = j;

		printf("Received SS connection : %d, %d\n", cData->port_1, cData->port_2);
		printf("Totally %d SS\n", sData->numServers);
	}
	printf("[+] Client id : %d\n", cData->id);

	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x81;
	buffer[1] = 0;
	SendMessage(cData, buffer, 2);
}

void MH_ListDir(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	int N_entr = *((int *)(buffer + 1));
	printf("[+] Receiving File List : %d\n", N_entr);
	int offset = 5;
	for (int i = 0; i < N_entr; i++)
	{
		int hash = *((int *)(buffer + offset));
		printf("[+] Hash recd : %d\n", hash);
		InsertNodeDirect(sData->file_ht, hash, cData->id);

		offset += 4;
	}
	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x82;
	buffer[1] = 0;
	SendMessage(cData, buffer, 2);

	// Change port to the listening port to send all other messages
	printf("SS_port = %d\n", cData->port_1);
	cData->addr.sin_port = cData->port_1;
}

void MH_GetFileLoc(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	int pathLen = *((int *)(buffer + 1));
	char *path_raw = strdup(buffer + 5);
	char *path_final = malloc((pathLen + 20) * sizeof(char));
	int offset = snprintf(path_final, pathLen + 20, "%d/", cData->id);
	strcpy(path_final + offset, path_raw);
	int final_hash = hash(path_final);

	for (int i = 0; i < 20; i++)
	{
		printf("%d\t", buffer[i]);
	}

	printf("Get File Loc : %d %s \n", pathLen, path_raw);

	int ss_idx = -1;
	int ss_id = -1;
	HM_Node *node = sData->file_ht->arr[final_hash % MOD];
	while (node != NULL)
	{
		if (node->hash == final_hash)
		{

			ss_id = node->idx;
			ss_idx = -1;
			for (int i = 0; i < MAX_CLIENTS; i++)
			{
				if (sData->clientData[i] == NULL)
					continue;
				if (sData->clientData[i]->id == ss_id)
				{
					if (Ping(sData, sData->clientData[i]))
					{
						ss_idx = i;
						break;
					}
				}
			}
		}

		node = node->next;
	}
	printf("%d\n", final_hash);
	if (ss_id == -1)
	{
		printf("Can't Find File\n");
	}

	// printf("Server Selected : %d %d %d\n", cData->id, sData->clientData[ss_idx]->addr.sin_addr.s_addr, sData->clientData[ss_idx]->port_2);
	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x89;
	buffer[1] = 0;
	*((int *)(buffer + 2)) = cData->id;
	*((int *)(buffer + 6)) = sData->clientData[ss_idx]->addr.sin_addr.s_addr;
	*((int *)(buffer + 10)) = sData->clientData[ss_idx]->port_2;
	SendMessage(cData, buffer, 15);
}

void MH_CreateFile(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	int pathLen = *((int *)(buffer + 1));
	char *path_raw = strdup(buffer + 5);
	char *path_final = malloc((pathLen + 20) * sizeof(char));
	int offset = snprintf(path_final, pathLen + 20, "%d/", cData->id);
	strcpy(path_final + offset, path_raw);
	int final_hash = hash(path_final);

	ClientData *ssDataList[2];
	ssDataList[0] = getSS(sData, cData);
	ssDataList[1] = getSS2(sData, cData);
	int found_server = 0;
	int return_code = 0;

	for (int j = 0; j < 2; j++)
	{
		if (ssDataList[j] == NULL)
			continue;
		ClientData *ssData = ssDataList[j];
		if (!Ping(sData, ssData))
		{
			continue;
		}
		found_server = 1;
		printf("[+] Creating file : %s \n", path_final);

		int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(ss_fd, (struct sockaddr *)&ssData->addr, sizeof(ssData->addr)) < 0)
		{
			perror("[-] Error connecting to ss ");
		}
		bzero(buffer, BUFFER_SIZE);
		buffer[0] = 0x03;
		*((int *)(buffer + 1)) = strlen(path_final);
		strcpy(buffer + 5, path_final);

		if (send(ss_fd, buffer, 1 + 4 + strlen(path_final) + 1, 0) < 0)
		{
			perror("[-]Send error to SS, CreateFile ");
			exit(1);
		}

		if (recv(ss_fd, buffer, BUFFER_SIZE, 0) < 0)
		{
			perror("[-]Recv error from SS, CreateFile ");
			exit(1);
		}

		return_code = 0;
		if (buffer[0] != 0x83 || buffer[1] != 0x0)
		{
			perror("[-] Recv error from SS, CreateFile (2) ");
			return_code = 1;
			// exit(1);
		}
		InsertNodeDirect(sData->file_ht, final_hash, ssData->id);
		close(ss_fd);
	}
	if (found_server == 0)
	{
		printf("[-] Couldnt find server \n");
		return_code = 1;
	}
	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x93;
	buffer[1] = return_code;
	// buffer[1] = 0x0;

	if (send(cData->sockfd, buffer, 2, 0) < 0)
	{
		perror("[-]Send error to SS, CreateFile ");
		exit(1);
	}
}

void MH_CreateDir(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	int pathLen = *((int *)(buffer + 1));
	char *path_raw = strdup(buffer + 5);
	char *path_final = malloc((pathLen + 20) * sizeof(char));
	int offset = snprintf(path_final, pathLen + 20, "%d/", cData->id);
	printf("Raw Path %s\n", path_raw);
	strcpy(path_final + offset, path_raw);

	int final_hash = hash(path_final);

	ClientData *ssDataList[2];
	ssDataList[0] = getSS(sData, cData);
	ssDataList[1] = getSS2(sData, cData);
	int found_server = 0;
	int return_code = 0;
	for (int j = 0; j < 2; j++)
	{
		if (ssDataList[j] == NULL)
			continue;
		ClientData *ssData = ssDataList[j];
		if (!Ping(sData, ssData))
		{
			continue;
		}
		found_server = 1;
		printf("[+] Creating dir : %s\n", path_final);
		int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(ss_fd, (struct sockaddr *)&ssData->addr, sizeof(ssData->addr)) < 0)
		{
			perror("[-] Error connecting to ss ");
		}
		printf(" > %d %d \n", sData->sockfd, ss_fd);

		bzero(buffer, BUFFER_SIZE);
		buffer[0] = 0x04;
		*((int *)(buffer + 1)) = strlen(path_final);
		strcpy(buffer + 5, path_final);

		if (send(ss_fd, buffer, 1 + 4 + strlen(path_final) + 1, 0) < 0)
		{
			perror("[-]Send error to SS, CreateDir ");
			exit(1);
		}

		if (recv(ss_fd, buffer, BUFFER_SIZE, 0) < 0)
		{
			perror("[-]Recv error from SS, CreateDir ");
			exit(1);
		}

		int return_code = 0;
		if (buffer[0] != 0x84 || buffer[1] != 0x0)
		{
			perror("[-] Recv error from SS, CreateDir (2) ");
			// exit(1);
			return_code = 1;
		}
		close(ss_fd);

		InsertNodeDirect(sData->file_ht, final_hash, ssData->id);
	}
	if (found_server == 0)
	{
		printf("[-] Couldnt find server \n");
		return_code = 1;
	}
	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x94;
	buffer[1] = return_code;

	if (send(cData->sockfd, buffer, 2, 0) < 0)
	{
		perror("[-]Send error to SS, CreateDir ");
		exit(1);
	}
}

void MH_DeleteFile(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	int pathLen = *((int *)(buffer + 1));
	char *path_raw = strdup(buffer + 5);

	char *path_final = malloc((pathLen + 20) * sizeof(char));
	int offset = snprintf(path_final, pathLen + 20, "%d/", cData->id);
	strcpy(path_final + offset, path_raw);
	int final_hash = hash(path_final);

	ClientData *ssDataList[2];
	ssDataList[0] = getSS(sData, cData);
	ssDataList[1] = getSS2(sData, cData);
	int found_server = 0;
	int return_code = 0;

	for (int j = 0; j < 2; j++)
	{
		if (ssDataList[j] == NULL)
			continue;
		ClientData *ssData = ssDataList[j];
		if (!Ping(sData, ssData))
		{
			continue;
		}
		found_server = 1;

		printf("[+] Deleting file : %s \n", path_final);

		int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(ss_fd, (struct sockaddr *)&ssData->addr, sizeof(ssData->addr)) < 0)
		{
			perror("[-] Error connecting to ss ");
		}

		// ss_fd=1;
		bzero(buffer, BUFFER_SIZE);
		buffer[0] = 0x05;
		*((int *)(buffer + 1)) = strlen(path_final);
		strcpy(buffer + 5, path_final);

		if (send(ss_fd, buffer, 1 + 4 + strlen(path_final) + 1, 0) < 0)
		{
			perror("[-]Send error to SS, DeleteFile ");
			exit(1);
		}

		if (recv(ss_fd, buffer, BUFFER_SIZE, 0) < 0)
		{
			perror("[-]Recv error from SS, DeleteFile ");
			exit(1);
		}

		close(ss_fd);
		int return_code = 0;
		if (buffer[0] != 0x85 || buffer[1] != 0x00)
		{
			perror("[-] Recv error from SS, DeleteFile (2) ");
			return_code = 1;
			// exit(1);
		}
	}
	if (found_server == 0)
	{
		printf("[-] Couldnt find server \n");
		return_code = 1;
	}
	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x95;
	buffer[1] = return_code;

	if (send(cData->sockfd, buffer, 2, 0) < 0)
	{
		perror("[-]Send error to SS, DeleteFile ");
		exit(1);
	}
}

void MH_DeleteDir(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{

	int pathLen = *((int *)(buffer + 1));
	char *path_raw = strdup(buffer + 5);
	char *path_final = malloc((pathLen + 20) * sizeof(char));
	int offset = snprintf(path_final, pathLen + 20, "%d/", cData->id);
	strcpy(path_final + offset, path_raw);
	int final_hash = hash(path_final);

	ClientData *ssDataList[2];
	ssDataList[0] = getSS(sData, cData);
	ssDataList[1] = getSS2(sData, cData);
	int found_server = 0;
	int return_code = 0;

	for (int j = 0; j < 2; j++)
	{
		if (ssDataList[j] == NULL)
			continue;
		ClientData *ssData = ssDataList[j];
		if (!Ping(sData, ssData))
		{
			continue;
		}
		found_server = 1;

		printf("[+] Deleting Directory : %s \n", path_final);

		int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (connect(ss_fd, (struct sockaddr *)&ssData->addr, sizeof(ssData->addr)) < 0)
		{
			perror("[-] Error connecting to ss ");
		}

		bzero(buffer, BUFFER_SIZE);
		buffer[0] = 0x06;
		*((int *)(buffer + 1)) = strlen(path_final);
		strcpy(buffer + 5, path_final);

		if (send(ss_fd, buffer, 1 + 4 + strlen(path_final) + 1, 0) < 0)
		{
			perror("[-]Send error to SS, DeleteDir ");
			exit(1);
		}

		if (recv(ss_fd, buffer, BUFFER_SIZE, 0) < 0)
		{
			perror("[-]Recv error from SS, DeleteDir ");
			exit(1);
		}

		int return_code = 0;
		if (buffer[0] != 0x86 || buffer[1] != 0x0)
		{
			perror("[-] Recv error from SS, DeleteDir (2) ");
			// exit(1);
			return_code = 1;
		}
		close(ss_fd);

		RemoveNode(sData->file_ht, final_hash);
	}
	if (found_server == 0)
	{
		printf("[-] Couldnt find server \n");
		return_code = 1;
	}
	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x96;
	buffer[1] = return_code;
	// buffer[1] = 0x0;

	if (send(cData->sockfd, buffer, 2, 0) < 0)
	{
		perror("[-]Send error to SS, DeleteDir ");
		exit(1);
	}
}

void MH_CopyFile(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{

	int srcPathLen = *((int *)(buffer + 1));
	char *srcPath_raw = strdup(buffer + 1 + 4);
	char *srcPath_final = malloc((srcPathLen + 20) * sizeof(char));
	int offset = snprintf(srcPath_final, srcPathLen + 20, "%d/", cData->id);
	strcpy(srcPath_final + offset, srcPath_raw);

	int destPathLen = *((int *)(buffer + 1 + 4 + srcPathLen + 1));
	char *destPath_raw = strdup(buffer + 1 + 4 + srcPathLen + 1 + 4);
	char *destPath_final = malloc((destPathLen + 20) * sizeof(char));
	offset = snprintf(destPath_final, destPathLen + 20, "%d/", cData->id);
	strcpy(destPath_final + offset, destPath_raw);

	int final_hash = hash(destPath_final);
	printf("fuwguiw\n");

	ClientData *ssData = getSS(sData, cData);

	//
	printf("[+] Copy File : %s to %s , %d \n", srcPath_final, destPath_final, sData->serverAddr.sin_addr.s_addr);

	int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(ss_fd, (struct sockaddr *)&ssData->addr, sizeof(ssData->addr)) < 0)
	{
		perror("[-] Error connecting to ss ");
	}

	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x07;
	int buf_idx = 1;
	*((int *)(buffer + buf_idx)) = strlen(srcPath_final);
	buf_idx += 4;
	strcpy(buffer + buf_idx, srcPath_final);
	buf_idx += strlen(srcPath_final) + 1;
	*((int *)(buffer + buf_idx)) = sData->serverAddr.sin_addr.s_addr;
	buf_idx += 4;
	*((int *)(buffer + buf_idx)) = ssData->addr.sin_port;
	buf_idx += 4;
	*((int *)(buffer + buf_idx)) = strlen(destPath_final);
	buf_idx += 4;
	strcpy(buffer + buf_idx, destPath_final);
	buf_idx += strlen(destPath_final) + 1;

	if (send(ss_fd, buffer, buf_idx, 0) < 0)
	{
		perror("[-]Send error to SS, CopyFile ");
		exit(1);
	}
	if (recv(ss_fd, buffer, BUFFER_SIZE, 0) < 0)
	{
		perror("[-]Recv error from SS, CopyFile ");
		exit(1);
	}
	int return_code = 0;
	if (buffer[0] != 0x87 || buffer[1] != 0x0)
	{
		perror("[-] Recv error from SS, CopyFile (2) ");
		// exit(1);
		return_code = 1;
	}
	close(ss_fd);
	InsertNodeDirect(sData->file_ht, final_hash, ssData->id);

	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x97;
	buffer[1] = return_code;

	if (send(cData->sockfd, buffer, 2, 0) < 0)
	{
		perror("[-]Send error to SS, CopyFile ");
		exit(1);
	}
}

void MH_CopyDir(ServerData *sData, ClientData *cData, uint8_t *buffer, int buflen)
{
	int srcPathLen = *((int *)(buffer + 1));
	char *srcPath_raw = strdup(buffer + 1 + 4);
	char *srcPath_final = malloc((srcPathLen + 20) * sizeof(char));
	int offset = snprintf(srcPath_final, srcPathLen + 20, "%d/", cData->id);
	strcpy(srcPath_final + offset, srcPath_raw);

	int destPathLen = *((int *)(buffer + 1 + 4 + srcPathLen + 1));
	char *destPath_raw = strdup(buffer + 1 + 4 + srcPathLen + 1 + 4);
	char *destPath_final = malloc((destPathLen + 20) * sizeof(char));
	offset = snprintf(destPath_final, destPathLen + 20, "%d/", cData->id);
	strcpy(destPath_final + offset, destPath_raw);

	int final_hash = hash(destPath_final);

	ClientData *ssData = getSS(sData, cData);

	printf("[+] Copy Directory : %s to %s ", srcPath_final, destPath_final);

	int ss_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(ss_fd, (struct sockaddr *)&ssData->addr, sizeof(ssData->addr)) < 0)
	{
		perror("[-] Error connecting to ss ");
	}

	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x08;
	int buf_idx = 1;
	*((int *)(buffer + buf_idx)) = strlen(srcPath_final);
	buf_idx += 4;
	strcpy(buffer + buf_idx, srcPath_final);
	buf_idx += strlen(srcPath_final) + 1;
	*((int *)(buffer + buf_idx)) = sData->serverAddr.sin_addr.s_addr;
	buf_idx += 4;
	*((int *)(buffer + buf_idx)) = ssData->addr.sin_port;
	buf_idx += 4;
	*((int *)(buffer + buf_idx)) = strlen(destPath_final);
	buf_idx += 4;
	strcpy(buffer + buf_idx, destPath_final);
	buf_idx += strlen(destPath_final) + 1;

	if (send(ss_fd, buffer, buf_idx, 0) < 0)
	{
		perror("[-]Send error to SS, CopyDir ");
		exit(1);
	}

	if (recv(ss_fd, buffer, BUFFER_SIZE, 0) < 0)
	{
		perror("[-]Recv error from SS, CopyDir ");
		exit(1);
	}
	int return_code = 0;
	if (buffer[0] != 0x88 || buffer[1] != 0x0)
	{
		perror("[-] Recv error from SS, CopyDir (2) ");
		// exit(1);
		return_code = 1;
	}
	close(ss_fd);
	InsertNodeDirect(sData->file_ht, final_hash, ssData->id);

	bzero(buffer, BUFFER_SIZE);
	buffer[0] = 0x98;
	buffer[1] = return_code;

	if (send(cData->sockfd, buffer, 2, 0) < 0)
	{
		perror("[-]Send error to SS, CopyDir ");
		exit(1);
	}
}
