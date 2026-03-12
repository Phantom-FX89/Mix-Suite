# Developer Agent Rules

## Global Rule for All Agents
Every agent prompt and execution assumes these universal operating rules:

### Universal Operating Doctrine

1. **Read Implementation Plan First**
   - Read `MISSION.md` and any active implementation plan before doing any work.
   - Pull only the details relevant to your assigned task.

2. **Use Repository Resources**
   - Use additional repository resources only as needed, including:
     - `specs/`
     - `docs/`
     - `data/`
     - `tests/`
     - plugin-specific folders
     - shared DSP / UI / core modules

3. **Handle Ambiguity**
   - If the implementation plan and the referenced repo materials do not provide enough information to complete the task cleanly, **prompt the user first** before proceeding.
   - Do not invent upstream systems, interfaces, parameter behavior, or dependencies that are not already specified.

4. **Respect Build Order**
   - Do not jump ahead of the build order or implement components that depend on unfinished upstream work.
   - Keep work scoped to your assigned ownership area and branch family.

5. **Handoff Protocol**
   - When handing off work, clearly state:
     - what was completed
     - what dependencies were assumed
     - what remains blocked
     - what downstream agents can now use
