#define _CRT_SECURE_NO_WARNINGS
#include "func.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

USER& database::operator[](unsigned int id)
{
	return *(this->users)[id];
}

USER& database::operator[](char* login)
{
	for (int i = 0; i < size; ++i)
	{
		if (SizeOfStr((this->users)[i]->login) == SizeOfStr(login))
		{
			int j = 0;
			bool isEqual = true;
			while (login[j] != '\0')
			{
				if (login[j] != (this->users)[i]->login[j])
				{
					isEqual = false;
					break;
				}
				++j;
			}
			if (isEqual) return *(this->users)[i];
		}
	}
}

void ParseStringUser(char* str, int* id, char** log, char** pass)
{
	char tmpID[10];
	char tmpLog[30];
	char tmpPass[30];
	int i = 0;
	while (str[i] != ' ')
	{
		tmpID[i] = str[i];
		++i;
	}
	tmpID[i] = '\0';
	i += 3;
	*id = atoi(tmpID);
	int j = 0;
	while (str[i] != ' ')
	{
		tmpLog[j] = str[i];
		++i;
		++j;
	}
	tmpLog[j] = '\0';
	strcpy(*log, tmpLog);
	j = 0;
	i += 3;
	while (str[i] != ' ' && str[i] != '\0' && str[i] != '\n')
	{
		tmpPass[j] = str[i];
		++i;
		++j;
	}
	tmpPass[j] = '\0';
	strcpy(*pass, tmpPass);
}

void ParseStringMessage(char* str, char** date, int* idS, int* idR, char** txt)
{
	char tmpIDs[10];
	char tmpIDr[10];
	char tmpDate[50];
	char tmpTxt[1024];
	int i = 0;
	while (str[i] != ']')
	{
		tmpDate[i] = str[i];
		++i;
	}
	tmpDate[i] = ']';
	tmpDate[i + 1] = '\0';
	i += 4;
	strcpy(*date, tmpDate);
	int j = 0;
	while (str[i] != ' ')
	{
		tmpIDs[j] = str[i];
		++i;
		++j;
	}
	tmpIDs[j] = '\0';
	*idS = atoi(tmpIDs);
	j = 0;
	i += 3;

	while (str[i] != ' ')
	{
		tmpIDr[j] = str[i];
		++i;
		++j;
	}
	tmpIDr[j] = '\0';
	*idR = atoi(tmpIDr);
	j = 0;
	i += 3;

	while (str[i] != '\0' && str[i] != '\n')
	{
		tmpTxt[j] = str[i];
		++i;
		++j;
	}
	tmpTxt[j] = '\0';
	strcpy(*txt, tmpTxt);
}

char* IntToStr(int num)
{
	char* str = (char*)malloc(sizeof(char) * 10);
	if (num == 0)
	{
		str[0] = '0';
		str[1] = '\0';
		return str;
	}
	int i = 0;
	while (num != 0)
	{
		str[i] = num % 10 + '0';
		i++;
		num /= 10;
	}
	str[i] = '\0';
	_strrev(str);
	return str;
}

void StrErase(char* source, int index)
{
	int i = 0;
	while (source[i + 1] != '\0')
	{
		source[i] = source[i + 1];
		++i;
	}
}


unsigned int SizeOfStr(char* str)
{
	unsigned int i = 0;
	while (str[i] != '\0')
		++i;
	return i;
}

USER* InitUser(unsigned int id, char* log, char* pass)
{
	USER* result = (USER*)malloc(sizeof(USER));
	result->messages = (message*)malloc(sizeof(message) * 200);
	result->login = (char*)malloc(sizeof(char) * 30);
	result->password = (char*)malloc(sizeof(char) * 30);

	result->id = id;
	strcpy(result->login, log);
	strcpy(result->password, pass);
	result->msgAmount = 0;

	return result;
}

message* InitMessage(int senderID, int recID, char* date, char* text)
{
	message* tmpMsg = (message*)malloc(sizeof(message));
	tmpMsg->time = (char*)malloc(sizeof(char) * 50);
	tmpMsg->time[0] = '\0';
	tmpMsg->text = (char*)malloc(sizeof(char) * 1024);
	tmpMsg->text[0] = '\0';
	strcpy(tmpMsg->time, date);
	strcpy(tmpMsg->text, text);
	tmpMsg->senderID = senderID;
	tmpMsg->recipientID = recID;
	return tmpMsg;
}

