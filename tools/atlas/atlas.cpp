#include <et/core/tools.h>
#include <et/gui/guibase.h>
#include <et/imaging/pngloader.h>
#include <et/gui/textureatlaswriter.h>

using namespace et;

void printHelp()
{
	std::cout << "Using: " << std::endl <<
		"atlas -root <ROOT FOLDER> -out <OUTPUT FILE>" << std::endl << 
		"\tOPTIONAL: -size <SIZE>, default: 1024 - size of the output atlas" << std::endl <<
		"\tOPTIONAL: -pattern <PATTERN>, default: texture_%d.png" << std::endl <<
		"\tOPTIONAL: -nospace, default off - don't add one pixel space between images in atlas." << std::endl;
}

inline bool sortFunc(const TextureDescription::Pointer& d1, const TextureDescription::Pointer& d2)
	{ return d1->size.square() > d2->size.square(); }

int main(int argc, char* argv[])
{
	bool hasPattern = false;
	bool hasRoot = false;
	bool hasOutput = false;
	bool addSpace = true;

	std::string rootFolder;
	std::string outFile;
	std::string pattern = "texture_%d.png";
	int outputSize = 1024;

	for (int i = 1; i < argc; ++i)
	{
		if ((strcmp(argv[i], "-root") == 0) && (i + 1 < argc))
		{
			rootFolder = addTrailingSlash(std::string(argv[i+1]));
			if (folderExists(rootFolder))
			{
				hasRoot = true;
				++i;
			}
			else
			{
				std::cout << "ERROR: root folder not found" << std::endl;
				return 0;
			}
		}
		else if ((strcmp(argv[i], "-out") == 0) && (i + 1 < argc))
		{
			outFile = std::string(argv[i+1]);
			hasOutput = true;
			++i;
		}
		else if ((strcmp(argv[i], "-pattern") == 0) && (i + 1 < argc))
		{
			pattern = std::string(argv[i+1]);
			hasPattern = true;
			++i;
		}
		else if (strcmp(argv[i], "-nospace") == 0)
		{
			addSpace = false;
		}
		else if ((strcmp(argv[i], "-size") == 0) && (i + 1 < argc))
		{
			outputSize = strToInt(std::string(argv[i+1]));
			++i;
		}

	}

	if (!hasRoot || !hasOutput)
	{
		printHelp();
		return 0;
	}
	
	StringList fileList;
	TextureDescription::List textureDescriptors;
	findFiles(rootFolder, "*.png", true, fileList);	

	for (auto i : fileList)
	{
		TextureDescription::Pointer desc(new TextureDescription);
		PNGLoader::loadInfoFromFile(i, desc.reference());
		if ((desc->size.x > outputSize) || (desc->size.y > outputSize))
		{
			std::cout << "ERROR: image " << i << " is larger (" << desc->size << ") than output size (" <<
				outputSize << ";" << outputSize << "), please use -size option" << std::endl;
			return 0;
		}
		textureDescriptors.push_back(desc);
	}
	std::sort(textureDescriptors.begin(), textureDescriptors.end(), sortFunc);

	TextureAtlasWriter placer(addSpace);
	while (textureDescriptors.size())
	{
		TextureAtlasWriter::TextureAtlasItem& texture = placer.addItem(vec2i(outputSize));
		TextureDescription::List::iterator i = textureDescriptors.begin();

		int placedItems = 0;
		while (i != textureDescriptors.end())
		{
			if (placer.placeImage(*i, texture))
			{
				++placedItems;
				i = textureDescriptors.erase(i);
			}
			else 
			{
				++i;
			}
		}

		texture.texture->size.x = roundToHighestPowerOfTow(texture.maxWidth);
		texture.texture->size.y = roundToHighestPowerOfTow(texture.maxHeight);
	}

	for (TextureAtlasWriter::TextureAtlasItemList::const_iterator i = placer.items().begin(), e = placer.items().end(); i != e; ++i)
	{
		std::cout << "Texture: " << i->texture->size.x << "x" << i->texture->size.y << ":" << std::endl;
		for (TextureAtlasWriter::ImageItemList::const_iterator ii = i->images.begin(), ie = i->images.end(); ii != ie; ++ii)
			std::cout << "\t" << ii->place.origin << "\t|\t" << ii->place.size << "\t|\t" << ii->image->source << std::endl;
	}

	placer.writeToFile(outFile, pattern.c_str());

	return 0;
}
