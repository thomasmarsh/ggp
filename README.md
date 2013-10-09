- display code is unused - needs to be generalized
- generalize number of players (currently only BLACK vs. WHITE)
- refactor common code from each game
- define GDL
- add LocalStorage to Task<T,R>
- fix/generalize TD learner (need to write vectorized TD NN weight update)
- keep various statistics, with optional logging

- understand why http://beej.us/blog/data/monte-carlo-method-game-ai/ runs
  better than my flat-MC

- minimax search
        - implement quiescent search
        - allow easy plugging of minimax into monte carlo rollout
        - cache scores during minimax/AB/PVS search
                - add transposition tables

- monte carlo rollout
        - MC-RAVE
        - UCT-V / UCT-V(inf)
        - UCT-Tuned
        - MinimaxUCT
        - reduce MCTS memory consumption
        - GA evolver for bandit formulas

- board shapes
        - read-only board viewws (e.g., stacked pyramid top view)
        - Y board (hex tiling)
        - irregular tilings
        - N x M hex tilings
        - traditional hex board (parallelogram)
        - graph based (freeform) board
        - reduce memory footprint of board::Hex
                - consider: index based rather then coord based

        - Ludi has:
                - tilings: tri, square, hex, trunc-square (4.8.8)
                - shapes: tri, square, hex, rhombus, trapezium, boardless

- conditions
        - connect two or more regions
                - adjacency + stacking constraints
        - group
        - in-a-row
        - reach a specified goal (cell, region or side)
        - capture - specified number or specific piece
        - eliminate - remove opponents pieces
        - score - reach a target
        - state - achieve specified cell or piece state
        - no-move - no moves remaining

- generalize board traversals; for each board shape:
        - n-in-row
        - at least n-in-row
        - find groups
        - y connection
        - opposite side connection
        - surround capture (go)
        - surround capture suicide (go)
        - group touches opponent
        - liberty count
        - ring (Havannah)
        - bridge (connected corners)

- clean up / generalize bitboard
        - also, fix naming (e.g., 'clear()' vs 'iclear()')

- more games
        - Abalone (lots of move generation)
        - Gonnect (go capture semantics + no suicide)
        - Akron (pyramidal stacking board + complex moves)
        - Pentalath (hex board, go capture semantics + no suicide)
        - Druid's Walk (track additional marker)
        - Druid Span (new board type; what are rules exactly?)
        - Oust (multiple moves)
        - Limit (placement phase, graph based board)
        - Chad (looks interesting)
        - Arimaa (placement phase, piece hierarchy, push/pull)
