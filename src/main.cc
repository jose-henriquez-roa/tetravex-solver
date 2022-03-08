#include <iostream>
#include <string>
#include <chrono>

#include "tetravex.hh"

int
main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "tetravex-solver: Wrong number of inputs" << std::endl
              << "tetravex-solver: tetravex-solver in.txt out.txt" << std::endl;

  } else {
    Tetravex tetravex{};
    tetravex.load(argv[1]);
    std::cout << "Input:" << std::endl;
    tetravex.print();
    size_t totalStepCount;

    auto start = std::chrono::high_resolution_clock::now();
    tetravex.solve(0.5f,    // initTemp
                   1.0f,    // deltaCoef
                   0.1f,    // tempRelax
                   10000,   // tempRelaxThreshold 1000000
                   1000,    // maxStepCountRelax 100000
                   9843750, // maxStepCount 1000000
                   totalStepCount);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << duration.count() * 0.000001 << std::endl;

    std::cout << std::endl << "Solution:" << std::endl;
    tetravex.print();
    tetravex.write(argv[2]);
  }
}
