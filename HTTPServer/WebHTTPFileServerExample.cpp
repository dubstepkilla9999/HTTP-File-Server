#include <winsock2.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>

std::string Page404()
{
	return "HTTP/1.1 404 Not Found\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"<html>"
		"<body>"
		"<h1>404 NOT FOUND</h1>"
		"<p>This page doesn't exits</p>"
		"<a href = \"/\">Return to main page</a>"
		"</body>"
		"</html>";
}

std::string Filename = "TestFile.txt";
#pragma comment(lib, "Ws2_32.lib")
int main()
{
	int Result;
	WSADATA wsadata;

	//Initialize winsock
	Result = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (Result != NO_ERROR)
	{
		std::cout << "WSAStartup failed with error: " << std::to_string(Result) << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Winsock initialized" << std::endl;
	}

	//Create a socket
	const SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		std::cout << "Socket failed with error: " << std::to_string(WSAGetLastError()) << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Socket created" << std::endl;
	}

	//socket address to bind
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(80);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	//Bind socket to address
	Result = bind(ListenSocket, (SOCKADDR*)&server_addr, sizeof(server_addr));
	if (Result == SOCKET_ERROR) {
		std::cout << "Bind failed with error " << std::to_string(WSAGetLastError()) << std::endl;
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	else
	{
		std::cout << "Bind returned success" << std::endl;
	}

	if (listen(ListenSocket, 5) == SOCKET_ERROR)
	{
		std::cout << "Listen function failed with error: " << std::to_string(WSAGetLastError()) << std::endl;
		return 1;
	}
	else
	{
		std::cout << "Listening..." << std::endl;
	}

	SOCKET AcceptSocket;
	std::cout << "Waiting for client to connect..." << std::endl;

	//Receiving bytes
	for (;;)
	{
		//Accept the connection.
		AcceptSocket = accept(ListenSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			std::cout << "Accept failed with error: " << std::to_string(WSAGetLastError()) << std::endl;
			closesocket(ListenSocket);
			WSACleanup();
			continue;
		}
		else
		{
			std::cout << "Client connected." << std::endl;
		}

		char buffer[1024];
		Result = recv(AcceptSocket, buffer, 1024, 0);
		std::string ClientRequest(buffer, buffer + Result);
		if (Result > 0)
		{
			std::cout << ClientRequest << std::endl;
		}
		else
		{
			std::cout << "Recv failed : " << std::to_string(WSAGetLastError()) << std::endl;
			closesocket(AcceptSocket);
		}
		std::istringstream ISS(ClientRequest);
		std::string RequestedMethod;
		ISS >> RequestedMethod;
		if (RequestedMethod == "GET")
		{
			std::string Answer;
			std::string RequestedPath;
			ISS >> RequestedPath;
			if (RequestedPath == "/")
			{
				Answer = "HTTP/1.1 200 OK\r\n"
					"Content-Type: text/html\r\n"
					"\r\n"
					"<html>"
					"<body>"
					"<h1>Hello World!</h1>"
					"<p>This is a test HTTP server </p>"
					"<a href = \"/download\" target = \"_blank\">Download test file</a>"
					"</body>"
					"</html>";
			}
			else if (RequestedPath == "/download")
			{
				std::ifstream File(Filename, std::ios::binary);
				if (File)
				{

					std::vector<char> FileData(
						(std::istreambuf_iterator<char>(File)),
						std::istreambuf_iterator<char>());
					File.close();
					Answer = "HTTP/1.1 200 OK\r\n"
						"Content-Type: application/octet-stream\r\n"
						"Content-Disposition: attachment; filename=\"" + Filename + "\"\r\n"
						"Content-Length: " + std::to_string(FileData.size()) + "\r\n"
						"\r\n";
					std::vector<char> BinaryResponse(Answer.begin(), Answer.end());
					BinaryResponse.insert(BinaryResponse.end(), FileData.begin(), FileData.end());
					send(AcceptSocket, BinaryResponse.data(), BinaryResponse.size(), 0);
					continue;
				}
				else
				{
					Answer = Page404();
				}
			}
			else
			{
				Answer = Page404();
			}
			send(AcceptSocket, Answer.data(), Answer.size(), 0);
		}
		else
		{
			std::cout << "Command not supported, ignoring" << std::endl;
		}
		closesocket(AcceptSocket);
	}
}