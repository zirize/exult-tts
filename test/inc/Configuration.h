#include <string>
struct Configuration { std::string cmd;
 void value(const char* k, std::string& v, const char* d){ v = std::string(k).find("command")!=std::string::npos ? cmd : d; } };
extern Configuration* config;
