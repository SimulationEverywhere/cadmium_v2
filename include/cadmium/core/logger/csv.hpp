/**
 * CSV logger.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */


#ifndef CADMIUM_CORE_LOGGER_CSV_LOGGER_HPP_
#define CADMIUM_CORE_LOGGER_CSV_LOGGER_HPP_

#include <fstream>
#include <string>
#include <utility>
#include "logger.hpp"

namespace cadmium {
    //! Cadmium CSV logger class.
    class CSVLogger: public Logger {
     private:
        std::string filepath;  //!< Path to the CSV file.
        std::string sep;       //!< String used as column separation.
        std::ofstream file;    //!< output file stream.
     public:
        /**
         * Constructor function.
         * @param filepath path to the CSV file.
         * @param sep string used as column separation.
         */
        CSVLogger(std::string filepath, std::string sep): Logger(), filepath(std::move(filepath)), sep(std::move(sep)), file() {}

        /**
         * Constructor function. Separation is set to ",".
         * @param filepath path to the CSV file.
         */
        explicit CSVLogger(std::string filepath): CSVLogger(std::move(filepath), ",") {}

        //! It starts the output file stream and prints the CSV header.
        void start() override {
            file.open(filepath);
            file << "time" << sep << "model_id" << sep << "model_name" << sep << "port_name" << sep << "data" << std::endl;
        }

        //! It closes the output file after the simulation.
        void stop() override {
            file.close();
        }

        /**
         * Virtual method to log atomic models' output messages.
         * @param time current simulation time.
         * @param modelId ID of the model that generated the output message.
         * @param modelName name of the model that generated the output message.
         * @param portName name of the model port in which the output message was created.
         * @param output string representation of the output message.
         */
        void logOutput(double time, long modelId, const std::string& modelName, const std::string& portName, const std::string& output) override {
            file << time << sep << modelId << sep << modelName << sep << portName << sep << output << std::endl;
        }

        /**
         * Virtual method to log atomic models' states.
         * @param time current simulation time.
         * @param modelId ID of the model that generated the output message.
         * @param modelName name of the model that generated the output message.
         * @param state string representation of the state.
         */
        void logState(double time, long modelId, const std::string& modelName, const std::string& state) override {
            file << time << sep << modelId << sep << modelName << sep << sep << state << std::endl;
        }
    };
}

#endif //CADMIUM_CORE_LOGGER_CSV_LOGGER_HPP_
