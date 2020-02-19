#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>

#include <intrin.h>

#include <cassert>
#include <cstdio>
#include "sqlite3.h" 

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


// little endian if true
namespace details 
{
		const int n = 1;
}

// little endian if true
const bool litte_endian = *(char*)&details::n == 1;

struct FileReader
{
		FileReader(std::string const& aFile)
		{
				//if (!std::filesystem::exists(aFile))
				//{
				//		return;
				//}

				std::ifstream file(aFile, std::ios::binary | std::ios::ate);

				if (file.is_open())
				{
						std::streamsize size = file.tellg();
						file.seekg(0, std::ios::beg);

						mData.resize(size);
						file.read(mData.data(), size);
						assert(false == file.bad());

						if (false == file.bad())
						{
								mOpened = true;
						}
				}
		}

		template<typename tType>
		static constexpr size_t GetSize()
		{
				return sizeof(std::aligned_storage_t<sizeof(tType), alignof(tType)>);
		}

		template<typename tType>
		tType& Read()
		{
				auto bytesToRead = GetSize<tType>();

				assert((mBytesRead + bytesToRead) <= mData.size());

				auto &value = *reinterpret_cast<tType*>(mData.data() + mBytesRead);

				mBytesRead += bytesToRead;

				return value;
		}
		

		template<typename tType>
		tType ReadValue()
		{
				auto bytesToRead = GetSize<tType>();

				assert((mBytesRead + bytesToRead) <= mData.size());

				tType value;
				memcpy(&value, reinterpret_cast<void*>(mData.data() + mBytesRead), sizeof(tType));

				mBytesRead += bytesToRead;

				return value;
		}
    
		template<typename tType>
		void Read(tType* aBuffer, size_t aSize)
		{
				if (aSize == 0)
				{
				return;
				}

				auto bytesToRead = GetSize<tType>() * aSize;
				assert((mBytesRead + bytesToRead) <= mData.size());

				memcpy(aBuffer, mData.data() + mBytesRead, bytesToRead);
    
				mBytesRead += bytesToRead;
		}
		
    
		template<typename tType>
		void Read(tType& aValueToReadInto)
		{
				Read(&aValueToReadInto, 1);
		}

		std::vector<char> mData;
		size_t mBytesRead = 0;
		bool mOpened = false;
};

