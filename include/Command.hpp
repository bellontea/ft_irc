#ifndef COMMAND_HPP
# define COMMAND_HPP

# include "utils.hpp"

# include <string>
# include <vector>
# include <queue>

class Command {
	private:
		std::string						prefix;
		std::string						command;
		std::vector<std::string>		params;

		Command();
		Command(const Command& copy);
		Command	&operator=(const Command& other);
	public:
		Command(const std::string &str);
		~Command();
		const std::string				&getPrefix() const;
		const std::string				&getCommand() const;
		const std::vector<std::string>	&getParams() const;
};

#endif