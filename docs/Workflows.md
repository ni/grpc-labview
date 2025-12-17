# GitHub Workflows Documentation

This document provides an overview of all GitHub Actions workflows in the grpc-labview project.

## Table of Contents

- [CI Workflow (Main Orchestrator)](#ci-workflow-main-orchestrator)
- [Build Workflows](#build-workflows)
  - [Windows x64 Build](#windows-x64-build)
  - [Windows x86 Build](#windows-x86-build)
  - [Linux Build](#linux-build)
  - [RT Build (Real-Time Linux)](#rt-build-real-time-linux)
- [Release Workflows](#release-workflows)
  - [Build Release Artifacts](#build-release-artifacts)
  - [Update VIPB on Publish Release](#update-vipb-on-publish-release)
- [Integration Workflows](#integration-workflows)
  - [Trigger Azure DevOps CI](#trigger-azure-devops-ci)
  - [Sync GitHub Issues to Azure DevOps](#sync-github-issues-to-azure-devops)

---

## CI Workflow (Main Orchestrator)

**File:** `.github/workflows/ci.yml`

**Purpose:** Main continuous integration workflow that orchestrates all builds and tests.

**Triggers:**
- Push to `master` branch
- Push of tags matching `v*` (e.g., `v1.0.0`)
- Pull request events (opened, synchronized, reopened)

**Concurrency:** Cancels in-progress runs for the same PR or branch to save resources.

**Jobs:**
1. **build_windows_x64** - Calls the Windows x64 build workflow
2. **build_windows_x86** - Calls the Windows x86 build workflow
3. **build_linux** - Calls the Linux build workflow
4. **build_linux_rt** - Calls the RT (Real-Time) Linux build workflow
5. **build_release_artifacts** - Only runs for `ni/grpc-labview` repository
   - Depends on all build jobs completing successfully
   - Calls the release artifacts workflow

---

## Build Workflows

### Windows x64 Build

**File:** `.github/workflows/windows_x64_build.yml`

**Purpose:** Builds the gRPC LabVIEW server and generator libraries for Windows 64-bit.

**Triggers:**
- Called by CI workflow
- Manual dispatch

**Runner:** `windows-2022`

**Build Configuration:** Release

**Steps:**
1. Checkout code
2. Setup MSVC development environment
3. Setup NASM (Netwide Assembler)
4. Update git submodules recursively
5. Configure CMake with Visual Studio 2022 for x64 architecture
6. Build with CMake (16 parallel jobs)
7. Print CMakeTests logs (always runs, even on failure)
8. Create tar.gz archive of build artifacts
9. Upload artifacts (retained for 5 days)

**Artifacts Produced:**
- `labview-grpc-server-windows-x64.tar.gz` containing:
  - `labview_grpc_server.dll`
  - `labview_grpc_generator.dll`

---

### Windows x86 Build

**File:** `.github/workflows/windows_x86_build.yml`

**Purpose:** Builds the gRPC LabVIEW server and generator libraries for Windows 32-bit.

**Triggers:**
- Called by CI workflow
- Manual dispatch

**Runner:** `windows-2022`

**Build Configuration:** Release

**Steps:**
1. Checkout code
2. Setup MSVC development environment for x86 architecture
3. Setup NASM
4. Update git submodules recursively
5. Configure CMake with Visual Studio 2022 for Win32 architecture
6. Build with CMake (16 parallel jobs)
7. Print CMakeTests logs (always runs, even on failure)
8. Create tar.gz archive of build artifacts
9. Upload artifacts (retained for 5 days)

**Artifacts Produced:**
- `labview-grpc-server-windows-x86.tar.gz` containing:
  - `labview_grpc_server.dll`
  - `labview_grpc_generator.dll`

---

### Linux Build

**File:** `.github/workflows/build_on_linux.yml`

**Purpose:** Builds the gRPC LabVIEW server and generator libraries for Linux x64.

**Triggers:**
- Called by CI workflow
- Manual dispatch

**Runner:** `ubuntu-22.04`

**Build Configuration:** Release

**Steps:**
1. Checkout code
2. Update git submodules recursively
3. Create CMake build directory
4. Configure CMake
5. Build with Make (16 parallel jobs)
6. Print CMakeTests logs (always runs, even on failure)
7. Create tar.gz archive of build artifacts
8. Upload artifacts (retained for 5 days)

**Artifacts Produced:**
- `liblabview-grpc-server-linux.tar.gz` containing:
  - `liblabview_grpc_server.so`
  - `liblabview_grpc_generator.so`

---

### RT Build (Real-Time Linux)

**File:** `.github/workflows/build_on_rt.yml`

**Purpose:** Cross-compiles the gRPC LabVIEW server for NI Linux Real-Time targets.

**Triggers:**
- Called by CI workflow
- Manual dispatch

**Runner:** `ubuntu-22.04`

**Build Configuration:** Release

**Special Requirements:**
- Uses NI Linux RT cross-compilation toolchain (2023Q1)
- Requires host OS gRPC support for build tools
- Uses CMake toolchain file `nilrt-x86_64.cmake`

**Steps:**
1. Checkout code
2. Setup Python 3
3. Download and install NI Linux RT cross-compilation toolchain
4. Update git submodules recursively
5. Cache host OS gRPC support (keyed by gRPC commit hash)
6. Configure host OS gRPC (if not cached)
7. Build host OS gRPC (if not cached)
8. Install host OS gRPC support
9. Configure cross-compilation with CMake toolchain
10. Cross-compile for RT target (16 parallel jobs)
11. Print CMakeTests logs (always runs, even on failure)
12. Create tar.gz archive of build artifacts
13. Upload artifacts (retained for 5 days)

**Artifacts Produced:**
- `liblabview-grpc-server-rt.tar.gz` containing:
  - `liblabview_grpc_server.so` (for RT targets)

---

## Release Workflows

### Build Release Artifacts

**File:** `.github/workflows/build_release_artifacts.yml`

**Purpose:** Downloads all platform build artifacts, signs DLLs, builds VI packages, runs tests, and creates GitHub releases.

**Triggers:**
- Called by CI workflow (after all builds complete)
- Manual dispatch

**Runner:** `self-hosted` (requires specific LabVIEW and signing tools)

**Conditional Execution:**
- Full workflow runs only for `ni/grpc-labview` repository or PRs from that repo

**Steps:**
1. Checkout code
2. Setup Python 3.10
3. **Download Build Artifacts:** Downloads compiled DLLs/SOs from all platform builds
   - These are unversioned binary libraries (compiled C++/gRPC code)
4. **Sign DLLs:**
   - Extracts all tar.gz artifacts
   - Signs all DLLs using `nisigntool`
   - Recompresses artifacts
5. **Stage Artifacts:** Organizes artifacts into proper directory structure
6. **Build VI Packages:** ⚠️ **VERSION IS APPLIED HERE**
   - For testing (non-release): Calls `build.py` without `--libraryVersion`
     - Reads version from `build-it/VERSION` file
     - Builds `.vip` packages using that version
   - For release (tags starting with `v*`): Calls `build.py --libraryVersion ${{github.ref_name}}`
     - **build.py updates the `build-it/VERSION` file** with the tag version (strips leading `v`)
     - Then reads the version and builds `.vip` packages
     - The VERSION file change is made but not committed during this workflow
   - Packages the signed binaries into versioned LabVIEW packages
7. **Run CI Tests:** Executes legacy test suite
8. **Run New Testing Suite:** Executes new automated test suite (60-minute timeout)
9. **Upload Test Results:** Saves New_ATS test logs as artifacts (always runs)
10. **Show Test Results:** Prints test logs to workflow output
11. **For Release Tags Only:**
    - Zip all `.vip` files into `grpc-labview.zip`
    - Create draft GitHub release
    - Upload `grpc-labview.zip` to the release

**Artifacts Produced:**
- For all runs: VI packages (`.vip` files) - versioned if built from a tag
- For release tags: `grpc-labview.zip` attached to GitHub release
- Test results from New_ATS suite (timestamped)

**Important Note:** During this workflow, `build.py` updates the `build-it/VERSION` file to set the version, but does NOT commit these changes. The VERSION file serves as the single source of truth for version numbers during builds. After a release is published, the VERSION file should be manually updated and committed (or via a separate workflow/script).

**Testing:**
- Legacy tests via `tests/run_tests.py`
- New automated test suite via `tests/New_ATS/pylib/run_tests.py`

---

### Update Version on Publish Release

**File:** `.github/workflows/update_version_on_publish_release.yml`

**Purpose:** Automatically updates the `build-it/VERSION` file when a release is published and commits it to the repository.

**Triggers:**
- GitHub release published event

**Runner:** `self-hosted`

**Steps:**
1. Checkout code with repository token
2. Setup Python 3.10
3. **Update VERSION File:**
   - Runs `build-it/update_version.py --library_version ${{github.ref_name}}`
   - Updates the `build-it/VERSION` file with the release tag version
   - Strips leading `v` from version (e.g., `v1.2.3.4` → `1.2.3.4`)
4. **Commit Changes:**
   - Auto-commits the updated `build-it/VERSION` file with message "VERSION bump"
   - Pushes directly to `master` branch
   - Uses force push option

**Side Effect:** ⚠️ **This push to master triggers the CI workflow again**, which will build all platforms and packages again. However, since no tag was pushed, the packages won't be attached to any release. This run serves to validate that the repository state is buildable with the updated version.

**Files Modified:**
- `build-it/VERSION`

**Why This Matters:** Keeps the repository's VERSION file in sync with the released version. Future builds (e.g., for development/testing) will use the correct version from the VERSION file as the baseline.

---

### Update VIPB on Publish Release (Deprecated)

**File:** `.github/workflows/update_vipb_on_publish_release.yml`

**Status:** ⚠️ **DISABLED** (via `if: false` condition)

**Purpose:** This is the old workflow that updated `.vipb` files. It has been replaced by `update_version_on_publish_release.yml` which updates the `build-it/VERSION` file instead.

**Why Replaced:** The VERSION file provides a simpler, single-source-of-truth versioning mechanism compared to updating multiple `.vipb` files.

---

## Integration Workflows

### Trigger Azure DevOps CI

**File:** `.github/workflows/trigger_azdo_ci.yml`

**Purpose:** Triggers Azure DevOps pipeline for additional CI/CD processes.

**Triggers:**
- Called by other workflows
- Manual dispatch

**Runner:** `windows-latest`

**Conditions:**
- Only runs for `ni/grpc-labview` repository (not forks)

**Steps:**
1. Trigger Azure DevOps pipeline:
   - Organization: `ni`
   - Project: `DevCentral`
   - Pipeline: `ni-central-grpc-labview-build-vipkg-in-windows`
   - Passes GitHub workflow run ID and release status as variables

**Requirements:**
- `AZURE_DEVOPS_TOKEN` secret must be configured

---

### Sync GitHub Issues to Azure DevOps

**File:** `.github/workflows/sync_github_issue_azdo.yml`

**Purpose:** Automatically synchronizes GitHub issues with Azure DevOps work items.

**Triggers:**
- Issue events: opened, edited, deleted, closed, reopened, labeled, unlabeled, assigned

**Runner:** `ubuntu-latest`

**Steps:**
1. Uses `danhellem/github-actions-issue-to-work-item` action
2. Syncs to Azure DevOps project:
   - Organization: `ni`
   - Project: `DevCentral`
   - Area Path: `DevCentral\Product RnD\Platform HW and SW\Core SW and Drivers\Test System SW\Dev Tools\LabVIEW\NIB Green\grpc-labview`
   - Work Item Type: `Bug`
   - State mapping:
     - New → `New`
     - Active → `Active`
     - Closed → `Closed`

**Requirements:**
- `AZDO_Work_Item_Token` secret for Azure DevOps authentication
- `GH_REPO_TOKEN` secret for GitHub authentication

---

## Workflow Dependencies

```
CI Workflow (ci.yml)
├── Windows x64 Build (windows_x64_build.yml)
├── Windows x86 Build (windows_x86_build.yml)
├── Linux Build (build_on_linux.yml)
├── RT Build (build_on_rt.yml)
└── Build Release Artifacts (build_release_artifacts.yml)
    └── [depends on all builds above]
        ├── Runs tests
        └── Creates release (if tag pushed)

Release Published Event
└── Update Version on Publish Release (update_version_on_publish_release.yml)

Issue Events
└── Sync GitHub Issues to Azure DevOps (sync_github_issue_azdo.yml)
```

---

## Versioning Strategy

The project uses **Git tags and a VERSION file** for versioning:

1. **Tag Creation:** Developers create a Git tag (e.g., `v1.2.3.4`) and push it
2. **First CI Run (Tag Trigger):**
   - Triggered by the tag push
   - Platform Builds: Builds all platforms, producing unversioned DLLs/SOs
   - Package Creation: Build Release Artifacts workflow:
     - Downloads compiled DLLs/SOs from platform builds
     - Signs the binaries
     - Runs `build.py --libraryVersion v1.2.3.4`
       - **build.py updates `build-it/VERSION` file** with version (strips `v` → `1.2.3.4`)
       - **Reads version from VERSION file and builds `.vip` packages**
       - VERSION file change is made but not committed during this workflow
     - Runs tests
     - Creates draft GitHub release
     - Attaches versioned `.vip` packages to release
4. **Manual Step:** Developer reviews and publishes the draft release
5. **Automatic Version File Sync:** When release is published:
   - Update Version workflow automatically triggers
   - Runs `update_version.py --library_version <tag>` to update `build-it/VERSION` file
   - Commits and pushes the updated VERSION file directly to master branch
   - This keeps the repository in sync with the released version
6. **Second CI Run (Master Push Trigger):**
   - Triggered by the VERSION file commit to master
   - Builds all platforms and packages again
   - Validates the repository state with the updated VERSION file
   - Packages are NOT attached to the release (release already exists)

**Version Management:**
- **VERSION file:** `build-it/VERSION` contains the current version in `version=X.Y.Z.W` format (single source of truth)
- **For releases:** Pass `--libraryVersion` to temporarily update VERSION file during build workflow
- **After release:** VERSION file is automatically updated and committed by the Update Version workflow
- **For development:** Build without `--libraryVersion` to use existing VERSION file
- **For testing:** Pass `--lib-version` to override version without updating VERSION file

**Key Points:**
- **Two CI runs occur** for each release (tag push + VERSION file commit to master)
- **First run** creates the release packages
- **Second run** validates the repository state after VERSION file update
- **Build artifacts (DLLs/SOs):** Unversioned binary libraries
- **VI Packages (.vip files):** Versioned based on VERSION file or `--libraryVersion` parameter
- **VERSION file:** Single source of truth, automatically updated after release publication

**Version Format:** `vMAJOR.MINOR.PATCH.BUILD` (e.g., `v1.2.3.4`)
</text>


---

## Self-Hosted Runner Requirements

The following workflows require a self-hosted runner with specific tools:

- **build_release_artifacts.yml:**
  - LabVIEW with VI Package Manager
  - LabVIEW CLI
  - Python 3.10
  - `nisigntool` for code signing
  - Access to test hardware/environment

- **update_vipb_on_publish_release.yml:**
  - LabVIEW CLI
  - Python 3.10

---

## Artifact Retention

All build artifacts are retained for **5 days** to balance storage costs with debugging needs.

Test result artifacts are retained with default settings (90 days).

---

## Common Patterns

### Error Handling
- All workflows print CMakeTests logs even on failure (`if: ${{always()}}`)
- Build failures are captured and reported with error messages
- Test failures upload logs for debugging

### Build Parallelization
- All builds use `-j 16` for parallel compilation
- Significantly reduces build times

### Conditional Release Steps
- Many steps check `if: startsWith(github.ref, 'refs/tags/v')` to run only for releases
- Ensures test builds don't trigger release processes

---

## Secrets Required

The following GitHub secrets must be configured:

- `GH_REPO_TOKEN` - GitHub token with repo access
- `GITHUB_TOKEN` - Automatically provided by GitHub Actions
- `AZDO_Work_Item_Token` - Azure DevOps personal access token for work item sync
- `AZURE_DEVOPS_TOKEN` - Azure DevOps token for pipeline triggering (if using trigger_azdo_ci)