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
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <chrono>  // NOLINT
#include <string>
#include <vector>

#include "LWE450_assembler.hpp"

TEST_CASE("Test LWE450MessageAssembler with empty payload.") {
  Message output;

  std::string DATA;

  LWE450MessageAssembler a{
      [&output](const Message& msg,
                const std::chrono::system_clock::time_point& timestamp) {
        output = msg;
      }};
  a(DATA, std::chrono::system_clock::time_point());

  const auto& [header, payload] = output;
  REQUIRE(header.empty());
  REQUIRE(payload.empty());
}

TEST_CASE("Test LWE450MessageAssembler with payload of wrong format") {
  size_t call_count{0};
  Message output;

  std::string DATA{"skfvdalfdadnsldasdc\nsakhbsacdsad\nahsdvds\nadjd"};

  LWE450MessageAssembler a{
      [&output, &call_count](
          const Message& line,
          const std::chrono::system_clock::time_point& timestamp) {
        output = line;
        call_count++;
      }};
  a(DATA, std::chrono::system_clock::time_point());

  const auto& [header, payload] = output;
  REQUIRE(header.empty());
  REQUIRE(payload.empty());
  REQUIRE_EQ(call_count, 0);
}

TEST_CASE("Test LWE450MessageAssembler with single correct message") {
  size_t call_count{0};
  Message output;

  std::string DATA{
      "UdPbC\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
      "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50\r\n"};

  LWE450MessageAssembler a{
      [&output, &call_count](
          const Message& line,
          const std::chrono::system_clock::time_point& timestamp) {
        output = line;
        call_count++;
      }};
  a(DATA, std::chrono::system_clock::time_point());

  const auto& [header, payload] = output;
  REQUIRE_EQ(header, "UdPbC");
  REQUIRE_EQ(payload,
             "\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
             "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50");
  REQUIRE_EQ(call_count, 1);
}

TEST_CASE(
    "Test LWE450MessageAssembler with single correct message inbetween "
    "scramble") {
  size_t call_count{0};
  Message output;

  std::string DATA{
      "UdPbC\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
      "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50\r\n"};

  std::string scramble{"dadnsldasdc\n"};

  LWE450MessageAssembler a{
      [&output, &call_count](
          const Message& msg,
          const std::chrono::system_clock::time_point& timestamp) {
        output = msg;
        call_count++;
      }};
  a(scramble + DATA + scramble, std::chrono::system_clock::time_point());

  const auto& [header, payload] = output;
  REQUIRE_EQ(header, "UdPbC");
  REQUIRE_EQ(payload,
             "\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
             "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50");
  REQUIRE_EQ(call_count, 1);
}

TEST_CASE("Test LWE450MessageAssembler with multiple messages") {
  size_t call_count{0};
  std::vector<Message> output;

  std::string DATA1{
      "UdPbC\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
      "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50\r\n"};
  std::string DATA2{
      "UdPbC\\s:EI9999,d:AI9610,n:271*7D\\!EIABB,04,01,0,265503900,5,,,83u="
      "4W006n3MD6Ab00@20PP0000000000000000000002,0*78\r\n"};
  std::string DATA3{
      "UdPbC\\s:EI9999,d:AI9610,n:272*7E\\!EIABB,04,02,0,265503900,5,,,T84=V3I`"
      "Vf`Wwwwwp202T84=V3I`Vf`Wwwwwp202T84=V,0*5F\r\n"};

  LWE450MessageAssembler a{
      [&output, &call_count](
          const Message& msg,
          const std::chrono::system_clock::time_point& timestamp) {
        output.push_back(msg);
        call_count++;
      }};
  a(DATA1, std::chrono::system_clock::time_point());
  a(DATA2, std::chrono::system_clock::time_point());
  a(DATA3, std::chrono::system_clock::time_point());

  REQUIRE_EQ(call_count, 3);
  const auto& [header0, payload0] = output[0];
  REQUIRE_EQ(header0, "UdPbC");
  REQUIRE_EQ(payload0,
             "\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
             "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50");

  const auto& [header1, payload1] = output[1];
  REQUIRE_EQ(header1, "UdPbC");
  REQUIRE_EQ(payload1,
             "\\s:EI9999,d:AI9610,n:271*7D\\!EIABB,04,01,0,265503900,5,,,83u="
             "4W006n3MD6Ab00@20PP0000000000000000000002,0*78");

  const auto& [header2, payload2] = output[2];
  REQUIRE_EQ(header2, "UdPbC");
  REQUIRE_EQ(
      payload2,
      "\\s:EI9999,d:AI9610,n:272*7E\\!EIABB,04,02,0,265503900,5,,,T84=V3I`"
      "Vf`Wwwwwp202T84=V3I`Vf`Wwwwwp202T84=V,0*5F");
}

TEST_CASE("Test LWE450MessageAssembler with split message") {
  size_t call_count{0};
  std::vector<Message> output;

  std::string DATA{
      "UdPbC\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
      "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50\r\n"};

  std::string input =
      DATA.substr(23, DATA.size()) + DATA + DATA + DATA.substr(0, 29);

  LWE450MessageAssembler a{
      [&output, &call_count](
          const Message& msg,
          const std::chrono::system_clock::time_point& timestamp) {
        output.push_back(msg);
        call_count++;
      }};
  a(input, std::chrono::system_clock::time_point());

  REQUIRE_EQ(call_count, 2);
  const auto& [header0, payload0] = output[0];
  REQUIRE_EQ(header0, "UdPbC");
  REQUIRE_EQ(payload0,
             "\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
             "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50");

  const auto& [header1, payload1] = output[1];
  REQUIRE_EQ(header1, "UdPbC");
  REQUIRE_EQ(payload1,
             "\\s:EI9999,d:AI9610,n:270*7C\\!EIABB,04,04,9,265503900,5,,,"
             "WWwwwwp202T84=R3I`VfWOwwwwp20,4*50");
}
