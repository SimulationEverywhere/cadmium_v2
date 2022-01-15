
#include <iostream>
#include <string>
#include <utility>

#include <cadmium/core/modeling/message.hpp>

#ifndef _GEO_HOSPITAL_EMERGENCY_T_HPP_
#define _GEO_HOSPITAL_EMERGENCY_T_HPP_

struct emergency_t: public cadmium::Message<emergency_t> {
    std::string area_id;
    int port_i;
    int quantity;

    emergency_t(): port_i(0), quantity(0) {}

    explicit emergency_t(std::string area_id, int port_i, int quantity) : area_id(std::move(area_id)), port_i(port_i), quantity(quantity) {}

    static cadmium::MessageType get_message_type() {
        return cadmium::MessageType("o_emergency", "N/A", { "area_id", "port_i", "quantity" });
    }
};

std::ostream& operator<<(std::ostream &out, emergency_t e) {
    out << e.area_id << "," << e.port_i << "," << e.quantity;

    return out;
}

#endif //_GEO_HOSPITAL_EMERGENCY_T_HPP_