DATABASE* InitDatabase(char* usersFile, char* msgFile)
{
	DATABASE* result = (DATABASE*)malloc(sizeof(DATABASE));
	result->size = 0;
	result->users = (USER**)malloc(sizeof(USER*) * 50);
	int j = 0;

	FILE* usersF = fopen(usersFile, "rt");
	if (usersF == NULL)
	{
		printf("Error opening users file");
		return NULL;
	}
	FILE* msgF = fopen(msgFile, "rt");

	int id;
	char* log = (char*)malloc(sizeof(char) * 30);
	char* pass = (char*)malloc(sizeof(char) * 30);

	char userInfo[1400];
	userInfo[0] = '\0';
	while (fgets(userInfo, sizeof(userInfo), usersF))
	{
		ParseStringUser(userInfo, &id, &log, &pass);
		USER* tmpUser = InitUser(id, log, pass);

		char* userMsgsFname = (char*)malloc(sizeof(char) * 30);
		userMsgsFname[0] = '\0';
		strcat(userMsgsFname, "user_");
		char* tst = IntToStr(id);
		strcat(userMsgsFname, tst);
		strcat(userMsgsFname, "_messages.txt");

		FILE* userMsgs = fopen(userMsgsFname, "rt");
		if (!userMsgs)
		{
			printf("Error opening user#%d messages file", id);
			return NULL;
		}

		int   idSender, idRecv;
		char* txt = (char*)malloc(sizeof(char) * 1024);
		char* date = (char*)malloc(sizeof(char) * 50);
		int   i = 0;
		char  msg[1400];
		msg[0] = '\0';

		while (fgets(msg, sizeof(msg), userMsgs))
		{
			ParseStringMessage(msg, &date, &idSender, &idRecv, &txt);
			message* tmpMsg = InitMessage(idSender, idRecv, date, txt);
			tmpUser->messages[i] = *tmpMsg;
			tmpUser->msgAmount++;
			i++;
		}
		result->users[j] = tmpUser;
		result->size++;
		j++;
		fclose(userMsgs);
	}
	fclose(usersF);
	fclose(msgF);

	char  msg[1400];
	msg[0] = '\0';
	int   idSender, idRecv;
	char* txt = (char*)malloc(sizeof(char) * 1024);
	char* date = (char*)malloc(sizeof(char) * 50);
	int   i = 0;

	result->msgAmount = 0;
	result->generalChat = (message*)malloc(sizeof(message) * 1000);

	FILE* msgs = fopen(msgFile, "r");
	while (fgets(msg, sizeof(msg), msgs))
	{
		ParseStringMessage(msg, &date, &idSender, &idRecv, &txt);
		message* tmpMsg = InitMessage(idSender, idRecv, date, txt);
		result->generalChat[i] = *tmpMsg;
		i++;
		result->msgAmount++;
	}
	fclose(msgs);
	return result;
}

void AddUser(DATABASE** data, char* login, char* password, char* usersFile)
{
	USER* user = InitUser((*data)->size, login, password);
	(*data)->users[(*data)->size] = user;
	(*data)->size++;

	FILE* usersF = fopen(usersFile, "a");

	fprintf(usersF, "%d : %s : %s\n", user->id, login, password);
	fclose(usersF);

	char str[80];
	str[0] = '\0';
	strcat(str, "user_");
	strcat(str, IntToStr(user->id));
	strcat(str, "_messages.txt");

	FILE* userMsgs = fopen(str, "wt");
	fclose(userMsgs);
}

char* settime(struct tm* u)
{
	char s[40];
	char* tmp;
	for (int i = 0; i < 40; i++) s[i] = 0;
	int length = strftime(s, 40, "%H:%M %d.%m.%Y", u);
	tmp = (char*)malloc(sizeof(s));
	strcpy(tmp, s);
	return(tmp);
}

