
#include "MasterHeader.h"
#include "../Extern/rapidjson/document.h"
#include "../Extern/rapidjson/error/en.h"
#include <fstream>
#include <sstream>

// forward declarations of internal functions & Global Variables
void Load_Cards(std::string const&);
std::array<std::vector<Card>, NUM_OF_RARITIES> cardPool;

void Initialise_System(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// set gamestate to "Running"
	int gGameRunning = 1;

	// Initialization of your own variables go here

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);

	// Changing the window title
	AESysSetWindowTitle("UI Demo!");

	// reset the system modules
	AESysReset();

	printf("Hello World\n");
}

void Load_Global_Assets() {
	// Load Cards from JSON
	//Load_Cards("../Data/Cards.JSON");
}

void Unload_Global_Assets() {
	//
}

// loads cards from JSON file
void Load_Cards(std::string const &filename) {
	//
}


// Parse all cards from JSON file
std::vector<CardStats> loadCards(const std::string& filepath) {
	std::vector<CardStats> cards;

	// Read file into string
	std::ifstream file(filepath);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string jsonStr = buffer.str();

	rapidjson::Document doc;
	doc.Parse(jsonStr.c_str());

	//for (const auto& cardJson : doc["cards"].GetArray()) {
	//	CardStats card;
	//	card.ID = cardJson["id"].GetString();
	//	card.rarity = cardJson["rarity"].GetInt();
	//	card.active = parseEffects(cardJson["active"]);
	//	card.passive = parseEffects(cardJson["passive"]);
	//	cards.push_back(card);
	//}

	return cards;
}
