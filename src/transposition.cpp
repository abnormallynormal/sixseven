#include "transposition.h"

HashEntry table[table_size];

std::atomic<u64> g_tt_probes{0};
std::atomic<u64> g_tt_probe_hash_matches{0};
std::atomic<u64> g_tt_probe_cutoffs{0};
std::atomic<u64> g_tt_stores{0};
