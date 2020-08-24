#include <epidemiology/parameter_studies/parameter_space.h>
#include <epidemiology/parameter_studies/parameter_studies.h>
#include <epidemiology/secir.h>
#include <gtest/gtest.h>
#include <stdio.h>

TEST(ParameterStudies, sample_from_secir_params)
{
    double t0   = 0;
    double tmax = 50;
    double dt   = 0.1;

    double tinc    = 5.2, // R_2^(-1)+R_3^(-1)
        tinfmild   = 6, // 4-14  (=R4^(-1))
        tserint    = 4.2, // 4-4.4 // R_2^(-1)+0.5*R_3^(-1)
        thosp2home = 12, // 7-16 (=R5^(-1))
        thome2hosp = 5, // 2.5-7 (=R6^(-1))
        thosp2icu  = 2, // 1-3.5 (=R7^(-1))
        ticu2home  = 8, // 5-16 (=R8^(-1))
        tinfasy    = 6.2, // (=R9^(-1)=R_3^(-1)+0.5*R_4^(-1))
        ticu2death = 5; // 3.5-7 (=R5^(-1))

    double cont_freq = 0.5, // 0.2-0.75
        alpha        = 0.09, // 0.01-0.16
        beta         = 0.25, // 0.05-0.5
        delta        = 0.3, // 0.15-0.77
        rho          = 0.2, // 0.1-0.35
        theta        = 0.25; // 0.15-0.4

    double num_total_t0 = 10000, num_exp_t0 = 100, num_inf_t0 = 50, num_car_t0 = 50, num_hosp_t0 = 20, num_icu_t0 = 10,
           num_rec_t0 = 10, num_dead_t0 = 0;

    int num_groups = 3;
    double fact    = 1.0 / (double)num_groups;

    epi::SecirParams params(num_groups);

    for (size_t i = 0; i < num_groups; i++) {
        params.times[i].set_incubation(tinc);
        params.times[i].set_infectious_mild(tinfmild);
        params.times[i].set_serialinterval(tserint);
        params.times[i].set_hospitalized_to_home(thosp2home);
        params.times[i].set_home_to_hospitalized(thome2hosp);
        params.times[i].set_hospitalized_to_icu(thosp2icu);
        params.times[i].set_icu_to_home(ticu2home);
        params.times[i].set_infectious_asymp(tinfasy);
        params.times[i].set_icu_to_death(ticu2death);

        params.populations.set({i, epi::SecirCompartments::E}, fact * num_exp_t0);
        params.populations.set({i, epi::SecirCompartments::C}, fact * num_car_t0);
        params.populations.set({i, epi::SecirCompartments::I}, fact * num_inf_t0);
        params.populations.set({i, epi::SecirCompartments::H}, fact * num_hosp_t0);
        params.populations.set({i, epi::SecirCompartments::U}, fact * num_icu_t0);
        params.populations.set({i, epi::SecirCompartments::R}, fact * num_rec_t0);
        params.populations.set({i, epi::SecirCompartments::D}, fact * num_dead_t0);
        params.populations.set_difference_from_group_total({i, epi::SecirCompartments::S}, epi::SecirCategory::AgeGroup,
                                                           i, fact * num_total_t0);

        params.probabilities[i].set_infection_from_contact(1.0);
        params.probabilities[i].set_asymp_per_infectious(alpha);
        params.probabilities[i].set_risk_from_symptomatic(beta);
        params.probabilities[i].set_hospitalized_per_infectious(rho);
        params.probabilities[i].set_icu_per_hospitalized(theta);
        params.probabilities[i].set_dead_per_icu(delta);
    }

    epi::ContactFrequencyMatrix& cont_freq_matrix = params.get_contact_patterns();
    for (int i = 0; i < num_groups; i++) {
        for (int j = i; j < num_groups; j++) {
            cont_freq_matrix.set_cont_freq(fact * cont_freq, i, j);
        }
    }

    epi::ParameterSpace parameter_space(params, 0., 100., 0.2);

    epi::SecirParams params_sample = parameter_space.draw_sample();

    for (size_t i = 0; i < params_sample.get_num_groups(); i++) {

        EXPECT_GE(params_sample.populations.get_group_total(epi::SecirCategory::AgeGroup, i), 0);

        EXPECT_NEAR(params_sample.populations.get_group_total(epi::SecirCategory::AgeGroup, i), fact * num_total_t0,
                    1e-6);

        EXPECT_GE(params_sample.times[i].get_incubation(), 0);

        EXPECT_GE(params_sample.probabilities[i].get_infection_from_contact(), 0);
    }

    epi::ContactFrequencyMatrix& cont_freq_matrix_sample = params_sample.get_contact_patterns();

    for (size_t i = 0; i < params_sample.get_num_groups(); i++) {
        for (size_t j = 0; j < params_sample.get_num_groups(); j++) {
            EXPECT_GE(cont_freq_matrix_sample.get_dampings(static_cast<int>(i), static_cast<int>(j)).get_factor(1.0),
                      0);
            EXPECT_GE(cont_freq_matrix_sample.get_cont_freq(static_cast<int>(i), static_cast<int>(j)), 0);
        }
    }
}

