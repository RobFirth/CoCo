// CoCo - Supernova templates and simulations package
// Copyright (C) 2016, 2017  Szymon Prajs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// Contact author: S.Prajs@soton.ac.uk

#ifndef COCO_MODELS_SPECMANGLE_HPP_
#define COCO_MODELS_SPECMANGLE_HPP_

#include <vector>
#include "../core/Filters.hpp"
#include "../core/Model.hpp"
#include "../core/SN.hpp"


class SpecMangle : public Model {
public:
    // Variables and methods specific to the model
    std::vector<Obs> lcData_;
    SpecData specData_;
    std::shared_ptr<Filters> filters_;

    // Constructor
    SpecMangle();

    // Override for the mathematical definition of the model
    double function(double);
    std::vector<double> function(std::vector<double>&);
    std::vector<double> residual();

    void setPriors();
};


#endif  // COCO_MODELS_SPECMANGLE_HPP_
