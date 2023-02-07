#ifndef UTILS_HPP
# define UTILS_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <vector>
# include <unistd.h>

class Utils {
	private:
	std::string botPass;
	std::string botUser;
	std::string botNick;
	std::string botHost;
	std::string botServ;
	std::string botName;
	std::string srvName;
	uint16_t    botPort;
	const std::string conf;
	Utils();

	public:
	const std::string getBotPass() const;
	const std::string getBotUser() const;
	const std::string getBotNick() const;
	const std::string getBotHost() const;
	const std::string getBotServ() const;
	const std::string getBotName() const;
	const std::string getSrvName() const;
	uint16_t getBotPort() const;
	~Utils();
	Utils(const std::string &conf);
	void parser();
	const std::string getConfName() const;
	void propertiesDebug() const;
	const std::string getSender(const std::string &str);
	const std::string getMessage(const std::string &str);
	const std::string getCommand(const std::string &str);
	void action(const std::string &str, int sd);
	std::vector<std::string> cpp_split(const std::string &str, char sep);
};

#endif
