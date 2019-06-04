#pragma once
#include <iostream>  
#include <stdio.h>  
#include <sys/socket.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <netdb.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <string.h>
#include <fcntl.h>
#include <thread>
#include <threadpool.hpp>
#include <json/json.h>
#include <tinytangle/logging.hpp>
using namespace std;

class cliBcast {
public:
	cliBcast(int port, const std::string& bcast_addr):port_(port), bcast_addr_(bcast_addr){
		//setvbuf(stdout, NULL, _IONBF, 0);//及时写入无缓冲
		//fflush(stdout);

		sock_fd = -1;
		if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			cout << "socket error" << endl;
			return;
		}
		const int opt = 1;
		//设置该套接字为广播类型 
		int rtn = 0;
		rtn = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
		if (rtn == -1)
		{
			cout << "set socket error..." << endl;
			return;
		}
		bzero(&addr_to, sizeof(struct sockaddr_in));
		addr_to.sin_family = AF_INET;//IPv4
		//addr_to.sin_addr.s_addr = htonl(INADDR_BROADCAST);
		inet_pton(AF_INET, bcast_addr_.c_str(), &addr_to.sin_addr);
		addr_to.sin_port = htons(port);
		addr_len_ = sizeof(addr_to);


		//sockaddr_in test_addr;
		//bzero(&test_addr, sizeof(test_addr));
		//test_addr.sin_family = AF_INET;
		//test_addr.sin_port = htons(6000);
		////test_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		//inet_pton(AF_INET, "172.17.0.2", &test_addr.sin_addr);
		//int a=connect(sock_fd, (struct sockaddr *)&test_addr, sizeof(test_addr));
		//sockaddr_in localaddr; socklen_t addrlen;
		//int b=getsockname(sock_fd, (struct sockaddr *)&localaddr, &addrlen);
		//cout << endl << ntohl(localaddr.sin_addr.s_addr)  << endl << endl;
	}

	void Sendto(const string& str, const string& cmd, const struct sockaddr_in *remote_addr = NULL, int len = 0) {
		const char *msg = str.c_str();
		int length = str.length();
		sockaddr* addr = NULL;
		int addr_len = 0;
		char* ch = NULL;
		if (remote_addr == NULL || len == 0) {
			addr = (sockaddr*)&addr_to;
			addr_len = addr_len_;
			ch = inet_ntoa(addr_to.sin_addr);
		}
		else {
			addr = (sockaddr*)remote_addr;
			addr_len = len;
			ch = inet_ntoa(remote_addr->sin_addr);
		}

		string ip_str(ch);
		tangle::log::info(__FUNCTION__) << "[send to:" << ip_str << "]: " << cmd;
		int rtn = sendto(sock_fd, msg, length, 0, addr, addr_len);
		if (rtn < 0)
		{
			cout << "send error...." << rtn << endl;
			return;
		}
		else
		{
		}

	}

	void request_dag(const struct sockaddr_in *serv_addr) {
		Json::Value root;
		root["head"] = "reqdag";
		char* ch = inet_ntoa(serv_addr->sin_addr);
		tangle::log::debug(__FUNCTION__) << "request dag from: "<<string(ch);
		string cmd("reqdag");
		Sendto(root.toStyledString(), cmd, serv_addr,sizeof(struct sockaddr_in));
		if (readable_timeo(sock_fd, 10)) {
			const int BUFF_N = 10 * 1024 * 1024;
			char *buff = new char[BUFF_N];
			int rtn = recvfrom(sock_fd, buff, BUFF_N, 0, NULL, NULL); //实际无法达到10MB，因为UDP报文的限制，需要另外设置
			if (rtn < 0)
			{
				tangle::log::error(__FUNCTION__) << "read error...";
			}
			else
			{
				string str(buff);
				Json::Reader reader;
				Json::Value root;
				reader.parse(str, root);
				string head = root["head"].asString();
				if (head == "reqdag") {
					Json::Value units  = root["body"];
					push_all_unit(units); //DAG初始化完成
					tangle::log::info(__FUNCTION__) << "sync ok";
				}
				else {
					tangle::log::error(__FUNCTION__) << "head error";
				}
			}
			delete[] buff;
		}
		else {
			//初始化超时直接退出
			tangle::log::error(__FUNCTION__) << "waiting dag units is timeout";
			exit(-1);
		}

	}

	void sync_dag(void) {
		Json::Value root;
		root["head"] = "syncDag";
		string msg = root.toStyledString();
		string cmd("syncDag");
		Sendto(msg, cmd);
		//5秒种的等待时间
		//thread([&]() {
		const int BUFF_N = 1024 * 1024;
		char *buff = new char[BUFF_N];
		int64_t maxcount = -1;
		struct sockaddr_in temp_addr, serv_addr;
		bzero(&temp_addr, sizeof(struct sockaddr_in));
		int len = sizeof(temp_addr);
		while (readable_timeo(sock_fd, 5)) {
			int rtn = recvfrom(sock_fd, buff, BUFF_N, 0, (struct sockaddr*)& temp_addr, (socklen_t*)&len);
			if (rtn < 0)
			{
				tangle::log::error(__FUNCTION__) << "read error...";
			}
			else
			{
				string str(buff);
				Json::Reader reader;
				Json::Value root;
				reader.parse(str, root);
				string head = root["head"].asString();
				if (head == "syncDag") {
					int64_t count = root["body"].asInt64();

					char* ch = inet_ntoa(temp_addr.sin_addr);
					string str(ch);
					tangle::log::info(__FUNCTION__) << "[recv from:" << str << "]: count=" << count;

					if (count > maxcount) {
						maxcount = count;
						serv_addr = temp_addr;
					}
				}
				else {
					tangle::log::error(__FUNCTION__) << "head error";
				}
			}
		}
		if (maxcount >= 0) {
			request_dag(&serv_addr);
		}
		else {
			tangle::log::debug(__FUNCTION__) << "no other nodes. start to initialize dag...";
		}

		delete[] buff;
		//}).detach();
	}

	void broadcast_unit(const Json::Value& unit) {
		Json::Value root;
		root["head"] = "bcastunit";
		root["body"] = unit;
		string msg = root.toStyledString();
		string cmd("bcastunit");
		Sendto(msg, cmd);
	}

	template<typename F>
	void set_push_all_unit(F&& fun) {
		push_all_unit = forward<F>(fun);
	}

