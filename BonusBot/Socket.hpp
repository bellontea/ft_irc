#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <signal.h>
# include <unistd.h>
# include <fcntl.h>
# include <poll.h>
# include <stdlib.h>
# include <csignal>

class Socket {

	private:

	Socket();
	int         m_sd;
	struct      sockaddr_in m_addr;
	uint16_t    m_port;
	std::string m_address;
	int         m_accept_sd;

	public:
	void create_socket();
	void set_addr();
	Socket(uint16_t port, const std::string address);
	void bind_socket();
	void connect_socket();
	~Socket();
	ssize_t send_message(const std::string &message, int sd);
	const std::string recive_message(int sd);
	void listen_socket();
	int getSocket() const;
	void accept_request();
	int get_accept_sd() const;
	void activateNonBlockMode();

};

#endif
