# Roadmap

These are proposed improvements, not completed features.

## 1. Export structured results

Add `--json` output containing map dimensions, algorithm, reachability, cost, steps, expansion count and route coordinates. Include a schema version and tests for unreachable maps and escaping. Keep existing CLI commands compatible.

## 2. Replay exploration in the report

Add optional playback controls using the recorded expansion order. Keep the static report readable when JavaScript is disabled, respect reduced-motion preferences, and avoid external assets. Verify keyboard control and reset behavior.

## 3. Compare multiple random seeds

Add a batch command with explicit seed range and CSV output. Report medians rather than a single fastest run. Keep timing separate from deterministic correctness results, and test invalid batch sizes.
