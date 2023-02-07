#include "Utils.hpp"

const std::string Utils::getBotPass() const { return botPass; }

const std::string Utils::getBotUser() const { return botUser; }

const std::string Utils::getBotNick() const { return botNick; }

const std::string Utils::getBotHost() const { return botHost; }

const std::string Utils::getBotServ() const { return botServ; }

const std::string Utils::getBotName() const { return botName; }

const std::string Utils::getSrvName() const { return srvName; }

uint16_t Utils::getBotPort() const { return botPort; }

Utils::~Utils() {}

Utils::Utils(const std::string &conf) : conf(conf) {}

void Utils::parser() {
	std::ifstream configfile(conf);
	std::string line;

	if (!configfile.is_open()) {
		std::cout << "file open error" << std::endl;
		return ;
	}

	std::getline(configfile, line);
	botPass = line.substr(line.find('=') + 1);

	std::getline(configfile, line);
	botUser = line.substr(line.find('=') + 1);

	std::getline(configfile, line);
	botNick = line.substr(line.find('=') + 1);

	std::getline(configfile, line);
	botHost = line.substr(line.find('=') + 1);

	std::getline(configfile, line);
	botServ = line.substr(line.find('=') + 1);

	std::getline(configfile, line);
	botName = line.substr(line.find('=') + 1);

	std::getline(configfile, line);
	botPort = std::atoi((line.substr(line.find('=') + 1)).c_str());

	std::getline(configfile, line);
	srvName = line.substr(line.find('=') + 1);

	configfile.close();
}

void Utils::propertiesDebug() const {
	std::cout << "--- parser debug ---" << std::endl;
	std::cout << botPass << std::endl;
	std::cout << botUser << std::endl;
	std::cout << botNick << std::endl;
	std::cout << botHost << std::endl;
	std::cout << botServ << std::endl;
	std::cout << botPort << std::endl;
	std::cout << botName << std::endl;
	std::cout << srvName << std::endl;
	std::cout << "--------------------" << std::endl;
}

const std::string Utils::getConfName() const { return conf; }

const std::string Utils::getSender(const std::string &str) {
	return str.substr(1, str.find('!') - 1);
}

const std::string Utils::getMessage(const std::string &str) {
	std::string message = str.substr(str.rfind(':') + 1);
	return message.substr(0, message.size() - 1);
}

std::vector<std::string> Utils::cpp_split(const std::string &str, char sep) {
	std::vector<std::string> ret;
	std::string::const_iterator i;

	i = str.begin();
	while (i != str.end()) {
		while (i != str.end() && *i == sep)
			i++;
		std::string::const_iterator j = std::find(i, str.end(), sep);
		if (i != str.end()) {
			if (j != str.end())
				ret.push_back(std::string(i, j + 1));
			else
				ret.push_back(std::string(i, j));
			i = j;
		}
	}
	return ret;
}

const std::string Utils::getCommand(const std::string &str) {
	std::string command = str.substr(str.find(' ') + 1);
	return command.substr(0, command.find(' '));
}

/*
** сообщения могут приходить сразу по несколько штук,
** а могут приходить НЕ целиком за раз
*/

void Utils::action(const std::string &str, int sd) {
	std::vector<std::string> sm;
	std::vector<std::string>::iterator i;
	static std::string message_piece = ""; // здесь сохраняется НЕ цельное сообщение
	std::string message;

	sm = cpp_split(message_piece + str, '\n');
	i = sm.begin();
	while (i != sm.end()) {
		if ((*i).find('\n') != std::string::npos) { // если сообщение цельное
			std::cout << *i;


			if ((*i).substr(1, getSrvName().size() + 1) != getSrvName() + " " \
				&& getCommand(*i) == "PRIVMSG") { // если это НЕ сервер
				message = getMessage(*i);
				std::reverse(message.begin(), message.end());
				message = "PRIVMSG " + getSender(*i) + " :" + message + "\n";
				write(sd, message.c_str(), message.size());
			}

			if ((*i).substr(1, getSrvName().size() + 1) == getSrvName() + " ") { // если это сервер PING
				if ((*i).substr(getSrvName().size() + 2, 5) == "PING ") {
					write(sd, ("PONG " + getSrvName() + "\n").c_str(), ("PONG " + getSrvName() + "\n").size());
				}
			}


		}
		else // если сообщение НЕ цельное
			message_piece = *i;
		i++;
	}
}