static int ColumnResultCallback(void* userData, int argc, char** argv, char** azColName)
{
		std::string& result = *reinterpret_cast<std::string*>(userData);
		result += "\nNext Row:\n";

		for (int i = 0; i < argc; i++)
		{
				result += azColName[i];
				result += " = ";
				result += argv[i] ? argv[i] : "NULL";
				result += '\n';
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
		result += '\n';

		/* Execute SQL statement */
		int rc = sqlite3_exec(aDatabase, aCommand, ColumnResultCallback, (void*)&result, &errorMessage);

		if (rc != SQLITE_OK)
		{
				result += errorMessage;
				sqlite3_free(errorMessage);
		}

		return result;
}


struct SqlHeader
{
//**      0      16     Header string: "SQLite format 3\000"
		char HeaderString[16];
//**     16       2     Page size in bytes.  (1 means 65536)
		uint16_t PageSizeInBytes; //Big Endian
//**     18       1     File format write version
		uint8_t FormatWriteVersion;
//**     19       1     File format read version
		uint8_t FormatReadVersion;
//**     20       1     Bytes of unused space at the end of each page
		uint8_t WastedPageSpaceInBytes;
//**     21       1     Max embedded payload fraction (must be 64)
		uint8_t MaxEmbeddedPayloadFraction;
//**     22       1     Min embedded payload fraction (must be 32)
		uint8_t MinEmbeddedPayloadFraction;
//**     23       1     Min leaf payload fraction (must be 32)
		uint8_t MinLeafPayloadFraction;
//**     24       4     File change counter
		uint32_t FileChangeCounter; //Big Endian
//**     28       4     Reserved for future use
		uint32_t Reserved;
//**     32       4     First freelist page
		uint32_t FirstFreelistPage;
//**     36       4     Number of freelist pages in the file
		uint32_t NumberOfFreelistPage;
//**     40      60     15 4-byte meta values passed to higher layers
		uint32_t MetaValues[15];
//**     40       4     Schema cookie
		uint32_t SchemaCookie;
//**     44       4     File format of schema layer
		uint32_t SchemaLayerFormat;
//**     48       4     Size of page cache
		uint32_t SizeOfPageCache;
//**     52       4     Largest root-page (auto/incr_vacuum)
		uint32_t LargestRootPage;
//**     56       4     1=UTF-8 2=UTF16le 3=UTF16be
		uint32_t UtfVerion;
//**     60       4     User version
		uint32_t UserVersion;
//**     64       4     Incremental vacuum mode
		uint32_t IncrementalVacuumMode;
//**     68       4     Application-ID
		uint32_t AppId;
//**     72      20     unused
		char Unused[16];
//**     92       4     The version-valid-for number
		uint32_t VersionValidForNumber;
//**     96       4     SQLITE_VERSION_NUMBER
		uint32_t SqliteVersionNumber;
};

void EndianSwap(uint16_t& aValue)
{
		if (litte_endian)
		{
				aValue = _byteswap_ushort(aValue);
		}
}

void EndianSwap(uint32_t& aValue)
{
		if (litte_endian)
		{
				aValue = _byteswap_ulong(aValue);
		}
}


void EndianSwap(uint64_t& aValue)
{
		if (litte_endian)
		{
				aValue = _byteswap_uint64(aValue);
		}
}


//SqlHeader ReadHeader(FileReader& aReader)
//{
//		SqlHeader header;
//		
//		aReader.Read(header.HeaderString);
//		aReader.Read(header.PageSizeInBytes);
//		aReader.Read(header.FormatWriteVersion);
//		aReader.Read(header.FormatReadVersion);
//		aReader.Read(header.WastedPageSpaceInBytes);
//		aReader.Read(header.MaxEmbeddedPayloadFraction);
//		aReader.Read(header.MinEmbeddedPayloadFraction);
//		aReader.Read(header.MinLeafPayloadFraction);
//		aReader.Read(header.FileChangeCounter);
//		aReader.Read(header.Reserved);
//		aReader.Read(header.FirstFreelistPage);
//		aReader.Read(header.NumberOfFreelistPage);
//		aReader.Read(header.MetaValues);
//		aReader.Read(header.SchemaCookie);
//		aReader.Read(header.SchemaLayerFormat);
//		aReader.Read(header.SizeOfPageCache);
//		aReader.Read(header.LargestRootPage);
//		aReader.Read(header.UtfVerion);
//		aReader.Read(header.UserVersion);
//		aReader.Read(header.IncrementalVacuumMode);
//		aReader.Read(header.AppId);
//		aReader.Read(header.Unused);
//		aReader.Read(header.VersionValidForNumber);
//		aReader.Read(header.SqliteVersionNumber);
//
//		
//		//EndianSwap(header.HeaderString);
//		EndianSwap(header.PageSizeInBytes);
//		//EndianSwap(header.FormatWriteVersion);
//		//EndianSwap(header.FormatReadVersion);
//		//EndianSwap(header.WastedPageSpaceInBytes);
//		//EndianSwap(header.MaxEmbeddedPayloadFraction);
//		//EndianSwap(header.MinEmbeddedPayloadFraction);
//		//EndianSwap(header.MinLeafPayloadFraction);
//		EndianSwap(header.FileChangeCounter);
//		EndianSwap(header.Reserved);
//		EndianSwap(header.FirstFreelistPage);
//		EndianSwap(header.NumberOfFreelistPage);
//		//EndianSwap(header.MetaValues);
//		EndianSwap(header.SchemaCookie);
//		EndianSwap(header.SchemaLayerFormat);
//		EndianSwap(header.SizeOfPageCache);
//		EndianSwap(header.LargestRootPage);
//		EndianSwap(header.UtfVerion);
//		EndianSwap(header.UserVersion);
//		EndianSwap(header.IncrementalVacuumMode);
//		EndianSwap(header.AppId);
//		//EndianSwap(header.Unused);
//		EndianSwap(header.VersionValidForNumber);
//		EndianSwap(header.SqliteVersionNumber);
//
//		return header;
//}

struct StbImage
{
		int width;
		int height;
		int channels;
		stbi_uc* pixels;
};

std::vector<StbImage> FindAllPotentialPngFiles(FileReader& reader)
{
		std::vector<unsigned char> niceData;
		niceData.reserve(reader.mData.size());

		for (auto byte : reader.mData)
		{
				niceData.emplace_back(static_cast<unsigned char>(byte));
		}

		const unsigned char* dataPtr = niceData.data();
		const unsigned char* endPtr = dataPtr + reader.mData.size();

		const unsigned char cPngStartHeader[] = { '\211', 'P', 'N', 'G', '\r', '\n', '\032', '\n' };
		std::vector<StbImage> possibleMatches;

		// We have to slowly search by byte
		for (; (dataPtr + 8) < endPtr; ++dataPtr)
		{
				if (0 == memcmp(dataPtr, cPngStartHeader, 8))
				{
						StbImage image;
						image.pixels = stbi_load_from_memory(dataPtr, static_cast<int>(endPtr - dataPtr), &image.width, &image.height, &image.channels, 4);

						if (image.pixels)
						{
								possibleMatches.emplace_back(image);
						}
						else
						{
								puts(stbi__g_failure_reason);
						}
				}
		}

		return possibleMatches;
}

int main()
{
		FileReader reader{"slime-toad.db"};

		auto matches = FindAllPotentialPngFiles(reader);

		//for (auto& match : matches)
		//{
		//		int width;
		//		int height;
		//		int channels;
		//		auto pixels = stbi_load_from_memory(match, reader.mData.size(), &width, &height, &channels, 4);
		//
		//}
		//auto header = ReadHeader(reader);

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
				std::string command = "SELECT sql FROM sqlite_master WHERE name = '";
				command += tableName;
				command += "';";

				auto result = GetResult(database, command.c_str());

				puts(result.c_str());


				totalResults += "\n\n\n\n\n\n\nTableName Schema: ";
				totalResults += tableName;
				totalResults += "\n";
				totalResults += result;
		}

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