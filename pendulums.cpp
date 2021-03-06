// https://chalkdustmagazine.com/features/the-magnetic-pendulum/
#include <CoreMacros.hpp>
#include <CoreStrings.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <chrono>
#include <thread>

#include <Eigen/Eigen>

#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

// Decide on floating point type
#define USE_DOUBLE
#ifdef USE_DOUBLE
typedef double Scalar;
#else
typedef float Scalar;
#endif

typedef Eigen::Matrix<Scalar, 2, 1> Vec2;

constexpr Scalar operator""_f(long double f) { return static_cast<Scalar>(f); }
constexpr Scalar Pi = Scalar(M_PI);

// Simulation of a magnetic pendulum with K magnets
template <uint K>
struct PendulumSim
{

    // Magnet parameters
    const int num_magnets = K;          // How many magnets
    const Scalar magnet_radius = 1.0_f; // Radius of circle on which magnets are positioned

    // Physical parameters in "natural" units i.e. assuming g = 1

    const int magnetic_exponent = 4;        // n such as magnetic force is proportional to 1/r^n
    const Scalar magnetic_constant = 1.0_f; // Multiplicative constant of magnetic force wrt gravity
    const Scalar friction = 0.1_f;          // Friction coeff m.s^-2
    const Scalar height = 0.5_f;            // Min distance between pendulum and magnets
    const Scalar dt = 0.01_f;               // Simulation step (in natural units)

    const Scalar vel_epsilon = 1e-4_f; // Condition on which the simulation is considered stopped

    // Computed values
    Eigen::Matrix<Scalar, 2, K> magnet_positions; // position of the magnets
    Scalar h2;                                    // height squared
    Scalar exp;                                   // exponent in magnetic force computation
    Scalar friction_coeff;                        // Multiplicative coefficient to velocity

    PendulumSim()
    {
        // Precompute stuff
        for (int i = 0; i < K; ++i)
        {
            const Scalar angle = 2 * Scalar(i) * Pi / Scalar(K);
            magnet_positions.col(i) = magnet_radius * Vec2{cos(angle), sin(angle)};
        }
        h2 = height * height;
        exp = 0.5_f * (magnetic_exponent + 1);
        friction_coeff = (1 - dt * friction);
    }

    // Compute one timestep for the simulation
    void update(Vec2 &pos, Vec2 &vel) const
    {

        Vec2 magnetic = Vec2::Zero();
        for (int i = 0; i < K; ++i)
        {
            const Vec2 diff = magnet_positions.col(i) - pos;
            const Scalar d2 = diff.squaredNorm();
            const Scalar magnitude = magnetic_constant / pow((d2 + h2), exp);
            magnetic += magnitude * diff;
        }

        // Semi-implicit Euler
        // Since m = 1 g = 1 gravity = -pos
        vel += dt * (magnetic - pos);
        vel *= friction_coeff;
        pos += dt * vel;
    }
};

// What we want to compute : an image which represents the attractor basins of the magnetic pendulum
// N = image width (& height)
template <uint N>
struct Image
{
    static const int width = N; // Image dimensions
    static const int height = N;
    static const int channels = 3;                     // RGB image
    static const int pixel_count = width * height;     // How many pixels ?
    static const size_t size = pixel_count * channels; // Data size in memory

    Vec2 center = Vec2{0, 0}; // What point in the plane we center on
    Scalar extents = 4.0_f;   // Half width of the plane represented by the image
    uchar *buffer = nullptr;  // Dynamically allocated memory for pixels

    Image()
        : buffer(new uchar[size])
    {
        memset(buffer, 0, size);
    }

    Image(const Vec2 &c, Scalar e)
        : extents(e), center(c), buffer(new uchar[size])
    {
        memset(buffer, 0, size);
    }

    void reset(const Vec2 &c, Scalar e)
    {
        center = c;
        extents = e;
        memset(buffer, 0, size);
    }

    ~Image()
    {
        delete[] buffer;
    }

