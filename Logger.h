#ifndef FILELOGGER_HPP
#define FILELOGGER_HPP

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <ctime>
//
class Logger
{
public:
	
	static void LogMessage(const std::string& szMsg)
	{
		return;
		char *pline = new char[szMsg.size() + 1];
		strcpy(pline, szMsg.c_str());
		//
		std::ofstream myfile;
		std::string path = "e:\\VCamCoreLog.txt";
		//
		myfile.open(path, std::fstream::out | std::fstream::app);
		if (myfile)
		{
			myfile << get_current_time() << ": " << pline << "\r\n";
			myfile.close();
		}
		delete pline;
	}

private:
	static std::string get_current_time()
	{
		time_t rawtime;
		struct tm * timeinfo;
		char buffer[80];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, 80, "%d-%m-%Y %I:%M:%S", timeinfo);
		std::string str(buffer);
		//
		//delete buffer;
		return str;
	}
	static bool dirExists(const std::string& dirName_in)
	{
		DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
		if (ftyp == INVALID_FILE_ATTRIBUTES)
			return false;  //something is wrong with your path!

		if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
			return true;   // this is a directory!

		return false;    // this is not a directory!
	}
};

#endif // FILELOGGER_HPP