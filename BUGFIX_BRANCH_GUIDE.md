# Bugfix Branch Guide

## Role
Short-lived branches used to repair defects found during development or testing in the develop branch.

## Structure Guidelines
- Focus exclusively on the fix; do not introduce new features in this branch.
- Use the `fix: [description]` commit format.

## Merge Rules
- **Must** branch from `develop`.
- **Must** merge back into `develop` once the bug is verified as closed.
