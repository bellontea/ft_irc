#include "Channel.hpp"
#include <sstream>

Channel::Channel(const std::string &name, const User &author, const std::string &password) :
	name(name),
	password(password),
	maxUsersLimit(0),
	flagPrivate(false),
	flagSecret(false),
	flagModerated(false),
	flagInviteOnly(false),
	flagTopicSet(false)
{
	usersList.push_back(&author);
	operatorsList.push_back(&author);
	getInfo(author);
}

Channel::~Channel()
{}
// private
bool	Channel::isInBan(const std::string &mask, const std::string &prefix) {
	return (checkRegex(mask, prefix));
}

void	Channel::getInfo(const User &user) {
	sendMessage("JOIN :" + name + "\n", user, true);
	ReplyWithTopic(user);
	ReplyWithUsersList(user);
	sendReplyMsg(user.getServername(), user, RPL_ENDOFNAMES, name);
}
// геттеры

const std::string	&Channel::getName() const {
	return (name);
}

bool	Channel::getFlagPrivate() const {
	return flagPrivate;
}

bool	Channel::getFlagSecret() const {
	return flagSecret;
}

bool	Channel::getFlagModerated() const {
	return flagModerated;
}

bool	Channel::getFlagInviteOnly() const {
	return flagInviteOnly;
}

bool	Channel::getFlagTopicSet() const {
	return flagTopicSet;
}
// сеттеры
void	Channel::setPassword(const User &user, const std::string &password)
{
	if (!password.empty())
		sendErrorMsg(user, ERR_KEYSET, name);
	else
		this->password = password;
}
void	Channel::setTopic(const User &user, const std::string &newTopic)
{
	if (getFlagTopicSet() && !isUserOperator(user))
		sendErrorMsg(user, ERR_CHANOPRIVSNEEDED, name);
	else {
		this->topic = newTopic;
		sendMessage("TOPIC " + name + " :" + this->topic + "\n", user, true);
	}
}

void	Channel::setUserLimit(unsigned int maxUsersLimit) {
	this->maxUsersLimit = maxUsersLimit;
}

void	Channel::setFlagPrivate(bool flagPrivate) {
	this->flagPrivate = flagPrivate;
}

void	Channel::setFlagSecret(bool flagSecret) {
	this->flagSecret = flagSecret;
}

void	Channel::setFlagModerated(bool flagModerated) {
	this->flagModerated = flagModerated;
}

void	Channel::setFlagInviteOnly(bool flagInviteOnly) {
	this->flagInviteOnly = flagInviteOnly;
}

void	Channel::setFlagTopicSet(bool flagTopicSet) {
	this->flagTopicSet = flagTopicSet;
}
//методы
std::string	Channel::getFlagsForUsers() const {
	std::string	result;
	if (getFlagPrivate())
		result += "p";
	if (getFlagSecret())
		result += "s";
	if (getFlagModerated())
		result += "m";
	if (getFlagInviteOnly())
		result += "i";
	if (getFlagTopicSet())
		result += "t";
	return result;
}

bool	Channel::isUserInvited(const User &user) const {
	for (size_t i = 0; i < invitationsList.size(); i++)
		if (invitationsList[i]->getPrefix() == user.getPrefix())
			return true;
	return false;
}

bool	Channel::isUserOperator(const User &user) const {
	for (size_t i = 0; i < operatorsList.size(); i++)
		if (operatorsList[i]->getPrefix() == user.getPrefix())
			return true;
	return false;
}

bool	Channel::isInChannel(const std::string &nickname) const {
	std::vector<const User *>::const_iterator	start = usersList.begin();
	std::vector<const User *>::const_iterator	finish = usersList.end();
	while (start != finish) {
		if ((*start)->getNickname() == nickname)
			return true;
		start++;
	}
	return false;
}

void	Channel::ReplyWithTopic(const User &user) {
	if (topic.empty())
		sendReplyMsg(user.getServername(), user, RPL_NOTOPIC, name);
	else
		sendReplyMsg(user.getServername(), user, RPL_TOPIC, name, topic);
}

void	Channel::ReplyWithUsersList(const User &user) {
	std::string	namesStr;
	std::vector<const User *>::const_iterator	start = usersList.begin();
	std::vector<const User *>::const_iterator	finish = usersList.end();
	while (start != finish) {
		const User	*tmpUser = *start;
		if (isUserOperator(*tmpUser))
			namesStr += "@";
		else if (isVoter(*tmpUser))
			namesStr += "+";
		namesStr += tmpUser->getNickname();
		start++;
		if (start != finish)
			namesStr += " ";
	}
	sendReplyMsg(user.getServername(), user, RPL_NAMREPLY, "= " + name, namesStr);
}

