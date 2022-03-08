#include <iostream>
#include <fstream>
#include <random>
#include <algorithm>

#include "tetravex.hh"

#define UNUSED(x) (void)(x)

void
parseTile(const std::string& str, Tetravex::Tile& tile) {
  tile.north = str[0] - '0';
  tile.west = str[1] - '0';
  tile.east = str[2] - '0';
  tile.south = str[3] - '0';
}

void
shuffleFreeTiles(const uint8_t& freeTileIdxStart,
                 std::vector<uint8_t>& freeTilePositions,
                 std::vector<uint8_t>& board) {
  std::random_device rd;
  std::default_random_engine eng(rd());
  std::shuffle(freeTilePositions.begin(), freeTilePositions.end(), eng);

  for (size_t i = 0; i < freeTilePositions.size(); i++) {
    board[freeTilePositions[i]] = freeTileIdxStart + i;
  }
}

void
Tetravex::load(const std::string& filePath) {
  std::ifstream ifs(filePath);

  std::vector<Tile> freeTiles;
  std::vector<Tile> fixedTiles;

  std::string curToken;

  ifs >> curToken;

  Tile tile;
  parseTile(curToken, tile);

  uint8_t boardIdx = 0;
  std::vector<uint8_t> fixedSlotIndices;

  bool isFixed;
  while (ifs >> curToken) {
    isFixed = curToken == "@";
    if (isFixed == true) {
      fixedTiles.push_back(tile);
      fixedSlotIndices.push_back(boardIdx);

      ifs >> curToken;
      isFixed = curToken == "@";
    } else {
      freeTiles.push_back(tile);
      m_freeTilePositions.push_back(boardIdx);
    }

    parseTile(curToken, tile);

    boardIdx += 1;
  }

  if (isFixed == false) {
    freeTiles.push_back(tile);
    m_freeTilePositions.push_back(boardIdx);
  }

  m_freeTileIdxStart = fixedTiles.size();

  m_tiles.insert(m_tiles.end(), fixedTiles.begin(), fixedTiles.end());
  m_tiles.insert(m_tiles.end(), freeTiles.begin(), freeTiles.end());

  m_board.resize(m_tiles.size());

  for (size_t i = 0; i < fixedSlotIndices.size(); i++) {
    m_board[fixedSlotIndices[i]] = i;
  }
  
  shuffleFreeTiles(m_freeTileIdxStart, m_freeTilePositions, m_board);
}

void
computeLoss(const std::vector<Tetravex::Tile>& tiles,
            const std::vector<uint8_t>& board,
            int& loss) {
  loss = 0;

  int8_t boardSize = board.size();
  int8_t boardSide = std::sqrt(boardSize);

  for (int8_t i = 0; i < boardSize; i++) {
    const Tetravex::Tile& tile = tiles[board[i]];
    uint8_t tileRow = i / boardSide;

    int8_t southIdx = i + boardSide;
    if (southIdx < boardSize && tile.south == tiles[board[southIdx]].north) {
      loss -= 1;
    }

    int8_t eastIdx = i + 1;
    uint8_t eastRow = eastIdx / boardSide;
    if (eastIdx < boardSize && eastRow == tileRow &&
        tile.east == tiles[board[eastIdx]].west) {
      loss -= 1;
    }
  }
}

void
computeTileLoss(const std::vector<Tetravex::Tile>& tiles,
                const std::vector<uint8_t>& board,
                const int8_t& tileIdx,
                int& loss,
                const int8_t& otherTileIdx = -1) {
  loss = 0;

  int8_t boardSize = board.size();
  int8_t boardSide = std::sqrt(boardSize);

  Tetravex::Tile tile = tiles[board[tileIdx]];
  uint8_t tileRow = tileIdx / boardSide;

  int8_t northIdx = tileIdx - boardSide;
  if (northIdx >= 0 && northIdx != otherTileIdx &&
      tile.north == tiles[board[northIdx]].south) {
    loss -= 1;
  }

  int8_t southIdx = tileIdx + boardSide;
  if (southIdx < boardSize && southIdx != otherTileIdx &&
      tile.south == tiles[board[southIdx]].north) {
    loss -= 1;
  }

  int8_t westIdx = tileIdx - 1;
  uint8_t westRow = westIdx / boardSide;
  if (westIdx >= 0 && westRow == tileRow && westIdx != otherTileIdx &&
      tile.west == tiles[board[westIdx]].east) {
    loss -= 1;
  }

  int8_t eastIdx = tileIdx + 1;
  uint8_t eastRow = eastIdx / boardSide;
  if (eastIdx < boardSize && eastRow == tileRow && eastIdx != otherTileIdx &&
      tile.east == tiles[board[eastIdx]].west) {
    loss -= 1;
  }
}

void
swapTiles(const uint8_t& tileIdx01,
          const uint8_t& tileIdx02,
          std::vector<uint8_t>& board) {
  uint8_t tmp = board[tileIdx01];
  board[tileIdx01] = board[tileIdx02];
  board[tileIdx02] = tmp;
}

