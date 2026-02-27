# GSD Platform CI/CD Pipeline

## Overview

This document describes the continuous integration and continuous deployment (CI/CD) pipeline for the Unreal GSD Platform.

## Workflow: unreal-ci.yml

### Triggers

- **Push:** Runs on every push to `main` and `develop` branches
- **Pull Request:** Runs on all PRs targeting `main`

### Jobs

#### 1. Build and Test

**Steps:**
1. Checkout repository (with LFS support)
2. Setup MSBuild
3. Generate Unreal project files
4. Build Development Editor configuration
5. Run automation tests
6. Upload test results as artifacts

**Artifacts:**
- `test-results/` - Test logs and automation results

### Requirements

#### Self-Hosted Runner Setup

For best performance, use a self-hosted runner with Unreal Engine pre-installed:

1. **Install Unreal Engine 5.4:**
   - Download from Epic Games Launcher
   - Install to standard location (e.g., C:\Program Files\Epic Games\UE_5.4)

2. **Configure Self-Hosted Runner:**
   ```bash
   # Download GitHub Actions runner
   ./config.sh --url https://github.com/YOUR_ORG/YOUR_REPO --token YOUR_TOKEN

   # Install as service
   sudo ./svc.sh install
   sudo ./svc.sh start
   ```

3. **Set Environment Variables:**
   - UE_ROOT: Path to Unreal Engine installation

#### GitHub-Hosted Runner Alternative

If using GitHub-hosted runners:

1. **Cache UE Installation:**
   - Use actions/cache to cache UE between runs
   - Initial run will be slow (UE download)

2. **Docker Alternative:**
   - Create Docker image with UE pre-installed
   - Use container action

### Test Execution

**Automation Tests:**
- All tests matching "GSD.*" pattern are executed
- Tests run in headless mode (-NullRHI)
- Results logged to Saved/Logs/AutomationTest.log

**Exit Codes:**
- 0 = All tests passed
- 1 = One or more tests failed

**JSON Output (Future):**
```bash
# Enable JSON report export
-ReportExportPath="%CD%\\Saved\\Reports"
```

### Troubleshooting

#### Build Fails

1. **Missing Dependencies:**
   - Ensure all GSD plugins have correct Build.cs dependencies
   - Check for missing module references

2. **Compilation Errors:**
   - Review build log for specific errors
   - Check API macros on public classes

#### Test Failures

1. **Test Discovery:**
   - Verify tests use IMPLEMENT_SIMPLE_AUTOMATION_TEST
   - Check ProductFilter flag is set

2. **Test Execution:**
   - Review AutomationTest.log for details
   - Check for null pointers or assertion failures

### Performance Optimization

1. **Incremental Builds:**
   - Workflow supports incremental compilation
   - Clean builds only on dependency changes

2. **Parallel Jobs:**
   - Build and test can run in parallel for multiple platforms
   - Use matrix strategy for platform combinations

3. **Caching:**
   - Cache UE installation
   - Cache compiled binaries
   - Cache intermediate build files

### Future Enhancements

- [ ] Add Linux build support
- [ ] Add macOS build support
- [ ] Add code coverage reporting
- [ ] Add performance benchmarking
- [ ] Add static analysis (Clang-Tidy)
- [ ] Add documentation generation

### Contact

For CI/CD issues, contact: Bret Bouchard
