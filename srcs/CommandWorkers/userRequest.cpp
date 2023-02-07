#include "Server.hpp"
#include "Channel.hpp"
#include <set>
#include <sstream>

int Server::privmsgWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		return sendErrorMsg(user, ERR_NORECIPIENT, cmd.getCommand());
	if (cmd.getParams().size() == 1)
		return sendErrorMsg(user, ERR_NOTEXTTOSEND);

	std::queue<std::string> receiversStrQ = splitToStrQueue(cmd.getParams()[0],
		',', false);
	std::set<std::string> uniqueReceiversStrSet;

	if (cmd.getCommand() == "NOTICE"
			&& (receiversStrQ.size() > 1 || receiversStrQ.front()[0] == '#' || receiversStrQ.front()[0] == '&'))
		return sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[0]);

	while (!receiversStrQ.empty())
	{
		if (uniqueReceiversStrSet.find(receiversStrQ.front()) != uniqueReceiversStrSet.end())
			return sendErrorMsg(user, ERR_TOOMANYTARGETS, receiversStrQ.front());
		if (receiversStrQ.front()[0] == '#' || receiversStrQ.front()[0] == '&')
		{
			if (!this->getChannelByName(receiversStrQ.front()))
				return sendErrorMsg(user, ERR_NOSUCHNICK, receiversStrQ.front());
			if (!this->channels[receiversStrQ.front()]->isInChannel(user.getNickname()))
				return sendErrorMsg(user, ERR_CANNOTSENDTOCHAN, receiversStrQ.front());
		}
		else if (!this->getUserByName(receiversStrQ.front()))
			return sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[0]);
		uniqueReceiversStrSet.insert(receiversStrQ.front());
		receiversStrQ.pop();
	}

	for (std::set<std::string>::iterator receiversIter = uniqueReceiversStrSet.begin();
			receiversIter != uniqueReceiversStrSet.end(); ++receiversIter)
	{
		if ((*receiversIter)[0] == '#' || (*receiversIter)[0] == '&')
		{
			Channel *receiverChannel = this->channels[*receiversIter];
			if (receiverChannel->getFlagModerated()
					&& (!receiverChannel->isUserOperator(user)
					&& !receiverChannel->isVoter(user)))
				sendErrorMsg(user, ERR_CANNOTSENDTOCHAN, *receiversIter);
			else
				receiverChannel->sendMessage(
					cmd.getCommand() + " " + *receiversIter + " :" + cmd.getParams()[1] + "\n",
					user, false
					);
		}
		else
		{
			if (cmd.getCommand() == "PRIVMSG" && (this->getUserByName(*receiversIter)->getFlagAway()))
				sendReplyMsg(user.getServername(), user, RPL_AWAY, *receiversIter,
					this->getUserByName(*receiversIter)->getAwayMessage()
				);
			if (cmd.getCommand() != "NOTICE" || (this->getUserByName(*receiversIter)->getFlagReceiveNotice()))
				this->getUserByName(*receiversIter)->sendCommand(
					":" + user.getPrefix() + " " + cmd.getCommand() + " " + *receiversIter + " :" + cmd.getParams()[1] + "\n"
					);
		}
	}
	return 0;
}

int Server::awayWorker(const Command &cmd, User &user) {
	if (!cmd.getParams().empty()) {
		user.setFlagAway(true);
		user.setAwayMessage(cmd.getParams()[0]);
		sendReplyMsg(user.getServername(), user, RPL_NOWAWAY);
	} else {
		user.setFlagAway(false);
		sendReplyMsg(user.getServername(), user, RPL_UNAWAY);
	}
	return 0;
}

int Server::noticeWorker(const Command &cmd, User &user) {
	privmsgWorker(cmd, user);
	return 0;
}

int Server::whoWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		return sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand());

	for (size_t i = 0; i < users.size(); ++i) {
		if (checkRegex(cmd.getParams()[0], users[i]->getNickname()) && !(users[i]->getFlagInvisible())) {
			std::string userState = "";
			std::string channelName = "*";
			const std::vector<const Channel *> channelsList = users[i]->getUserChannels();

			for (int j = channelsList.size() - 1; j >= 0; --j) {
				if ( ( !channelsList[j]->getFlagSecret() && !channelsList[j]->getFlagPrivate() )
						|| channelsList[j]->isInChannel(user.getNickname()) ) {
					channelName = channelsList[j]->getName();
					if (channelsList[j]->isUserOperator(*(users[i])))
						userState = "@";
					else if (channelsList[j]->isVoter(*(users[i])))
						userState = "+";
					break;
				}
			}

			if (cmd.getParams().size() == 1 || cmd.getParams()[1] != "o"
					|| (cmd.getParams()[1] == "o" && users[i]->getFlagIrcOperator()) )
				sendReplyMsg(user.getServername(), user, RPL_WHOREPLY,
					channelName, users[i]->getUsername(),
					users[i]->getHostname(), users[i]->getServername(),
					users[i]->getNickname(), "H" + userState,
					"0", users[i]->getRealname());
		}
	}
	return sendReplyMsg(user.getServername(), user, RPL_ENDOFWHO,
		user.getNickname());
}