private:
	//等待一个描述符变为可读
	int readable_timeo(int fd, int sec) {
		fd_set rset;
		struct timeval tv;
		FD_ZERO(&rset);
		FD_SET(sock_fd, &rset);
		tv.tv_sec = sec;//5s超时接收
		tv.tv_usec = 0;
		return(select(fd + 1, &rset, NULL, NULL, &tv));
		/* >0  if descriptor is readable */
	}

private:
	int port_;
	struct sockaddr_in addr_to; ////IPv4地址结构体
	int addr_len_;
	int sock_fd;
	std::string bcast_addr_;


	std::function<void(const Json::Value&)> push_all_unit;

};

class servBcast {
public:
	servBcast(int port):port_(port), thread_pool_(2,0){
		is_stop_ = false;

		// 绑定地址  
		bzero(&serv_addr, sizeof(struct sockaddr_in));
		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		serv_addr.sin_port = htons(port);

		sock_fd = -1;
		if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			cout << "socket error" << endl;
			return;
		}

		const int opt = 1;
		//设置该套接字为广播类型，  
		int rtn = 0;
		rtn = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));
		if (rtn == -1)
		{
			cout << "set socket error..." << endl;
			return;
		}

		if (bind(sock_fd, (struct sockaddr *)&(serv_addr), sizeof(struct sockaddr_in)) == -1)
		{
			cout << "bind error..." << endl;
			return;
		}
		thread_pool_.start();
	}

	~servBcast() {
		stop_listen();
	}

	void Sendto(const string& str, const struct sockaddr_in *remote_addr, int len , string &cmd) {
		const char *msg = str.c_str();
		int length = str.length();
		sockaddr* addr = (sockaddr*)remote_addr;

		char* ch = inet_ntoa(remote_addr->sin_addr);
		string ip_str(ch);
		tangle::log::info(__FUNCTION__) << "[response to:" << ip_str << "]: " << cmd;

		int rtn = sendto(sock_fd, msg, length, 0, addr, len);
		if (rtn < 0)
		{
			cout << "send error...." << rtn << endl;
			return;
		}
		else
		{

		}

	}
	void listen() {
		const int BUFF_N = 1024 * 1024;
		char *buff = new char[BUFF_N];
		int nRead = 0;
		fd_set rd;
		// 广播地址  
		struct sockaddr_in cli_addr;
		bzero(&cli_addr, sizeof(struct sockaddr_in));
		int len = sizeof(cli_addr);
		while (!is_stop_) {
			FD_ZERO(&rd);
			FD_SET(sock_fd, &rd);
			select(sock_fd + 1, &rd, NULL, NULL, NULL);

			int rtn = recvfrom(sock_fd, buff, BUFF_N, 0, (struct sockaddr*)&cli_addr, (socklen_t*)&len);
			if (rtn < 0)
			{
				cout << "read error...." << sock_fd << endl;
			}
			else
			{
				char* ch = inet_ntoa(cli_addr.sin_addr);
				string ip_str(ch);
				if (!self_addr_.is_init_) { //填写自身IP
					self_addr_.addr = cli_addr.sin_addr.s_addr;
					cout <<"self addr:" << ip_str << endl;
					self_addr_.is_init_ = true;
				}
				
				//先client 发送广播包，再打开的server
				//即使这样，在server打开的时候仍然能够收到之前client发出的广播内容，而且收到的第一个广播包就是刚才自身发出的广播
				//所以，过滤自身IP广播包
				if (self_addr_.addr != cli_addr.sin_addr.s_addr) {
					string root_str(buff);
					bzero(buff, BUFF_N);

					Json::Reader reader;
					Json::Value root;
					reader.parse(root_str, root);
					
					if(! root.isObject()){
						//thread_pool_.execute(bind(&push_cmd, root_str)); //error
						push_cmd(root_str);
						continue;
					}

					string head = root["head"].asString();
					tangle::log::info(__FUNCTION__) << "[recv from:" << ip_str << "]: head=" << head;
					if (head == "syncDag") {
						thread_pool_.execute([=]() {
							Json::Value response;
							response["head"] = "syncDag";
							uint64_t num = get_units_count();
							response["body"] = static_cast<int64_t>(num);
							string msg = response.toStyledString();
							string cmd("syncDag");
							Sendto(msg, &cli_addr, len, cmd);
							//sendto(sock_fd, msg.c_str(), msg.size(), 0, (struct sockaddr*)&cli_addr, len);
						});
					}
					else if (head == "reqdag") {
						thread_pool_.execute([=]() {
							Json::Value response, body;
							response["head"] = "reqdag";
							get_all_units(body);
							response["body"] = body;
							string msg = response.toStyledString();
							string cmd("reqdag");
							Sendto(msg, &cli_addr, len, cmd);
							//sendto(sock_fd, msg.c_str(), msg.size(), 0, (struct sockaddr*)&cli_addr, len);
						});
					}
					else if (head == "bcastunit") {
						thread_pool_.execute([=](){ // &?
							Json::Value unit;
							unit = root["body"];
							//是否为合法且单元
							if (push_one_unit(unit)) {
								broadcast_unit(unit);
							}
						});
					}
					else {
					}
					//tangle::log::info(__FUNCTION__) << "\n"<<str;
				}
			}

			//重置 清零
			bzero(&cli_addr, sizeof(struct sockaddr_in));
			bzero(buff, BUFF_N);
		}
		delete[] buff;
	}

	void start_listen() {
		thread(bind(&servBcast::listen, this)).detach();
	}

	void stop_listen() {
		is_stop_ = true;
	}

	string get_host_ip() {
		if (self_addr_.is_init_) {
			struct in_addr temp;
			temp.s_addr = self_addr_.addr;
			char* ch = inet_ntoa(temp);
			string ip_str(ch);
			return ip_str;
		}
		string ip_str("0.0.0.0");
		return ip_str;
	}

	template<typename F>
	void set_get_units_count(F&& fun) {
		get_units_count = forward<F>(fun);
	}
	template<typename F>
	void set_get_all_units(F&& fun) {
		get_all_units = forward<F>(fun);
	}
	template<typename F>
	void set_push_one_unit(F&& fun) {
		push_one_unit = forward<F>(fun);
	}
	template<typename F>
	void set_broadcast_unit(F&& fun) {
		broadcast_unit = forward<F>(fun);
	}
	template<typename F>
	void set_push_cmd(F&& fun) {
		push_cmd = forward<F>(fun);
	}
