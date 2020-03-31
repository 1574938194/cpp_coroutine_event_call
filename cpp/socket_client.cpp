// cppClient.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include <iostream>
#include <winsock.h>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
void initialization();

bool is_little_endian(void) noexcept
{
	static short _buff = 0xFFFE;
	return (((const char*)_buff)[0] == 0xFE);
}

class cycle_buffer
{
public:
	cycle_buffer(std::size_t size)
	{
		first = new char[size]();
		last = first + size;
		begin = first;
		end = first;
	}

	~cycle_buffer()
	{
		if (first != 0 &&
			last != 0 &&
			last > first
			)
		{
			delete first;
		}
	}
public:
	inline bool is_full(void) const noexcept
	{
		return false;
	}

	inline bool is_empty(void)const noexcept
	{
		return (end == begin);
	}

	inline size_t capacity(void)const noexcept
	{
		if (begin >= end)
		{
			return (begin - end);
		}

		auto _size = this->size();
		return (_size - (end - begin));
	}

	inline size_t size(void)const noexcept
	{
		return (last - first);
	}

	void push(char*arr, size_t size)
	{

	}
public:
	char* first;
	char* last;
	char* begin;
	char* end;
};


class tcp_client
{
public:
	tcp_client()
	{
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		server_addr.sin_port = htons(5010);
		server = socket(AF_INET, SOCK_STREAM, 0);
	}
	~tcp_client()
	{
		closesocket(server);
	}
public:
	bool link()
	{
		if (connect(server, (SOCKADDR *)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR)
		{
			cout << "服务器连接失败！" << endl;
			//WSACleanup();
			return false;
		}
		return true;
	}

	bool send(const char* buf, size_t size)
	{
		// 先验size
		auto len = ::send(server, (char*)&size, sizeof(int), 0);
		if (len < 0) { return false; }

		// 正文包
		len = ::send(server, buf, size, 0);
		if (len < 0) { return false; }

		return true;
	}
public:
	char buff[1024];
	SOCKET server;
	SOCKADDR_IN server_addr;
};


int __main() {
	initialization();

	/*	recv_len = recv(s_server, recv_buf, 100, 0);
		if (recv_len < 0) {
			cout << "接受失败！" << endl;

		}*/

	tcp_client client;

	std::cout << "连接到服务器..." << client.link() << std::endl;

	std::cout << "发送到服务器..." << client.send("hello this is a client", 23) << std::endl;

	//释放DLL资源
	WSACleanup();
	return 0;
}


void initialization() {
	//初始化套接字库
	WORD w_req = MAKEWORD(2, 2);//版本号
	WSADATA wsadata;
	int err;
	err = WSAStartup(w_req, &wsadata);
	if (err != 0) {
		cout << "初始化套接字库失败！" << endl;
	}
	else {
		cout << "初始化套接字库成功！" << endl;
	}
	//检测版本号
	if (LOBYTE(wsadata.wVersion) != 2 || HIBYTE(wsadata.wHighVersion) != 2) {
		cout << "套接字库版本号不符！" << endl;
		WSACleanup();
	}
	else {
		cout << "套接字库版本正确！" << endl;
	}
	//填充服务端地址信息

}

