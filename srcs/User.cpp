#include "User.hpp"

User::User(int fd, std::string hostname, std::string servername) :
	fd(fd),
	hostname (hostname),
	servername(servername),
	registrationTime(time(0)),
	flagRegistered(false),
	flagInvisible(false),
	flagReceiveNotice(true),
	flagReceiveWallops(false),
	flagIrcOperator(false),
	flagAway(false),
	flagPinging(false),
	flagBreakConnection(false)
{
}

User::~User() {}
// Геттеры
int User::getFd() const{
	return fd;
}

const std::string &User::getPassword() const {
	return password;
}

const std::string &User::getNickname() const {
	return nickname;
}

const std::string &User::getUsername() const {
	return username;
}

const std::string &User::getHostname() const {
	return hostname;
}

const std::string &User::getServername() const {
	return servername;
}

const std::string &User::getRealname() const {
	return realname;
}

const std::string &User::getAwayMessage() const {
	return awayMessage;
}

const std::string &User::getQuitMessage() const {
	return quitMessage;
}

const time_t &User::getRegistrationTime() const {
	return registrationTime;
}

const time_t &User::getTimeOfLastCommand() const {
	return timeOfLastCommand;
}

const time_t &User::getTimeAfterPing() const {
	return timeAfterPing;
}

const std::queue<std::string> &User::getCommands() const {
	return commands;
}

std::string	User::getPrefix() const
{
	return nickname + "!" + username + "@" + hostname;
}

bool	User::getFlagRegistered() const {
	return flagRegistered;
}

bool	User::getFlagInvisible() const {
	return flagInvisible;
}

bool	User::getFlagReceiveNotice() const {
	return flagReceiveNotice;
}

bool	User::getFlagReceiveWallops() const {
	return flagReceiveWallops;
}

bool	User::getFlagIrcOperator() const {
	return flagIrcOperator;
}

bool	User::getFlagAway() const {
	return flagAway;
}

bool	User::getFlagPinging() const {
	return flagPinging;
}

bool	User::getFlagBreakConnection() const {
	return flagBreakConnection;
}

const std::vector<const Channel *>	&User::getUserChannels() const {
	return channels;
}
// Сеттеры
void User::setPassword(const std::string &password) {
	this->password = password;
}

void User::setNickname(const std::string &nickname) {
	this->nickname = nickname;
}

void User::setUsername(const std::string &username) {
	this->username = username;
}

void User::setServername(const std::string &servername) {
	this->servername = servername;
}

void User::setRealname(const std::string &realname) {
	this->realname = realname;
}

void User::setAwayMessage(const std::string &awayMessage) {
	this->awayMessage = awayMessage;
}

void User::setQuitMessage(const std::string &quitMessage) {
	this->quitMessage = quitMessage;
}

void User::setFlagRegistered(bool flagRegistered) {
	this->flagRegistered = flagRegistered;
}

void User::setFlagInvisible(bool flagInvisible) {
	this->flagInvisible = flagInvisible;
}

void User::setFlagReceiveNotice(bool flagReceiveNotice) {
	this->flagReceiveNotice = flagReceiveNotice;
}

void User::setFlagReceiveWallops(bool flagReceiveWallops) {
	this->flagReceiveWallops = flagReceiveWallops;
}

void User::setFlagIrcOperator(bool flagIrcOperator) {
	this->flagIrcOperator = flagIrcOperator;
}

void User::setFlagAway(bool flagAway) {
	this->flagAway = flagAway;
}

void User::setFlagPinging(bool flagPinging) {
	this->flagPinging = flagPinging;
}

void User::setFlagBreakConnection(bool flagBreakConnection) {
	this->flagBreakConnection = flagBreakConnection;
	if (quitMessage.empty())
		quitMessage = "Client exited";
}

// методы
void	User::updateTimeOfLastCommand() {
	this->timeOfLastCommand = time(0);
}

void	User::updateTimeAfterPing() {
	this->timeAfterPing = time(0);
}

void	User::sendCommand(const std::string &command) const {
	if (!command.empty())
		send(fd, command.c_str(), command.size(), SRV_NOSIGNAL);
}

int		User::recvCommand() {
	std::string	text;
	ssize_t bytesRead;
	char buffer[300];
	if (!commands.empty())
		text = commands.front();
	while ((bytesRead = recv(fd, buffer, 299, 0)) > 0) {
		buffer[bytesRead] = 0;
		text += buffer;
		buffer[0] = 0;
		if (text.find('\n') != std::string::npos)
			break;
	}
	if (bytesRead == 0)
		return (DISCONNECT);
	if (text.length() > 512)
		text = text.substr(0, 510) + "\r\n";
	while (text.find("\r\n") != std::string::npos)
		text.replace(text.find("\r\n"), 2, "\n");
	if (text.size() > 1)
		commands = splitToStrQueue(text, '\n', true);
	return 0;
}

void	User::popCommand() {
	if (!commands.empty())
		commands.pop();
}

bool	User::isOnChannel(const std::string &name) const {
	size_t i = 0;
	while (i < channels.size()) {
		if (channels[i]->getName() == name)
			return true;
		i++;
	}
	return false;
}

void	User::enterChannel(const Channel &channel) {
	channels.push_back(&channel);
}

void	User::quitChannel(const std::string &name) {
	std::vector<const Channel *>::iterator	start = channels.begin();
	std::vector<const Channel *>::iterator	end = channels.end();
	while (start != end) {
		if ((*start)->getName() == name)
			break ;
		start++;
	}
	channels.erase(start);
}
