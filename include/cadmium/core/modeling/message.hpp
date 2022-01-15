//
// Created by Bruno St-Aubin on 2021-12-30.
//

#ifndef _CADMIUM_CORE_MODELING_MESSAGE_HPP_
#define _CADMIUM_CORE_MODELING_MESSAGE_HPP_

#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <json.hpp>

namespace cadmium {
    class MessageType {
    public:
        long id = -1;
        std::string name;
        std::string description;
        std::vector<std::string> fields = {};

        explicit MessageType(std::string _name, std::string _description, std::vector<std::string> _fields) {
            name = std::move(_name);
            description = std::move(_description);
            fields = std::move(_fields);
        }

        nlohmann::json to_json() {
            return {
                    {"id", id},
                    {"name", name},
                    {"template", fields},
                    {"description", description}
            };
        }
    };

    template<class T>
    class Message {
    public:
        static MessageType get_message_type() {
            return T::get_message_type();
        }
    };
}

#endif //_CADMIUM_CORE_MODELING_MESSAGE_HPP_