void
computeSwapLoss(const std::vector<Tetravex::Tile>& tiles,
                const uint8_t& tileIdx01,
                const uint8_t& tileIdx02,
                std::vector<uint8_t>& board,
                int& swapLoss) {
  // Remove loss induced by the two pieces
  int tile01Loss;
  computeTileLoss(tiles, board, tileIdx01, tile01Loss, tileIdx02);
  swapLoss -= tile01Loss;

  int tile02Loss;
  computeTileLoss(tiles, board, tileIdx02, tile02Loss);
  swapLoss -= tile02Loss;

  swapTiles(tileIdx01, tileIdx02, board);

  // Compute the new loss induced by the two pieces after the swap
  computeTileLoss(tiles, board, tileIdx01, tile01Loss, tileIdx02);
  swapLoss += tile01Loss;
  computeTileLoss(tiles, board, tileIdx02, tile02Loss);
  swapLoss += tile02Loss;
}

void
chooseTilesToSwap(const std::vector<uint8_t>& freeTilePositions,
                  std::default_random_engine& eng,
                  std::uniform_int_distribution<uint8_t>& intDistr,
                  uint8_t& tileIdx01,
                  uint8_t& tileIdx02) {
  tileIdx01 = freeTilePositions[intDistr(eng)];

  do {
    tileIdx02 = freeTilePositions[intDistr(eng)];
  } while (tileIdx01 == tileIdx02);
}

void
updateTemp(const size_t& maxStepCount, float& temp) {
  temp *= std::pow(1e-6f / temp, 1.f / maxStepCount);
}

void
checkSwapAccepted(const int& beforeLoss,
                  const int& afterLoss,
                  const float& temp,
                  const float& deltaCoef,
                  std::default_random_engine& eng,
                  std::uniform_real_distribution<float>& floatDistr,
                  bool& swapAccepted) {
  if (afterLoss < beforeLoss) {
    swapAccepted = true;
  } else {
    swapAccepted =
        std::exp(deltaCoef * (beforeLoss - afterLoss) / temp) > floatDistr(eng);
  }
}

void
printVerticalSeparator(const size_t& boardSide) {
  for (size_t i = 0; i < boardSide; i++) {
    std::cout << "+-----";
  }
  std::cout << "+" << std::endl;
}

void
printBoard(const std::vector<uint8_t>& board,
           const std::vector<Tetravex::Tile>& tiles) {
  size_t boardSize = board.size();
  size_t boardSide = std::sqrt(boardSize);

  printVerticalSeparator(boardSide);

  for (size_t i = 0; i < boardSize; i += boardSide) {
    for (size_t j = 0; j < boardSide; j++) {
      std::cout << "|  " << unsigned(tiles[board[i + j]].north) << "  ";
    }
    std::cout << "|" << std::endl;

    for (size_t j = 0; j < boardSide; j++) {
      std::cout << "| " << unsigned(tiles[board[i + j]].west) << " "
                << unsigned(tiles[board[i + j]].east) << " ";
    }
    std::cout << "|" << std::endl;

    for (size_t j = 0; j < boardSide; j++) {
      std::cout << "|  " << unsigned(tiles[board[i + j]].south) << "  ";
    }
    std::cout << "|" << std::endl;

    printVerticalSeparator(boardSide);
  }
}

void
Tetravex::solve(const float& initTemp,
                const float& deltaCoef,
                const float& tempRelax,
                const size_t& tempRelaxThreshold,
                const size_t& maxStepCountRelax,
                size_t maxStepCount,
                size_t& totalStepCount) {
  int loss;
  computeLoss(m_tiles, m_board, loss);

  std::random_device rd;
  std::default_random_engine eng(rd());
  std::uniform_real_distribution<float> floatDistr(0.0f, 1.0f);
  std::uniform_int_distribution<uint8_t> intDistr(
      0, m_freeTilePositions.size() - 1);

  size_t boardSide = std::sqrt(m_board.size());
  int optimalLoss = -2 * (boardSide - 1) * boardSide;
  size_t stepCount = 0;
  float temp = initTemp;
  totalStepCount = 0;
  while (loss != optimalLoss) {
    uint8_t tileIdx01;
    uint8_t tileIdx02;
    chooseTilesToSwap(m_freeTilePositions, eng, intDistr, tileIdx01, tileIdx02);

    int afterLoss = loss;
    computeSwapLoss(m_tiles, tileIdx01, tileIdx02, m_board, afterLoss);

    if (afterLoss == optimalLoss) {
      loss = afterLoss;
    } else {
      bool swapAccepted;
      checkSwapAccepted(
          loss, afterLoss, temp, deltaCoef, eng, floatDistr, swapAccepted);

      if (swapAccepted == true) {
        if (afterLoss == loss) {
          stepCount += 1;
        } else {
          stepCount = 0;
        }

        loss = afterLoss;
      } else {
        swapTiles(tileIdx01, tileIdx02, m_board);
        stepCount += 1;
      }

      if (stepCount == tempRelaxThreshold) {
        stepCount = 0;
        temp += tempRelax;

        maxStepCount += maxStepCountRelax;
      }

      updateTemp(maxStepCount, temp);
    }

    totalStepCount += 1;
  }
}

void
Tetravex::print() {
  printBoard(m_board, m_tiles);
}

void
Tetravex::write(const std::string& filePath) {
  std::ofstream ofs(filePath);

  for (const uint8_t& tileIndex : m_board) {
    const Tile& tile = m_tiles[tileIndex];
    ofs << unsigned(tile.north) << unsigned(tile.west) << unsigned(tile.east)
        << unsigned(tile.south) << std::endl;
  }
}
