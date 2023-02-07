#include <iostream>
#include "Server.hpp"
#include "utils.hpp"
#include "Sha256.hpp"
#include <stdlib.h>

bool stopScheduled = true;

static int get_right_port(const char *str)
{
	long port = 0;

	for (int i = 0; str[i] && i <= 6; i++)
	{
		if (str[i] >= '0' && str[i] <= '9')
			port = port * 10 + str[i] - '0';
		else
			return 0;
	}
	if (port < 1024 || port > 49151)
		return 0;
	return (int)port;
}

static void printUsage()
{
	std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
}

static void printError(std::string str)
{
	std::cout << "Error: " << str << std::endl;
}

void	sigintEventHandler(int notUsedSignalNumber)
{
	(void)notUsedSignalNumber;
	stopScheduled = true;
}

int main_server_proc(int port, const char *pass)
{
	Server srv(port, pass);
	try {
		srv.getConfig(SRV_CONFIG_FILE);
	} catch (const std::exception& e) {
		log("Fatal error in server configuration!");
		return EXIT_ERROR_DEFAULT;
	}

	try {
		srv.initNewSocket();
	} catch (const std::exception& e) {
		log("Fatal error on server start!");
		return EXIT_ERROR_DEFAULT;
	}

	try {
		while (!stopScheduled)
		{
			srv.acceptConnection();
			srv.checkNewCommands();
			srv.checkActiveUsers();
			srv.clearBrokenConnections();
			srv.removeEmptyChannels();
		}
	} catch (const std::exception& e) {
		log("Fatal error in server loop!");
		return EXIT_ERROR_DEFAULT;
	}


	/*
	 * 0 = OK, Shutdown
	 * 1 = Reboot
	 * 2,3,4... = error code
	*/
	if (srv.isRebootScheduled())
	{
		log("Rebooting by flag...");
		return EXIT_REBOOT;
	}
	log("Exiting...");
	return EXIT_OK;
}

int main(int argc, char **argv)
{
	/// DEBUG COMMANDS
	if (argc == 4 && (std::string)argv[1] == "DEBUG" && (std::string)argv[2] == "hash")
	{
		std::cout << sha256(argv[3]) << std::endl;
		return 0;
	}
	else if (argc == 5 && (std::string)argv[1] == "DEBUG" && (std::string)argv[2] == "addoper")
	{
		std::string tmp = (std::string)"echo \"operator="
			+ argv[3] + "=" + sha256(argv[4]) + "\" >> " + SRV_CONFIG_FILE;
		system(tmp.c_str());
		return 0;
	}
	/// END of Debug commands

	if (argc != 3)
	{
		printUsage();
		return EXIT_ERROR_DEFAULT;
	}

	int port = get_right_port(argv[1]);
	if (!port)
	{
		printError("Incorrect port!");
		printUsage();
		return EXIT_ERROR_DEFAULT;
	}

	stopScheduled = false;
	signal(SIGINT, sigintEventHandler);

	int exitCode = 0;
	while ((exitCode = main_server_proc(port, argv[2])) == EXIT_REBOOT);

	return exitCode;
}
