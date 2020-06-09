#define PTW32_STATIC_LIB
#define HAVE_STRUCT_TIMESPEC
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "func.h"
#include <pthread.h>
#pragma comment(lib, "ws2_32.lib")
#define maxusers 50

//‘айл дл€ записи данных пользователей и их сообщений
char* usersFile = (char*)"users.txt";
char* msgsFile = (char*)"messages.txt";

typedef struct client
{
	SOCKET sock;
	int id;
} CLIENT;

//ћассив подключенных пользователей
CLIENT* Clients = (CLIENT*)malloc(sizeof(CLIENT) * maxusers);

pthread_mutex_t mutex;
DATABASE* data = InitDatabase(usersFile, msgsFile);
SOCKET* clients = (SOCKET*)malloc(sizeof(SOCKET) * maxusers);
int* dialogueWith = (int*)malloc(sizeof(int) * maxusers);
int    clientsSize = 0;
char** lastMessages;
int    ClientsSize = 0;

// —ообщени€ дл€ сервера
char* OK = (char*)"1\0";
char* Error = (char*)"0\0";
char* NewMsg = (char*)"!NEWMSG\0";
char* Dialogue = (char*)"!DIALOGUE\0";
char* Chat = (char*)"!CHAT\0";

SOCKET sockID(int id)
{
	for (int i = 0; i < ClientsSize; ++i)
		if (Clients[i].id == id)
			return Clients[i].sock;
}

//функци€ отправки сообщени€ пользователю
int SendMessageToClient(SOCKET client, char* message)
{
	int x = send(client, message, strlen(message), 0);
	if (x == SOCKET_ERROR)
	{
		printf("Ќе получаетс€ отправить сообщение\n");
		closesocket(client);
		return -1;
	}
	return x;
}

//функци€ получени€ сообщени€ от пользовател€
char* RecieveMessage(SOCKET client)
{
	char* message = (char*)malloc(1024);
	int x = SOCKET_ERROR;
	while (x == SOCKET_ERROR)
	{
		//полчение ответа
		x = recv(client, message, 1024, 0);
		//обработка ошибок
		if (x == 0 || x == WSAECONNRESET)
		{
			printf("Connection closed\n");
			break;
		}
		if (x < 0)
			continue;
	}
	message[x] = '\0';
	return message;
}

