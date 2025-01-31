#include "Rect.h"

int32_t GetRectWidth(const struct Rect* r) { return r->right - r->left; }
int32_t GetRectHeight(const struct Rect* r) { return r->bottom - r->top; }
