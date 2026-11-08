# Tetris AI in C

A search-based Tetris controller implemented in C. The program simulates legal placements for every rotation and horizontal position, evaluates the resulting board with domain-specific heuristics, and emits the highest-scoring move through a lightweight text protocol.

## Highlights

- Exhaustive move search across all four rotation states and legal horizontal positions.
- Collision-aware piece dropping on a compact 10x20 board model.
- Board simulation through cloned states before committing a move.
- Line clearing with standard single, double, triple, and Tetris scoring.
- Heuristic evaluation that rewards cleared lines and penalizes holes, surface-height variation, and excessive stack height.
- Deterministic command-line I/O suitable for automated game environments and agent evaluation.

## Decision Model

For each candidate placement, the controller:

1. Clones the current board.
2. Drops and places the active tetromino.
3. Clears completed lines.
4. Computes a weighted board score.
5. Selects the best rotation and x-coordinate.

The scoring model combines line rewards with structural penalties for hidden holes, uneven column heights, and dangerous stack growth. This makes the project a compact example of heuristic search, state simulation, and algorithmic decision-making in C.

## Build and Run

~~~bash
gcc Tetris.c -O2 -o tetris-ai
./tetris-ai
~~~

The program reads tetromino identifiers from standard input and outputs the selected rotation, x-coordinate, and current score. `X` or `E` ends the session.

## Repository Structure

- `Tetris.c` — complete implementation of the board model, piece geometry, simulator, heuristic evaluator, and controller.

## Technical Focus

C, structs and enums, array-based state modeling, collision detection, simulation, greedy heuristic optimization, and stream-based process communication.
