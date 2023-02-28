# Notes on cross-platform compatibility

## Don't use %S in printf family of functions

Because it is not standard and on Windows behave differently depending on the function.
Use `%s` for `char *` and `%ls` for `wchar_t *`.

On Linux:
- `%s` means `char *`
- `%ls` means `wchar_t *`
- `%S` means `wchar_t *`, don't use

On Windows:
- `%s` means `char *` or `wchar_t *` depending on the function (`char *` for printf, `wchart_t *` for wprintf)
- `%ls` means `wchar_t *`
- `%S` means `wchar_t *`, don't use
