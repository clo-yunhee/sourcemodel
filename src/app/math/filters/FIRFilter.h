#ifndef SOURCEMODEL__MATH_FILTERS_FIRFILTER_H
#define SOURCEMODEL__MATH_FILTERS_FIRFILTER_H

#include <array>
#include <boost/circular_buffer.hpp>
#include <vector>

class FIRFilter {
   public:
    FIRFilter(const std::vector<double>& coefs = {});

    template <size_t N>
    FIRFilter(const std::array<double, N>& coefs)
        : FIRFilter(std::vector<double>(coefs.begin(), coefs.end())) {}

    std::vector<double> filter(const std::vector<double>& x);

   private:
    std::vector<double>            m_coefs;
    boost::circular_buffer<double> m_memory;
};

#endif  // SOURCEMODEL__MATH_FILTERS_FIRFILTER_H48000