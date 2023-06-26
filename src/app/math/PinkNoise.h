#ifndef SOURCEMODEL__PINK_NOISE_H_
#define SOURCEMODEL__PINK_NOISE_H_

#include <array>
#include <random>

#include "math/utils.h"

class PinkNoise {
   private:
    static constexpr std::array<Scalar, 5> pA{3.80240, 2.96940, 2.59700, 3.08700,
                                              3.40060};
    static constexpr std::array<Scalar, 5> pSum{0.00198, 0.01478, 0.06378, 0.23378,
                                                0.91578};
    static constexpr Scalar                pASum{15.8564};

   public:
    PinkNoise() : contrib{0, 0, 0, 0, 0}, genout(0), dist(0, 1) {}

    template <typename RNG>
    Scalar operator()(RNG& rng) {
        const Scalar ur1 = dist(rng);
        const Scalar ur2 = dist(rng);

        for (int i = 0; i < 5; ++i) {
            if (ur1 < pSum[i]) {
                genout -= contrib[i];
                contrib[i] = 2 * (ur2 - 0.5) * pA[i];
                genout += contrib[i];
                break;
            }
        }
        return genout / pASum;
    }

   private:
    std::array<Scalar, 5> contrib;
    Scalar                genout;

    std::uniform_real_distribution<Scalar> dist;
};

#endif  // SOURCEMODEL__PINK_NOISE_H_