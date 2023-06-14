#include "../include/KMeansStrategy.hpp"
#include "../include/Statistics.hpp"
#include "TestHelpers.hpp"
#include <numeric>

// Tests if kMeans algorithm approximates the underlying Gaussian mixture
// distribution correctly.
TEST(FittingTest, kMeansStrategyFittingTest) {

  constexpr int DIM = 2;
  constexpr size_t N_SAMPLES = 1E5;
  gm::GaussianMixtureKMeans<DIM> sample_gmm;
  sample_gmm.add_component(
      {0.5, (gm::Vector<DIM>() << 2.0, 9.0).finished(),
       (gm::Matrix<DIM, DIM>() << 1.0, 0.0, 0.0, 1.0).finished()});
  sample_gmm.add_component(
      {0.5, (gm::Vector<DIM>() << -5.0, 4.0).finished(),
       (gm::Matrix<DIM, DIM>() << 1.0, 0.0, 0.0, 1.0).finished()});
  const auto samples = gm::draw_from_gaussian_mixture(sample_gmm, N_SAMPLES);

  gm::GaussianMixtureKMeans<DIM> gmm;
  gm::KMeansStrategy<DIM>::Parameters parameters;
  parameters.n_components = 2;
  parameters.n_iterations = 10;
  gmm.set_strategy(parameters);
  gmm.fit(samples);

  EXPECT_TRUE(
      test::compare_gaussian_mixtures(sample_gmm, gmm, test::THRESHOLD));
}

// Tests if Expectation Maximization algorithm approximates the underlying
// Gaussian mixture distribution correctly.
TEST(FittingTest, ExpectationMaximizationStrategyFittingTest) {

  constexpr int DIM = 2;
  constexpr size_t N_SAMPLES = 1E5;
  gm::GaussianMixtureExpectationMaximization<DIM> sample_gmm;
  sample_gmm.add_component(
      {0.5, (gm::Vector<DIM>() << 2.0, 9.0).finished(),
       (gm::Matrix<DIM, DIM>() << 1.0, 0.0, 0.0, 1.0).finished()});
  sample_gmm.add_component(
      {0.5, (gm::Vector<DIM>() << -5.0, 4.0).finished(),
       (gm::Matrix<DIM, DIM>() << 1.0, 0.0, 0.0, 1.0).finished()});
  const auto samples = gm::draw_from_gaussian_mixture(sample_gmm, N_SAMPLES);

  gm::GaussianMixtureExpectationMaximization<DIM> gmm;
  gm::ExpectationMaximizationStrategy<DIM>::Parameters parameters;
  parameters.n_components = 2;
  parameters.n_iterations = 10;
  gmm.set_strategy(parameters);
  gmm.fit(samples);

  EXPECT_TRUE(
      test::compare_gaussian_mixtures(sample_gmm, gmm, test::THRESHOLD));
}

// Tests if Variational Bayesian Inference algorithm approximates the underlying
// Gaussian mixture distribution correctly.
TEST(FittingTest, VariationalBayesianInferenceStrategyFittingTest) {

  constexpr int DIM = 2;
  constexpr size_t N_SAMPLES = 1E5;
  gm::GaussianMixtureVariationalBayesianInference<DIM> sample_gmm;
  sample_gmm.add_component(
      {0.5, (gm::Vector<DIM>() << 2.0, 9.0).finished(),
       (gm::Matrix<DIM, DIM>() << 1.0, 0.0, 0.0, 1.0).finished()});
  sample_gmm.add_component(
      {0.5, (gm::Vector<DIM>() << -5.0, 4.0).finished(),
       (gm::Matrix<DIM, DIM>() << 1.0, 0.0, 0.0, 1.0).finished()});
  const auto samples = gm::draw_from_gaussian_mixture(sample_gmm, N_SAMPLES);

  gm::GaussianMixtureVariationalBayesianInference<DIM> gmm;
  gm::VariationalBayesianInferenceStrategy<DIM>::Parameters parameters;
  parameters.n_components = 2;
  parameters.n_iterations = 10;
  gmm.set_strategy(parameters);
  gmm.fit(samples);

  EXPECT_TRUE(
      test::compare_gaussian_mixtures(sample_gmm, gmm, test::THRESHOLD));
}