void	Channel::ReplyWithInfo(const User &user) {
	std::string	channelName, infoMsg;

	if (getFlagSecret() && !isInChannel(user.getNickname()))
		return ;
	if (getFlagPrivate() && !isInChannel(user.getNickname()))
		channelName = "*";
	else {
		channelName = name;
		infoMsg = "[+" + getFlagsForUsers() + "] " + topic;
	}
	std::stringstream	ss;
	ss << usersList.size();
	sendReplyMsg(user.getServername(), user, RPL_LIST, channelName, ss.str(), infoMsg);
}

bool	Channel::isVoter(const User &user) const {
	for (size_t i = 0; i < votersList.size(); i++)
		if (votersList[i]->getPrefix() == user.getPrefix())
			return true;
	return false;
}

bool	Channel::isEmptyChannel() const {
	if (usersList.empty())
		return true;
	return false;
}

void	Channel::connect(const User &user, const std::string &pass) {
	if (getFlagPrivate() && pass != password)
		sendErrorMsg(user, ERR_BADCHANNELKEY, name);
	else if (maxUsersLimit != 0 && usersList.size() >= maxUsersLimit)
		sendErrorMsg(user, ERR_CHANNELISFULL, name);
	else if (getFlagInviteOnly() && !isUserInvited(user))
		sendErrorMsg(user, ERR_INVITEONLYCHAN, name);
	else {
		for (size_t i = 0; i < banMasksList.size(); i++) {
			if (isInBan(banMasksList[i], user.getPrefix())) {
				sendErrorMsg(user, ERR_BANNEDFROMCHAN, name);
				return ;
			}
		}
		std::vector<const User *>::iterator	start = usersList.begin();
		std::vector<const User *>::iterator	finish = usersList.end();
		while (start != finish) {
			if ((*start)->getPrefix() == user.getPrefix())
				return ;
			start++;
		}
		usersList.push_back(&user);
		removeInvitation(user);
		getInfo(user);
	}
}

void	Channel::sendMessage(const std::string &message, const User &fromUser, bool includeUser) const
{
	std::string	msg;
	msg += ":" + fromUser.getPrefix() + " " + message;
	std::vector<const User *>::const_iterator	start = usersList.begin();
	std::vector<const User *>::const_iterator	finish = usersList.end();
	while (start != finish) {
		if (includeUser || *start != &fromUser)
			(*start)->sendCommand(msg);
		start++;
	}
}

void	Channel::inviteUser(const User &user, const User &invitedUser) {
	if (getFlagInviteOnly() && !isUserOperator(user))
		sendErrorMsg(user, ERR_CHANOPRIVSNEEDED, name);
	else {
		invitationsList.push_back(&invitedUser);
		invitedUser.sendCommand(":" + user.getPrefix() + " INVITE " + invitedUser
		.getNickname() + " :" + name + "\n");
		sendReplyMsg(user.getServername(), user, RPL_INVITING, name,
			invitedUser.getNickname());
		if (invitedUser.getFlagAway())
			sendReplyMsg(user.getServername(), user, RPL_AWAY,
				invitedUser.getNickname(), invitedUser.getAwayMessage());
	}
}

void	Channel::addToOperators(const User &user) {
	if (!isUserOperator(user))
		operatorsList.push_back(&user);
}

void	Channel::removeFromOperators(const User &user) {
	if (isUserOperator(user)) {
		size_t	i;
		for (i = 0; i < operatorsList.size(); i++)
			if (operatorsList[i] == &user)
				break;
		operatorsList.erase(operatorsList.begin() + i);
		if (operatorsList.empty() && !usersList.empty()) {
			operatorsList.push_back(usersList[0]);
			sendMessage("MODE " + this->name + " +o " + usersList[0]->getNickname() + "\n", user, true);
		}
	}
}

void	Channel::addToVotersList(const User &user) {
	if (!isVoter(user))
		votersList.push_back(&user);
}

void	Channel::removeFromVotersList(const User &user) {
	if (isVoter(user)) {
		size_t	i;
		for (i = 0; i < votersList.size(); i++)
			if (votersList[i] == &user)
				break;
		votersList.erase(votersList.begin() + i);
	}
}

void	Channel::addToBanMaskList(const std::string &mask) {
	banMasksList.push_back(mask);
}

void	Channel::removeFromBanMaskList(const std::string &mask) {
	size_t	i;
	for (i = 0; i < banMasksList.size(); i++)
		if (banMasksList[i] == mask)
			break;
	banMasksList.erase(banMasksList.begin() + i);
}

void	Channel::disconnectUser(const User &user) {
	std::vector<const User *>::iterator	start = usersList.begin();
	std::vector<const User *>::iterator	finish = usersList.end();
	while (start != finish) {
		if (*start == &user)
			break ;
		start++;
	}
	usersList.erase(start);
	removeFromOperators(user);
	removeFromVotersList(user);
}

void	Channel::removeInvitation(const User &user) {
	if (isUserInvited(user)) {
		size_t	i;
		for (i = 0; i < invitationsList.size(); i++)
			if (invitationsList[i] == &user)
				break;
		invitationsList.erase(invitationsList.begin() + i);
	}
}
