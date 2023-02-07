#include "Server.hpp"
#include <iostream>
#include <fstream>
#include <limits>
#include <poll.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string &pass) :
	port(port),
	fd(-1),
	channelLimit(10),
	inactiveTimeoutLimit(120),
	responseTimeoutLimit(60),
	rebootScheduled(false),
	pollTimeout(1),
	password(pass),
	serverName("None"),
	serverInfo("None"),
	serverVersion("None"),
	serverDescribe("None"),
	adminUserName("admin"),
	adminNickname("Admin"),
	adminMail("None"),
	bindIP(0UL)
{
	commands["USER"] = &Server::userWorker;
	commands["NICK"] = &Server::nickWorker;
	commands["PASS"] = &Server::passWorker;
	commands["OPER"] = &Server::operWorker;
	commands["QUIT"] = &Server::quitWorker;
	commands["AWAY"] = &Server::awayWorker;
	commands["KILL"] = &Server::killWorker;
	commands["RESTART"] = &Server::restartWorker;
	commands["REHASH"] = &Server::rehashWorker;
	commands["PRIVMSG"] = &Server::privmsgWorker;
	commands["NOTICE"] = &Server::noticeWorker;
	commands["WHOWAS"] = &Server::whowasWorker;
	commands["WHOIS"] = &Server::whoisWorker;
	commands["WHO"] = &Server::whoWorker;
	commands["JOIN"] = &Server::joinWorker;
	commands["PART"] = &Server::partWorker;
	commands["MODE"] = &Server::modeWorker;
	commands["TOPIC"] = &Server::topicWorker;
	commands["NAMES"] = &Server::namesWorker;
	commands["LIST"] = &Server::listWorker;
	commands["INVITE"] = &Server::inviteWorker;
	commands["KICK"] = &Server::kickWorker;
	commands["VERSION"] = &Server::versionWorker;
	commands["TIME"] = &Server::timeWorker;
	commands["ADMIN"] = &Server::adminWorker;
	commands["INFO"] = &Server::infoWorker;
	commands["PING"] = &Server::pingWorker;
	commands["PONG"] = &Server::pongWorker;
	commands["WALLOPS"] = &Server::wallopsWorker;
	commands["USERHOST"] = &Server::userhostWorker;
	commands["ISON"] = &Server::isonWorker;
}

Server::~Server()
{
	for (size_t i = 0; i < users.size(); i++)
	{
		close(users[i]->getFd());
		delete users[i];
	}
	for (size_t i = 0; i < usersDeprecated.size(); i++)
	{
		close(usersDeprecated[i]->getFd());
		delete usersDeprecated[i];
	}
	close(fd);
}

void	Server::applyConfigFromMap(const std::map<std::string, std::string>& cfgMap)
{
	try {
		serverName = cfgMap.at("serverName");
	} catch (const std::exception& e) {
		log("Can't find \"serverName\" in config, using default: " + serverName);
	}

	try {
		serverInfo = cfgMap.at("serverInfo");
	} catch (const std::exception& e) {
		log("Can't find \"serverInfo\" in config, using default: " + serverInfo);
	}

	try {
		serverVersion = cfgMap.at("serverVersion");
	} catch (const std::exception& e) {
		log("Can't find \"serverVersion\" in config, using default: " + serverVersion);
	}

	try {
		serverDescribe = cfgMap.at("serverDescribe");
	} catch (const std::exception& e) {
		log("Can't find \"serverDescribe\" in config, using default: " + serverDescribe);
	}

	try {
		adminUserName = cfgMap.at("adminUserName");
	} catch (const std::exception& e) {
		log("Can't find \"adminUserName\" in config, using default: " + adminUserName);
	}

	try {
		adminNickname = cfgMap.at("adminNickname");
	} catch (const std::exception& e) {
		log("Can't find \"adminNickname\" in config, using default: " + adminNickname);
	}

	try {
		adminMail = cfgMap.at("adminMail");
	} catch (const std::exception& e) {
		log("Can't find \"adminMail\" in config, using default: " + adminMail);
	}

	try {
		bindIP = inet_addr(cfgMap.at("bindIP").c_str());
	} catch (const std::exception& e) {
		bindIP = inet_addr("0.0.0.0");
		log("Can't find \"bindIP\" in config, using default: 0.0.0.0");
	}

	try {
		long tmp = string_to_long(cfgMap.at("channelLimit"));
		if (tmp < 1 || tmp > std::numeric_limits<int>::max())
			throw std::out_of_range ("Param out of range");
		channelLimit = tmp;
	} catch (const std::exception& e) {
		log("Can't find correct \"channelLimit\" in config, using default: "
		+ long_to_string((long)channelLimit));
	}

	try {
		long tmp = string_to_long(cfgMap.at("inactiveTimeoutLimit"));
		if (tmp < 1 || tmp > std::numeric_limits<int>::max())
			throw std::out_of_range ("Param out of range");
		inactiveTimeoutLimit = (int)tmp;
	} catch (const std::exception& e) {
		log("Can't find correct \"inactiveTimeoutLimit\" in config, using default: " + long_to_string(inactiveTimeoutLimit));
	}

	try {
		long tmp = string_to_long(cfgMap.at("responseTimeoutLimit"));
		if (tmp < 1 || tmp > std::numeric_limits<int>::max())
			throw std::out_of_range ("Param out of range");
		responseTimeoutLimit = (int)tmp;
	} catch (const std::exception& e) {
		log("Can't find correct \"responseTimeoutLimit\" in config, using default: " + long_to_string(responseTimeoutLimit));
	}
}

