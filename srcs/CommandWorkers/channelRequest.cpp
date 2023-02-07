#include "Server.hpp"
#include <limits>

int	Server::joinWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand());
	else {
		std::queue<std::string>	tmpChannels = splitToStrQueue(
			cmd.getParams()[0], ',', false);
		std::queue<std::string>	keys;
		std::string	key;
		if (cmd.getParams().size() > 1)
			keys = splitToStrQueue(cmd.getParams()[1], ',', false);
		while (!tmpChannels.empty()) {
			if (!keys.empty())
				key = keys.front();
			else
				key = "";
			if (!keys.empty())
				keys.pop();
			if (!checkChannelname(tmpChannels.front()))
				sendErrorMsg(user, ERR_NOSUCHCHANNEL, tmpChannels.front());
			else if (user.getUserChannels().size() >= channelLimit)
				sendErrorMsg(user, ERR_TOOMANYCHANNELS, tmpChannels.front());
			else {
				std::string name = tmpChannels.front();
				try {
					Channel	*tmp = channels.at(name);
					tmp->connect(user, key);
				} catch (const std::exception& e) {
					channels[name] = new Channel(name, user, key);
				}
				user.enterChannel(*(channels.at(tmpChannels.front())));
			}
			tmpChannels.pop();
		}
	}
	return 0;
}

int	Server::partWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, "PART");
	else {
		std::queue<std::string>	tmp_channels = splitToStrQueue(
			cmd.getParams()[0], ',', false);
		while (!tmp_channels.empty()) {
			if (!getChannelByName(tmp_channels.front()))
				sendErrorMsg(user, ERR_NOSUCHCHANNEL, tmp_channels.front());
			else if (!user.isOnChannel(tmp_channels.front()))
				sendErrorMsg(user, ERR_NOTONCHANNEL, tmp_channels.front());
			else {
				channels.at(tmp_channels.front())->sendMessage("PART " + tmp_channels.front() + "\n", user, true);
				channels.at(tmp_channels.front())->disconnectUser(user);
				user.quitChannel(tmp_channels.front());
			}
			tmp_channels.pop();
		}
	}
	return 0;
}

int		handleUserFlags(const Command &command, User &user) {
	std::string	flag = command.getParams()[1];
	bool result;
	switch (flag[0]) {
		case '+':
			result = true;
			break;
		case '-':
			result = false;
			break;
	}
	switch (flag[1]) {
		case 'i':
			user.setFlagInvisible(result);
			break;
		case 's':
			user.setFlagReceiveNotice(result);
			break;
		case 'w':
			user.setFlagReceiveWallops(result);
			break;
		case 'o':
			user.setFlagIrcOperator(result);
			break;
		default:
			return sendErrorMsg(user, ERR_UMODEUNKNOWNFLAG);
	}
	return 0;
}

int		Server::modeWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, "MODE");
	else {
		if (cmd.getParams()[0][0] == '#') {
			if (!getChannelByName(cmd.getParams()[0]))
				sendErrorMsg(user, ERR_NOSUCHCHANNEL, cmd.getParams()[0]);
			else if (!channels.at(cmd.getParams()[0])->isUserOperator(user))
				sendErrorMsg(user, ERR_CHANOPRIVSNEEDED, cmd.getParams()[0]);
			else if (!channels.at(cmd.getParams()[0])->isInChannel(user.getNickname()))
				sendErrorMsg(user, ERR_NOTONCHANNEL, cmd.getParams()[0]);
			else if (cmd.getParams().size() == 1)
				sendReplyMsg(user.getServername(), user, RPL_CHANNELMODEIS,
					cmd.getParams()[0], channels.at(cmd.getParams()[0])->getFlagsForUsers());
			else if (handleChannelFlags(cmd, user) != -1) {
				std::string	flag = cmd.getParams()[1];
				std::string	tmp;
				if (flag[1] == 'o' || flag[1] == 'v')
					tmp = " " + cmd.getParams()[2];
				else
					tmp = "";
				channels.at(cmd.getParams()[0])->sendMessage("MODE " + cmd.getParams()[0]
				+ " " + cmd.getParams()[1] + tmp + "\n", user, true);
			}
		}
		else {
			if (cmd.getParams()[0] != user.getNickname())
				sendErrorMsg(user, ERR_USERSDONTMATCH);
			else {
				if (cmd.getParams().size() == 1) {
					std::string	flags = "+";
					if (user.getFlagInvisible())
						flags += "i";
					if (user.getFlagReceiveNotice())
						flags += "s";
					if (user.getFlagReceiveWallops())
						flags += "w";
					if (user.getFlagIrcOperator())
						flags += "o";
					sendReplyMsg(user.getServername(), user, RPL_UMODEIS, flags);
				}
				else if (handleUserFlags(cmd, user) != -1)
					user.sendCommand(":" + user.getPrefix() + " MODE " +
						cmd.getParams()[0] + " " + cmd.getParams()[1] + "\n");
			}
		}
	}
	return 0;
}

