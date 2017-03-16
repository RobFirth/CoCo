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


#include "CoCo.hpp"

#include <iostream>

#include "../src/vmath/algebra.hpp"
#include "../src/vmath/loadtxt.hpp"
#include "../src/vmath/convert.hpp"

#include "../src/core/utils.hpp"
#include "../src/models/Karpenka12.hpp"
#include "../src/solvers/MPFitter.hpp"


CoCo::CoCo() {
    // TEMP
    filtersDir_ = "/Users/szymon/Projects/CoCo/data/filters";
    reconRoot_ = "/Users/szymon/Projects/CoCo";

    // Initialise shared data structures
    filters_ = std::shared_ptr<Filters>(new Filters(filtersDir_));
    cosmology_ = std::shared_ptr<Cosmology>(new Cosmology(0));

    // Find all phase files and load SN templates
    std::vector<std::string> phaseFiles = utils::dirlist(reconRoot_ + "/recon/");
    for (auto it = phaseFiles.begin(); it != phaseFiles.end(); ) {
        if (utils::split(*it, '.').back() != "phase") {
            it->erase();
        } else {
            std::vector< std::vector<std::string> > phaseFile =
              vmath::loadtxt<std::string>(reconRoot_ + "/recon/" + (*it), 2);

            for (size_t i = 0; i < phaseFile[0].size(); ++i) {
                templateSNe_[utils::split(*it, '.').front()].addSpec(phaseFile[0][i],
                    atof(phaseFile[1][i].c_str()));
                templateSNe_[utils::split(*it, '.').front()].z_ = 0.0;
            }

            it++;
        }
    }
}


void CoCo::simulate() {
    SN sn = templateSNe_[templateName_];

    // Apply host galaxy reddening
    sn.applyReddening(Ebv_Host_, R_v_);

    // Move the spectra to new redshift
    sn.redshift(z_, cosmology_, true);
    sn.moveMJD((1.0 + z_), mjdPeak_);

    // offset absolute magnitude
    sn.scaleSpectra(pow(10, -0.4 * (absMag_)));

    // Apply Milky Way extinction
    sn.applyReddening(Ebv_MW_, 3.1);

    // synthesise LC for every unique filter
    std::vector<std::string> uniqueFilters = filterSim_;
    utils::removeDuplicates(uniqueFilters);
    sn.synthesiseLC(uniqueFilters, filters_);


    for (auto &lc : sn.lc_) {
        // Initialise model
        std::shared_ptr<Karpenka12> karpenka12(new Karpenka12);
        karpenka12->x_ = vmath::sub<double>(lc.second.mjd_, lc.second.mjdMin_);
        karpenka12->y_ = vmath::div<double>(lc.second.flux_, lc.second.normalization_);
        karpenka12->sigma_ = std::vector<double>(lc.second.flux_.size(), 1);
        std::shared_ptr<Model> model = std::dynamic_pointer_cast<Model>(karpenka12);

        // Initialise solver
        MPFitter solver(model);
        std::vector<double> xTemp = mjdRange(lc.second.filter_, mjdSim_, filterSim_);
        solver.xRecon_ = vmath::sub<double>(xTemp, lc.second.mjdMin_);

        // Perform fitting
        solver.analyse();
        solver.xRecon_ = vmath::add<double>(solver.xRecon_, lc.second.mjdMin_);
        solver.bestFit_ = vmath::mult<double>(solver.bestFit_, lc.second.normalization_);
    }
}


// Split mjdSim_ into individual vectors for each filter
std::vector<double> CoCo::mjdRange(std::string flt,
                             const std::vector<double> &mjd,
                             const std::vector<std::string> &filters) {

    std::vector<double> res;
    for (size_t i = 0; i < mjd.size(); ++i) {
        if (filters[i] == flt) {
            res.push_back(mjd[i]);
        }
    }

    return res;
}