int Server::whoisWorker(const Command &cmd, User &user) {
	bool nickFound = false;
	if (cmd.getParams().empty())
		return (sendErrorMsg(user, ERR_NONICKNAMEGIVEN));

	for (size_t i = 0; i < users.size(); ++i) {
		if (checkRegex(cmd.getParams()[0], users[i]->getNickname()) && !users[i]->getFlagIrcOperator()) {
			sendReplyMsg(user.getServername(), user, RPL_WHOISUSER,
				users[i]->getNickname(), users[i]->getUsername(),
				users[i]->getHostname(), users[i]->getRealname());

			std::string channelsListForReply;
			const std::vector<const Channel *> userChannelsList = users[i]->getUserChannels();

			for (size_t j = 0; j < userChannelsList.size(); ++j) {
				if ( (!userChannelsList[j]->getFlagSecret() && !userChannelsList[j]->getFlagPrivate())
 						|| userChannelsList[j]->isInChannel(user.getNickname()) ) {
					if (j != 0)
						channelsListForReply += " ";
					if (userChannelsList[j]->isUserOperator(*(users[i])))
						channelsListForReply += "@";
					else if (userChannelsList[j]->isVoter(*(users[i])))
						channelsListForReply += "+";
					channelsListForReply += userChannelsList[j]->getName();
				}
			}
			sendReplyMsg(user.getServername(), user, RPL_WHOISCHANNELS,
				users[i]->getNickname(), channelsListForReply);
			sendReplyMsg(user.getServername(), user, RPL_WHOISSERVER,
				users[i]->getNickname(), users[i]->getServername(), serverInfo);
			if (users[i]->getFlagAway())
				sendReplyMsg(user.getServername(), user, RPL_AWAY,
					users[i]->getNickname(), users[i]->getAwayMessage());
			if (users[i]->getFlagIrcOperator())
				sendReplyMsg(user.getServername(), user, RPL_WHOISOPERATOR,
					users[i]->getNickname());

			std::stringstream userRegTime, userTimeOnServer;
			userRegTime << users[i]->getRegistrationTime();
			userTimeOnServer << (time(0) - users[i]->getRegistrationTime() );
			sendReplyMsg(user.getServername(), user, RPL_WHOISIDLE,
				users[i]->getNickname(), userTimeOnServer.str(),
				userRegTime.str());
			nickFound = true;
		}
	}
	if (!nickFound)
		sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[0]);
	return sendReplyMsg(user.getServername(), user, RPL_ENDOFWHOIS, cmd.getParams()[0]);
}

int Server::whowasWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		return (sendErrorMsg(user, ERR_NONICKNAMEGIVEN));

	else if (!this->getUserByName(cmd.getParams()[0]))
	{
		std::vector<const User*> userQuery;
		std::string nick = cmd.getParams()[0];

		for (size_t i = 0; i < usersDeprecated.size(); ++i)
			if (usersDeprecated[i]->getNickname() == nick)
				userQuery.push_back(usersDeprecated[i]);

		if (userQuery.empty())
			sendErrorMsg(user, ERR_WASNOSUCHNICK, nick);
		else {
			long showMax = 0;
			if (cmd.getParams().size() > 1)
			{
				try {
					showMax = string_to_long(cmd.getParams()[1]);
				} catch (const std::exception& e) {
					log("User " + user.getNickname() + " tried to overflow server in command WHOWAS");
					return sendReplyMsg(user.getServername(), user, RPL_ENDOFWHOWAS, cmd.getParams()[0]);
				}
			}
			if (showMax == 0)
				showMax = userQuery.size();

			for (long i = 0; i < showMax && i < static_cast<int>(userQuery.size()); ++i)
			{
				sendReplyMsg(user.getServername(), user, RPL_WHOWASUSER,
					userQuery[i]->getNickname(), userQuery[i]->getUsername(),
					userQuery[i]->getHostname(), userQuery[i]->getRealname());
				sendReplyMsg(user.getServername(), user, RPL_WHOISSERVER,
					userQuery[i]->getNickname(), userQuery[i]->getServername(),
					serverInfo);
			}
		}
	}
	return sendReplyMsg(user.getServername(), user, RPL_ENDOFWHOWAS,
		cmd.getParams()[0]);
}
