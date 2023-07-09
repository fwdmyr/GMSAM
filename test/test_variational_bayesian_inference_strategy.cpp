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
        {0.5, gm::initialize<gm::ColVector<2>>({2.0, 9.0}),
         gm::initialize<gm::Matrix<2, 2>>({{2.0, 0.0}, {0.0, 2.0}})});
    gmm_.add_component(
        {0.5, gm::initialize<gm::ColVector<2>>({-5.0, 4.0}),
         gm::initialize<gm::Matrix<2, 2>>({{0.5, 0.0}, {0.0, 0.5}})});
    samples_ = gm::draw_from_gaussian_mixture(gmm_, 1E5);
  }

  void SetUp() override {
    gm::VariationalBayesianInferenceParameters<2> parameters;
    parameters.n_components = 2;
    parameters.n_iterations = 10;
    parameters.early_stopping_threshold = 0.0;
    parameters.warm_start = GetParam();
    parameters.dirichlet_prior_weight = 1.0;
    parameters.normal_prior_mean = gm::initialize<gm::ColVector<2>>({0.0, 0.0});
    parameters.normal_prior_covariance_scaling = 1.0;
    parameters.wishart_prior_degrees_of_freedom = 2.0;
    parameters.wishart_prior_information =
        gm::initialize<gm::Matrix<2, 2>>({{1.0, 0.0}, {0.0, 1.0}});
    strategy_ = gm::VariationalBayesianInferenceStrategy<2>{parameters};
  }

  static gm::GaussianMixture<2> gmm_;
  static gm::StaticRowsMatrix<2> samples_;
  gm::VariationalBayesianInferenceParameters<2> parameters_{};
  gm::VariationalBayesianInferenceStrategy<2> strategy_{
      gm::VariationalBayesianInferenceParameters<2>{}};
};

gm::GaussianMixture<2> VariationalBayesianInferenceFixture::gmm_{};
gm::StaticRowsMatrix<2> VariationalBayesianInferenceFixture::samples_{};

TEST_P(
    VariationalBayesianInferenceFixture,
    Fit_GivenParametersAndSamples_ExpectCorrectApproximationOfUnderlyingDistribution) {
  gm::GaussianMixture<2> gmm;
  if (GetParam()) {
    gmm.add_component(
        {0.5, gm::initialize<gm::ColVector<2>>({1.0, 1.0}),
         gm::initialize<gm::Matrix<2, 2>>({{1.0, 0.0}, {0.0, 1.0}})});
    gmm.add_component(
        {0.5, gm::initialize<gm::ColVector<2>>({-1.0, -1.0}),
         gm::initialize<gm::Matrix<2, 2>>({{1.0, 0.0}, {0.0, 1.0}})});
  }

  gm::fit(samples_, strategy_, gmm);

  EXPECT_TRUE(
      test::compare_gaussian_mixtures(gmm_, gmm, test::RANDOM_TOLERANCE));
}

INSTANTIATE_TEST_SUITE_P(VariationalBayesianInferenceStrategyWarmColdStart,
                         VariationalBayesianInferenceFixture,
                         testing::Values(true, false));

} // namespace