//кусок кода от ≈лены јлександровны дл€ начала работы с пользователем
void* ClientStart(void* param)
{
	int clientID;
	SOCKET client = (SOCKET)param;
	char recieve[1024], transmit[1024];
	int x;
	x = recv(client, recieve, 1024, 0);//принимаем сообщение
	if (!x || x == SOCKET_ERROR)
	{
		pthread_mutex_lock(&mutex);
		printf("Error getting data\n");
		pthread_mutex_unlock(&mutex);
		return (void*)1;
	}
	recieve[x] = '\0';
	
	//логин
	if (isEqualStr(recieve, (char*)"!LOGIN"))
	{
		while (1)
		{
			sprintf(transmit, "\nEnter the login: \0");
			x = send(client, transmit, sizeof(transmit), 0);
			char* login = RecieveMessage(client);
			printf("Recieved msg: %s\n", login);

			if (!isCorrectLogin(data, login))
			{
				SendMessageToClient(client, Error);
				continue;
			}
			SendMessageToClient(client, OK);

			sprintf(transmit, "\nEnter the password: ");
			x = send(client, transmit, sizeof(transmit), 0);
			printf("pass request sent\n");
			char* password = RecieveMessage(client);
			printf("Recieved msg: %s\n", password);
			if (!isCorrectPassword(data, login, password))
			{
				SendMessageToClient(client, Error);
				continue;
			}
			SendMessageToClient(client, OK);

			sprintf(transmit, "\nSuccess!\n");
			x = send(client, transmit, sizeof(transmit), 0);
			printf("message sent");
			clientID = (*data)[login].id;
			(*data)[login].isOnline = true;
			dialogueWith[clientID] = -1;

			Clients[ClientsSize].sock = client;
			Clients[ClientsSize].id = clientID;
			ClientsSize++;

			break;
		}
	}

	//регистраци€

	if (isEqualStr(recieve, (char*)"!REG"))
	{
		while (1)
		{
			sprintf(transmit, "\nEnter the login: \0");
			x = send(client, transmit, sizeof(transmit), 0);
			char* login = RecieveMessage(client);
			printf("Recieved msg: %s\n", login);

			sprintf(transmit, "\nConfirm the login: \0");
			x = send(client, transmit, sizeof(transmit), 0);
			char* login1 = RecieveMessage(client);
			printf("Recieved msg: %s\n", login1);

			if (!isEqualStr(login, login1))
			{
				SendMessageToClient(client, Error);
				continue;
			}
			SendMessageToClient(client, OK);

			sprintf(transmit, "\nEnter the password: ");
			x = send(client, transmit, sizeof(transmit), 0);
			printf("pass request sent\n");
			char* password = RecieveMessage(client);
			printf("Recieved msg: %s\n", password);

			sprintf(transmit, "\nConfirm the password: ");
			x = send(client, transmit, sizeof(transmit), 0);
			printf("pass request sent\n");
			char* password1 = RecieveMessage(client);
			printf("Recieved msg: %s\n", password);

			if (!isEqualStr(password, password1))
			{
				SendMessageToClient(client, Error);
				continue;
			}
			SendMessageToClient(client, OK);

			AddUser(&data, login, password, usersFile);//добавл€ем нового пользовател€ в базу данных

			sprintf(transmit, "\nSuccess!\n");
			x = send(client, transmit, sizeof(transmit), 0);
			printf("message sent");
			clientID = (*data)[login].id;//достаЄм из базы данных id пользовател€
			(*data)[login].isOnline = true;
			dialogueWith[clientID] = -1;

			Clients[ClientsSize].sock = client;
			Clients[ClientsSize].id = clientID;
			ClientsSize++;

			break;
		}
	}
	//„ат
	RecieveMessage(client);
	while (1)
	{
		//печать основного чата
		lastMessages = GetTenLastGenMessages(data);
		SendMessageToClient(client, NewMsg);
		pthread_mutex_lock(&mutex);
		for (int j = 0; j < 10; ++j)
			SendMessageToClient(client, lastMessages[j]);
		pthread_mutex_unlock(&mutex);

		//основной чат
		while (1)
		{
			//получить сообщение от клиента
			char* req = RecieveMessage(client);
			if (isEqualStr(req, NewMsg))
			{
				char* newmsgtxt = RecieveMessage(client);//принимаем сообщение
				struct tm* u;//узнаЄм врем€
				char* timeS;
				const time_t timer = time(NULL);
				u = localtime(&timer);
				timeS = settime(u);

				AddMessageToGeneralChat(&data, newmsgtxt, clientID, msgsFile);//добавл€ем сообщение в основной чат
				lastMessages = GetTenLastGenMessages(data);//получаем 10 последних сообщений
				for (int i = 0; i < clientsSize; ++i)
				{
					if (dialogueWith[Clients[i].id] == -1)
					{
						SendMessageToClient(clients[i], NewMsg);
						pthread_mutex_lock(&mutex);
						for (int j = 0; j < 10; ++j)
							SendMessageToClient(clients[i], lastMessages[j]);
						pthread_mutex_unlock(&mutex);
					}
				}

			}

			// если !DIALOGUE перейти в другой цикл
			if (isEqualStr(req, Dialogue))
				break;
		}

		//приватный чат

		// прочиать id собеседника
		char* interid = RecieveMessage(client);
		int interlocutorID = atoi(interid);
		dialogueWith[clientID] = interlocutorID;

		//вывести 10 последних сообщений с ним
		int lastMesSize = 0;
		lastMessages = GetTenLastDiaMessages(data, clientID, interlocutorID, lastMesSize);
		if (lastMesSize != 0)
		{
			SendMessageToClient(client, NewMsg);
			pthread_mutex_lock(&mutex);
			char lastmes[1024 * 10];
			lastmes[0] = '\0';
			for (int j = 0; j < 10 && j < lastMesSize; ++j)
			{
				strcat(lastmes, lastMessages[j]);
			}
			SendMessageToClient(client, lastmes);
			pthread_mutex_unlock(&mutex);
		}
		
		while (1)
		{
			//если !NEWMSG прочитать сообщение -> добавить в базу -> отправить 10 последних сообщений
			char* req = RecieveMessage(client);
			if (isEqualStr(req, NewMsg))
			{
				char* newmsgtxt = RecieveMessage(client);
				struct tm* u;
				char* timeS;
				const time_t timer = time(NULL);
				u = localtime(&timer);
				timeS = settime(u);
				AddMessageToDialogue(&data, newmsgtxt, clientID, interlocutorID);

				if (dialogueWith[interlocutorID] == clientID)
				{
					lastMesSize = 0;
					lastMessages = GetTenLastDiaMessages(data, clientID, interlocutorID, lastMesSize);
					SendMessageToClient(sockID(interlocutorID), NewMsg);
					char lastmes[1024 * 10];
					lastmes[0] = '\0';
					pthread_mutex_lock(&mutex);
					for (int j = 0; j < 10 && j < lastMesSize; ++j)
						strcat(lastmes, lastMessages[j]);
					SendMessageToClient(sockID(interlocutorID), lastmes);
					pthread_mutex_unlock(&mutex);
				}



				lastMesSize = 0;
				lastMessages = GetTenLastDiaMessages(data, clientID, interlocutorID, lastMesSize);
				SendMessageToClient(client, NewMsg);
				pthread_mutex_lock(&mutex);
				char lastmes[1024 * 10];
				lastmes[0] = '\0';
				for (int j = 0; j < 10 && j < lastMesSize; ++j)
					strcat(lastmes, lastMessages[j]);
				SendMessageToClient(client, lastmes);
				pthread_mutex_unlock(&mutex);
			}
			//≈сли !CHAT вернутьс€ в общий чат
			else if (isEqualStr(req, Chat))
			{
				dialogueWith[clientID] = -1;
				break;
			}
		}
	}
	return (void*)0;
}

