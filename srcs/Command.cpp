#include "Command.hpp"

Command::Command(const std::string &str) {
	std::queue<std::string>	tmp;
	tmp = splitToStrQueue(std::string(str.begin(), str.end() - 1), ' ', false);
	if (tmp.size() > 0 && tmp.front()[0] == ':') {
		prefix = std::string(tmp.front().begin() + 1, tmp.front().end());
		tmp.pop();
	}
	if (!tmp.empty()) {
		command = tmp.front();
		tmp.pop();
	}
	while (!tmp.empty()) {
		if (tmp.front()[0] == ':') {
			std::string	s(tmp.front().begin() + 1, tmp.front().end());
			tmp.pop();
			while (tmp.size() > 0) {
				s.append(" ");
				s.append(tmp.front());
				tmp.pop();
			}
			params.push_back(s);
		}
		else {
			params.push_back(tmp.front());
			tmp.pop();
		}
	}
}

Command::~Command() {}

const std::string	&Command::getPrefix() const {
	return prefix;
}

const std::string	&Command::getCommand() const {
	return command;
}

const std::vector<std::string>	&Command::getParams() const {
	return params;
}
