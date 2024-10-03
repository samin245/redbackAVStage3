// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <cstdlib>

#include "rclcpp/rclcpp.hpp"
#include "cipher_interfaces/msg/cipher_message.hpp"
#include "cipher_interfaces/srv/cipher_answer.hpp"
#include "std_msgs/msg/header.hpp"

using namespace std::chrono_literals;
using std::placeholders::_1;

std::string decipher(std::string message, int key);
void verify_result(std_msgs::msg::Header header, std::string result);

class MinimalSubscriber : public rclcpp::Node
{
public:
  MinimalSubscriber()
  : Node("minimal_subscriber")
  {
    subscription_ = this->create_subscription<::cipher_interfaces::msg::CipherMessage>(
      "topic", 10, std::bind(&MinimalSubscriber::topic_callback, this, _1));
  }

private:
  void topic_callback(const cipher_interfaces::msg::CipherMessage & data) const
  {
    RCLCPP_INFO(this->get_logger(), "Message: '%s'\n Key: %d", 
    data.message.c_str(), data.key);

    std::string deciphered_msg = decipher(data.message, data.key);

    verify_result(data.header, deciphered_msg);
  }

  rclcpp::Subscription<cipher_interfaces::msg::CipherMessage>::SharedPtr subscription_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<MinimalSubscriber>());
  rclcpp::shutdown();
  return 0;
}

std::string decipher(std::string message, int key) {
  std::string result = "";
  for (char letter : message) {
    if (isalpha(letter)) {
      char start;
      char shift = (std::tolower(letter) - 'a' + key) % 26;
      (isupper(letter)) ? start = 'A' : start = 'a';
      result += (start + shift);
    } else {
      result += letter;
    }
  }

  return result;
}

void verify_result(std_msgs::msg::Header header, std::string result) {
  std::shared_ptr<rclcpp::Node> node = rclcpp::Node::make_shared("cipher_answer_client");  
  rclcpp::Client<cipher_interfaces::srv::CipherAnswer>::SharedPtr client =      
  node->create_client<cipher_interfaces::srv::CipherAnswer>("cipher_answer");   

  auto request = std::make_shared<cipher_interfaces::srv::CipherAnswer::Request>();
  request->header = header;
  request->answer = result;

  while (!client->wait_for_service(1s)) {
    if (!rclcpp::ok()) {
      RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Interrupted while waiting for the service. Exiting.");
      return;
    }
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "service not available, waiting again...");
  }

  auto response = client->async_send_request(request);
  // Wait for the result.
  if (rclcpp::spin_until_future_complete(node, response) ==
    rclcpp::FutureReturnCode::SUCCESS)
  {
    if (response.get()->result) {
       RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Decrypted result is correct!\n");
    } else {
      RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Decrypted result is wrong. Try again...\n");
    }
   
  } else {
    RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Failed to call service cipher_message");    
  }

  

}