private:
	int port_;
	struct sockaddr_in serv_addr; ////IPv4地址结构体
	int addr_len;
	int sock_fd;
	bool is_stop_;
	threadpool::fixed_thread_pool thread_pool_;

	struct self_addr {
		int addr;
		bool is_init_ = false;
	}self_addr_;

	function<uint64_t(void)> get_units_count;
	function<void(Json::Value &)> get_all_units;
	function<bool(const Json::Value&)> push_one_unit;
	function<void(const Json::Value&)> broadcast_unit;
	function<void(const string&)> push_cmd;
	
};

class Network {
public:
	Network(int port, const string& bcast_addr):client(port, bcast_addr), server(port), port_(port){
		server.set_broadcast_unit(bind(&cliBcast::broadcast_unit, &client, placeholders::_1));
	}

	string get_ip(void) {
		return server.get_host_ip();
	}
/*
	void broadcast(const string& unit) {
		client.broadcast_unit(unit);
	}*/

	void broadcast_unit(const Json::Value& unit) {
		client.broadcast_unit(unit);
	}

	void start_listen() {
		server.start_listen();
	}

	void sync_dag() {
		client.sync_dag();
	}

	template<typename F>
	void set_push_all_unit(F&& fun) {
		client.set_push_all_unit(fun);
	}
	template<typename F>
	void set_get_units_count(F&& fun) {
		server.set_get_units_count(fun);
	}
	template<typename F>
	void set_get_all_units(F&& fun) {
		server.set_get_all_units(fun);
	}
	template<typename F>
	void set_push_one_unit(F&& fun) {
		server.set_push_one_unit(fun);
	}
	template<typename F>
	void set_push_cmd(F&& fun) {
		server.set_push_cmd(fun);
	}
private:
	cliBcast client;
	servBcast server;
	int port_;

};