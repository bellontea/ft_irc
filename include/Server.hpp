#ifndef SERVER_HPP
# define SERVER_HPP

# include "User.hpp"
# include "Command.hpp"
# include "Channel.hpp"
# include "utils.hpp"

# include <map>
# include <string>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>

# ifdef __APPLE__
#  define SRV_NOSIGNAL SO_NOSIGPIPE
# else
#  define SRV_NOSIGNAL MSG_NOSIGNAL
# endif

# define EXIT_OK 0
# define EXIT_REBOOT 1
# define EXIT_ERROR_DEFAULT 2

# define SRV_CONFIG_FILE "ircserv.conf"
# define SRV_MOTD_FILE "pepe.txt"
# define DISCONNECT -2

class Server;
class User;
class Command;
class Channel;

typedef  int (Server::*Method) (const Command &, User &);

extern bool stopScheduled;

class Server
{
	int				port;
	int				fd;
	unsigned long	channelLimit;
	int				inactiveTimeoutLimit;
	int				responseTimeoutLimit;
	bool			rebootScheduled;
	int				pollTimeout;
	sockaddr_in		sockAddr;
	std::string		password;
	std::string		serverName;
	std::string		serverInfo;
	std::string		serverVersion;
	std::string		serverDescribe;
	std::string		adminUserName;
	std::string		adminNickname;
	std::string		adminMail;
	in_addr_t		bindIP;
	std::vector<std::string>			motdStorage;
	std::vector<User *>					users;
	std::vector<User *>					usersDeprecated;
	std::map<std::string, std::string>	operatorsList;
	std::vector<struct pollfd>			userConnectionFDs;
	std::map<std::string, Method>		commands;
	std::map<std::string, Channel *>	channels;

	void	applyConfigFromMap(const std::map<std::string, std::string>& cfgMap);

	int		checkNickNamePass(User &user);

public:
	Server(int port, const std::string &pass);
	~Server();

	void			getConfig(const std::string& path);
	void			initNewSocket();
	void			acceptConnection();
	void			checkNewCommands();
	int				processCommands(User &user);
	void			clearBrokenConnections();
	void			sendBroadcastNotification(User &initiatorUser, const std::string &msg);
	void			removeEmptyChannels();
	void			checkActiveUsers();

	int				isRebootScheduled();
	User*			getUserByName(const std::string &userName) const;
	Channel*		getChannelByName(const std::string &channelName) const;

	void			setRebootScheduled(bool rebootScheduled);

	int				userWorker(const Command &cmd, User &user);
	int				passWorker(const Command &cmd, User &user);
	int				nickWorker(const Command &cmd, User &user);
	int				operWorker(const Command &cmd, User &user);
	int				quitWorker(const Command &cmd, User &user);
	int				killWorker(const Command &cmd, User &user);
	int				restartWorker(const Command &cmd, User &user);
	int				rehashWorker(const Command &cmd, User &user);

	//user request
	int				privmsgWorker(const Command &cmd, User &user);
	int				awayWorker(const Command &cmd, User &user);
	int				whowasWorker(const Command &cmd, User &user);
	int				whoisWorker(const Command &cmd, User &user);
	int				whoWorker(const Command &cmd, User &user);
	int				noticeWorker(const Command &cmd, User &user);

	//channel request
	int				joinWorker(const Command &cmd, User &user);
	int				partWorker(const Command &cmd, User &user);
	int				modeWorker(const Command &cmd, User &user);
	int				handleChannelFlags(const Command &cmd, User &user);
	int				topicWorker(const Command &cmd, User &user);
	int				namesWorker(const Command &cmd, User &user);
	int				listWorker(const Command &cmd, User &user);
	int				inviteWorker(const Command &cmd, User &user);
	int				kickWorker(const Command &cmd, User &user);

	//info request
	int				versionWorker(const Command &cmd, User &user);
	int				timeWorker(const Command &msg, User &user);
	int				adminWorker(const Command &cmd, User &user);
	int				infoWorker(const Command &cmd, User &user);
	int				pingWorker(const Command &cmd, User &user);
	int				pongWorker(const Command &cmd, User &user);
	int				wallopsWorker(const Command &cmd, User &user);
	int				userhostWorker(const Command &cmd, User &user);
	int				isonWorker(const Command &cmd, User &user);
};

#endif