    // Compute the color at pixel #index
    template <uint K>
    void simulate(const PendulumSim<K> *sim, int index)
    {

        CORE_ASSERT(index < pixel_count, "Wrong index");

        // Pixel coords
        const int yi = index % N;
        const int xi = index / N;

        // Compute the initial position in simulation matching pixel coords
        const Scalar x = (xi + 0.5_f) / N;
        const Scalar y = (yi + 0.5_f) / N;

        Vec2 pos = extents * (2 * Vec2{x, y} - Vec2::Ones()) + center;
        Vec2 vel = Vec2::Zero();

        ON_DEBUG(std::cout << "Sim " << index << "|" << xi << " " << yi << "|" << pos.transpose());

        // Simulate until we come to a rest
        const int max_iters = 10000; // Emergency stop for simulation loop
        int iter = 0;
        do
        {
            sim->update(pos, vel);
            ++iter;
        } while (vel.squaredNorm() > sim->vel_epsilon && iter < max_iters);

        ON_DEBUG(std::cout << " /  " << iter << " " << pos.transpose() << " " << vel.squaredNorm() << std::endl);

        // Choose color depending on nearest magnet
        int nearest = 0;
        Scalar d2_min = FLT_MAX;
        for (int i = 0; i < sim->num_magnets; ++i)
        {
            const Scalar d2 = (pos - sim->magnet_positions.col(i)).squaredNorm();
            if (d2 < d2_min)
            {
                nearest = i;
                d2_min = d2;
            }
        }

        // Black for nearest = 0 (do nothing)
        // Red for nearest = 1
        if (nearest > 0)
        {
            buffer[channels * index + 0] = 255;
        }
        // White for nearest = 2
        if (nearest > 1)
        {
            buffer[channels * index + 1] = 255;
            buffer[channels * index + 2] = 255;
        }
    }

    // Save image to file
    void save(const char *filename) const
    {
        stbi_write_png(filename, N, N, channels, buffer, N * channels);
    }

    // Render a full sim to an image, potentially on multiple threads
    template <uint K>
    void render(const PendulumSim<K> *sim, const char *filename)
    {
        const auto start = std::chrono::high_resolution_clock::now();

#ifdef SINGLE_THREADED
        const uint thread_count = 1;
        for (int i = 0; i < pixel_count; ++i)
        {
            simulate(sim, i);
        }
#else
        const uint thread_count = std::thread::hardware_concurrency();
        std::vector<std::thread> threads;
        threads.reserve(thread_count);

        for (uint t = 0; t < thread_count; ++t)
        {
            threads.emplace_back(
                [sim, this, t, thread_count] {
                    const int start = pixel_count * t / thread_count;
                    const int stop = std::min<int>(pixel_count, pixel_count * (t + 1) / thread_count);
                    for (int i = start; i < stop; ++i)
                    {
                        simulate(sim, i);
                    }
                });
        }
        for (auto &t : threads)
        {
            t.join();
        }
#endif
        const auto end = std::chrono::high_resolution_clock::now();
        const uint64 us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        std::cout << filename << ": " << us / 1000.0 << " ms  on " << thread_count << " threads (" << us / (pixel_count) << " us/pixel)" << std::endl;
        save(filename);
    }
};

int main()
{
    const Vec2 start = {0, 0};
    const Vec2 target = {1.325, 1.480};
    const Scalar start_ext = 4.0_f;
    const Scalar target_ext = 0.01_f;

    PendulumSim<3> sim;
    Image<1080> img;

    const int num_frames = 30 * 20;
    // Compute a full animation
    for (int i = 0; i < num_frames; ++i)
    {
        const Scalar a = Scalar(i) / (num_frames - 1);

        const Scalar tPos = std::min<Scalar>(2 * a, 1); // Reach target position halfway through
        const Vec2 center = tPos * target + (1 - tPos) * start;

        const Scalar tZoom = a * a * (3 - 2 * a); // smoothstep
        const Scalar zoom_level = tZoom * log(target_ext) + (1 - tZoom) * log(start_ext);
        const Scalar ext = exp(zoom_level);

        img.reset(center, ext);
        std::string name;
        core::stringPrintf(name, "frame%04d.png", i);
        img.render(&sim, name.c_str());
    }
    return 0;
}