//создаЄм сервер (тоже из файлов ≈лены јлександровны)
int CreateServer()
{
	SOCKET server, client;
	sockaddr_in localaddr, clientaddr;
	int size;
	server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (server == INVALID_SOCKET)
	{
		printf("Error create server\n");
		return 1;
	}
	localaddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(5510);
	if (bind(server, (sockaddr*)&localaddr, sizeof(localaddr)) == SOCKET_ERROR)
	{
		printf("Can't start server\n");
		return 2;
	}
	else
	{
		printf("Server is started\n");
	}
	listen(server, 50);//50 клиентов в очереди могут сто€ть
	pthread_mutex_init(&mutex, NULL);
	while (1)
	{
		size = sizeof(clientaddr);
		client = accept(server, (sockaddr*)&clientaddr, &size);
		if (client == INVALID_SOCKET)
		{
			printf("Error accept client\n");
			continue;
		}
		clients[clientsSize] = client;
		clientsSize++;
		pthread_t mythread;
		int status = pthread_create(&mythread, NULL, ClientStart, (void*)client);
		pthread_detach(mythread);
	}
	pthread_mutex_destroy(&mutex);
	printf("Server is stopped\n");
	closesocket(server);
	return 0;
}

int main()
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(1, 1), &wsd) == 0)
	{
		printf("Connected to socket lib\n");
	}
	else
	{
		return 1;
	}
	int err = 0;
	if (err = CreateServer())
	{
		return err;
	}
	return 0;
}