/*

muduo网络库提供的两个类:
TcpServer:用于编写服务器程序
TcpClient:用于编写客户端的程序

epoll+线程池
好处: 能够把网络I/O的代码和业务代码区分开

用户的连接和断开   洪湖的可读写事件
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <string>

#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;

/*
基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EnventLoop事件循环对象指针
3.明确TcpServer的构造函数需要什么参数
4.在当前服务器类的构造函数中,注册处理连接的回调函数和处理读写事件的回调函数
5.设置合适的服务端线程数量,muduo会自己划分IO和worker线程
*/

class ChatServer
{

public:
	ChatServer(EventLoop *loop,               //事件循环
		const InetAddress &listenAddr, //ip+port
		const string &nameArg)         //服务器名称
		: _server(loop, listenAddr, nameArg), _loop(loop)
	{

		//给服务器注册用户连接的创建和断开回调
		_server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
		//给服务器注册用户读写事件回调
		_server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
		//设置服务器端的线程数量    1个IO线程  3个worker线程
		_server.setThreadNum(4);
	}

	//开启事件循环
	void start()
	{

		_server.start();
	}

private:
	//专门处理用户的连接创建和断开
	void onConnection(const TcpConnectionPtr &conn)
	{
		if (conn->connected())
		{
			cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state: online" << endl;
		}
		else
		{
			cout << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "state: offline" << endl;
			conn->shutdown(); //close(fd)
			//_loop->quit();
		}
	}
	//专门处理用户的读写事件
	void onMessage(const TcpConnectionPtr &conn, //连接发送数据
		Buffer *buffer,               //缓冲区
		Timestamp time)               //接收数据的时间信息
	{
		string buf = buffer->retrieveAllAsString();
		cout << "rece data:" << buf << "time:" << time.toString() << endl;
		conn->send(buf);
	}

	TcpServer _server;
	EventLoop *_loop; //epoll
};

int main()
{

	EventLoop loop;
	InetAddress addr("127.0.0.1", 6000);
	ChatServer server(&loop, addr, "ChatServer");

	server.start();
	loop.loop(); //epoll_wait,用阻塞方式等待新的用户来连接

	return 0;
}
