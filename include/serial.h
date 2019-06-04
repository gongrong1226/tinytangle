#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <tinytangle/logging.hpp>
#include <iostream>
#include <stdio.h>
#include <cstring>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <pthread.h>
#include <errno.h>
#include <thread>
//#include <time.h>
//#include <signal.h>
#include <tinytangle/logging.hpp>  
#include <tinytangle/dag.h>

namespace embeded_data {
	
class serial
{
public:
	serial(const char* uart, int baud_rate, tangle::Dag *dag);
	virtual ~serial();

	void info(void); //���ںͲ�������Ϣ

	int write_data(const std::string string);

	int write_data(const char* src, int length);

	void start_read(void);

private:
	void read_thread();

	int init_serial(void); //���ڳ�ʼ��
	tangle::Dag *dag_;
	int fd;
	int baud_rate;
	bool init_ok;
	std::string uart;
	struct termios options;
	pthread_t thread_t = 0;
};
}