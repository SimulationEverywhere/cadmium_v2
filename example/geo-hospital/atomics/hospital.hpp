#ifndef CADMIUM_EXAMPLE_HOSPITAL_HPP
#define CADMIUM_EXAMPLE_HOSPITAL_HPP

#include <iostream>
#include <string>
#include <random>
#include <utility>

#include <cadmium/core/modeling/atomic.hpp>
#include <cadmium/core/modeling/message.hpp>

#include "../data_structures/emergency.hpp"
#include "../utils/tools.hpp"
#include "../external/json.hpp"

// model parameters
struct hospital_params {
	std::string id = "";
    std::string name = "";
	int rate = 0;
	int capacity = 0;
};

struct hospital_state: public cadmium::Message<hospital_state> {
    int active = 0;
    int total = 0;
    int released = 0;
    int rejected = 0;

    std::vector<emergency_t> emergencies;

    static cadmium::MessageType get_message_type() {
        return cadmium::MessageType("s_hospital", "N/A", { "active", "total", "released", "rejected" });
    }
};

// TODO: Is there any way we can remove this and include it with the model? Like a get_state_message function
// TODO: that returns a string or a vector of strings that can be logged with proper separators after?
// TODO: this way, there's less risk of users messing up the log file with weird symbols or separators.
std::ostream& operator<<(std::ostream &out, const hospital_state& s) {
	out << s.active << "," << s.total << "," << s.released << "," << s.rejected;

	return out;
}

class hospital: public cadmium::Atomic<hospital_state> {
private:
    hospital_params params;
public:
    hospital() : Atomic("test", "test", hospital_state()) { }

    hospital(std::string id, nlohmann::json j_params): cadmium::Atomic<hospital_state>(std::move(id), "hospital", hospital_state()) {
        addInPort<emergency_t>("processor_in");
        addOutPort<emergency_t>("processor_out");

		params.id = j_params.at("index").get<std::string>();
		params.name = j_params.at("facility_name").get<std::string>();
		params.rate = j_params.at("rate").get<int>();
		params.capacity = j_params.at("capacity").get<int>();
    }

    void internalTransition(hospital_state& s) const override {
    	if (!s.emergencies.empty()) s.emergencies.clear();

    	else {
            int released = params.rate > s.active ? s.active : params.rate;

            s.active -= released;
            s.released += released;
    	}
    }

    void externalTransition(hospital_state& s, double e, const cadmium::PortSet& x) const override {
        // TODO: I really like how receiving inputs is done, really clear and concise.
        for (const auto &i : x.getBag<emergency_t>("processor_in")) {
            s.active += i->quantity;
            s.total += i->quantity;

            int rejected = s.active > params.capacity ? s.active - params.capacity : 0;

            s.rejected += rejected;
            s.active -= rejected;

            if (rejected > 0) {
                s.emergencies.emplace_back(emergency_t(i->area_id, i->port_i, rejected));
            }
        }
    }

    void output(const hospital_state& s, const cadmium::PortSet& y) const override {
        // TODO: I really like how outputs are done, really clear and concise.
		for(const auto &em : s.emergencies){
            y.addMessage("processor_out", em);
		}
    }

    [[nodiscard]] double timeAdvance(const hospital_state& s) const override {
        // TODO: I like not having to deal with NDTime. I know the prof contains about double as time. Maybe we can
        // TODO: think of using an int and leaving the interpretation for post simulation.
        if (!s.emergencies.empty()) return 0;

        if (s.active > 0) return 1;

        else return std::numeric_limits<double>::infinity();
    }
};

#endif //CADMIUM_EXAMPLE_HOSPITAL_HPP