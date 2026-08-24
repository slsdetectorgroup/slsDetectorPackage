<!--
Please review the checklist at the bottom before submitting. If ignored, the PR validation could fail.
-->

## PR type

<details>
<summary>PR types and its descriptions</summary>

- Feature
  - Minor/Major
  - Features or changes
  - Compilation or dependency
  - Bug fixes to a feature not released yet
- Bug Fix
  - Bug fixes to released code
  - Forgotten/modifying released documentation about code
- Infrastructure
  - Workflows fix/change
  - Internal processes documentation
  - Helper Scripts
  - Bug fixes to an infrastructure not released yet

</details>

**Select exactly one:**
<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] Feature
- [ ] Bug Fix
- [ ] Infrastructure

## Breaking API
<details>
<summary>Note</summary>
- Ignore this section for an Infrastructure PR<br><br>

Others:
Breaking API includes:
- API breaks
- Not server-firmware API break
- Major file version change (not just aare affected)
</details>

<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] This is a Breaking API change.

## Description
<!-- Describe what this PR changes and why. -->

## Equivalent PRs
<!-- Link any equivalent PRs in a release candidate or developer branch here.
eg. #1245 (prefix '#')
-->

## Release notes
<details>
<summary>Note</summary>
- Ignore this section for an Infrastructure PR<br><br>

Others:
- Check the 'No Release Note' if PR deals with:
    - Ctb, Xilinx Ctb or developing detector
    - a linked PR already including release notes for this PR
- Write release notes
</details>
<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] No Release Notes
<!-- Describe the user-visible change introduced by this PR below. -->



## Checklist:
- chosen the primary PR type.

Feature or Bug Fix PRs:
- checked if this is a Breaking API change.
- linked any equivalent PRs that is in an another branch.
- provided a release note above or checked 'No Release Note'.

Infrastructure PRs:
- linked any equivalent PRs that is in an another branch.