void AddMessageToGeneralChat(DATABASE** data, char* text, int senderID, char* msgFile)
{
	struct tm* u;
	char* timeS;
	const time_t timer = time(NULL);
	u = localtime(&timer);
	timeS = settime(u);
	char time[100];
	sprintf(time, "[%s]", timeS);
	message* msg = InitMessage(senderID, -1, time, text);
	(*data)->generalChat[(*data)->msgAmount] = *msg;
	(*data)->msgAmount++;

	FILE* msgs = fopen(msgFile, "a");
	fprintf(msgs, "[%s] : %d : -1 : %s\n", timeS, senderID, text);
	fclose(msgs);
}

void AddMessageToDialogue(DATABASE** data, char* text, int senderID, int recID)
{
	struct tm* u;
	char* timeS;
	const time_t timer = time(NULL);
	u = localtime(&timer);
	timeS = settime(u);

	char time[100];
	sprintf(time, "[%s]", timeS);


	USER* sender = &(**data)[senderID];
	USER* recipient = &(**data)[recID];
	message* msg = InitMessage(senderID, recID, time, text);

	sender->messages[sender->msgAmount] = *msg;
	sender->msgAmount++;

	recipient->messages[recipient->msgAmount] = *msg;
	recipient->msgAmount++;

	char str[80];
	str[0] = '\0';
	strcat(str, "user_");
	strcat(str, IntToStr(sender->id));
	strcat(str, "_messages.txt");

	FILE* userF = fopen(str, "a");
	fprintf(userF, "[%s] : %d : %d : %s\n", timeS, senderID, recID, text);
	fclose(userF);

	char str1[80];
	str1[0] = '\0';
	strcat(str1, "user_");
	strcat(str1, IntToStr(recipient->id));
	strcat(str1, "_messages.txt");

	userF = fopen(str1, "a");
	fprintf(userF, "[%s] : %d : %d : %s\n", timeS, senderID, recID, text);
	fclose(userF);

}

char** GetTenLastGenMessages(DATABASE* data)
{
	char** result = (char**)malloc(sizeof(char*) * 10);
	for (int i = 0; i < 10; ++i)
	{
		if (data->msgAmount - i - 1 < 0)
			break;
		result[i] = (char*)malloc(sizeof(char) * 1024);
		char* mes = (char*)malloc(sizeof(char) * 1024);
		mes[0] = '\0';
		strcat(mes, data->generalChat[data->msgAmount - i - 1].time);
		strcat(mes, " : ");
		strcat(mes, IntToStr(data->generalChat[data->msgAmount - i - 1].senderID));
		strcat(mes, " : ");
		strcat(mes, data->generalChat[data->msgAmount - i - 1].text);
		strcat(mes, "\n");
		result[i] = mes;
	}
	return result;
}

char** GetTenLastDiaMessages(DATABASE* data, int clientID, int interlocutorID, int& size)
{
	char** result = (char**)malloc(sizeof(char*) * 10);
	USER client = (*data)[clientID];
	int j = client.msgAmount;
	for (int i = 0; i < 10; ++i)
	{
		if (j - i - 1 < 0)
			break;
		result[i] = (char*)malloc(sizeof(char) * 1024);
		if (client.messages[j - i - 1].recipientID != interlocutorID && client.messages[j - i - 1].senderID != interlocutorID)
		{
			j--;
			i--;
			continue;
		}
		char* mes = (char*)malloc(sizeof(char) * 1024);
		mes[0] = '\0';
		strcat(mes, client.messages[j - i - 1].time);
		strcat(mes, " : ");
		strcat(mes, IntToStr(client.messages[j - i - 1].senderID));
		strcat(mes, " : ");
		strcat(mes, client.messages[j - i - 1].text);
		strcat(mes, "\n");
		result[i] = mes;
		size++;
	}
	return result;
}

bool isEqualStr(char* str1, char* str2)
{
	if (strcmp(str1, str2) != 0)
		return false;
	for (int i = 0; i < strlen(str1); ++i)
		if (str1[i] != str2[i]) return false;
	return true;
}

bool isCorrectLogin(DATABASE* data, char* login)
{
	for (int i = 0; i < data->size; ++i)
		if (isEqualStr(login, (*data)[i].login)) return true;
	return false;
}

bool isCorrectPassword(DATABASE* data, char* login, char* pass)
{
	USER* user = &(*data)[login];
	return isEqualStr(pass, user->password);
}

