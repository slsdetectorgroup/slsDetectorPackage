<!--
Please stick to this template and fill out every section.
The expandable <details> sections provide additional information and tips to help you choose the appropriate options. You do not need to modify them.
Please review the checklist at the bottom before submitting. If ignored, the PR validation could fail.
-->

## PR type

<details>
<summary>?</summary>

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
<summary>?</summary>

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



## Referenced PRs
<!-- Link any 
- equivalent PRs in a release candidate or developer branch here
- related PRs e.g. fixes/additions to an unreleased feature PR.
eg. #1245 (prefix '#')
-->



## Release notes

<details>
<summary>?</summary>

Not affecting Infrastructure PRs.

Choose one:
- Write release notes
- Check the 'No Release Note' if PR deals with:
    - Chip test board type or developing detector
    - a linked PR already including release notes for this PR
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