void Server::getConfig(const std::string& path)
{
	std::map<std::string, std::string> cfgMap;
	std::ifstream cfgFile (path.c_str(), std::ios::in);
	std::string line;

	if (cfgFile.is_open())
	{
		std::string paramName;

		while(getline(cfgFile, line))
		{
			if(line.empty() || line[0] == '#')
				continue;
			size_t delimiterPos = line.find('=');
			if (delimiterPos == std::string::npos)
			{
				log("Incorrect config file syntax! Missing \"=\" in nonempty line.");
				throw std::invalid_argument ("Incorrect config file syntax!");
			}
			paramName = trim(line.substr(0, delimiterPos));
			if (paramName == "operator")
			{
				std::string operatorData = trim(line.substr(delimiterPos + 1));
				delimiterPos = operatorData.find('=');
				if (delimiterPos == std::string::npos)
				{
					log("Incorrect config file syntax! Missing second \"=\" in operator line.");
					throw std::invalid_argument ("Incorrect config file syntax!");
				}
				paramName = trim(operatorData.substr(0, delimiterPos));
				operatorsList[paramName] = trim(operatorData.substr(delimiterPos + 1));
			}
			else
				cfgMap[paramName] = trim(line.substr(delimiterPos + 1));
		}

		std::ifstream motdStorageFile(SRV_MOTD_FILE, std::ios::in);
		if (motdStorageFile.is_open())
		{
			while (getline(motdStorageFile, line))
				motdStorage.push_back(line);
			motdStorageFile.close();
		}
	}
	else
	{
		log("Couldn't open config file for reading");
		throw std::runtime_error ("Couldn't open config file for reading");
	}
	applyConfigFromMap(cfgMap);
}

void	Server::initNewSocket()
{
	///creating socket
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
	{
		log("Failed to create socket");
		throw std::runtime_error ("Failed to create socket");
	}

	///binding
	const int trueFlag = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(int)) < 0)
	{
		log("setsockopt() failed");
		throw std::runtime_error ("setsockopt() failed");
	}
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = bindIP;
	sockAddr.sin_port = htons(port);
	if (bind(fd, (struct sockaddr*)&sockAddr, sizeof(sockAddr)) < 0)
	{
		log("Failed to bind to port " + long_to_string((long)port));
		throw std::runtime_error ("Failed to bind to port " + long_to_string((long)port));
	}

	if (listen(fd, 128) < 0)
	{
		log("Failed to listen on socket");
		throw std::runtime_error ("Failed to listen on socket");
	}
	fcntl(fd, F_SETFL, O_NONBLOCK);
	log("Server started successfully and waiting for connections");
}

void	Server::acceptConnection()
{
	size_t addrSize = sizeof(sockAddr);
	int userFD = accept(fd, (struct sockaddr*)&sockAddr, (socklen_t*)&addrSize);
	if (userFD >= 0)
	{
		struct	pollfd	tmpUserFD;
		char	hostname[INET_ADDRSTRLEN];

		tmpUserFD.fd = userFD;
		tmpUserFD.events = POLLIN;
		tmpUserFD.revents = 0;
		userConnectionFDs.push_back(tmpUserFD);

		inet_ntop(AF_INET, &(sockAddr.sin_addr), hostname, INET_ADDRSTRLEN);
		users.push_back(new User(userFD, hostname, serverName));
	}
}

