#include "utils.hpp"
#include <sstream>
#include <ctime>
#include <iostream>
#include <algorithm>

bool	checkRegex(std::string strMask, std::string subString)
{
	const char *strPtr = subString.c_str();
	const char *maskPtr = strMask.c_str();
	const char *rpPtr, *rsPtr = NULL;

	while (true)
	{
		if (*maskPtr == '*')
		{
			rpPtr = ++maskPtr;
			rsPtr = strPtr;
		}
		else if (!*strPtr)
			return !(*maskPtr);
		else if (*strPtr == *maskPtr)
		{
			maskPtr++;
			strPtr++;
		}
		else if (rsPtr)
		{
			maskPtr = rpPtr;
			strPtr = ++rsPtr;
		}
		else
			return false;
	}
}

std::string	trim(std::string str)
{
	unsigned long start = 0, end = str.size();
	while (isspace(str[start]))
		start++;
	while (isspace(str[end - 1]))
		end--;
	return str.substr(start, end - start);
}

void	log(const std::string& str)
{
	std::time_t currentTime = std::time(0);
	std::cerr << std::ctime(&currentTime) << "           " << str << std::endl;
}

void	logCmdWithParams(const Command& cmd, const std::string& msg)
{
	std::time_t currentTime = std::time(0);
	std::cerr << std::ctime(&currentTime) << "           :["
				<< cmd.getPrefix() << "] [" << cmd.getCommand() << "] <";
	std::vector<std::string>	params = cmd.getParams();
	if (!params.empty())
	{
		size_t i = 0;
		for (; i < params.size() - 1; ++i)
			std::cerr << "[" << params[i] << "] ";
		std::cerr << "[" << params[i] << "]";
	}
	std::cerr << ">" << std::endl
			<< "           " << msg << std::endl;
}

long	string_to_long(const std::string& str)
{
	std::stringstream sStream;
	long number = 0;

	sStream << str;
	sStream >> number;

	return number;
}

std::string long_to_string(long number)
{
	std::stringstream sStream;
	std::string tmpStr;

	sStream << number;
	sStream >> tmpStr;

	return tmpStr;
}

bool	checkChannelname(const std::string &channelName)
{
	if (channelName.length() > 200)
		return false;
	if ((channelName[0] != '#') && (channelName[0] != '&'))
		return false;
	for (size_t i = 1; i < channelName.size(); i++)
	{
		if ((channelName[i] == ' ') || (channelName[i] == 7)
			|| (channelName[i] == 0) || (channelName[i] == 13)
			|| (channelName[i] == 10) || (channelName[i] == ','))
			return false;
	}
	return true;
}

bool	checkNickname(const std::string &nickname) {
	if (nickname.length() > 9)
		return false;
	for (size_t i = 0; i < nickname.size(); i++)
	{
		if ((nickname[i] >= 'a' && nickname[i] <= 'z') || (nickname[i] >= 'A'
			&& nickname[i] <= 'Z') || (nickname[i] >= '0' && nickname[i] <= '9')
			|| (nickname[i] == '-') || (nickname[i] == '[')
			|| (nickname[i] == ']')  || (nickname[i] == '\\')
			|| (nickname[i] == '^')  || (nickname[i] == '{')
			|| (nickname[i] == '}'))
			continue;
		else
			return false;
	}
	return true;
}

