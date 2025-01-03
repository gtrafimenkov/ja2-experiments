# Big change: video cleanup started in Dec 2024

## Potential transparency bugs

During the refactoring I was removing code that adds a new video surface
to the global list of surfaces using function `AddVSurface`.

That function not only added the surface to the list, but also set transparency
of the video surface, which was not obvious from the name.

That caused at least one bug.  They might be more of them.
