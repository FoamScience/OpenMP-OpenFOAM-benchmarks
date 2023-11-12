#include "ListOps.H"
#include "catch2/catch_all.hpp"
#include "catch2/catch_message.hpp"
#include "catch2/catch_test_macros.hpp"
#include "vector.H"
#include "Field.H"
#include "Random.H"
#include <functional>
#include <ranges>

// This file benchmarks a trivial expression template implementation against views
// for lazy evaluation

using namespace Foam;
label ELEMENTS_COUNT = 500000;

// Default mode of computation
template<class T>
Field<T> originalLoop(const scalarField& r) {
    Field<T> vf(ELEMENTS_COUNT, zero());
    auto W = [&](const scalar x) {
        return Foam::exp(-pow(1e-2*x,2));
    };
    scalarField rho(vf.size(), 1.0);
    scalarField m(vf.size(), 0.5);
    scalarField w(vf.size(), 0.0);
    forAll(vf, i) {
        vf[i] = m[i] * W(r[i]) / rho[i] + (m[i] + rho[i]) * W(r[i]) / m[i];
    }
    return vf;
}

#define THREAD_NUM 8

// OpenMP multi-threading mode of computation
template<class T>
Field<T> openMPLoop(const scalarField& r) {
    Field<T> vf(ELEMENTS_COUNT, zero());
    auto W = [&](const scalar x) {
        return Foam::exp(-pow(1e-2*x,2));
    };
    scalarField rho(vf.size(), 1.0);
    scalarField m(vf.size(), 0.5);
    scalarField w(vf.size(), 0.0);
    #pragma omp parallel for
    forAll(vf, i) {
        vf[i] = m[i] * W(r[i]) / rho[i] + (m[i] + rho[i]) * W(r[i]) / m[i];
    }
    return vf;
}

// OpenMP multi-threading mode of computation
template<class T>
Field<T> openMPGPULoop(const scalarField& r) {
    Field<T> vf(ELEMENTS_COUNT, zero());
    auto W = [&](const scalar x) {
        return Foam::exp(-pow(1e-2*x,2));
    };
    scalarField rho(vf.size(), 1.0);
    scalarField m(vf.size(), 0.5);
    scalarField w(vf.size(), 0.0);
    #pragma omp target
    #pragma omp teams distribute parallel for
    forAll(vf, i) {
        vf[i] = m[i] * W(r[i]) / rho[i] + (m[i] + rho[i]) * W(r[i]) / m[i];
    }
    return vf;
}

TEMPLATE_TEST_CASE
(
    "Execution time for OpenFOAM loops over a list of elements",
    "[cavity][serial][benchmark]",
    scalar
) {
    Info << "Running with OpenMP " << _OPENMP << nl;
    scalarField r(ELEMENTS_COUNT, 1.0);
    for (auto& ri: r) {
        ri = Random().sample01<scalar>();
    }
    auto ol = originalLoop<TestType>(r);
    auto omp = openMPLoop<TestType>(r);
    auto gpuomp = openMPGPULoop<TestType>(r);
    auto testExp = Catch::Matchers::Predicate<Field<TestType>>
    (
        [&ol](const Field<TestType>& result) {
        if (result.size() != ol.size()) return false;
        for (int i = 0; i < result.size(); ++i)
            if (Catch::Approx(result[i]).margin(1e-6) != ol[i]) return false;
        return true;
        },
        "Field elements must approximately match"
    );
    REQUIRE_THAT(omp, testExp);
    REQUIRE_THAT(gpuomp, testExp);
    BENCHMARK("Original looping") {
        return originalLoop<TestType>(r);
    };
    BENCHMARK("CPU openMP looping") {
        return openMPLoop<TestType>(r);
    };
    BENCHMARK("GPU OpenMP offloading") {
        return openMPGPULoop<TestType>(r);
    };
}
