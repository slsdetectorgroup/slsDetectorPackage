<!--
Please complete all relevant sections. If not, validation could fail.

The expandable sections are guidance only — do not modify or fill them out.
-->

<details>
<summary>Help</summary>

The following are managed automatically by workflow. Do not set them manually. 
- Labels: Feature, Bug Fix, Infrastructure, Breaking API
- Milestone

See PR Automation & Validation below for details.
</details>



## PR type

<details>
<summary>Help</summary>

- Feature PR:
   - Minor/Major change
   - Enhancement
   - Compilation/dependency change
   - Bug fix to an unreleased feature
- Bug Fix PR:
   - Bug fix to released code
   - Modifications to released documentation
- Infrastructure PR:
   - Workflow/process change
   - Helper script
   - Internal documentation change
   - Bug fix to unreleased infrastructure
</details>

**Select exactly one:**
<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] Feature
- [ ] Bug Fix
- [ ] Infrastructure



## Breaking API

<details>
<summary>Help</summary>

Not affecting Infrastructure PRs.

Breaking API includes:
- API breaks
- Not server-firmware API break
- Major file version change
</details>

<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] This is a Breaking API change.



## Description
<!-- Describe what this PR changes and why. -->



## Equivalent PRs
<details>
<summary>Help</summary>

- Equivalent PR is a duplicated PR on another branch (RC or developer).
- If the current PR doesn't have any release notes, link the PR that does.
- Equivalent PRs in RC branches keep release notes instead of developer branch PRs.
</details>

<!-- Link any equivalent PRs using a prefix '#' followed by PR number -->



## Referenced PRs
<details>
<summary>Help</summary>

- Referenced PR is any related PR.
- If the current PR doesn't have any release notes, link the PR that does.
- Eg. Current PR has no release notes when it is a bug fix to an unreleased feature PR that already has release notes.
</details>

<!-- Link any referenced PRs using a prefix '#' followed by PR number -->



## Release notes

<details>
<summary>Help</summary>

Not affecting Infrastructure PRs.

Choose one:
- Write release notes
- Check the 'No Release Note' if PR deals with:
    - Chip test board type or developing detector
    - an equivalent PR in release candidate branch with release notes for this PR
    - a linked PR with release notes for this PR
</details>

<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] No Release Note
<!-- Choose either 'No Release Note' or describe the user-visible change introduced by this PR below. -->



## Checklist:

<details>
<summary>Before submitting</summary>

- I have chosen the primary PR type.

Feature or Bug Fix PRs:
- I have checked if this is a Breaking API change.
- I have linked any 
    - equivalent PRs that are on another branch.
    - related PRs eg. fixes/additions to an unreleased feature PR. 
- I have provided a release note above or checked 'No Release Note'.
- I have added detector labels. Some automation in adding labels from code change and PR description.

Infrastructure PRs:
- I have linked any equivalent PRs that are in another branch.

</details>



## PR Automation & Validation
<details>
<summary>Additional Information</summary>

List of automation and validations can be found at https://psich.atlassian.net/wiki/x/AQB2P

</details>