TEST(ParameterStudies, test_normal_distribution)
{

    epi::ParameterDistributionNormal parameter_dist_normal_1;

    // check if standard deviation is reduced if between too narrow interval [min,max] has to be sampled.
    parameter_dist_normal_1.set_upper_bound(1);
    parameter_dist_normal_1.set_lower_bound(-1);
    parameter_dist_normal_1.log_stddev_changes(false); // only avoid warning output in tests

    double std_dev_demanded = parameter_dist_normal_1.get_standard_dev();
    parameter_dist_normal_1.get_sample();

    EXPECT_GE(std_dev_demanded, parameter_dist_normal_1.get_standard_dev());

    // check if full argument constructor works correctly
    epi::ParameterDistributionNormal parameter_dist_normal_2(-1.0, 1.0, 0, parameter_dist_normal_1.get_standard_dev());

    EXPECT_EQ(parameter_dist_normal_1.get_lower_bound(), parameter_dist_normal_2.get_lower_bound());
    EXPECT_EQ(parameter_dist_normal_1.get_upper_bound(), parameter_dist_normal_2.get_upper_bound());
    EXPECT_EQ(parameter_dist_normal_1.get_mean(), parameter_dist_normal_2.get_mean());
    EXPECT_EQ(parameter_dist_normal_1.get_standard_dev(), parameter_dist_normal_2.get_standard_dev());

    // check if std_dev is not changed if boundaries are far enough away such that 99% of the density fits into the interval
    parameter_dist_normal_2.set_mean(5);
    parameter_dist_normal_2.set_standard_dev(1.5);
    parameter_dist_normal_2.set_lower_bound(1);
    parameter_dist_normal_2.set_upper_bound(10);
    std_dev_demanded = parameter_dist_normal_2.get_standard_dev();

    parameter_dist_normal_2.check_quantiles();
    EXPECT_EQ(std_dev_demanded, parameter_dist_normal_2.get_standard_dev());

    // check that sampling only occurs in boundaries
    int counter[10] = {0};
    for (int i = 0; i < 1000; i++) {
        double val = parameter_dist_normal_2.get_sample();
        EXPECT_GE(parameter_dist_normal_2.get_upper_bound() + 1e-10, val);
        EXPECT_LE(parameter_dist_normal_2.get_lower_bound() - 1e-10, val);
    }
}

TEST(ParameterStudies, test_uniform_distribution)
{

    // check if full argument constructor works correctly
    epi::ParameterDistributionUniform parameter_dist_unif(1.0, 10.0);

    EXPECT_EQ(parameter_dist_unif.get_lower_bound(), 1.0);
    EXPECT_EQ(parameter_dist_unif.get_upper_bound(), 10.0);

    // check that sampling only occurs in boundaries
    for (int i = 0; i < 1000; i++) {
        double val = parameter_dist_unif.get_sample();
        EXPECT_GE(parameter_dist_unif.get_upper_bound() + 1e-10, val);
        EXPECT_LE(parameter_dist_unif.get_lower_bound() - 1e-10, val);
    }
}

TEST(ParameterStudies, test_predefined_samples)
{
    epi::ParameterDistributionUniform parameter_dist_unif(1.0, 10.0);

    epi::ParameterDistributionNormal parameter_dist_normal(-1.0, 1.0, 0, 0.1);

    // set predefined sample (can be out of [min,max]) and get it
    parameter_dist_unif.add_predefined_sample(2);
    double var = parameter_dist_unif.get_sample();
    EXPECT_EQ(var, 2);

    // predefined sample was deleted, get real sample which cannot be 2 due to [min,max]
    var = parameter_dist_unif.get_sample();
    EXPECT_NE(var, 2);

    // set predefined sample (can be out of [min,max]) and get it
    parameter_dist_normal.add_predefined_sample(2);
    var = parameter_dist_normal.get_sample();
    EXPECT_EQ(var, 2);

    // predefined sample was deleted, get real sample which cannot be 2 due to [min,max]
    var = parameter_dist_normal.get_sample();
    EXPECT_NE(var, 2);
}

