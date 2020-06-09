#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <windows.h>

pthread_mutex_t mutex;

//сообщения для сервера
char* OK = (char*)"1\0";
char* Error = (char*)"0\0";
char* NewMsg = (char*)"!NEWMSG\0";
char* Dialogue = (char*)"!DIALOGUE\0";
char* Chat = (char*)"!CHAT\0";

//функция удаления элемента из строки
void StrErase(char* source, int index)
{
	int i = 0;
	while (source[i] != '\0')
	{
		source[i] = source[i + 1];
		++i;
	}
}

//функция сравнивания строк
bool isEqualStr(char* str1, char* str2)
{
	if (strcmp(str1, str2) != 0)
		return false;
	for (int i = 0; i < strlen(str1); ++i)
		if (str1[i] != str2[i]) return false;
	return true;
}

//функция отправки сообщения на сервер
int SendMessageToServer(SOCKET client, char* message)
{
	int ret = send(client, message, strlen(message), 0);
	if (ret == SOCKET_ERROR)
	{
		printf("Can't send message\n");
		closesocket(client);
		return -1;
	}
	return ret;
}

//функция получения сообщения от сервера
char* RecieveMessage(SOCKET client)
{
	char* message = (char*)malloc(1024);
	int ret = SOCKET_ERROR;
	while (ret == SOCKET_ERROR)
	{
		//полчение ответа
		ret = recv(client, message, 1024, 0);
		//обработка ошибок
		if (ret == 0 || ret == WSAECONNRESET)
		{
			printf("Connection closed\n");
			break;
		}
		if (ret < 0)
			continue;
	}
	message[ret] = '\0';
	return message;
}

//функция потока чтения и отправки сообщений на сервер
void* WriteThread(void* param)
{

	printf("joined writing thread\n");
	char tmp[1024];
	fgets(tmp, 1024, stdin);
	SOCKET client = (SOCKET)param;
	while (1)
	{
		char msg[1024];

		fgets(msg, 1024, stdin);
		msg[strlen(msg) - 1] = '\0';
		if (msg[0] == '!')
		{
			StrErase(msg, 0);
			char intID[5];
			intID[0] = '\0';
			strcat(intID, msg);
			SendMessageToServer(client, Dialogue);
			SendMessageToServer(client, intID);
		}
		else if (isEqualStr(msg, (char*)"CHAT\0"))
		{
			SendMessageToServer(client, Chat);
		}
		else
		{
			SendMessageToServer(client, NewMsg);
			SendMessageToServer(client, msg);
		}
	}
}

//функция чтения и печати сообщений от сервера
void* ReadThread(void* param)
{

	printf("joined reading thread\n");
	SOCKET client = (SOCKET)param;
	SendMessageToServer(client, OK);
	while (1)
	{
		char* recieved = RecieveMessage(client);
		if (isEqualStr(recieved, NewMsg))
		{
			system("cls");
			char* recMsg = RecieveMessage(client);
			printf("Type CHAT to get back to the general chat\n Type !n to get to the private chat with n user (e.g. !1)\n");
			printf("%s", recMsg);
		}
	}
}

//функция авторизации пользователя
void Login(SOCKET& client)
{

	char message[1024];
	sprintf(message, "!LOGIN");
	int x = SendMessageToServer(client, message);

	while (1)
	{
		char* recievedMsg = RecieveMessage(client);

		printf("%s\n", recievedMsg);

		char login[30];
		char password[30];

		scanf("%s", &login);

		x = SendMessageToServer(client, login);
		recievedMsg = RecieveMessage(client);
		if (!isEqualStr(recievedMsg, OK))
		{
			printf("Wrong login\n");
			continue;
		}

		recievedMsg = RecieveMessage(client);
		printf("%s\n", recievedMsg);

		scanf("%s", &password);
		x = SendMessageToServer(client, password);

		recievedMsg = RecieveMessage(client);
		if (!isEqualStr(recievedMsg, OK))
		{
			printf("Wrong password\n");
			continue;
		}
		recievedMsg = RecieveMessage(client);
		printf("%s\n", recievedMsg);
		break;
	}

}

//функция регистрации нового пользователя
void Registration(SOCKET& client)
{

	char message[1024];
	sprintf(message, "!REG");
	int x = SendMessageToServer(client, message);

	while (1)
	{
		char* recievedMsg = RecieveMessage(client);

		printf("%s\n", recievedMsg);

		char login[30];
		char password[30];

		scanf("%s", &login);
		x = SendMessageToServer(client, login);
		recievedMsg = RecieveMessage(client);

		printf("%s\n", recievedMsg);

		scanf("%s", &login);
		x = SendMessageToServer(client, login);
		recievedMsg = RecieveMessage(client);

		if (!isEqualStr(recievedMsg, OK))
		{
			printf("Wrong login\n");
			continue;
		}

		recievedMsg = RecieveMessage(client);
		printf("%s\n", recievedMsg);

		scanf("%s", &password);
		x = SendMessageToServer(client, password);

		recievedMsg = RecieveMessage(client);
		printf("%s\n", recievedMsg);

		scanf("%s", &password);
		x = SendMessageToServer(client, password);

		recievedMsg = RecieveMessage(client);
		if (!isEqualStr(recievedMsg, OK))
		{
			printf("Wrong password\n");
			continue;
		}
		recievedMsg = RecieveMessage(client);
		printf("%s\n", recievedMsg);
		break;
	}

}

//основная функция
void MainChat(SOCKET& client)
{
	pthread_mutex_init(&mutex, NULL);
	pthread_t mythread;
	int status = pthread_create(&mythread, NULL, WriteThread, (void*)client);

	pthread_t mythread1;
	int status1 = pthread_create(&mythread1, NULL, ReadThread, (void*)client);
	pthread_join(mythread1, NULL);
	pthread_join(mythread, NULL);


	while (1)
	{
		int a = 0;
	}

}

void StartClient()
{
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (client == INVALID_SOCKET)
	{
		printf("Error create socket\n");
		return;
	}
	sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(5510); 
	server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); 
	if (connect(client, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Can't connect to server\n");
		closesocket(client);
		return;
	}

	printf("Type 0 to sign in\n Type 1 to sign up\n");
	int action;
	scanf("%d", &action);
	if (action == 0)
		Login(client);
	else
		Registration(client);

	MainChat(client);

}

int main()
{
	WSADATA wsd;
	if (WSAStartup(MAKEWORD(1, 1), &wsd) != 0)
	{
		printf("Can't connect to socket lib");
		return 1;
	}

	StartClient();

	int a;
	scanf("%d", &a);
	return 0;
}