#pragma once

#include <cstring>
#include <iostream>
#include <ctime>
#include <queue>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include "Server.hpp"
#include "utils.hpp"
#include "Channel.hpp"

class Channel;

class User {
	private:
		int 							fd;
		std::string						password;
		std::string						nickname;
		std::string						username;
		std::string 					hostname;
		std::string 					servername;
		std::string 					realname;
		std::string 					awayMessage;
		std::string 					quitMessage;
		time_t							registrationTime;
		time_t							timeOfLastCommand;
		time_t							timeAfterPing;
		std::queue<std::string>			commands;
		std::vector<const Channel *>	channels;
		bool							flagRegistered;
		bool 							flagInvisible;
		bool 							flagReceiveNotice;
		bool 							flagReceiveWallops;
		bool 							flagIrcOperator;
		bool 							flagAway;
		bool 							flagPinging;
		bool 							flagBreakConnection;

		User();
		User(const User& copy);
		User	&operator=(const User& other);
	public:
		User(int fd, std::string hostname, std::string servername);
		~User();
		/// Геттеры
		int 								getFd() const;
		const std::string					&getPassword() const;
		const std::string					&getNickname() const;
		const std::string					&getUsername() const;
		const std::string					&getHostname() const;
		const std::string					&getServername() const;
		const std::string					&getRealname() const;
		const std::string					&getAwayMessage() const;
		const std::string					&getQuitMessage() const;
		const time_t						&getRegistrationTime() const;
		const time_t						&getTimeOfLastCommand() const;
		const time_t						&getTimeAfterPing() const;
		const std::queue<std::string>		&getCommands() const;
		std::string							getPrefix() const;
		bool								getFlagRegistered() const;
		bool 								getFlagInvisible() const;
		bool 								getFlagReceiveNotice() const;
		bool 								getFlagReceiveWallops() const;
		bool 								getFlagIrcOperator() const;
		bool 								getFlagAway() const;
		bool 								getFlagPinging() const;
		bool 								getFlagBreakConnection() const;
		const std::vector<const Channel *>	&getUserChannels() const;
		/// Сеттеры
		void	setPassword(const std::string &password);
		void	setNickname(const std::string &nickname);
		void 	setUsername(const std::string &username);
		void 	setServername(const std::string &servername);
		void 	setRealname(const std::string &realname);
		void 	setAwayMessage(const std::string &awayMessage);
		void 	setQuitMessage(const std::string &quitMessage);
		void	setFlagRegistered(bool flagRegistered);
		void	setFlagInvisible(bool flagInvisible);
		void	setFlagReceiveNotice(bool flagReceiveNotice);
		void	setFlagReceiveWallops(bool flagReceiveWallops);
		void	setFlagIrcOperator(bool flagIrcOperator);
		void	setFlagAway(bool flagAway);
		void	setFlagPinging(bool flagPinging);
		void	setFlagBreakConnection(bool flagBreakConnection);
		/// методы
		void	updateTimeOfLastCommand();
		void	updateTimeAfterPing();
		void	sendCommand(const std::string &command) const;
		int		recvCommand();
		void	popCommand();
		bool	isOnChannel(const std::string &name) const;
		void	enterChannel(const Channel &channel);
		void	quitChannel(const std::string &name);
};