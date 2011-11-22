/**
 * @file cxx/machine/src/Gaussian.cc
 * @date Tue May 10 11:35:58 2011 +0200
 * @author Francois Moulin <Francois.Moulin@idiap.ch>
 *
 * Copyright (C) 2011 Idiap Reasearch Institute, Martigny, Switzerland
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "machine/Gaussian.h"
#include <cfloat>
#include <io/Array.h>
#include <io/Arrayset.h>
#include <machine/Exception.h>

double Torch::machine::Log::LogAdd(double log_a, double log_b) {
  double minusdif;

  if (log_a < log_b)
  {
    double tmp = log_a;
    log_a = log_b;
    log_b = tmp;
  }

  minusdif = log_b - log_a;
  //#ifdef DEBUG
  if (std::isnan(minusdif)) {
    printf("LogAdd: minusdif (%f) log_b (%f) or log_a (%f) is nan\n", minusdif, log_b, log_a);
    throw Torch::machine::Exception();
  }
  //#endif
  if (minusdif < MINUS_LOG_THRESHOLD) return log_a;
  else return log_a + log1p(exp(minusdif));
}

double Torch::machine::Log::LogSub(double log_a, double log_b) {
  double minusdif;

  if (log_a < log_b) {
    printf("LogSub: log_a (%f) should be greater than log_b (%f)", log_a, log_b);
    throw Torch::machine::Exception();
  }

  minusdif = log_b - log_a;
  //#ifdef DEBUG
  if (std::isnan(minusdif)) {
    printf("LogSub: minusdif (%f) log_b (%f) or log_a (%f) is nan", minusdif, log_b, log_a);
    throw Torch::machine::Exception();
  }
  //#endif
  if (log_a == log_b) return LogZero;
  else if (minusdif < MINUS_LOG_THRESHOLD) return log_a;
  else return log_a + log1p(-exp(minusdif));
}



Torch::machine::Gaussian::Gaussian() {
  resize(0);
}

Torch::machine::Gaussian::Gaussian(int n_inputs) {
  resize(n_inputs);
}

Torch::machine::Gaussian::Gaussian(Torch::io::HDF5File& config) {
  load(config);
}

Torch::machine::Gaussian::~Gaussian() {
}

Torch::machine::Gaussian::Gaussian(const Gaussian& other) {
  copy(other);
}

Torch::machine::Gaussian& Torch::machine::Gaussian::operator=(const Gaussian &other) {
  if (this != &other) {
    copy(other);
  }

  return *this;
}

bool Torch::machine::Gaussian::operator==(const Gaussian& b) const {
  return m_n_inputs == b.m_n_inputs &&
         blitz::all(m_mean == b.m_mean) &&
         blitz::all(m_variance == b.m_variance) &&
         blitz::all(m_variance_thresholds == b.m_variance_thresholds);
}


void Torch::machine::Gaussian::copy(const Gaussian& other) {
  m_n_inputs = other.m_n_inputs;

  m_mean.resize(m_n_inputs);
  m_mean = other.m_mean;

  m_variance.resize(m_n_inputs);
  m_variance = other.m_variance;

  m_variance_thresholds.resize(m_n_inputs);
  m_variance_thresholds = other.m_variance_thresholds;

  g_norm = other.g_norm;
}

void Torch::machine::Gaussian::setNInputs(int n_inputs) {
  resize(n_inputs);
}


int Torch::machine::Gaussian::getNInputs() {
  return m_n_inputs;
}

void Torch::machine::Gaussian::resize(int n_inputs) {
  m_n_inputs = n_inputs;
  m_mean.resize(m_n_inputs);
  m_mean = 0;
  m_variance.resize(m_n_inputs);
  m_variance = 1;
  m_variance_thresholds.resize(m_n_inputs);
  m_variance_thresholds = 0;

  // Re-compute g_norm, because m_n_inputs and m_variance
  // have changed
  preComputeConstants();
}

void Torch::machine::Gaussian::setMean(const blitz::Array<double,1> &mean) {
  m_mean = mean;
}

void Torch::machine::Gaussian::setVariance(const blitz::Array<double,1> &variance) {

  m_variance = variance;

  // Apply variance flooring threshold
  blitz::Array<bool,1> isTooSmall(m_n_inputs);
  isTooSmall = m_variance < m_variance_thresholds;
  m_variance += (m_variance_thresholds - m_variance) * isTooSmall;

  // Re-compute g_norm, because m_variance has changed
  preComputeConstants();
}

void Torch::machine::Gaussian::setVarianceThresholds(const blitz::Array<double,1> &variance_thresholds) {
  m_variance_thresholds = variance_thresholds;

  // setVariance() will reset the variances that are now too small
  setVariance(m_variance);
}

void Torch::machine::Gaussian::setVarianceThresholds(double factor) {
  blitz::Array<double,1> variance_thresholds(m_n_inputs);
  variance_thresholds = m_variance * factor;
  setVarianceThresholds(variance_thresholds);
}

double Torch::machine::Gaussian::logLikelihood(const blitz::Array<double,1> &x) const {
  double z = blitz::sum(blitz::pow2(x - m_mean) / m_variance);

  // Log Likelihood
  return (-0.5 * (g_norm + z));
}

void Torch::machine::Gaussian::getVarianceThresholds(blitz::Array<double,1> &variance_thresholds) const {
  variance_thresholds.resize(m_n_inputs);
  variance_thresholds = m_variance_thresholds;
}

void Torch::machine::Gaussian::getMean(blitz::Array<double,1> &mean) const {
  mean.resize(m_n_inputs);
  mean = m_mean;
}

void Torch::machine::Gaussian::getVariance(blitz::Array<double,1> &variance) const {
  variance.resize(m_n_inputs);
  variance = m_variance;
}

void Torch::machine::Gaussian::preComputeConstants() {
  double c = m_n_inputs * Log::Log2Pi;
  double log_det = blitz::sum(blitz::log(m_variance));
  g_norm = c + log_det;
}


void Torch::machine::Gaussian::save(Torch::io::HDF5File& config) const {
  config.setArray("m_mean", m_mean);
  config.setArray("m_variance", m_variance);
  config.setArray("m_variance_thresholds", m_variance_thresholds);
  config.set("g_norm", g_norm);
  config.set("m_n_inputs", m_n_inputs);
}

void Torch::machine::Gaussian::load(Torch::io::HDF5File& config) {
  m_n_inputs = config.read<int64_t>("m_n_inputs");
  
  m_mean.resize(m_n_inputs);
  m_variance.resize(m_n_inputs);
  m_variance_thresholds.resize(m_n_inputs);
 
  config.readArray("m_mean", m_mean);
  config.readArray("m_variance", m_variance);
  config.readArray("m_variance_thresholds", m_variance_thresholds);

  g_norm = config.read<double>("g_norm");
}

namespace Torch{
  namespace machine{
    std::ostream& operator<<(std::ostream& os, const Gaussian& g) {
      os << "Mean = " << g.m_mean << std::endl;
      os << "Variance = " << g.m_variance << std::endl;
      return os;
    }
  }
}
