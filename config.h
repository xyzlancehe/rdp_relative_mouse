#include <fstream>
#include <string>

constexpr char *const configFileName = "./config.ini";

void createNewConfig()
{
    std::ofstream f;
    f.open(configFileName, std::ios::out);
    f << "[Boundary]" << std::endl
      << "xMin = 0" << std::endl
      << "xMax = 65535" << std::endl
      << "yMin = 0" << std::endl
      << "yMax = 65535" << std::endl
      << "[Main]" << std::endl
      << "factor = 1.0" << std::endl
      << "mode = 1" << std::endl
      << "[Manual]" << std::endl
      << "speed = 50" << std::endl
      << "[Auto]" << std::endl
      << "interval = 100" << std::endl
      << "speedStart = 30" << std::endl
      << "speedEnd = 80" << std::endl
      << "speedStep = 10" << std::endl;
    f.close();
}
