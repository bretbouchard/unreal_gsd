# Phase 11: Council Fixes - Research Summary

## Overview

This phase addresses all critical, high, and medium priority issues identified by the Council of Ricks All Hands review on 2026-02-27.

## Council Review Summary

**Total Issues Found**: 34
- **Critical (P0)**: 12 - Build blockers, must fix immediately
- **High (P1)**: 16 - Functional issues, fix within sprint
- **Medium (P2)**: 5 - Technical debt, schedule appropriately
- **Low (P3)**: 1 - Minor improvements

## Issue Categories

### P0 - Critical (Blocking)

| # | Category | Issue | Specialist | Fix Complexity |
|---|----------|-------|------------|----------------|
| 1 | API/Macro | Missing GSD_*_API macros on public classes | Architecture Rick | Low |
| 2 | Dependency | Missing MassEntity module dependencies | Architecture Rick | Low |
| 3 | Dependency | Missing GameplayTags module dependency | Architecture Rick | Low |
| 4 | Testing | No automation tests exist (claimed 11) | Validation Rick | Medium |
| 5 | Testing | No CI/CD pipeline configuration | Validation Rick | Medium |
| 6 | Testing | No test module structure | Validation Rick | Medium |
| 7 | Audio | No IGSDAudioInterface implementation | Audio Rick | Medium |
| 8 | Audio | No Sound Class hierarchy | Audio Rick | Medium |
| 9 | Audio | No Submix chain | Audio Rick | Medium |
| 10 | AI/Crowds | Missing Build.cs for GSD_Crowds | Swarm Rick | Low |
| 11 | AI/Crowds | No Hero NPC AI Controller | Swarm Rick | High |
| 12 | AI/Crowds | No ZoneGraph navigation integration | Swarm Rick | High |

### P1 - High Priority

| # | Category | Issue | Specialist | Fix Complexity |
|---|----------|-------|------------|----------------|
| 13 | Data-Driven | Hardcoded detection radius (5000.0f) | Architecture Rick | Low |
| 14 | Data-Driven | Hardcoded LOD thresholds | Architecture Rick | Low |
| 15 | Data-Driven | Hardcoded behavior values | Architecture Rick | Low |
| 16 | Data-Driven | Hardcoded timing values | Architecture Rick | Low |
| 17 | Audio | No spatial audio attenuation | Audio Rick | Medium |
| 18 | Audio | No audio LOD in crowd processor | Audio Rick | Medium |
| 19 | Audio | No audio concurrency rules | Audio Rick | Low |
| 20 | AI/Crowds | No Smart Object system | Swarm Rick | High |
| 21 | AI/Crowds | No budget management | Swarm Rick | Medium |
| 22 | AI/Crowds | No archetype data assets | Swarm Rick | Medium |
| 23 | AI/Crowds | No navigation processor | Swarm Rick | High |
| 24 | Testing | No JSON output for CI | Validation Rick | Low |
| 25 | Testing | No standardized exit codes | Validation Rick | Low |
| 26 | Testing | No performance benchmark tests | Validation Rick | Medium |
| 27 | Testing | No headless execution support | Validation Rick | Low |
| 28 | Interface | Missing cross-module interfaces | Architecture Rick | Medium |

### P2 - Medium Priority

| # | Category | Issue | Specialist | Fix Complexity |
|---|----------|-------|------------|----------------|
| 29 | Architecture | Game-specific "Zombie" in platform code | Prime Rick | Medium |
| 30 | Architecture | Missing PCHUsage configuration | Architecture Rick | Low |
| 31 | Architecture | No Editor module for plugins | Architecture Rick | Medium |
| 32 | Architecture | Processor execution order not specified | Architecture Rick | Low |
| 33 | Testing | Fragment missing validation | Validation Rick | Low |
| 34 | Documentation | TODO comments without tickets | Slick Rick | Low |

## Proposed Plan Structure

### Wave 1: Critical Build Fixes (P0 - 1-2 hours)
- **11-01**: API Macro Fixes - Add GSD_*_API to all public classes
- **11-02**: Module Dependency Fixes - Add missing Build.cs dependencies

### Wave 2: Test Infrastructure (P0 - 2-3 hours)
- **11-03**: Automation Test Framework - Create test module structure and 15+ tests
- **11-04**: CI/CD Pipeline - GitHub Actions workflow with automation tests

### Wave 3: Audio Infrastructure (P0/P1 - 2-3 hours)
- **11-05**: Core Audio System - IGSDAudioInterface, Sound Classes, Submix chain
- **11-06**: Spatial Audio Configuration - Attenuation settings, audio LOD

### Wave 4: Data-Driven Configuration (P1 - 1-2 hours)
- **11-07**: Config DataAssets - Replace all hardcoded values with configurable assets

### Wave 5: Crowd AI Completion (P1 - 3-4 hours)
- **11-08**: Hero NPC AI System - AI Controller, Behavior Tree, Perception
- **11-09**: Navigation Integration - ZoneGraph processor, Smart Objects, Budget system

### Wave 6: Platform Cleanup (P2 - 1-2 hours)
- **11-10**: Code Quality Pass - Remove game-specific code, add PCH config, fix TODOs

## Estimated Total Effort

| Wave | Plans | Estimated Time | Priority |
|------|-------|----------------|----------|
| 1 | 2 | 1-2 hours | P0 Critical |
| 2 | 2 | 2-3 hours | P0 Critical |
| 3 | 2 | 2-3 hours | P0/P1 |
| 4 | 1 | 1-2 hours | P1 High |
| 5 | 2 | 3-4 hours | P1 High |
| 6 | 1 | 1-2 hours | P2 Medium |
| **Total** | **10** | **10-16 hours** | - |

## Success Criteria

After Phase 11 completion:

1. **All plugins compile** without linker errors (API macros fixed)
2. **All module dependencies declared** in Build.cs files
3. **15+ automation tests exist** and pass
4. **CI/CD pipeline runs** on every push
5. **Audio infrastructure complete** with Sound Classes, Submixes, spatial audio
6. **All hardcoded values replaced** with DataAsset configuration
7. **Hero NPC AI functional** with Behavior Tree and Perception
8. **Crowd navigation working** via ZoneGraph
9. **No game-specific code** in platform plugins
10. **Council re-review passes** with APPROVE status

## Requirements Coverage

This phase introduces new internal requirements:

| ID | Description | Priority |
|----|-------------|----------|
| FIX-01 | All public classes have API macros | Critical |
| FIX-02 | All module dependencies declared | Critical |
| FIX-03 | Automation test coverage >60% | Critical |
| FIX-04 | CI/CD pipeline operational | Critical |
| FIX-05 | Audio infrastructure complete | High |
| FIX-06 | Data-driven configuration | High |
| FIX-07 | Crowd AI systems complete | High |
| FIX-08 | Platform code is game-agnostic | Medium |

## Dependencies

- Phase 11 depends on: Phases 1-10 (all complete)
- Phase 11 enables: Production deployment, Council approval

## Risks

1. **Scope creep** - Some fixes may reveal deeper issues
2. **Test flakiness** - New tests may be unstable initially
3. **Audio complexity** - MetaSounds integration may require iteration
4. **AI behavior** - Crowd navigation tuning is iterative

## Mitigation

1. Time-box each wave to prevent scope creep
2. Start with smoke tests, expand coverage incrementally
3. Use placeholder sounds initially, refine later
4. Document known limitations for future phases
