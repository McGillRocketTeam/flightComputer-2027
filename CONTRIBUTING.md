# Flight Computer Software Contributing Guide

So you'd like to contribute the MRT Flight Computer Software project? Below are some guidelines for contributers to follow. Any help with documentation, unit tests, framework code, continuous-integration, or simply reporting bugs and improvement ideas is greatly appreciated.<br><br>

> If you are not a McGill student, then unfortunately the only way of contributing is through the GitHub issues. Any questions/ideas should be placed in the issues pages.

## Code of Conduct

This project and everyone in it is governed by the [Code of Conduct](https://github.com/McGillRocketTeam/flightComputer-2027/blob/main/CODE_OF_CONDUCT.md). Any violation of these rules will result in immediate expulsion from team and will be prevented from any further code contributions.

## Ways to Contribute

### Discussions and Questions

For discussions, questions, or ideas please start a new discussion in the `#av_flightcomputer` slack channel. 

### Bug Reports

Before reporting a bug, please perform a search to see if the bug has already been reported. If not report exists:

1. Create a new issue.
2. Tag the issue with the `bug` label and any other applicable labels.
3. Describe the issue.
4. Provide the reproduction steps that someone else can follow to recreate bug.
5. If applicable, add code snippets or references to the software.
6. Provide any additional information.

### Pull Requests

Ready to merge your code? Follow these steps:

1. Create a [pull request(PR)](https://docs.github.com/en/pull-requests).
2. For the title, use the convention `Fix #ISSUE_NUM, SHORT_DESCRIPTION`.
3. Provide what testing was used to confirm the PR resolves the issue. If you're writing new code, please provide the associated unit testing files.
4. Provide any additional context (expected behavior, any known issues)
5. Verify that the PR passes all workflow checks.

## Writing High-Quality Code

Writing code is hard. Writing flight critical code is harder. Here are some tips to make your (and our) life a little easier.

- Check the [NASA naming guide](https://github.com/nasa/cFE/blob/main/docs/cFS_IdentifierNamingConvention.md) for some good practices when naming code.
- The [JSF Style Guide](https://www.stroustrup.com/JSF-AV-rules.pdf) dictates rules to write safe code, it is preferable to follow these.
- Commit early and often. You will not remember later to commit your code, do it before leaving your workstation.

And most importantly, **HAVE FUN**. This is a really cool project and you're on an incredible team. There are people here to help you and the best coders have fun while doing it.
