# Design notes

## Data flow

```text
map file → validated Grid → search algorithm → Result → terminal / HTML report
```

`Grid` flattens coordinates as `row * columns + column`. Neighbor order is fixed: up, right, down, left. The CLI uses only validated or generated grids. Callers using headers directly are responsible for providing a valid `Grid`.

`Result` records the expansion order separately from the final path. This is useful for a future playback view and prevents conflating visited cells with the actual route.

## Correctness decisions

- BFS uses a FIFO queue and records each cell on its first discovery.
- Weighted algorithms relax an edge only when its cost improves.
- A priority-queue entry includes its known distance; stale entries are discarded.
- A* uses Manhattan distance with minimum entry cost 1. The heuristic is consistent under the current movement rules.
- Parent pointers reconstruct a route only after the goal has been expanded.
- Final route cost is recalculated from the returned path, including for BFS.

Grid dimensions and costs bound all path sums far below the maximum value of a 32-bit integer. Negative and zero weights are not accepted.

## Reporting

HTML is self-contained and script-free. The input filename is escaped before insertion into the document. The report uses inline SVG rectangles and labels, not copied icons, map tiles, web fonts or chart libraries. A narrow layout stacks algorithm cards vertically.

## Testing

The test executable covers a case where BFS is shorter but more expensive, an unreachable goal, malformed maps, BOM/CRLF parsing, deterministic generation, HTML escaping, and 100 weighted maps. Each returned weighted path must use legal adjacent cells; A* and Dijkstra must agree on cost.

For a proper benchmark, use repeated runs, a release build and a fixed machine. The displayed single-run microseconds are intentionally not used for assertions.
