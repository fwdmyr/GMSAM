#ifndef GMSAM_GAUSSIAN_MIXTURE_HPP
#define GMSAM_GAUSSIAN_MIXTURE_HPP

#include "BaseStrategy.hpp"
#include "ExpectationMaximizationStrategy.hpp"
#include "KMeansStrategy.hpp"
#include "VariationalBayesianInferenceStrategy.hpp"
#include <initializer_list>
#include <memory>
#include <random>
#include <vector>

namespace gm {

template <int Dim, typename Strategy = BaseStrategy<Dim>>
class GaussianMixture {

public:
  using container_type = std::vector<GaussianComponent<Dim>>;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  GaussianMixture() = default;
  GaussianMixture(std::initializer_list<GaussianComponent<Dim>>);
  GaussianMixture(const GaussianMixture<Dim, Strategy> &) = default;
  GaussianMixture(GaussianMixture<Dim, Strategy> &&) = default;
  GaussianMixture<Dim, Strategy> &
  operator=(const GaussianMixture<Dim, Strategy> &) = default;
  GaussianMixture<Dim, Strategy> &
  operator=(GaussianMixture<Dim, Strategy> &&) = default;

  inline iterator begin() noexcept { return components_.begin(); }
  inline const_iterator cbegin() const noexcept { return components_.cbegin(); }
  inline iterator end() noexcept { return components_.end(); }
  inline const_iterator cend() const noexcept { return components_.cend(); }

  const GaussianComponent<Dim> &get_component(size_t idx) const {
    return components_[idx];
  }

  size_t get_size() const { return components_.size(); }

  void add_component(const GaussianComponent<Dim> &component) {
    components_.push_back(component);
  };
  void add_component(GaussianComponent<Dim> &&component) {
    components_.emplace_back(component);
  }

  void reset() { components_.resize(0); }

  void set_strategy(const typename Strategy::Parameters &params) {
    p_strategy_ = std::make_unique<Strategy>(params);
  }

  void set_strategy(typename Strategy::Parameters &&params) {
    p_strategy_ = std::make_unique<Strategy>(params);
  }

  void fit(const StaticRowsMatrix<Dim> &samples) {
    if (p_strategy_)
      p_strategy_->fit(components_, samples);
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const GaussianMixture<Dim, Strategy> &gmm) {
    os << "GaussianMixture<" << Dim << ">\n";
    for (const auto &component : gmm.components_)
      os << component << '\n';
    return os;
  }

private:
  std::vector<GaussianComponent<Dim>> components_{};
  std::unique_ptr<Strategy> p_strategy_{nullptr};
};

template <int Dim>
using GaussianMixtureKMeans = GaussianMixture<Dim, gm::KMeansStrategy<Dim>>;

template <int Dim>
using GaussianMixtureExpectationMaximization =
    GaussianMixture<Dim, gm::ExpectationMaximizationStrategy<Dim>>;

template <int Dim>
using GaussianMixtureVariationalBayesianInference =
    GaussianMixture<Dim, gm::VariationalBayesianInferenceStrategy<Dim>>;

template <typename Strategy, int Dim>
StaticRowsMatrix<Dim>
draw_from_gaussian_mixture(const GaussianMixture<Dim, Strategy> &gmm,
                           size_t n_samples) {

  static std::mt19937 gen{std::random_device{}()};
  static std::normal_distribution<> nd;

  std::vector<double> weights;
  weights.reserve(gmm.get_size());
  for (auto it = gmm.cbegin(); it < gmm.cend(); ++it)
    weights.push_back(it->get_weight());
  std::discrete_distribution<> dd(weights.begin(), weights.end());
  auto samples = StaticRowsMatrix<Dim>::Zero(Dim, n_samples).eval();

  for (size_t i = 0; i < n_samples; ++i) {
    const auto &component = gmm.get_component(dd(gen));
    Eigen::SelfAdjointEigenSolver<Matrix<Dim, Dim>> eigen_solver(
        component.get_covariance());
    const auto transform = eigen_solver.eigenvectors() *
                           eigen_solver.eigenvalues().cwiseSqrt().asDiagonal();
    samples.col(i) =
        component.get_mean() +
        transform * Vector<Dim>{}.unaryExpr([](auto x) { return nd(gen); });
  }
  return samples;
}

} // namespace gm
#endif // !GMSAM_GAUSSIAN_MIXTURE_HPP
