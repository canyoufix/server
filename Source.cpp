#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <cstring>
#pragma warning(disable: 4996)

int main()
{
	// ���� ��������� �������� ������
	BOOL   _isConnected;

	// ������������� ������ Pipe
	HANDLE hNamedPipe, hIn, hOut;

	// ��� ������������ ������ Pipe
	LPCTSTR  lpszPipeName = "\\\\.\\pipe\\$MyPipe$";

	char   szBuf[512], Buf[256], Count[256], outFile[256], Buffer[256];
	char* NameOfFile;
	char* NumOfReplace;

	// ���������� ���� ������, �������� � ��������� ����� �����
	DWORD  cbRead, cbWritten;

	DWORD   total = 0;
	DWORD nIn, nOut;

	// ����� ���  ��������� �� ������, ����������
	char message[256] = { 0 };
	printf("Named pipe server demo\n");
	
	// ������� ����� Pipe, ������� ��� lpszPipeName
	hNamedPipe = CreateNamedPipe(
		lpszPipeName,
		PIPE_ACCESS_DUPLEX,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		PIPE_UNLIMITED_INSTANCES,
		512, 512, 5000, NULL);
	// ���� �������� ������, ������� �� ��� � ��������� ������ ����������
	if (hNamedPipe == INVALID_HANDLE_VALUE)
	{
		fprintf(stdout, "CreateNamedPipe: Error %ld\n",
			GetLastError());
		_getch();
		return 0;
	}

	// ������� ��������� � ������ �������� �������� ������
	fprintf(stdout, "Waiting for connect...\n");

	// ������� ���������� �� ������� �������
	_isConnected = ConnectNamedPipe(hNamedPipe, NULL);

	// ��� ������������� ������ ������� �� ���
	if (!_isConnected)
	{
		switch (GetLastError())
		{
		case ERROR_NO_DATA:
			fprintf(stdout, "ConnectNamedPipe: ERROR_NO_DATA");
			_getch();
			CloseHandle(hNamedPipe);
			return 0;
			break;

		case ERROR_PIPE_CONNECTED:
			fprintf(stdout,
				"ConnectNamedPipe: ERROR_PIPE_CONNECTED");
			_getch();
			CloseHandle(hNamedPipe);
			return 0;
			break;

		case ERROR_PIPE_LISTENING:
			fprintf(stdout,
				"ConnectNamedPipe: ERROR_PIPE_LISTENING");
			_getch();
			CloseHandle(hNamedPipe);
			return 0;
			break;

		case ERROR_CALL_NOT_IMPLEMENTED:
			fprintf(stdout,
				"ConnectNamedPipe: ERROR_CALL_NOT_IMPLEMENTED");
			_getch();
			CloseHandle(hNamedPipe);
			return 0;
			break;

		default:
			fprintf(stdout, "ConnectNamedPipe: Error %ld\n",
				GetLastError());
			_getch();
			CloseHandle(hNamedPipe);
			return 0;
			break;
		}
		CloseHandle(hNamedPipe);
		_getch();
		return 0;
	}

	// ������� ��������� �� �������� �������� ������
	fprintf(stdout, "\nConnected. Waiting for command...\n");

	// ���� ��������� ������ ����� �����
	while (1)
	{
		// �������� ��������� ������� ����� ����� Pipe
		if (ReadFile(hNamedPipe, szBuf, 512, &cbRead, NULL))
		{
			// ������� �������� ������� �� ������� 
			printf("Received: %s\n", szBuf);

			// ���� ������ ������� "exit", ��������� ������ ����������
			strtok(szBuf, "\n");
			if (strcmp(szBuf, "exit") == 0)
			{
				WriteFile(hNamedPipe, szBuf, strlen(szBuf) + 1, &cbWritten, NULL);
				break;
			}

			// ����� ������� ��� ������� ��� �����
			while (1)
			{
				// ��������� ���� �� "������" � ��������� ������, �� ���� ��������� �� ������ ����
				if (strchr(szBuf, ' ')) {
					// ��������� ��� ����� � ���������� ����� � ������ ����������
					NameOfFile = strtok(szBuf, " ");
					NumOfReplace = strtok(NULL, " ");
					strcpy(Buf, NameOfFile);
					strcpy(Count, NumOfReplace);

					// ��������� ����
					hIn = CreateFile(Buf, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
					if (hIn == INVALID_HANDLE_VALUE)
					{
						sprintf(message, "(Server)Can't open %s!", Buf);
						WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
						printf("\n");
						WriteFile(hNamedPipe, message, strlen(message) + 1, &cbWritten, NULL);
						CloseHandle(hIn);
						break;
					}
					// ������ ����� ���� � ������ "������� ���_New.txt"
					int i = 0;
					while (*(Buf + i) != '.') {
						outFile[i] = *(Buf + i);
						i++;
					}
					outFile[i] = '\0';
					strcat(outFile, "_New.txt");
					hOut = CreateFile(outFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hOut == INVALID_HANDLE_VALUE)
					{
						sprintf(message, "(Server)Can't open %s!", outFile);
						WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
						printf("\n");
						WriteFile(hNamedPipe, message, strlen(message) + 1, &cbWritten, NULL);
						CloseHandle(hIn);
						CloseHandle(hOut);
						break;
					}
					// ������ ��������� � ������ � ����� ����
					while (ReadFile(hIn, Buffer, 256, &nIn, NULL) && nIn > 0) {
						int numFromCMD = atoi(Count);

						for (int i = 0; i < nIn; i++)
						{
							if (Buffer[i] >= '0' && Buffer[i] <= '9' && total < numFromCMD) {
								Buffer[i] = ' ';
								total++;
							}
						}
						
						WriteFile(hOut, Buffer, nIn, &nOut, NULL);
						if (nIn != nOut) {
							sprintf(message, "(Server)fatal recording error: %x\n", GetLastError());
							WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
							printf("\n");
							WriteFile(hNamedPipe, message, strlen(message) + 1, &cbWritten, NULL);
							CloseHandle(hIn);
							CloseHandle(hOut);
							break;
						}
					}
					// ��������� � ������� ������ 
					sprintf(message, "\n(Server): file: %s, number of replacements = %d, in a new file: %s\n", Buf, total, outFile);
					WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message), &cbWritten, NULL);
					// ��������� � ����� 
					sprintf(message, "The number of replacements in the new file(%s): %d", outFile, total);
					WriteFile(hNamedPipe, message, strlen(message) + 1, &cbWritten, NULL);
					// �������� ������������ 
					CloseHandle(hOut);
					CloseHandle(hIn);
					total = 0;
					break;
				}
				// ���� ��� �������, ������� �� �����
				else
				{
					sprintf(message, "(Server) incorrect input!");
					WriteFile(GetStdHandle(STD_ERROR_HANDLE), message, strlen(message) + 1, &cbWritten, NULL);
					printf("\n");
					WriteFile(hNamedPipe, message, strlen(message) + 1, &cbWritten, NULL);
					break;
				}
			}
		}
		else
		{
			printf("ReadFile: Error %ld\n", GetLastError());
			break;
		}
	}
	CloseHandle(hNamedPipe);
	return 0;
}