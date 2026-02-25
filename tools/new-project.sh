#!/bin/bash
# new-project.sh - Create a new game project client
# Usage: ./tools/new-project.sh <project_name>

set -e

PROJECTS_DIR="$(cd "$(dirname "$0")/.." && pwd)/projects"
PROJECT_NAME="$1"

if [ -z "$PROJECT_NAME" ]; then
    echo "Usage: ./tools/new-project.sh <project_name>"
    echo "Example: ./tools/new-project.sh zombie_taxi"
    exit 1
fi

PROJECT_PATH="$PROJECTS_DIR/$PROJECT_NAME"

if [ -d "$PROJECT_PATH" ]; then
    echo "Error: Project '$PROJECT_NAME' already exists at $PROJECT_PATH"
    exit 1
fi

echo "Creating new project: $PROJECT_NAME"
echo "Location: $PROJECT_PATH"
echo ""

# Create project directory
mkdir -p "$PROJECT_PATH"
cd "$PROJECT_PATH"

# Initialize private git repo
git init
git config user.email "bret@example.com"
git config user.name "Bret Bouchard"
git branch -m main

# Create .gitignore for Unreal projects
cat > .gitignore << 'EOF'
# Unreal Engine
Binaries/
Build/
DerivedDataCache/
Intermediate/
Saved/
*.sln
*.suo
*.xcodeproj
*.xcworkspace

# Compiled Object files
*.slo
*.lo
*.o
*.obj

# Precompiled Headers
*.gch
*.pch

# Compiled Dynamic libraries
*.so
*.dylib
*.dll

# Fortran module files
*.mod

# Compiled Static libraries
*.lai
*.la
*.a
*.lib

# Executables
*.exe
*.out
*.app

# Visual Studio
.vs/
*.VC.db
*.VC.opendb

# Rider
.idea/

# OS files
.DS_Store
Thumbs.db

# GSD planning (local state)
.planning/STATE.md
.planning/autonomous-log.md
.planning/debug/

# Beads local database
.beads/
EOF

# Create GSD planning structure
mkdir -p .planning/phases

cat > .planning/PROJECT.md << EOF
# PROJECT: $PROJECT_NAME

> Game client for Unreal GSD Platform

## Status

- **Created:** $(date +%Y-%m-%d)
- **Phase:** 0 - Initialization
- **Dependencies:** GSD Platform plugins

## Platform Dependencies

This project depends on:
- GSD_Core
- GSD_CityStreaming
- GSD_Vehicles
- GSD_Crowds
- GSD_DailyEvents
- GSD_Telemetry

## Game-Specific Systems

(Add game-specific systems here)

## Milestones

(Define game milestones here)
EOF

cat > .planning/REQUIREMENTS.md << EOF
# REQUIREMENTS: $PROJECT_NAME

## Game-Specific Requirements

(Add game-specific requirements here)

| ID | Requirement | Priority | Status |
|----|-------------|----------|--------|
| GS-01 | Example requirement | P0 | Open |
EOF

cat > .planning/ROADMAP.md << EOF
# ROADMAP: $PROJECT_NAME

## Phase 0: Initialization

- [x] Project created
- [ ] GSD planning structure
- [ ] Initial commit

## Phase 1: (Define first phase)

(Add phases here)
EOF

# Create Unreal project structure (placeholder)
mkdir -p Config
mkdir -p Content
mkdir -p Source

# Create default Engine config
cat > Config/DefaultEngine.ini << 'EOF'
[/Script/EngineSettings.GameMapsSettings]
GameDefaultMap=/Engine/Maps/Templates/OpenWorld

[/Script/Engine.Engine]
+ActiveGameNameRedirects=(OldGameName="TP_Blank",NewGameName="/Script/PROJECT_NAME")
+ActiveGameNameRedirects=(OldGameName="/Script/TP_Blank",NewGameName="/Script/PROJECT_NAME")

[/Script/HardwareTargeting.HardwareTargetingSettings]
TargetedHardwareClass=Desktop
AppliedTargetedHardwareClass=Desktop
DefaultGraphicsPerformance=Maximum
AppliedDefaultGraphicsPerformance=Maximum
EOF

# Initial commit
git add .
git commit -m "$(cat <<'EOF'
Initial project structure

Generated with [Claude Code](https://claude.ai/code)
via [Happy](https://happy.engineering)

Co-Authored-By: Claude <noreply@anthropic.com>
Co-Authored-By: Happy <yesreply@happy.engineering>
EOF
)"

echo ""
echo "✓ Project created successfully!"
echo ""
echo "Next steps:"
echo "  1. cd projects/$PROJECT_NAME"
echo "  2. Create Unreal project in this directory"
echo "  3. Add GSD platform plugins as dependencies"
echo "  4. Run /gsd:new-project to initialize GSD workflow"
echo "  5. Run /bret:sync to sync with beads tracking"
echo ""
echo "Note: This is a PRIVATE repo. Push to your private remote:"
echo "  git remote add origin <your-private-repo-url>"
echo "  git push -u origin main"
