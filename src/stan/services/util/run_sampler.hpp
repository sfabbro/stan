#ifndef STAN_SERVICES_UTIL_RUN_SAMPLER_HPP
#define STAN_SERVICES_UTIL_RUN_SAMPLER_HPP

#include <stan/services/util/generate_transitions.hpp>
#include <stan/services/util/mcmc_writer.hpp>
#include <ctime>
#include <vector>

namespace stan {
  namespace services {
    namespace util {

      /**
       * Runs the sampler without adaptation.
       *
       * @tparam Model Type of model
       * @tparam RNG Type of random number generator
       * @param[in,out] sampler the mcmc sampler to use on the model
       * @param[in] model the model concept to use for computing log probability
       * @param[in] cont_vector initial parameter values
       * @param[in] num_warmup number of warmup draws
       * @param[in] num_samples number of post warmup draws
       * @param[in] num_thin number to thin the draws. Must be greater than or equal to 1.
       * @param[in] refresh controls output to the <code>message_writer</code>
       * @param[in] save_warmup indicates whether the warmup draws should be sent
       *   to the sample writer
       * @param[in,out] rng random number generator
       * @param[out] interrupt interrupt callback
       * @param[out] message_writer writer for messages
       * @param[out] error_writer writer for error messages
       * @param[out] sample_writer writer for draws
       * @param[out] diagnostic_writer writer for diagnostic information
       */
      template <class Model, class RNG>
      void run_sampler(stan::mcmc::base_mcmc& sampler,
                       Model& model,
                       std::vector<double>& cont_vector,
                       int num_warmup,
                       int num_samples,
                       int num_thin,
                       int refresh,
                       bool save_warmup,
                       RNG& rng,
                       callbacks::interrupt&
                       interrupt,
                       callbacks::writer& message_writer,
                       callbacks::writer& error_writer,
                       callbacks::writer& sample_writer,
                       callbacks::writer&
                       diagnostic_writer) {
        Eigen::Map<Eigen::VectorXd> cont_params(cont_vector.data(),
                                                cont_vector.size());
        services::sample::mcmc_writer
          writer(sample_writer, diagnostic_writer, message_writer);
        stan::mcmc::sample s(cont_params, 0, 0);

        // Headers
        writer.write_sample_names(s, sampler, model);
        writer.write_diagnostic_names(s, sampler, model);

        clock_t start = clock();
        stan::services::util::generate_transitions
          (sampler, num_warmup, 0, num_warmup + num_samples, num_thin,
           refresh, save_warmup, true,
           writer,
           s, model, rng,
           interrupt, message_writer, error_writer);
        clock_t end = clock();
        double warm_delta_t = static_cast<double>(end - start) / CLOCKS_PER_SEC;

        start = clock();
        stan::services::util::generate_transitions
          (sampler, num_samples, num_warmup, num_warmup + num_samples, num_thin,
           refresh, true, false,
           writer,
           s, model, rng,
           interrupt, message_writer, error_writer);
        end = clock();
        double sample_delta_t
          = static_cast<double>(end - start) / CLOCKS_PER_SEC;

        writer.write_timing(warm_delta_t, sample_delta_t);
      }
    }
  }
}

#endif