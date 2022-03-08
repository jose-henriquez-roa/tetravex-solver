#include <string>
#include <vector>

class Tetravex {
public:
  struct Tile {
    uint8_t north;
    uint8_t south;
    uint8_t west;
    uint8_t east;
  };

  void
  load(const std::string& filePath);

  void
  solve(const float& initTemp,
        const float& deltaCoef,
        const float& tempRelax,
        const size_t& tempRelaxThreshold,
        const size_t& maxStepCountRelax,
        size_t maxStepCount,
        size_t& totalStepCount);

  void
  multithreadSolve();

  void
  print();

  void
  write(const std::string& filePath);
private:
  std::vector<uint8_t> m_board;

  std::vector<Tile> m_tiles;

  std::vector<uint8_t> m_freeTilePositions;

  uint8_t m_freeTileIdxStart;
};
