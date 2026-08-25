<!--
Please review the checklist at the bottom before submitting. If ignored, the PR validation could fail.
-->

## PR type

<details>
<summary>PR types and its descriptions</summary>

- Feature PR:
  - can be a Minor or Major change.
  - can be an enhancement or change to existing features.
  - can be a compilation or dependency change.
  - can be a bug fix to a feature not released yet.

- Bug Fix PR:
  - can be a bug fix to already released code.
  - can be modifications to already released documentation regarding code.

- Infrastructure PR:
  - can be a workflows fix or change.
  - can be a change in internal processes documentation.
  - can be a helper script.
  - can be a bug fix to an infrastructure not released yet.

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

## Referenced PRs
<!-- Link any 
- equivalent PRs in a release candidate or developer branch here
- related PRs e.g. fixes/additions to an unreleased feature PR.
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
- [ ] No Release Note
<!-- Describe the user-visible change introduced by this PR below. -->



## Checklist:
- I have chosen the primary PR type.

Feature or Bug Fix PRs:
- I have checked if this is a Breaking API change.
- I have linked any 
    - equivalent PRs that is on another branch.
    - related PRs eg. fixes/additions to an unrelease feature PR. 
- I have provided a release note above or checked 'No Release Note'.

Infrastructure PRs:
- I have linked any equivalent PRs that is in an another branch.