# How to contribute to Medley Interlisp

Be aware that we are working with 30-year-old code, and there are quite a few unforseen interactions.
The C code was originally written to K&R C before C standards, for a big-endian 32-bit machine.
* The most useful contributions are reproducible errors -- things that don't work as documented.
* Second most useful are reports of unexpected behavior -- things that aren't documented but behave unexpectedly.

## Reporting a bug or feature request
* Ensure the bug was not already reported by searching on GitHub under [Issues](https://github.com/Interlisp/medley/issues) or [Discussions](https://github.com/Interlisp/medley/discussions).
Discussions are for questions or topics where there is some disagreement or uncertainty about the "right" direction.
* If you're unable to find a discussion or open issue addressing the problem, open a new one. Be sure to include a title 
and clear description, as much relevant information as possible, 
and a code sample or an executable test case demonstrating the expected behavior that is not occurring.

## Did you write a patch that fixes a bug?
* Open a new GitHub pull request with the patch
* Ensure the PR description clearly describes the problem and solution. Include the relevant issue number if applicable.
* Keep Pull Requests small and easily reviewable. See https://www.thedroidsonroids.com/blog/splitting-pull-request for
a writeup of good practices.

## Did you fix whitespace, format code, or make a purely cosmetic patch?
Changes that are cosmetic in nature and do not add anything substantial to the stability, functionality, or testability of Medley Interlisp will generally not be accepted.


## Do you intend to add a new feature or change an existing one?
* Suggest your change as a Discussion before starting to write code. Is the feature consistent with the overall Medley Interlisp project goals?
* Do not open an issue on GitHub until you have collected positive feedback about the change. GitHub issues are primarily intended for bug reports and fixes.

Medley Interlisp is a volunteer effort. We want to make this a fun experience for everyone. We encourage you to pitch in and join the team.


(guidelines adapted from https://github.com/rails/rails/blob/master/CONTRIBUTING.md )
