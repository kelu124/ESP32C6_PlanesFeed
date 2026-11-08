---
name: commit-docs
description: Use before every git commit in this repo. Every commit must include a document in .claude/commits describing what changed and why.
---

# Documenting commits

Every commit includes one new file in `.claude/commits/`, staged in that same commit.

## File name
`YYYY-MM-DD-short-topic.md`. If the name is already taken, add `-2`, `-3` and so on.

## Contents
```markdown
# <commit subject>

Date: YYYY-MM-DD

## What changed
The main changes, grouped by area.

## Why
The reason for the change and any decision behind it.

## Testing
What was run and its result. Say plainly what was not tested.
```

## Before committing
1. Write the file and stage it with the rest of the change.
2. Check that the staged files contain no `.env` values: Wi-Fi details or personal coordinates.
3. Give the commit a message that explains what changed and why.

Tasks that are finished also move from `.claude/TODO.md` to `.claude/DONE.md`.