int		sendErrorMsg(const User &user, int err, const std::string &arg1, const std::string &arg2)
{
	std::string	msg = ":" + user.getServername() + " ";
	std::stringstream	ss;
	ss << err;
	msg += ss.str() + " " + user.getNickname();
	switch (err)
	{
		case ERR_NOSUCHNICK:
			msg += " " + arg1 + " :No such nick/channel\n";
			break;
		case ERR_NOSUCHSERVER:
			msg += " " + arg1 + " :No such server\n";
			break;
		case ERR_NOSUCHCHANNEL:
			msg += " " + arg1 + " :No such channel\n";
			break;
		case ERR_CANNOTSENDTOCHAN:
			msg += " " + arg1 + " :Cannot send to channel\n";
			break;
		case ERR_TOOMANYCHANNELS:
			msg += " " + arg1 + " :You have joined too many channels\n";
			break;
		case ERR_WASNOSUCHNICK:
			msg += " " + arg1 + " :There was no such nickname\n";
			break;
		case ERR_TOOMANYTARGETS:
			msg += " " + arg1 + " :Duplicate recipients. No arg1 delivered\n";
			break;
		case ERR_NOORIGIN:
			msg += " :No origin specified\n";
			break;
		case ERR_NORECIPIENT:
			msg += " :No recipient given (" + arg1 + ")\n";
			break;
		case ERR_NOTEXTTOSEND:
			msg += " :No text to send\n";
			break;
		case ERR_NOTOPLEVEL:
			msg += " " + arg1 + " :No toplevel domain specified\n";
			break;
		case ERR_WILDTOPLEVEL:
			msg += " " + arg1 + " :Wildcard in toplevel domain\n";
			break;
		case ERR_UNKNOWNCOMMAND:
			msg += " " + arg1 + " :Unknown command\n";
			break;
		case ERR_NOMOTD:
			msg += " :MOTD File is missing\n";
			break;
		case ERR_NOADMININFO:
			msg += " " + arg1 + " :No administrative info available\n";
			break;
		case ERR_FILEERROR:
			msg += " :File error doing \n" + arg1 + " on " + arg2 + "\n";
			break;
		case ERR_NONICKNAMEGIVEN:
			msg += " :No nickname given\n";
			break;
		case ERR_ERRONEUSNICKNAME:
			msg += " " + arg1 + " :Erroneus nickname\n";
			break;
		case ERR_NICKNAMEINUSE:
			msg += " " + arg1 + " :Nickname is already in use\n";
			break;
		case ERR_NICKCOLLISION:
			msg += " " + arg1 + " :Nickname collision KILL\n";
			break;
		case ERR_USERNOTINCHANNEL:
			msg += " " + arg1 + " " + arg2 + " :They aren't on that channel\n";
			break;
		case ERR_NOTONCHANNEL:
			msg += " " + arg1 + " :You're not on that channel\n";
			break;
		case ERR_USERONCHANNEL:
			msg += " " + arg1 + " " + arg2 + " :is already on channel\n";
			break;
		case ERR_NOLOGIN:
			msg += " " + arg1 + " :User not logged in\n";
			break;
		case ERR_SUMMONDISABLED:
			msg += " :SUMMON has been disabled\n";
			break;
		case ERR_USERSDISABLED:
			msg += " :USERS has been disabled\n";
			break;
		case ERR_NOTREGISTERED:
			msg += " :You have not registered\n";
			break;
		case ERR_NEEDMOREPARAMS:
			msg += " " + arg1 + " :Not enough params\n";
			break;
		case ERR_ALREADYREGISTRED:
			msg += " :You may not reregister\n";
			break;
		case ERR_NOPERMFORHOST:
			msg += " :Your host isn't among the privileged\n";
			break;
		case ERR_PASSWDMISMATCH:
			msg += " :Password incorrect\n";
			break;
		case ERR_YOUREBANNEDCREEP:
			msg += " :You are banned from this server\n";
			break;
		case ERR_KEYSET:
			msg += " " + arg1 + " :Channel key already set\n";
			break;
		case ERR_CHANNELISFULL:
			msg += " " + arg1 + " :Cannot join channel (+l)\n";
			break;
		case ERR_UNKNOWNMODE:
			msg += " " + arg1 + " :is unknown mode char to me\n";
			break;
		case ERR_INVITEONLYCHAN:
			msg += " " + arg1 + " :Cannot join channel (+i)\n";
			break;
		case ERR_BANNEDFROMCHAN:
			msg += " " + arg1 + " :Cannot join channel (+b)\n";
			break;
		case ERR_BADCHANNELKEY:
			msg += " " + arg1 + " :Cannot join channel (+k)\n";
			break;
		case ERR_NOPRIVILEGES:
			msg += " :Permission Denied- You're not an IRC operator\n";
			break;
		case ERR_CHANOPRIVSNEEDED:
			msg += " " + arg1 + " :You're not channel operator\n";
			break;
		case ERR_CANTKILLSERVER:
			msg += " :You cant kill a server!\n";
			break;
		case ERR_NOOPERHOST:
			msg += " :No O-lines for your host\n";
			break;
		case ERR_UMODEUNKNOWNFLAG:
			msg += " :Unknown MODE flag\n";
			break;
		case ERR_USERSDONTMATCH:
			msg += " :Cant change mode for other users\n";
			break;
		default:
			msg += "UNKNOWN ERROR\n";
			break;
	}
	send(user.getFd(), msg.c_str(), msg.size(), SRV_NOSIGNAL);
	return (-1);
}

