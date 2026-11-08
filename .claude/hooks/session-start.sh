#!/usr/bin/env bash
# SessionStart hook: anything printed here is added to Claude's context,
# so each session opens with the list of pending actions.
set -euo pipefail

todo="${CLAUDE_PROJECT_DIR:-$(pwd)}/.claude/TODO.md"

if [[ -f "$todo" ]]; then
  echo "Pending actions from .claude/TODO.md. Review these with the user before starting new work:"
  echo
  cat "$todo"
else
  echo "No .claude/TODO.md found in this project."
fi
