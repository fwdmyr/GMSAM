#ifndef GMSAM_K_MEANS_STRATEGY_HPP
#define GMSAM_K_MEANS_STRATEGY_HPP

#include "BaseStrategy.hpp"
#include "Common.hpp"
#include "Statistics.hpp"
#include <limits>
#include <numeric>
#include <random>

namespace gm {

template <int Dim> class KMeansStrategy final : public BaseStrategy<Dim> {
public:
  struct Parameters {
    int n_components{0};
    int n_iterations{0};
    bool warm_start{false};
  };

  explicit KMeansStrategy(const Parameters &parameters)
      : parameters_(parameters) {}
  virtual void fit(std::vector<GaussianComponent<Dim>> &,
                   const StaticRowsMatrix<Dim> &) const override;

private:
  Parameters parameters_{};
};

namespace internal {

template <int Dim>
std::vector<StaticRowsMatrix<Dim>> partition_samples_responsibly(
    const std::vector<GaussianComponent<Dim>> &components,
    const StaticRowsMatrix<Dim> &samples) {
  const auto n_components = components.size();
  std::vector<std::vector<int>> responsibilities{n_components};
  std::for_each(
      responsibilities.begin(), responsibilities.end(),
      [n_samples = samples.cols()](auto &component_responsibilities) -> void {
        component_responsibilities.reserve(n_samples);
      });
  // TODO: Probably also not vectorizable as we need to access the components
  for (size_t i = 0; i < samples.cols(); ++i) {
    auto squared_l2_min = std::numeric_limits<double>::max();
    size_t dominant_mode = 0;
    for (size_t j = 0; j < n_components; ++j) {
      const auto &component = components[j];
      const auto squared_l2 =
          (samples.col(i) - component.get_mean()).squaredNorm();
      if (squared_l2 <= squared_l2_min) {
        squared_l2_min = squared_l2;
        dominant_mode = j;
      }
    }
    responsibilities[dominant_mode].push_back(i);
  }
  std::vector<StaticRowsMatrix<Dim>> partitions;
  partitions.reserve(n_components);
  for (size_t i = 0; i < n_components; ++i) {
    const auto partition_size = responsibilities[i].size();
    auto partition = static_cast<StaticRowsMatrix<Dim>>(
        StaticRowsMatrix<Dim>::Zero(Dim, partition_size));
    for (size_t j = 0; j < partition_size; ++j) {
      partition.col(j) = samples.col(responsibilities[i][j]);
    }
    partitions.push_back(partition);
  }
  return partitions;
}

template <int Dim>
void update_weight(std::vector<GaussianComponent<Dim>> &components,
                   const std::vector<StaticRowsMatrix<Dim>> &partitions) {
  const auto n_samples = std::accumulate(
      partitions.begin(), partitions.end(), 0,
      [](auto acc, const auto &rhs) { return acc + rhs.cols(); });
  const auto n_components = components.size();
  for (size_t i = 0; i < n_components; ++i) {
    auto &component = components[i];
    auto &partition = partitions[i];
    const auto weight = static_cast<double>(partition.cols()) / n_samples;
    component.set_weight(weight);
  }
}

template <int Dim>
void update_mean(std::vector<GaussianComponent<Dim>> &components,
                 const std::vector<StaticRowsMatrix<Dim>> &partitions) {
  const auto n_components = components.size();
  for (size_t i = 0; i < n_components; ++i) {
    auto &component = components[i];
    const auto mu = sample_mean(partitions[i]);
    component.set_mean(mu);
  }
}

template <int Dim>
void update_covariance(std::vector<GaussianComponent<Dim>> &components,
                       const std::vector<StaticRowsMatrix<Dim>> &partitions) {
  const auto n_components = components.size();
  for (size_t i = 0; i < n_components; ++i) {
    auto &component = components[i];
    const auto mu = component.get_mean();
    const auto sigma = sample_covariance(partitions[i], mu);
    component.set_covariance(sigma);
  }
}

} // namespace internal

template <int Dim>
void KMeansStrategy<Dim>::fit(std::vector<GaussianComponent<Dim>> &components,
                              const StaticRowsMatrix<Dim> &samples) const {
  const auto n_components = parameters_.n_components;
  if (!parameters_.warm_start || components.size() != n_components)
    this->initialize(components, samples, n_components);
  std::vector<StaticRowsMatrix<Dim>> partitions;
  for (size_t i = 0; i < parameters_.n_iterations; ++i) {
    partitions = internal::partition_samples_responsibly(components, samples);
    internal::update_mean(components, partitions);
  }
  internal::update_weight(components, partitions);
  internal::update_covariance(components, partitions);
}

} // namespace gm
#endif // !GMSAM_K_MEANS_STRATEGY_HPP
