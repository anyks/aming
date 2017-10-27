/* SERVER PROXY AMING */
/*
*  author:     Yuriy Lobarev
*  phone:      +7(910)983-95-90
*  telegram:   @forman
*  email:      info@anyks.com
*  date:       10/27/2017 18:42:35
*  copyright:  © 2017 anyks.com
*/



#include <iostream>
#include <string>
#include <regex>

int main(int argc, char *argv[])
{
  std::string url (argv[1]);
  unsigned counter = 0;

  std::regex url_regex (
    R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
    std::regex::extended
  );
  std::smatch url_match_result;

  std::cout << "Checking: " << url << std::endl;

  if (std::regex_match(url, url_match_result, url_regex)) {
    for (const auto& res : url_match_result) {
      std::cout << counter++ << ": " << res << std::endl;
    }
  } else {
    std::cerr << "Malformed url." << std::endl;
  }

  return EXIT_SUCCESS;
}