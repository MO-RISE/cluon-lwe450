///  Copyright 2021 RISE Research Institute of Sweden - Maritime Operations
///
///  Licensed under the Apache License, Version 2.0 (the "License");
///  you may not use this file except in compliance with the License.
///  You may obtain a copy of the License at
///
///      http://www.apache.org/licenses/LICENSE-2.0
///
///  Unless required by applicable law or agreed to in writing, software
///  distributed under the License is distributed on an "AS IS" BASIS,
///  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
///  See the License for the specific language governing permissions and
///  limitations under the License.
#ifndef SRC_LWE450_ASSEMBLER_HPP_
#define SRC_LWE450_ASSEMBLER_HPP_

#include <chrono>  // NOLINT
#include <functional>
#include <numeric>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

const std::set<std::string> VALID_HEADERS = {"UdPbC", "RaUdP", "RrUdP", "NkPgN",
                                             "RrTcP"};

using Message = std::pair<std::string, std::string>;

class LWE450MessageAssembler {
  std::string remainder_;
  std::function<void(const Message &,
                     const std::chrono::system_clock::time_point &)>
      delegate_{};

 public:
  LWE450MessageAssembler(
      std::function<void(const Message &,
                         const std::chrono::system_clock::time_point &)>
          delegate)
      : delegate_(delegate) {}

  void operator()(const std::string &data,
                  std::chrono::system_clock::time_point &&tp) {
    // Get total buffered data
    std::stringstream buffer{remainder_ + data};

    const std::chrono::system_clock::time_point time_stamp{std::move(tp)};

    // Handle any fully received lines
    std::string line;
    while (std::getline(buffer, line, '\n')) {
      if (buffer.eof()) {
        // If we run out of buffer, we dont have a full line and bail
        // accordingly
        break;
      }

      // Extract identifier and data
      // Trim for any trailing whitespace
      line.erase(line.find_last_not_of(" \n\r\t") + 1);

      // Simple validation
      // Example dataset
      // UdPbC\s:EI9999,d:AI9610,n:270*7C\!EIABB,04,04,9,265503900,5,,,WWwwwwp202T84=R3I`VfWOwwwwp20,4*50
      // UdPbC\s:EI9999,d:AI9610,n:271*7D\!EIABB,04,01,0,265503900,5,,,83u=4W006n3MD6Ab00@20PP0000000000000000000002,0*78
      // UdPbC\s:EI9999,d:AI9610,n:272*7E\!EIABB,04,02,0,265503900,5,,,T84=V3I`Vf`Wwwwwp202T84=V3I`Vf`Wwwwwp202T84=V,0*5F
      // UdPbC\s:EI9999,d:AI9610,n:273*7F\!EIABB,04,03,0,265503900,5,,,3I`Vf`Owwwwp202T84=T3I`Vf`Owwwwp202T84=T3I`Vf,0*47
      auto header = line.substr(0, 5);
      if (VALID_HEADERS.count(header) > 0) {
        auto payload = line.substr(5, std::string::npos);
        delegate_(std::make_pair(header, payload), time_stamp);
      }
    }
    // Make sure we save any remaining characters for next incoming
    remainder_ = line;
  }
};

#endif  // SRC_LWE450_ASSEMBLER_HPP_