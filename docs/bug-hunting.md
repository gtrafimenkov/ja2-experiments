# Bug hunting tips and tricks

It is very easy to introduce bugs into the codebase.  It is unavoidable
when doing any changes.

Here are some tips to make bug hunting easier:
- make small logical commits, don't squash history.  It will be easier to find
  bugs using `git bisect` when there are a number of small commits instead of a
  big one commit
