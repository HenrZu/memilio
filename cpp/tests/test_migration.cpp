#define _USE_MATH_DEFINES

#include <cmath>

#include <Eigen/Core>
#include "epidemiology/migration.h"
#include "epidemiology/seir.h"
#include "epidemiology/eigen_util.h"
#include "gtest/gtest.h"

TEST(TestMigration, compareWithSingleIntegration)
{
    auto t0   = 0;
    auto tmax = 5;
    auto dt   = 1;

    auto params1 = epi::SeirParams();
    params1.populations.set({epi::SeirCompartments::S}, 0.9);
    params1.populations.set({epi::SeirCompartments::E}, 0.1);
    params1.populations.set_total(1000);
    params1.times.set_cont_freq(2.5);
    params1.times.set_incubation(4);
    params1.times.set_infectious(10);

    auto params2 = params1;
    params2.populations.set({epi::SeirCompartments::S}, 1.);
    params2.populations.set_total(500);

    auto graph_sim =
        epi::make_migration_sim(t0, dt, epi::Graph<epi::ModelNode<epi::SeirSimulation>, epi::MigrationEdge>());
    auto& g = graph_sim.get_graph();
    g.add_node(params1, t0);
    g.add_node(params2, t0);
    g.add_edge(0, 1, Eigen::VectorXd::Constant(4, 0)); //no migration along this edge
    g.add_edge(1, 0, Eigen::VectorXd::Constant(4, 0));

    auto single_sim1 = epi::SeirSimulation(params1, t0);
    auto single_sim2 = epi::SeirSimulation(params2, t0);

    auto nsteps = int((tmax - t0) / dt);
    EXPECT_EQ(nsteps, (tmax - t0) / dt);
    graph_sim.advance(nsteps);
    single_sim1.advance(tmax);
    single_sim2.advance(tmax);

    EXPECT_DOUBLE_EQ(g.nodes()[0].model.get_t().back(), single_sim1.get_t().back());
    EXPECT_DOUBLE_EQ(g.nodes()[1].model.get_t().back(), single_sim2.get_t().back());

    //graph may have different time steps, so we can't expect high accuracy here
    EXPECT_NEAR((g.nodes()[0].model.get_y().back() - single_sim1.get_y().back()).norm(), 0.0, 1e-6);
    EXPECT_NEAR((g.nodes()[1].model.get_y().back() - single_sim2.get_y().back()).norm(), 0.0, 1e-6);
}