#include "gtest/gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "TacticalAI/NPC.h"

#ifdef __cplusplus
}
#endif

// Originally NPCQuoteInfo is 32 bytes with 4 unused bytes at the beginning in
// Russian version and 4 unused bytes at the end in all other versions.
TEST(OnDiskStructures, NPCQuoteInfo) { EXPECT_EQ(sizeof(NPCQuoteInfo), 28); }
