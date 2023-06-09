#include "../include/VariationalBayesianInferenceStrategy.hpp"
#include "TestHelpers.hpp"
#include <gtest/gtest.h>

namespace {

TEST(EvaluateResponsibilities, Dummy) {}

TEST(ComputeStatistics, Dummy) {}

TEST(UpdateRandomVariables, Dummy) {}

class VariationalBayesianInferenceFixture
    : public ::testing::TestWithParam<bool> {
protected:
  static void SetUpTestSuite() {
    gmm_.add_component(
        {0.5, gmix::initialize<gmix::ColVector<2>>({2.0, 9.0}),
         gmix::initialize<gmix::Matrix<2, 2>>({{2.0, 0.0}, {0.0, 2.0}})});
    gmm_.add_component(
        {0.5, gmix::initialize<gmix::ColVector<2>>({-5.0, 4.0}),
         gmix::initialize<gmix::Matrix<2, 2>>({{0.5, 0.0}, {0.0, 0.5}})});
    samples_ = gmix::draw_from_gaussian_mixture(gmm_, 1E5);
  }

  void SetUp() override {
    gmix::VariationalBayesianInferenceParameters<2> parameters;
    parameters.n_components = 2;
    parameters.n_iterations = 10;
    parameters.early_stopping_threshold = 0.0;
    parameters.warm_start = GetParam();
    parameters.dirichlet_prior_weight = 1.0;
    parameters.normal_prior_mean =
        gmix::initialize<gmix::ColVector<2>>({0.0, 0.0});
    parameters.normal_prior_covariance_scaling = 1.0;
    parameters.wishart_prior_degrees_of_freedom = 2.0;
    parameters.wishart_prior_information =
        gmix::initialize<gmix::Matrix<2, 2>>({{1.0, 0.0}, {0.0, 1.0}});
    strategy_ = gmix::VariationalBayesianInferenceStrategy<2>{parameters};
  }

  static gmix::GaussianMixture<2> gmm_;
  static gmix::StaticRowsMatrix<2> samples_;
  gmix::VariationalBayesianInferenceParameters<2> parameters_{};
  gmix::VariationalBayesianInferenceStrategy<2> strategy_{
      gmix::VariationalBayesianInferenceParameters<2>{}};
};

gmix::GaussianMixture<2> VariationalBayesianInferenceFixture::gmm_{};
gmix::StaticRowsMatrix<2> VariationalBayesianInferenceFixture::samples_{};

TEST_P(
    VariationalBayesianInferenceFixture,
    Fit_GivenParametersAndSamples_ExpectCorrectApproximationOfUnderlyingDistribution) {
  gmix::GaussianMixture<2> gmm;
  if (GetParam()) {
    gmm.add_component(
        {0.5, gmix::initialize<gmix::ColVector<2>>({1.0, 1.0}),
         gmix::initialize<gmix::Matrix<2, 2>>({{1.0, 0.0}, {0.0, 1.0}})});
    gmm.add_component(
        {0.5, gmix::initialize<gmix::ColVector<2>>({-1.0, -1.0}),
         gmix::initialize<gmix::Matrix<2, 2>>({{1.0, 0.0}, {0.0, 1.0}})});
  }

  gmix::fit(samples_, strategy_, gmm);

  EXPECT_TRUE(
      test::compare_gaussian_mixtures(gmm_, gmm, test::RANDOM_TOLERANCE));
}

INSTANTIATE_TEST_SUITE_P(VariationalBayesianInferenceStrategyWarmColdStart,
                         VariationalBayesianInferenceFixture,
                         testing::Values(true, false));

} // namespace
