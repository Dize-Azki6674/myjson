#include <nlohmann/json.hpp>
#include <snap/snap.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

/* myjson ****************************************/
     constexpr std::string_view VERSION = "1.0";
/*                                               */
/*   made by Azkey                               */
/*************************************************/

/* ToDo ************************************
    
********************************************/

const std::size_t indent = 4;

int main( int argc, char* argv[] ){
    using namespace snap;
	using json = nlohmann::json;


    App app = App{ "myjson" };
    app.about("JSONファイルを読み込んで成型・表示するCLIツール")
        .version("1.0")
        .author("Azkey");

    auto arg = Arg<std::filesystem::path>{ "FILE" };
    app.arg(arg);

    auto result = app.parse(argc, argv);

    auto path = result.at("FILE")->view<std::filesystem::path>().begin()[0];

    std::ifstream ifs{ path };
    if (!ifs.is_open())
    {
        std::cerr << "File cannot open: " << path << std::endl;
        return 1;
    }

    json j;
    try {
        ifs >> j;
    }
    catch (const json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << j.dump(indent) << std::endl;
    return 0;
}