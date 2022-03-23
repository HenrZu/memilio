/* 
* Copyright (C) 2020-2021 German Aerospace Center (DLR-SC)
*
* Authors: Daniel Abele, Wadim Koslow
*
* Contact: Martin J. Kuehn <Martin.Kuehn@DLR.de>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "load_test_data.h"
#include "test_data_dir.h"
#include "secir/secir.h"
#include "secir/parameter_space.h"
#include "secir/secir_parameters_io.h"
#include "memilio/io/mobility_io.h"
#include "memilio/io/result_io.h"
#include <distributions_helpers.h>
#include <matchers.h>
#include "temp_file_register.h"
#include "memilio/utils/date.h"
#include <gtest/gtest.h>

TEST(TestSaveParameters, json_single_sim_write_read_compare)
{
    double t0   = 0.0;
    double tmax = 50.5;

    double tinc = 5.2, tinfmild = 6, tserint = 4.2, thosp2home = 12, thome2hosp = 5, thosp2icu = 2, ticu2home = 8,
           tinfasy = 6.2, ticu2death = 5;

    double cont_freq = 10, alpha = 0.09, beta = 0.25, delta = 0.3, rho = 0.2, theta = 0.25;

    double num_total_t0 = 10000, num_exp_t0 = 100, num_inf_t0 = 50, num_car_t0 = 50, num_hosp_t0 = 20, num_icu_t0 = 10,
           num_rec_t0 = 10, num_dead_t0 = 0;

    mio::SecirModel model(2);
    mio::AgeGroup num_groups = model.parameters.get_num_groups();
    double fact              = 1.0 / (double)(size_t)num_groups;

    auto& params = model.parameters;

    for (auto i = mio::AgeGroup(0); i < num_groups; i++) {
        model.parameters.get<mio::IncubationTime>()[i]             = tinc;
        model.parameters.get<mio::InfectiousTimeMild>()[i]         = tinfmild;
        model.parameters.get<mio::SerialInterval>()[i]             = tserint;
        model.parameters.get<mio::HospitalizedToHomeTime>()[i]     = thosp2home;
        model.parameters.get<mio::HomeToHospitalizedTime>()[i]     = thome2hosp;
        model.parameters.get<mio::HospitalizedToICUTime>()[i]      = thosp2icu;
        model.parameters.get<mio::ICUToHomeTime>()[i]              = ticu2home;
        model.parameters.get<mio::InfectiousTimeAsymptomatic>()[i] = tinfasy;
        model.parameters.get<mio::ICUToDeathTime>()[i]             = ticu2death;

        model.populations[{i, mio::InfectionState::Exposed}]      = fact * num_exp_t0;
        model.populations[{i, mio::InfectionState::Carrier}]      = fact * num_car_t0;
        model.populations[{i, mio::InfectionState::Infected}]     = fact * num_inf_t0;
        model.populations[{i, mio::InfectionState::Hospitalized}] = fact * num_hosp_t0;
        model.populations[{i, mio::InfectionState::ICU}]          = fact * num_icu_t0;
        model.populations[{i, mio::InfectionState::Recovered}]    = fact * num_rec_t0;
        model.populations[{i, mio::InfectionState::Dead}]         = fact * num_dead_t0;
        model.populations.set_difference_from_group_total<mio::AgeGroup>({i, mio::InfectionState::Susceptible},
                                                                         fact * num_total_t0);

        model.parameters.get<mio::InfectionProbabilityFromContact>()[i] = 0.06;
        model.parameters.get<mio::RelativeCarrierInfectability>()[i]    = 0.67;
        model.parameters.get<mio::AsymptoticCasesPerInfectious>()[i]    = alpha;
        model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[i]   = beta;
        model.parameters.get<mio::HospitalizedCasesPerInfectious>()[i]  = rho;
        model.parameters.get<mio::ICUCasesPerHospitalized>()[i]         = theta;
        model.parameters.get<mio::DeathsPerICU>()[i]                    = delta;
    }

    mio::ContactMatrixGroup& contact_matrix = params.get<mio::ContactPatterns>();
    contact_matrix[0] =
        mio::ContactMatrix(Eigen::MatrixXd::Constant((size_t)num_groups, (size_t)num_groups, fact * cont_freq));
    contact_matrix.add_damping(0.7, mio::SimulationTime(30.));
    auto damping2  = Eigen::MatrixXd::Zero((size_t)num_groups, (size_t)num_groups).eval();
    damping2(0, 0) = 0.8;
    contact_matrix.add_damping(damping2, mio::SimulationTime(35));

    mio::set_params_distributions_normal(model, t0, tmax, 0.2);

    params.get<mio::IncubationTime>()[(mio::AgeGroup)0].get_distribution()->add_predefined_sample(4711.0);

    TempFileRegister file_register;
    auto filename     = file_register.get_unique_path("TestParameters-%%%%-%%%%.json");
    auto write_status = mio::write_json(filename, model);
    ASSERT_THAT(print_wrap(write_status), IsSuccess());

    auto read_result = mio::read_json(filename, mio::Tag<mio::SecirModel>{});
    ASSERT_THAT(print_wrap(read_result), IsSuccess());
    auto& read_model = read_result.value();

    const mio::UncertainContactMatrix& contact      = model.parameters.get<mio::ContactPatterns>();
    const mio::UncertainContactMatrix& read_contact = read_model.parameters.get<mio::ContactPatterns>();

    num_groups           = model.parameters.get_num_groups();
    auto num_groups_read = read_model.parameters.get_num_groups();
    ASSERT_EQ(num_groups, num_groups_read);

    for (auto i = mio::AgeGroup(0); i < num_groups; i++) {
        ASSERT_EQ((model.populations[{i, mio::InfectionState::Dead}]),
                  (read_model.populations[{i, mio::InfectionState::Dead}]));
        ASSERT_EQ((model.populations.get_group_total(i)), (read_model.populations.get_group_total(i)));
        ASSERT_EQ((model.populations[{i, mio::InfectionState::Exposed}]),
                  (read_model.populations[{i, mio::InfectionState::Exposed}]));
        ASSERT_EQ((model.populations[{i, mio::InfectionState::Carrier}]),
                  (read_model.populations[{i, mio::InfectionState::Carrier}]));
        ASSERT_EQ((model.populations[{i, mio::InfectionState::Infected}]),
                  (read_model.populations[{i, mio::InfectionState::Infected}]));
        ASSERT_EQ((model.populations[{i, mio::InfectionState::Hospitalized}]),
                  (read_model.populations[{i, mio::InfectionState::Hospitalized}]));
        ASSERT_EQ((model.populations[{i, mio::InfectionState::ICU}]),
                  (read_model.populations[{i, mio::InfectionState::ICU}]));
        ASSERT_EQ((model.populations[{i, mio::InfectionState::Recovered}]),
                  (read_model.populations[{i, mio::InfectionState::Recovered}]));

        check_distribution(*model.populations[{i, mio::InfectionState::Exposed}].get_distribution(),
                           *read_model.populations[{i, mio::InfectionState::Exposed}].get_distribution());
        check_distribution(*model.populations[{i, mio::InfectionState::Carrier}].get_distribution(),
                           *read_model.populations[{i, mio::InfectionState::Carrier}].get_distribution());
        check_distribution(*model.populations[{i, mio::InfectionState::Infected}].get_distribution(),
                           *read_model.populations[{i, mio::InfectionState::Infected}].get_distribution());
        check_distribution(*model.populations[{i, mio::InfectionState::Hospitalized}].get_distribution(),
                           *read_model.populations[{i, mio::InfectionState::Hospitalized}].get_distribution());
        check_distribution(*model.populations[{i, mio::InfectionState::ICU}].get_distribution(),
                           *read_model.populations[{i, mio::InfectionState::ICU}].get_distribution());
        check_distribution(*model.populations[{i, mio::InfectionState::Recovered}].get_distribution(),
                           *read_model.populations[{i, mio::InfectionState::Recovered}].get_distribution());

        ASSERT_EQ(model.parameters.get<mio::IncubationTime>()[i], read_model.parameters.get<mio::IncubationTime>()[i]);
        ASSERT_EQ(model.parameters.get<mio::InfectiousTimeMild>()[i],
                  read_model.parameters.get<mio::InfectiousTimeMild>()[i]);
        ASSERT_EQ(model.parameters.get<mio::SerialInterval>()[i], read_model.parameters.get<mio::SerialInterval>()[i]);
        ASSERT_EQ(model.parameters.get<mio::HospitalizedToHomeTime>()[i],
                  read_model.parameters.get<mio::HospitalizedToHomeTime>()[i]);
        ASSERT_EQ(model.parameters.get<mio::HomeToHospitalizedTime>()[i],
                  read_model.parameters.get<mio::HomeToHospitalizedTime>()[i]);
        ASSERT_EQ(model.parameters.get<mio::InfectiousTimeAsymptomatic>()[i],
                  read_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[i]);
        ASSERT_EQ(model.parameters.get<mio::HospitalizedToICUTime>()[i],
                  read_model.parameters.get<mio::HospitalizedToICUTime>()[i]);
        ASSERT_EQ(model.parameters.get<mio::ICUToHomeTime>()[i], read_model.parameters.get<mio::ICUToHomeTime>()[i]);
        ASSERT_EQ(model.parameters.get<mio::ICUToDeathTime>()[i], read_model.parameters.get<mio::ICUToDeathTime>()[i]);

        check_distribution(*model.parameters.get<mio::IncubationTime>()[i].get_distribution(),
                           *read_model.parameters.get<mio::IncubationTime>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::InfectiousTimeMild>()[i].get_distribution(),
                           *read_model.parameters.get<mio::InfectiousTimeMild>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::SerialInterval>()[i].get_distribution(),
                           *read_model.parameters.get<mio::SerialInterval>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::HospitalizedToHomeTime>()[i].get_distribution(),
                           *read_model.parameters.get<mio::HospitalizedToHomeTime>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::HomeToHospitalizedTime>()[i].get_distribution(),
                           *read_model.parameters.get<mio::HomeToHospitalizedTime>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::InfectiousTimeAsymptomatic>()[i].get_distribution(),
                           *read_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::HospitalizedToICUTime>()[i].get_distribution(),
                           *read_model.parameters.get<mio::HospitalizedToICUTime>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::ICUToHomeTime>()[i].get_distribution(),
                           *read_model.parameters.get<mio::ICUToHomeTime>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::ICUToDeathTime>()[i].get_distribution(),
                           *read_model.parameters.get<mio::ICUToDeathTime>()[i].get_distribution());

        ASSERT_EQ(model.parameters.get<mio::InfectionProbabilityFromContact>()[i],
                  read_model.parameters.get<mio::InfectionProbabilityFromContact>()[i]);
        ASSERT_EQ(model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[i],
                  read_model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[i]);
        ASSERT_EQ(model.parameters.get<mio::AsymptoticCasesPerInfectious>()[i],
                  read_model.parameters.get<mio::AsymptoticCasesPerInfectious>()[i]);
        ASSERT_EQ(model.parameters.get<mio::DeathsPerICU>()[i], read_model.parameters.get<mio::DeathsPerICU>()[i]);
        ASSERT_EQ(model.parameters.get<mio::HospitalizedCasesPerInfectious>()[i],
                  read_model.parameters.get<mio::HospitalizedCasesPerInfectious>()[i]);
        ASSERT_EQ(model.parameters.get<mio::ICUCasesPerHospitalized>()[i],
                  read_model.parameters.get<mio::ICUCasesPerHospitalized>()[i]);

        check_distribution(*model.parameters.get<mio::InfectionProbabilityFromContact>()[i].get_distribution(),
                           *read_model.parameters.get<mio::InfectionProbabilityFromContact>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[i].get_distribution(),
                           *read_model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::AsymptoticCasesPerInfectious>()[i].get_distribution(),
                           *read_model.parameters.get<mio::AsymptoticCasesPerInfectious>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::DeathsPerICU>()[i].get_distribution(),
                           *read_model.parameters.get<mio::DeathsPerICU>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::HospitalizedCasesPerInfectious>()[i].get_distribution(),
                           *read_model.parameters.get<mio::HospitalizedCasesPerInfectious>()[i].get_distribution());
        check_distribution(*model.parameters.get<mio::ICUCasesPerHospitalized>()[i].get_distribution(),
                           *read_model.parameters.get<mio::ICUCasesPerHospitalized>()[i].get_distribution());

        ASSERT_THAT(contact.get_cont_freq_mat(), testing::ContainerEq(read_contact.get_cont_freq_mat()));
        ASSERT_EQ(contact.get_dampings(), read_contact.get_dampings());
    }
}

TEST(TestSaveParameters, json_graphs_write_read_compare)
{
    double t0   = 0.0;
    double tmax = 50.5;

    double tinc = 5.2, tinfmild = 6, tserint = 4.2, thosp2home = 12, thome2hosp = 5, thosp2icu = 2, ticu2home = 8,
           tinfasy = 6.2, ticu2death = 5;

    double cont_freq = 10, alpha = 0.09, beta = 0.25, delta = 0.3, rho = 0.2, theta = 0.25;

    double num_total_t0 = 10000, num_exp_t0 = 100, num_inf_t0 = 50, num_car_t0 = 50, num_hosp_t0 = 20, num_icu_t0 = 10,
           num_rec_t0 = 10, num_dead_t0 = 0;

    mio::SecirModel model(2);
    mio::AgeGroup num_groups = model.parameters.get_num_groups();
    double fact              = 1.0 / (double)(size_t)num_groups;

    model.parameters.set<mio::TestAndTraceCapacity>(30);

    for (auto i = mio::AgeGroup(0); i < num_groups; i++) {
        model.parameters.get<mio::IncubationTime>()[i]             = tinc;
        model.parameters.get<mio::InfectiousTimeMild>()[i]         = tinfmild;
        model.parameters.get<mio::SerialInterval>()[i]             = tserint;
        model.parameters.get<mio::HospitalizedToHomeTime>()[i]     = thosp2home;
        model.parameters.get<mio::HomeToHospitalizedTime>()[i]     = thome2hosp;
        model.parameters.get<mio::HospitalizedToICUTime>()[i]      = thosp2icu;
        model.parameters.get<mio::ICUToHomeTime>()[i]              = ticu2home;
        model.parameters.get<mio::InfectiousTimeAsymptomatic>()[i] = tinfasy;
        model.parameters.get<mio::ICUToDeathTime>()[i]             = ticu2death;

        model.populations[{i, mio::InfectionState::Exposed}]      = fact * num_exp_t0;
        model.populations[{i, mio::InfectionState::Carrier}]      = fact * num_car_t0;
        model.populations[{i, mio::InfectionState::Infected}]     = fact * num_inf_t0;
        model.populations[{i, mio::InfectionState::Hospitalized}] = fact * num_hosp_t0;
        model.populations[{i, mio::InfectionState::ICU}]          = fact * num_icu_t0;
        model.populations[{i, mio::InfectionState::Recovered}]    = fact * num_rec_t0;
        model.populations[{i, mio::InfectionState::Dead}]         = fact * num_dead_t0;
        model.populations.set_difference_from_group_total<mio::AgeGroup>({i, mio::InfectionState::Susceptible},
                                                                         fact * num_total_t0);

        model.parameters.get<mio::InfectionProbabilityFromContact>()[i]  = 0.06;
        model.parameters.get<mio::RelativeCarrierInfectability>()[i]     = 0.67;
        model.parameters.get<mio::AsymptoticCasesPerInfectious>()[i]     = alpha;
        model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[i]    = beta;
        model.parameters.get<mio::MaxRiskOfInfectionFromSympomatic>()[i] = beta * 3;
        model.parameters.get<mio::HospitalizedCasesPerInfectious>()[i]   = rho;
        model.parameters.get<mio::ICUCasesPerHospitalized>()[i]          = theta;
        model.parameters.get<mio::DeathsPerICU>()[i]                     = delta;
    }

    mio::ContactMatrixGroup& contact_matrix = model.parameters.get<mio::ContactPatterns>();
    contact_matrix[0] =
        mio::ContactMatrix(Eigen::MatrixXd::Constant((size_t)num_groups, (size_t)num_groups, fact * cont_freq));
    Eigen::MatrixXd m =
        Eigen::MatrixXd::Constant((size_t)num_groups, (size_t)num_groups, 0.7).triangularView<Eigen::Upper>();
    contact_matrix.add_damping(m, mio::SimulationTime(30.));

    mio::set_params_distributions_normal(model, t0, tmax, 0.15);

    mio::Graph<mio::SecirModel, mio::MigrationParameters> graph;
    graph.add_node(0, model);
    graph.add_node(1, model);
    graph.add_edge(0, 1, Eigen::VectorXd::Constant(model.populations.get_num_compartments(), 0.01));
    graph.add_edge(1, 0, Eigen::VectorXd::Constant(model.populations.get_num_compartments(), 0.01));

    TempFileRegister file_register;
    auto graph_dir    = file_register.get_unique_path("graph_parameters-%%%%-%%%%");
    auto write_status = mio::write_graph(graph, graph_dir);
    ASSERT_THAT(print_wrap(write_status), IsSuccess());

    auto read_result = mio::read_graph<mio::SecirModel>(graph_dir);
    ASSERT_THAT(print_wrap(read_result), IsSuccess());

    auto& graph_read = read_result.value();
    auto num_nodes   = graph.nodes().size();
    auto num_edges   = graph.edges().size();

    ASSERT_EQ(num_nodes, graph_read.nodes().size());
    ASSERT_EQ(num_edges, graph_read.edges().size());

    for (size_t node = 0; node < num_nodes; node++) {
        mio::SecirModel graph_model                = graph.nodes()[0].property;
        mio::ContactMatrixGroup& graph_cont_matrix = graph_model.parameters.get<mio::ContactPatterns>();

        mio::SecirModel graph_read_model                = graph_read.nodes()[0].property;
        mio::ContactMatrixGroup& graph_read_cont_matrix = graph_read_model.parameters.get<mio::ContactPatterns>();

        ASSERT_EQ(graph_read_cont_matrix.get_num_groups(), static_cast<Eigen::Index>((size_t)num_groups));
        ASSERT_EQ(graph_read_cont_matrix, graph_cont_matrix);
        ASSERT_EQ(graph_model.populations.get_num_compartments(), graph_read_model.populations.get_num_compartments());
        ASSERT_EQ(graph.nodes()[node].id, graph_read.nodes()[node].id);
        EXPECT_THAT(graph_read_model.parameters.get<mio::TestAndTraceCapacity>().value(),
                    FloatingPointEqual(graph_model.parameters.get<mio::TestAndTraceCapacity>().value(), 1e-12, 1e-12));
        check_distribution(*graph_model.parameters.get<mio::TestAndTraceCapacity>().get_distribution().get(),
                           *graph_read_model.parameters.get<mio::TestAndTraceCapacity>().get_distribution().get());

        for (auto group = mio::AgeGroup(0); group < mio::AgeGroup(num_groups); group++) {
            ASSERT_EQ((graph_model.populations[{group, mio::InfectionState::Dead}]),
                      (graph_read_model.populations[{group, mio::InfectionState::Dead}]));
            ASSERT_EQ(graph_model.populations.get_total(), graph_read_model.populations.get_total());
            check_distribution(
                *graph_model.populations[{group, mio::InfectionState::Exposed}].get_distribution().get(),
                *graph_read_model.populations[{group, mio::InfectionState::Exposed}].get_distribution().get());
            check_distribution(
                *graph_model.populations[{group, mio::InfectionState::Carrier}].get_distribution().get(),
                *graph_read_model.populations[{group, mio::InfectionState::Carrier}].get_distribution().get());
            check_distribution(
                *graph_model.populations[{group, mio::InfectionState::Infected}].get_distribution().get(),
                *graph_read_model.populations[{group, mio::InfectionState::Infected}].get_distribution().get());
            check_distribution(
                *graph_model.populations[{group, mio::InfectionState::Hospitalized}].get_distribution().get(),
                *graph_read_model.populations[{group, mio::InfectionState::Hospitalized}].get_distribution().get());
            check_distribution(
                *graph_model.populations[{group, mio::InfectionState::ICU}].get_distribution().get(),
                *graph_read_model.populations[{group, mio::InfectionState::ICU}].get_distribution().get());
            check_distribution(
                *graph_model.populations[{group, mio::InfectionState::Recovered}].get_distribution().get(),
                *graph_read_model.populations[{group, mio::InfectionState::Recovered}].get_distribution().get());
            check_distribution(
                *graph_model.populations[{group, mio::InfectionState::Exposed}].get_distribution().get(),
                *graph_read_model.populations[{group, mio::InfectionState::Exposed}].get_distribution().get());

            ASSERT_EQ(graph_model.parameters.get<mio::IncubationTime>()[group],
                      graph_read_model.parameters.get<mio::IncubationTime>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::InfectiousTimeMild>()[group],
                      graph_read_model.parameters.get<mio::InfectiousTimeMild>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::SerialInterval>()[group],
                      graph_read_model.parameters.get<mio::SerialInterval>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::HospitalizedToHomeTime>()[group],
                      graph_read_model.parameters.get<mio::HospitalizedToHomeTime>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::HomeToHospitalizedTime>()[group],
                      graph_read_model.parameters.get<mio::HomeToHospitalizedTime>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[group],
                      graph_read_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::HospitalizedToICUTime>()[group],
                      graph_read_model.parameters.get<mio::HospitalizedToICUTime>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::ICUToHomeTime>()[group],
                      graph_read_model.parameters.get<mio::ICUToHomeTime>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::ICUToDeathTime>()[group],
                      graph_read_model.parameters.get<mio::ICUToDeathTime>()[group]);

            ASSERT_EQ(graph_model.parameters.get<mio::InfectionProbabilityFromContact>()[group],
                      graph_read_model.parameters.get<mio::InfectionProbabilityFromContact>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[group],
                      graph_read_model.parameters.get<mio::RiskOfInfectionFromSympomatic>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::MaxRiskOfInfectionFromSympomatic>()[group],
                      graph_read_model.parameters.get<mio::MaxRiskOfInfectionFromSympomatic>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::AsymptoticCasesPerInfectious>()[group],
                      graph_read_model.parameters.get<mio::AsymptoticCasesPerInfectious>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::DeathsPerICU>()[group],
                      graph_read_model.parameters.get<mio::DeathsPerICU>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::HospitalizedCasesPerInfectious>()[group],
                      graph_read_model.parameters.get<mio::HospitalizedCasesPerInfectious>()[group]);
            ASSERT_EQ(graph_model.parameters.get<mio::ICUCasesPerHospitalized>()[group],
                      graph_read_model.parameters.get<mio::ICUCasesPerHospitalized>()[group]);

            check_distribution(*graph_model.parameters.get<mio::IncubationTime>()[group].get_distribution().get(),
                               *graph_read_model.parameters.get<mio::IncubationTime>()[group].get_distribution().get());
            check_distribution(*graph_model.parameters.get<mio::SerialInterval>()[group].get_distribution().get(),
                               *graph_read_model.parameters.get<mio::SerialInterval>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::InfectiousTimeMild>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::InfectiousTimeMild>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::HospitalizedToHomeTime>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::HospitalizedToHomeTime>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::HomeToHospitalizedTime>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::HomeToHospitalizedTime>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::HospitalizedToICUTime>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::HospitalizedToICUTime>()[group].get_distribution().get());
            check_distribution(*graph_model.parameters.get<mio::ICUToHomeTime>()[group].get_distribution().get(),
                               *graph_read_model.parameters.get<mio::ICUToHomeTime>()[group].get_distribution().get());
            check_distribution(*graph_model.parameters.get<mio::ICUToDeathTime>()[group].get_distribution().get(),
                               *graph_read_model.parameters.get<mio::ICUToDeathTime>()[group].get_distribution().get());

            check_distribution(
                *graph_model.parameters.get<mio::InfectiousTimeMild>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::InfectiousTimeMild>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::HospitalizedToHomeTime>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::HospitalizedToHomeTime>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::HomeToHospitalizedTime>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::HomeToHospitalizedTime>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::MaxRiskOfInfectionFromSympomatic>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::MaxRiskOfInfectionFromSympomatic>()[group]
                     .get_distribution()
                     .get());
            check_distribution(*graph_model.parameters.get<mio::DeathsPerICU>()[group].get_distribution().get(),
                               *graph_read_model.parameters.get<mio::DeathsPerICU>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::InfectiousTimeAsymptomatic>()[group].get_distribution().get());
            check_distribution(
                *graph_model.parameters.get<mio::ICUCasesPerHospitalized>()[group].get_distribution().get(),
                *graph_read_model.parameters.get<mio::ICUCasesPerHospitalized>()[group].get_distribution().get());

            ASSERT_EQ(graph_model.parameters.get<mio::ContactPatterns>().get_dampings(),
                      graph_read_model.parameters.get<mio::ContactPatterns>().get_dampings());
        }

        ASSERT_THAT(graph_read.edges(), testing::ElementsAreArray(graph.edges()));
    }
}

TEST(TestSaveParameters, ReadPopulationDataRKIAges)
{
    std::vector<mio::SecirModel> model(1, {6});
    model[0].apply_constraints();
    std::vector<double> scaling_factor_inf(6, 1.0);
    double scaling_factor_icu = 1.0;
    mio::Date date(2020, 12, 10);

    std::string path = TEST_DATA_DIR;

    for (auto group = mio::AgeGroup(0); group < mio::AgeGroup(6); group++) {
        model[0].parameters.get<mio::AsymptoticCasesPerInfectious>()[group]   = 0.1 * ((size_t)group + 1);
        model[0].parameters.get<mio::HospitalizedCasesPerInfectious>()[group] = 0.11 * ((size_t)group + 1);
        model[0].parameters.get<mio::ICUCasesPerHospitalized>()[group]        = 0.12 * ((size_t)group + 1);
    }
    auto read_result = mio::read_population_data_germany(model, date, scaling_factor_inf, scaling_factor_icu, path);
    ASSERT_THAT(print_wrap(read_result), IsSuccess());

    std::vector<double> sus   = {3444023.09, 7666389.350, 18801939.83, 29522450.59, 16317865.95, 6059469.35};
    std::vector<double> exp   = {389.843, 1417.37, 6171.74, 8765.6, 3554.5, 2573.89};
    std::vector<double> car   = {389.443, 1412.86, 6077.14, 8554.77, 3437.57, 2462.09};
    std::vector<double> inf   = {297.924, 811.551, 2270.16, 1442.03, 0, 0};
    std::vector<double> hosp  = {39.9614, 303.191, 1934.84, 3621.2, 1793.39, 1557.03};
    std::vector<double> icu   = {47.6813, 190.725, 429.132, 762.901, 1192.03, 1716.53};
    std::vector<double> rec   = {23557.7, 78946.3, 398585.142, 487273.71, 178660.14, 96021.9};
    std::vector<double> death = {2, 4, 48, 1137.86, 8174.14, 18528.9};

    for (size_t i = 0; i < 6; i++) {
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Susceptible}]), sus[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Exposed}]), exp[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Carrier}]), car[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Infected}]), inf[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Hospitalized}]), hosp[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::ICU}]), icu[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Recovered}]), rec[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Dead}]), death[i], 1e-1);
    }

    EXPECT_NEAR(model[0].populations.get_total(), 83166695, 1e-6);
}

TEST(TestSaveParameters, ReadPopulationDataStateAllAges)
{
    std::vector<mio::SecirModel> model(1, {6});
    model[0].apply_constraints();
    std::vector<double> scaling_factor_inf(6, 1.0);
    double scaling_factor_icu = 1.0;
    mio::Date date(2020, 12, 10);

    std::vector<int> state = {1};

    std::string path = TEST_DATA_DIR;

    for (auto group = mio::AgeGroup(0); group < mio::AgeGroup(6); group++) {
        model[0].parameters.get<mio::AsymptoticCasesPerInfectious>()[group]   = 0.1 * ((size_t)group + 1);
        model[0].parameters.get<mio::HospitalizedCasesPerInfectious>()[group] = 0.11 * ((size_t)group + 1);
        model[0].parameters.get<mio::ICUCasesPerHospitalized>()[group]        = 0.12 * ((size_t)group + 1);
    }
    auto read_result =
        mio::read_population_data_state(model, date, state, scaling_factor_inf, scaling_factor_icu, path);
    ASSERT_THAT(print_wrap(read_result), IsSuccess());

    std::vector<double> sus   = {116695.3, 283933, 622945.61, 1042462.09, 606578.8, 212990};
    std::vector<double> exp   = {7.64286, 23.7143, 103.243, 134.486, 43, 38};
    std::vector<double> car   = {7, 20.4286, 99.4143, 126.971, 41.6429, 36.4286};
    std::vector<double> inf   = {5.59286, 11.0429, 37.7571, 22.6629, 0.0785714, 0};
    std::vector<double> hosp  = {0.707143, 3.92857, 30.6429, 50.5371, 20.35, 19.9886};
    std::vector<double> icu   = {0.274725, 1.0989, 2.47253, 4.3956, 6.86813, 9.89011};
    std::vector<double> rec   = {393.143, 1216.14, 5467.86, 6543.57, 2281.29, 1045.71};
    std::vector<double> death = {0, 0, 0, 16.2857, 99.5714, 198.286};

    for (size_t i = 0; i < 6; i++) {
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Susceptible}]), sus[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Exposed}]), exp[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Carrier}]), car[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Infected}]), inf[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Hospitalized}]), hosp[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::ICU}]), icu[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Recovered}]), rec[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Dead}]), death[i], 1e-1);
    }

    EXPECT_NEAR(model[0].populations.get_total(), 2903777, 1e-6);
}

TEST(TestSaveParameters, ReadPopulationDataCountyAllAges)
{

    std::vector<mio::SecirModel> model(1, {6});
    model[0].apply_constraints();
    std::vector<double> scaling_factor_inf(6, 1.0);
    double scaling_factor_icu = 1.0;
    mio::Date date(2020, 12, 10);

    std::vector<int> county = {1002};

    std::string path = TEST_DATA_DIR;

    for (auto group = mio::AgeGroup(0); group < mio::AgeGroup(6); group++) {
        model[0].parameters.get<mio::AsymptoticCasesPerInfectious>()[group]   = 0.1 * ((size_t)group + 1);
        model[0].parameters.get<mio::HospitalizedCasesPerInfectious>()[group] = 0.11 * ((size_t)group + 1);
        model[0].parameters.get<mio::ICUCasesPerHospitalized>()[group]        = 0.12 * ((size_t)group + 1);
    }
    auto read_result =
        mio::read_population_data_county(model, date, county, scaling_factor_inf, scaling_factor_icu, path);
    ASSERT_THAT(print_wrap(read_result), IsSuccess());

    std::vector<double> sus   = {10284.4, 19086.2, 73805.3, 82522.6, 43731.9, 15620.2};
    std::vector<double> exp   = {0.571429, 3.8, 14.8286, 12.9429, 2.21429, 1.85714};
    std::vector<double> car   = {0.557143, 3.51429, 15.3857, 12.6571, 2.28571, 1.94286};
    std::vector<double> inf   = {0.291429, 1.93714, 5.79714, 2.45714, 0, 0};
    std::vector<double> hosp  = {0.0942857, 0.691429, 4.90286, 5.34286, 1.41429, 2.45143};
    std::vector<double> icu   = {0.0769231, 0.307692, 0.692308, 1.23077, 1.92308, 2.76923};
    std::vector<double> rec   = {35, 108.571, 640.143, 573.429, 180.429, 75.5714};
    std::vector<double> death = {0, 0, 0, 0, 10, 14.4286};

    for (size_t i = 0; i < 6; i++) {
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Susceptible}]), sus[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Exposed}]), exp[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Carrier}]), car[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Infected}]), inf[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Hospitalized}]), hosp[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::ICU}]), icu[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Recovered}]), rec[i], 1e-1);
        EXPECT_NEAR((model[0].populations[{mio::AgeGroup(i), mio::InfectionState::Dead}]), death[i], 1e-1);
    }

    EXPECT_NEAR(model[0].populations.get_total(), 246793, 1e-6);
}

TEST(TestSaveParameters, ExtrapolateRKI)
{
    std::vector<mio::SecirModel> model{mio::SecirModel(6)};

    model[0].apply_constraints();
    std::vector<double> scaling_factor_inf(6, 1.0);
    double scaling_factor_icu = 1.0;
    mio::Date date(2020, 12, 10);

    std::vector<int> county = {1002};

    for (auto group = mio::AgeGroup(0); group < mio::AgeGroup(6); group++) {
        model[0].parameters.get<mio::AsymptoticCasesPerInfectious>()[group]   = 0.1 * ((size_t)group + 1);
        model[0].parameters.get<mio::HospitalizedCasesPerInfectious>()[group] = 0.11 * ((size_t)group + 1);
        model[0].parameters.get<mio::ICUCasesPerHospitalized>()[group]        = 0.12 * ((size_t)group + 1);
    }

    TempFileRegister file_register;
    auto results_dir = file_register.get_unique_path("ExtrapolateRKI-%%%%-%%%%");
    boost::filesystem::create_directory(results_dir);
    auto extrapolate_result = mio::extrapolate_rki_results(model, TEST_DATA_DIR, results_dir, county, date,
                                                           scaling_factor_inf, scaling_factor_icu, 1);
    ASSERT_THAT(print_wrap(extrapolate_result), IsSuccess());

    auto read_result = mio::read_result(mio::path_join(results_dir, "Results_rki.h5"));
    ASSERT_THAT(print_wrap(read_result), IsSuccess());
    auto& file_results = read_result.value();
    auto results       = file_results[0].get_groups();

    std::vector<double> sus   = {10284.4, 19086.2, 73805.3, 82522.6, 43731.9, 15620.2};
    std::vector<double> exp   = {0.571429, 3.8, 14.8286, 12.9429, 2.21429, 1.85714};
    std::vector<double> car   = {0.557143, 3.51429, 15.3857, 12.6571, 2.28571, 1.94286};
    std::vector<double> inf   = {0.291429, 1.93714, 5.79714, 2.45714, 0, 0};
    std::vector<double> hosp  = {0.0942857, 0.691429, 4.90286, 5.34286, 1.41429, 2.45143};
    std::vector<double> icu   = {0.0769231, 0.307692, 0.692308, 1.23077, 1.92308, 2.76923};
    std::vector<double> rec   = {35, 108.571, 640.143, 573.429, 180.429, 75.5714};
    std::vector<double> death = {0, 0, 0, 0, 10, 14.4286};

    for (size_t i = 0; i < 6; i++) {
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::Susceptible + (size_t)mio::InfectionState::Count * i),
                    sus[i], 1e-1);
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::Exposed + (size_t)mio::InfectionState::Count * i), exp[i],
                    1e-1);
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::Carrier + (size_t)mio::InfectionState::Count * i), car[i],
                    1e-1);
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::Infected + (size_t)mio::InfectionState::Count * i), inf[i],
                    1e-1);
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::Hospitalized + (size_t)mio::InfectionState::Count * i),
                    hosp[i], 1e-1);
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::ICU + (size_t)mio::InfectionState::Count * i), icu[i],
                    1e-1);
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::Recovered + (size_t)mio::InfectionState::Count * i), rec[i],
                    1e-1);
        EXPECT_NEAR(results[0]((size_t)mio::InfectionState::Dead + (size_t)mio::InfectionState::Count * i), death[i],
                    1e-1);
    }
}
