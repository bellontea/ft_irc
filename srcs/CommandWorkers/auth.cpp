#include "Server.hpp"
#include "Sha256.hpp"
#include "utils.hpp"

int		Server::userWorker(const Command &cmd, User &user)
{
	if (cmd.getParams().size() < 4)
	{
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand());
		logCmdWithParams(cmd, "USER worker send error ERR_NEEDMOREPARAMS");
	}
	else if (user.getFlagRegistered())
	{
		sendErrorMsg(user, ERR_ALREADYREGISTRED);
		logCmdWithParams(cmd, "USER worker send error to " + user.getNickname() + " ERR_ALREADYREGISTRED");
	}
	else
	{
		user.setRealname(cmd.getParams()[3]);
		user.setUsername(cmd.getParams()[0]);
		logCmdWithParams(cmd, "USER command succeeded");
	}
	return (checkNickNamePass(user));
}

int		Server::nickWorker(const Command &cmd, User &user)
{
	if (cmd.getParams().empty())
	{
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand());
		logCmdWithParams(cmd, "NICK worker send error ERR_NEEDMOREPARAMS");
	}
	else if (cmd.getParams()[0] == serverName || !checkNickname(cmd.getParams()[0]))
	{
		sendErrorMsg(user, ERR_ERRONEUSNICKNAME, cmd.getParams()[0]);
		logCmdWithParams(cmd, "NICK worker send error ERR_ERRONEUSNICKNAME");
	}
	else if (getUserByName(cmd.getParams()[0]))
	{
		sendErrorMsg(user, ERR_NICKNAMEINUSE, cmd.getParams()[0]);
		logCmdWithParams(cmd, "NICK worker send error ERR_NICKNAMEINUSE");
	}
	else
	{
		if (user.getFlagRegistered())
			sendBroadcastNotification(user, ":" + user.getPrefix() + " " + cmd.getCommand() + " " + cmd.getParams()[0] + "\n");
		user.setNickname(cmd.getParams()[0]);
		logCmdWithParams(cmd, "NICK command succeeded");
	}
	return (checkNickNamePass(user));
}

int		Server::passWorker(const Command &cmd, User &user)
{
	if (cmd.getParams().empty())
	{
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand());
		logCmdWithParams(cmd, "PASS worker send error ERR_NEEDMOREPARAMS");
	}
	else if (user.getFlagRegistered())
	{
		sendErrorMsg(user, ERR_ALREADYREGISTRED);
		logCmdWithParams(cmd, "PASS worker send error to " + user.getNickname() + " ERR_ALREADYREGISTRED");
	}
	else
	{
		user.setPassword(cmd.getParams()[0]);
		logCmdWithParams(cmd, "PASS command succeeded");
	}
	return 0;
}

int		Server::operWorker(const Command &cmd, User &user)
{
	if (cmd.getParams().size() < 2)
	{
		sendErrorMsg(user, ERR_NEEDMOREPARAMS, cmd.getCommand());
		logCmdWithParams(cmd, "OPER worker send error ERR_NEEDMOREPARAMS");
	}
	else if (operatorsList.empty())
	{
		sendErrorMsg(user, ERR_NOOPERHOST);
		logCmdWithParams(cmd, "OPER worker send error ERR_NOOPERHOST");
	}
	else
	{
		try
		{
			std::string	pass = operatorsList.at(cmd.getParams()[0]);
			if (sha256(cmd.getParams()[1]) == pass)
			{
				user.setFlagIrcOperator(true);
				logCmdWithParams(cmd, "OPER command succeeded");
				return sendReplyMsg(user.getServername(), user, RPL_YOUREOPER);
			}
		}
		catch(const std::exception& e)
		{}
		sendErrorMsg(user, ERR_PASSWDMISMATCH);
		logCmdWithParams(cmd, "OPER worker send error ERR_PASSWDMISMATCH");
	}
	return (0);
}

int		Server::quitWorker(const Command &cmd, User &user)
{
	if (!cmd.getParams().empty())
		user.setQuitMessage(cmd.getParams()[0]);
	logCmdWithParams(cmd, "QUIT command succeeded for nickname " + user.getNickname());
	return (DISCONNECT);
}

int		Server::checkNickNamePass(User &user)
{
	if (!user.getNickname().empty() && !user.getUsername().empty())
	{
		if ((user.getPassword() == password || password.empty()))
		{
			if (!user.getFlagRegistered())
			{
				user.setFlagRegistered(true);
				if (motdStorage.empty())
					sendErrorMsg(user, ERR_NOMOTD);
				else
				{
					sendReplyMsg(serverName, user, RPL_MOTDSTART, serverName);
					for (size_t i = 0; i < motdStorage.size(); ++i)
						sendReplyMsg(serverName, user, RPL_MOTD, motdStorage[i]);
					sendReplyMsg(serverName, user, RPL_ENDOFMOTD);
				}
				log("USER REGISTERED, nickname=" + user.getNickname());
			}
		}
		else
			return (DISCONNECT);
	}
	return (0);
}
