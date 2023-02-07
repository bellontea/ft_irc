#include "Socket.hpp"

/*
** создать сокет, который может
** использоваться для работы в IP-сети
** через предварительно установленное соединение
** с использованием протокола TCP по умолчанию
*/

void Socket::create_socket() {
	int sd;

	sd = socket(AF_INET, SOCK_STREAM, 0);
	if (sd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	m_sd = sd;
}

/*
** заполнить структуру данными о том, что
** сокет может использоваться для работы в IP-сети
** и данными о порте и хосте,
** предварительно определив порядок хоста и сетевой порядок
*/

void Socket::set_addr() {
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(m_port);
	m_addr.sin_addr.s_addr = inet_addr(m_address.c_str());
}

Socket::Socket(uint16_t port, const std::string address) {
	m_port = port;
	m_address = address;
}

/*
** связать сокет и адрес в выбранном домене
*/

void Socket::bind_socket() {
	int b;
	int opt;

	opt = 1;
	setsockopt(m_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	b = bind(m_sd, (struct sockaddr *)&m_addr, sizeof(m_addr));
	if (b == -1) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
}

Socket::~Socket() {
	close(m_sd);
}

/*
** перевести сокет в режим ожидания запросов со стороны клиента
*/

void Socket::listen_socket() {
	int l;
	int flags;

	l = listen(m_sd, 128);
	if (l < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	flags = fcntl(m_sd, F_GETFL);
	fcntl(m_sd, F_SETFL, flags | O_NONBLOCK);
}

int Socket::getSocket() const {
	return m_sd;
}

int Socket::get_accept_sd() const {
	return m_accept_sd;
}

/*
** принимать запросы со стороны клиента
*/

void Socket::accept_request() {
	int accept_sd;

	accept_sd = accept(m_sd, NULL, NULL);
	if (accept_sd == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}
	m_accept_sd = accept_sd;
}

/*
** связать сокет и адрес в выбранном домене
** подключиться к серверу
*/

void Socket::connect_socket() {
	int c;

	c = connect(m_sd, (struct sockaddr *)&m_addr, sizeof(m_addr));
	if (c == -1) {
		perror("connect");
		exit(EXIT_FAILURE);
	}
}

/*
** отправить сообщение
*/

ssize_t Socket::send_message(const std::string &message, int sd) {
	ssize_t bytes;

	bytes = write(sd, message.c_str(), message.length());
	return bytes;
}

/*
** прочитать принятое сообщение
*/

const std::string Socket::recive_message(int sd) {
	ssize_t bytes;
	char buf[1024];
	std::string message;

	while ((bytes = read(sd, buf, 1023)) > 0) {
		buf[bytes] = '\0';
		message += buf;
	}
	return message;
}

void Socket::activateNonBlockMode() {
	int flags;

	flags = fcntl(m_sd, F_GETFL);
	fcntl(m_sd, F_SETFL, flags | O_NONBLOCK);
}
