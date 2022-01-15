#ifndef WEB_STRUCTURE_HPP
#define WEB_STRUCTURE_HPP

#include <string>
#include <utility>
#include <vector>
#include <cadmium/core/modeling/atomic.hpp>
#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/core/modeling/message.hpp>
#include <json.hpp>

#include "submodel.hpp"
#include "model_type.hpp"

using json = nlohmann::json;

namespace cadmium::structure {
    class structure{
    private:
        std::vector<ModelType*> model_types = std::vector<ModelType*>();
        std::vector<MessageType*> message_types = std::vector<MessageType*>();
        std::vector<SubModel*> submodels = std::vector<SubModel*>();

        std::map<std::string, ModelType*> model_types_index;
        std::map<std::string, MessageType*> message_types_index;
        std::map<std::string, SubModel*> submodels_index;

    public:
        std::string formalism = "DEVS";
        std::string simulator = "Cadmium";
        int top = -1;

        structure() = default;

        ~structure() {
            for (ModelType* mt : model_types) delete mt;
            for (MessageType* mt : message_types) delete mt;
            for (SubModel* sm : submodels) delete sm;
        }

        static structure * from_coupled_model(const std::shared_ptr<Coupled>& model) {
            auto * s = new structure();

            model->traverse([s](Component* c) { s->add_model_type(c); });
            model->traverse([s](Component* c) { s->add_component(c); });
            model->traverse([s](Component* c) { s->add_model_type_components(c); });
            model->traverse([s](Component* c) { s->add_couplings(c); });

            s->top = s->get_component(model->id)->id;

            return s;
        }

        ModelType* get_model_type(const std::string& type) {
            auto pos = model_types_index.find(type);

            if (pos == model_types_index.end()) return nullptr;

            else return pos->second;
        }

        SubModel* get_component(const std::string& id) {
            auto pos = submodels_index.find(id);

            if (pos == submodels_index.end()) return nullptr;

            else return pos->second;
        }

        MessageType* get_message_type(const std::string& type) {
            auto pos = message_types_index.find(type);

            if (pos == message_types_index.end()) return nullptr;

            else return pos->second;
        }

        Port* get_port(const std::string& model_id, const std::string& port) {
            SubModel* c = get_component(model_id);
            ModelType* mt = model_types.at(c->model_type_id);

            return mt->get_port(port);
        }

        ModelType* add_model_type(Component * c) {
            // Check if model_type already exists
            if (c->className.empty()) {
                throw std::bad_exception();  // TODO custom exceptions
            }

            ModelType* p_mt = get_model_type(c->className);

            if (p_mt) return p_mt;

            auto atomic = dynamic_cast<cadmium::AbstractAtomic*>(c);

            std::string devsType = (atomic == nullptr)  ? "coupled" : "atomic";

            // Model type doesn't already exist, must be created from model extension
            p_mt = add_model_type(new ModelType(c->className, devsType));

            if (atomic != nullptr) {
                p_mt->message_type = add_message_type(atomic->get_message_type());
            }

            add_ports(p_mt, "input", c->inPorts);
            add_ports(p_mt, "output", c->outPorts);

            return p_mt;
        }

        ModelType* add_model_type(ModelType* p_mt) {
            // Add built model to data_structures, set idx, etc.
            p_mt->id = model_types.size();
            model_types.push_back(p_mt);
            model_types_index.insert({ p_mt->name, p_mt });

            return p_mt;
        }

        MessageType* add_message_type(const MessageType& msg) {
            MessageType* p_msg = get_message_type(msg.name);

            if (p_msg) return p_msg;

            p_msg = new MessageType(msg.name, msg.description, msg.fields);
            p_msg->id = message_types.size();

            message_types.push_back(p_msg);
            message_types_index.insert({ p_msg->name, p_msg });

            return p_msg;
        }

        SubModel* add_component(Component* c) {
            ModelType* mt = get_model_type(c->className);

            return add_component(new SubModel(c->id, mt->id));
        }

        SubModel* add_component(SubModel* sm) {
            sm->id = submodels.size();
            submodels.push_back(sm);
            submodels_index.insert({ sm->instance_id, sm });

            return sm;
        }

        void add_model_type_components(Component * c) {
            ModelType * mt = get_model_type(c->className);

            for (const auto& p_component : c->get_components()) {
                mt->add_component(get_component(p_component->id));
            }
        }

        void add_ports(ModelType * p_mt, const std::string& type, const cadmium::PortSet& portSet) {
            for (auto const& port: portSet.ports) {
                Port* p_port = p_mt->add_port(new Port(port->id, type));
                MessageType* p_msg = add_message_type(port->get_message_type());
                p_port->message_type_idx = p_msg->id;
            }
        }

        InternalCoupling* make_coupling(cadmium::coupling link) {
            const auto& a = std::get<0>(link);
            auto b = std::get<1>(link);

            SubModel* from_c = get_component(a->parent->id);
            SubModel* to_c = get_component(b->parent->id);

            if (from_c == nullptr || to_c == nullptr) return nullptr;

            Port* from_p = get_port(a->parent->id, a->id);
            Port* to_p = get_port(b->parent->id, b->id);

            if (from_p == nullptr || to_p == nullptr) return nullptr;

            return new InternalCoupling(from_c->id, from_p->id, to_c->id, to_p->id);
        }

        void add_couplings(Component* c) {
            auto coupled = dynamic_cast<cadmium::Coupled*>(c);

            if (coupled == nullptr) return;

            // Check if model_type already exists
            ModelType* p_mt = get_model_type(c->className);

            for (const auto& l : coupled->EOC) p_mt->add_coupling(make_coupling(l));
            for (const auto& l : coupled->EIC) p_mt->add_coupling(make_coupling(l));
            for (const auto& l : coupled->IC) p_mt->add_coupling(make_coupling(l));
        }

        nlohmann::json to_json() {
            json j_structure = {
                    {"simulator", simulator},
                    {"formalism", formalism},
                    {"top", top},
                    {"model_types", json::array()},
                    {"message_types", json::array()}
            };

            for (ModelType* mt : model_types) {
                j_structure["model_types"].push_back(mt->to_json());
            }

            for (MessageType* mt : message_types) {
                j_structure["message_types"].push_back(mt->to_json());
            }

            for (SubModel* sm : submodels) {
                j_structure["components"].push_back(sm->to_json());
            }

            return j_structure;
        }
    };
}

#endif // WEB_STRUCTURE_HPP