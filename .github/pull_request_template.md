<!--
Please stick to this template and fill out every section.
The expandable <details> sections provide additional information and tips to help you choose the appropriate options. You do not need to modify them.
Please review the checklist at the bottom before submitting. If ignored, the PR validation could fail.
-->

## PR type

<details>
<summary>Help</summary>

- Feature PR:<ul><li>Minor/Major change</li><li>Enhancement</li><li>Compilation/dependency change</li><li>Bug fix to an unreleased feature</li></ul>
- Bug Fix PR:<ul><li>Bug fix to released code</li><li>Modifications to released documentation</li></ul>
- Infrastructure PR:<ul><li>Workflow/process change</li><li>Helper script</li><li>Internal documentation change</li><li>Bug fix to unreleased infrastructure</li></ul>
</details>

**Select exactly one:**
<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] Feature
- [ ] Bug Fix
- [ ] Infrastructure



## Breaking API

<details>
<summary>Help</summary>

Not affecting Infrastructure PRs.<br><br>
Breaking API includes:<ul><li>API breaks</li><li>Not server-firmware API break</li><li>Major file version change (not just aare affected)</li></ul>
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
<summary>Help</summary>

Not affecting Infrastructure PRs.

<ul><li>Either write release notes</li>
<li> Or check the 'No Release Note' if PR deals with:
<ul><li>Chip test board type or developing detector</li>
<li>A referenced PR already including release notes for this PR</li></ul></li></ul>
</details>

<!-- Selected: '[x]', Unselected: '[ ]' or Click when not in editing mode -->
- [ ] No Release Note
<!-- Describe the user-visible change introduced by this PR below. -->



##  Checklist:

<details>
<summary> Before submitting </summary>

<ul><li>I have chosen the primary PR type.</li>
<li>Feature or Bug Fix PRs:
<ul><li>I have checked if this is a Breaking API change.</li>
<li>I have linked any
<ul><li>equivalent PRs that is on another branch.</li>
<li>related PRs eg. fixes/additions to an unrelease feature PR. </li></ul></li>
<li>I have provided a release note above or checked 'No Release Note'.</li>
<li>I have added detector labels. Some automation in adding labels from code change and PR description.</li></ul></li>
<li>Infrastructure PRs:
<ul><li>I have linked any equivalent PRs that is in an another branch.</li></ul></li></ul>

</details>



## PR Automation & Validation
<details>
<summary> Additional Info</summary>

List of automation and validations can be found at https://psich.atlassian.net/wiki/x/AQB2P

</details>