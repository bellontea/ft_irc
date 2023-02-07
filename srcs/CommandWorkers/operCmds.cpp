#include "Server.hpp"
#include <unistd.h>

int	Server::killWorker(const Command &cmd, User &user)
{
	if (!user.getFlagIrcOperator())
		return sendErrorMsg(user, ERR_NOPRIVILEGES);

	if (cmd.getParams().size() < 2)
		return sendErrorMsg(user, ERR_NEEDMOREPARAMS);

	std::string userName = cmd.getParams()[0];

	if (userName == this->serverName)
		return sendErrorMsg(user, ERR_CANTKILLSERVER);

	User *userPtr = getUserByName(userName);
	if (!userPtr)
		return sendErrorMsg(user, ERR_NOSUCHNICK, cmd.getParams()[0]);

	userPtr->sendCommand(cmd.getParams()[1] + "\n");
	userPtr->setFlagBreakConnection(true);
	return 0;
}

int	Server::restartWorker(const Command &cmd, User &user)
{
	(void)cmd;
	if (!user.getFlagIrcOperator())
		return (sendErrorMsg(user, ERR_NOPRIVILEGES));

	for (size_t i = 0; i < users.size(); i++)
		users[i]->setFlagBreakConnection(true);

	close(fd);
	stopScheduled = true;
	setRebootScheduled(true);
	return 0;
}

int	Server::rehashWorker(const Command &cmd, User &user)
{
	(void)cmd;
	if (!user.getFlagIrcOperator())
		return sendErrorMsg(user, ERR_NOPRIVILEGES);

	getConfig(SRV_CONFIG_FILE);
	return sendReplyMsg(user.getServername(), user, RPL_REHASHING);
}
