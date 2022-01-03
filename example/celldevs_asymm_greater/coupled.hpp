/**
 * TODO
 * Copyright (C) 2021  Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _CADMIUM_EXAMPLE_CELLDEVS_ASYMM_GREATER_COUPLED_HPP_
#define _CADMIUM_EXAMPLE_CELLDEVS_ASYMM_GREATER_COUPLED_HPP_

#include <cadmium/celldevs/asymm.hpp>
#include <string>
#include "cells/default.hpp"

using namespace cadmium::celldevs;

class GreaterCoupled: public AsymmCellDEVSCoupled<int, int> {
 public:
	GreaterCoupled(const std::string& id, const std::string& configFilePath): AsymmCellDEVSCoupled<int, int>(id, configFilePath) {}

	void addCell(const std::string& cellId, const std::shared_ptr<AsymmCellConfig<int, int>>& cellConfig) override {
		auto cellModel = cellConfig->cellModel;
		if (cellModel == "default") {
			addComponent(DefaultCell(cellId, cellConfig));
		} else {
			throw std::bad_typeid();
		}
	}
};


#endif //_CADMIUM_EXAMPLE_CELLDEVS_ASYMM_GREATER_COUPLED_HPP_