int		Server::handleChannelFlags(const Command &cmd, User &user)
{
	std::string	name = cmd.getParams()[0];
	std::string	flag = cmd.getParams()[1];
	if (flag == "+p")
		channels[name]->setFlagPrivate(true);
	else if (flag == "-p")
		channels[name]->setFlagPrivate(false);
	else if (flag == "+s")
		channels[name]->setFlagSecret(true);
	else if (flag == "-s")
		channels[name]->setFlagSecret(false);
	else if (flag == "+i")
		channels[name]->setFlagInviteOnly(true);
	else if (flag == "-i")
		channels[name]->setFlagInviteOnly(false);
	else if (flag == "+t")
		channels[name]->setFlagTopicSet(true);
	else if (flag == "-t")
		channels[name]->setFlagTopicSet(false);
	else if (flag == "+m")
		channels[name]->setFlagModerated(true);
	else if (flag == "-m")
		channels[name]->setFlagModerated(false);
	else if (cmd.getParams().size() < 3)
		return sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand());
	else if (flag == "+o") {
		if (getUserByName(cmd.getParams()[2]))
			channels[name]->addToOperators(*(getUserByName(cmd.getParams()[2])));
		else
			return sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[2]);
	}
	else if (flag == "-o") {
		if (getUserByName(cmd.getParams()[2]))
			channels[name]->removeFromOperators(*(getUserByName(cmd.getParams()[2])));
		else
			return sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[2]);
	}
	else if (flag == "+l") {
		try {
			long tmp = string_to_long(cmd.getParams()[2]);
			if (tmp < 1 || tmp > std::numeric_limits<int>::max())
				throw std::out_of_range("Param out of range");
			channels[name]->setUserLimit(tmp);
		} catch (const std::exception& e) {
			log("Can't find correct \"maxUsersLimit in Channel\" in "
				+ name);
		}
	}
	else if (flag == "-l")
		channels[name]->setUserLimit(0);
	else if (flag == "+b")
		channels[name]->addToBanMaskList(cmd.getParams()[2]);
	else if (flag == "-b")
		channels[name]->removeFromBanMaskList(cmd.getParams()[2]);
	else if (flag == "+v") {
		if (getUserByName(cmd.getParams()[2]))
			channels[name]->addToVotersList(*(getUserByName(cmd.getParams()[2])));
		else
			return sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[2]);
	}
	else if (flag == "-v") {
		if (getUserByName(cmd.getParams()[2]))
			channels[name]->removeFromVotersList(*(getUserByName(cmd.getParams()[2])));
		else
			return sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[2]);
	}
	else if (flag == "+k")
		channels[name]->setPassword(user, cmd.getParams()[2]);
	else if (flag == "-k")
		channels[name]->setPassword(user, "");
	else
		return sendErrorMsg(user, ERR_UNKNOWNMODE, flag);
	return 0;
}

int		Server::topicWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, "TOPIC");
	else if (getChannelByName(cmd.getParams()[0])) {
		Channel	*channel = channels.at(cmd.getParams()[0]);
		if (!channel->isInChannel(user.getNickname()))
			sendErrorMsg(user, ERR_NOTONCHANNEL, cmd.getParams()[0]);
		else if (cmd.getParams().size() < 2)
			channel->ReplyWithTopic(user);
		else
			channel->setTopic(user, cmd.getParams()[1]);
	}
	else
		sendErrorMsg(user, ERR_NOTONCHANNEL, cmd.getParams()[0]);
	return 0;
}

int		Server::namesWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty()) {
		std::vector<std::string>	usersWithoutChannel;
		std::map<std::string, Channel *>::const_iterator	start = channels.begin();
		std::map<std::string, Channel *>::const_iterator	finish = channels.end();
		std::string	names;
		for (size_t i = 0; i < users.size(); i++)
			usersWithoutChannel.push_back(users[i]->getNickname());
		while (start != finish) {
			if (!((*start).second->getFlagSecret()) && !((*start).second->getFlagPrivate())) {
				(*start).second->ReplyWithUsersList(user);
				for (size_t i = 0; i < usersWithoutChannel.size(); i++) {
					if ((*start).second->isInChannel(usersWithoutChannel[i])) {
						usersWithoutChannel.erase(usersWithoutChannel.begin() + i--);
					}
				}
			}
			start++;
		}
		for (size_t i = 0; i < usersWithoutChannel.size(); i++) {
			names += usersWithoutChannel[i];
			if (i != (usersWithoutChannel.size() - 1)) {
				names += " ";
			}
		}
		sendReplyMsg(user.getServername(), user, RPL_NAMREPLY, "* *", names);
		sendReplyMsg(user.getServername(), user, RPL_ENDOFNAMES, "*");
	}
	else {
		std::queue<std::string>	channelsToDisplay = splitToStrQueue(
			cmd.getParams()[0], ',', false);
		while (!channelsToDisplay.empty()) {
			try {
				Channel	*tmp = channels.at(channelsToDisplay.front());
				if (!(tmp->getFlagSecret()) && !(tmp->getFlagPrivate())) {
					tmp->ReplyWithUsersList(user);
					sendReplyMsg(user.getServername(), user, RPL_ENDOFNAMES, tmp->getName());
				}
			}
			catch(const std::exception& e) {
				log("Can't find correct \"channels to display\" or \"users to display\"");
			}
			channelsToDisplay.pop();
		}
	}
	return 0;
}

