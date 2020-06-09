#pragma once
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct message;

typedef struct User
{
	unsigned int id;
	char* login = (char*)malloc(sizeof(char) * 30);
	char* password = (char*)malloc(sizeof(char) * 30);
	message* messages;
	unsigned int msgAmount;
	bool isOnline = false;
} USER;

struct message
{
	char* text = (char*)malloc(sizeof(char) * 1024);
	char* time = (char*)malloc(sizeof(char) * 50);
	USER* sender;
	USER* recipient;
	int senderID;
	int recipientID;
};


typedef struct database
{
	USER** users = (USER**)malloc(sizeof(USER*) * 50);
	message* generalChat;
	unsigned int size;
	unsigned int msgAmount;
	USER& operator[](unsigned int id);
	USER& operator[](char* login);
} DATABASE;

char* settime(struct tm* u);
void ParseStringUser(char* str, int* id, char** log, char** pass);
void ParseStringMessage(char* str, char** date, int* idS, int* idR, char** txt);
//int StrToInt(char* str);
char* IntToStr(int num);
void StrErase(char* source, int index);
unsigned int SizeOfStr(char* str);
USER* InitUser(unsigned int id, char* log, char* pass);
message* InitMessage(int senderID, int recID, char* date, char* text);
DATABASE* InitDatabase(char* usersFile, char* msgFile);
void AddUser(DATABASE** data, char* login, char* password, char* usersFile);
void AddMessageToGeneralChat(DATABASE** data, char* text, int senderID, char* msgFile);
void AddMessageToDialogue(DATABASE** data, char* text, int senderID, int recID);
char** GetTenLastGenMessages(DATABASE* data);
char** GetTenLastDiaMessages(DATABASE* data, int clientID, int interlocutorID, int& size);
bool isEqualStr(char* str1, char* str2);
bool isCorrectLogin(DATABASE* data, char* login);
bool isCorrectPassword(DATABASE* data, char* login, char* pass);
