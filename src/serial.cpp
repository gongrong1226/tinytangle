#include <serial.h>
namespace embeded_data {

using namespace tangle;
serial::serial(const char* uart, int baud_rate, Dag* dag):uart(uart), baud_rate(baud_rate), dag_(dag){
	//if ((fd = open(uart, O_RDWR | O_CREAT, 0777)) < 0){	//打开串口
	if ((fd = open(uart, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {	//打开串口
		log::error(__FUNCTION__) << "open error...\n maybe there is no \"" <<string(uart) << "\"";
		init_ok = false;
		return;
	}
	else {
		init_serial();
		init_ok = true;
	}

}

serial::~serial() {

	if (thread_t > 0) {
		pthread_cancel(thread_t);
		pthread_join(thread_t, NULL);
	}
	close(fd);
}

int serial::init_serial(void) {
	 
	if (tcgetattr(fd, &options) < 0)	//获取串口终端属性
	{
		log::error("serial") << "tcgetattr error\n";
		return -1;
	}
	cfmakeraw(&options);   //config to raw mode   shabi code
	/*设置控制模式*/
	//options.c_cflag |= CLOCAL;//保证程序不占用串口
	//options.c_cflag |= CREAD;//保证程序可以从串口中读取数据

	/*设置数据位:八位*/
	//options.c_cflag &= ~CSIZE;//屏蔽其它标志位
	//options.c_cflag |= CS8;  //八位

	/*设置校验位：无校验*/
	//options.c_cflag &= ~PARENB;//PARENB：产生奇偶位，执行奇偶校验
	//options.c_cflag &= ~INPCK;//INPCK：使奇偶校验起作用

	/*设置停止位：一位*/
	//options.c_cflag &= ~CSTOPB;//CSTOPB：使用一位停止位

	/*设置等待时间和最小接受字符*/
	//options.c_cc[VTIME] = 0;//可以在select中设置
	//options.c_cc[VMIN] = 1;//最少读取一个字符
	 
	/*清空串口Buff*/
	//tcflush(fd, TCIFLUSH);

	/*设置波特率*/
	switch (baud_rate) {
	case 4800:
		cfsetispeed(&options, B4800);
		cfsetospeed(&options, B4800);
		break;
	case 9600:
		cfsetispeed(&options, B9600);
		cfsetospeed(&options, B9600);
		break;
	case 19200:
		cfsetispeed(&options, B19200);
		cfsetospeed(&options, B19200);
		break;
	case 38400:
		cfsetispeed(&options, B38400);
		cfsetospeed(&options, B38400);
		break;
	case 115200:
		cfsetispeed(&options, B115200);
		cfsetospeed(&options, B115200);
		break;
	default:
		log::error("serial") << "Unkown baude!\n";
		return -1;
	}

	/*激活配置*/
	if (tcsetattr(fd, TCSANOW, &options) < 0)
	{
		log::error("serial") << "tcsetattr failed\n";
		return -1;
	}
	log::info("serial") << "initialize serial port successful\n";
	return 0;
}

void serial::info(void) {
	std::cout << "serial " << uart << ": baud rate = " << baud_rate
				<<". init:"<<init_ok << std::endl;
}

int serial::write_data(const std::string string) {
	if (!init_ok) {
		log::error(__FUNCTION__) << "serial port is not ready\n";
		return -1;
	}
	const char *str = string.c_str();
	int length = string.size();
	int nwritten;

	if (!init_ok) {
		log::error(__FUNCTION__) << "init error!\n";
		return -1;
	}
	nwritten = write_data(str, length);
	log::info(__FUNCTION__) << "wirtten: " << nwritten;
	return nwritten;
}

/**
@brief 使用串口发送字符串
@param src 字符串首地址
@parama length 需要发送的长度
*/
int serial::write_data(const char* str, int length) {
	if (!init_ok) {
		log::error(__FUNCTION__) << "serial port is not ready\n";
		return -1;
	}
	int nwritten = 0;
	if (!init_ok) {
		log::error(__FUNCTION__) << "init error!\n";
		return -1;
	}
	while (nwritten != length) { //未做超时处理
		write(fd, str+ nwritten, sizeof(char));
		++nwritten;
	}
	return nwritten;
}

void serial::start_read(void) {
	if (!init_ok) {
		log::error(__FUNCTION__) << "serial port is not ready\n";
		return;
	}
	if (!init_ok) {
		log::error(__FUNCTION__) << "init error!\n";
		return;
	}
	auto fun = std::bind(&serial::read_thread, this);
	std::thread t(fun);
	t.detach();
	/*if(pthread_create(&thread_t, NULL, &read_thread, &fd) != 0) {
		log::error(__FUNCTION__) << "pthread create error!\n";
	}
	else {
		//pthread_detach(thread_t); //线程结束自动释放资源
	}*/
}

void serial::read_thread() {
	char buff[1024];
	const char READ_HEAD[] = "rf_rx_data";
	int nRead = 0;
	fd_set rd;

	//pthread_detach(pthread_self()); //线程结束自动释放资源
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //设置本线程对Cancel信号的反应  CANCEL_ENABLE
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //设置本线程取消动作的执行时机 立即取消
	while (1) {
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		select(fd + 1, &rd, NULL, NULL, NULL);
		read(fd, buff + nRead, sizeof(char));
		if (buff[nRead] == '\n') {
			//接收完一帧数据过后的操作
			//上链等操作在这里完成
			const char n = '\n';
			putchar('\n');
			fflush(stdout);
			if (nRead < 1024 - 1)
				buff[nRead + 1] = 0;
			else
				buff[nRead] = 0;
			
			if (strstr(buff, READ_HEAD)!= NULL) {
				string buff_str(buff);
				if (buff_str.find('=') != string::npos) {
					buff_str = buff_str.substr(buff_str.find('=')+1);
					cout << buff_str << endl;
					dag_->pushInfo(buff_str);
				}
			}
			//write_data(&n, 1);

			nRead = 0;
		}
		else {
			//write_data((const char*)(buff + nRead), 1);
			printf("%c", buff[nRead]);
			fflush(stdout);
			++nRead;
		}

		if (nRead >= 1023) {
			nRead = 1023;
		}

	}
}
}

