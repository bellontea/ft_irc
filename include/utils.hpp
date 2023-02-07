#ifndef UTILS_HPP
# define UTILS_HPP

# include "answers.hpp"
# include "User.hpp"
# include "Command.hpp"

# include <string>
# include <queue>

class Command;
class User;

bool	checkRegex(std::string strMask, std::string subString);

bool	checkNickname(const std::string &nickname);

bool	checkChannelname(const std::string &channelName);

int		sendErrorMsg(const User &user, int err, const std::string &arg1 = "",
					 const std::string &arg2 = "");

std::queue<std::string>	splitToStrQueue(const std::string &str, char separator, bool includeFlag);

int		sendReplyMsg(const std::string &from, const User &user, int rpl, \
				const std::string &arg1 = "",const std::string &arg2 = "", \
				const std::string &arg3 = "",const std::string &arg4 = "", \
				const std::string &arg5 = "",const std::string &arg6 = "", \
				const std::string &arg7 = "",const std::string &arg8 = "");

std::string	trim(std::string str);

void		log(const std::string& str);
void		logCmdWithParams(const Command& cmd, const std::string& msg);

long		string_to_long(const std::string& str);

std::string	long_to_string(long number);
#endif
