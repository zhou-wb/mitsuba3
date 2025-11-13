#include <mitsuba/render/sampler.h>
#include <mitsuba/core/properties.h>
#include <mitsuba/core/profiler.h>

NAMESPACE_BEGIN(mitsuba)

/**!

.. _sampler-repeat:

Repeat PCG32 sampler (:monosp:`repeat`)
----------------------------------------

.. pluginparameters::

 * - sample_count
   - |int|
   - Number of samples per pixel (Default: 4)

 * - seed
   - |int|
   - Seed offset (Default: 0)

 * - repeat_every
   - |int|
   - Number of lanes that share the same random sequence (Default: 1)

This sampler extends the PCG32 random number generator to support repeating
random sequences across groups of lanes in vectorized rendering. Each group
of ``repeat_every`` lanes will generate identical random numbers, which can
be useful for certain debugging or analysis scenarios.

 */

template <typename Float, typename Spectrum>
class RepeatPCG32Sampler final : public PCG32Sampler<Float, Spectrum> {
public:
    MI_IMPORT_BASE(PCG32Sampler, m_sample_count, m_base_seed, m_rng, seed,
                   seeded, m_samples_per_wavefront, m_wavefront_size,
                   schedule_state)
    MI_IMPORT_TYPES()

    RepeatPCG32Sampler(const Properties &props) : Base(props) {
        m_repeat_every = props.get<uint32_t>("repeat_every", 1);
        if (m_repeat_every == 0)
            Throw("repeat_every must be >= 1");
    }

    ref<Sampler<Float, Spectrum>> fork() override {
        RepeatPCG32Sampler *sampler = new RepeatPCG32Sampler(Properties());
        sampler->m_sample_count = m_sample_count;
        sampler->m_base_seed = m_base_seed;
        sampler->m_repeat_every = m_repeat_every;
        return sampler;
    }

    ref<Sampler<Float, Spectrum>> clone() override {
        return new RepeatPCG32Sampler(*this);
    }

    void seed(UInt32 seed_val, uint32_t wavefront_size = (uint32_t) -1) override {
        Base::seed(seed_val, wavefront_size);

        // Compute base seed
        UInt32 seed_value = m_base_seed + seed_val;

        if constexpr (dr::is_array_v<Float>) {
            // Lane indices
            UInt32 idx = dr::arange<UInt32>(m_wavefront_size);
            // Group head index: group every repeat_every lanes together
            UInt32 head = (idx / UInt32(m_repeat_every)) * UInt32(m_repeat_every);

            dr::make_opaque(seed_value);

            // Use TEA to shuffle (seed_value, head) to ensure independence between groups
            auto [v0, v1] = sample_tea_32(seed_value, head);

            // Assign the same RNG state/stream to each lane in the same group
            m_rng.seed(v0, v1);
        } else {
            // Scalar backend
            using PCG32Type = mitsuba::PCG32<UInt32>;
            m_rng.seed(seed_value, PCG32Type::PCG32_DEFAULT_STREAM);
        }
    }

    Float next_1d(Mask active = true) override {
        Assert(seeded());
        return m_rng.template next_float<Float>(active);
    }

    Point2f next_2d(Mask active = true) override {
        Float f1 = next_1d(active),
              f2 = next_1d(active);
        return Point2f(f1, f2);
    }

    std::string to_string() const override {
        std::ostringstream oss;
        oss << "RepeatPCG32Sampler[" << std::endl
            << "  base_seed = " << m_base_seed << "," << std::endl
            << "  sample_count = " << m_sample_count << "," << std::endl
            << "  samples_per_wavefront = " << m_samples_per_wavefront << "," << std::endl
            << "  wavefront_size = " << m_wavefront_size << "," << std::endl
            << "  repeat_every = " << m_repeat_every << std::endl
            << "]";
        return oss.str();
    }

    MI_DECLARE_CLASS(RepeatPCG32Sampler)

private:
    RepeatPCG32Sampler(const RepeatPCG32Sampler &sampler) 
        : Base(sampler), m_repeat_every(sampler.m_repeat_every) {}

    uint32_t m_repeat_every = 1;
};

MI_EXPORT_PLUGIN(RepeatPCG32Sampler)
NAMESPACE_END(mitsuba)