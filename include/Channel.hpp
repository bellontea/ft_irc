#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "User.hpp"
# include "utils.hpp"

# include <string>
# include <vector>

class User;

class Channel {
	private:
		std::string					name;
		std::string					password;
		std::vector<const User *>	operatorsList;
		std::vector<const User *>	usersList;
		std::vector<const User *>	invitationsList;
		std::vector<std::string>	banMasksList;
		std::vector<const User *>	votersList;
		unsigned int				maxUsersLimit;
		std::string					topic;
		bool 						flagPrivate;
		bool 						flagSecret;
		bool 						flagModerated;
		bool 						flagInviteOnly;
		bool 						flagTopicSet;

		Channel();
		Channel(const Channel &copy);
		Channel &operator=(const Channel &other);

		bool	isInBan(const std::string &mask, const std::string &prefix);
		void	getInfo(const User &user);

	public:
		Channel(const std::string &name, const User &author, const \
			std::string &password = "");
		~Channel();
		// геттеры
		const std::string	&getName() const;
		bool 				getFlagPrivate() const;
		bool 				getFlagSecret() const;
		bool 				getFlagModerated() const;
		bool 				getFlagInviteOnly() const;
		bool 				getFlagTopicSet() const;
		// сеттеры
		void	setPassword(const User &user, const std::string &password);
		void 	setTopic(const User &user, const std::string &topic);
		void 	setUserLimit(unsigned int maxUsersLimit);
		void	setFlagPrivate(bool flagPrivate);
		void	setFlagSecret(bool flagSecret);
		void	setFlagModerated(bool flagModerated);
		void	setFlagInviteOnly(bool flagInviteOnly);
		void	setFlagTopicSet(bool flagTopicSet);
		// методы
		std::string	getFlagsForUsers() const;
		bool		isUserInvited(const User &user) const;
		bool		isUserOperator(const User &user) const;
		bool		isInChannel(const std::string &nickname) const;
		void		ReplyWithTopic(const User &user);
		void		ReplyWithUsersList(const User &user);
		void		ReplyWithInfo(const User &user);
		bool		isVoter(const User &user) const;
		bool		isEmptyChannel() const;
		void		connect(const User &user, const std::string &pass);
		void		sendMessage(const std::string &message, const User &fromUser, bool includeUser) const;
		void		inviteUser(const User &user, const User &invitedUser);
		void		addToOperators(const User &user);
		void		removeFromOperators(const User &user);
		void		addToVotersList(const User &user);
		void		removeFromVotersList(const User &user);
		void		addToBanMaskList(const std::string &mask);
		void		removeFromBanMaskList(const std::string &mask);
		void		disconnectUser(const User &user);
		void		removeInvitation(const User &user);
};

#endif