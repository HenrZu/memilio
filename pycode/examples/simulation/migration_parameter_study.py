#############################################################################
# Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
#
# Authors: 
#
# Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#############################################################################
import memilio.simulation as mio
import memilio.simulation.secir as secir
import numpy as np
import argparse

def parameter_study():
    mio.set_log_level(mio.LogLevel.Warning)

    #setup basic parameters
    model = secir.SecirModel(1)

    model.parameters.IncubationTime[secir.AgeGroup(0)] = 5.2
    model.parameters.SerialInterval[secir.AgeGroup(0)] = 4.2
    model.parameters.InfectiousTimeMild[secir.AgeGroup(0)] = 6
    model.parameters.HomeToHospitalizedTime[secir.AgeGroup(0)] = 5
    model.parameters.HospitalizedToHomeTime[secir.AgeGroup(0)] = 12
    model.parameters.HospitalizedToICUTime[secir.AgeGroup(0)] = 2
    model.parameters.ICUToHomeTime[secir.AgeGroup(0)] = 8
    model.parameters.ICUToDeathTime[secir.AgeGroup(0)] = 5

    model.parameters.ContactPatterns.cont_freq_mat[0].baseline = np.r_[0.5]
    model.parameters.ContactPatterns.cont_freq_mat[0].add_damping(mio.Damping(np.r_[0.3], t = 0.3))

    model.parameters.InfectionProbabilityFromContact[secir.AgeGroup(0)] = 1.0
    model.parameters.RelativeCarrierInfectability[secir.AgeGroup(0)] = 0.67
    model.parameters.AsymptoticCasesPerInfectious[secir.AgeGroup(0)] = 0.09
    model.parameters.RiskOfInfectionFromSympomatic[secir.AgeGroup(0)] = 0.25
    model.parameters.HospitalizedCasesPerInfectious[secir.AgeGroup(0)] = 0.2
    model.parameters.ICUCasesPerHospitalized[secir.AgeGroup(0)] = 0.25
    model.parameters.DeathsPerICU[secir.AgeGroup(0)] = 0.3

    #two regions with different populations and with some migration between them
    graph = secir.SecirModelGraph()
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Exposed)] = 100
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Carrier)] = 50
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Infected)] = 50
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Hospitalized)] = 20
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.ICU)] = 10 
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Recovered)] = 10
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Dead)] = 0
    model.populations.set_difference_from_group_total_AgeGroup(secir.Index_Agegroup_InfectionState(
        secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Susceptible)), 10000)
    model.apply_constraints()
    graph.add_node(id = 0, model = model) #copies the model into the graph
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Exposed)] = 0
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Carrier)] = 0
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Infected)] = 0
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Hospitalized)] = 0
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.ICU)] = 0
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Recovered)] = 0
    model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Dead)] = 0
    model.populations.set_difference_from_group_total_AgeGroup(secir.Index_Agegroup_InfectionState(
        secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Susceptible)), 2000)
    model.apply_constraints()
    graph.add_node(id = 1, model = model)
    migration_coefficients = 0.1 * np.ones(8)
    migration_params = mio.MigrationParameters(migration_coefficients)
    graph.add_edge(0, 1, migration_params) #one coefficient per (age group x compartment)
    graph.add_edge(1, 0, migration_params) #directed graph -> add both directions so coefficients can be different

    #process the result of one run
    def handle_result(graph):
        print('run {}'.format(handle_result.c))
        handle_result.c = handle_result.c + 1
        for node_idx in range(graph.num_nodes):
            node = graph.get_node(node_idx)
            result = node.property.result
            model = node.property.model
            print("  node {}".format(node_idx))
            print("  initial carrier count {}.".format(model.populations[secir.AgeGroup(0), secir.Index_InfectionState(secir.InfectionState.Carrier)].value))
            print("  compartments at t = {}:".format(result.get_time(0)))
            print("  ", result.get_value(0))
            print("  compartments at t = {}:".format(result.get_last_time()))
            print("  ", result.get_last_value())
    handle_result.c = 0

    #study with unknown number of undetected carriers
    carrier_distribution = mio.ParameterDistributionNormal(50, 2000, 200, 100)
    graph.get_node(0).property.populations[secir.AgeGroup(0), secir.Index_InfectionState(
        secir.InfectionState.Carrier)].set_distribution(carrier_distribution)
    
    t0 = 0
    tmax = 50
    study = secir.ParameterStudy(graph, t0, tmax, dt = 1.0, num_runs = 3)
    study.run(handle_result)

if __name__ == "__main__":
    arg_parser = argparse.ArgumentParser(
        'migration_parameter_study', 
        description = 'Example demonstrating setup of ensemble runs of a geographically resolved SECIR model with travel.')
    args = arg_parser.parse_args()
    parameter_study()
