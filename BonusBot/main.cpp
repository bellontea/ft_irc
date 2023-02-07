#include "Socket.hpp"
#include "Utils.hpp"

bool exec_serv = true;

void signal_handler(int signal) {
	(void)signal;
	exec_serv = false;
}

/*
** argv[1] == config
*/

int main(int argc, char **argv) {

	if (argc != 2) {
		std::cout << "DEBUG: need config" << std::endl;
		return 1;
	}

	Utils utils(argv[1]);
	utils.parser();
	utils.propertiesDebug();

	Socket sock(utils.getBotPort(), utils.getBotServ());
	sock.create_socket();
	sock.set_addr();
	sock.connect_socket();
	sock.activateNonBlockMode();
	signal(SIGINT, signal_handler);

	sock.send_message("PASS " + utils.getBotPass() + "\n", sock.getSocket());
	sock.send_message("USER " + utils.getBotUser() + " " + utils.getBotHost() \
	+ " " + utils.getBotServ() + " :" + utils.getBotName() + "\n", sock.getSocket());
	sock.send_message("NICK " + utils.getBotNick() + "\n", sock.getSocket());

	std::string ans;
	while (exec_serv) {
		ans = sock.recive_message(sock.getSocket());
		utils.action(ans, sock.getSocket());
	}

	return 0;
}
