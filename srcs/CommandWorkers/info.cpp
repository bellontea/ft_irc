#include "Server.hpp"

int		Server::versionWorker(const Command &cmd, User &user) {
	if (!cmd.getParams().empty() && cmd.getParams()[0] != user.getServername())
		return (sendErrorMsg(user, ERR_NOSUCHSERVER, cmd.getParams()[0]));
	return (sendReplyMsg(user.getServername(), user, RPL_VERSION, serverVersion));
}

int		Server::timeWorker(const Command &msg, User &user) {
	if (!msg.getParams().empty() && msg.getParams()[0] != user.getServername())
		return (sendErrorMsg(user, ERR_NOSUCHSERVER, msg.getParams()[0]));
	time_t tmp = time(0);
	return (sendReplyMsg(user.getServername(), user, RPL_TIME,
		user.getServername(), ctime(&tmp)));
}

int		Server::adminWorker(const Command &cmd, User &user) {
	if (!cmd.getParams().empty() && cmd.getParams()[0] != user.getServername())
		return (sendErrorMsg(user, ERR_NOSUCHSERVER, cmd.getParams()[0]));
	sendReplyMsg(user.getServername(), user, RPL_ADMINME, user.getServername());
	sendReplyMsg(user.getServername(), user, RPL_ADMINLOC1, adminUserName);
	sendReplyMsg(user.getServername(), user, RPL_ADMINLOC2, adminNickname);
	sendReplyMsg(user.getServername(), user, RPL_ADMINEMAIL, adminMail);
	return 0;
}

int		Server::infoWorker(const Command &cmd, User &user) {
	if (!cmd.getParams().empty() && cmd.getParams()[0] != user.getServername())
		return (sendErrorMsg(user, ERR_NOSUCHSERVER, cmd.getParams()[0]));
	std::queue<std::string>	lines = splitToStrQueue(serverDescribe, '\n', false);
	while (!lines.empty()) {
		sendReplyMsg(user.getServername(), user, RPL_INFO, lines.front());
		lines.pop();
	}
	sendReplyMsg(user.getServername(), user, RPL_INFO, serverInfo);
	return (sendReplyMsg(user.getServername(), user, RPL_ENDOFINFO));
}

int		Server::pingWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		return (sendErrorMsg(user, ERR_NOORIGIN));
	user.sendCommand(":" + this->serverName + " PONG :" + cmd.getParams()[0] + "\n");
	return 0;
}

int		Server::pongWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty() || cmd.getParams()[0] != this->serverName) {
		if (!cmd.getParams().empty()){
			return (sendErrorMsg(user, ERR_NOSUCHSERVER, cmd.getParams()[0]));
		} else {
			return (sendErrorMsg(user, ERR_NOSUCHSERVER, ""));
		}
	}
	user.setFlagPinging(false);
	return 0;
}

int		Server::wallopsWorker(const Command &cmd, User &user) {
	if (!user.getFlagIrcOperator())
		return (sendErrorMsg(user, ERR_NOPRIVILEGES));
	if (cmd.getParams().empty())
		return (sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand()));
	const std::vector<User *> usersList = this->users;
	for (size_t i = 0; i < usersList.size(); i++) {
		if (usersList[i]->getFlagIrcOperator()) {
			usersList[i]->sendCommand(":" + user.getPrefix() + " " \
				 + cmd.getCommand() + " :" + cmd.getParams()[0] + "\n");
		}
	}
	return 0;
}

int		Server::userhostWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		return (sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand()));
	std::string	replyMessage;
	for (size_t i = 0; i < cmd.getParams().size() && i < 5; ++i) {
		if (this->getUserByName(cmd.getParams()[i])) {
			User *currentUser = this->getUserByName(cmd.getParams()[i]);
			if (!replyMessage.empty())
				replyMessage += " ";
			replyMessage += cmd.getParams()[i];
			if (currentUser->getFlagIrcOperator())
				replyMessage += "*";
			if (currentUser->getFlagAway())
				replyMessage += "=-@";
			else
				replyMessage += "=+@";
			replyMessage += currentUser->getHostname();
		}
	}
	return (sendReplyMsg(user.getServername(), user, RPL_USERHOST, replyMessage));
}

int		Server::isonWorker(const Command &cmd, User &user) {
	if (cmd.getParams().empty())
		return (sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand()));
	std::string	nicknamesList;
	for (size_t i = 0; i < cmd.getParams().size(); i++) {
		if (this->getUserByName(cmd.getParams()[i])) {
			if (!nicknamesList.empty())
				nicknamesList += " ";
			nicknamesList += cmd.getParams()[i];
		}
	}
	return (sendReplyMsg(user.getServername(), user, RPL_ISON, nicknamesList));
}