void	Server::checkNewCommands()
{
	int	status = poll(userConnectionFDs.data(), userConnectionFDs.size(), pollTimeout);
	if (status > 0)
	{
		for (size_t i = 0; i < userConnectionFDs.size(); i++)
		{
			if (userConnectionFDs[i].revents & POLLIN)
			{
				if (users[i]->recvCommand() == DISCONNECT
					|| processCommands(*(users[i])) == DISCONNECT)
					users[i]->setFlagBreakConnection(true);
				userConnectionFDs[i].revents = 0;
			}
		}
	}
	else if (status < 0 && !stopScheduled)
	{
		log("Poll error occurred!");
		throw std::runtime_error ("Poll error occurred!");
	}
}

int		Server::processCommands(User &user)
{
	while (!user.getCommands().empty()
			&& user.getCommands().front()[user.getCommands().front().size() - 1] == '\n')
	{
		Command	cmd(user.getCommands().front());
		user.popCommand();
		if (!(user.getFlagRegistered()) && cmd.getCommand() != "QUIT" &&
			cmd.getCommand() != "PASS" &&
			cmd.getCommand() != "USER" && cmd.getCommand() != "NICK")
			sendErrorMsg(user, ERR_NOTREGISTERED);
		else
		{
			try
			{
				if ((this->*(commands.at(cmd.getCommand())))(cmd, user) == DISCONNECT)
					return (DISCONNECT);
			}
			catch(const std::exception& e)
			{
				sendErrorMsg(user, ERR_UNKNOWNCOMMAND, cmd.getCommand());
			}
		}
	}
	user.updateTimeOfLastCommand();
	return (0);
}

void	Server::clearBrokenConnections()
{
	for (size_t i = 0; i < users.size(); ++i)
	{
		if (users[i]->getFlagBreakConnection())
		{
			sendBroadcastNotification(*(users[i]),
				":" + users[i]->getPrefix() + " QUIT :" + users[i]->getQuitMessage() + "\n");
			close(users[i]->getFd());
			std::map<std::string, Channel *>::iterator	pos = channels.begin();
			std::map<std::string, Channel *>::iterator	end = channels.end();
			while (pos != end) {
				if ((*pos).second->isInChannel(users[i]->getNickname()))
					(*pos).second->disconnectUser(*(users[i]));
				pos++;
			}
			usersDeprecated.push_back(users[i]);
			users.erase(users.begin() + i);
			userConnectionFDs.erase(userConnectionFDs.begin() + i);
			--i;
		}
	}
}

void	Server::sendBroadcastNotification(User &initiatorUser, const std::string &msg)
{
	const std::vector<const Channel *> initiatorChannels = initiatorUser.getUserChannels();
	for (size_t i = 0; i < users.size(); i++)
	{
		for (size_t j = 0; j < initiatorChannels.size(); j++)
		{
			if (initiatorChannels[j]->isInChannel(users[i]->getNickname()))
			{
				users[i]->sendCommand(msg);
				break ;
			}
		}
	}
}

void	Server::removeEmptyChannels()
{
	std::map<std::string, Channel *>::const_iterator	pos = channels.begin();
	std::map<std::string, Channel *>::const_iterator	end = channels.end();
	while (pos != end)
	{
		if ((*pos).second->isEmptyChannel())
		{
			delete (*pos).second;
			channels.erase((*pos).first);
			pos = channels.begin();
		}
		else
			++pos;
	}
}

void	Server::checkActiveUsers()
{
	for (size_t i = 0; i < users.size(); i++)
	{
		if (this->users[i]->getFlagRegistered())
		{
			if (time(0) - this->users[i]->getTimeOfLastCommand() > static_cast<time_t>(inactiveTimeoutLimit) )
			{
				this->users[i]->sendCommand(":" + this->serverName + " PING :" + this->serverName + "\n");
				this->users[i]->updateTimeAfterPing();
				this->users[i]->updateTimeOfLastCommand();
				this->users[i]->setFlagPinging(true);
			}
			if ((users[i]->getFlagPinging()) && time(0) - users[i]->getTimeAfterPing() > static_cast<time_t>(responseTimeoutLimit))
				users[i]->setFlagBreakConnection(true);
		}
	}
}

User	*Server::getUserByName(const std::string &userName) const
{
	size_t i = users.size() - 1;
	while (i > 0) {
		if (users[i]->getNickname() == userName)
			return users[i];
		i--;
	}
	if (i == 0 && users[i]->getNickname() == userName)
		return users[i];
	return NULL;
}

Channel	*Server::getChannelByName(const std::string &channelName) const
{
	try
	{
		return channels.at(channelName);
	}
	catch(const std::exception& e)
	{
		return NULL;
	}
}

int		Server::isRebootScheduled()
{
	return rebootScheduled;
}

void	Server::setRebootScheduled(bool rebootScheduled)
{
	this->rebootScheduled = rebootScheduled;
}