TEST(ParameterStudies, check_ensemble_run_result)
{
    double t0   = 0;
    double tmax = 50;
    double dt   = 0.1;

    double tinc    = 5.2, // R_2^(-1)+R_3^(-1)
        tinfmild   = 6, // 4-14  (=R4^(-1))
        tserint    = 4.2, // 4-4.4 // R_2^(-1)+0.5*R_3^(-1)
        thosp2home = 12, // 7-16 (=R5^(-1))
        thome2hosp = 5, // 2.5-7 (=R6^(-1))
        thosp2icu  = 2, // 1-3.5 (=R7^(-1))
        ticu2home  = 8, // 5-16 (=R8^(-1))
        tinfasy    = 6.2, // (=R9^(-1)=R_3^(-1)+0.5*R_4^(-1))
        ticu2death = 5; // 3.5-7 (=R5^(-1))

    double cont_freq = 0.5, // 0.2-0.75
        alpha        = 0.09, // 0.01-0.16
        beta         = 0.25, // 0.05-0.5
        delta        = 0.3, // 0.15-0.77
        rho          = 0.2, // 0.1-0.35
        theta        = 0.25; // 0.15-0.4

    double num_total_t0 = 10000, num_exp_t0 = 100, num_inf_t0 = 50, num_car_t0 = 50, num_hosp_t0 = 20, num_icu_t0 = 10,
           num_rec_t0 = 10, num_dead_t0 = 0;

    int num_groups = 1;
    double fact    = 1.0 / (double)num_groups;

    epi::SecirParams params(num_groups);

    for (size_t i = 0; i < num_groups; i++) {
        params.times[i].set_incubation(tinc);
        params.times[i].set_infectious_mild(tinfmild);
        params.times[i].set_serialinterval(tserint);
        params.times[i].set_hospitalized_to_home(thosp2home);
        params.times[i].set_home_to_hospitalized(thome2hosp);
        params.times[i].set_hospitalized_to_icu(thosp2icu);
        params.times[i].set_icu_to_home(ticu2home);
        params.times[i].set_infectious_asymp(tinfasy);
        params.times[i].set_icu_to_death(ticu2death);

        params.populations.set({i, epi::SecirCompartments::E}, fact * num_exp_t0);
        params.populations.set({i, epi::SecirCompartments::C}, fact * num_car_t0);
        params.populations.set({i, epi::SecirCompartments::I}, fact * num_inf_t0);
        params.populations.set({i, epi::SecirCompartments::H}, fact * num_hosp_t0);
        params.populations.set({i, epi::SecirCompartments::U}, fact * num_icu_t0);
        params.populations.set({i, epi::SecirCompartments::R}, fact * num_rec_t0);
        params.populations.set({i, epi::SecirCompartments::D}, fact * num_dead_t0);
        params.populations.set_difference_from_group_total({i, epi::SecirCompartments::S}, epi::SecirCategory::AgeGroup,
                                                           i, fact * num_total_t0);

        params.probabilities[i].set_infection_from_contact(1.0);
        params.probabilities[i].set_asymp_per_infectious(alpha);
        params.probabilities[i].set_risk_from_symptomatic(beta);
        params.probabilities[i].set_hospitalized_per_infectious(rho);
        params.probabilities[i].set_icu_per_hospitalized(theta);
        params.probabilities[i].set_dead_per_icu(delta);

        params.probabilities[i].get_asymp_per_infectious().get_distribution().get();
    }

    epi::ContactFrequencyMatrix& cont_freq_matrix = params.get_contact_patterns();
    epi::Damping dummy(30., 0.3);
    for (int i = 0; i < num_groups; i++) {
        for (int j = i; j < num_groups; j++) {
            cont_freq_matrix.set_cont_freq(fact * cont_freq, i, j);
        }
    }

    epi::ParameterStudy parameter_study(
        [](double t0, double tmax, double dt, epi::SecirParams const& params, std::vector<Eigen::VectorXd>& secir) {
            return epi::simulate(t0, tmax, dt, params, secir);
        },
        params, t0, tmax, 0.2, 1);

    // Run parameter study
    int run = 0;
    parameter_study.set_num_runs(1);
    std::vector<std::vector<Eigen::VectorXd>> results = parameter_study.run();

    // printf("\n %d %d %d %d ", results.size(), results[0].size(), results[0][0].size(), params.size());
    for (size_t i = 0; i < results[0].size(); i++) { // number of time steps
        std::vector<double> total_at_ti(epi::SecirCompartments::SecirCount, 0);
        // printf("\n");
        for (size_t j = 0; j < static_cast<size_t>(results[0][i].size()); j++) { // number of compartments per time step
            // printf(" %.2e ( %d ) ", results[0][i][j], j % epi::SecirCompartments::SecirCount);
            EXPECT_GE(results[0][i][j], -1e-3) << " day " << i << " group " << j;
            total_at_ti[j / epi::SecirCompartments::SecirCount] += results[0][i][j];
        }
        for (size_t j = 0; j < params.get_num_groups(); j++) {
            EXPECT_NEAR(total_at_ti[j], params.populations.get_group_total(epi::SecirCategory::AgeGroup, j), 1e-3)
                << " day " << i << " group " << j;
        }
    }
}
