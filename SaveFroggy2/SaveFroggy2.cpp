#include <string>
#include <iostream>
#include <vector>
#include <fstream>

#include <stdio.h>
#include "sqlite3.h" 

static int ColumnResultCallback(void* userData, int argc, char** argv, char** azColName)
{
		std::string& result = *reinterpret_cast<std::string*>(userData);
		result += "\nNext Row:\n";

		for (int i = 0; i < argc; i++)
		{
				result += azColName[i];
				result += " = ";
				result += argv[i] ? argv[i] : "NULL";
				result += "\n";
		}

		printf("\n");
		return 0;
}


std::string GetResult(sqlite3* aDatabase, const char* aCommand)
{
		char* errorMessage = nullptr;
		std::string result;
		result += "Running: ";
		result += aCommand;

		/* Execute SQL statement */
		int rc = sqlite3_exec(aDatabase, aCommand, ColumnResultCallback, (void*)&result, &errorMessage);

		if (rc != SQLITE_OK)
		{
				result = errorMessage;
				sqlite3_free(errorMessage);
		}

		return result;
}


int main()
{
		char* zErrMsg = 0;
		int rc;

		const char* data = "Callback function called";

		sqlite3* database;
		rc = sqlite3_open("slime-toad.db", &database);

		if (rc)
		{
				fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(database));
				return 0;
		}

		std::vector<std::string> tables = { 
				"Canvas",
				"ElemScheme",
				"Mipmap",
				"ParamScheme",
				"CanvasNode",
				"Layer",
				"MipmapInfo",
				"Project",
				"CanvasPreview",
				"LayerThumbnail",
				"Offscreen" 
		};

		std::string totalResults;

		for (auto& tableName : tables)
		{
				std::string command = "SELECT * from ";
				command += tableName;

				auto result = GetResult(database, command.c_str());

				puts(result.c_str());


				totalResults += "\n\n\n\n\n\n\nTableName: ";
				totalResults += tableName;
				totalResults += "\n";
				totalResults += result;
		}


		std::ofstream outfile("output.txt");
		outfile << totalResults;
		outfile.close();

	 /* Create SQL statement */
		//auto result = GetResult(database, "SELECT * from Layer");
		//
		//puts(result.c_str());



		sqlite3_close(database);

		return 0;
}