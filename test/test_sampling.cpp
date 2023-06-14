#include "../include/KMeansStrategy.hpp"
#include "../include/Statistics.hpp"
#include "TestHelpers.hpp"
#include <gtest/gtest.h>
#include <numeric>

// Tests if univariate sampling distribution approximates underlying
// univeriate distribution.
TEST(SamplingTest, SingleUnivariateComponentMeanTest) {

  constexpr int DIM = 1;
  constexpr size_t N_SAMPLES = 1E5;
  gm::GaussianMixtureKMeans<DIM> gmm;
  const auto weight = 1.0;
  const auto mean = (gm::Vector<DIM>() << 9.7).finished();
  const auto covariance = (gm::Matrix<DIM, DIM>() << 1.7).finished();
  gmm.add_component({weight, mean, covariance});

  const auto samples = gm::draw_from_gaussian_mixture(gmm, N_SAMPLES);

  const auto sample_mean = gm::sample_mean(samples);
  const auto sample_covariance = gm::sample_covariance(samples, sample_mean);

  EXPECT_TRUE(test::is_near(mean, sample_mean, test::THRESHOLD));
  EXPECT_TRUE(test::is_near(covariance, sample_covariance, test::THRESHOLD));
}

// Tests if multivariate sampling distribution approximates underlying
// multivariate distribution.
TEST(SamplingTest, SingleMultivariateComponentTest) {

  constexpr int DIM = 2;
  constexpr size_t N_SAMPLES = 1E5;
  gm::GaussianMixtureKMeans<DIM> gmm;
  const auto weight = 1.0;
  const auto mean = (gm::Vector<DIM>() << 9.7, 4.2).finished();
  const auto covariance =
      (gm::Matrix<DIM, DIM>() << 1.7, 0.0, 0.0, 2.4).finished();
  gmm.add_component({weight, mean, covariance});

  const auto samples = gm::draw_from_gaussian_mixture(gmm, N_SAMPLES);

  const auto sample_mean = gm::sample_mean(samples);
  const auto sample_covariance = gm::sample_covariance(samples, sample_mean);

  EXPECT_TRUE(test::is_near(mean, sample_mean, test::THRESHOLD));
  EXPECT_TRUE(test::is_near(covariance, sample_covariance, test::THRESHOLD));
}
