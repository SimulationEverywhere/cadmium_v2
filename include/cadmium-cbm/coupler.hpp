//
// Created by Bruno St-Aubin on 2022-01-06.
//

#ifndef CADMIUM_COUPLER_HPP
#define CADMIUM_COUPLER_HPP

#include <fstream>
#include <sstream>
#include <map>

#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium-cbm/model_mapping.hpp>
#include <cadmium-cbm/factory.hpp>

namespace cadmium::cbm::coupler {
    static void AddFromComponent(cadmium::Coupled& coupled, const std::shared_ptr<Component>& mm_component);

    static std::shared_ptr<cadmium::Component> BuildFromAutoCoupled(const std::shared_ptr<AutoCoupled>& mm_coupled) {
        cadmium::Coupled coupled = cadmium::Coupled(mm_coupled->id, mm_coupled->model_type);

        for (auto &c: mm_coupled->components) {
            cadmium::cbm::coupler::AddFromComponent(coupled, c);
        }

        for (auto &c: mm_coupled->couplings) {
            for (auto &l: c->links) {
                coupled.addInternalCoupling(l.at(0), c->from_port, l.at(1), c->to_port);
            }
        }

        // TODO: ports

        return std::make_shared<cadmium::Coupled>(coupled);
    }

    static std::shared_ptr<cadmium::Coupled> BuildTop(const std::shared_ptr<AutoCoupled>& mm_coupled) {
        auto comp = BuildFromAutoCoupled(mm_coupled);

        return std::dynamic_pointer_cast<cadmium::Coupled>(comp);
    }

    static std::shared_ptr<cadmium::Component> BuildFromJson(const std::string& model_type, const nlohmann::json& json) {
        if (!json.contains("id")) throw std::bad_exception(); // TODO custom exception

        auto id = json.at("id").get<std::string>();

        if (!cadmium::cbm::Factory::contains(model_type)) throw std::bad_exception(); // TODO custom exception

        return cadmium::cbm::Factory::build(model_type, id, json);
    }

    static void AddFromAutoCoupled(cadmium::Coupled& coupled, const std::shared_ptr<AutoCoupled>& auto_coupled) {
        auto comp = BuildFromAutoCoupled(auto_coupled);
        coupled.addComponent(comp);
    }

    static void AddFromSourced(cadmium::Coupled& coupled, const std::shared_ptr<Sourced>& sourced) {
        for (nlohmann::json &json: sourced->source->Read()) {
            auto comp = BuildFromJson(sourced->model_type, json);
            coupled.addComponent(comp);
        }
    }

    static void AddFromComponent(cadmium::Coupled& coupled, const std::shared_ptr<Component>& mm_component) {
        auto auto_coupled = std::dynamic_pointer_cast<AutoCoupled>(mm_component);
        auto sourced = std::dynamic_pointer_cast<Sourced>(mm_component);

        if (auto_coupled != nullptr) AddFromAutoCoupled(coupled, auto_coupled);

        else if (sourced != nullptr) AddFromSourced(coupled, sourced);

        else throw std::bad_exception(); // TODO custom exceptions
    }
}

#endif //CADMIUM_COUPLER_HPP