int		Server::listWorker(const Command &cmd, User &user) {
	if (cmd.getParams().size() > 1 && cmd.getParams()[1] != user.getServername())
		return (sendErrorMsg(user, ERR_NOSUCHSERVER, cmd.getParams()[1]));
	std::queue<std::string>		tmpChannels;
	std::vector<std::string>	channelsToDisplay;
	if (!cmd.getParams().empty()) {
		tmpChannels = splitToStrQueue(cmd.getParams()[0], ',', false);
		while (!tmpChannels.empty()) {
			if (getChannelByName(tmpChannels.front()))
				channelsToDisplay.push_back(tmpChannels.front());
			tmpChannels.pop();
		}
	}
	else {
		std::map<std::string, Channel *>::const_iterator	start = channels.begin();
		std::map<std::string, Channel *>::const_iterator	finish = channels.end();
		while (start != finish) {
			channelsToDisplay.push_back((*start).first);
			start++;
		}
	}
	sendReplyMsg(user.getServername(), user, RPL_LISTSTART);
	for (size_t i = 0; i < channelsToDisplay.size(); i++)
		channels.at(channelsToDisplay[i])->ReplyWithInfo(user);
	sendReplyMsg(user.getServername(), user, RPL_LISTEND);
	return 0;
}

int		Server::inviteWorker(const Command &cmd, User &user)
{
	if (cmd.getParams().size() < 2)
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, "INVITE");
	else if (!getUserByName(cmd.getParams()[0]))
		sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[0]);
	else if (!user.isOnChannel(cmd.getParams()[1]) || !getChannelByName(cmd.getParams()[1]))
		sendErrorMsg(user, ERR_NOTONCHANNEL, cmd.getParams()[1]);
	else {
		User	*receiver;
		for (size_t i = 0; i < users.size(); i++)
			if (users[i]->getNickname() == cmd.getParams()[0])
				receiver = users[i];
		Channel	*tmpChannel = channels.at(cmd.getParams()[1]);
		if (tmpChannel->isInChannel(cmd.getParams()[0]))
			sendErrorMsg(user, ERR_USERONCHANNEL, cmd.getParams()[0], serverName);
		else
			tmpChannel->inviteUser(user, *receiver);
	}
	return 0;
}

int		Server::kickWorker(const Command &cmd, User &user)
{
	if (cmd.getParams().size() < 2)
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, "KICK");
	else if (!getChannelByName(cmd.getParams()[0]))
		sendErrorMsg(user, ERR_NOSUCHCHANNEL, cmd.getParams()[0]);
	else if (!channels.at(cmd.getParams()[0])->isUserOperator(user))
		sendErrorMsg(user, ERR_CHANOPRIVSNEEDED, cmd.getParams()[0]);
	else if (!getUserByName(cmd.getParams()[1]))
		sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[1]);
	else if (!channels.at(cmd.getParams()[0])->isInChannel(user.getNickname()))
		sendErrorMsg(user, ERR_NOTONCHANNEL, cmd.getParams()[0]);
	else if (!channels.at(cmd.getParams()[0])->isInChannel(cmd.getParams()[1]))
		sendErrorMsg(user, ERR_USERNOTINCHANNEL, cmd.getParams()[1], cmd.getParams()[0]);
	else {
		Channel	*tmpChannel = channels.at(cmd.getParams()[0]);
		std::string	parameters = "KICK " + tmpChannel->getName() + " " + cmd.getParams()[1] + " :";
		if (cmd.getParams().size() > 2)
			parameters += cmd.getParams()[2];
		else
			parameters += user.getNickname();
		tmpChannel->sendMessage(parameters + "\n", user, true);
		tmpChannel->disconnectUser(*(getUserByName(cmd.getParams()[1])));
		getUserByName(cmd.getParams()[1])->quitChannel(cmd.getParams()[0]);
	}
	return 0;
}
