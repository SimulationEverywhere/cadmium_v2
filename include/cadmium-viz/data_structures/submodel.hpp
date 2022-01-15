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

#ifndef WEB_STRUCTURE_SUBMODEL_HPP
#define WEB_STRUCTURE_SUBMODEL_HPP

#include <string>
#include <utility>
#include <vector>
#include <json.hpp>

namespace cadmium::structure {

    class SubModel {
    public:
        long id = -1;
        std::string instance_id;
        long model_type_id = -1;

        SubModel() = default;

        explicit SubModel(std::string model_id): instance_id(std::move(model_id)) {}

        SubModel(std::string instance_id, long model_type_id): instance_id(std::move(instance_id)), model_type_id(model_type_id) { }

        nlohmann::json to_json() {
            return {
                {"id", instance_id},
                {"model_type", model_type_id}
            };
        }
    };
}

#endif // WEB_STRUCTURE_SUBMODEL_HPP