int		sendReplyMsg(const std::string &from, const User &user, int rpl, \
					const std::string &arg1, const std::string &arg2, \
					const std::string &arg3, const std::string &arg4, \
					const std::string &arg5, const std::string &arg6, \
					const std::string &arg7, const std::string &arg8)
{
	std::string	msg = ":" + from + " ";
	std::stringstream	ss;
	ss << rpl;
	msg += ss.str() + " " + user.getNickname() + " ";
	switch (rpl)
	{
		case RPL_USERHOST:
			msg += ":" + arg1 + "\n";
			break;
		case RPL_ISON:
			msg += ":" + arg1 + "\n";
			break;
		case RPL_AWAY:
			msg += arg1 + " :" + arg2 + "\n";
			break;
		case RPL_UNAWAY:
			msg += ":You are no longer marked as being away\n";
			break;
		case RPL_NOWAWAY:
			msg += ":You have been marked as being away\n";
			break;
		case RPL_WHOISUSER:
			msg += arg1 + " " + arg2 + " " + arg3 + " * :" + arg4 + "\n";
			break;
		case RPL_WHOISSERVER:
			msg += arg1 + " " + arg2 + " :" + arg3 + "\n";
			break;
		case RPL_WHOISOPERATOR:
			msg += arg1 + " :is an IRC operator\n";
			break;
		case RPL_WHOISIDLE:
			msg += arg1 + " " + arg2 + " " + arg3 + " :seconds idle\n";
			break;
		case RPL_ENDOFWHOIS:
			msg += arg1 + " :End of /WHOIS list\n";
			break;
		case RPL_WHOISCHANNELS:
			msg += arg1 + " :" + arg2 + "\n";
			break;
		case RPL_WHOWASUSER:
			msg += arg1 + " " + arg2 + " " + arg3 + " * :" + arg4 + "\n";
			break;
		case RPL_ENDOFWHOWAS:
			msg += arg1 + " :End of WHOWAS\n";
			break;
		case RPL_LISTSTART:
			msg += "Channel :Users  Name\n";
			break;
		case RPL_LIST:
			msg += arg1 +  " " + arg2 + " :" + arg3 + "\n";
			break;
		case RPL_LISTEND:
			msg += ":End of /LIST\n";
			break;
		case RPL_CHANNELMODEIS:
			msg += arg1 + " +" + arg2 + "\n";
			break;
		case RPL_NOTOPIC:
			msg += arg1 + " :No topic is set\n";
			break;
		case RPL_TOPIC:
			msg += arg1 + " :" + arg2 + "\n";
			break;
		case RPL_INVITING:
			msg += arg1 + " " + arg2 + "\n";
			break;
		case RPL_SUMMONING:
			msg += arg1 + " :Summoning user to IRC\n";
			break;
		case RPL_VERSION:
			msg += arg1 + "." + arg2 + " " + arg3 + " :" + arg4 + "\n";
			break;
		case RPL_WHOREPLY:
			msg += arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " ";
			msg += arg5 + " " + arg6 + " :" + arg7 + " " + arg8 + "\n";
			break;
		case RPL_ENDOFWHO:
			msg += arg1 + " :End of /WHO list\n";
			break;
		case RPL_NAMREPLY:
			msg += arg1 + " :" + arg2 + "\n";
			break;
		case RPL_ENDOFNAMES:
			msg += arg1 + " :End of /NAMES list\n";
			break;
		case RPL_LINKS:
			msg += arg1 + " " + arg2 + ": " + arg3 + " " + arg4 + "\n";
			break;
		case RPL_ENDOFLINKS:
			msg += arg1 + " :End of /LINKS list\n";
			break;
		case RPL_BANLIST:
			msg += arg1 + " " + arg2 + "\n";
			break;
		case RPL_ENDOFBANLIST:
			msg += arg1 + " :End of channel ban list\n";
			break;
		case RPL_INFO:
			msg += ":" + arg1 + "\n";
			break;
		case RPL_ENDOFINFO:
			msg += ":End of /INFO list\n";
			break;
		case RPL_MOTDSTART:
			msg += ":- " + arg1 + " Message of the day - \n";
			break;
		case RPL_MOTD:
			msg += ":- " + arg1 + "\n";
			break;
		case RPL_ENDOFMOTD:
			msg += ":End of /MOTD command\n";
			break;
		case RPL_YOUREOPER:
			msg += ":You are now an IRC operator\n";
			break;
		case RPL_REHASHING:
			msg += arg1 + " :Rehashing\n";
			break;
		case RPL_TIME:
			msg += arg1 + " :" + arg2; // ctime return string following '\n'
			break;
		case RPL_USERSSTART:
			msg += ":UserID   Terminal  Host\n";
			break;
		case RPL_USERS:
			msg += ":%-8s %-9s %-8s\n"; // Хз, что это UPD: Понял, строка длиной 8 символов
			break;
		case RPL_ENDOFUSERS:
			msg += ":End of users\n";
			break;
		case RPL_NOUSERS:
			msg += ":Nobody logged in\n";
			break;
		case RPL_STATSLINKINFO:
			msg += arg1 + " " + arg2 + " " + arg3 + " " + arg4 + " ";
			msg += arg5 + " " + arg6 + " " + arg7 + "\n";
			break;
		case RPL_STATSCOMMANDS:
			msg += arg1 + " " + arg2 + "\n";
			break;
		case RPL_STATSCLINE:
			msg += "C " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n";
			break;
		case RPL_STATSNLINE:
			msg += "N " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n";
			break;
		case RPL_STATSILINE:
			msg += "I " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n";
			break;
		case RPL_STATSKLINE:
			msg += "K " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n";
			break;
		case RPL_STATSYLINE:
			msg += "Y " + arg1 + " * " + arg2 + " " + arg3 + " " + arg4 + "\n";
			break;
		case RPL_ENDOFSTATS:
			msg += arg1 + " :End of /STATS report\n";
			break;
		case RPL_STATSLLINE:
			msg += "L " + arg1 + " * " + arg2 + " " + arg3 + "\n";
			break;
		case RPL_STATSUPTIME:
			msg += ":Server Up %d days %d:%02d:%02d\n";
			break;
		case RPL_STATSOLINE:
			msg += "O " + arg1 + " * " + arg2 + "\n";
			break;
		case RPL_STATSHLINE:
			msg += "H " + arg1 + " * " + arg2 + "\n";
			break;
		case RPL_UMODEIS:
			msg += arg1 + "\n";
			break;
		case RPL_LUSERCLIENT:
			msg += ":There are " + arg1 + " users and " + arg2;
			msg += " invisible on " + arg3 + " servers\n";
			break;
		case RPL_LUSEROP:
			msg += arg1 + " :operator(s) online\n";
			break;
		case RPL_LUSERUNKNOWN:
			msg += arg1 + " :unknown connection(s)\n";
			break;
		case RPL_LUSERCHANNELS:
			msg += arg1 + " :channels formed\n";
			break;
		case RPL_LUSERME:
			msg += ":I have " + arg1 + " clients and " + arg2 + " servers\n";
			break;
		case RPL_ADMINME:
			msg += arg1 + " :Administrative info\n";
			break;
		case RPL_ADMINLOC1:
			msg += ":Name     " + arg1 + "\n";
			break;
		case RPL_ADMINLOC2:
			msg += ":Nickname " + arg1 + "\n";
			break;
		case RPL_ADMINEMAIL:
			msg += ":E-Mail   " + arg1 + "\n";
			break;
		default:
			msg += "UNKNOWN REPLY\n";
			break;
	}
	send(user.getFd(), msg.c_str(), msg.size(), SRV_NOSIGNAL);
	return 0;
}

std::queue<std::string>	splitToStrQueue(const std::string &str, char separator, bool includeFlag)
{
	std::queue<std::string>	resQueue;
	std::string::const_iterator	it = str.begin();

	while(it != str.end())
	{
		while (it != str.end() && *it == separator)
			++it;
		std::string::const_iterator	subIt = std::find(it, str.end(), separator);
		if (it != str.end())
		{
			if (includeFlag && subIt != str.end())
				resQueue.push(std::string(it, subIt + 1));
			else
				resQueue.push(std::string(it, subIt));
			it = subIt;
		}
	}
	return resQueue;
}
