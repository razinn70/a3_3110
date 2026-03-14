(Files content cropped to 300k characters, download full ingest to see more)
================================================
FILE: README.md
================================================
# Interpretable Context Methdology (ICM)

Folder structure as agent architecture.

ICM replaces framework-level orchestration with filesystem structure. Numbered folders represent stages. Markdown files carry the prompts and context that tell a single AI agent what role to play at each step. The result is a system where one agent, reading the right files at the right moment, does the work that would otherwise require a multi-agent framework.

**Created by Jake Van Clief**

---

## Why This Exists

There are genuinely good agentic frameworks available today. CrewAI, LangChain, AutoGen, and others handle multi-step orchestration, memory management, tool use, and error recovery. They work. But they work within their own structures, and adjusting those structures requires development work. Changing the order of steps, swapping a prompt, adding or removing a stage: these actions typically mean editing code, understanding abstractions, and redeploying.

For practitioners whose workflows are sequential and need human review at each step, the control surface can be much simpler.

ICM is built on an observation that is almost too simple to write down: if the prompts and context for each stage of a workflow already exist as files in a well-organized folder hierarchy, you do not need multiple agents or a coordination framework. You need one agent that reads the right files at the right moment. The folder structure tells it what to do at each step.

This is going backward before going forward. The principles that made Unix pipelines effective in the 1970s -- programs that do one thing, output of one becomes input of another, plain text as universal interface -- apply directly to AI agent orchestration today.

## Design Principles

Five ideas, each borrowed from established practice.

**One stage, one job.** Each stage handles a single step. A stage that researches does not also write. A stage that writes does not also build. This follows the Unix principle and Parnas's information-hiding criterion.

**Plain text as the interface.** Stages communicate through markdown files. No binary formats, no database connections, no proprietary serialization. Any tool that can read a text file can participate. Any human who can open a text editor can inspect or modify any artifact.

**Layered context loading.** Agents load only the context they need for the current stage. Less irrelevant context means better model performance. This is prevention rather than compression.

**Every output is an edit surface.** The intermediate output of each stage is a file a human can open, read, edit, and save before the next stage runs. The system picks up whatever the human left there.

**Configure the factory, not the product.** A workspace is set up once with the user's preferences, brand, style, and structural decisions. After that, each run of the pipeline produces a new deliverable using the same configuration.

## How It Works

Agents read down five layers and stop when they have what they need.

```
Layer 0: CLAUDE.md           "Where am I?"            Always loaded (~800 tokens)
Layer 1: CONTEXT.md          "Where do I go?"          Read on entry (~300 tokens)
Layer 2: Stage CONTEXT.md    "What do I do?"            Read per-task (~200-500 tokens)
Layer 3: Reference material  "What rules apply?"        Loaded selectively (varies)
Layer 4: Working artifacts   "What am I working with?"  Loaded selectively (varies)
```

Layers 3 and 4 are both content the agent loads while executing a stage, but they represent different kinds of context. Layer 3 is reference material -- design systems, voice rules, build conventions, domain knowledge. These files are configured once during setup and stay the same across every run. They are the factory. Layer 4 is working artifacts -- previous stage output, user-provided source material, anything specific to this run. These change every time.

The distinction matters because these layers require different things from the model. Layer 3 material needs to be internalized as constraints and patterns -- write *like this*, use *these colors*, follow *these conventions*. Layer 4 material needs to be processed as input -- transform *this research* into a script, convert *this script* into a specification.

A rendering agent might only need Layers 0 through 2. A script-writing agent reads down to Layer 4 to access both voice rules (Layer 3) and source material (Layer 4). No agent reads everything. The total context delivered at any given stage typically ranges from 2,000 to 8,000 tokens -- well within the range where current models perform at their best.

Contrast this with a monolithic approach where all stage instructions, all reference files, and all prior outputs are loaded into a single prompt. That approach can easily reach 30,000 to 50,000 tokens, pushing into the range where models start losing track of what matters.

A workspace looks like this:

```
workspace/
  CONTEXT.md               # Layer 1: task routing
  stages/
    01-research/
      CONTEXT.md            # Layer 2: stage contract
      references/           # Layer 3: reference material for this stage
      output/               # Layer 4: working artifacts
    02-script/
      CONTEXT.md
      references/
      output/
    03-production/
      CONTEXT.md
      references/
      output/
  _config/                  # Layer 3: brand, voice, design system
  shared/                   # Layer 3: cross-stage resources
  skills/                   # Layer 3: bundled domain knowledge
  setup/
    questionnaire.md        # One-time onboarding
```

The numbering encodes execution order. The folder boundaries enforce separation of concerns. The `output/` directories are the Layer 4 handoff points: the output of stage 01 becomes available as input to stage 02. If a human edits a file in `01-research/output/` before running stage 02, the agent picks up the edited version. The `references/` directories and `_config/` folder hold Layer 3 material -- stable knowledge that persists across runs.

Layer 2 is the control point. Each stage contract includes an Inputs table that specifies exactly which files from Layers 3 and 4 to load, and which sections of those files are relevant. Without this scoping, an agent would either load everything or guess. The Inputs table makes the selection explicit, editable, and auditable.

The filesystem is doing the work that a framework would otherwise do in code. Stage sequencing is the folder numbering. Context scoping is the folder hierarchy. State management is the files on disk. Coordination between stages is one folder's output being another folder's input.

## Stage Contracts

Each stage defines a contract in its CONTEXT.md with three parts: what it reads, what it does, and what it writes.

```markdown
## Inputs
| Source | File/Location | Section/Scope | Why |
|--------|--------------|---------------|-----|
| Previous stage | ../01-research/output/ | Full file | Source material |
| Style guide | ../../brand-vault/voice-rules.md | Voice Rules section | Tone guidance |

## Process
1. Read the research output
2. Identify the narrative angle
3. Write the script following voice-rules
4. Run audit checks
5. Save to output/

## Outputs
| Artifact | Location | Format |
|----------|----------|--------|
| Script | output/[slug]-script.md | Markdown with metadata header |
```

Creative stages also include **checkpoints** (where the agent pauses for human steering) and **audits** (quality checklists the agent runs before writing output). Not every stage needs these. Linear stages like extraction or rendering often run straight through. But any stage where the model is making creative decisions benefits from at least one checkpoint and a quality gate.

## Observability

The most useful property of this approach may be one that was not designed as a feature. Because every intermediate output is a plain file, the system is observable by default. There is no logging layer to build, no dashboard to configure, no special tooling to inspect pipeline state. You open a folder and read the files.

This is a glass-box AI workflow. It did not become transparent through the addition of an explanation layer. It was never opaque in the first place, because every artifact is a plain-text file that a human can read.

If stage 3 produces bad output, you know exactly where to look. You can read the stage's CONTEXT.md to see what instructions it received. You can read the input files to see what it was working with. You can edit the output and re-run the next stage. The entire system state is visible at all times because the system state is the filesystem.

## Portability

A workspace is a folder. It can be copied to another machine, committed to Git, emailed as a zip file, or synced through any cloud storage service. It carries its own prompts, its own context structure, its own stage definitions. There is no server to configure, no environment to replicate, no deployment step.

Every change to a prompt, every edit to a stage output, every configuration adjustment is diffable and reversible through standard version control. Stage outputs can be committed after each run, creating a version history of the entire pipeline's behavior over time.

If you build a workspace for a client's weekly reporting workflow, handing it over means copying a folder. The client can run it, edit the prompts to match their evolving needs, and adjust stages without involving a developer.

## Where This Works

ICM handles sequential multi-step workflows where a human reviews output at each stage. Content production pipelines. Research and analysis workflows. Monitoring and digest systems. Reporting processes. Training material development.

