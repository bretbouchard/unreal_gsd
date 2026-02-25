# Council of Ricks Infrastructure

The "shepherd" layer that enforces standards and automates work.

## Council Services

### 1. Project Shepherd API

- "Create new game client"
- "Register new city dataset"
- "Run validation suite"
- "Trigger build"
- "Publish nightly artifacts"

### 2. Asset Ingestion Service

- Watches city exports
- Runs conversion scripts
- Validates chunking/LODs/collision budgets
- Produces "import manifest" for Unreal

### 3. Validation/QA Agent

- Runs Unreal automation tests headlessly
- Reads telemetry logs
- Fails builds when budgets exceeded

### 4. Gameplay Standards Agent

- Enforces naming conventions
- Gameplay tag registry
- Data asset schemas
- "No hardcoding" rules for events

### 5. Performance Agent

- Runs perf test maps on fixed camera routes
- Compares against baselines
- Flags regressions

## Interfaces

| Interface | Purpose |
|-----------|---------|
| MCP Server | Exposes "project operations" as tools |
| A2A Manifests | Chaining between agents (ingest → validate → build → report) |
| OpenAPI | Optional external control panels |

## CI/CD Pipeline

### Build Targets

- Win64 Development Editor
- Win64 Shipping
- (Optional) Linux server build

### Automated Steps

1. Lint Unreal project structure (naming, required folders)
2. Run Editor commandlets:
   - Asset validation commandlet
   - Map check commandlet
3. Run automated tests:
   - Vehicle testbed
   - Crowd testbed
   - City streaming testbed
4. Package build
5. Upload artifacts + telemetry summary

## Validation Tooling (Editor Plugin)

### Editor UI

- "Validate City Tiles"
- "Validate Asset Budgets"
- "Generate LODs for selection"
- "Fix collision presets"
- "Generate streaming chunks"

### Headless Commandlets

- `GSDValidateAssets`
- `GSDValidateWorldPartition`
- `GSDRunPerfRoute`

Council calls these headlessly in CI.

## Definition of Done

Infrastructure is "ready" when:

- ✅ Can ingest Blender export tile set with:
  - Correct scale
  - Correct collision
  - Correct LODs
  - Correct streaming
- ✅ Can spawn:
  - Vehicles from data
  - 200+ zombies (Mass)
  - Hero passenger NPCs
- ✅ Can run:
  - Validation commandlets headlessly
  - Perf route tests
  - Nightly CI packaging
- ✅ Council can:
  - Orchestrate the above
  - Produce single report
