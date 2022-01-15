/**
 * Copyright (c) 2017, Bruno St-Aubin
 * Carleton University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WEB_STRUCTURE_MODEL_TYPE_HPP
#define WEB_STRUCTURE_MODEL_TYPE_HPP

#include <string>
#include <utility>
#include <vector>
#include <cadmium/core/modeling/message.hpp>
#include <json.hpp>

#include "port.hpp"
#include "port.hpp"
#include "submodel.hpp"
#include "int_coupling.hpp"

namespace cadmium::structure {
    class ModelType{
        private:
            std::map<std::string, Port *> ports_index;

        public:
            int id = -1;
            std::string name;
            std::string type;
            MessageType* message_type = nullptr;
            std::vector<Port*> ports = std::vector<Port *>();
            std::vector<InternalCoupling *> internal_couplings = std::vector<InternalCoupling *>();
            std::vector<int> components = std::vector<int>();
            std::vector<int> dim = std::vector<int>();

            std::vector<int> get_dim() const { return dim; }
            void set_dim(std::vector<int> value) {
                if (value.size() == 2) value.push_back(1);

                dim = value;
            }

            ModelType() = default;

            ModelType(std::string name, std::string type): name(std::move(name)), type(std::move(type)) { }

            ~ModelType() {
                for (Port* p : ports) delete p;
                for (InternalCoupling* ic : internal_couplings) delete ic;
            }

            Port * add_port(Port* port) {
                port->id = ports.size();
                ports.push_back(port);
                ports_index.insert({ port->name, port });

                return port;
            }

            Port* get_port(const std::string& port_name) {
                auto pos = ports_index.find(port_name);

                if (pos == ports_index.end()) return nullptr;

                else return pos->second;
            }

            void add_coupling(InternalCoupling* coupling) {
                if (coupling != nullptr) internal_couplings.push_back(coupling);
            }

            void add_component(SubModel* component) {
                if (component != nullptr) components.push_back(component->id);
            }

            nlohmann::json to_json() const {
                nlohmann::json out = {
                    {"id", id},
                    {"name", name},
                    {"type", type},
                    {"ports", nlohmann::json::array()}
                };

                if (!get_dim().empty()) out["dim"] = get_dim();

                if (message_type != nullptr) out["message_type"] = message_type->id;

                if (type == "coupled") {
                    for (InternalCoupling* lnk : internal_couplings) out["couplings"].push_back(lnk->to_json());

                    for (int c_idx : components) out["components"].push_back(c_idx);
                }

                for (Port* port : ports) out["ports"].push_back(port->to_json());

                return out;
            }
    };
}

#endif // WEB_STRUCTURE_MODEL_TYPE_HPP