The common thread: these workflows are sequential (step 2 follows step 1), reviewable (a human should check each step's output), and repeatable (the same pipeline runs regularly with different input).

## Where This Does Not Work

ICM is not a replacement for multi-agent frameworks in every context.

**Real-time multi-agent collaboration** -- where agents need to communicate dynamically in tight loops -- requires message-passing infrastructure that frameworks like AutoGen provide. File-based handoffs are too slow for this.

**High-concurrency systems** -- where many users hit the same pipeline simultaneously -- need queueing, state isolation, and deployment infrastructure. ICM is local-first by design.

**Complex branching logic** -- where automated decisions mid-pipeline determine the next step -- is awkward in ICM. A human can make branching decisions between stages, but automated branching would require scripting that moves ICM toward being a framework itself.

The claim is not that ICM replaces existing tools across the board. The claim is that for a large and common class of workflows, the existing tools provide more complexity than the problem requires, and that complexity has real costs.

## A Note on MCP

It is worth distinguishing ICM from Anthropic's Model Context Protocol (MCP). MCP standardizes how models access external tools and data sources -- the integration problem between AI systems and the services they need to call. ICM addresses a different layer: how to structure and deliver context to an agent across a multi-stage workflow. The two are complementary. An ICM stage might use MCP connections to access external services, while the stage's folder structure determines what context the agent receives when doing so.

---

## Getting Started

1. Clone this repo
2. `cd workspaces/script-to-animation` (or any workspace)
3. Open [Claude Code](https://docs.anthropic.com/en/docs/claude-code)
4. Type `setup`
5. Answer the onboarding questions -- all at once, one pass
6. Your answers populate across the workspace files
7. Start producing -- give the agent a topic and walk through the stages

Each stage produces an output file. You can edit that file before moving on. The next stage reads whatever you left there.

## Available Workspaces

| Workspace | What it does | Stages |
|-----------|-------------|--------|
| [script-to-animation](workspaces/script-to-animation/) | Content idea through script writing, animation spec, and Remotion code | 3 |
| [course-deck-production](workspaces/course-deck-production/) | Unstructured material (PDFs, papers, notes) into polished PowerPoint slide decks | 5 |
| [workspace-builder](workspaces/workspace-builder/) | Build a new ICM workspace for any domain | 5 |

## Build Your Own Workspace

The workspace-builder is a workspace whose output is a new workspace. It follows ICM conventions to produce workspaces that follow ICM conventions.

1. `cd workspaces/workspace-builder`
2. Type `setup` to describe your domain
3. Walk through the five stages: discovery, mapping, scaffolding, questionnaire design, and validation
4. The output is a complete, ready-to-use workspace

The pattern transfers to any repeatable multi-step workflow: report generation, audit procedures, curriculum development, code documentation, or any process where someone currently does the same sequence of steps with different input material each time.

## The Conventions

Every workspace follows 15 patterns defined in [`_core/CONVENTIONS.md`](_core/CONVENTIONS.md). These are old ideas -- separation of concerns, one-way dependencies, canonical sources, pipe-and-filter architecture -- applied to the specific problem of structuring context for AI agents.

### Architecture

- **Stage contracts** -- Every stage CONTEXT.md has Inputs, Process, and Outputs. Simple enough for anyone to read. Structured enough for an agent to follow.
- **Stage handoffs** -- Output folders connect stages. Edit any output and the next stage picks up your changes.
- **One-way references** -- If A references B, B does not reference A. Prevents circular dependencies and scales linearly.
- **Selective section routing** -- CONTEXT.md tables specify which sections of which files to load. Not the whole file. The section you need.
- **Canonical sources** -- Every piece of information has one home. Other files point there. The moment the same rule exists in two files, they drift.

### Quality

- **Specs are contracts** -- Specification stages define WHAT and WHEN. They do not prescribe HOW. The build stage has creative freedom within the quality floor defined by the design system.
- **Checkpoints** -- Creative stages pause for human steering between steps. The agent completes a unit of work, presents options, and the human redirects before the next unit begins.
- **Stage audits** -- Quality checklists the agent runs after completing a stage but before writing output. Each check has an unambiguous pass condition.
- **Value validation** -- Content stages define what types of value their output delivers. The value is locked before drafting begins.
- **Docs over outputs** -- Reference docs are the authoritative source for how to build. Agents do not read previous outputs to learn patterns. Early outputs are the worst outputs. If future agents learn from them, quality never improves.

### Onboarding

- **Questionnaire design** -- Flat, all-at-once, system-level only. Configure the factory, not the product. Voice questions extract concrete examples, not descriptions.
- **Shared constants** -- Code-producing workspaces define shared constant files that all outputs import from. Change a value once, it updates everywhere.

## Contributing

**New workspaces are the main contribution.** If you have a repeatable workflow that benefits from staged human-in-the-loop AI automation, it probably belongs here.

### How to contribute a workspace

1. Fork the repo
2. Use the workspace-builder to create your workspace:
   ```
   cd workspaces/workspace-builder
   # Type "setup", describe your domain, walk through all 5 stages
   ```
3. The builder outputs a validated workspace to `workspaces/[your-workspace]/`
4. Test it: run `setup` in your new workspace, then run through the pipeline at least once
5. Open a PR

### What makes a good workspace

- **Repeatable workflow.** Something you or others will run many times, not a one-off task.
- **Clear stage boundaries.** Each stage produces a distinct artifact that a human might want to review or edit before proceeding.
- **System-level setup.** The questionnaire configures the production system (identity, design, preferences), not a specific run.
- **Follows ICM conventions.** The workspace-builder enforces this automatically. See [`_core/CONVENTIONS.md`](_core/CONVENTIONS.md) for the full spec.

### PR checklist

- [ ] Workspace was built using the workspace-builder (not hand-assembled)
- [ ] `setup` runs cleanly and all placeholders resolve
- [ ] At least one end-to-end run completed successfully
- [ ] No stage outputs committed (output folders should only contain `.gitkeep`)
- [ ] All CONTEXT.md files are under 80 lines
- [ ] All reference files are under 200 lines
- [ ] Creative stages have at least one checkpoint and an audit section
- [ ] No circular dependencies between stages

### Other contributions

- **Bug fixes** to existing workspaces or conventions
- **Improvements** to the workspace-builder itself
- **New patterns** for `_core/CONVENTIONS.md` (propose in an issue first)

## Origin

ICM grew out of a [content production system](https://github.com/RinDig/Content-Agent-Routing-Promptbase) that applies separation of concerns to AI context windows instead of code modules. That system runs a full content operation: scripting, animation specs, Remotion builds, brand management. ICM is the general-purpose version -- the structural patterns extracted so anyone can scaffold their own workflows.

For the academic treatment, see [Model Workspace Protocol: Folder Structure as Agent Architecture](link-to-paper) (Van Clief, 2026).

## License

MIT License. See [LICENSE](LICENSE) for details.



================================================
FILE: CLAUDE.md
================================================
# Model Workspace Protocol

MWP is a framework for building structured, multi-stage AI workflows out of markdown files and folder conventions. Each workspace gives AI agents the right context at each stage of a task, and gives humans clear edit surfaces between stages.

## Folder Map

```
model-workspace-protocol/
├── CLAUDE.md                          (you are here)
├── README.md                          (project overview)
├── LICENSE
├── _core/                             (shared conventions and templates)
│   ├── CONVENTIONS.md                 (source of truth for all patterns)
│   ├── placeholder-syntax.md          (how {{VARIABLES}} work)
│   └── templates/                     (blank starting points for new workspaces)
└── workspaces/
    ├── script-to-animation/           (content idea -> animated video)
    ├── course-deck-production/        (unstructured material -> course PowerPoints)
    └── workspace-builder/             (builds new MWP workspaces)
```

## Routing

| You want to... | Go to |
|-----------------|-------|
| Create content with script-to-animation | `workspaces/script-to-animation/CLAUDE.md` |
| Build course slide decks from source material | `workspaces/course-deck-production/CLAUDE.md` |
| Build a new workspace for any domain | `workspaces/workspace-builder/CLAUDE.md` |
| Read the full MWP specification | `_core/CONVENTIONS.md` |
| Understand the placeholder system | `_core/placeholder-syntax.md` |
| Use a template for a new workspace | `_core/templates/` |

## Triggers

| Keyword | Action |
|---------|--------|
| `setup` | Run onboarding in whatever workspace you are in |
| `status` | Show pipeline completion for the current workspace |

## How It Works

Each workspace is self-contained with its own CLAUDE.md. Navigate into a workspace folder and that workspace's CLAUDE.md takes over. You do not need to read this root file once you are inside a workspace.



================================================
FILE: LICENSE
================================================
MIT License

Copyright (c) 2026 Model Workspace Protocol Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.



================================================
FILE: MWP-PRD.md
================================================
# Product Requirements Document: Model Workspace Protocol (MWP)

## What This Is

MWP is an open-source framework for building structured, multi-stage AI workflows out of markdown files and folder conventions. Each "workspace" is a self-contained workflow scaffold that gives AI agents (specifically Claude Code agents) exactly the right context at each stage of a task, and gives humans clear edit surfaces between stages.

Think of it as development environments for AI work. The folder structure IS the product. The markdown files route agents to the right context, define stage boundaries, and create natural intervention points where a human can step in, edit outputs, and let the next stage pick up the edited version.

The repo ships as a collection of domain-specific workspaces that people clone and configure for their own use. Each workspace has an onboarding questionnaire that hydrates placeholder variables across the markdown files with real details (brand voice, target audience, project specifics, etc).

## Why This Exists

The core problem: most AI tooling gives you one shot. You prompt, you get output, you like it or you redo the whole thing. There is no way to isolate problems at specific stages of a multi-step workflow. If step 3 of 5 goes wrong, you often have to re-run from scratch.

MWP solves this by breaking workflows into discrete stages where each stage has its own agent context, its own output artifact, and its own edit surface. An agent at stage 3 knows what stages 1 and 2 produced because the routing tells it where to look. If stage 3 produces something off, you fix stage 3. You do not re-run the pipeline.

The secondary problem: context window management. Loading your entire content system into one conversation burns tokens on files the agent does not need and dilutes the context that matters. MWP gives each agent exactly the right context for its job and nothing else, through a tiered routing architecture.

## Target Users

Vibe coders and builders who use Claude Code. They can run terminal commands and navigate repos but may not have deep software engineering backgrounds. The system needs to be readable and navigable by someone who understands markdown and git basics. Every design decision should favor clarity over cleverness.

## The Three-Layer Routing Architecture

This is the core pattern that every workspace follows. Agents read down the layers, stopping as soon as they have what they need.

### Layer 0: CLAUDE.md (always loaded)
- Auto-loaded by Claude Code into every conversation
- Contains: folder map, ID systems, naming conventions, and a routing table that says "doing X? go to Y/CONTEXT.md"
- Purpose: orientation ("where am I?")
- Token cost: ~800 tokens, always present

### Layer 1: Top-level CONTEXT.md (read on entry)
- The first thing an agent reads when entering the workspace
- Contains: a task routing table that maps task types to specific stage folders
- Purpose: navigation ("where do I go?")
- Token cost: ~300 tokens, read once

### Layer 2: Stage-level CONTEXT.md files (read per-task)
- Each stage folder has its own CONTEXT.md
- Contains: scope definition, what-to-load tables (which files, which sections), and step-by-step process
- Purpose: instruction ("what do I do?")
- Token cost: ~200-500 tokens each

### Layer 3: Content files (loaded selectively)
- The actual reference material, loaded only when a CONTEXT.md table says to
- Contains: voice rules, templates, specs, whatever the stage needs
- Purpose: knowledge ("what do I need to know?")
- Token cost: varies, 500-3000 tokens each

An agent reads down the layers and stops when it has what it needs. A rendering agent might only need Layers 0-1. A script-writing agent reads down to Layer 3. No agent reads everything.

## Key Design Patterns to Enforce Across All Workspaces

### Pattern 1: Stage Contracts

Every stage CONTEXT.md follows the same three-section shape:

```markdown
## Inputs
What files to read and where to find them.

## Process
What the agent does, step by step.

## Outputs
What it produces and where it goes.
```

This is the contract. It is simple enough that a non-technical user can read it and understand what is happening, and structured enough that an agent can follow it reliably.

### Pattern 2: Stage Handoffs via Output Folders

Every stage has an `output/` subfolder. The agent writes its artifact there. The next stage's CONTEXT.md points to the previous stage's `output/` folder for its input. The convention:

- Stage N produces: `stages/0N-name/output/artifact-name.md`
- Stage N+1's CONTEXT.md says: "Read `../0N-name/output/artifact-name.md` as your input"

This is the handoff. A human can open the output file, edit it, and the next stage picks up the edited version because it just reads the file. No state management, no orchestration layer, just files in predictable places.

### Pattern 3: One-Way Cross-References

Every workspace folder points outward to what it needs. No folder points back. If workspace A references workspace B, workspace B does NOT reference workspace A. This prevents N-squared reference growth as the system scales.

### Pattern 4: Selective Section Routing

CONTEXT.md files do not just say "read voice-rules.md." They say "read the Voice Rules section of voice-rules.md." This way a 174-line file gets loaded as 80 lines of actionable rules. The other 94 lines of strategic rationale stay unloaded.

Format in CONTEXT.md tables:
```
| File | Section to Load | Why |
|------|----------------|-----|
| voice-rules.md | "Voice Rules" through "What the Voice Is NOT" | Tone guidance for writing |
```

### Pattern 5: Canonical Sources

Every piece of information has ONE home. Other files point there, they do not duplicate it. If you need to update a rule, you update it in one place. Every other file just has a pointer.

### Pattern 6: CONTEXT.md = Routing, Not Content

CONTEXT.md files answer three questions: What is this folder? What do I load? What is the process? They never contain the actual reference material. This keeps them small (25-80 lines) and prevents them from going stale.

## Placeholder and Onboarding System

### How Placeholders Work

Throughout the workspace markdown files, configurable details use double-brace placeholders:

```
{{BRAND_NAME}}
{{TARGET_AUDIENCE}}
{{VOICE_DESCRIPTION}}
{{PRIMARY_PLATFORM}}
```

These are NOT code variables. They are literal strings in markdown files that the onboarding agent finds and replaces with real content.

### How Onboarding Works

Each workspace has a `setup/` folder containing:

- `questionnaire.md`: Defines the questions the onboarding agent asks, grouped by category. Each question maps to one or more placeholders and specifies which files those placeholders appear in.
- `defaults.md` (optional): Default values for placeholders if the user wants to skip certain questions.

The onboarding flow:

1. User opens the workspace in Claude Code
2. User types `setup` (or whatever trigger keyword is in the workspace CLAUDE.md)
3. The agent reads `setup/questionnaire.md`
4. The agent asks the questions conversationally, collecting answers
5. The agent replaces placeholders across all specified files with the collected answers
6. If any answers indicate entire workflow branches are irrelevant (e.g., "I don't do short-form video"), the agent removes those stage folders entirely
7. The agent confirms completion and summarizes what was configured

### Questionnaire.md Format

```markdown
# Onboarding Questionnaire

## Brand Identity
These questions configure your brand-vault files.

### Q1: What is your brand or project name?
- Placeholder: {{BRAND_NAME}}
- Files: brand-vault/identity.md, stages/01-script/references/templates.md
- Type: free text

### Q2: Describe your voice in one sentence.
- Placeholder: {{VOICE_DESCRIPTION}}
- Files: brand-vault/voice-rules.md
- Type: free text
- Follow-up: If the answer is vague, ask for 2-3 adjectives that describe the tone.

### Q3: Who is your primary audience?
- Placeholder: {{TARGET_AUDIENCE}}
- Files: brand-vault/identity.md, stages/01-script/CONTEXT.md
- Type: free text

## Workflow Configuration
These questions determine which stages are active.

### Q4: Does your workflow include animation/video production?
- If NO: Remove stages/03-build/ entirely
- If YES: Continue to Q5

### Q5: What platform are you primarily creating for?
- Placeholder: {{PRIMARY_PLATFORM}}
- Files: stages/01-script/references/platform-specs.md
- Options: TikTok/Reels (vertical, 30-90s), YouTube (horizontal, 3-15min), Both
```

## Repo Structure

```
model-workspace-protocol/
├── README.md                              ← project overview, philosophy, getting started
├── LICENSE
├── CLAUDE.md                              ← meta-level: orients Claude Code to the repo itself
│
├── _core/                                 ← shared conventions and templates
│   ├── CONVENTIONS.md                     ← the rules above (stage contracts, handoffs, naming)
│   ├── placeholder-syntax.md              ← how {{VARIABLES}} work, replacement rules
│   └── templates/                         ← blank starting points for building new workspaces
│       ├── stage-context-template.md      ← the Inputs/Process/Outputs skeleton
│       ├── questionnaire-template.md      ← the onboarding question format
│       ├── workspace-claude-template.md   ← CLAUDE.md template for new workspaces
│       └── workspace-context-template.md  ← top-level CONTEXT.md template
│
├── workspaces/
│   │
│   ├── script-to-animation/               ← WORKSPACE 1: content script → animation pipeline
│   │   ├── CLAUDE.md                      ← workspace orientation + trigger keywords
│   │   ├── CONTEXT.md                     ← routing table for this workflow
│   │   ├── setup/
│   │   │   └── questionnaire.md           ← onboarding questions for this domain
│   │   ├── brand-vault/                   ← hydrated during onboarding
│   │   │   ├── CONTEXT.md                 ← routes agents to correct sections
│   │   │   ├── voice-rules.md             ← writing voice, tone, style constraints
│   │   │   └── identity.md                ← brand name, audience, positioning
│   │   ├── stages/
│   │   │   ├── 01-script/
│   │   │   │   ├── CONTEXT.md             ← Inputs/Process/Outputs contract
│   │   │   │   ├── output/                ← where finished scripts land
│   │   │   │   │   └── .gitkeep
│   │   │   │   └── references/            ← hook system, templates, pillar docs
│   │   │   │       ├── hook-system.md
│   │   │   │       ├── script-templates.md
│   │   │   │       └── content-pillars.md
│   │   │   ├── 02-spec/
│   │   │   │   ├── CONTEXT.md
│   │   │   │   ├── output/
│   │   │   │   │   └── .gitkeep
│   │   │   │   └── references/
│   │   │   │       ├── spec-format.md     ← BEATS constants, frame ranges, etc
│   │   │   │       ├── component-registry.md
│   │   │   │       └── design-system.md
│   │   │   └── 03-build/
│   │   │       ├── CONTEXT.md
│   │   │       ├── output/
│   │   │       │   └── .gitkeep
│   │   │       └── references/
│   │   │           └── build-conventions.md  ← Remotion patterns, file naming, etc
│   │   └── shared/                        ← files used across multiple stages
│   │       └── platform-specs.md          ← resolution, duration, format per platform
│   │
│   └── workspace-builder/                 ← WORKSPACE 2: builds new MWP workspaces
│       ├── CLAUDE.md
│       ├── CONTEXT.md
│       ├── setup/
│       │   └── questionnaire.md           ← asks about the domain being built for
│       ├── stages/
│       │   ├── 01-discovery/
│       │   │   ├── CONTEXT.md
│       │   │   └── output/
│       │   │       └── .gitkeep
│       │   ├── 02-mapping/
│       │   │   ├── CONTEXT.md
│       │   │   └── output/
│       │   │       └── .gitkeep
│       │   ├── 03-scaffolding/
│       │   │   ├── CONTEXT.md
│       │   │   └── output/
│       │   │       └── .gitkeep
│       │   └── 04-questionnaire-design/
│       │       ├── CONTEXT.md
│       │       └── output/
│       │           └── .gitkeep
│       └── references/
│           ├── conventions-reference.md   ← points to /_core/CONVENTIONS.md
│           └── examples/                  ← completed workspace examples to learn from
│               └── script-to-animation-summary.md
```

## Workspace 1: Script-to-Animation

This is the first domain workspace and serves as the reference implementation. It covers the workflow of turning a content idea into a finished animated video through three stages.

### Stage 01-script

**Purpose:** Take a topic/idea and produce a finished script ready for animation spec work.

**Inputs:**
- Topic or content idea (from user)
- brand-vault/voice-rules.md (the "Voice Rules" and "What the Voice Is NOT" sections)
- brand-vault/identity.md (the "One-Sentence Brand" and "Audience" sections)
- references/hook-system.md (hook patterns and examples)
- references/script-templates.md (structural templates for different content types)
- references/content-pillars.md (only the relevant pillar section)
- shared/platform-specs.md (duration and format constraints)

**Process:**
1. Identify which content pillar this topic falls under
2. Select a hook pattern from the hook system
3. Choose a script template based on content type
4. Write the script following voice rules
5. Check against platform specs for length/format
6. Save to output/

**Outputs:**
- A markdown file in `output/` containing the finished script with metadata header (pillar, hook type, target duration, platform)

**Human edit surface:** The finished script in `output/`. A user can open this, rewrite lines, adjust timing notes, change the hook, whatever they want. Stage 02 reads whatever is in that file.

### Stage 02-spec

**Purpose:** Take a finished script and produce an animation specification that defines the visual treatment for every line.

**Inputs:**
- stages/01-script/output/ (the finished script)
- references/spec-format.md (BEATS constants, frame-range syntax, inline prop format)
- references/component-registry.md (available animation components)
- references/design-system.md (colors, typography, motion rules)

**Process:**
1. Read the script from 01-script/output/
2. Break script into beats (logical visual segments)
3. For each beat, define: frame range, components used, props, transitions
4. Follow the spec format conventions
5. Reference only components that exist in the registry
6. Save to output/

**Outputs:**
- A markdown spec file in `output/` with beat-by-beat animation instructions

**Human edit surface:** The spec file. User can adjust timing, swap components, change visual treatments. Stage 03 reads whatever is in that file.

### Stage 03-build

**Purpose:** Take an animation spec and produce the actual code/build files.

**Inputs:**
- stages/02-spec/output/ (the animation spec)
- references/build-conventions.md (Remotion patterns, file structure, naming)
- references/component-registry.md (same as stage 02, for implementation reference)
- references/design-system.md (for exact values: hex colors, font sizes, easing curves)

**Process:**
1. Read the spec from 02-spec/output/
2. Create the Remotion component structure
3. Implement each beat as defined in the spec
4. Follow build conventions for file naming and organization
5. Save to output/

**Outputs:**
- Remotion/React component files in `output/`

**Human edit surface:** The actual code files. User can tweak animations, adjust timing, modify components directly.

## Workspace 2: Workspace Builder

This is the meta-workspace. It guides an agent through the process of creating a new MWP workspace for any domain. This proves the concept: if MWP can describe how to build MWP workspaces, the abstraction is general enough to describe anything.

### Stage 01-discovery

**Purpose:** Understand the domain workflow through conversation with the user.

**Inputs:**
- User conversation (the agent asks questions about the workflow being built)
- references/conventions-reference.md (so the agent knows MWP patterns)
- references/examples/script-to-animation-summary.md (concrete example to reference)

**Process:**
1. Ask the user to describe their workflow end to end
2. Identify the distinct stages (where does one task end and another begin?)
3. For each stage, ask: what goes in? what comes out? what does the agent need to know?
4. Identify what context is shared across stages vs. stage-specific
5. Identify what details are user-specific (these become questionnaire items)
6. Produce a workflow map document

**Outputs:**
- `output/workflow-map.md`: A structured document listing all stages, their inputs/outputs, shared context, and user-specific variables

### Stage 02-mapping

**Purpose:** Turn the workflow map into formal stage contracts and a dependency graph.

**Inputs:**
- stages/01-discovery/output/workflow-map.md
- /_core/CONVENTIONS.md (the rules for stage contracts)

**Process:**
1. Read the workflow map
2. For each stage, write the Inputs/Process/Outputs contract
3. Map cross-references (which stages read from which other stages)
4. Identify canonical sources (where does each piece of info live?)
5. Verify one-way references (no circular dependencies)
6. Produce the contracts document

**Outputs:**
- `output/stage-contracts.md`: Formal Inputs/Process/Outputs blocks for every stage, plus a dependency diagram

### Stage 03-scaffolding

**Purpose:** Generate the actual folder tree and CONTEXT.md files for the new workspace.

**Inputs:**
- stages/02-mapping/output/stage-contracts.md
- /_core/templates/ (all the template files)

**Process:**
1. Read the stage contracts
2. Create the folder structure (CLAUDE.md, CONTEXT.md, setup/, brand-vault or equivalent, stages/, shared/)
3. Populate each CONTEXT.md using the stage-context-template, filled with the contract info
4. Create placeholder reference files for each stage
5. Create the workspace CLAUDE.md with trigger keywords
6. Create the top-level CONTEXT.md routing table
7. Output the complete workspace folder

**Outputs:**
- `output/` contains the entire generated workspace folder structure, ready for the next stage to add the questionnaire

### Stage 04-questionnaire-design

**Purpose:** Build the onboarding questionnaire for the new workspace.

**Inputs:**
- stages/01-discovery/output/workflow-map.md (for the user-specific variables identified)
- stages/03-scaffolding/output/ (the generated workspace, to know which files contain placeholders)
- /_core/templates/questionnaire-template.md

**Process:**
1. Read the workflow map for user-specific variables
2. Read the scaffolded workspace to find all {{PLACEHOLDER}} instances
3. For each placeholder, write a clear question with context
4. Group questions by category (identity, workflow config, stage-specific)
5. Add conditional logic (if answer X, remove stage Y)
6. Map each question to the files and locations where its placeholder appears
7. Write the questionnaire.md

**Outputs:**
- `output/questionnaire.md`: The complete onboarding questionnaire, ready to be placed in the workspace's setup/ folder

After all four stages complete, the builder has produced a new, fully functional MWP workspace with onboarding.

## File Content Specifications

### Root CLAUDE.md

This file orients Claude Code to the entire repo. It should contain:

- One paragraph explaining what MWP is
- A folder map showing the repo structure (workspaces and core)
- A routing table: "Working on X? Go to workspaces/X/CLAUDE.md"
- A note that each workspace is self-contained and has its own CLAUDE.md
- The trigger keyword convention: typing `setup` in any workspace starts onboarding

### Root README.md

This is the GitHub-facing README. It should contain:

- What MWP is (the "app without code" pitch: structured markdown files that turn AI agents into controlled, multi-stage workflows)
- The core value: control at every stage, not just one-shot prompting
- The three-layer routing architecture explained simply
- How to get started (clone the repo, cd into a workspace, open in Claude Code, type `setup`)
- List of available workspaces with one-line descriptions
- How to build your own workspace (use workspace-builder)
- Link to CONVENTIONS.md for the full spec
- License info

### _core/CONVENTIONS.md

The full specification of MWP patterns. This is the source of truth for how workspaces are built. Should cover all six design patterns listed above (stage contracts, handoffs, one-way references, selective section routing, canonical sources, CONTEXT as routing). Include concrete examples for each pattern. This file should be thorough because the workspace-builder references it.

### _core/placeholder-syntax.md

How the placeholder system works:

- Syntax: `{{VARIABLE_NAME}}` with SCREAMING_SNAKE_CASE
- Replacement is literal string substitution in markdown files
- Placeholders can appear in any markdown file within a workspace
- The questionnaire.md maps each placeholder to its target files
- After onboarding, no placeholders should remain (the agent verifies this)
- Conditional placeholders: `{{?SECTION_NAME}}...{{/SECTION_NAME}}` wraps blocks that get removed if the corresponding questionnaire answer indicates they are not needed

### Workspace CLAUDE.md files

Each workspace CLAUDE.md should contain:

- One sentence: what this workspace does
- The folder map for this workspace specifically
- Trigger keywords: `setup` (run onboarding), `status` (show which stages have outputs), and any workspace-specific triggers
- The routing table: "Writing a script? → stages/01-script/CONTEXT.md"
- A note about the stage handoff convention: outputs go in output/ folders, next stage reads from there

### Stage CONTEXT.md files

Every stage CONTEXT.md follows this exact structure:

```markdown
# [Stage Name]

[One sentence: what this stage does]

## Inputs

| Source | File/Location | Section/Scope | Why |
|--------|--------------|---------------|-----|
| Previous stage | ../0N-prev/output/artifact.md | Full file | The artifact to work from |
| Brand vault | ../../brand-vault/voice-rules.md | "Voice Rules" section | Tone guidance |

## Process

1. [Step one]
2. [Step two]
3. [Step three]
...

## Outputs

| Artifact | Location | Format |
|----------|----------|--------|
| [Name] | output/[filename].md | [Description of format] |
```

## Build Instructions

Build this repo in the following order:

### Phase 1: Core infrastructure
1. Create the root folder structure
2. Write `_core/CONVENTIONS.md` (this is the source of truth, write it thoroughly)
3. Write `_core/placeholder-syntax.md`
4. Create all template files in `_core/templates/`
5. Write root `CLAUDE.md`
6. Write root `README.md`

### Phase 2: Script-to-animation workspace
1. Create the folder structure under `workspaces/script-to-animation/`
2. Write the workspace `CLAUDE.md`
3. Write the workspace `CONTEXT.md` (routing table)
4. Write `setup/questionnaire.md` with onboarding questions
5. Write `brand-vault/` files with `{{PLACEHOLDER}}` variables
6. Write each stage's `CONTEXT.md` following the Inputs/Process/Outputs contract format
7. Write reference files for each stage with appropriate placeholders
8. Write `shared/platform-specs.md`
9. Add `.gitkeep` files in all `output/` directories

### Phase 3: Workspace-builder workspace
1. Create the folder structure under `workspaces/workspace-builder/`
2. Write the workspace `CLAUDE.md`
3. Write the workspace `CONTEXT.md`
4. Write `setup/questionnaire.md` (asks about the domain being built for)
5. Write each stage's `CONTEXT.md`
6. Write `references/conventions-reference.md` (points to core conventions)
7. Create `references/examples/script-to-animation-summary.md` (summarizes workspace 1 as a learning example)

### Phase 4: Verification
1. Read through the entire repo and verify all cross-references point to real files
2. Verify no CONTEXT.md contains actual content (only routing)
3. Verify every placeholder has a corresponding questionnaire entry
4. Verify the stage handoff chain is unbroken (every stage's output is the next stage's input)
5. Verify no circular references exist

## Quality Standards

- Every markdown file should be readable by a non-technical person
- CONTEXT.md files should stay under 80 lines
- Reference files should stay under 200 lines (if longer, split them)
- Use plain English, avoid jargon
- No em dashes anywhere in the repo
- Folder and file names use lowercase-with-hyphens
- Stage folders use zero-padded numbers: 01-, 02-, 03-
- Every folder that should persist but starts empty gets a .gitkeep

## What Success Looks Like

When complete, a user should be able to:

1. Clone the repo
2. `cd workspaces/script-to-animation`
3. Open Claude Code
4. Type `setup`
5. Answer 8-12 questions about their brand, voice, audience, and platform
6. See their answers populated across all the workspace files
7. Start working through stages, with each stage's agent loading only its required context
8. Edit any intermediate output and have the next stage pick up their edits
9. Use `workspace-builder` to create an entirely new domain workspace from scratch

That is the product.



================================================
FILE: MWP-V2-PRD.md
================================================
# PRD: MWP v2 -- Battle-Tested Patterns

## Context

MWP v1 was built as a generalized framework for structured AI workflows. It works architecturally -- the 4-layer routing, stage handoffs, one-way references, placeholder system, and workspace builder are all sound.

But v1 was built from theory. The Content Writing workspace (the system MWP was abstracted from) has been in daily production use for months and has evolved significantly. The patterns that emerged from real use -- what actually makes agent output good vs mediocre -- never made it back into MWP.

This PRD defines 12 changes that bring MWP's conventions, templates, and example workspaces up to the standard of the production system. The goal: someone cloning MWP and running `setup` should get a workspace that produces output close to what the battle-tested system produces.

MWP should remain a simplified starting point, not a clone of the Content Writing workspace. These changes are about encoding the structural patterns that make agent output good, not copying domain-specific content.

---

## Reference System

The production system that these patterns were learned from lives at:

```
../                                (Content Writing workspace root)
├── CLAUDE.md                      Layer 0 routing
├── CONTEXT.md                     Layer 1 routing
├── script-lab/                    Script writing workspace
│   ├── CONTEXT.md
│   └── Script-Docs/               The 4-doc framework
│       ├── 01-voice.md            Hard constraints + verbatim examples
│       ├── 02-audience.md         Decision rules, not analytics
│       ├── 03-reel-format.md      Formats, value slots, hook rules
│       └── 04-workflow.md         3-step process with injection points
└── animation-studio/              Production workspace
    ├── CONTEXT.md
    ├── docs/
    │   ├── design-system.md       382 lines of code recipes + quality floor
    │   └── component-registry.md  Available components + conventions
    └── workflows/
        ├── CONTEXT.md             Pipeline routing + builder freedom rules
        └── 02-specs/
            └── spec-template.md   The contract-style spec format
```

When implementing these changes, read the corresponding file from the reference system to understand the pattern before modifying MWP. Do not copy content verbatim -- extract the structural pattern and make it domain-agnostic.

---

## Change 1: Spec Format -- Contracts, Not Blueprints

**Priority: 1 (highest -- cascades into changes 6, 9)**

### Problem

`workspaces/script-to-animation/stages/02-spec/references/spec-format.md` uses frame-range specs with explicit component names and props:

```markdown
### Beat 1 [0-90]
**Components:** PhoneFrame, TapRipple, TextReveal
**Props:**
- PhoneFrame: { screen: "message-app", scale: 0.8 }
```

This prescribes HOW, which removes creative freedom from the build stage and produces rigid, uncreative output.

### What the production system does

Specs define WHAT and WHEN. The builder decides HOW. A spec contains:

- **Beat Map** -- timing contract with approximate durations, narration text, and mood (not frame numbers)
- **Visual Philosophy** -- 3-5 paragraphs on what a muted viewer should understand, what visual density the video calls for
- **Key Moments** -- the 2-3 animations that MUST land for the piece to work, and WHY each matters
- **Persistent Elements** -- anything spanning multiple scenes
- **Audio Sync Points** -- specific narration words mapped to visual events
- **Color Flow** -- scene-by-scene dominant color and mood (not hex codes for every element)

No frame numbers. No component names. No pixel positions. No spring configs. No prop definitions.

Reference: `../animation-studio/workflows/02-specs/spec-template.md`

### What to change

1. Rewrite `spec-format.md` to use the contract-style format above
2. Make it domain-agnostic -- replace "animation" language with general "build" language where appropriate, but keep a concrete example that shows the pattern in action
3. Update the `02-spec` stage CONTEXT.md Process section to match: "define visual concepts and timing, not implementation"
4. Add the principle to CONVENTIONS.md as a new pattern: **Pattern 10: Specs Are Contracts** -- "Specification stages define WHAT the output should achieve and WHEN things happen. They do not prescribe HOW to implement. The build stage has creative freedom within the quality floor defined by the design system."

---

## Change 2: Add Checkpoints to Stage Contracts

**Priority: 2 (structural change to CONVENTIONS.md)**

### Problem

The stage contract template has Inputs / Process / Outputs. The Process section is a linear numbered list. There is no concept of pausing for human input within a stage.

### What the production system does

The script workflow has 3 steps with injection points BETWEEN them. Claude does a complete pass (e.g., generates 3-5 concept angles), then the human picks a direction, then Claude does the next pass. The key insight: injection points are between steps, not within them. The agent completes a unit of work, presents it, and the human steers.

Reference: `../script-lab/Script-Docs/04-workflow.md`

### What to change

1. Add a **Checkpoints** section to `_core/templates/stage-context-template.md`:

```markdown
## Checkpoints

<!-- Points where the agent pauses for human input before continuing.
     Not every stage needs checkpoints. Linear stages (extract, render, validate)
     often run straight through. Creative stages (writing, design, ideation)
     benefit from at least one.

     Format: after which process step, what the agent presents, what the human decides. -->

| After Step | Agent Presents | Human Decides |
|------------|---------------|---------------|
| [step #] | [what options/output to show] | [what direction to choose] |
```

2. Add to CONVENTIONS.md as **Pattern 11: Checkpoints** -- "Creative stages should include at least one checkpoint where the agent pauses and the human steers. The agent completes a full unit of work, presents options or a draft, and the human redirects before the next unit begins. Checkpoints go between process steps, not within them."

3. Update the script-to-animation `01-script` stage CONTEXT.md to demonstrate the pattern: a checkpoint between ideation and drafting.

---

## Change 3: Voice Rules as Error Conditions

**Priority: 4 (template change)**

### Problem

`voice-rules.md` template has `{{VOICE_TONE_RULE}}` -- a single placeholder sentence. The agent derives constraints from a vague description. Descriptions are abstractions the agent has to interpret. Rules with examples are pattern-matchable.

### What the production system does

Voice rules have three structural sections:

1. **Hard Constraints** -- numbered list of things that are always errors. "If the output contains any of these, rewrite." Each constraint is specific and checkable (e.g., "Antithesis pattern. Maximum one per script.", "Em dashes. Never.", "Filler transitions. Just start the next thought.").

2. **Sentence Rules** -- do-this/not-this pairs with verbatim examples. "Wrong: 'commoditization of basic services'. Right: 'you can prompt a landing page into existence now for a couple hundred dollars.'"

3. **Pacing** -- rhythm description with notation. "The rhythm alternates: setup, dense, dense, breath, dense. Not metronomic. Natural."

Reference: `../script-lab/Script-Docs/01-voice.md`

### What to change

1. Restructure `brand-vault/voice-rules.md` template to have three sections:

   - **Hard Constraints** -- "These are errors. If the output contains any of these, rewrite." Placeholder list items populated from questionnaire.
   - **Sentence Rules** -- Do/Don't pairs with example sentences. Populated from questionnaire.
   - **Pacing** -- Rhythm and density description. Populated from questionnaire.

   Keep the existing "Strategic Rationale" and "What the Voice Is NOT" sections.

2. Update the questionnaire (Change 11) to extract concrete rules, not descriptions.

---

## Change 4: Add Audit Section to Stage Contracts

**Priority: 2 (structural change to CONVENTIONS.md)**

### Problem

Stages have Inputs / Process / Outputs. There is no quality check before output is considered done. The agent writes output and moves on.

### What the production system does

Multiple quality audits built into the workflow:

- **Retention beat audit** (script stage) -- map every beat, tag it by type (Recognition, Surprise, Stack, Reversal, New Voice, Reframe, Implication), check that no gap exceeds 5 seconds without a beat
- **Share test** (script stage) -- "would someone who learned this 5 minutes ago feel confident sharing it?"
- **Mute test** (script/spec stage) -- "does the visual create curiosity without audio?"
- **Pre-flight checklist** (build stage) -- 15-item checklist covering hook, depth stack, surface treatment, typography, safe zones, motion

Reference: `../script-lab/Script-Docs/04-workflow.md` (retention beat audit), `../animation-studio/docs/design-system.md` (pre-flight checklist)

### What to change

1. Add an **Audit** section to `_core/templates/stage-context-template.md`, between Process and Outputs:

```markdown
## Audit

<!-- Quality checks before the output is considered done. The agent runs these
     after completing the process steps. If any check fails, revise before saving.

     Not every stage needs an audit. Data extraction or file conversion stages
     may not benefit. Creative and build stages almost always do. -->

| Check | Pass Condition |
|-------|---------------|
| [Check name] | [What "passing" looks like] |
```

2. Add to CONVENTIONS.md as **Pattern 12: Stage Audits** -- "Creative and build stages should include an Audit section: a checklist the agent runs after completing the process but before writing to output/. Audits catch quality issues before they propagate downstream. Each check should be specific enough that pass/fail is unambiguous."

3. Add audit sections to the script-to-animation workspace stages:
   - `01-script`: content quality checks (voice constraint violations, value delivery, hook timing)
   - `03-build`: production quality checklist (adapted from the design system pre-flight)

---

## Change 5: Value Framework

**Priority: 9 (conventions addition)**

### Problem

The script stage produces content but has no mechanism to validate what value that content delivers. Scripts can be structurally correct but not actually do anything for the viewer/reader.

### What the production system does

Four value slots: NOVEL, USABLE, QUESTION-GENERATING, INTERESTING. Every script must hit at least 2. The value is locked in Step 2 (before writing begins) so the draft has a clear target.

Reference: `../script-lab/Script-Docs/03-reel-format.md` (value slots section)

### What to change

1. Add to CONVENTIONS.md as **Pattern 13: Value Validation** -- "Content-producing stages should define what types of value their output can deliver. Before the main creative work begins (ideally at a checkpoint), the agent and human should agree on which value types this specific piece will hit. This prevents 'interesting but doesn't DO anything' output."

2. Add a `references/value-framework.md` file to the script-to-animation `01-script` stage with a placeholder-populated value framework. The questionnaire asks what kinds of value the user's content typically delivers (e.g., teaches something new, gives a usable tool, raises a question, shifts perspective).

3. Wire it into the script stage process: after the ideation checkpoint, the agent proposes which value types the piece will hit before drafting begins.

---

## Change 6: Builder Creative Freedom Within Quality Floor

**Priority: 5 (build-conventions template change)**

### Problem

`build-conventions.md` says "Frame ranges from the spec are authoritative. Do not adjust timing in the build." The build stage is positioned as a translator (spec to code) rather than a creator. The spec prescribes components and props, so the builder just implements what it is told.

### What the production system does

The spec is the creative contract (WHAT/WHEN). The design system is the quality floor (the production standard every output must meet). The builder has full creative latitude for HOW:

- Animation approach (which APIs, spring configs, easing)
- Layout (pixel positions, element sizing, composition)
- Component choices (reuse, customize, or build new)
- Visual interpretation (how to realize the spec's concepts in actual output)
- Transitions (fade, slide, wipe, custom)

The design system defines required quality patterns (e.g., depth stack, surface treatment) that every scene must have. Creative freedom means choosing how to implement those requirements, not whether to implement them.

Reference: `../animation-studio/workflows/CONTEXT.md` (Builder Creative Freedom section), `../animation-studio/docs/CONTEXT.md` (For Build Agents section)

### What to change

1. Rewrite `build-conventions.md` to establish the separation:
   - The spec defines the creative contract (what to achieve, when things happen)
   - The design system defines the quality floor (the minimum production standard)
   - The builder has creative freedom within both
   - Remove "do not adjust timing" -- replace with "the spec's timing is approximate; the builder calculates exact implementation"

2. Add the principle to the `03-build` stage CONTEXT.md Process section.

3. This change depends on Change 1 (spec format). The spec must stop prescribing components/props before the builder can have freedom.

---

## Change 7: Don't Learn From Existing Outputs

**Priority: 6 (one line in CONVENTIONS.md)**

### Problem

No guidance on whether agents should read other outputs in `output/` folders to learn patterns. Early outputs will be lower quality. If future agents copy from them, quality doesn't improve.

### What the production system does

"Do not explore existing clips or compositions. The docs above are the complete reference." This is enforced in 4 separate files. The docs (design system, component registry, skill rules) are the authoritative source. Previous outputs are artifacts, not templates.

Reference: `../animation-studio/docs/CONTEXT.md` (the "Do not explore existing clips" section)

### What to change

1. Add to CONVENTIONS.md as **Pattern 14: Docs Over Outputs** -- "Reference docs (design system, build conventions, skill rules) are the authoritative source for how to build. Previous stage outputs in output/ folders are artifacts, not templates. Agents should not read other outputs to learn patterns. This prevents copying from older, lower-quality work and ensures docs remain the single source of truth for quality standards."

2. Add a note to the build stage CONTEXT.md template: "After loading the reference docs and skill rules, you have the complete reference. Do not read other output/ files to learn patterns."

---

## Change 8: Constants as Shared Files

**Priority: 11 (conventions addition for code workspaces)**

### Problem

`build-conventions.md` has hardcoded color/font constants inline. After onboarding, these values are baked into each build output file separately. No shared source of truth for code values.

### What the production system does

A `constants/` folder with small, focused files (`colors.ts`, `timing.ts`, `typography.ts`, ~170 lines total). Every build imports from these. Change a color once, it updates everywhere.

Reference: `../animation-studio/src/compositions/Eduba/constants/`

### What to change

1. Add to CONVENTIONS.md as **Pattern 15: Shared Constants** -- "Workspaces that produce code should define a constants pattern. Configurable values (colors, fonts, timing, layout) live in shared files that all build outputs import from. The questionnaire populates these files once during onboarding. This is canonical sources (Pattern 5) applied to code values."

2. Update `build-conventions.md` to reference a shared constants file instead of inline values. The onboarding questionnaire writes color/font answers to a constants file, not scattered across build convention docs.

3. This is only relevant for code-producing workspaces (like script-to-animation with Remotion). Non-code workspaces can skip this pattern.

---

## Change 9: Design System as Code Recipes

**Priority: 12 (design system template addition)**

### Problem

The design system reference would contain generic rules and placeholder colors after onboarding. No concrete, copy-paste patterns for common build tasks.

### What the production system does

382 lines of concrete code examples:
- Depth stack recipe (9 layers, each with code)
- Surface treatment recipe (5 layers, full JSX)
- Camera drift code (copy-paste ready)
- Deterministic particle code (copy-paste ready)
- Spring configs table (by semantic meaning)
- Anti-patterns table (with "why it fails" column)
- Pre-flight checklist (15 items)

An agent can literally copy a recipe and adapt it. No interpretation required.

Reference: `../animation-studio/docs/design-system.md`

### What to change

1. Add a **Recipes** section to the design system reference template. Recipes are concrete, copy-paste patterns for common tasks. For the script-to-animation workspace, this means scene structure patterns, transition patterns, and timing patterns.

2. Add an **Anti-Patterns** table: what to avoid and why it fails. This is a quality guardrail that works across domains.

3. Add a **Production Checklist** at the end: a quick pre-flight the agent runs before considering a build done. (This connects to Change 4 -- the checklist can be referenced from the build stage audit.)

4. The design system template for new workspaces should include skeleton sections for Recipes, Anti-Patterns, and Production Checklist, even if the specific entries are domain-dependent.

---

## Change 10: Token Management Guidance

**Priority: 10 (CLAUDE.md template addition)**

### Problem

The workspace CLAUDE.md template has a routing table but no explicit guidance on what NOT to load. Users and agents don't understand that loading extra files actively degrades output quality.

### What the production system does

CLAUDE.md has a "Token Management" section:

```
- Writing a script? -> Load all 4 files in Script-Docs/
- Generating a spec? -> Load the script only
- Building animation? -> Follow this specific load order
- Rendering? -> Just the composition name and render settings
```

Plus the principle: "Each workspace is siloed. You don't need to load everything."

Reference: `../CLAUDE.md` (Token Management section)

### What to change

1. Add a **What to Load** section to `_core/templates/workspace-claude-template.md`:

```markdown
## What to Load

<!-- Map each task to its minimal file set. Loading more files dilutes quality.
     The context window is working memory, not storage. -->

| Task | Load These | Do NOT Load |
|------|-----------|-------------|
| [Task 1] | [minimal file list] | [what to skip and why] |
| [Task 2] | [minimal file list] | [what to skip and why] |
```

2. Add a note to CONVENTIONS.md under the routing architecture section: "Every token of irrelevant context is a token of diluted attention. Workspace CLAUDE.md files should explicitly map each task to its minimal required files."

---

## Change 11: Questionnaire Extracts Rules, Not Descriptions

**Priority: 7 (questionnaire template change)**

### Problem

The current questionnaire asks Q2: "How should your content sound? (describe your voice and give 2-3 adjectives)." The agent then derives tone rules, personality markers, and anti-patterns from a thin description. The most critical constraints in the system come from the weakest input signal.

### What the production system does

The voice rules are concrete: verbatim example sentences, numbered error conditions, do/don't pairs. These were built from real examples over months.

### What to change

1. Restructure the voice questions in the questionnaire to extract concrete rules:

   - "Give me 2-3 sentences that sound exactly like your brand." (extracts positive examples)
   - "Give me 2-3 sentences your brand would never say." (extracts anti-patterns)
   - "List things that are always errors in your content -- patterns, phrases, or habits to avoid." (extracts hard constraints)
   - Keep the adjective question as supplementary, not primary.

2. Consider making the questionnaire a **two-pass process**: quick answers first, then the agent generates draft voice rules and the human edits them before they are baked in. This mirrors the checkpoint pattern (Change 2) and produces better rules than one-shot derivation.

3. Update the questionnaire template in `_core/templates/questionnaire-template.md` to note: "For voice/style questions, ask for concrete examples (sentences that sound right, sentences that sound wrong) rather than abstract descriptions. Examples are pattern-matchable. Descriptions require interpretation."

---

## Change 12: Specific Process Steps

**Priority: 8 (template guidance)**

### Problem

Stage process sections use generic verbs: "Write the script following voice rules." "Break script into beats." Two different agents following these steps would produce structurally different outputs.

### What the production system does

Process steps are specific enough that agents produce structurally consistent output:

- "Offer 3-5 concept angles, each framed as a single sentence a viewer could repeat to a friend"
- "For each angle, tag which value slots it naturally hits and which format it leans toward"
- "Claude writes the full script in one pass, then immediately audits against: voice hard constraints, value delivery, hook timing, close quality, share test"

Reference: `../script-lab/Script-Docs/04-workflow.md`

### What to change

1. Add guidance to the stage-context-template.md comments:

```markdown
## Process

<!-- Numbered steps. Each step is one concrete action. Be specific enough that
     two different agents following these steps would produce structurally similar
     outputs.

     Too vague: "Write the script"
     Good: "Write the full script in one pass, then audit against the voice
            hard constraints and value brief"

     Too vague: "Generate ideas"
     Good: "Propose 3-5 concept angles, each as a single sentence. Tag each
            with its value type and format."  -->
```

2. Update the script-to-animation `01-script` process steps to demonstrate the level of specificity expected. Replace "Write the script following voice rules" with the actual multi-step process: propose angles (with tags) -> checkpoint -> propose value brief -> checkpoint -> write full draft -> run audit.

3. This is guidance, not a strict rule. Some stages (data extraction, file conversion) genuinely are simple steps. The specificity matters most for creative and build stages.

---

## Implementation Order

The changes have dependencies. Implement in this order:

### Phase 1: Structural Changes to CONVENTIONS.md

These add new patterns that all workspaces follow.

1. **Change 4** -- Add Audit section to stage contract template
2. **Change 2** -- Add Checkpoints section to stage contract template
3. **Change 7** -- Add "Docs Over Outputs" pattern
4. **Change 5** -- Add "Value Validation" pattern
5. **Change 10** -- Add token management note

After Phase 1, `CONVENTIONS.md` has 5 new patterns (10-14) and the stage-context-template has two new optional sections (Checkpoints, Audit).

### Phase 2: Template Updates

These improve the templates that new workspaces are built from.

6. **Change 3** -- Restructure voice-rules.md template
7. **Change 11** -- Restructure questionnaire to extract rules
8. **Change 12** -- Add specificity guidance to stage-context-template
9. **Change 10** -- Add "What to Load" section to workspace-claude-template

### Phase 3: Script-to-Animation Workspace Updates

These update the reference workspace to demonstrate all new patterns.

10. **Change 1** -- Rewrite spec-format.md (contract style)
11. **Change 6** -- Rewrite build-conventions.md (creative freedom + quality floor)
12. **Change 8** -- Add shared constants pattern
13. **Change 9** -- Add code recipes to design system reference
14. Update all three stage CONTEXT.md files to use Checkpoints, Audits, and specific process steps
15. Update the questionnaire with new voice questions

### Phase 4: Workspace Builder Updates

16. Update the workspace-builder's scaffolding stage to generate workspaces with the new template sections (Checkpoints, Audits)
17. Update the workspace-builder's references/examples to reflect the updated script-to-animation workspace

### Phase 5: Validation

18. Run through the script-to-animation workspace end-to-end (setup -> script -> spec -> build) and verify the new patterns produce better output
19. Verify all cross-references still point to real files
20. Verify CONVENTIONS.md, templates, and example workspace are internally consistent

---

## What NOT to Change

These parts of MWP v1 are working and should be preserved:

- The 4-layer routing architecture (Layers 0-3)
- The placeholder and onboarding system (`{{VARIABLE}}` syntax, conditional sections)
- Stage handoffs via output folders
- One-way cross-references (Pattern 3)
- Selective section routing (Pattern 4)
- Canonical sources (Pattern 5)
- CONTEXT.md = routing only (Pattern 6)
- Trigger keywords (setup, status)
- Naming conventions
- Quality guardrails (line limits, no em dashes, plain English)
- The workspace-builder's 5-stage structure
- Bundled skills pattern (Pattern 9)
- The overall repo structure (_core/, workspaces/)

---

## Success Criteria

After all changes:

1. A new user can clone MWP, run `setup` in script-to-animation, answer 10 questions, and get a workspace where:
   - The script stage has a collaborative workflow with checkpoints, not a linear checklist
   - Voice rules are concrete constraints with examples, not derived from adjectives
   - The spec stage produces contracts (WHAT/WHEN), not blueprints (HOW)
   - The build stage has creative freedom within a documented quality floor
   - Every stage has quality audits before output is written

2. The workspace-builder produces new workspaces that automatically include Checkpoint and Audit sections in their stage templates.

3. CONVENTIONS.md is the complete reference for all MWP patterns (original 9 + new 5 = 14 patterns, plus the shared constants pattern for code workspaces).



================================================
FILE: _core/CONVENTIONS.md
================================================
# MWP Conventions

The rules for building and maintaining MWP workspaces. This is the canonical source. Every workspace follows these patterns.

---

## Five-Layer Routing Architecture

Agents read down the layers. They stop as soon as they have what they need.

```
Layer 0: CLAUDE.md           -> "Where am I?"            (always loaded, ~800 tokens)
Layer 1: CONTEXT.md          -> "Where do I go?"          (read on entry, ~300 tokens)
Layer 2: Stage CONTEXT.md    -> "What do I do?"            (read per-task, ~200-500 tokens)
Layer 3: Reference material  -> "What rules apply?"        (loaded selectively, varies)
Layer 4: Working artifacts   -> "What am I working with?"  (loaded selectively, varies)
```

**Layer 0 -- CLAUDE.md** is auto-loaded by Claude Code into every conversation. It contains the folder map, naming conventions, and a routing table that points to workspace-level files. One per workspace.

**Layer 1 -- Top-level CONTEXT.md** is the first thing an agent reads when entering the workspace. It contains a task routing table that maps task types to specific stage folders. One per workspace.

**Layer 2 -- Stage CONTEXT.md files** live inside each stage folder. They contain the scope definition, what-to-load tables, and step-by-step process. One per stage. Layer 2 is the control point of the system -- its Inputs table determines exactly which files from Layers 3 and 4 the agent loads.

**Layer 3 -- Reference material** is the persistent context: design systems, voice rules, build conventions, style guides, domain knowledge bundled as skill files. These files are configured once during workspace setup and remain stable across every run of the pipeline. They live in `references/` folders within stages, in workspace-level configuration folders (like `brand-vault/` or `design-system/`), in `shared/`, and in `skills/`. Larger reference collections can include their own CONTEXT.md routing files to help agents navigate within the collection.

**Layer 4 -- Working artifacts** are the per-run context: previous stage outputs, user-provided source material, anything specific to this particular run. These files are produced and consumed during execution and change every time the pipeline runs. They live in `output/` folders.

The distinction between Layers 3 and 4 matters because they require different things from the model. Layer 3 material needs to be internalized as constraints and patterns -- write like this, use these colors, follow these conventions. Layer 4 material needs to be processed as input -- transform this research into a script, convert this script into a specification. Layer 3 is the factory. Layer 4 is the product.

A rendering agent might only need Layers 0 through 2. A script-writing agent reads down to Layer 4 to access both voice rules (Layer 3) and source material (Layer 4). No agent reads everything.

Every token of irrelevant context is a token of diluted attention. Workspace CLAUDE.md files should explicitly map each task to its minimal required files. Loading more context does not make output better. It makes it worse.

---

## Pattern 1: Stage Contracts

Every stage CONTEXT.md follows the same three-section shape:

```markdown
## Inputs

| Source | File/Location | Section/Scope | Why |
|--------|--------------|---------------|-----|
| ... | ... | ... | ... |

## Process

1. Step one
2. Step two
3. Step three

## Outputs

| Artifact | Location | Format |
|----------|----------|--------|
| ... | ... | ... |
```

This is the contract. It is simple enough that a non-technical user can read it and understand what is happening. It is structured enough that an agent can follow it reliably. Every stage follows this exact shape. No exceptions.

---

## Pattern 2: Stage Handoffs via Output Folders

Every stage has an `output/` subfolder. The agent writes its artifact there. The next stage reads from the previous stage's `output/` folder.

The convention:
- Stage N produces: `stages/0N-name/output/artifact-name.md`
- Stage N+1's CONTEXT.md says: "Read `../0N-name/output/artifact-name.md` as your input"

This is the handoff. A human can open the output file, edit it, and the next stage picks up the edited version. No state management. No orchestration layer. Just files in predictable places.

File naming in output folders: `[topic-slug]-[stage-artifact].md`
- Example: `hello-world-script.md`, `hello-world-spec.md`

---

## Pattern 3: One-Way Cross-References

Every folder points outward to what it needs. No folder points back.

If Stage 03 references Stage 02's component registry, Stage 02 does NOT reference anything in Stage 03. If the brand-vault is referenced by multiple stages, the brand-vault does NOT reference any stage.

This prevents reference growth from going N-squared as the system scales. When adding a new reference, check: "Does the target file already reference my folder?" If yes, restructure.

---

## Pattern 4: Selective Section Routing

CONTEXT.md Inputs tables do not just say "read voice-rules.md." They say "read the Voice Rules section of voice-rules.md."

This keeps token cost low. A 150-line file might have only 60 lines of actionable rules for a specific stage. The other 90 lines of strategic rationale stay unloaded.

Format in CONTEXT.md Inputs tables:

```
| File | Section to Load | Why |
|------|----------------|-----|
| voice-rules.md | "Voice Rules" through "What the Voice Is NOT" | Tone guidance |
| identity.md | "One-Sentence Brand" and "Audience" sections | Audience context |
```

When a full file is needed, write "Full file" in the Section/Scope column.

---

## Pattern 5: Canonical Sources

Every piece of information has ONE home. Other files point there. They do not duplicate it.

If you need to update a rule, you update it in one place. Every other file has a pointer. If you find the same information in two files, one of them should be replaced with a reference to the other.

Smell test: search the repo for a specific phrase. If it appears in more than one file and both instances are meant to be authoritative, one needs to become a pointer.

---

## Pattern 6: CONTEXT.md = Routing, Not Content

CONTEXT.md files answer three questions:
1. What is this folder?
2. What do I load?
3. What is the process?

They never contain the actual reference material. No definitions. No rules. No extended examples. No voice guidelines. This keeps them small (25-80 lines) and prevents them from going stale when the content they would otherwise duplicate gets updated.

If you find yourself writing more than a one-sentence description in a CONTEXT.md, that content belongs in a separate file that the CONTEXT.md points to.

---

## Pattern 7: Tool Prerequisites

Some stages require external tools (Node.js, LibreOffice, ffmpeg, etc.). Setup guides for these tools live in the `references/` folder of the stage that uses them (e.g., `stages/03-build/references/remotion-setup.md`).

Setup guides are written for someone who has never installed the tool: what it is (one sentence), installation steps, how to verify it works, and how the workspace uses it.

If a tool is needed by multiple stages, it can live in `shared/` instead. The `setup` onboarding process should check which tools are needed based on the user's answers and point them to the right setup guide.

Note: When a workspace bundles skills (Pattern 9), many tools that would have needed separate prerequisites (scripts, libraries, utilities) come bundled inside the skill folder. Only tools that require system-level installation (Node.js, Python, LibreOffice) still need setup guides.

---

## Trigger Keywords

Every workspace recognizes these triggers:

**`setup`** -- Starts the onboarding questionnaire. The agent reads `setup/questionnaire.md`, asks the questions conversationally, collects answers, replaces placeholders across the workspace, and verifies no placeholders remain.

**`status`** -- Shows pipeline completion. The agent scans all `stages/*/output/` folders and renders an ASCII pipeline diagram:

```
Pipeline Status: [workspace-name]

  [01-stage-name]  ------>  [02-stage-name]  ------>  [03-stage-name]
     COMPLETE                  PENDING                  PENDING
  (artifact.md)              (empty)                  (empty)
```

For each stage: if the output folder contains files (other than .gitkeep), the stage is COMPLETE and the filenames are listed. If the output folder is empty or contains only .gitkeep, the stage is PENDING.

Workspaces can define additional trigger keywords in their own CLAUDE.md.

---

## Naming Conventions

- Folders and files: `lowercase-with-hyphens`
- Stage folders: zero-padded numbers prefix: `01-`, `02-`, `03-`
- Placeholders: `{{SCREAMING_SNAKE_CASE}}`
- Output artifacts: `[topic-slug]-[artifact-type].md`
- No spaces in file or folder names

---

## Pattern 8: Questionnaire Design

Onboarding questionnaires configure the production system, not a specific run. They follow these rules:

1. **Flat structure.** No category groupings. Just a numbered list of questions.
2. **All at once.** Every question appears in one pass. The user should be able to answer everything in a single message.
3. **System-level only.** Questions configure things that stay the same across runs: identity, brand, design, tool preferences, default workflow. Per-run details (project name, topic, audience, scope) are collected conversationally at the start of each pipeline run by the entry stage.
4. **Derive, do not ask.** If a field can be inferred from another answer, the agent fills it in. List derived fields under the question they depend on. Do not add a separate question.
5. **Sensible defaults.** Every question should have a default or example so the user can skip what they do not care about.
6. **Ask once, never again.** After setup, the user should never see these questions again. The answers are baked into the workspace files permanently.

The questionnaire template at `_core/templates/questionnaire-template.md` encodes these rules.

---

## Pattern 9: Bundled Skills

Workspaces can bundle Claude Code skills directly into a `skills/` folder. This gives agents domain-specific knowledge (APIs, best practices, code examples) without requiring the user to have the skills installed globally.

```
workspace/
├── skills/
│   ├── [skill-name]/          (copied from ~/.claude/skills/ or cloned from GitHub)
│   │   ├── SKILL.md           (skill entry point)
│   │   ├── rules/             (detailed rule files, if any)
│   │   └── scripts/           (utility scripts, if any)
│   └── [another-skill]/
│       └── SKILL.md
```

**Discovery:** During workspace building (Stage 01), the builder identifies relevant skills by:
1. Scanning `~/.claude/skills/` and `~/.agents/skills/` for locally installed skills
2. Searching GitHub for skill repos matching the workspace domain (e.g., "remotion skill", "pptx skill")
3. Presenting candidates to the user for selection

**Bundling:** Selected skills are copied (local) or cloned (GitHub) into the workspace's `skills/` folder during scaffolding (Stage 03). This makes the workspace self-contained.

**Referencing:** Stage CONTEXT.md files reference skills in their Inputs table:

```
| Skill | `../../skills/[name]/SKILL.md` | Index, then load rules as needed | [What it provides] |
```

Skills replace custom reference docs when an official skill covers the same ground. Keep workspace-specific files (design systems, brand config, build conventions) alongside skills, not inside them.

**When NOT to bundle:** Do not bundle skills that are purely about Claude Code itself (e.g., skill-creator, mcp-builder). Only bundle skills that provide domain knowledge the workspace's agents need at runtime.

---

## Pattern 10: Specs Are Contracts

Specification stages define WHAT the output should achieve and WHEN things happen. They do not prescribe HOW to implement. The build stage has creative freedom within the quality floor defined by the design system.

A spec contains:
- **Beat map** with approximate durations, narration, and mood
- **Visual philosophy** describing what a muted viewer should understand
- **Key moments** that MUST land, and why each matters
- **Audio sync points** mapping narration words to visual events
- **Color flow** with per-scene dominant color and mood

A spec does NOT contain: frame numbers, component names, pixel positions, spring configs, or prop definitions. These are implementation decisions that belong to the build stage.

---

## Pattern 11: Checkpoints

Creative stages should include at least one checkpoint where the agent pauses and the human steers. The agent completes a full unit of work, presents options or a draft, and the human redirects before the next unit begins. Checkpoints go between process steps, not within them.

Not every stage needs checkpoints. Linear stages (extract, render, validate) often run straight through. Creative stages (writing, design, ideation) benefit from at least one.

The Checkpoints section in a stage CONTEXT.md is a table:

```
| After Step | Agent Presents | Human Decides |
|------------|---------------|---------------|
| [step #] | [what to show] | [what to choose] |
```

---

## Pattern 12: Stage Audits

Creative and build stages should include an Audit section: a checklist the agent runs after completing the process but before writing to output/. Audits catch quality issues before they propagate downstream. Each check should be specific enough that pass/fail is unambiguous.

Not every stage needs an audit. Data extraction or file conversion stages may not benefit. Creative and build stages almost always do.

The Audit section in a stage CONTEXT.md is a table:

```
| Check | Pass Condition |
|-------|---------------|
| [Check name] | [What "passing" looks like] |
```

If any check fails, the agent revises before saving to output/.

---

## Pattern 13: Value Validation

Content-producing stages should define what types of value their output can deliver. Before the main creative work begins (ideally at a checkpoint), the agent and human should agree on which value types this specific piece will hit. This prevents "interesting but doesn't DO anything" output.

Value types are workspace-specific. A content workspace might use NOVEL, USABLE, QUESTION-GENERATING, INTERESTING. A course workspace might use TEACHES, PRACTICES, CHALLENGES. The framework is defined once in a reference file and used at every checkpoint.

---

## Pattern 14: Docs Over Outputs

Reference docs (design system, build conventions, skill rules) are the authoritative source for how to build. Previous stage outputs in `output/` folders are artifacts, not templates. Agents should not read other outputs to learn patterns.

This prevents copying from older, lower-quality work and ensures docs remain the single source of truth for quality standards. Early outputs are the worst outputs. If future agents learn from them, quality never improves.

---

## Pattern 15: Shared Constants

Workspaces that produce code should define a constants pattern. Configurable values (colors, fonts, timing, layout) live in shared files that all build outputs import from. The questionnaire populates these files once during onboarding. Change a value once, it updates everywhere.

This is Pattern 5 (Canonical Sources) applied to code values. Without shared constants, the same hex code or font name is hardcoded in every output file. Changing the brand color means a find-and-replace across every file ever built.

For non-code workspaces (content writing, course design), this pattern does not apply. Shared values live in reference docs instead.

---

## Quality Guardrails

- CONTEXT.md files: under 80 lines
- Reference files: under 200 lines (if longer, split into multiple files)
- Use plain English. Avoid jargon. If a term needs explaining, it is too specialized.
- No em dashes anywhere in the repo
- Every folder that should persist but starts empty gets a `.gitkeep` file
- Every markdown file should be readable by someone who understands markdown and git basics but does not have a deep engineering background



================================================
FILE: _core/placeholder-syntax.md
================================================
# Placeholder Syntax

How the onboarding system works. Workspaces ship with placeholder variables in their markdown files. The onboarding agent replaces these with real content when a user runs `setup`.

---

## Basic Syntax

Placeholders use double braces and SCREAMING_SNAKE_CASE:

```
{{BRAND_NAME}}
{{TARGET_AUDIENCE}}
{{PRIMARY_COLOR}}
```

These are literal strings in markdown files. They are not code variables. The onboarding agent finds them and replaces them with the user's answers through string substitution.

---

## Replacement Rules

1. The onboarding agent reads `setup/questionnaire.md` for the list of questions
2. Each question maps to one or more placeholders
3. Each question specifies which files contain its placeholder
4. The agent asks the questions conversationally, collecting answers
5. The agent replaces every instance of each placeholder with the corresponding answer
6. After all replacements, the agent scans the entire workspace for any remaining `{{` patterns
7. If any remain, the agent flags them and asks the user for the missing information
8. Onboarding is complete only when zero placeholders remain

---

## Where Placeholders Can Appear

Placeholders can appear in any markdown file within a workspace:
- Brand vault files (voice-rules.md, identity.md)
- Reference files (hook-system.md, design-system.md, etc.)
- Shared files (platform-specs.md)
- Stage CONTEXT.md files (only in Inputs table values, not in routing structure)

Placeholders should NOT appear in:
- CLAUDE.md files (these need to work before onboarding runs)
- Top-level CONTEXT.md routing tables (these need to work before onboarding runs)
- The questionnaire.md itself (the questions are the source, not the target)

---

## Conditional Sections

Conditional sections wrap content that gets removed if the user indicates it is not needed.

Syntax:

```markdown
{{?SECTION_NAME}}

## Section Heading

Content that may or may not be relevant...

{{/SECTION_NAME}}
```

**Rule: Conditional blocks can only wrap entire sections.** A section means a heading and all content below it, up to the next heading of the same or higher level.

Valid:

```markdown
{{?VIDEO_PRODUCTION}}

## Video Production Settings

Resolution, frame rate, and export format for your video pipeline.

- Resolution: 1920x1080
- Frame rate: 30fps
- Export format: MP4

{{/VIDEO_PRODUCTION}}
```

Invalid (do not do this):

```markdown
- Item one
{{?OPTIONAL_ITEM}}
- Item two (optional)
{{/OPTIONAL_ITEM}}
- Item three
```

Invalid (do not do this):

```markdown
The brand voice is {{?FORMAL}}formal and authoritative{{/FORMAL}}
{{?CASUAL}}casual and conversational{{/CASUAL}}.
```

Why this rule exists: removing inline content leaves orphaned list markers, broken sentences, or malformed markdown. Wrapping complete sections means removal always produces clean markdown.

---

## Naming Conventions

Use descriptive names: `{{BRAND_NAME}}` not `{{BN}}`.

Group related placeholders with common prefixes:
- `{{VOICE_DESCRIPTION}}`, `{{VOICE_ADJECTIVES}}`
- `{{PRIMARY_COLOR}}`, `{{SECONDARY_COLOR}}`, `{{ACCENT_COLOR}}`
- `{{CONTENT_PILLAR_1}}`, `{{CONTENT_PILLAR_2}}`

Conditional section names should describe what they wrap:
- `{{?BUILD_STAGE}}` for the build stage section
- `{{?PILLAR_4}}` for the fourth content pillar

---

## Questionnaire Mapping

The `setup/questionnaire.md` file is the bridge between questions and placeholders. Each question entry specifies:

- The question text (what the agent asks the user)
- The placeholder(s) it populates
- The file(s) where those placeholders appear
- The input type (free text, multiple choice, yes/no)
- Optional: follow-up questions for vague answers
- Optional: conditional logic (if answer is X, remove section Y)

See `_core/templates/questionnaire-template.md` for the format.



================================================
FILE: _core/templates/questionnaire-template.md
================================================
# Onboarding Questionnaire

<!-- Agent instructions: Read this file when the user types "setup". Ask ALL questions
     in a single conversational pass. The user should be able to answer everything in one
     message. Collect answers. Replace placeholders across the specified files. After all
     replacements, verify no {{PLACEHOLDER}} patterns remain in the workspace. -->

<!-- Questionnaire design rules:
     1. FLAT STRUCTURE: No category groupings. Just a numbered list of questions.
     2. ALL AT ONCE: Every question appears in one pass. The user answers in one message.
     3. SYSTEM-LEVEL ONLY: Questions configure the production system, not a specific run.
        Per-run details (project name, topic, audience) are collected conversationally
        at the start of each pipeline run by the entry stage.
     4. DERIVE, DON'T ASK: If a field can be derived from other answers, the agent fills
        it in without asking. List derived fields under the question they depend on.
     5. SENSIBLE DEFAULTS: Every question should have a default or example so the user
        can skip what they don't care about.
     6. ASK ONCE, NEVER AGAIN: After setup, the user should never be asked these questions
        again. The answers are baked into the workspace files permanently.
     7. EXAMPLES OVER DESCRIPTIONS: For voice/style questions, ask for concrete examples
        (sentences that sound right, sentences that sound wrong, specific error patterns)
        rather than abstract descriptions. Examples are pattern-matchable. Descriptions
        require interpretation and produce weaker constraints. -->

### Q1: [Question text]
- Placeholder: `{{PLACEHOLDER_NAME}}`
- Files: `path/to/file1.md`, `path/to/file2.md`
- Type: free text
- Default: [Default value if user wants to skip]

### Q2: [Question text]
- Placeholder: `{{PLACEHOLDER_NAME}}`
- Files: `path/to/file.md`
- Type: selection
- Options: Option A, Option B, Option C

### Q3: [Question about an optional feature -- yes/no]
- Type: yes/no
- If NO: Remove `stages/0N-name/` entirely
- If YES: Keep it

---

## After Onboarding

[Tell the user what was configured and where to start.]

After all replacements, scan the entire workspace for remaining `{{` patterns. If any remain, ask for the missing info.



================================================
FILE: _core/templates/stage-context-template.md
================================================
# [Stage Name]

[One sentence: what this stage does.]

## Inputs

<!-- List every file the agent needs. Be specific about which sections. -->

| Source | File/Location | Section/Scope | Why |
|--------|--------------|---------------|-----|
| Previous stage | `../0N-prev/output/artifact.md` | Full file | The artifact to work from |
| Reference | `references/example.md` | "Relevant Section" | What it provides |

## Process

<!-- Numbered steps. Each step is one concrete action. Be specific enough that
     two different agents following these steps would produce structurally similar
     outputs.

     Too vague: "Write the script"
     Good: "Write the full script in one pass, then audit against the voice
            hard constraints and value brief"

     Too vague: "Generate ideas"
     Good: "Propose 3-5 concept angles, each as a single sentence. Tag each
            with its value type and format." -->

1. Read the input artifact from the previous stage
2. [Step two]
3. [Step three]
4. Save to output/

## Checkpoints

<!-- Points where the agent pauses for human input before continuing.
     Not every stage needs checkpoints. Linear stages (extract, render, validate)
     often run straight through. Creative stages (writing, design, ideation)
     benefit from at least one.

     Format: after which process step, what the agent presents, what the human decides.
     Delete this section if the stage runs straight through. -->

| After Step | Agent Presents | Human Decides |
|------------|---------------|---------------|
| [step #] | [what options/output to show] | [what direction to choose] |

## Audit

<!-- Quality checks before the output is considered done. The agent runs these
     after completing the process steps. If any check fails, revise before saving.

     Not every stage needs an audit. Data extraction or file conversion stages
     may not benefit. Creative and build stages almost always do.
     Delete this section if no audit applies. -->

| Check | Pass Condition |
|-------|---------------|
| [Check name] | [What "passing" looks like] |

## Outputs

<!-- What this stage produces and where it goes. -->

| Artifact | Location | Format |
|----------|----------|--------|
| [Name] | `output/[slug]-[type].md` | [Description of the format] |

<!-- Target: keep this file under 80 lines. -->



================================================
FILE: _core/templates/workspace-claude-template.md
================================================
# [Workspace Name]

[One sentence: what this workspace does.]

## Folder Map

```
[workspace-name]/
├── CLAUDE.md          (you are here)
├── CONTEXT.md         (start here for task routing)
├── setup/             (onboarding questionnaire)
├── skills/            (bundled Claude skills for domain knowledge)
├── [context-folder]/  (shared context files)
├── stages/
│   ├── 01-[name]/     ([brief description])
│   ├── 02-[name]/     ([brief description])
│   └── 03-[name]/     ([brief description])
└── shared/            (cross-stage reference files)
```

## Triggers

| Keyword | Action |
|---------|--------|
| `setup` | Run onboarding questionnaire |
| `status` | Show pipeline completion for all stages |

## Routing

| Task | Go To |
|------|-------|
| [Task type 1] | `stages/01-[name]/CONTEXT.md` |
| [Task type 2] | `stages/02-[name]/CONTEXT.md` |
| [Task type 3] | `stages/03-[name]/CONTEXT.md` |

## What to Load

<!-- Map each task to its minimal file set. Loading more files dilutes quality.
     The context window is working memory, not storage. -->

| Task | Load These | Do NOT Load |
|------|-----------|-------------|
| [Task 1] | [minimal file list] | [what to skip and why] |
| [Task 2] | [minimal file list] | [what to skip and why] |

## Stage Handoffs

Each stage writes its output to its own `output/` folder. The next stage reads from there. If you edit an output file, the next stage picks up your edits.



================================================
FILE: _core/templates/workspace-context-template.md
================================================
# [Workspace Name]

[One sentence: what this workspace covers.]

## Task Routing

| Task Type | Go To | Description |
|-----------|-------|-------------|
| [Task 1] | `stages/01-[name]/CONTEXT.md` | [What this stage does] |
| [Task 2] | `stages/02-[name]/CONTEXT.md` | [What this stage does] |
| [Task 3] | `stages/03-[name]/CONTEXT.md` | [What this stage does] |

## Shared Resources

| Resource | Location | Contains |
|----------|----------|----------|
| [Context folder] | `[folder]/CONTEXT.md` | [What it routes to] |
| [Shared files] | `shared/` | [What cross-stage files live here] |
| [Skill name] | `skills/[name]/SKILL.md` | [What domain knowledge this skill provides] |



================================================
FILE: workspaces/course-deck-production/CLAUDE.md
================================================
# Course Deck Production

A production system for turning unstructured source material (PDFs, papers, notes, scripts) into polished PowerPoint slide decks. Run `setup` once to configure your identity and design. Then start a new course anytime -- just provide source material and a topic.

## Folder Map

```
course-deck-production/
├── CLAUDE.md              (you are here)
├── CONTEXT.md             (start here for task routing)
├── setup/
│   └── questionnaire.md   (one-time onboarding -- identity and design)
├── skills/                (bundled Claude skills for generation and design)
│   ├── pptx/              (full pptx skill: SKILL.md, html2pptx.md, ooxml.md, scripts/)
│   └── frontend-design/   (design philosophy and aesthetics: SKILL.md)
├── design-system/         (colors and typography for your brand)
│   ├── CONTEXT.md
│   ├── palette.md
│   └── typography.md
├── shared/
│   ├── producer-identity.md  (who you are -- set during setup)
│   └── course-meta.md        (template for per-course metadata)
├── stages/
│   ├── 01-extraction/     (source material to structured content)
│   ├── 02-curriculum/     (structured content to course plan)
│   ├── 03-outline/        (course plan to slide-by-slide outlines)
│   ├── 04-generation/     (outlines to .pptx via html2pptx)
│   └── 05-qa-delivery/    (visual QA, fix, deliver)
```

## Triggers

| Keyword | Action |
|---------|--------|
| `setup` | Run one-time onboarding -- configures identity, design system, default workflow |
| `status` | Show pipeline completion for all five stages |

### How `status` works

Scan `stages/*/output/` folders. For each stage, if the output folder contains files (other than .gitkeep), the stage is COMPLETE. Otherwise PENDING. Render:

```
Pipeline Status: course-deck-production

  [01-extraction]  -->  [02-curriculum]  -->  [03-outline]  -->  [04-generation]  -->  [05-qa-delivery]
      STATUS               STATUS               STATUS              STATUS               STATUS
```

## Starting a New Course

Each course is a new run through the pipeline. Provide source material and a topic. The agent collects course details (name, audience, session count) conversationally at the start of whichever stage you enter, and writes a `[course-slug]-meta.md` to that stage's output. This metadata travels forward through the pipeline.

Your default starting stage is set during setup (see `shared/producer-identity.md`). You can always override per course.

To start a fresh run, clear the output folders from the previous course.

## Routing

| Task | Go To |
|------|-------|
| Extract content from source material | `stages/01-extraction/CONTEXT.md` |
| Design the course curriculum | `stages/02-curriculum/CONTEXT.md` |
| Create slide outlines for sessions | `stages/03-outline/CONTEXT.md` |
| Generate PowerPoint decks | `stages/04-generation/CONTEXT.md` |
| Run visual QA and deliver | `stages/05-qa-delivery/CONTEXT.md` |

## What to Load

| Task | Load These | Do NOT Load |
|------|-----------|-------------|
| Extract content | `stages/01-extraction/references/*`, `shared/producer-identity.md`, `shared/course-meta.md` | `design-system/`, `skills/pptx/`, `stages/03-outline/` through `stages/05-qa-delivery/` |
| Design curriculum | `stages/01-extraction/output/`, `stages/02-curriculum/references/*` | `design-system/`, `skills/pptx/`, `stages/03-outline/` through `stages/05-qa-delivery/` |
| Create slide outlines | `stages/02-curriculum/output/`, `stages/03-outline/references/*`, `design-system/palette.md`, `design-system/typography.md` | `skills/pptx/`, `stages/01-extraction/`, `stages/04-generation/`, `stages/05-qa-delivery/` |
| Generate PowerPoint | `stages/03-outline/output/`, `stages/04-generation/references/*`, `design-system/*`, `skills/pptx/SKILL.md` | `stages/01-extraction/`, `stages/02-curriculum/` |
| Run QA and deliver | `stages/04-generation/output/`, `stages/05-qa-delivery/references/*`, `design-system/*` | `stages/01-extraction/`, `stages/02-curriculum/`, `stages/03-outline/` |

## Stage Handoffs

Each stage writes its output to its own `output/` folder. The next stage reads from there. If you edit an output file between stages, the next stage picks up your edits.

The typical flow is sequential (01 through 05). Users can enter at any stage depending on what they already have.



================================================
FILE: workspaces/course-deck-production/CONTEXT.md
================================================
# Course Deck Production

A production system for building polished PowerPoint decks from unstructured source material. Set up once, run per course.

## Task Routing

| Task Type | Go To | Description |
|-----------|-------|-------------|
| Extract source content | `stages/01-extraction/CONTEXT.md` | Pull structured teaching points from PDFs, papers, notes |
| Design curriculum | `stages/02-curriculum/CONTEXT.md` | Organize content into modules, sessions, and learning objectives |
| Outline slides | `stages/03-outline/CONTEXT.md` | Create slide-by-slide outlines for each session |
| Generate decks | `stages/04-generation/CONTEXT.md` | Build .pptx files from outlines via html2pptx |
| QA and deliver | `stages/05-qa-delivery/CONTEXT.md` | Visual QA, fix issues, deliver final decks |

## Shared Resources

| Resource | Location | Contains |
|----------|----------|----------|
| Producer identity | `shared/producer-identity.md` | Name/org, typical sources, default start stage |
| Course meta template | `shared/course-meta.md` | Template for per-course metadata (filled each run) |
| Design system | `design-system/CONTEXT.md` | Color palette and typography for all decks |
| Pptx skill | `skills/pptx/SKILL.md` | Full pptx creation workflow, html2pptx guide, scripts, OOXML editing |
| Frontend design skill | `skills/frontend-design/SKILL.md` | Design philosophy, aesthetics, avoiding generic AI look |



================================================
FILE: workspaces/course-deck-production/design-system/CONTEXT.md
================================================
# Design System

Visual identity for this course's slide decks. All sessions share these settings.

| File | Section to Load | Why |
|------|----------------|-----|
| `palette.md` | Full file | Color values for backgrounds, text, accents |
| `typography.md` | Full file | Font choices and size hierarchy |

For layout rules, slide constraints, and design philosophy, see the bundled skills:
- `../skills/pptx/SKILL.md` -- slide layout constraints, html2pptx rules
- `../skills/pptx/html2pptx.md` -- HTML syntax, supported elements, critical text rules
- `../skills/frontend-design/SKILL.md` -- design thinking process, aesthetics guidelines

These files are referenced by Stages 03 (outline), 04 (generation), and 05 (QA).



================================================
FILE: workspaces/course-deck-production/design-system/palette.md
================================================
# Color Palette

All colors as 6-digit hex. Use without `#` prefix in PptxGenJS code. Use with `#` prefix in HTML/CSS.

## Primary Colors

| Role | Hex | Usage |
|------|-----|-------|
| Primary | {{PRIMARY_COLOR}} | Headers, key shapes, accent bars |
| Secondary | {{SECONDARY_COLOR}} | Supporting shapes, secondary headers |
| Accent | {{ACCENT_COLOR}} | Call-outs, highlights, interactive elements |

## Neutrals

| Role | Hex | Usage |
|------|-----|-------|
| Background | {{BG_COLOR}} | Slide background |
| Text | {{TEXT_COLOR}} | Body text, default text color |

## Chart Colors

Use these in order for multi-series charts. Derived from primary palette:

```
chartColors: ["{{PRIMARY_COLOR}}", "{{SECONDARY_COLOR}}", "{{ACCENT_COLOR}}"]
```

## Contrast Rules

- Body text on background must have minimum 4.5:1 contrast ratio
- Heading text on colored shapes must have minimum 3:1 contrast ratio
- Never place light text on light backgrounds or dark text on dark backgrounds



================================================
FILE: workspaces/course-deck-production/design-system/typography.md
================================================
# Typography

All fonts must be web-safe. Non-web-safe fonts will not render correctly in PowerPoint.

## Font Choices

| Role | Font | Fallback |
|------|------|----------|
| Headings | {{HEADING_FONT}} | Arial |
| Body | {{BODY_FONT}} | Arial |
| Code/data | Courier New | -- |

## Allowed Web-Safe Fonts

Arial, Helvetica, Times New Roman, Georgia, Courier New, Verdana, Tahoma, Trebuchet MS, Impact.

Do not use any font not on this list.

## Size Hierarchy

| Element | Size | Weight |
|---------|------|--------|
| Slide title | 32-36pt | Bold |
| Section header | 24-28pt | Bold |
| Body text | 16-18pt | Regular |
| Bullet text | 14-16pt | Regular |
| Caption/footnote | 11-12pt | Regular |
| Oversized metric | 48-72pt | Bold |

## Spacing

- Line spacing: 1.2-1.5x for body text
- Space before headings: 10-14pt
- Space after headings: 6-8pt
- Minimum margin from slide edges: 20pt



================================================
FILE: workspaces/course-deck-production/setup/questionnaire.md
================================================
# Onboarding Questionnaire: Course Deck Production

Read this file when the user types "setup". Ask ALL questions below in a single conversational pass. These configure the production system -- not a specific course. Individual course details are collected at the start of each pipeline run.

---

Ask these questions all at once. The user should be able to answer everything in one message.

### Q1: What name or organization should appear on the decks?
- Placeholder: `{{PRODUCER_NAME}}`
- Files: `shared/producer-identity.md`
- Type: free text
- Example: "Dr. Smith", "Acme Training Co.", "Stanford CS Department"

### Q2: Pick a color palette (or bring your own).
- Placeholders: `{{PRIMARY_COLOR}}`, `{{SECONDARY_COLOR}}`, `{{ACCENT_COLOR}}`, `{{BG_COLOR}}`, `{{TEXT_COLOR}}`
- Files: `design-system/palette.md`
- Type: selection or free text
- Options:
  - **Classic Blue:** Primary 1C2833, Secondary 2E4053, Accent 3498DB, BG F4F6F6, Text 2C3E50
  - **Teal & Coral:** Primary 277884, Secondary 5EA8A7, Accent FE4447, BG FFFFFF, Text 2C3E50
  - **Burgundy Luxury:** Primary 5D1D2E, Secondary 951233, Accent 997929, BG FAF7F2, Text 2C3E50
  - **Forest Green:** Primary 1E5128, Secondary 4E9F3D, Accent D4A843, BG FFFFFF, Text 191A19
  - **Custom:** provide five hex values (primary, secondary, accent, background, text)

### Q3: Heading font and body font?
- Placeholders: `{{HEADING_FONT}}`, `{{BODY_FONT}}`
- Files: `design-system/typography.md`
- Type: multiple choice
- Web-safe only: Arial, Georgia, Verdana, Trebuchet MS, Impact, Tahoma, Times New Roman
- Default: Arial / Arial

### Q4: What kind of source material do you typically work from?
- Placeholder: `{{TYPICAL_SOURCES}}`
- Files: `shared/producer-identity.md`
- Type: free text
- Examples: "academic papers and PDFs", "my own lecture notes", "textbook chapters", "mixed -- varies per course"
- Purpose: Helps the agent know which extraction strategies to default to. Does NOT lock you in -- you can provide anything per course.

### Q5: Where do you usually start in the pipeline?
- Placeholder: `{{DEFAULT_START_STAGE}}`
- Files: `shared/producer-identity.md`
- Type: selection
- Options:
  - **From scratch** -- I bring raw material and need everything (Stage 01)
  - **From notes** -- I have organized content, skip extraction (Stage 02)
  - **From a syllabus** -- I already have a curriculum, just need decks (Stage 03)
- Purpose: Sets the default starting point. You can always override per course.

---

## After Onboarding

Tell the user:

"You are set up. Here is your production config:

- **Identity:** [producer name]
- **Design:** [palette name], [heading font] / [body font]
- **Default start:** Stage [01|02|03]

To start a new course, just say what you want to build and provide your source material. I will collect the course details (name, audience, session count) at the start of each run."

Then scan the workspace for any remaining `{{` patterns. If any remain, ask for the missing info.



================================================
FILE: workspaces/course-deck-production/shared/course-meta.md
================================================
# Course Metadata

This file is a template. The agent fills it in at the start of each new course run by asking the user conversationally. Do NOT put placeholders here -- this is per-course data, not system config.

## How to Use

At the start of Stage 01 (or whichever stage the user enters), collect this info from the user and write a copy to the current stage's output folder as `[course-slug]-meta.md`:

```markdown
# [Course Name]

- **Description:** [one sentence]
- **Target audience:** [who and what level]
- **Session count:** [number, default 8]
- **Session duration:** [minutes, default 45]
- **Slides per session:** 15-25
- **Aspect ratio:** 16:9 (720pt x 405pt)
- **Source material:** [what the user provided for this run]
```

This metadata file travels forward through the pipeline. Each stage reads it from the previous stage's output.



================================================
FILE: workspaces/course-deck-production/shared/producer-identity.md
================================================
# Producer Identity

Permanent config for whoever is producing these decks. Set during onboarding, reused across all courses.

- **Name/organization:** {{PRODUCER_NAME}}
- **Typical source material:** {{TYPICAL_SOURCES}}
- **Default starting stage:** {{DEFAULT_START_STAGE}}



================================================
FILE: workspaces/course-deck-production/skills/frontend-design/LICENSE.txt
================================================

                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS



================================================
FILE: workspaces/course-deck-production/skills/frontend-design/SKILL.md
================================================
---
name: frontend-design
description: Create distinctive, production-grade frontend interfaces with high design quality. Use this skill when the user asks to build web components, pages, artifacts, posters, or applications (examples include websites, landing pages, dashboards, React components, HTML/CSS layouts, or when styling/beautifying any web UI). Generates creative, polished code and UI design that avoids generic AI aesthetics.
license: Complete terms in LICENSE.txt
---

This skill guides creation of distinctive, production-grade frontend interfaces that avoid generic "AI slop" aesthetics. Implement real working code with exceptional attention to aesthetic details and creative choices.

The user provides frontend requirements: a component, page, application, or interface to build. They may include context about the purpose, audience, or technical constraints.

## Design Thinking

Before coding, understand the context and commit to a BOLD aesthetic direction:
- **Purpose**: What problem does this interface solve? Who uses it?
- **Tone**: Pick an extreme: brutally minimal, maximalist chaos, retro-futuristic, organic/natural, luxury/refined, playful/toy-like, editorial/magazine, brutalist/raw, art deco/geometric, soft/pastel, industrial/utilitarian, etc. There are so many flavors to choose from. Use these for inspiration but design one that is true to the aesthetic direction.
- **Constraints**: Technical requirements (framework, performance, accessibility).
- **Differentiation**: What makes this UNFORGETTABLE? What's the one thing someone will remember?

**CRITICAL**: Choose a clear conceptual direction and execute it with precision. Bold maximalism and refined minimalism both work - the key is intentionality, not intensity.

Then implement working code (HTML/CSS/JS, React, Vue, etc.) that is:
- Production-grade and functional
- Visually striking and memorable
- Cohesive with a clear aesthetic point-of-view
- Meticulously refined in every detail

## Frontend Aesthetics Guidelines

Focus on:
- **Typography**: Choose fonts that are beautiful, unique, and interesting. Avoid generic fonts like Arial and Inter; opt instead for distinctive choices that elevate the frontend's aesthetics; unexpected, characterful font choices. Pair a distinctive display font with a refined body font.
- **Color & Theme**: Commit to a cohesive aesthetic. Use CSS variables for consistency. Dominant colors with sharp accents outperform timid, evenly-distributed palettes.
- **Motion**: Use animations for effects and micro-interactions. Prioritize CSS-only solutions for HTML. Use Motion library for React when available. Focus on high-impact moments: one well-orchestrated page load with staggered reveals (animation-delay) creates more delight than scattered micro-interactions. Use scroll-triggering and hover states that surprise.
- **Spatial Composition**: Unexpected layouts. Asymmetry. Overlap. Diagonal flow. Grid-breaking elements. Generous negative space OR controlled density.
- **Backgrounds & Visual Details**: Create atmosphere and depth rather than defaulting to solid colors. Add contextual effects and textures that match the overall aesthetic. Apply creative forms like gradient meshes, noise textures, geometric patterns, layered transparencies, dramatic shadows, decorative borders, custom cursors, and grain overlays.

NEVER use generic AI-generated aesthetics like overused font families (Inter, Roboto, Arial, system fonts), cliched color schemes (particularly purple gradients on white backgrounds), predictable layouts and component patterns, and cookie-cutter design that lacks context-specific character.

Interpret creatively and make unexpected choices that feel genuinely designed for the context. No design should be the same. Vary between light and dark themes, different fonts, different aesthetics. NEVER converge on common choices (Space Grotesk, for example) across generations.

**IMPORTANT**: Match implementation complexity to the aesthetic vision. Maximalist designs need elaborate code with extensive animations and effects. Minimalist or refined designs need restraint, precision, and careful attention to spacing, typography, and subtle details. Elegance comes from executing the vision well.

Remember: Claude is capable of extraordinary creative work. Don't hold back, show what can truly be created when thinking outside the box and committing fully to a distinctive vision.



================================================
FILE: workspaces/course-deck-production/skills/pptx/html2pptx.md
================================================
# HTML to PowerPoint Guide

Convert HTML slides to PowerPoint presentations with accurate positioning using the `html2pptx.js` library.

## Table of Contents

1. [Creating HTML Slides](#creating-html-slides)
2. [Using the html2pptx Library](#using-the-html2pptx-library)
3. [Using PptxGenJS](#using-pptxgenjs)

---

## Creating HTML Slides

Every HTML slide must include proper body dimensions:

### Layout Dimensions

- **16:9** (default): `width: 720pt; height: 405pt`
- **4:3**: `width: 720pt; height: 540pt`
- **16:10**: `width: 720pt; height: 450pt`

### Supported Elements

- `<p>`, `<h1>`-`<h6>` - Text with styling
- `<ul>`, `<ol>` - Lists (never use manual bullets •, -, *)
- `<b>`, `<strong>` - Bold text (inline formatting)
- `<i>`, `<em>` - Italic text (inline formatting)
- `<u>` - Underlined text (inline formatting)
- `<span>` - Inline formatting with CSS styles (bold, italic, underline, color)
- `<br>` - Line breaks
- `<div>` with bg/border - Becomes shape
- `<img>` - Images
- `class="placeholder"` - Reserved space for charts (returns `{ id, x, y, w, h }`)

### Critical Text Rules

**ALL text MUST be inside `<p>`, `<h1>`-`<h6>`, `<ul>`, or `<ol>` tags:**
- ✅ Correct: `<div><p>Text here</p></div>`
- ❌ Wrong: `<div>Text here</div>` - **Text will NOT appear in PowerPoint**
- ❌ Wrong: `<span>Text</span>` - **Text will NOT appear in PowerPoint**
- Text in `<div>` or `<span>` without a text tag will be silently ignored

**NEVER use manual bullet symbols (•, -, *, etc.)** - Use `<ul>` or `<ol>` lists instead

**ONLY use web-safe fonts that are universally available:**
- ✅ Web-safe fonts: `Arial`, `Helvetica`, `Times New Roman`, `Georgia`, `Courier New`, `Verdana`, `Tahoma`, `Trebuchet MS`, `Impact`, `Comic Sans MS`
- ❌ Wrong: `'Segoe UI'`, `'SF Pro'`, `'Roboto'`, custom fonts - **Might cause rendering issues**

### Styling

- Use `display: flex` on body to prevent margin collapse from breaking overflow validation
- Use `margin` for spacing (padding included in size)
- Inline formatting: Use `<b>`, `<i>`, `<u>` tags OR `<span>` with CSS styles
  - `<span>` supports: `font-weight: bold`, `font-style: italic`, `text-decoration: underline`, `color: #rrggbb`
  - `<span>` does NOT support: `margin`, `padding` (not supported in PowerPoint text runs)
  - Example: `<span style="font-weight: bold; color: #667eea;">Bold blue text</span>`
- Flexbox works - positions calculated from rendered layout
- Use hex colors with `#` prefix in CSS
- **Text alignment**: Use CSS `text-align` (`center`, `right`, etc.) when needed as a hint to PptxGenJS for text formatting if text lengths are slightly off

### Shape Styling (DIV elements only)

**IMPORTANT: Backgrounds, borders, and shadows only work on `<div>` elements, NOT on text elements (`<p>`, `<h1>`-`<h6>`, `<ul>`, `<ol>`)**

- **Backgrounds**: CSS `background` or `background-color` on `<div>` elements only
  - Example: `<div style="background: #f0f0f0;">` - Creates a shape with background
- **Borders**: CSS `border` on `<div>` elements converts to PowerPoint shape borders
  - Supports uniform borders: `border: 2px solid #333333`
  - Supports partial borders: `border-left`, `border-right`, `border-top`, `border-bottom` (rendered as line shapes)
  - Example: `<div style="border-left: 8pt solid #E76F51;">`
- **Border radius**: CSS `border-radius` on `<div>` elements for rounded corners
  - `border-radius: 50%` or higher creates circular shape
  - Percentages <50% calculated relative to shape's smaller dimension
  - Supports px and pt units (e.g., `border-radius: 8pt;`, `border-radius: 12px;`)
  - Example: `<div style="border-radius: 25%;">` on 100x200px box = 25% of 100px = 25px radius
- **Box shadows**: CSS `box-shadow` on `<div>` elements converts to PowerPoint shadows
  - Supports outer shadows only (inset shadows are ignored to prevent corruption)
  - Example: `<div style="box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.3);">`
  - Note: Inset/inner shadows are not supported by PowerPoint and will be skipped

### Icons & Gradients

- **CRITICAL: Never use CSS gradients (`linear-gradient`, `radial-gradient`)** - They don't convert to PowerPoint
- **ALWAYS create gradient/icon PNGs FIRST using Sharp, then reference in HTML**
- For gradients: Rasterize SVG to PNG background images
- For icons: Rasterize react-icons SVG to PNG images
- All visual effects must be pre-rendered as raster images before HTML rendering

**Rasterizing Icons with Sharp:**

```javascript
const React = require('react');
const ReactDOMServer = require('react-dom/server');
const sharp = require('sharp');
const { FaHome } = require('react-icons/fa');

async function rasterizeIconPng(IconComponent, color, size = "256", filename) {
  const svgString = ReactDOMServer.renderToStaticMarkup(
    React.createElement(IconComponent, { color: `#${color}`, size: size })
  );

  // Convert SVG to PNG using Sharp
  await sharp(Buffer.from(svgString))
    .png()
    .toFile(filename);

  return filename;
}

// Usage: Rasterize icon before using in HTML
const iconPath = await rasterizeIconPng(FaHome, "4472c4", "256", "home-icon.png");
// Then reference in HTML: <img src="home-icon.png" style="width: 40pt; height: 40pt;">
```

**Rasterizing Gradients with Sharp:**

```javascript
const sharp = require('sharp');

async function createGradientBackground(filename) {
  const svg = `<svg xmlns="http://www.w3.org/2000/svg" width="1000" height="562.5">
    <defs>
      <linearGradient id="g" x1="0%" y1="0%" x2="100%" y2="100%">
        <stop offset="0%" style="stop-color:#COLOR1"/>
        <stop offset="100%" style="stop-color:#COLOR2"/>
      </linearGradient>
    </defs>
    <rect width="100%" height="100%" fill="url(#g)"/>
  </svg>`;

  await sharp(Buffer.from(svg))
    .png()
    .toFile(filename);

  return filename;
}

// Usage: Create gradient background before HTML
const bgPath = await createGradientBackground("gradient-bg.png");
// Then in HTML: <body style="background-image: url('gradient-bg.png');">
```

### Example

```html
<!DOCTYPE html>
<html>
<head>
<style>
html { background: #ffffff; }
body {
  width: 720pt; height: 405pt; margin: 0; padding: 0;
  background: #f5f5f5; font-family: Arial, sans-serif;
  display: flex;
}
.content { margin: 30pt; padding: 40pt; background: #ffffff; border-radius: 8pt; }
h1 { color: #2d3748; font-size: 32pt; }
.box {
  background: #70ad47; padding: 20pt; border: 3px solid #5a8f37;
  border-radius: 12pt; box-shadow: 3px 3px 10px rgba(0, 0, 0, 0.25);
}
</style>
</head>
<body>
<div class="content">
  <h1>Recipe Title</h1>
  <ul>
    <li><b>Item:</b> Description</li>
  </ul>
  <p>Text with <b>bold</b>, <i>italic</i>, <u>underline</u>.</p>
  <div id="chart" class="placeholder" style="width: 350pt; height: 200pt;"></div>

  <!-- Text MUST be in <p> tags -->
  <div class="box">
    <p>5</p>
  </div>
</div>
</body>
</html>
```

## Using the html2pptx Library

### Dependencies

These libraries have been globally installed and are available to use:
- `pptxgenjs`
- `playwright`
- `sharp`

### Basic Usage

```javascript
const pptxgen = require('pptxgenjs');
const html2pptx = require('./html2pptx');

const pptx = new pptxgen();
pptx.layout = 'LAYOUT_16x9';  // Must match HTML body dimensions

const { slide, placeholders } = await html2pptx('slide1.html', pptx);

// Add chart to placeholder area
if (placeholders.length > 0) {
    slide.addChart(pptx.charts.LINE, chartData, placeholders[0]);
}

await pptx.writeFile('output.pptx');
```

### API Reference

#### Function Signature
```javascript
await html2pptx(htmlFile, pres, options)
```

#### Parameters
- `htmlFile` (string): Path to HTML file (absolute or relative)
- `pres` (pptxgen): PptxGenJS presentation instance with layout already set
- `options` (object, optional):
  - `tmpDir` (string): Temporary directory for generated files (default: `process.env.TMPDIR || '/tmp'`)
  - `slide` (object): Existing slide to reuse (default: creates new slide)

#### Returns
```javascript
{
    slide: pptxgenSlide,           // The created/updated slide
    placeholders: [                 // Array of placeholder positions
        { id: string, x: number, y: number, w: number, h: number },
        ...
    ]
}
```

### Validation

The library automatically validates and collects all errors before throwing:

1. **HTML dimensions must match presentation layout** - Reports dimension mismatches
2. **Content must not overflow body** - Reports overflow with exact measurements
3. **CSS gradients** - Reports unsupported gradient usage
4. **Text element styling** - Reports backgrounds/borders/shadows on text elements (only allowed on divs)

**All validation errors are collected and reported together** in a single error message, allowing you to fix all issues at once instead of one at a time.

### Working with Placeholders

```javascript
const { slide, placeholders } = await html2pptx('slide.html', pptx);

// Use first placeholder
slide.addChart(pptx.charts.BAR, data, placeholders[0]);

// Find by ID
const chartArea = placeholders.find(p => p.id === 'chart-area');
slide.addChart(pptx.charts.LINE, data, chartArea);
```

### Complete Example

```javascript
const pptxgen = require('pptxgenjs');
const html2pptx = require('./html2pptx');

async function createPresentation() {
    const pptx = new pptxgen();
    pptx.layout = 'LAYOUT_16x9';
    pptx.author = 'Your Name';
    pptx.title = 'My Presentation';

    // Slide 1: Title
    const { slide: slide1 } = await html2pptx('slides/title.html', pptx);

    // Slide 2: Content with chart
    const { slide: slide2, placeholders } = await html2pptx('slides/data.html', pptx);

    const chartData = [{
        name: 'Sales',
        labels: ['Q1', 'Q2', 'Q3', 'Q4'],
        values: [4500, 5500, 6200, 7100]
    }];

    slide2.addChart(pptx.charts.BAR, chartData, {
        ...placeholders[0],
        showTitle: true,
        title: 'Quarterly Sales',
        showCatAxisTitle: true,
        catAxisTitle: 'Quarter',
        showValAxisTitle: true,
        valAxisTitle: 'Sales ($000s)'
    });

    // Save
    await pptx.writeFile({ fileName: 'presentation.pptx' });
    console.log('Presentation created successfully!');
}

createPresentation().catch(console.error);
```

## Using PptxGenJS

After converting HTML to slides with `html2pptx`, you'll use PptxGenJS to add dynamic content like charts, images, and additional elements.

### ⚠️ Critical Rules

#### Colors
- **NEVER use `#` prefix** with hex colors in PptxGenJS - causes file corruption
- ✅ Correct: `color: "FF0000"`, `fill: { color: "0066CC" }`
- ❌ Wrong: `color: "#FF0000"` (breaks document)

### Adding Images

Always calculate aspect ratios from actual image dimensions:

```javascript
// Get image dimensions: identify image.png | grep -o '[0-9]* x [0-9]*'
const imgWidth = 1860, imgHeight = 1519;  // From actual file
const aspectRatio = imgWidth / imgHeight;

const h = 3;  // Max height
const w = h * aspectRatio;
const x = (10 - w) / 2;  // Center on 16:9 slide

slide.addImage({ path: "chart.png", x, y: 1.5, w, h });
```

### Adding Text

```javascript
// Rich text with formatting
slide.addText([
    { text: "Bold ", options: { bold: true } },
    { text: "Italic ", options: { italic: true } },
    { text: "Normal" }
], {
    x: 1, y: 2, w: 8, h: 1
});
```

### Adding Shapes

```javascript
// Rectangle
slide.addShape(pptx.shapes.RECTANGLE, {
    x: 1, y: 1, w: 3, h: 2,
    fill: { color: "4472C4" },
    line: { color: "000000", width: 2 }
});

// Circle
slide.addShape(pptx.shapes.OVAL, {
    x: 5, y: 1, w: 2, h: 2,
    fill: { color: "ED7D31" }
});

// Rounded rectangle
slide.addShape(pptx.shapes.ROUNDED_RECTANGLE, {
    x: 1, y: 4, w: 3, h: 1.5,
    fill: { color: "70AD47" },
    rectRadius: 0.2
});
```

### Adding Charts

**Required for most charts:** Axis labels using `catAxisTitle` (category) and `valAxisTitle` (value).

**Chart Data Format:**
- Use **single series with all labels** for simple bar/line charts
- Each series creates a separate legend entry
- Labels array defines X-axis values

**Time Series Data - Choose Correct Granularity:**
- **< 30 days**: Use daily grouping (e.g., "10-01", "10-02") - avoid monthly aggregation that creates single-point charts
- **30-365 days**: Use monthly grouping (e.g., "2024-01", "2024-02")
- **> 365 days**: Use yearly grouping (e.g., "2023", "2024")
- **Validate**: Charts with only 1 data point likely indicate incorrect aggregation for the time period

```javascript
const { slide, placeholders } = await html2pptx('slide.html', pptx);

// CORRECT: Single series with all labels
slide.addChart(pptx.charts.BAR, [{
    name: "Sales 2024",
    labels: ["Q1", "Q2", "Q3", "Q4"],
    values: [4500, 5500, 6200, 7100]
}], {
    ...placeholders[0],  // Use placeholder position
    barDir: 'col',       // 'col' = vertical bars, 'bar' = horizontal
    showTitle: true,
    title: 'Quarterly Sales',
    showLegend: false,   // No legend needed for single series
    // Required axis labels
    showCatAxisTitle: true,
    catAxisTitle: 'Quarter',
    showValAxisTitle: true,
    valAxisTitle: 'Sales ($000s)',
    // Optional: Control scaling (adjust min based on data range for better visualization)
    valAxisMaxVal: 8000,
    valAxisMinVal: 0,  // Use 0 for counts/amounts; for clustered data (e.g., 4500-7100), consider starting closer to min value
    valAxisMajorUnit: 2000,  // Control y-axis label spacing to prevent crowding
    catAxisLabelRotate: 45,  // Rotate labels if crowded
    dataLabelPosition: 'outEnd',
    dataLabelColor: '000000',
    // Use single color for single-series charts
    chartColors: ["4472C4"]  // All bars same color
});
```

#### Scatter Chart

**IMPORTANT**: Scatter chart data format is unusual - first series contains X-axis values, subsequent series contain Y-values:

```javascript
// Prepare data
const data1 = [{ x: 10, y: 20 }, { x: 15, y: 25 }, { x: 20, y: 30 }];
const data2 = [{ x: 12, y: 18 }, { x: 18, y: 22 }];

const allXValues = [...data1.map(d => d.x), ...data2.map(d => d.x)];

slide.addChart(pptx.charts.SCATTER, [
    { name: 'X-Axis', values: allXValues },  // First series = X values
    { name: 'Series 1', values: data1.map(d => d.y) },  // Y values only
    { name: 'Series 2', values: data2.map(d => d.y) }   // Y values only
], {
    x: 1, y: 1, w: 8, h: 4,
    lineSize: 0,  // 0 = no connecting lines
    lineDataSymbol: 'circle',
    lineDataSymbolSize: 6,
    showCatAxisTitle: true,
    catAxisTitle: 'X Axis',
    showValAxisTitle: true,
    valAxisTitle: 'Y Axis',
    chartColors: ["4472C4", "ED7D31"]
});
```

#### Line Chart

```javascript
slide.addChart(pptx.charts.LINE, [{
    name: "Temperature",
    labels: ["Jan", "Feb", "Mar", "Apr"],
    values: [32, 35, 42, 55]
}], {
    x: 1, y: 1, w: 8, h: 4,
    lineSize: 4,
    lineSmooth: true,
    // Required axis labels
    showCatAxisTitle: true,
    catAxisTitle: 'Month',
    showValAxisTitle: true,
    valAxisTitle: 'Temperature (°F)',
    // Optional: Y-axis range (set min based on data range for better visualization)
    valAxisMinVal: 0,     // For ranges starting at 0 (counts, percentages, etc.)
    valAxisMaxVal: 60,
    valAxisMajorUnit: 20,  // Control y-axis label spacing to prevent crowding (e.g., 10, 20, 25)
    // valAxisMinVal: 30,  // PREFERRED: For data clustered in a range (e.g., 32-55 or ratings 3-5), start axis closer to min value to show variation
    // Optional: Chart colors
    chartColors: ["4472C4", "ED7D31", "A5A5A5"]
});
```

#### Pie Chart (No Axis Labels Required)

**CRITICAL**: Pie charts require a **single data series** with all categories in the `labels` array and corresponding values in the `values` array.

```javascript
slide.addChart(pptx.charts.PIE, [{
    name: "Market Share",
    labels: ["Product A", "Product B", "Other"],  // All categories in one array
    values: [35, 45, 20]  // All values in one array
}], {
    x: 2, y: 1, w: 6, h: 4,
    showPercent: true,
    showLegend: true,
    legendPos: 'r',  // right
    chartColors: ["4472C4", "ED7D31", "A5A5A5"]
});
```

#### Multiple Data Series

```javascript
slide.addChart(pptx.charts.LINE, [
    {
        name: "Product A",
        labels: ["Q1", "Q2", "Q3", "Q4"],
        values: [10, 20, 30, 40]
    },
    {
        name: "Product B",
        labels: ["Q1", "Q2", "Q3", "Q4"],
        values: [15, 25, 20, 35]
    }
], {
    x: 1, y: 1, w: 8, h: 4,
    showCatAxisTitle: true,
    catAxisTitle: 'Quarter',
    showValAxisTitle: true,
    valAxisTitle: 'Revenue ($M)'
});
```

### Chart Colors

**CRITICAL**: Use hex colors **without** the `#` prefix - including `#` causes file corruption.

**Align chart colors with your chosen design palette**, ensuring sufficient contrast and distinctiveness for data visualization. Adjust colors for:
- Strong contrast between adjacent series
- Readability against slide backgrounds
- Accessibility (avoid red-green only combinations)

```javascript
// Example: Ocean palette-inspired chart colors (adjusted for contrast)
const chartColors = ["16A085", "FF6B9D", "2C3E50", "F39C12", "9B59B6"];

// Single-series chart: Use one color for all bars/points
slide.addChart(pptx.charts.BAR, [{
    name: "Sales",
    labels: ["Q1", "Q2", "Q3", "Q4"],
    values: [4500, 5500, 6200, 7100]
}], {
    ...placeholders[0],
    chartColors: ["16A085"],  // All bars same color
    showLegend: false
});

// Multi-series chart: Each series gets a different color
slide.addChart(pptx.charts.LINE, [
    { name: "Product A", labels: ["Q1", "Q2", "Q3"], values: [10, 20, 30] },
    { name: "Product B", labels: ["Q1", "Q2", "Q3"], values: [15, 25, 20] }
], {
    ...placeholders[0],
    chartColors: ["16A085", "FF6B9D"]  // One color per series
});
```

### Adding Tables

Tables can be added with basic or advanced formatting:

#### Basic Table

```javascript
slide.addTable([
    ["Header 1", "Header 2", "Header 3"],
    ["Row 1, Col 1", "Row 1, Col 2", "Row 1, Col 3"],
    ["Row 2, Col 1", "Row 2, Col 2", "Row 2, Col 3"]
], {
    x: 0.5,
    y: 1,
    w: 9,
    h: 3,
    border: { pt: 1, color: "999999" },
    fill: { color: "F1F1F1" }
});
```

#### Table with Custom Formatting

```javascript
const tableData = [
    // Header row with custom styling
    [
        { text: "Product", options: { fill: { color: "4472C4" }, color: "FFFFFF", bold: true } },
        { text: "Revenue", options: { fill: { color: "4472C4" }, color: "FFFFFF", bold: true } },
        { text: "Growth", options: { fill: { color: "4472C4" }, color: "FFFFFF", bold: true } }
    ],
    // Data rows
    ["Product A", "$50M", "+15%"],
    ["Product B", "$35M", "+22%"],
    ["Product C", "$28M", "+8%"]
];

slide.addTable(tableData, {
    x: 1,
    y: 1.5,
    w: 8,
    h: 3,
    colW: [3, 2.5, 2.5],  // Column widths
    rowH: [0.5, 0.6, 0.6, 0.6],  // Row heights
    border: { pt: 1, color: "CCCCCC" },
    align: "center",
    valign: "middle",
    fontSize: 14
});
```

#### Table with Merged Cells

```javascript
const mergedTableData = [
    [
        { text: "Q1 Results", options: { colspan: 3, fill: { color: "4472C4" }, color: "FFFFFF", bold: true } }
    ],
    ["Product", "Sales", "Market Share"],
    ["Product A", "$25M", "35%"],
    ["Product B", "$18M", "25%"]
];

slide.addTable(mergedTableData, {
    x: 1,
    y: 1,
    w: 8,
    h: 2.5,
    colW: [3, 2.5, 2.5],
    border: { pt: 1, color: "DDDDDD" }
});
```

### Table Options

Common table options:
- `x, y, w, h` - Position and size
- `colW` - Array of column widths (in inches)
- `rowH` - Array of row heights (in inches)
- `border` - Border style: `{ pt: 1, color: "999999" }`
- `fill` - Background color (no # prefix)
- `align` - Text alignment: "left", "center", "right"
- `valign` - Vertical alignment: "top", "middle", "bottom"
- `fontSize` - Text size
- `autoPage` - Auto-create new slides if content overflows


================================================
FILE: workspaces/course-deck-production/skills/pptx/LICENSE.txt
================================================
© 2025 Anthropic, PBC. All rights reserved.

LICENSE: Use of these materials (including all code, prompts, assets, files,
and other components of this Skill) is governed by your agreement with
Anthropic regarding use of Anthropic's services. If no separate agreement
exists, use is governed by Anthropic's Consumer Terms of Service or
Commercial Terms of Service, as applicable:
https://www.anthropic.com/legal/consumer-terms
https://www.anthropic.com/legal/commercial-terms
Your applicable agreement is referred to as the "Agreement." "Services" are
as defined in the Agreement.

ADDITIONAL RESTRICTIONS: Notwithstanding anything in the Agreement to the
contrary, users may not:

- Extract these materials from the Services or retain copies of these
  materials outside the Services
- Reproduce or copy these materials, except for temporary copies created
  automatically during authorized use of the Services
- Create derivative works based on these materials
- Distribute, sublicense, or transfer these materials to any third party
- Make, offer to sell, sell, or import any inventions embodied in these
  materials
- Reverse engineer, decompile, or disassemble these materials

The receipt, viewing, or possession of these materials does not convey or
imply any license or right beyond those expressly granted above.

Anthropic retains all right, title, and interest in these materials,
including all copyrights, patents, and other intellectual property rights.



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml.md
================================================
# Office Open XML Technical Reference for PowerPoint

**Important: Read this entire document before starting.** Critical XML schema rules and formatting requirements are covered throughout. Incorrect implementation can create invalid PPTX files that PowerPoint cannot open.

## Technical Guidelines

### Schema Compliance
- **Element ordering in `<p:txBody>`**: `<a:bodyPr>`, `<a:lstStyle>`, `<a:p>`
- **Whitespace**: Add `xml:space='preserve'` to `<a:t>` elements with leading/trailing spaces
- **Unicode**: Escape characters in ASCII content: `"` becomes `&#8220;`
- **Images**: Add to `ppt/media/`, reference in slide XML, set dimensions to fit slide bounds
- **Relationships**: Update `ppt/slides/_rels/slideN.xml.rels` for each slide's resources
- **Dirty attribute**: Add `dirty="0"` to `<a:rPr>` and `<a:endParaRPr>` elements to indicate clean state

## Presentation Structure

### Basic Slide Structure
```xml
<!-- ppt/slides/slide1.xml -->
<p:sld>
  <p:cSld>
    <p:spTree>
      <p:nvGrpSpPr>...</p:nvGrpSpPr>
      <p:grpSpPr>...</p:grpSpPr>
      <!-- Shapes go here -->
    </p:spTree>
  </p:cSld>
</p:sld>
```

### Text Box / Shape with Text
```xml
<p:sp>
  <p:nvSpPr>
    <p:cNvPr id="2" name="Title"/>
    <p:cNvSpPr>
      <a:spLocks noGrp="1"/>
    </p:cNvSpPr>
    <p:nvPr>
      <p:ph type="ctrTitle"/>
    </p:nvPr>
  </p:nvSpPr>
  <p:spPr>
    <a:xfrm>
      <a:off x="838200" y="365125"/>
      <a:ext cx="7772400" cy="1470025"/>
    </a:xfrm>
  </p:spPr>
  <p:txBody>
    <a:bodyPr/>
    <a:lstStyle/>
    <a:p>
      <a:r>
        <a:t>Slide Title</a:t>
      </a:r>
    </a:p>
  </p:txBody>
</p:sp>
```

### Text Formatting
```xml
<!-- Bold -->
<a:r>
  <a:rPr b="1"/>
  <a:t>Bold Text</a:t>
</a:r>

<!-- Italic -->
<a:r>
  <a:rPr i="1"/>
  <a:t>Italic Text</a:t>
</a:r>

<!-- Underline -->
<a:r>
  <a:rPr u="sng"/>
  <a:t>Underlined</a:t>
</a:r>

<!-- Highlight -->
<a:r>
  <a:rPr>
    <a:highlight>
      <a:srgbClr val="FFFF00"/>
    </a:highlight>
  </a:rPr>
  <a:t>Highlighted Text</a:t>
</a:r>

<!-- Font and Size -->
<a:r>
  <a:rPr sz="2400" typeface="Arial">
    <a:solidFill>
      <a:srgbClr val="FF0000"/>
    </a:solidFill>
  </a:rPr>
  <a:t>Colored Arial 24pt</a:t>
</a:r>

<!-- Complete formatting example -->
<a:r>
  <a:rPr lang="en-US" sz="1400" b="1" dirty="0">
    <a:solidFill>
      <a:srgbClr val="FAFAFA"/>
    </a:solidFill>
  </a:rPr>
  <a:t>Formatted text</a:t>
</a:r>
```

### Lists
```xml
<!-- Bullet list -->
<a:p>
  <a:pPr lvl="0">
    <a:buChar char="•"/>
  </a:pPr>
  <a:r>
    <a:t>First bullet point</a:t>
  </a:r>
</a:p>

<!-- Numbered list -->
<a:p>
  <a:pPr lvl="0">
    <a:buAutoNum type="arabicPeriod"/>
  </a:pPr>
  <a:r>
    <a:t>First numbered item</a:t>
  </a:r>
</a:p>

<!-- Second level indent -->
<a:p>
  <a:pPr lvl="1">
    <a:buChar char="•"/>
  </a:pPr>
  <a:r>
    <a:t>Indented bullet</a:t>
  </a:r>
</a:p>
```

### Shapes
```xml
<!-- Rectangle -->
<p:sp>
  <p:nvSpPr>
    <p:cNvPr id="3" name="Rectangle"/>
    <p:cNvSpPr/>
    <p:nvPr/>
  </p:nvSpPr>
  <p:spPr>
    <a:xfrm>
      <a:off x="1000000" y="1000000"/>
      <a:ext cx="3000000" cy="2000000"/>
    </a:xfrm>
    <a:prstGeom prst="rect">
      <a:avLst/>
    </a:prstGeom>
    <a:solidFill>
      <a:srgbClr val="FF0000"/>
    </a:solidFill>
    <a:ln w="25400">
      <a:solidFill>
        <a:srgbClr val="000000"/>
      </a:solidFill>
    </a:ln>
  </p:spPr>
</p:sp>

<!-- Rounded Rectangle -->
<p:sp>
  <p:spPr>
    <a:prstGeom prst="roundRect">
      <a:avLst/>
    </a:prstGeom>
  </p:spPr>
</p:sp>

<!-- Circle/Ellipse -->
<p:sp>
  <p:spPr>
    <a:prstGeom prst="ellipse">
      <a:avLst/>
    </a:prstGeom>
  </p:spPr>
</p:sp>
```

### Images
```xml
<p:pic>
  <p:nvPicPr>
    <p:cNvPr id="4" name="Picture">
      <a:hlinkClick r:id="" action="ppaction://media"/>
    </p:cNvPr>
    <p:cNvPicPr>
      <a:picLocks noChangeAspect="1"/>
    </p:cNvPicPr>
    <p:nvPr/>
  </p:nvPicPr>
  <p:blipFill>
    <a:blip r:embed="rId2"/>
    <a:stretch>
      <a:fillRect/>
    </a:stretch>
  </p:blipFill>
  <p:spPr>
    <a:xfrm>
      <a:off x="1000000" y="1000000"/>
      <a:ext cx="3000000" cy="2000000"/>
    </a:xfrm>
    <a:prstGeom prst="rect">
      <a:avLst/>
    </a:prstGeom>
  </p:spPr>
</p:pic>
```

### Tables
```xml
<p:graphicFrame>
  <p:nvGraphicFramePr>
    <p:cNvPr id="5" name="Table"/>
    <p:cNvGraphicFramePr>
      <a:graphicFrameLocks noGrp="1"/>
    </p:cNvGraphicFramePr>
    <p:nvPr/>
  </p:nvGraphicFramePr>
  <p:xfrm>
    <a:off x="1000000" y="1000000"/>
    <a:ext cx="6000000" cy="2000000"/>
  </p:xfrm>
  <a:graphic>
    <a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/table">
      <a:tbl>
        <a:tblGrid>
          <a:gridCol w="3000000"/>
          <a:gridCol w="3000000"/>
        </a:tblGrid>
        <a:tr h="500000">
          <a:tc>
            <a:txBody>
              <a:bodyPr/>
              <a:lstStyle/>
              <a:p>
                <a:r>
                  <a:t>Cell 1</a:t>
                </a:r>
              </a:p>
            </a:txBody>
          </a:tc>
          <a:tc>
            <a:txBody>
              <a:bodyPr/>
              <a:lstStyle/>
              <a:p>
                <a:r>
                  <a:t>Cell 2</a:t>
                </a:r>
              </a:p>
            </a:txBody>
          </a:tc>
        </a:tr>
      </a:tbl>
    </a:graphicData>
  </a:graphic>
</p:graphicFrame>
```

### Slide Layouts

```xml
<!-- Title Slide Layout -->
<p:sp>
  <p:nvSpPr>
    <p:nvPr>
      <p:ph type="ctrTitle"/>
    </p:nvPr>
  </p:nvSpPr>
  <!-- Title content -->
</p:sp>

<p:sp>
  <p:nvSpPr>
    <p:nvPr>
      <p:ph type="subTitle" idx="1"/>
    </p:nvPr>
  </p:nvSpPr>
  <!-- Subtitle content -->
</p:sp>

<!-- Content Slide Layout -->
<p:sp>
  <p:nvSpPr>
    <p:nvPr>
      <p:ph type="title"/>
    </p:nvPr>
  </p:nvSpPr>
  <!-- Slide title -->
</p:sp>

<p:sp>
  <p:nvSpPr>
    <p:nvPr>
      <p:ph type="body" idx="1"/>
    </p:nvPr>
  </p:nvSpPr>
  <!-- Content body -->
</p:sp>
```

## File Updates

When adding content, update these files:

**`ppt/_rels/presentation.xml.rels`:**
```xml
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slide" Target="slides/slide1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideMaster" Target="slideMasters/slideMaster1.xml"/>
```

**`ppt/slides/_rels/slide1.xml.rels`:**
```xml
<Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/slideLayout" Target="../slideLayouts/slideLayout1.xml"/>
<Relationship Id="rId2" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="../media/image1.png"/>
```

**`[Content_Types].xml`:**
```xml
<Default Extension="png" ContentType="image/png"/>
<Default Extension="jpg" ContentType="image/jpeg"/>
<Override PartName="/ppt/slides/slide1.xml" ContentType="application/vnd.openxmlformats-officedocument.presentationml.slide+xml"/>
```

**`ppt/presentation.xml`:**
```xml
<p:sldIdLst>
  <p:sldId id="256" r:id="rId1"/>
  <p:sldId id="257" r:id="rId2"/>
</p:sldIdLst>
```

**`docProps/app.xml`:** Update slide count and statistics
```xml
<Slides>2</Slides>
<Paragraphs>10</Paragraphs>
<Words>50</Words>
```

## Slide Operations

### Adding a New Slide
When adding a slide to the end of the presentation:

1. **Create the slide file** (`ppt/slides/slideN.xml`)
2. **Update `[Content_Types].xml`**: Add Override for the new slide
3. **Update `ppt/_rels/presentation.xml.rels`**: Add relationship for the new slide
4. **Update `ppt/presentation.xml`**: Add slide ID to `<p:sldIdLst>`
5. **Create slide relationships** (`ppt/slides/_rels/slideN.xml.rels`) if needed
6. **Update `docProps/app.xml`**: Increment slide count and update statistics (if present)

### Duplicating a Slide
1. Copy the source slide XML file with a new name
2. Update all IDs in the new slide to be unique
3. Follow the "Adding a New Slide" steps above
4. **CRITICAL**: Remove or update any notes slide references in `_rels` files
5. Remove references to unused media files

### Reordering Slides
1. **Update `ppt/presentation.xml`**: Reorder `<p:sldId>` elements in `<p:sldIdLst>`
2. The order of `<p:sldId>` elements determines slide order
3. Keep slide IDs and relationship IDs unchanged

Example:
```xml
<!-- Original order -->
<p:sldIdLst>
  <p:sldId id="256" r:id="rId2"/>
  <p:sldId id="257" r:id="rId3"/>
  <p:sldId id="258" r:id="rId4"/>
</p:sldIdLst>

<!-- After moving slide 3 to position 2 -->
<p:sldIdLst>
  <p:sldId id="256" r:id="rId2"/>
  <p:sldId id="258" r:id="rId4"/>
  <p:sldId id="257" r:id="rId3"/>
</p:sldIdLst>
```

### Deleting a Slide
1. **Remove from `ppt/presentation.xml`**: Delete the `<p:sldId>` entry
2. **Remove from `ppt/_rels/presentation.xml.rels`**: Delete the relationship
3. **Remove from `[Content_Types].xml`**: Delete the Override entry
4. **Delete files**: Remove `ppt/slides/slideN.xml` and `ppt/slides/_rels/slideN.xml.rels`
5. **Update `docProps/app.xml`**: Decrement slide count and update statistics
6. **Clean up unused media**: Remove orphaned images from `ppt/media/`

Note: Don't renumber remaining slides - keep their original IDs and filenames.


## Common Errors to Avoid

- **Encodings**: Escape unicode characters in ASCII content: `"` becomes `&#8220;`
- **Images**: Add to `ppt/media/` and update relationship files
- **Lists**: Omit bullets from list headers
- **IDs**: Use valid hexadecimal values for UUIDs
- **Themes**: Check all themes in `theme` directory for colors

## Validation Checklist for Template-Based Presentations

### Before Packing, Always:
- **Clean unused resources**: Remove unreferenced media, fonts, and notes directories
- **Fix Content_Types.xml**: Declare ALL slides, layouts, and themes present in the package
- **Fix relationship IDs**: 
   - Remove font embed references if not using embedded fonts
- **Remove broken references**: Check all `_rels` files for references to deleted resources

### Common Template Duplication Pitfalls:
- Multiple slides referencing the same notes slide after duplication
- Image/media references from template slides that no longer exist
- Font embedding references when fonts aren't included
- Missing slideLayout declarations for layouts 12-25
- docProps directory may not unpack - this is optional


================================================
FILE: workspaces/course-deck-production/skills/pptx/SKILL.md
================================================
---
name: pptx
description: "Presentation creation, editing, and analysis. When Claude needs to work with presentations (.pptx files) for: (1) Creating new presentations, (2) Modifying or editing content, (3) Working with layouts, (4) Adding comments or speaker notes, or any other presentation tasks"
license: Proprietary. LICENSE.txt has complete terms
---

# PPTX creation, editing, and analysis

## Overview

A user may ask you to create, edit, or analyze the contents of a .pptx file. A .pptx file is essentially a ZIP archive containing XML files and other resources that you can read or edit. You have different tools and workflows available for different tasks.

## Reading and analyzing content

### Text extraction
If you just need to read the text contents of a presentation, you should convert the document to markdown:

```bash
# Convert document to markdown
python -m markitdown path-to-file.pptx
```

### Raw XML access
You need raw XML access for: comments, speaker notes, slide layouts, animations, design elements, and complex formatting. For any of these features, you'll need to unpack a presentation and read its raw XML contents.

#### Unpacking a file
`python ooxml/scripts/unpack.py <office_file> <output_dir>`

**Note**: The unpack.py script is located at `skills/pptx/ooxml/scripts/unpack.py` relative to the project root. If the script doesn't exist at this path, use `find . -name "unpack.py"` to locate it.

#### Key file structures
* `ppt/presentation.xml` - Main presentation metadata and slide references
* `ppt/slides/slide{N}.xml` - Individual slide contents (slide1.xml, slide2.xml, etc.)
* `ppt/notesSlides/notesSlide{N}.xml` - Speaker notes for each slide
* `ppt/comments/modernComment_*.xml` - Comments for specific slides
* `ppt/slideLayouts/` - Layout templates for slides
* `ppt/slideMasters/` - Master slide templates
* `ppt/theme/` - Theme and styling information
* `ppt/media/` - Images and other media files

#### Typography and color extraction
**When given an example design to emulate**: Always analyze the presentation's typography and colors first using the methods below:
1. **Read theme file**: Check `ppt/theme/theme1.xml` for colors (`<a:clrScheme>`) and fonts (`<a:fontScheme>`)
2. **Sample slide content**: Examine `ppt/slides/slide1.xml` for actual font usage (`<a:rPr>`) and colors
3. **Search for patterns**: Use grep to find color (`<a:solidFill>`, `<a:srgbClr>`) and font references across all XML files

## Creating a new PowerPoint presentation **without a template**

When creating a new PowerPoint presentation from scratch, use the **html2pptx** workflow to convert HTML slides to PowerPoint with accurate positioning.

### Design Principles

**CRITICAL**: Before creating any presentation, analyze the content and choose appropriate design elements:
1. **Consider the subject matter**: What is this presentation about? What tone, industry, or mood does it suggest?
2. **Check for branding**: If the user mentions a company/organization, consider their brand colors and identity
3. **Match palette to content**: Select colors that reflect the subject
4. **State your approach**: Explain your design choices before writing code

**Requirements**:
- ✅ State your content-informed design approach BEFORE writing code
- ✅ Use web-safe fonts only: Arial, Helvetica, Times New Roman, Georgia, Courier New, Verdana, Tahoma, Trebuchet MS, Impact
- ✅ Create clear visual hierarchy through size, weight, and color
- ✅ Ensure readability: strong contrast, appropriately sized text, clean alignment
- ✅ Be consistent: repeat patterns, spacing, and visual language across slides

#### Color Palette Selection

**Choosing colors creatively**:
- **Think beyond defaults**: What colors genuinely match this specific topic? Avoid autopilot choices.
- **Consider multiple angles**: Topic, industry, mood, energy level, target audience, brand identity (if mentioned)
- **Be adventurous**: Try unexpected combinations - a healthcare presentation doesn't have to be green, finance doesn't have to be navy
- **Build your palette**: Pick 3-5 colors that work together (dominant colors + supporting tones + accent)
- **Ensure contrast**: Text must be clearly readable on backgrounds

**Example color palettes** (use these to spark creativity - choose one, adapt it, or create your own):

1. **Classic Blue**: Deep navy (#1C2833), slate gray (#2E4053), silver (#AAB7B8), off-white (#F4F6F6)
2. **Teal & Coral**: Teal (#5EA8A7), deep teal (#277884), coral (#FE4447), white (#FFFFFF)
3. **Bold Red**: Red (#C0392B), bright red (#E74C3C), orange (#F39C12), yellow (#F1C40F), green (#2ECC71)
4. **Warm Blush**: Mauve (#A49393), blush (#EED6D3), rose (#E8B4B8), cream (#FAF7F2)
5. **Burgundy Luxury**: Burgundy (#5D1D2E), crimson (#951233), rust (#C15937), gold (#997929)
6. **Deep Purple & Emerald**: Purple (#B165FB), dark blue (#181B24), emerald (#40695B), white (#FFFFFF)
7. **Cream & Forest Green**: Cream (#FFE1C7), forest green (#40695B), white (#FCFCFC)
8. **Pink & Purple**: Pink (#F8275B), coral (#FF574A), rose (#FF737D), purple (#3D2F68)
9. **Lime & Plum**: Lime (#C5DE82), plum (#7C3A5F), coral (#FD8C6E), blue-gray (#98ACB5)
10. **Black & Gold**: Gold (#BF9A4A), black (#000000), cream (#F4F6F6)
11. **Sage & Terracotta**: Sage (#87A96B), terracotta (#E07A5F), cream (#F4F1DE), charcoal (#2C2C2C)
12. **Charcoal & Red**: Charcoal (#292929), red (#E33737), light gray (#CCCBCB)
13. **Vibrant Orange**: Orange (#F96D00), light gray (#F2F2F2), charcoal (#222831)
14. **Forest Green**: Black (#191A19), green (#4E9F3D), dark green (#1E5128), white (#FFFFFF)
15. **Retro Rainbow**: Purple (#722880), pink (#D72D51), orange (#EB5C18), amber (#F08800), gold (#DEB600)
16. **Vintage Earthy**: Mustard (#E3B448), sage (#CBD18F), forest green (#3A6B35), cream (#F4F1DE)
17. **Coastal Rose**: Old rose (#AD7670), beaver (#B49886), eggshell (#F3ECDC), ash gray (#BFD5BE)
18. **Orange & Turquoise**: Light orange (#FC993E), grayish turquoise (#667C6F), white (#FCFCFC)

#### Visual Details Options

**Geometric Patterns**:
- Diagonal section dividers instead of horizontal
- Asymmetric column widths (30/70, 40/60, 25/75)
- Rotated text headers at 90° or 270°
- Circular/hexagonal frames for images
- Triangular accent shapes in corners
- Overlapping shapes for depth

**Border & Frame Treatments**:
- Thick single-color borders (10-20pt) on one side only
- Double-line borders with contrasting colors
- Corner brackets instead of full frames
- L-shaped borders (top+left or bottom+right)
- Underline accents beneath headers (3-5pt thick)

**Typography Treatments**:
- Extreme size contrast (72pt headlines vs 11pt body)
- All-caps headers with wide letter spacing
- Numbered sections in oversized display type
- Monospace (Courier New) for data/stats/technical content
- Condensed fonts (Arial Narrow) for dense information
- Outlined text for emphasis

**Chart & Data Styling**:
- Monochrome charts with single accent color for key data
- Horizontal bar charts instead of vertical
- Dot plots instead of bar charts
- Minimal gridlines or none at all
- Data labels directly on elements (no legends)
- Oversized numbers for key metrics

**Layout Innovations**:
- Full-bleed images with text overlays
- Sidebar column (20-30% width) for navigation/context
- Modular grid systems (3×3, 4×4 blocks)
- Z-pattern or F-pattern content flow
- Floating text boxes over colored shapes
- Magazine-style multi-column layouts

**Background Treatments**:
- Solid color blocks occupying 40-60% of slide
- Gradient fills (vertical or diagonal only)
- Split backgrounds (two colors, diagonal or vertical)
- Edge-to-edge color bands
- Negative space as a design element

### Layout Tips
**When creating slides with charts or tables:**
- **Two-column layout (PREFERRED)**: Use a header spanning the full width, then two columns below - text/bullets in one column and the featured content in the other. This provides better balance and makes charts/tables more readable. Use flexbox with unequal column widths (e.g., 40%/60% split) to optimize space for each content type.
- **Full-slide layout**: Let the featured content (chart/table) take up the entire slide for maximum impact and readability
- **NEVER vertically stack**: Do not place charts/tables below text in a single column - this causes poor readability and layout issues

### Workflow
1. **MANDATORY - READ ENTIRE FILE**: Read [`html2pptx.md`](html2pptx.md) completely from start to finish. **NEVER set any range limits when reading this file.** Read the full file content for detailed syntax, critical formatting rules, and best practices before proceeding with presentation creation.
2. Create an HTML file for each slide with proper dimensions (e.g., 720pt × 405pt for 16:9)
   - Use `<p>`, `<h1>`-`<h6>`, `<ul>`, `<ol>` for all text content
   - Use `class="placeholder"` for areas where charts/tables will be added (render with gray background for visibility)
   - **CRITICAL**: Rasterize gradients and icons as PNG images FIRST using Sharp, then reference in HTML
   - **LAYOUT**: For slides with charts/tables/images, use either full-slide layout or two-column layout for better readability
3. Create and run a JavaScript file using the [`html2pptx.js`](scripts/html2pptx.js) library to convert HTML slides to PowerPoint and save the presentation
   - Use the `html2pptx()` function to process each HTML file
   - Add charts and tables to placeholder areas using PptxGenJS API
   - Save the presentation using `pptx.writeFile()`
4. **Visual validation**: Generate thumbnails and inspect for layout issues
   - Create thumbnail grid: `python scripts/thumbnail.py output.pptx workspace/thumbnails --cols 4`
   - Read and carefully examine the thumbnail image for:
     - **Text cutoff**: Text being cut off by header bars, shapes, or slide edges
     - **Text overlap**: Text overlapping with other text or shapes
     - **Positioning issues**: Content too close to slide boundaries or other elements
     - **Contrast issues**: Insufficient contrast between text and backgrounds
   - If issues found, adjust HTML margins/spacing/colors and regenerate the presentation
   - Repeat until all slides are visually correct

## Editing an existing PowerPoint presentation

When edit slides in an existing PowerPoint presentation, you need to work with the raw Office Open XML (OOXML) format. This involves unpacking the .pptx file, editing the XML content, and repacking it.

### Workflow
1. **MANDATORY - READ ENTIRE FILE**: Read [`ooxml.md`](ooxml.md) (~500 lines) completely from start to finish.  **NEVER set any range limits when reading this file.**  Read the full file content for detailed guidance on OOXML structure and editing workflows before any presentation editing.
2. Unpack the presentation: `python ooxml/scripts/unpack.py <office_file> <output_dir>`
3. Edit the XML files (primarily `ppt/slides/slide{N}.xml` and related files)
4. **CRITICAL**: Validate immediately after each edit and fix any validation errors before proceeding: `python ooxml/scripts/validate.py <dir> --original <file>`
5. Pack the final presentation: `python ooxml/scripts/pack.py <input_directory> <office_file>`

## Creating a new PowerPoint presentation **using a template**

When you need to create a presentation that follows an existing template's design, you'll need to duplicate and re-arrange template slides before then replacing placeholder context.

### Workflow
1. **Extract template text AND create visual thumbnail grid**:
   * Extract text: `python -m markitdown template.pptx > template-content.md`
   * Read `template-content.md`: Read the entire file to understand the contents of the template presentation. **NEVER set any range limits when reading this file.**
   * Create thumbnail grids: `python scripts/thumbnail.py template.pptx`
   * See [Creating Thumbnail Grids](#creating-thumbnail-grids) section for more details

2. **Analyze template and save inventory to a file**:
   * **Visual Analysis**: Review thumbnail grid(s) to understand slide layouts, design patterns, and visual structure
   * Create and save a template inventory file at `template-inventory.md` containing:
     ```markdown
     # Template Inventory Analysis
     **Total Slides: [count]**
     **IMPORTANT: Slides are 0-indexed (first slide = 0, last slide = count-1)**

     ## [Category Name]
     - Slide 0: [Layout code if available] - Description/purpose
     - Slide 1: [Layout code] - Description/purpose
     - Slide 2: [Layout code] - Description/purpose
     [... EVERY slide must be listed individually with its index ...]
     ```
   * **Using the thumbnail grid**: Reference the visual thumbnails to identify:
     - Layout patterns (title slides, content layouts, section dividers)
     - Image placeholder locations and counts
     - Design consistency across slide groups
     - Visual hierarchy and structure
   * This inventory file is REQUIRED for selecting appropriate templates in the next step

3. **Create presentation outline based on template inventory**:
   * Review available templates from step 2.
   * Choose an intro or title template for the first slide. This should be one of the first templates.
   * Choose safe, text-based layouts for the other slides.
   * **CRITICAL: Match layout structure to actual content**:
     - Single-column layouts: Use for unified narrative or single topic
     - Two-column layouts: Use ONLY when you have exactly 2 distinct items/concepts
     - Three-column layouts: Use ONLY when you have exactly 3 distinct items/concepts
     - Image + text layouts: Use ONLY when you have actual images to insert
     - Quote layouts: Use ONLY for actual quotes from people (with attribution), never for emphasis
     - Never use layouts with more placeholders than you have content
     - If you have 2 items, don't force them into a 3-column layout
     - If you have 4+ items, consider breaking into multiple slides or using a list format
   * Count your actual content pieces BEFORE selecting the layout
   * Verify each placeholder in the chosen layout will be filled with meaningful content
   * Select one option representing the **best** layout for each content section.
   * Save `outline.md` with content AND template mapping that leverages available designs
   * Example template mapping:
      ```
      # Template slides to use (0-based indexing)
      # WARNING: Verify indices are within range! Template with 73 slides has indices 0-72
      # Mapping: slide numbers from outline -> template slide indices
      template_mapping = [
          0,   # Use slide 0 (Title/Cover)
          34,  # Use slide 34 (B1: Title and body)
          34,  # Use slide 34 again (duplicate for second B1)
          50,  # Use slide 50 (E1: Quote)
          54,  # Use slide 54 (F2: Closing + Text)
      ]
      ```

4. **Duplicate, reorder, and delete slides using `rearrange.py`**:
   * Use the `scripts/rearrange.py` script to create a new presentation with slides in the desired order:
     ```bash
     python scripts/rearrange.py template.pptx working.pptx 0,34,34,50,52
     ```
   * The script handles duplicating repeated slides, deleting unused slides, and reordering automatically
   * Slide indices are 0-based (first slide is 0, second is 1, etc.)
   * The same slide index can appear multiple times to duplicate that slide

5. **Extract ALL text using the `inventory.py` script**:
   * **Run inventory extraction**:
     ```bash
     python scripts/inventory.py working.pptx text-inventory.json
     ```
   * **Read text-inventory.json**: Read the entire text-inventory.json file to understand all shapes and their properties. **NEVER set any range limits when reading this file.**

   * The inventory JSON structure:
      ```json
        {
          "slide-0": {
            "shape-0": {
              "placeholder_type": "TITLE",  // or null for non-placeholders
              "left": 1.5,                  // position in inches
              "top": 2.0,
              "width": 7.5,
              "height": 1.2,
              "paragraphs": [
                {
                  "text": "Paragraph text",
                  // Optional properties (only included when non-default):
                  "bullet": true,           // explicit bullet detected
                  "level": 0,               // only included when bullet is true
                  "alignment": "CENTER",    // CENTER, RIGHT (not LEFT)
                  "space_before": 10.0,     // space before paragraph in points
                  "space_after": 6.0,       // space after paragraph in points
                  "line_spacing": 22.4,     // line spacing in points
                  "font_name": "Arial",     // from first run
                  "font_size": 14.0,        // in points
                  "bold": true,
                  "italic": false,
                  "underline": false,
                  "color": "FF0000"         // RGB color
                }
              ]
            }
          }
        }
      ```

   * Key features:
     - **Slides**: Named as "slide-0", "slide-1", etc.
     - **Shapes**: Ordered by visual position (top-to-bottom, left-to-right) as "shape-0", "shape-1", etc.
     - **Placeholder types**: TITLE, CENTER_TITLE, SUBTITLE, BODY, OBJECT, or null
     - **Default font size**: `default_font_size` in points extracted from layout placeholders (when available)
     - **Slide numbers are filtered**: Shapes with SLIDE_NUMBER placeholder type are automatically excluded from inventory
     - **Bullets**: When `bullet: true`, `level` is always included (even if 0)
     - **Spacing**: `space_before`, `space_after`, and `line_spacing` in points (only included when set)
     - **Colors**: `color` for RGB (e.g., "FF0000"), `theme_color` for theme colors (e.g., "DARK_1")
     - **Properties**: Only non-default values are included in the output

6. **Generate replacement text and save the data to a JSON file**
   Based on the text inventory from the previous step:
   - **CRITICAL**: First verify which shapes exist in the inventory - only reference shapes that are actually present
   - **VALIDATION**: The replace.py script will validate that all shapes in your replacement JSON exist in the inventory
     - If you reference a non-existent shape, you'll get an error showing available shapes
     - If you reference a non-existent slide, you'll get an error indicating the slide doesn't exist
     - All validation errors are shown at once before the script exits
   - **IMPORTANT**: The replace.py script uses inventory.py internally to identify ALL text shapes
   - **AUTOMATIC CLEARING**: ALL text shapes from the inventory will be cleared unless you provide "paragraphs" for them
   - Add a "paragraphs" field to shapes that need content (not "replacement_paragraphs")
   - Shapes without "paragraphs" in the replacement JSON will have their text cleared automatically
   - Paragraphs with bullets will be automatically left aligned. Don't set the `alignment` property on when `"bullet": true`
   - Generate appropriate replacement content for placeholder text
   - Use shape size to determine appropriate content length
   - **CRITICAL**: Include paragraph properties from the original inventory - don't just provide text
   - **IMPORTANT**: When bullet: true, do NOT include bullet symbols (•, -, *) in text - they're added automatically
   - **ESSENTIAL FORMATTING RULES**:
     - Headers/titles should typically have `"bold": true`
     - List items should have `"bullet": true, "level": 0` (level is required when bullet is true)
     - Preserve any alignment properties (e.g., `"alignment": "CENTER"` for centered text)
     - Include font properties when different from default (e.g., `"font_size": 14.0`, `"font_name": "Lora"`)
     - Colors: Use `"color": "FF0000"` for RGB or `"theme_color": "DARK_1"` for theme colors
     - The replacement script expects **properly formatted paragraphs**, not just text strings
     - **Overlapping shapes**: Prefer shapes with larger default_font_size or more appropriate placeholder_type
   - Save the updated inventory with replacements to `replacement-text.json`
   - **WARNING**: Different template layouts have different shape counts - always check the actual inventory before creating replacements

   Example paragraphs field showing proper formatting:
   ```json
   "paragraphs": [
     {
       "text": "New presentation title text",
       "alignment": "CENTER",
       "bold": true
     },
     {
       "text": "Section Header",
       "bold": true
     },
     {
       "text": "First bullet point without bullet symbol",
       "bullet": true,
       "level": 0
     },
     {
       "text": "Red colored text",
       "color": "FF0000"
     },
     {
       "text": "Theme colored text",
       "theme_color": "DARK_1"
     },
     {
       "text": "Regular paragraph text without special formatting"
     }
   ]
   ```

   **Shapes not listed in the replacement JSON are automatically cleared**:
   ```json
   {
     "slide-0": {
       "shape-0": {
         "paragraphs": [...] // This shape gets new text
       }
       // shape-1 and shape-2 from inventory will be cleared automatically
     }
   }
   ```

   **Common formatting patterns for presentations**:
   - Title slides: Bold text, sometimes centered
   - Section headers within slides: Bold text
   - Bullet lists: Each item needs `"bullet": true, "level": 0`
   - Body text: Usually no special properties needed
   - Quotes: May have special alignment or font properties

7. **Apply replacements using the `replace.py` script**
   ```bash
   python scripts/replace.py working.pptx replacement-text.json output.pptx
   ```

   The script will:
   - First extract the inventory of ALL text shapes using functions from inventory.py
   - Validate that all shapes in the replacement JSON exist in the inventory
   - Clear text from ALL shapes identified in the inventory
   - Apply new text only to shapes with "paragraphs" defined in the replacement JSON
   - Preserve formatting by applying paragraph properties from the JSON
   - Handle bullets, alignment, font properties, and colors automatically
   - Save the updated presentation

   Example validation errors:
   ```
   ERROR: Invalid shapes in replacement JSON:
     - Shape 'shape-99' not found on 'slide-0'. Available shapes: shape-0, shape-1, shape-4
     - Slide 'slide-999' not found in inventory
   ```

   ```
   ERROR: Replacement text made overflow worse in these shapes:
     - slide-0/shape-2: overflow worsened by 1.25" (was 0.00", now 1.25")
   ```

## Creating Thumbnail Grids

To create visual thumbnail grids of PowerPoint slides for quick analysis and reference:

```bash
python scripts/thumbnail.py template.pptx [output_prefix]
```

**Features**:
- Creates: `thumbnails.jpg` (or `thumbnails-1.jpg`, `thumbnails-2.jpg`, etc. for large decks)
- Default: 5 columns, max 30 slides per grid (5×6)
- Custom prefix: `python scripts/thumbnail.py template.pptx my-grid`
  - Note: The output prefix should include the path if you want output in a specific directory (e.g., `workspace/my-grid`)
- Adjust columns: `--cols 4` (range: 3-6, affects slides per grid)
- Grid limits: 3 cols = 12 slides/grid, 4 cols = 20, 5 cols = 30, 6 cols = 42
- Slides are zero-indexed (Slide 0, Slide 1, etc.)

**Use cases**:
- Template analysis: Quickly understand slide layouts and design patterns
- Content review: Visual overview of entire presentation
- Navigation reference: Find specific slides by their visual appearance
- Quality check: Verify all slides are properly formatted

**Examples**:
```bash
# Basic usage
python scripts/thumbnail.py presentation.pptx

# Combine options: custom name, columns
python scripts/thumbnail.py template.pptx analysis --cols 4
```

## Converting Slides to Images

To visually analyze PowerPoint slides, convert them to images using a two-step process:

1. **Convert PPTX to PDF**:
   ```bash
   soffice --headless --convert-to pdf template.pptx
   ```

2. **Convert PDF pages to JPEG images**:
   ```bash
   pdftoppm -jpeg -r 150 template.pdf slide
   ```
   This creates files like `slide-1.jpg`, `slide-2.jpg`, etc.

Options:
- `-r 150`: Sets resolution to 150 DPI (adjust for quality/size balance)
- `-jpeg`: Output JPEG format (use `-png` for PNG if preferred)
- `-f N`: First page to convert (e.g., `-f 2` starts from page 2)
- `-l N`: Last page to convert (e.g., `-l 5` stops at page 5)
- `slide`: Prefix for output files

Example for specific range:
```bash
pdftoppm -jpeg -r 150 -f 2 -l 5 template.pdf slide  # Converts only pages 2-5
```

## Code Style Guidelines
**IMPORTANT**: When generating code for PPTX operations:
- Write concise code
- Avoid verbose variable names and redundant operations
- Avoid unnecessary print statements

## Dependencies

Required dependencies (should already be installed):

- **markitdown**: `pip install "markitdown[pptx]"` (for text extraction from presentations)
- **pptxgenjs**: `npm install -g pptxgenjs` (for creating presentations via html2pptx)
- **playwright**: `npm install -g playwright` (for HTML rendering in html2pptx)
- **react-icons**: `npm install -g react-icons react react-dom` (for icons)
- **sharp**: `npm install -g sharp` (for SVG rasterization and image processing)
- **LibreOffice**: `sudo apt-get install libreoffice` (for PDF conversion)
- **Poppler**: `sudo apt-get install poppler-utils` (for pdftoppm to convert PDF to images)
- **defusedxml**: `pip install defusedxml` (for secure XML parsing)


================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ecma/fouth-edition/opc-contentTypes.xsd
================================================
﻿<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<xs:schema xmlns="http://schemas.openxmlformats.org/package/2006/content-types"
  xmlns:xs="http://www.w3.org/2001/XMLSchema"
  targetNamespace="http://schemas.openxmlformats.org/package/2006/content-types"
  elementFormDefault="qualified" attributeFormDefault="unqualified" blockDefault="#all">

  <xs:element name="Types" type="CT_Types"/>
  <xs:element name="Default" type="CT_Default"/>
  <xs:element name="Override" type="CT_Override"/>

  <xs:complexType name="CT_Types">
    <xs:choice minOccurs="0" maxOccurs="unbounded">
      <xs:element ref="Default"/>
      <xs:element ref="Override"/>
    </xs:choice>
  </xs:complexType>

  <xs:complexType name="CT_Default">
    <xs:attribute name="Extension" type="ST_Extension" use="required"/>
    <xs:attribute name="ContentType" type="ST_ContentType" use="required"/>
  </xs:complexType>

  <xs:complexType name="CT_Override">
    <xs:attribute name="ContentType" type="ST_ContentType" use="required"/>
    <xs:attribute name="PartName" type="xs:anyURI" use="required"/>
  </xs:complexType>

  <xs:simpleType name="ST_ContentType">
    <xs:restriction base="xs:string">
      <xs:pattern
        value="(((([\p{IsBasicLatin}-[\p{Cc}&#127;\(\)&lt;&gt;@,;:\\&quot;/\[\]\?=\{\}\s\t]])+))/((([\p{IsBasicLatin}-[\p{Cc}&#127;\(\)&lt;&gt;@,;:\\&quot;/\[\]\?=\{\}\s\t]])+))((\s+)*;(\s+)*(((([\p{IsBasicLatin}-[\p{Cc}&#127;\(\)&lt;&gt;@,;:\\&quot;/\[\]\?=\{\}\s\t]])+))=((([\p{IsBasicLatin}-[\p{Cc}&#127;\(\)&lt;&gt;@,;:\\&quot;/\[\]\?=\{\}\s\t]])+)|(&quot;(([\p{IsLatin-1Supplement}\p{IsBasicLatin}-[\p{Cc}&#127;&quot;\n\r]]|(\s+))|(\\[\p{IsBasicLatin}]))*&quot;))))*)"
      />
    </xs:restriction>
  </xs:simpleType>

  <xs:simpleType name="ST_Extension">
    <xs:restriction base="xs:string">
      <xs:pattern
        value="([!$&amp;'\(\)\*\+,:=]|(%[0-9a-fA-F][0-9a-fA-F])|[:@]|[a-zA-Z0-9\-_~])+"/>
    </xs:restriction>
  </xs:simpleType>
</xs:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ecma/fouth-edition/opc-coreProperties.xsd
================================================
﻿<?xml version="1.0" encoding="UTF-8"?>
<xs:schema targetNamespace="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
  xmlns="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
  xmlns:xs="http://www.w3.org/2001/XMLSchema" xmlns:dc="http://purl.org/dc/elements/1.1/"
  xmlns:dcterms="http://purl.org/dc/terms/" elementFormDefault="qualified" blockDefault="#all">

  <xs:import namespace="http://purl.org/dc/elements/1.1/"
    schemaLocation="http://dublincore.org/schemas/xmls/qdc/2003/04/02/dc.xsd"/>
  <xs:import namespace="http://purl.org/dc/terms/"
    schemaLocation="http://dublincore.org/schemas/xmls/qdc/2003/04/02/dcterms.xsd"/>
  <xs:import id="xml" namespace="http://www.w3.org/XML/1998/namespace"/>

  <xs:element name="coreProperties" type="CT_CoreProperties"/>

  <xs:complexType name="CT_CoreProperties">
    <xs:all>
      <xs:element name="category" minOccurs="0" maxOccurs="1" type="xs:string"/>
      <xs:element name="contentStatus" minOccurs="0" maxOccurs="1" type="xs:string"/>
      <xs:element ref="dcterms:created" minOccurs="0" maxOccurs="1"/>
      <xs:element ref="dc:creator" minOccurs="0" maxOccurs="1"/>
      <xs:element ref="dc:description" minOccurs="0" maxOccurs="1"/>
      <xs:element ref="dc:identifier" minOccurs="0" maxOccurs="1"/>
      <xs:element name="keywords" minOccurs="0" maxOccurs="1" type="CT_Keywords"/>
      <xs:element ref="dc:language" minOccurs="0" maxOccurs="1"/>
      <xs:element name="lastModifiedBy" minOccurs="0" maxOccurs="1" type="xs:string"/>
      <xs:element name="lastPrinted" minOccurs="0" maxOccurs="1" type="xs:dateTime"/>
      <xs:element ref="dcterms:modified" minOccurs="0" maxOccurs="1"/>
      <xs:element name="revision" minOccurs="0" maxOccurs="1" type="xs:string"/>
      <xs:element ref="dc:subject" minOccurs="0" maxOccurs="1"/>
      <xs:element ref="dc:title" minOccurs="0" maxOccurs="1"/>
      <xs:element name="version" minOccurs="0" maxOccurs="1" type="xs:string"/>
    </xs:all>
  </xs:complexType>

  <xs:complexType name="CT_Keywords" mixed="true">
    <xs:sequence>
      <xs:element name="value" minOccurs="0" maxOccurs="unbounded" type="CT_Keyword"/>
    </xs:sequence>
    <xs:attribute ref="xml:lang" use="optional"/>
  </xs:complexType>

  <xs:complexType name="CT_Keyword">
    <xs:simpleContent>
      <xs:extension base="xs:string">
        <xs:attribute ref="xml:lang" use="optional"/>
      </xs:extension>
    </xs:simpleContent>
  </xs:complexType>

</xs:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ecma/fouth-edition/opc-digSig.xsd
================================================
<?xml version="1.0" encoding="UTF-8"?>
<xsd:schema xmlns="http://schemas.openxmlformats.org/package/2006/digital-signature"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  targetNamespace="http://schemas.openxmlformats.org/package/2006/digital-signature"
  elementFormDefault="qualified" attributeFormDefault="unqualified" blockDefault="#all">

  <xsd:element name="SignatureTime" type="CT_SignatureTime"/>
  <xsd:element name="RelationshipReference" type="CT_RelationshipReference"/>
  <xsd:element name="RelationshipsGroupReference" type="CT_RelationshipsGroupReference"/>

  <xsd:complexType name="CT_SignatureTime">
    <xsd:sequence>
      <xsd:element name="Format" type="ST_Format"/>
      <xsd:element name="Value" type="ST_Value"/>
    </xsd:sequence>
  </xsd:complexType>

  <xsd:complexType name="CT_RelationshipReference">
    <xsd:simpleContent>
      <xsd:extension base="xsd:string">
        <xsd:attribute name="SourceId" type="xsd:string" use="required"/>
      </xsd:extension>
    </xsd:simpleContent>
  </xsd:complexType>

  <xsd:complexType name="CT_RelationshipsGroupReference">
    <xsd:simpleContent>
      <xsd:extension base="xsd:string">
        <xsd:attribute name="SourceType" type="xsd:anyURI" use="required"/>
      </xsd:extension>
    </xsd:simpleContent>
  </xsd:complexType>

  <xsd:simpleType name="ST_Format">
    <xsd:restriction base="xsd:string">
      <xsd:pattern
        value="(YYYY)|(YYYY-MM)|(YYYY-MM-DD)|(YYYY-MM-DDThh:mmTZD)|(YYYY-MM-DDThh:mm:ssTZD)|(YYYY-MM-DDThh:mm:ss.sTZD)"
      />
    </xsd:restriction>
  </xsd:simpleType>

  <xsd:simpleType name="ST_Value">
    <xsd:restriction base="xsd:string">
      <xsd:pattern
        value="(([0-9][0-9][0-9][0-9]))|(([0-9][0-9][0-9][0-9])-((0[1-9])|(1(0|1|2))))|(([0-9][0-9][0-9][0-9])-((0[1-9])|(1(0|1|2)))-((0[1-9])|(1[0-9])|(2[0-9])|(3(0|1))))|(([0-9][0-9][0-9][0-9])-((0[1-9])|(1(0|1|2)))-((0[1-9])|(1[0-9])|(2[0-9])|(3(0|1)))T((0[0-9])|(1[0-9])|(2(0|1|2|3))):((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9]))(((\+|-)((0[0-9])|(1[0-9])|(2(0|1|2|3))):((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9])))|Z))|(([0-9][0-9][0-9][0-9])-((0[1-9])|(1(0|1|2)))-((0[1-9])|(1[0-9])|(2[0-9])|(3(0|1)))T((0[0-9])|(1[0-9])|(2(0|1|2|3))):((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9])):((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9]))(((\+|-)((0[0-9])|(1[0-9])|(2(0|1|2|3))):((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9])))|Z))|(([0-9][0-9][0-9][0-9])-((0[1-9])|(1(0|1|2)))-((0[1-9])|(1[0-9])|(2[0-9])|(3(0|1)))T((0[0-9])|(1[0-9])|(2(0|1|2|3))):((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9])):(((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9]))\.[0-9])(((\+|-)((0[0-9])|(1[0-9])|(2(0|1|2|3))):((0[0-9])|(1[0-9])|(2[0-9])|(3[0-9])|(4[0-9])|(5[0-9])))|Z))"
      />
    </xsd:restriction>
  </xsd:simpleType>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ecma/fouth-edition/opc-relationships.xsd
================================================
﻿<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<xsd:schema xmlns="http://schemas.openxmlformats.org/package/2006/relationships"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  targetNamespace="http://schemas.openxmlformats.org/package/2006/relationships"
  elementFormDefault="qualified" attributeFormDefault="unqualified" blockDefault="#all">

  <xsd:element name="Relationships" type="CT_Relationships"/>
  <xsd:element name="Relationship" type="CT_Relationship"/>

  <xsd:complexType name="CT_Relationships">
    <xsd:sequence>
      <xsd:element ref="Relationship" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>

  <xsd:complexType name="CT_Relationship">
    <xsd:simpleContent>
      <xsd:extension base="xsd:string">
        <xsd:attribute name="TargetMode" type="ST_TargetMode" use="optional"/>
        <xsd:attribute name="Target" type="xsd:anyURI" use="required"/>
        <xsd:attribute name="Type" type="xsd:anyURI" use="required"/>
        <xsd:attribute name="Id" type="xsd:ID" use="required"/>
      </xsd:extension>
    </xsd:simpleContent>
  </xsd:complexType>

  <xsd:simpleType name="ST_TargetMode">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="External"/>
      <xsd:enumeration value="Internal"/>
    </xsd:restriction>
  </xsd:simpleType>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/dml-chartDrawing.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
  xmlns="http://schemas.openxmlformats.org/drawingml/2006/chartDrawing"
  targetNamespace="http://schemas.openxmlformats.org/drawingml/2006/chartDrawing"
  elementFormDefault="qualified">
  <xsd:import namespace="http://schemas.openxmlformats.org/drawingml/2006/main"
    schemaLocation="dml-main.xsd"/>
  <xsd:complexType name="CT_ShapeNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvSpPr" type="a:CT_NonVisualDrawingShapeProps" minOccurs="1" maxOccurs="1"
      />
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Shape">
    <xsd:sequence>
      <xsd:element name="nvSpPr" type="CT_ShapeNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="style" type="a:CT_ShapeStyle" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="txBody" type="a:CT_TextBody" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional"/>
    <xsd:attribute name="textlink" type="xsd:string" use="optional"/>
    <xsd:attribute name="fLocksText" type="xsd:boolean" use="optional" default="true"/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_ConnectorNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvCxnSpPr" type="a:CT_NonVisualConnectorProperties" minOccurs="1"
        maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Connector">
    <xsd:sequence>
      <xsd:element name="nvCxnSpPr" type="CT_ConnectorNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="style" type="a:CT_ShapeStyle" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional"/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_PictureNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvPicPr" type="a:CT_NonVisualPictureProperties" minOccurs="1"
        maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Picture">
    <xsd:sequence>
      <xsd:element name="nvPicPr" type="CT_PictureNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="blipFill" type="a:CT_BlipFillProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="style" type="a:CT_ShapeStyle" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional" default=""/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_GraphicFrameNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvGraphicFramePr" type="a:CT_NonVisualGraphicFrameProperties"
        minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_GraphicFrame">
    <xsd:sequence>
      <xsd:element name="nvGraphicFramePr" type="CT_GraphicFrameNonVisual" minOccurs="1"
        maxOccurs="1"/>
      <xsd:element name="xfrm" type="a:CT_Transform2D" minOccurs="1" maxOccurs="1"/>
      <xsd:element ref="a:graphic" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional"/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_GroupShapeNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvGrpSpPr" type="a:CT_NonVisualGroupDrawingShapeProps" minOccurs="1"
        maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_GroupShape">
    <xsd:sequence>
      <xsd:element name="nvGrpSpPr" type="CT_GroupShapeNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="grpSpPr" type="a:CT_GroupShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:choice minOccurs="0" maxOccurs="unbounded">
        <xsd:element name="sp" type="CT_Shape"/>
        <xsd:element name="grpSp" type="CT_GroupShape"/>
        <xsd:element name="graphicFrame" type="CT_GraphicFrame"/>
        <xsd:element name="cxnSp" type="CT_Connector"/>
        <xsd:element name="pic" type="CT_Picture"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:group name="EG_ObjectChoices">
    <xsd:sequence>
      <xsd:choice minOccurs="1" maxOccurs="1">
        <xsd:element name="sp" type="CT_Shape"/>
        <xsd:element name="grpSp" type="CT_GroupShape"/>
        <xsd:element name="graphicFrame" type="CT_GraphicFrame"/>
        <xsd:element name="cxnSp" type="CT_Connector"/>
        <xsd:element name="pic" type="CT_Picture"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:group>
  <xsd:simpleType name="ST_MarkerCoordinate">
    <xsd:restriction base="xsd:double">
      <xsd:minInclusive value="0.0"/>
      <xsd:maxInclusive value="1.0"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Marker">
    <xsd:sequence>
      <xsd:element name="x" type="ST_MarkerCoordinate" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="y" type="ST_MarkerCoordinate" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_RelSizeAnchor">
    <xsd:sequence>
      <xsd:element name="from" type="CT_Marker"/>
      <xsd:element name="to" type="CT_Marker"/>
      <xsd:group ref="EG_ObjectChoices"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_AbsSizeAnchor">
    <xsd:sequence>
      <xsd:element name="from" type="CT_Marker"/>
      <xsd:element name="ext" type="a:CT_PositiveSize2D"/>
      <xsd:group ref="EG_ObjectChoices"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:group name="EG_Anchor">
    <xsd:choice>
      <xsd:element name="relSizeAnchor" type="CT_RelSizeAnchor"/>
      <xsd:element name="absSizeAnchor" type="CT_AbsSizeAnchor"/>
    </xsd:choice>
  </xsd:group>
  <xsd:complexType name="CT_Drawing">
    <xsd:sequence>
      <xsd:group ref="EG_Anchor" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/dml-lockedCanvas.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/drawingml/2006/lockedCanvas"
  xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
  xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  elementFormDefault="qualified"
  targetNamespace="http://schemas.openxmlformats.org/drawingml/2006/lockedCanvas">
  <xsd:import namespace="http://schemas.openxmlformats.org/drawingml/2006/main"
    schemaLocation="dml-main.xsd"/>
  <xsd:element name="lockedCanvas" type="a:CT_GvmlGroupShape"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/dml-picture.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/drawingml/2006/picture"
  xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main" elementFormDefault="qualified"
  targetNamespace="http://schemas.openxmlformats.org/drawingml/2006/picture">
  <xsd:import namespace="http://schemas.openxmlformats.org/drawingml/2006/main"
    schemaLocation="dml-main.xsd"/>
  <xsd:complexType name="CT_PictureNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvPicPr" type="a:CT_NonVisualPictureProperties" minOccurs="1"
        maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Picture">
    <xsd:sequence minOccurs="1" maxOccurs="1">
      <xsd:element name="nvPicPr" type="CT_PictureNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="blipFill" type="a:CT_BlipFillProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:element name="pic" type="CT_Picture"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/dml-spreadsheetDrawing.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
  xmlns="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
  xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  targetNamespace="http://schemas.openxmlformats.org/drawingml/2006/spreadsheetDrawing"
  elementFormDefault="qualified">
  <xsd:import namespace="http://schemas.openxmlformats.org/drawingml/2006/main"
    schemaLocation="dml-main.xsd"/>
  <xsd:import schemaLocation="shared-relationshipReference.xsd"
    namespace="http://schemas.openxmlformats.org/officeDocument/2006/relationships"/>
  <xsd:element name="from" type="CT_Marker"/>
  <xsd:element name="to" type="CT_Marker"/>
  <xsd:complexType name="CT_AnchorClientData">
    <xsd:attribute name="fLocksWithSheet" type="xsd:boolean" use="optional" default="true"/>
    <xsd:attribute name="fPrintsWithSheet" type="xsd:boolean" use="optional" default="true"/>
  </xsd:complexType>
  <xsd:complexType name="CT_ShapeNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvSpPr" type="a:CT_NonVisualDrawingShapeProps" minOccurs="1" maxOccurs="1"
      />
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Shape">
    <xsd:sequence>
      <xsd:element name="nvSpPr" type="CT_ShapeNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="style" type="a:CT_ShapeStyle" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="txBody" type="a:CT_TextBody" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional"/>
    <xsd:attribute name="textlink" type="xsd:string" use="optional"/>
    <xsd:attribute name="fLocksText" type="xsd:boolean" use="optional" default="true"/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_ConnectorNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvCxnSpPr" type="a:CT_NonVisualConnectorProperties" minOccurs="1"
        maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Connector">
    <xsd:sequence>
      <xsd:element name="nvCxnSpPr" type="CT_ConnectorNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="style" type="a:CT_ShapeStyle" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional"/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_PictureNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvPicPr" type="a:CT_NonVisualPictureProperties" minOccurs="1"
        maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Picture">
    <xsd:sequence>
      <xsd:element name="nvPicPr" type="CT_PictureNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="blipFill" type="a:CT_BlipFillProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="style" type="a:CT_ShapeStyle" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional" default=""/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_GraphicalObjectFrameNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvGraphicFramePr" type="a:CT_NonVisualGraphicFrameProperties"
        minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_GraphicalObjectFrame">
    <xsd:sequence>
      <xsd:element name="nvGraphicFramePr" type="CT_GraphicalObjectFrameNonVisual" minOccurs="1"
        maxOccurs="1"/>
      <xsd:element name="xfrm" type="a:CT_Transform2D" minOccurs="1" maxOccurs="1"/>
      <xsd:element ref="a:graphic" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="macro" type="xsd:string" use="optional"/>
    <xsd:attribute name="fPublished" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_GroupShapeNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvGrpSpPr" type="a:CT_NonVisualGroupDrawingShapeProps" minOccurs="1"
        maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_GroupShape">
    <xsd:sequence>
      <xsd:element name="nvGrpSpPr" type="CT_GroupShapeNonVisual" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="grpSpPr" type="a:CT_GroupShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:choice minOccurs="0" maxOccurs="unbounded">
        <xsd:element name="sp" type="CT_Shape"/>
        <xsd:element name="grpSp" type="CT_GroupShape"/>
        <xsd:element name="graphicFrame" type="CT_GraphicalObjectFrame"/>
        <xsd:element name="cxnSp" type="CT_Connector"/>
        <xsd:element name="pic" type="CT_Picture"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:group name="EG_ObjectChoices">
    <xsd:sequence>
      <xsd:choice minOccurs="1" maxOccurs="1">
        <xsd:element name="sp" type="CT_Shape"/>
        <xsd:element name="grpSp" type="CT_GroupShape"/>
        <xsd:element name="graphicFrame" type="CT_GraphicalObjectFrame"/>
        <xsd:element name="cxnSp" type="CT_Connector"/>
        <xsd:element name="pic" type="CT_Picture"/>
        <xsd:element name="contentPart" type="CT_Rel"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:group>
  <xsd:complexType name="CT_Rel">
    <xsd:attribute ref="r:id" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_ColID">
    <xsd:restriction base="xsd:int">
      <xsd:minInclusive value="0"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_RowID">
    <xsd:restriction base="xsd:int">
      <xsd:minInclusive value="0"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Marker">
    <xsd:sequence>
      <xsd:element name="col" type="ST_ColID"/>
      <xsd:element name="colOff" type="a:ST_Coordinate"/>
      <xsd:element name="row" type="ST_RowID"/>
      <xsd:element name="rowOff" type="a:ST_Coordinate"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:simpleType name="ST_EditAs">
    <xsd:restriction base="xsd:token">
      <xsd:enumeration value="twoCell"/>
      <xsd:enumeration value="oneCell"/>
      <xsd:enumeration value="absolute"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_TwoCellAnchor">
    <xsd:sequence>
      <xsd:element name="from" type="CT_Marker"/>
      <xsd:element name="to" type="CT_Marker"/>
      <xsd:group ref="EG_ObjectChoices"/>
      <xsd:element name="clientData" type="CT_AnchorClientData" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="editAs" type="ST_EditAs" use="optional" default="twoCell"/>
  </xsd:complexType>
  <xsd:complexType name="CT_OneCellAnchor">
    <xsd:sequence>
      <xsd:element name="from" type="CT_Marker"/>
      <xsd:element name="ext" type="a:CT_PositiveSize2D"/>
      <xsd:group ref="EG_ObjectChoices"/>
      <xsd:element name="clientData" type="CT_AnchorClientData" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_AbsoluteAnchor">
    <xsd:sequence>
      <xsd:element name="pos" type="a:CT_Point2D"/>
      <xsd:element name="ext" type="a:CT_PositiveSize2D"/>
      <xsd:group ref="EG_ObjectChoices"/>
      <xsd:element name="clientData" type="CT_AnchorClientData" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:group name="EG_Anchor">
    <xsd:choice>
      <xsd:element name="twoCellAnchor" type="CT_TwoCellAnchor"/>
      <xsd:element name="oneCellAnchor" type="CT_OneCellAnchor"/>
      <xsd:element name="absoluteAnchor" type="CT_AbsoluteAnchor"/>
    </xsd:choice>
  </xsd:group>
  <xsd:complexType name="CT_Drawing">
    <xsd:sequence>
      <xsd:group ref="EG_Anchor" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:element name="wsDr" type="CT_Drawing"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/dml-wordprocessingDrawing.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"
  xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
  xmlns:dpct="http://schemas.openxmlformats.org/drawingml/2006/picture"
  xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  xmlns="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
  targetNamespace="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
  elementFormDefault="qualified">
  <xsd:import namespace="http://schemas.openxmlformats.org/drawingml/2006/main"
    schemaLocation="dml-main.xsd"/>
  <xsd:import schemaLocation="wml.xsd"
    namespace="http://schemas.openxmlformats.org/wordprocessingml/2006/main"/>
  <xsd:import namespace="http://schemas.openxmlformats.org/drawingml/2006/picture"
    schemaLocation="dml-picture.xsd"/>
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
    schemaLocation="shared-relationshipReference.xsd"/>
  <xsd:complexType name="CT_EffectExtent">
    <xsd:attribute name="l" type="a:ST_Coordinate" use="required"/>
    <xsd:attribute name="t" type="a:ST_Coordinate" use="required"/>
    <xsd:attribute name="r" type="a:ST_Coordinate" use="required"/>
    <xsd:attribute name="b" type="a:ST_Coordinate" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_WrapDistance">
    <xsd:restriction base="xsd:unsignedInt"/>
  </xsd:simpleType>
  <xsd:complexType name="CT_Inline">
    <xsd:sequence>
      <xsd:element name="extent" type="a:CT_PositiveSize2D"/>
      <xsd:element name="effectExtent" type="CT_EffectExtent" minOccurs="0"/>
      <xsd:element name="docPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvGraphicFramePr" type="a:CT_NonVisualGraphicFrameProperties"
        minOccurs="0" maxOccurs="1"/>
      <xsd:element ref="a:graphic" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="distT" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distB" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distL" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distR" type="ST_WrapDistance" use="optional"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_WrapText">
    <xsd:restriction base="xsd:token">
      <xsd:enumeration value="bothSides"/>
      <xsd:enumeration value="left"/>
      <xsd:enumeration value="right"/>
      <xsd:enumeration value="largest"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_WrapPath">
    <xsd:sequence>
      <xsd:element name="start" type="a:CT_Point2D" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="lineTo" type="a:CT_Point2D" minOccurs="2" maxOccurs="unbounded"/>
    </xsd:sequence>
    <xsd:attribute name="edited" type="xsd:boolean" use="optional"/>
  </xsd:complexType>
  <xsd:complexType name="CT_WrapNone"/>
  <xsd:complexType name="CT_WrapSquare">
    <xsd:sequence>
      <xsd:element name="effectExtent" type="CT_EffectExtent" minOccurs="0"/>
    </xsd:sequence>
    <xsd:attribute name="wrapText" type="ST_WrapText" use="required"/>
    <xsd:attribute name="distT" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distB" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distL" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distR" type="ST_WrapDistance" use="optional"/>
  </xsd:complexType>
  <xsd:complexType name="CT_WrapTight">
    <xsd:sequence>
      <xsd:element name="wrapPolygon" type="CT_WrapPath" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="wrapText" type="ST_WrapText" use="required"/>
    <xsd:attribute name="distL" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distR" type="ST_WrapDistance" use="optional"/>
  </xsd:complexType>
  <xsd:complexType name="CT_WrapThrough">
    <xsd:sequence>
      <xsd:element name="wrapPolygon" type="CT_WrapPath" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="wrapText" type="ST_WrapText" use="required"/>
    <xsd:attribute name="distL" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distR" type="ST_WrapDistance" use="optional"/>
  </xsd:complexType>
  <xsd:complexType name="CT_WrapTopBottom">
    <xsd:sequence>
      <xsd:element name="effectExtent" type="CT_EffectExtent" minOccurs="0"/>
    </xsd:sequence>
    <xsd:attribute name="distT" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distB" type="ST_WrapDistance" use="optional"/>
  </xsd:complexType>
  <xsd:group name="EG_WrapType">
    <xsd:sequence>
      <xsd:choice minOccurs="1" maxOccurs="1">
        <xsd:element name="wrapNone" type="CT_WrapNone" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="wrapSquare" type="CT_WrapSquare" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="wrapTight" type="CT_WrapTight" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="wrapThrough" type="CT_WrapThrough" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="wrapTopAndBottom" type="CT_WrapTopBottom" minOccurs="1" maxOccurs="1"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:group>
  <xsd:simpleType name="ST_PositionOffset">
    <xsd:restriction base="xsd:int"/>
  </xsd:simpleType>
  <xsd:simpleType name="ST_AlignH">
    <xsd:restriction base="xsd:token">
      <xsd:enumeration value="left"/>
      <xsd:enumeration value="right"/>
      <xsd:enumeration value="center"/>
      <xsd:enumeration value="inside"/>
      <xsd:enumeration value="outside"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_RelFromH">
    <xsd:restriction base="xsd:token">
      <xsd:enumeration value="margin"/>
      <xsd:enumeration value="page"/>
      <xsd:enumeration value="column"/>
      <xsd:enumeration value="character"/>
      <xsd:enumeration value="leftMargin"/>
      <xsd:enumeration value="rightMargin"/>
      <xsd:enumeration value="insideMargin"/>
      <xsd:enumeration value="outsideMargin"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_PosH">
    <xsd:sequence>
      <xsd:choice minOccurs="1" maxOccurs="1">
        <xsd:element name="align" type="ST_AlignH" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="posOffset" type="ST_PositionOffset" minOccurs="1" maxOccurs="1"/>
      </xsd:choice>
    </xsd:sequence>
    <xsd:attribute name="relativeFrom" type="ST_RelFromH" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_AlignV">
    <xsd:restriction base="xsd:token">
      <xsd:enumeration value="top"/>
      <xsd:enumeration value="bottom"/>
      <xsd:enumeration value="center"/>
      <xsd:enumeration value="inside"/>
      <xsd:enumeration value="outside"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_RelFromV">
    <xsd:restriction base="xsd:token">
      <xsd:enumeration value="margin"/>
      <xsd:enumeration value="page"/>
      <xsd:enumeration value="paragraph"/>
      <xsd:enumeration value="line"/>
      <xsd:enumeration value="topMargin"/>
      <xsd:enumeration value="bottomMargin"/>
      <xsd:enumeration value="insideMargin"/>
      <xsd:enumeration value="outsideMargin"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_PosV">
    <xsd:sequence>
      <xsd:choice minOccurs="1" maxOccurs="1">
        <xsd:element name="align" type="ST_AlignV" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="posOffset" type="ST_PositionOffset" minOccurs="1" maxOccurs="1"/>
      </xsd:choice>
    </xsd:sequence>
    <xsd:attribute name="relativeFrom" type="ST_RelFromV" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_Anchor">
    <xsd:sequence>
      <xsd:element name="simplePos" type="a:CT_Point2D"/>
      <xsd:element name="positionH" type="CT_PosH"/>
      <xsd:element name="positionV" type="CT_PosV"/>
      <xsd:element name="extent" type="a:CT_PositiveSize2D"/>
      <xsd:element name="effectExtent" type="CT_EffectExtent" minOccurs="0"/>
      <xsd:group ref="EG_WrapType"/>
      <xsd:element name="docPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvGraphicFramePr" type="a:CT_NonVisualGraphicFrameProperties"
        minOccurs="0" maxOccurs="1"/>
      <xsd:element ref="a:graphic" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="distT" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distB" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distL" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="distR" type="ST_WrapDistance" use="optional"/>
    <xsd:attribute name="simplePos" type="xsd:boolean"/>
    <xsd:attribute name="relativeHeight" type="xsd:unsignedInt" use="required"/>
    <xsd:attribute name="behindDoc" type="xsd:boolean" use="required"/>
    <xsd:attribute name="locked" type="xsd:boolean" use="required"/>
    <xsd:attribute name="layoutInCell" type="xsd:boolean" use="required"/>
    <xsd:attribute name="hidden" type="xsd:boolean" use="optional"/>
    <xsd:attribute name="allowOverlap" type="xsd:boolean" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_TxbxContent">
    <xsd:group ref="w:EG_BlockLevelElts" minOccurs="1" maxOccurs="unbounded"/>
  </xsd:complexType>
  <xsd:complexType name="CT_TextboxInfo">
    <xsd:sequence>
      <xsd:element name="txbxContent" type="CT_TxbxContent" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="extLst" type="a:CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="id" type="xsd:unsignedShort" use="optional" default="0"/>
  </xsd:complexType>
  <xsd:complexType name="CT_LinkedTextboxInformation">
    <xsd:sequence>
      <xsd:element name="extLst" type="a:CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="id" type="xsd:unsignedShort" use="required"/>
    <xsd:attribute name="seq" type="xsd:unsignedShort" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_WordprocessingShape">
    <xsd:sequence minOccurs="1" maxOccurs="1">
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="0" maxOccurs="1"/>
      <xsd:choice minOccurs="1" maxOccurs="1">
        <xsd:element name="cNvSpPr" type="a:CT_NonVisualDrawingShapeProps" minOccurs="1"
          maxOccurs="1"/>
        <xsd:element name="cNvCnPr" type="a:CT_NonVisualConnectorProperties" minOccurs="1"
          maxOccurs="1"/>
      </xsd:choice>
      <xsd:element name="spPr" type="a:CT_ShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="style" type="a:CT_ShapeStyle" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="extLst" type="a:CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
      <xsd:choice minOccurs="0" maxOccurs="1">
        <xsd:element name="txbx" type="CT_TextboxInfo" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="linkedTxbx" type="CT_LinkedTextboxInformation" minOccurs="1"
          maxOccurs="1"/>
      </xsd:choice>
      <xsd:element name="bodyPr" type="a:CT_TextBodyProperties" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="normalEastAsianFlow" type="xsd:boolean" use="optional" default="false"/>
  </xsd:complexType>
  <xsd:complexType name="CT_GraphicFrame">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="cNvFrPr" type="a:CT_NonVisualGraphicFrameProperties" minOccurs="1"
        maxOccurs="1"/>
      <xsd:element name="xfrm" type="a:CT_Transform2D" minOccurs="1" maxOccurs="1"/>
      <xsd:element ref="a:graphic" minOccurs="1" maxOccurs="1"/>
      <xsd:element name="extLst" type="a:CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_WordprocessingContentPartNonVisual">
    <xsd:sequence>
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="cNvContentPartPr" type="a:CT_NonVisualContentPartProperties" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_WordprocessingContentPart">
    <xsd:sequence>
      <xsd:element name="nvContentPartPr" type="CT_WordprocessingContentPartNonVisual" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="xfrm" type="a:CT_Transform2D" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="extLst" type="a:CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
    <xsd:attribute name="bwMode" type="a:ST_BlackWhiteMode" use="optional"/>
    <xsd:attribute ref="r:id" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_WordprocessingGroup">
    <xsd:sequence minOccurs="1" maxOccurs="1">
      <xsd:element name="cNvPr" type="a:CT_NonVisualDrawingProps" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="cNvGrpSpPr" type="a:CT_NonVisualGroupDrawingShapeProps" minOccurs="1"
        maxOccurs="1"/>
      <xsd:element name="grpSpPr" type="a:CT_GroupShapeProperties" minOccurs="1" maxOccurs="1"/>
      <xsd:choice minOccurs="0" maxOccurs="unbounded">
        <xsd:element ref="wsp"/>
        <xsd:element name="grpSp" type="CT_WordprocessingGroup"/>
        <xsd:element name="graphicFrame" type="CT_GraphicFrame"/>
        <xsd:element ref="dpct:pic"/>
        <xsd:element name="contentPart" type="CT_WordprocessingContentPart"/>
      </xsd:choice>
      <xsd:element name="extLst" type="a:CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_WordprocessingCanvas">
    <xsd:sequence minOccurs="1" maxOccurs="1">
      <xsd:element name="bg" type="a:CT_BackgroundFormatting" minOccurs="0" maxOccurs="1"/>
      <xsd:element name="whole" type="a:CT_WholeE2oFormatting" minOccurs="0" maxOccurs="1"/>
      <xsd:choice minOccurs="0" maxOccurs="unbounded">
        <xsd:element ref="wsp"/>
        <xsd:element ref="dpct:pic"/>
        <xsd:element name="contentPart" type="CT_WordprocessingContentPart"/>
        <xsd:element ref="wgp"/>
        <xsd:element name="graphicFrame" type="CT_GraphicFrame"/>
      </xsd:choice>
      <xsd:element name="extLst" type="a:CT_OfficeArtExtensionList" minOccurs="0" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:element name="wpc" type="CT_WordprocessingCanvas"/>
  <xsd:element name="wgp" type="CT_WordprocessingGroup"/>
  <xsd:element name="wsp" type="CT_WordprocessingShape"/>
  <xsd:element name="inline" type="CT_Inline"/>
  <xsd:element name="anchor" type="CT_Anchor"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-additionalCharacteristics.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/characteristics"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/characteristics"
  elementFormDefault="qualified">
  <xsd:complexType name="CT_AdditionalCharacteristics">
    <xsd:sequence>
      <xsd:element name="characteristic" type="CT_Characteristic" minOccurs="0"
        maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Characteristic">
    <xsd:attribute name="name" type="xsd:string" use="required"/>
    <xsd:attribute name="relation" type="ST_Relation" use="required"/>
    <xsd:attribute name="val" type="xsd:string" use="required"/>
    <xsd:attribute name="vocabulary" type="xsd:anyURI" use="optional"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_Relation">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="ge"/>
      <xsd:enumeration value="le"/>
      <xsd:enumeration value="gt"/>
      <xsd:enumeration value="lt"/>
      <xsd:enumeration value="eq"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:element name="additionalCharacteristics" type="CT_AdditionalCharacteristics"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-bibliography.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/bibliography"
  xmlns:s="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/bibliography"
  elementFormDefault="qualified">
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
    schemaLocation="shared-commonSimpleTypes.xsd"/>
  <xsd:simpleType name="ST_SourceType">
    <xsd:restriction base="s:ST_String">
      <xsd:enumeration value="ArticleInAPeriodical"/>
      <xsd:enumeration value="Book"/>
      <xsd:enumeration value="BookSection"/>
      <xsd:enumeration value="JournalArticle"/>
      <xsd:enumeration value="ConferenceProceedings"/>
      <xsd:enumeration value="Report"/>
      <xsd:enumeration value="SoundRecording"/>
      <xsd:enumeration value="Performance"/>
      <xsd:enumeration value="Art"/>
      <xsd:enumeration value="DocumentFromInternetSite"/>
      <xsd:enumeration value="InternetSite"/>
      <xsd:enumeration value="Film"/>
      <xsd:enumeration value="Interview"/>
      <xsd:enumeration value="Patent"/>
      <xsd:enumeration value="ElectronicSource"/>
      <xsd:enumeration value="Case"/>
      <xsd:enumeration value="Misc"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_NameListType">
    <xsd:sequence>
      <xsd:element name="Person" type="CT_PersonType" minOccurs="1" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_PersonType">
    <xsd:sequence>
      <xsd:element name="Last" type="s:ST_String" minOccurs="0" maxOccurs="unbounded"/>
      <xsd:element name="First" type="s:ST_String" minOccurs="0" maxOccurs="unbounded"/>
      <xsd:element name="Middle" type="s:ST_String" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_NameType">
    <xsd:sequence>
      <xsd:element name="NameList" type="CT_NameListType" minOccurs="1" maxOccurs="1"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_NameOrCorporateType">
    <xsd:sequence>
      <xsd:choice minOccurs="0" maxOccurs="1">
        <xsd:element name="NameList" type="CT_NameListType" minOccurs="1" maxOccurs="1"/>
        <xsd:element name="Corporate" minOccurs="1" maxOccurs="1" type="s:ST_String"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_AuthorType">
    <xsd:sequence>
      <xsd:choice minOccurs="0" maxOccurs="unbounded">
        <xsd:element name="Artist" type="CT_NameType"/>
        <xsd:element name="Author" type="CT_NameOrCorporateType"/>
        <xsd:element name="BookAuthor" type="CT_NameType"/>
        <xsd:element name="Compiler" type="CT_NameType"/>
        <xsd:element name="Composer" type="CT_NameType"/>
        <xsd:element name="Conductor" type="CT_NameType"/>
        <xsd:element name="Counsel" type="CT_NameType"/>
        <xsd:element name="Director" type="CT_NameType"/>
        <xsd:element name="Editor" type="CT_NameType"/>
        <xsd:element name="Interviewee" type="CT_NameType"/>
        <xsd:element name="Interviewer" type="CT_NameType"/>
        <xsd:element name="Inventor" type="CT_NameType"/>
        <xsd:element name="Performer" type="CT_NameOrCorporateType"/>
        <xsd:element name="ProducerName" type="CT_NameType"/>
        <xsd:element name="Translator" type="CT_NameType"/>
        <xsd:element name="Writer" type="CT_NameType"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SourceType">
    <xsd:sequence>
      <xsd:choice minOccurs="0" maxOccurs="unbounded">
        <xsd:element name="AbbreviatedCaseNumber" type="s:ST_String"/>
        <xsd:element name="AlbumTitle" type="s:ST_String"/>
        <xsd:element name="Author" type="CT_AuthorType"/>
        <xsd:element name="BookTitle" type="s:ST_String"/>
        <xsd:element name="Broadcaster" type="s:ST_String"/>
        <xsd:element name="BroadcastTitle" type="s:ST_String"/>
        <xsd:element name="CaseNumber" type="s:ST_String"/>
        <xsd:element name="ChapterNumber" type="s:ST_String"/>
        <xsd:element name="City" type="s:ST_String"/>
        <xsd:element name="Comments" type="s:ST_String"/>
        <xsd:element name="ConferenceName" type="s:ST_String"/>
        <xsd:element name="CountryRegion" type="s:ST_String"/>
        <xsd:element name="Court" type="s:ST_String"/>
        <xsd:element name="Day" type="s:ST_String"/>
        <xsd:element name="DayAccessed" type="s:ST_String"/>
        <xsd:element name="Department" type="s:ST_String"/>
        <xsd:element name="Distributor" type="s:ST_String"/>
        <xsd:element name="Edition" type="s:ST_String"/>
        <xsd:element name="Guid" type="s:ST_String"/>
        <xsd:element name="Institution" type="s:ST_String"/>
        <xsd:element name="InternetSiteTitle" type="s:ST_String"/>
        <xsd:element name="Issue" type="s:ST_String"/>
        <xsd:element name="JournalName" type="s:ST_String"/>
        <xsd:element name="LCID" type="s:ST_Lang"/>
        <xsd:element name="Medium" type="s:ST_String"/>
        <xsd:element name="Month" type="s:ST_String"/>
        <xsd:element name="MonthAccessed" type="s:ST_String"/>
        <xsd:element name="NumberVolumes" type="s:ST_String"/>
        <xsd:element name="Pages" type="s:ST_String"/>
        <xsd:element name="PatentNumber" type="s:ST_String"/>
        <xsd:element name="PeriodicalTitle" type="s:ST_String"/>
        <xsd:element name="ProductionCompany" type="s:ST_String"/>
        <xsd:element name="PublicationTitle" type="s:ST_String"/>
        <xsd:element name="Publisher" type="s:ST_String"/>
        <xsd:element name="RecordingNumber" type="s:ST_String"/>
        <xsd:element name="RefOrder" type="s:ST_String"/>
        <xsd:element name="Reporter" type="s:ST_String"/>
        <xsd:element name="SourceType" type="ST_SourceType"/>
        <xsd:element name="ShortTitle" type="s:ST_String"/>
        <xsd:element name="StandardNumber" type="s:ST_String"/>
        <xsd:element name="StateProvince" type="s:ST_String"/>
        <xsd:element name="Station" type="s:ST_String"/>
        <xsd:element name="Tag" type="s:ST_String"/>
        <xsd:element name="Theater" type="s:ST_String"/>
        <xsd:element name="ThesisType" type="s:ST_String"/>
        <xsd:element name="Title" type="s:ST_String"/>
        <xsd:element name="Type" type="s:ST_String"/>
        <xsd:element name="URL" type="s:ST_String"/>
        <xsd:element name="Version" type="s:ST_String"/>
        <xsd:element name="Volume" type="s:ST_String"/>
        <xsd:element name="Year" type="s:ST_String"/>
        <xsd:element name="YearAccessed" type="s:ST_String"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:element name="Sources" type="CT_Sources"/>
  <xsd:complexType name="CT_Sources">
    <xsd:sequence>
      <xsd:element name="Source" type="CT_SourceType" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
    <xsd:attribute name="SelectedStyle" type="s:ST_String"/>
    <xsd:attribute name="StyleName" type="s:ST_String"/>
    <xsd:attribute name="URI" type="s:ST_String"/>
  </xsd:complexType>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-commonSimpleTypes.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  elementFormDefault="qualified">
  <xsd:simpleType name="ST_Lang">
    <xsd:restriction base="xsd:string"/>
  </xsd:simpleType>
  <xsd:simpleType name="ST_HexColorRGB">
    <xsd:restriction base="xsd:hexBinary">
      <xsd:length value="3" fixed="true"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_Panose">
    <xsd:restriction base="xsd:hexBinary">
      <xsd:length value="10"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_CalendarType">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="gregorian"/>
      <xsd:enumeration value="gregorianUs"/>
      <xsd:enumeration value="gregorianMeFrench"/>
      <xsd:enumeration value="gregorianArabic"/>
      <xsd:enumeration value="hijri"/>
      <xsd:enumeration value="hebrew"/>
      <xsd:enumeration value="taiwan"/>
      <xsd:enumeration value="japan"/>
      <xsd:enumeration value="thai"/>
      <xsd:enumeration value="korea"/>
      <xsd:enumeration value="saka"/>
      <xsd:enumeration value="gregorianXlitEnglish"/>
      <xsd:enumeration value="gregorianXlitFrench"/>
      <xsd:enumeration value="none"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_AlgClass">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="hash"/>
      <xsd:enumeration value="custom"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_CryptProv">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="rsaAES"/>
      <xsd:enumeration value="rsaFull"/>
      <xsd:enumeration value="custom"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_AlgType">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="typeAny"/>
      <xsd:enumeration value="custom"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_ColorType">
    <xsd:restriction base="xsd:string"/>
  </xsd:simpleType>
  <xsd:simpleType name="ST_Guid">
    <xsd:restriction base="xsd:token">
      <xsd:pattern value="\{[0-9A-F]{8}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{4}-[0-9A-F]{12}\}"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_OnOff">
    <xsd:union memberTypes="xsd:boolean ST_OnOff1"/>
  </xsd:simpleType>
  <xsd:simpleType name="ST_OnOff1">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="on"/>
      <xsd:enumeration value="off"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_String">
    <xsd:restriction base="xsd:string"/>
  </xsd:simpleType>
  <xsd:simpleType name="ST_XmlName">
    <xsd:restriction base="xsd:NCName">
      <xsd:minLength value="1"/>
      <xsd:maxLength value="255"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_TrueFalse">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="t"/>
      <xsd:enumeration value="f"/>
      <xsd:enumeration value="true"/>
      <xsd:enumeration value="false"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_TrueFalseBlank">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="t"/>
      <xsd:enumeration value="f"/>
      <xsd:enumeration value="true"/>
      <xsd:enumeration value="false"/>
      <xsd:enumeration value=""/>
      <xsd:enumeration value="True"/>
      <xsd:enumeration value="False"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_UnsignedDecimalNumber">
    <xsd:restriction base="xsd:decimal">
      <xsd:minInclusive value="0"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_TwipsMeasure">
    <xsd:union memberTypes="ST_UnsignedDecimalNumber ST_PositiveUniversalMeasure"/>
  </xsd:simpleType>
  <xsd:simpleType name="ST_VerticalAlignRun">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="baseline"/>
      <xsd:enumeration value="superscript"/>
      <xsd:enumeration value="subscript"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_Xstring">
    <xsd:restriction base="xsd:string"/>
  </xsd:simpleType>
  <xsd:simpleType name="ST_XAlign">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="left"/>
      <xsd:enumeration value="center"/>
      <xsd:enumeration value="right"/>
      <xsd:enumeration value="inside"/>
      <xsd:enumeration value="outside"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_YAlign">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="inline"/>
      <xsd:enumeration value="top"/>
      <xsd:enumeration value="center"/>
      <xsd:enumeration value="bottom"/>
      <xsd:enumeration value="inside"/>
      <xsd:enumeration value="outside"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_ConformanceClass">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="strict"/>
      <xsd:enumeration value="transitional"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_UniversalMeasure">
    <xsd:restriction base="xsd:string">
      <xsd:pattern value="-?[0-9]+(\.[0-9]+)?(mm|cm|in|pt|pc|pi)"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_PositiveUniversalMeasure">
    <xsd:restriction base="ST_UniversalMeasure">
      <xsd:pattern value="[0-9]+(\.[0-9]+)?(mm|cm|in|pt|pc|pi)"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_Percentage">
    <xsd:restriction base="xsd:string">
      <xsd:pattern value="-?[0-9]+(\.[0-9]+)?%"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_FixedPercentage">
    <xsd:restriction base="ST_Percentage">
      <xsd:pattern value="-?((100)|([0-9][0-9]?))(\.[0-9][0-9]?)?%"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_PositivePercentage">
    <xsd:restriction base="ST_Percentage">
      <xsd:pattern value="[0-9]+(\.[0-9]+)?%"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_PositiveFixedPercentage">
    <xsd:restriction base="ST_Percentage">
      <xsd:pattern value="((100)|([0-9][0-9]?))(\.[0-9][0-9]?)?%"/>
    </xsd:restriction>
  </xsd:simpleType>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-customXmlDataProperties.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/customXml"
  xmlns:s="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/customXml"
  elementFormDefault="qualified" attributeFormDefault="qualified" blockDefault="#all">
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
    schemaLocation="shared-commonSimpleTypes.xsd"/>
  <xsd:complexType name="CT_DatastoreSchemaRef">
    <xsd:attribute name="uri" type="xsd:string" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_DatastoreSchemaRefs">
    <xsd:sequence>
      <xsd:element name="schemaRef" type="CT_DatastoreSchemaRef" minOccurs="0" maxOccurs="unbounded"
      />
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_DatastoreItem">
    <xsd:sequence>
      <xsd:element name="schemaRefs" type="CT_DatastoreSchemaRefs" minOccurs="0"/>
    </xsd:sequence>
    <xsd:attribute name="itemID" type="s:ST_Guid" use="required"/>
  </xsd:complexType>
  <xsd:element name="datastoreItem" type="CT_DatastoreItem"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-customXmlSchemaProperties.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/schemaLibrary/2006/main"
  targetNamespace="http://schemas.openxmlformats.org/schemaLibrary/2006/main"
  attributeFormDefault="qualified" elementFormDefault="qualified">
  <xsd:complexType name="CT_Schema">
    <xsd:attribute name="uri" type="xsd:string" default=""/>
    <xsd:attribute name="manifestLocation" type="xsd:string"/>
    <xsd:attribute name="schemaLocation" type="xsd:string"/>
    <xsd:attribute name="schemaLanguage" type="xsd:token"/>
  </xsd:complexType>
  <xsd:complexType name="CT_SchemaLibrary">
    <xsd:sequence>
      <xsd:element name="schema" type="CT_Schema" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:element name="schemaLibrary" type="CT_SchemaLibrary"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-documentPropertiesCustom.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/custom-properties"
  xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"
  xmlns:s="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/custom-properties"
  blockDefault="#all" elementFormDefault="qualified">
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"
    schemaLocation="shared-documentPropertiesVariantTypes.xsd"/>
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
    schemaLocation="shared-commonSimpleTypes.xsd"/>
  <xsd:element name="Properties" type="CT_Properties"/>
  <xsd:complexType name="CT_Properties">
    <xsd:sequence>
      <xsd:element name="property" minOccurs="0" maxOccurs="unbounded" type="CT_Property"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Property">
    <xsd:choice minOccurs="1" maxOccurs="1">
      <xsd:element ref="vt:vector"/>
      <xsd:element ref="vt:array"/>
      <xsd:element ref="vt:blob"/>
      <xsd:element ref="vt:oblob"/>
      <xsd:element ref="vt:empty"/>
      <xsd:element ref="vt:null"/>
      <xsd:element ref="vt:i1"/>
      <xsd:element ref="vt:i2"/>
      <xsd:element ref="vt:i4"/>
      <xsd:element ref="vt:i8"/>
      <xsd:element ref="vt:int"/>
      <xsd:element ref="vt:ui1"/>
      <xsd:element ref="vt:ui2"/>
      <xsd:element ref="vt:ui4"/>
      <xsd:element ref="vt:ui8"/>
      <xsd:element ref="vt:uint"/>
      <xsd:element ref="vt:r4"/>
      <xsd:element ref="vt:r8"/>
      <xsd:element ref="vt:decimal"/>
      <xsd:element ref="vt:lpstr"/>
      <xsd:element ref="vt:lpwstr"/>
      <xsd:element ref="vt:bstr"/>
      <xsd:element ref="vt:date"/>
      <xsd:element ref="vt:filetime"/>
      <xsd:element ref="vt:bool"/>
      <xsd:element ref="vt:cy"/>
      <xsd:element ref="vt:error"/>
      <xsd:element ref="vt:stream"/>
      <xsd:element ref="vt:ostream"/>
      <xsd:element ref="vt:storage"/>
      <xsd:element ref="vt:ostorage"/>
      <xsd:element ref="vt:vstream"/>
      <xsd:element ref="vt:clsid"/>
    </xsd:choice>
    <xsd:attribute name="fmtid" use="required" type="s:ST_Guid"/>
    <xsd:attribute name="pid" use="required" type="xsd:int"/>
    <xsd:attribute name="name" use="optional" type="xsd:string"/>
    <xsd:attribute name="linkTarget" use="optional" type="xsd:string"/>
  </xsd:complexType>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-documentPropertiesExtended.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"
  xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"
  elementFormDefault="qualified" blockDefault="#all">
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"
    schemaLocation="shared-documentPropertiesVariantTypes.xsd"/>
  <xsd:element name="Properties" type="CT_Properties"/>
  <xsd:complexType name="CT_Properties">
    <xsd:all>
      <xsd:element name="Template" minOccurs="0" maxOccurs="1" type="xsd:string"/>
      <xsd:element name="Manager" minOccurs="0" maxOccurs="1" type="xsd:string"/>
      <xsd:element name="Company" minOccurs="0" maxOccurs="1" type="xsd:string"/>
      <xsd:element name="Pages" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="Words" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="Characters" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="PresentationFormat" minOccurs="0" maxOccurs="1" type="xsd:string"/>
      <xsd:element name="Lines" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="Paragraphs" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="Slides" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="Notes" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="TotalTime" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="HiddenSlides" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="MMClips" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="ScaleCrop" minOccurs="0" maxOccurs="1" type="xsd:boolean"/>
      <xsd:element name="HeadingPairs" minOccurs="0" maxOccurs="1" type="CT_VectorVariant"/>
      <xsd:element name="TitlesOfParts" minOccurs="0" maxOccurs="1" type="CT_VectorLpstr"/>
      <xsd:element name="LinksUpToDate" minOccurs="0" maxOccurs="1" type="xsd:boolean"/>
      <xsd:element name="CharactersWithSpaces" minOccurs="0" maxOccurs="1" type="xsd:int"/>
      <xsd:element name="SharedDoc" minOccurs="0" maxOccurs="1" type="xsd:boolean"/>
      <xsd:element name="HyperlinkBase" minOccurs="0" maxOccurs="1" type="xsd:string"/>
      <xsd:element name="HLinks" minOccurs="0" maxOccurs="1" type="CT_VectorVariant"/>
      <xsd:element name="HyperlinksChanged" minOccurs="0" maxOccurs="1" type="xsd:boolean"/>
      <xsd:element name="DigSig" minOccurs="0" maxOccurs="1" type="CT_DigSigBlob"/>
      <xsd:element name="Application" minOccurs="0" maxOccurs="1" type="xsd:string"/>
      <xsd:element name="AppVersion" minOccurs="0" maxOccurs="1" type="xsd:string"/>
      <xsd:element name="DocSecurity" minOccurs="0" maxOccurs="1" type="xsd:int"/>
    </xsd:all>
  </xsd:complexType>
  <xsd:complexType name="CT_VectorVariant">
    <xsd:sequence minOccurs="1" maxOccurs="1">
      <xsd:element ref="vt:vector"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_VectorLpstr">
    <xsd:sequence minOccurs="1" maxOccurs="1">
      <xsd:element ref="vt:vector"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_DigSigBlob">
    <xsd:sequence minOccurs="1" maxOccurs="1">
      <xsd:element ref="vt:blob"/>
    </xsd:sequence>
  </xsd:complexType>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-documentPropertiesVariantTypes.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"
  xmlns:s="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"
  blockDefault="#all" elementFormDefault="qualified">
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
    schemaLocation="shared-commonSimpleTypes.xsd"/>
  <xsd:simpleType name="ST_VectorBaseType">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="variant"/>
      <xsd:enumeration value="i1"/>
      <xsd:enumeration value="i2"/>
      <xsd:enumeration value="i4"/>
      <xsd:enumeration value="i8"/>
      <xsd:enumeration value="ui1"/>
      <xsd:enumeration value="ui2"/>
      <xsd:enumeration value="ui4"/>
      <xsd:enumeration value="ui8"/>
      <xsd:enumeration value="r4"/>
      <xsd:enumeration value="r8"/>
      <xsd:enumeration value="lpstr"/>
      <xsd:enumeration value="lpwstr"/>
      <xsd:enumeration value="bstr"/>
      <xsd:enumeration value="date"/>
      <xsd:enumeration value="filetime"/>
      <xsd:enumeration value="bool"/>
      <xsd:enumeration value="cy"/>
      <xsd:enumeration value="error"/>
      <xsd:enumeration value="clsid"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_ArrayBaseType">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="variant"/>
      <xsd:enumeration value="i1"/>
      <xsd:enumeration value="i2"/>
      <xsd:enumeration value="i4"/>
      <xsd:enumeration value="int"/>
      <xsd:enumeration value="ui1"/>
      <xsd:enumeration value="ui2"/>
      <xsd:enumeration value="ui4"/>
      <xsd:enumeration value="uint"/>
      <xsd:enumeration value="r4"/>
      <xsd:enumeration value="r8"/>
      <xsd:enumeration value="decimal"/>
      <xsd:enumeration value="bstr"/>
      <xsd:enumeration value="date"/>
      <xsd:enumeration value="bool"/>
      <xsd:enumeration value="cy"/>
      <xsd:enumeration value="error"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_Cy">
    <xsd:restriction base="xsd:string">
      <xsd:pattern value="\s*[0-9]*\.[0-9]{4}\s*"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:simpleType name="ST_Error">
    <xsd:restriction base="xsd:string">
      <xsd:pattern value="\s*0x[0-9A-Za-z]{8}\s*"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Empty"/>
  <xsd:complexType name="CT_Null"/>
  <xsd:complexType name="CT_Vector">
    <xsd:choice minOccurs="1" maxOccurs="unbounded">
      <xsd:element ref="variant"/>
      <xsd:element ref="i1"/>
      <xsd:element ref="i2"/>
      <xsd:element ref="i4"/>
      <xsd:element ref="i8"/>
      <xsd:element ref="ui1"/>
      <xsd:element ref="ui2"/>
      <xsd:element ref="ui4"/>
      <xsd:element ref="ui8"/>
      <xsd:element ref="r4"/>
      <xsd:element ref="r8"/>
      <xsd:element ref="lpstr"/>
      <xsd:element ref="lpwstr"/>
      <xsd:element ref="bstr"/>
      <xsd:element ref="date"/>
      <xsd:element ref="filetime"/>
      <xsd:element ref="bool"/>
      <xsd:element ref="cy"/>
      <xsd:element ref="error"/>
      <xsd:element ref="clsid"/>
    </xsd:choice>
    <xsd:attribute name="baseType" type="ST_VectorBaseType" use="required"/>
    <xsd:attribute name="size" type="xsd:unsignedInt" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_Array">
    <xsd:choice minOccurs="1" maxOccurs="unbounded">
      <xsd:element ref="variant"/>
      <xsd:element ref="i1"/>
      <xsd:element ref="i2"/>
      <xsd:element ref="i4"/>
      <xsd:element ref="int"/>
      <xsd:element ref="ui1"/>
      <xsd:element ref="ui2"/>
      <xsd:element ref="ui4"/>
      <xsd:element ref="uint"/>
      <xsd:element ref="r4"/>
      <xsd:element ref="r8"/>
      <xsd:element ref="decimal"/>
      <xsd:element ref="bstr"/>
      <xsd:element ref="date"/>
      <xsd:element ref="bool"/>
      <xsd:element ref="error"/>
      <xsd:element ref="cy"/>
    </xsd:choice>
    <xsd:attribute name="lBounds" type="xsd:int" use="required"/>
    <xsd:attribute name="uBounds" type="xsd:int" use="required"/>
    <xsd:attribute name="baseType" type="ST_ArrayBaseType" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_Variant">
    <xsd:choice minOccurs="1" maxOccurs="1">
      <xsd:element ref="variant"/>
      <xsd:element ref="vector"/>
      <xsd:element ref="array"/>
      <xsd:element ref="blob"/>
      <xsd:element ref="oblob"/>
      <xsd:element ref="empty"/>
      <xsd:element ref="null"/>
      <xsd:element ref="i1"/>
      <xsd:element ref="i2"/>
      <xsd:element ref="i4"/>
      <xsd:element ref="i8"/>
      <xsd:element ref="int"/>
      <xsd:element ref="ui1"/>
      <xsd:element ref="ui2"/>
      <xsd:element ref="ui4"/>
      <xsd:element ref="ui8"/>
      <xsd:element ref="uint"/>
      <xsd:element ref="r4"/>
      <xsd:element ref="r8"/>
      <xsd:element ref="decimal"/>
      <xsd:element ref="lpstr"/>
      <xsd:element ref="lpwstr"/>
      <xsd:element ref="bstr"/>
      <xsd:element ref="date"/>
      <xsd:element ref="filetime"/>
      <xsd:element ref="bool"/>
      <xsd:element ref="cy"/>
      <xsd:element ref="error"/>
      <xsd:element ref="stream"/>
      <xsd:element ref="ostream"/>
      <xsd:element ref="storage"/>
      <xsd:element ref="ostorage"/>
      <xsd:element ref="vstream"/>
      <xsd:element ref="clsid"/>
    </xsd:choice>
  </xsd:complexType>
  <xsd:complexType name="CT_Vstream">
    <xsd:simpleContent>
      <xsd:extension base="xsd:base64Binary">
        <xsd:attribute name="version" type="s:ST_Guid"/>
      </xsd:extension>
    </xsd:simpleContent>
  </xsd:complexType>
  <xsd:element name="variant" type="CT_Variant"/>
  <xsd:element name="vector" type="CT_Vector"/>
  <xsd:element name="array" type="CT_Array"/>
  <xsd:element name="blob" type="xsd:base64Binary"/>
  <xsd:element name="oblob" type="xsd:base64Binary"/>
  <xsd:element name="empty" type="CT_Empty"/>
  <xsd:element name="null" type="CT_Null"/>
  <xsd:element name="i1" type="xsd:byte"/>
  <xsd:element name="i2" type="xsd:short"/>
  <xsd:element name="i4" type="xsd:int"/>
  <xsd:element name="i8" type="xsd:long"/>
  <xsd:element name="int" type="xsd:int"/>
  <xsd:element name="ui1" type="xsd:unsignedByte"/>
  <xsd:element name="ui2" type="xsd:unsignedShort"/>
  <xsd:element name="ui4" type="xsd:unsignedInt"/>
  <xsd:element name="ui8" type="xsd:unsignedLong"/>
  <xsd:element name="uint" type="xsd:unsignedInt"/>
  <xsd:element name="r4" type="xsd:float"/>
  <xsd:element name="r8" type="xsd:double"/>
  <xsd:element name="decimal" type="xsd:decimal"/>
  <xsd:element name="lpstr" type="xsd:string"/>
  <xsd:element name="lpwstr" type="xsd:string"/>
  <xsd:element name="bstr" type="xsd:string"/>
  <xsd:element name="date" type="xsd:dateTime"/>
  <xsd:element name="filetime" type="xsd:dateTime"/>
  <xsd:element name="bool" type="xsd:boolean"/>
  <xsd:element name="cy" type="ST_Cy"/>
  <xsd:element name="error" type="ST_Error"/>
  <xsd:element name="stream" type="xsd:base64Binary"/>
  <xsd:element name="ostream" type="xsd:base64Binary"/>
  <xsd:element name="storage" type="xsd:base64Binary"/>
  <xsd:element name="ostorage" type="xsd:base64Binary"/>
  <xsd:element name="vstream" type="CT_Vstream"/>
  <xsd:element name="clsid" type="s:ST_Guid"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-math.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/math"
  xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math"
  xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
  xmlns:s="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  elementFormDefault="qualified" attributeFormDefault="qualified" blockDefault="#all"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/math">
  <xsd:import namespace="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
    schemaLocation="wml.xsd"/>
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
    schemaLocation="shared-commonSimpleTypes.xsd"/>
  <xsd:import namespace="http://www.w3.org/XML/1998/namespace" schemaLocation="xml.xsd"/>
  <xsd:simpleType name="ST_Integer255">
    <xsd:restriction base="xsd:integer">
      <xsd:minInclusive value="1"/>
      <xsd:maxInclusive value="255"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Integer255">
    <xsd:attribute name="val" type="ST_Integer255" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_Integer2">
    <xsd:restriction base="xsd:integer">
      <xsd:minInclusive value="-2"/>
      <xsd:maxInclusive value="2"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Integer2">
    <xsd:attribute name="val" type="ST_Integer2" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_SpacingRule">
    <xsd:restriction base="xsd:integer">
      <xsd:minInclusive value="0"/>
      <xsd:maxInclusive value="4"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_SpacingRule">
    <xsd:attribute name="val" type="ST_SpacingRule" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_UnSignedInteger">
    <xsd:restriction base="xsd:unsignedInt"/>
  </xsd:simpleType>
  <xsd:complexType name="CT_UnSignedInteger">
    <xsd:attribute name="val" type="ST_UnSignedInteger" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_Char">
    <xsd:restriction base="xsd:string">
      <xsd:maxLength value="1"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Char">
    <xsd:attribute name="val" type="ST_Char" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_OnOff">
    <xsd:attribute name="val" type="s:ST_OnOff"/>
  </xsd:complexType>
  <xsd:complexType name="CT_String">
    <xsd:attribute name="val" type="s:ST_String"/>
  </xsd:complexType>
  <xsd:complexType name="CT_XAlign">
    <xsd:attribute name="val" type="s:ST_XAlign" use="required"/>
  </xsd:complexType>
  <xsd:complexType name="CT_YAlign">
    <xsd:attribute name="val" type="s:ST_YAlign" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_Shp">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="centered"/>
      <xsd:enumeration value="match"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Shp">
    <xsd:attribute name="val" type="ST_Shp" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_FType">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="bar"/>
      <xsd:enumeration value="skw"/>
      <xsd:enumeration value="lin"/>
      <xsd:enumeration value="noBar"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_FType">
    <xsd:attribute name="val" type="ST_FType" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_LimLoc">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="undOvr"/>
      <xsd:enumeration value="subSup"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_LimLoc">
    <xsd:attribute name="val" type="ST_LimLoc" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_TopBot">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="top"/>
      <xsd:enumeration value="bot"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_TopBot">
    <xsd:attribute name="val" type="ST_TopBot" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_Script">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="roman"/>
      <xsd:enumeration value="script"/>
      <xsd:enumeration value="fraktur"/>
      <xsd:enumeration value="double-struck"/>
      <xsd:enumeration value="sans-serif"/>
      <xsd:enumeration value="monospace"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Script">
    <xsd:attribute name="val" type="ST_Script"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_Style">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="p"/>
      <xsd:enumeration value="b"/>
      <xsd:enumeration value="i"/>
      <xsd:enumeration value="bi"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_Style">
    <xsd:attribute name="val" type="ST_Style"/>
  </xsd:complexType>
  <xsd:complexType name="CT_ManualBreak">
    <xsd:attribute name="alnAt" type="ST_Integer255"/>
  </xsd:complexType>
  <xsd:group name="EG_ScriptStyle">
    <xsd:sequence>
      <xsd:element name="scr" minOccurs="0" type="CT_Script"/>
      <xsd:element name="sty" minOccurs="0" type="CT_Style"/>
    </xsd:sequence>
  </xsd:group>
  <xsd:complexType name="CT_RPR">
    <xsd:sequence>
      <xsd:element name="lit" minOccurs="0" type="CT_OnOff"/>
      <xsd:choice>
        <xsd:element name="nor" minOccurs="0" type="CT_OnOff"/>
        <xsd:sequence>
          <xsd:group ref="EG_ScriptStyle"/>
        </xsd:sequence>
      </xsd:choice>
      <xsd:element name="brk" minOccurs="0" type="CT_ManualBreak"/>
      <xsd:element name="aln" minOccurs="0" type="CT_OnOff"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Text">
    <xsd:simpleContent>
      <xsd:extension base="s:ST_String">
        <xsd:attribute ref="xml:space" use="optional"/>
      </xsd:extension>
    </xsd:simpleContent>
  </xsd:complexType>
  <xsd:complexType name="CT_R">
    <xsd:sequence>
      <xsd:element name="rPr" type="CT_RPR" minOccurs="0"/>
      <xsd:group ref="w:EG_RPr" minOccurs="0"/>
      <xsd:choice minOccurs="0" maxOccurs="unbounded">
        <xsd:group ref="w:EG_RunInnerContent"/>
        <xsd:element name="t" type="CT_Text" minOccurs="0"/>
      </xsd:choice>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_CtrlPr">
    <xsd:sequence>
      <xsd:group ref="w:EG_RPrMath" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_AccPr">
    <xsd:sequence>
      <xsd:element name="chr" type="CT_Char" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Acc">
    <xsd:sequence>
      <xsd:element name="accPr" type="CT_AccPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_BarPr">
    <xsd:sequence>
      <xsd:element name="pos" type="CT_TopBot" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Bar">
    <xsd:sequence>
      <xsd:element name="barPr" type="CT_BarPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_BoxPr">
    <xsd:sequence>
      <xsd:element name="opEmu" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="noBreak" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="diff" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="brk" type="CT_ManualBreak" minOccurs="0"/>
      <xsd:element name="aln" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Box">
    <xsd:sequence>
      <xsd:element name="boxPr" type="CT_BoxPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_BorderBoxPr">
    <xsd:sequence>
      <xsd:element name="hideTop" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="hideBot" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="hideLeft" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="hideRight" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="strikeH" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="strikeV" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="strikeBLTR" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="strikeTLBR" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_BorderBox">
    <xsd:sequence>
      <xsd:element name="borderBoxPr" type="CT_BorderBoxPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_DPr">
    <xsd:sequence>
      <xsd:element name="begChr" type="CT_Char" minOccurs="0"/>
      <xsd:element name="sepChr" type="CT_Char" minOccurs="0"/>
      <xsd:element name="endChr" type="CT_Char" minOccurs="0"/>
      <xsd:element name="grow" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="shp" type="CT_Shp" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_D">
    <xsd:sequence>
      <xsd:element name="dPr" type="CT_DPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_EqArrPr">
    <xsd:sequence>
      <xsd:element name="baseJc" type="CT_YAlign" minOccurs="0"/>
      <xsd:element name="maxDist" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="objDist" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="rSpRule" type="CT_SpacingRule" minOccurs="0"/>
      <xsd:element name="rSp" type="CT_UnSignedInteger" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_EqArr">
    <xsd:sequence>
      <xsd:element name="eqArrPr" type="CT_EqArrPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_FPr">
    <xsd:sequence>
      <xsd:element name="type" type="CT_FType" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_F">
    <xsd:sequence>
      <xsd:element name="fPr" type="CT_FPr" minOccurs="0"/>
      <xsd:element name="num" type="CT_OMathArg"/>
      <xsd:element name="den" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_FuncPr">
    <xsd:sequence>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Func">
    <xsd:sequence>
      <xsd:element name="funcPr" type="CT_FuncPr" minOccurs="0"/>
      <xsd:element name="fName" type="CT_OMathArg"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_GroupChrPr">
    <xsd:sequence>
      <xsd:element name="chr" type="CT_Char" minOccurs="0"/>
      <xsd:element name="pos" type="CT_TopBot" minOccurs="0"/>
      <xsd:element name="vertJc" type="CT_TopBot" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_GroupChr">
    <xsd:sequence>
      <xsd:element name="groupChrPr" type="CT_GroupChrPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_LimLowPr">
    <xsd:sequence>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_LimLow">
    <xsd:sequence>
      <xsd:element name="limLowPr" type="CT_LimLowPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
      <xsd:element name="lim" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_LimUppPr">
    <xsd:sequence>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_LimUpp">
    <xsd:sequence>
      <xsd:element name="limUppPr" type="CT_LimUppPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
      <xsd:element name="lim" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_MCPr">
    <xsd:sequence>
      <xsd:element name="count" type="CT_Integer255" minOccurs="0"/>
      <xsd:element name="mcJc" type="CT_XAlign" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_MC">
    <xsd:sequence>
      <xsd:element name="mcPr" type="CT_MCPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_MCS">
    <xsd:sequence>
      <xsd:element name="mc" type="CT_MC" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_MPr">
    <xsd:sequence>
      <xsd:element name="baseJc" type="CT_YAlign" minOccurs="0"/>
      <xsd:element name="plcHide" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="rSpRule" type="CT_SpacingRule" minOccurs="0"/>
      <xsd:element name="cGpRule" type="CT_SpacingRule" minOccurs="0"/>
      <xsd:element name="rSp" type="CT_UnSignedInteger" minOccurs="0"/>
      <xsd:element name="cSp" type="CT_UnSignedInteger" minOccurs="0"/>
      <xsd:element name="cGp" type="CT_UnSignedInteger" minOccurs="0"/>
      <xsd:element name="mcs" type="CT_MCS" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_MR">
    <xsd:sequence>
      <xsd:element name="e" type="CT_OMathArg" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_M">
    <xsd:sequence>
      <xsd:element name="mPr" type="CT_MPr" minOccurs="0"/>
      <xsd:element name="mr" type="CT_MR" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_NaryPr">
    <xsd:sequence>
      <xsd:element name="chr" type="CT_Char" minOccurs="0"/>
      <xsd:element name="limLoc" type="CT_LimLoc" minOccurs="0"/>
      <xsd:element name="grow" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="subHide" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="supHide" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Nary">
    <xsd:sequence>
      <xsd:element name="naryPr" type="CT_NaryPr" minOccurs="0"/>
      <xsd:element name="sub" type="CT_OMathArg"/>
      <xsd:element name="sup" type="CT_OMathArg"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_PhantPr">
    <xsd:sequence>
      <xsd:element name="show" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="zeroWid" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="zeroAsc" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="zeroDesc" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="transp" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Phant">
    <xsd:sequence>
      <xsd:element name="phantPr" type="CT_PhantPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_RadPr">
    <xsd:sequence>
      <xsd:element name="degHide" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_Rad">
    <xsd:sequence>
      <xsd:element name="radPr" type="CT_RadPr" minOccurs="0"/>
      <xsd:element name="deg" type="CT_OMathArg"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SPrePr">
    <xsd:sequence>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SPre">
    <xsd:sequence>
      <xsd:element name="sPrePr" type="CT_SPrePr" minOccurs="0"/>
      <xsd:element name="sub" type="CT_OMathArg"/>
      <xsd:element name="sup" type="CT_OMathArg"/>
      <xsd:element name="e" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SSubPr">
    <xsd:sequence>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SSub">
    <xsd:sequence>
      <xsd:element name="sSubPr" type="CT_SSubPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
      <xsd:element name="sub" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SSubSupPr">
    <xsd:sequence>
      <xsd:element name="alnScr" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SSubSup">
    <xsd:sequence>
      <xsd:element name="sSubSupPr" type="CT_SSubSupPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
      <xsd:element name="sub" type="CT_OMathArg"/>
      <xsd:element name="sup" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SSupPr">
    <xsd:sequence>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_SSup">
    <xsd:sequence>
      <xsd:element name="sSupPr" type="CT_SSupPr" minOccurs="0"/>
      <xsd:element name="e" type="CT_OMathArg"/>
      <xsd:element name="sup" type="CT_OMathArg"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:group name="EG_OMathMathElements">
    <xsd:choice>
      <xsd:element name="acc" type="CT_Acc"/>
      <xsd:element name="bar" type="CT_Bar"/>
      <xsd:element name="box" type="CT_Box"/>
      <xsd:element name="borderBox" type="CT_BorderBox"/>
      <xsd:element name="d" type="CT_D"/>
      <xsd:element name="eqArr" type="CT_EqArr"/>
      <xsd:element name="f" type="CT_F"/>
      <xsd:element name="func" type="CT_Func"/>
      <xsd:element name="groupChr" type="CT_GroupChr"/>
      <xsd:element name="limLow" type="CT_LimLow"/>
      <xsd:element name="limUpp" type="CT_LimUpp"/>
      <xsd:element name="m" type="CT_M"/>
      <xsd:element name="nary" type="CT_Nary"/>
      <xsd:element name="phant" type="CT_Phant"/>
      <xsd:element name="rad" type="CT_Rad"/>
      <xsd:element name="sPre" type="CT_SPre"/>
      <xsd:element name="sSub" type="CT_SSub"/>
      <xsd:element name="sSubSup" type="CT_SSubSup"/>
      <xsd:element name="sSup" type="CT_SSup"/>
      <xsd:element name="r" type="CT_R"/>
    </xsd:choice>
  </xsd:group>
  <xsd:group name="EG_OMathElements">
    <xsd:choice>
      <xsd:group ref="EG_OMathMathElements"/>
      <xsd:group ref="w:EG_PContentMath"/>
    </xsd:choice>
  </xsd:group>
  <xsd:complexType name="CT_OMathArgPr">
    <xsd:sequence>
      <xsd:element name="argSz" type="CT_Integer2" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_OMathArg">
    <xsd:sequence>
      <xsd:element name="argPr" type="CT_OMathArgPr" minOccurs="0"/>
      <xsd:group ref="EG_OMathElements" minOccurs="0" maxOccurs="unbounded"/>
      <xsd:element name="ctrlPr" type="CT_CtrlPr" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:simpleType name="ST_Jc">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="left"/>
      <xsd:enumeration value="right"/>
      <xsd:enumeration value="center"/>
      <xsd:enumeration value="centerGroup"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_OMathJc">
    <xsd:attribute name="val" type="ST_Jc"/>
  </xsd:complexType>
  <xsd:complexType name="CT_OMathParaPr">
    <xsd:sequence>
      <xsd:element name="jc" type="CT_OMathJc" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_TwipsMeasure">
    <xsd:attribute name="val" type="s:ST_TwipsMeasure" use="required"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_BreakBin">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="before"/>
      <xsd:enumeration value="after"/>
      <xsd:enumeration value="repeat"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_BreakBin">
    <xsd:attribute name="val" type="ST_BreakBin"/>
  </xsd:complexType>
  <xsd:simpleType name="ST_BreakBinSub">
    <xsd:restriction base="xsd:string">
      <xsd:enumeration value="--"/>
      <xsd:enumeration value="-+"/>
      <xsd:enumeration value="+-"/>
    </xsd:restriction>
  </xsd:simpleType>
  <xsd:complexType name="CT_BreakBinSub">
    <xsd:attribute name="val" type="ST_BreakBinSub"/>
  </xsd:complexType>
  <xsd:complexType name="CT_MathPr">
    <xsd:sequence>
      <xsd:element name="mathFont" type="CT_String" minOccurs="0"/>
      <xsd:element name="brkBin" type="CT_BreakBin" minOccurs="0"/>
      <xsd:element name="brkBinSub" type="CT_BreakBinSub" minOccurs="0"/>
      <xsd:element name="smallFrac" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="dispDef" type="CT_OnOff" minOccurs="0"/>
      <xsd:element name="lMargin" type="CT_TwipsMeasure" minOccurs="0"/>
      <xsd:element name="rMargin" type="CT_TwipsMeasure" minOccurs="0"/>
      <xsd:element name="defJc" type="CT_OMathJc" minOccurs="0"/>
      <xsd:element name="preSp" type="CT_TwipsMeasure" minOccurs="0"/>
      <xsd:element name="postSp" type="CT_TwipsMeasure" minOccurs="0"/>
      <xsd:element name="interSp" type="CT_TwipsMeasure" minOccurs="0"/>
      <xsd:element name="intraSp" type="CT_TwipsMeasure" minOccurs="0"/>
      <xsd:choice minOccurs="0">
        <xsd:element name="wrapIndent" type="CT_TwipsMeasure"/>
        <xsd:element name="wrapRight" type="CT_OnOff"/>
      </xsd:choice>
      <xsd:element name="intLim" type="CT_LimLoc" minOccurs="0"/>
      <xsd:element name="naryLim" type="CT_LimLoc" minOccurs="0"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:element name="mathPr" type="CT_MathPr"/>
  <xsd:complexType name="CT_OMathPara">
    <xsd:sequence>
      <xsd:element name="oMathParaPr" type="CT_OMathParaPr" minOccurs="0"/>
      <xsd:element name="oMath" type="CT_OMath" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:complexType name="CT_OMath">
    <xsd:sequence>
      <xsd:group ref="EG_OMathElements" minOccurs="0" maxOccurs="unbounded"/>
    </xsd:sequence>
  </xsd:complexType>
  <xsd:element name="oMathPara" type="CT_OMathPara"/>
  <xsd:element name="oMath" type="CT_OMath"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/shared-relationshipReference.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  elementFormDefault="qualified"
  targetNamespace="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  blockDefault="#all">
  <xsd:simpleType name="ST_RelationshipId">
    <xsd:restriction base="xsd:string"/>
  </xsd:simpleType>
  <xsd:attribute name="id" type="ST_RelationshipId"/>
  <xsd:attribute name="embed" type="ST_RelationshipId"/>
  <xsd:attribute name="link" type="ST_RelationshipId"/>
  <xsd:attribute name="dm" type="ST_RelationshipId" default=""/>
  <xsd:attribute name="lo" type="ST_RelationshipId" default=""/>
  <xsd:attribute name="qs" type="ST_RelationshipId" default=""/>
  <xsd:attribute name="cs" type="ST_RelationshipId" default=""/>
  <xsd:attribute name="blip" type="ST_RelationshipId" default=""/>
  <xsd:attribute name="pict" type="ST_RelationshipId"/>
  <xsd:attribute name="href" type="ST_RelationshipId"/>
  <xsd:attribute name="topLeft" type="ST_RelationshipId"/>
  <xsd:attribute name="topRight" type="ST_RelationshipId"/>
  <xsd:attribute name="bottomLeft" type="ST_RelationshipId"/>
  <xsd:attribute name="bottomRight" type="ST_RelationshipId"/>
</xsd:schema>



================================================
FILE: workspaces/course-deck-production/skills/pptx/ooxml/schemas/ISO-IEC29500-4_2016/vml-main.xsd
================================================
<?xml version="1.0" encoding="utf-8"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns="urn:schemas-microsoft-com:vml"
  xmlns:pvml="urn:schemas-microsoft-com:office:powerpoint"
  xmlns:o="urn:schemas-microsoft-com:office:office"
  xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
  xmlns:w10="urn:schemas-microsoft-com:office:word"
  xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
  xmlns:x="urn:schemas-microsoft-com:office:excel"
  xmlns:s="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
  targetNamespace="urn:schemas-microsoft-com:vml" elementFormDefault="qualified"
  attributeFormDefault="unqualified">
  <xsd:import namespace="urn:schemas-microsoft-com:office:office"
    schemaLocation="vml-officeDrawing.xsd"/>
  <xsd:import namespace="http://schemas.openxmlformats.org/wordprocessingml/2006/main"
    schemaLocation="wml.xsd"/>
  <xsd:import namespace="urn:schemas-microsoft-com:office:word"
    schemaLocation="vml-wordprocessingDrawing.xsd"/>
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/relationships"
    schemaLocation="shared-relationshipReference.xsd"/>
  <xsd:import namespace="urn:schemas-microsoft-com:office:excel"
    schemaLocation="vml-spreadsheetDrawing.xsd"/>
  <xsd:import namespace="urn:schemas-microsoft-com:office:powerpoint"
    schemaLocation="vml-presentationDrawing.xsd"/>
  <xsd:import namespace="http://schemas.openxmlformats.org/officeDocument/2006/sharedTypes"
    schemaLocation="shared-commonSimpleTypes.xsd"/>
  <xsd:attributeGroup name="AG_Id">
    <xsd:attribute name="id" type="xsd:string" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_Style">
    <xsd:attribute name="style" type="xsd:string" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_Type">
    <xsd:attribute name="type" type="xsd:string" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_Adj">
    <xsd:attribute name="adj" type="xsd:string" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_Path">
    <xsd:attribute name="path" type="xsd:string" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_Fill">
    <xsd:attribute name="filled" type="s:ST_TrueFalse" use="optional"/>
    <xsd:attribute name="fillcolor" type="s:ST_ColorType" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_Chromakey">
    <xsd:attribute name="chromakey" type="s:ST_ColorType" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_Ext">
    <xsd:attribute name="ext" form="qualified" type="ST_Ext"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_CoreAttributes">
    <xsd:attributeGroup ref="AG_Id"/>
    <xsd:attributeGroup ref="AG_Style"/>
    <xsd:attribute name="href" type="xsd:string" use="optional"/>
    <xsd:attribute name="target" type="xsd:string" use="optional"/>
    <xsd:attribute name="class" type="xsd:string" use="optional"/>
    <xsd:attribute name="title" type="xsd:string" use="optional"/>
    <xsd:attribute name="alt" type="xsd:string" use="optional"/>
    <xsd:attribute name="coordsize" type="xsd:string" use="optional"/>
    <xsd:attribute name="coordorigin" type="xsd:string" use="optional"/>
    <xsd:attribute name="wrapcoords" type="xsd:string" use="optional"/>
    <xsd:attribute name="print" type="s:ST_TrueFalse" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_ShapeAttributes">
    <xsd:attributeGroup ref="AG_Chromakey"/>
    <xsd:attributeGroup ref="AG_Fill"/>
    <xsd:attribute name="opacity" type="xsd:string" use="optional"/>
    <xsd:attribute name="stroked" type="s:ST_TrueFalse" use="optional"/>
    <xsd:attribute name="strokecolor" type="s:ST_ColorType" use="optional"/>
    <xsd:attribute name="strokeweight" type="xsd:string" use="optional"/>
    <xsd:attribute name="insetpen" type="s:ST_TrueFalse" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_OfficeCoreAttributes">
    <xsd:attribute ref="o:spid"/>
    <xsd:attribute ref="o:oned"/>
    <xsd:attribute ref="o:regroupid"/>
    <xsd:attribute ref="o:doubleclicknotify"/>
    <xsd:attribute ref="o:button"/>
    <xsd:attribute ref="o:userhidden"/>
    <xsd:attribute ref="o:bullet"/>
    <xsd:attribute ref="o:hr"/>
    <xsd:attribute ref="o:hrstd"/>
    <xsd:attribute ref="o:hrnoshade"/>
    <xsd:attribute ref="o:hrpct"/>
    <xsd:attribute ref="o:hralign"/>
    <xsd:attribute ref="o:allowincell"/>
    <xsd:attribute ref="o:allowoverlap"/>
    <xsd:attribute ref="o:userdrawn"/>
    <xsd:attribute ref="o:bordertopcolor"/>
    <xsd:attribute ref="o:borderleftcolor"/>
    <xsd:attribute ref="o:borderbottomcolor"/>
    <xsd:attribute ref="o:borderrightcolor"/>
    <xsd:attribute ref="o:dgmlayout"/>
    <xsd:attribute ref="o:dgmnodekind"/>
    <xsd:attribute ref="o:dgmlayoutmru"/>
    <xsd:attribute ref="o:insetmode"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_OfficeShapeAttributes">
    <xsd:attribute ref="o:spt"/>
    <xsd:attribute ref="o:connectortype"/>
    <xsd:attribute ref="o:bwmode"/>
    <xsd:attribute ref="o:bwpure"/>
    <xsd:attribute ref="o:bwnormal"/>
    <xsd:attribute ref="o:forcedash"/>
    <xsd:attribute ref="o:oleicon"/>
    <xsd:attribute ref="o:ole"/>
    <xsd:attribute ref="o:preferrelative"/>
    <xsd:attribute ref="o:cliptowrap"/>
    <xsd:attribute ref="o:clip"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_AllCoreAttributes">
    <xsd:attributeGroup ref="AG_CoreAttributes"/>
    <xsd:attributeGroup ref="AG_OfficeCoreAttributes"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_AllShapeAttributes">
    <xsd:attributeGroup ref="AG_ShapeAttributes"/>
    <xsd:attributeGroup ref="AG_OfficeShapeAttributes"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_ImageAttributes">
    <xsd:attribute name="src" type="xsd:string" use="optional"/>
    <xsd:attribute name="cropleft" type="xsd:string" use="optional"/>
    <xsd:attribute name="croptop" type="xsd:string" use="optional"/>
    <xsd:attribute name="cropright" type="xsd:string" use="optional"/>
    <xsd:attribute name="cropbottom" type="xsd:string" use="optional"/>
    <xsd:attribute name="gain" type="xsd:string" use="optional"/>
    <xsd:attribute name="blacklevel" type="xsd:string" use="optional"/>
    <xsd:attribute name="gamma" type="xsd:string" use="optional"/>
    <xsd:attribute name="grayscale" type="s:ST_TrueFalse" use="optional"/>
    <xsd:attribute name="bilevel" type="s:ST_TrueFalse" use="optional"/>
  </xsd:attributeGroup>
  <xsd:attributeGroup name="AG_StrokeAttributes">
    <xsd:attribute name="on" type="s:ST_TrueFalse" use="optional"/>
    <xsd:attribute name="weight" type="xsd:string" use="optional"/>
    <xsd:attribute name="color" type="s:ST_ColorType" use="optional"/>
    <xsd:attribute name="opacity" type="xsd:string" use="optional"/>
    <xsd:attribute name="linestyle" type="ST_StrokeLineStyle" use="optional"/>
    <xsd:attribute name="miterlimit" type="xsd:decimal" use="optional"/>
    <xsd:attribute name="joinstyle" type="ST_StrokeJoinStyle" use="optional"/>
    <xsd:attribute name="endcap" type="ST_StrokeEndCap" use="optional"/>
    <xsd:attribute name="dashstyle" type="xsd:string" use="optional"/>
    <xsd:attribute name="filltype" type="ST_FillType" use="optional"/>
    <xsd:attribute name="src" type="xsd:string" use="optional"/>
    <xsd:attribute name="imageaspect" type="ST_ImageAspect" use="optional"/>
    <xsd:attribute name="imagesize" type="xsd:string" use="optional"/>
    <xsd:attribute name="imagealignshape" type="s:ST_TrueFalse" use="optional"/>
    <xsd:attribute name="color2" type="s:ST_ColorType" use="optional"/>
    <xsd:attribute name="startarrow" type="ST_StrokeArrowType" use="optional"/>
    <xsd:attribute name="startarrowwidth" type="ST_StrokeArrowWidth" use="optional"/>
    <xsd:attribute name="startarrowlength" type="ST_StrokeArrowLength" use="optional"/>
    <xsd:attribute name="endarrow" type="ST_StrokeArrowType" use="optional"/>
    <xsd:attribute name="endarrowwidth" type="ST_StrokeArrowWidth" use="optional"/>
    <xsd:attribute name="endarrowlength" type="ST_StrokeArrowLength" use="optional"/>
    <xsd:attribute ref="o:href"/>
    <xsd:attribute ref="o:althref"/>
    <xsd:attribute ref="o:title"/>
    <xsd:attribute ref="o:forcedash"/>
    <xsd:attribute ref="r:id" use="optional"/>
    <xsd:attribute name="insetpen" type="s:ST_TrueFalse" use="optional"/>
    <xsd:attribute ref="o:relid"/>
  </xsd:attributeGroup>
  <xsd:group name="EG_ShapeElements">
    <xsd:choice>
      <xsd:element ref="path"/>
      <xsd:element ref="formulas"/>
      <xsd:element ref="handles"/>
      <xsd:element ref="fill"/>
      <xsd:element ref="stroke"/>
      <xsd:element ref="shadow"/>
      <xsd:element ref="textbox"/>
      <xsd:element ref="textpath"/>
      <xsd:element ref="imagedata"/>
      <xsd:element ref="o:skew"/>
      <xsd:element ref="o:extrusion"/>
      <xsd:element ref="o:callout"/>
      <xsd:element ref="o:lock"/>
      <xsd:element ref="o:clippath"/>
      <xsd:element ref="o:signatureline"/>
      <xsd:element ref="w10:wrap"/>
      <xsd:element ref="w10:anchorlock"/>
      <xsd:element ref="w10:bordertop"/>
      <xsd:element ref="w10:borderbottom"/>
      <xsd:element ref="w10:borderleft"/>
      <xsd:element ref="w10:borderright"/>
      <xsd:element ref="x:ClientData" minOccurs="0"/>
      <xsd:element ref="pvml:textdata" minOccurs="0"/>
    </xsd:choice>
  </xsd:group>
  <xsd:element name="shape" type="CT_Shape"/>
  <xsd:element name="shapetype" type="CT_Shapetype"/>
  <xsd:element name="group" type="CT_Group"/>
  <xsd:element name="background" type="CT_Background"/>
  <xsd:complexType name="CT_Shape">
    <xsd:choice maxOccurs="unbounded">
      <xsd:group ref="EG_ShapeElements"/>
      <xsd:element ref="o:ink"/>
      <xsd:element ref="pvml:iscomment"/>
      <xsd:element ref="o:equationxml"/>
    </xsd:choice>
    <xsd:attributeGroup ref="AG_AllCoreAttributes"/>
    <xsd:attributeGroup ref="AG_AllShapeAttributes"/>
    <xsd:attributeGroup ref="AG_Type"/>
    <xsd:attributeGroup ref="AG_Adj"/>
    <xsd:attributeGroup ref="AG_Path"/>
    <xsd:attribute ref="o:gfxdata"/>
    <xsd:attribute name="equationxml" type="xsd:string" use="optional"/>
  </xsd:complexType>
  <xsd:complexType name="CT_Shapetype">
    <xsd:sequence>
      <xsd:group ref="EG_ShapeElements" minOccurs="0" maxOccurs="unbounded"/>
      <xsd:element ref="o:complex" minOccurs="0"/>
    </xsd:sequence>
    <xsd:attributeGroup ref="AG_AllCoreAttributes"/>
    <xsd:attributeGroup ref="AG_AllShapeAttributes"/>
    <xsd:attributeGroup ref="AG_Adj"/>
    <xsd:attributeGroup ref="AG_Path"/>
    <xsd:attribute ref="o:master"/>
  </xsd:complexType>
  <xsd:complexType name="CT_Group">
    <xsd:choice maxOccurs="unbounded">
      <xsd:group ref="EG_ShapeElements"/>
      <xsd:element ref="group"/>
      <xsd:element ref="shape"/>
      <xsd:element ref="shapetype"/>
      <xsd:element ref="arc"/>
      <xsd:element ref="curve"/>
      <xsd:element ref="image"/>
      <xsd:element ref="line"/>
      <xsd:element ref="oval"/>
      <xsd:element ref="polyline"/>
      <xsd:element ref="rect"/>
      <xsd:element ref="roundrect"/>
      <xsd:element ref="o:diagram"/>
    </xsd:choice>
    <xsd:attributeGroup ref="AG_AllCoreAttributes"/>
    <xsd:attributeGroup ref="AG_Fill"/>
    <xsd:attribute name="editas" type="ST_EditAs" use="optional"/>
    <xsd:attribute ref="o:tableproperties"/>
    <xsd:attribute ref="o:tablelimits"/>
  </xsd:complexType>
  <xsd:complexType name="CT_Background">
    <xsd:sequence>
      <xsd:element ref="fill" minOccurs="0"/>
    </xsd:sequence>
    <xsd:attributeGroup ref="AG_Id"/>
    <xsd:attributeGroup ref="AG_Fill"/>
    <xsd:attribute ref="o:bwmode"/