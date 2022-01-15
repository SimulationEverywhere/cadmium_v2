//
// Created by Bruno St-Aubin on 2021-12-24.
//

#ifndef _CADMIUM_WEB_LOGGER_WEB_VIEWER_LOGGER_HPP_
#define _CADMIUM_WEB_LOGGER_WEB_VIEWER_LOGGER_HPP_

#include <fstream>
#include <string>
#include <utility>
#include <cadmium/logger/logger.hpp>
#include <cadmium/core/modeling/coupled.hpp>
#include <cadmium/core/modeling/message.hpp>
#include <cadmium-cbm/tools.hpp>
#include <cadmium-viz/data_structures/structure.hpp>
#include <json.hpp>

namespace cadmium {
    class WebViewerLogger: public cadmium::Logger {
    private:
        std::ofstream o_messages;
        std::ofstream o_structure;
        std::shared_ptr<Coupled> model;
        std::string folder;
        structure::structure * structure = nullptr;

    public:
        explicit WebViewerLogger(std::shared_ptr<Coupled> model, std::string folder): Logger(), model(std::move(model)), folder(std::move(folder)) { }

        void start() override {
            std::string p_messages = cadmium::cbm::tools::path({ folder, "messages.log" });
            std::string p_structure = cadmium::cbm::tools::path({ folder, "structure.json" });

            o_messages.open(p_messages);
            o_structure.open(p_structure);

            structure = structure::structure::from_coupled_model(model);

            o_structure << structure->to_json() << std::endl;
        }

        void stop() override {
            o_messages.close();
            o_structure.close();
        }

        void logOutput(double time, long modelId, const std::string& modelName, const std::string& portName, const std::string& output) override {
            o_messages << modelId << "," << structure->get_port(modelName, portName)->id << ";" << output << std::endl;
        }

        void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {
            o_messages << modelId << ";" << state << std::endl;
        }

        void logTime(double time) override {
            o_messages << time <<  std::endl;
        }
    };
}

#endif //_CADMIUM_WEB_LOGGER_WEB_VIEWER_LOGGER_HPP_
