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

#ifndef WEB_STRUCTURE_INT_COUPLING_HPP
#define WEB_STRUCTURE_INT_COUPLING_HPP

#include <string>
#include <vector>
#include <json.hpp>

namespace cadmium::structure {
    class InternalCoupling{
    private:
        long from_c_idx = -1;
        long from_p_idx = -1;
        long to_c_idx = -1;
        long to_p_idx = -1;

    public:
        InternalCoupling(long from_c_idx, long from_p_idx, long to_c_idx, long to_p_idx): from_c_idx(from_c_idx), from_p_idx(from_p_idx), to_c_idx(to_c_idx), to_p_idx(to_p_idx) { }

        nlohmann::json to_json() {
            nlohmann::json out = nlohmann::json::array();

            out.push_back(from_c_idx);
            out.push_back(from_p_idx);
            out.push_back(to_c_idx);
            out.push_back(to_p_idx);

            return out;
        }
    };
}

#endif // WEB_STRUCTURE_INT_COUPLING_HPP
