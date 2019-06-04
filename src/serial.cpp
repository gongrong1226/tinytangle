#include <serial.h>
namespace embeded_data {

using namespace tangle;
serial::serial(const char* uart, int baud_rate, Dag* dag):uart(uart), baud_rate(baud_rate), dag_(dag){
	//if ((fd = open(uart, O_RDWR | O_CREAT, 0777)) < 0){	//�򿪴���
	if ((fd = open(uart, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) {	//�򿪴���
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
	 
	if (tcgetattr(fd, &options) < 0)	//��ȡ�����ն�����
	{
		log::error("serial") << "tcgetattr error\n";
		return -1;
	}
	cfmakeraw(&options);   //config to raw mode   shabi code
	/*���ÿ���ģʽ*/
	//options.c_cflag |= CLOCAL;//��֤����ռ�ô���
	//options.c_cflag |= CREAD;//��֤������ԴӴ����ж�ȡ����

	/*��������λ:��λ*/
	//options.c_cflag &= ~CSIZE;//����������־λ
	//options.c_cflag |= CS8;  //��λ

	/*����У��λ����У��*/
	//options.c_cflag &= ~PARENB;//PARENB��������żλ��ִ����żУ��
	//options.c_cflag &= ~INPCK;//INPCK��ʹ��żУ��������

	/*����ֹͣλ��һλ*/
	//options.c_cflag &= ~CSTOPB;//CSTOPB��ʹ��һλֹͣλ

	/*���õȴ�ʱ�����С�����ַ�*/
	//options.c_cc[VTIME] = 0;//������select������
	//options.c_cc[VMIN] = 1;//���ٶ�ȡһ���ַ�
	 
	/*��մ���Buff*/
	//tcflush(fd, TCIFLUSH);

	/*���ò�����*/
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

	/*��������*/
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
@brief ʹ�ô��ڷ����ַ���
@param src �ַ����׵�ַ
@parama length ��Ҫ���͵ĳ���
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
	while (nwritten != length) { //δ����ʱ����
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
		//pthread_detach(thread_t); //�߳̽����Զ��ͷ���Դ
	}*/
}

void serial::read_thread() {
	char buff[1024];
	const char READ_HEAD[] = "rf_rx_data";
	int nRead = 0;
	fd_set rd;

	//pthread_detach(pthread_self()); //�߳̽����Զ��ͷ���Դ
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //���ñ��̶߳�Cancel�źŵķ�Ӧ  CANCEL_ENABLE
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL); //���ñ��߳�ȡ��������ִ��ʱ�� ����ȡ��
	while (1) {
		FD_ZERO(&rd);
		FD_SET(fd, &rd);
		select(fd + 1, &rd, NULL, NULL, NULL);
		read(fd, buff + nRead, sizeof(char));
		if (buff[nRead] == '\n') {
			//������һ֡���ݹ���Ĳ���
			//�����Ȳ������������
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

