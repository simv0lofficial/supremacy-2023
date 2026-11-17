#pragma once
#include <cstddef>
#include <cstdint>

namespace skeet::engine {

struct RecvTable;
struct RecvProp;
struct RecvProxyData;
using RecvVarProxy = void (*)(const RecvProxyData*, void*, void*);
using ArrayLengthProxy = void (*)(void*, int, int);
using DataTableProxy = void (*)(const RecvProp*, void**, void*, int);

// Field order is copied from the ntv_* declarations embedded in
// skeet/Binary/initializer.h, byte offsets 0x4B074..0x4B8BD. The native
// 0x433ECB60 walker independently confirms prop stride, type, child and offset.
// Host-width pointers are used by private tests; the target ABI is x86.
#pragma pack(push, 4)
struct RecvProp {
    const char* name;
    std::int32_t type;
    std::int32_t flags;
    std::int32_t string_buffer_size;
    bool inside_array;
    std::uint8_t reserved[3];
    const void* extra_data;
    RecvProp* array_property;
    ArrayLengthProxy array_length_proxy;
    RecvVarProxy proxy;
    DataTableProxy data_table_proxy;
    RecvTable* data_table;
    std::int32_t offset;
    std::int32_t element_stride;
    std::int32_t element_count;
    const char* parent_array_property_name;
};

struct RecvTable {
    RecvProp* properties;
    std::int32_t property_count;
    void* decoder;
    const char* name;
    bool initialized;
    bool in_main_list;
    std::uint8_t reserved[2];
};

struct ClientClass {
    void* create;
    void* create_event;
    const char* network_name;
    RecvTable* table;
    ClientClass* next;
    std::int32_t class_id;
};

struct DataMap;
struct DataMapField {
    std::int32_t type;
    const char* name;
    std::uint32_t offset;
    std::uint8_t unknown_0c[0x10];
    DataMap* embedded;
    std::uint8_t unknown_20[0x1c];
};
struct DataMap {
    DataMapField* fields;
    std::int32_t field_count;
    const char* class_name;
    DataMap* base;
    std::uint32_t opaque_10 = 0;
    void* optimized = nullptr; // native+14, consumed by433C046C
};
#pragma pack(pop)

#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(RecvProp) == 0x3c);
static_assert(offsetof(RecvProp, type) == 4);
static_assert(offsetof(RecvProp, data_table) == 0x28);
static_assert(offsetof(RecvProp, offset) == 0x2c);
static_assert(sizeof(RecvTable) == 0x14);
static_assert(offsetof(RecvTable, property_count) == 4);
static_assert(offsetof(RecvTable, name) == 0xc);
static_assert(sizeof(ClientClass) == 0x18);
static_assert(offsetof(ClientClass, table) == 0xc);
static_assert(offsetof(ClientClass, next) == 0x10);
static_assert(sizeof(DataMapField) == 0x3c);
static_assert(offsetof(DataMapField, name) == 4);
static_assert(offsetof(DataMapField, offset) == 8);
static_assert(offsetof(DataMapField, embedded) == 0x1c);
static_assert(offsetof(DataMap, class_name) == 8);
static_assert(offsetof(DataMap, base) == 0xc);
static_assert(offsetof(DataMap, optimized) == 0x14);
#endif

inline constexpr std::int32_t recv_data_table = 6;

// Complete 0x433ECB60 on a valid acyclic receive-table graph. Offsets are
// unsigned dwords, including wraparound, and the result is an offset, not size.
std::uint32_t maximum_property_offset(const RecvTable& table, std::uint32_t base = 0,
                                      std::uint32_t minimum = 0);

// Complete native lookup helpers. Inputs are valid engine-owned lists/trees;
// these preserve first-match traversal, including duplicate hash behavior.
std::uint32_t netvar_name_hash(const char* name) noexcept; // 0x43359C70
ClientClass* find_client_class(void* base_client, std::uint32_t hash); // 0x433412C1
RecvTable* find_receive_table(RecvTable& table, std::uint32_t hash); // 0x4336AF41
RecvProp* find_receive_property(RecvTable& table, std::uint32_t hash); // 0x433F1534
// 0x43381D8D ignores properties whose name starts with the four bytes "base".
// A missing property is UINT32_MAX, and zero is a valid discovered offset.
std::uint32_t find_receive_offset(RecvTable& table, std::uint32_t hash, std::uint32_t base = 0);
// Complete 0x4340C2A3: engine class list -> recursive table -> recursive property.
RecvProp* find_receive_property(void* base_client, std::uint32_t class_hash,
                                std::uint32_t table_hash, std::uint32_t property_hash);

// Metadata operands returned by the protected 0x43493903 accessor. Resolving
// that accessor is outside this native unit; no table/property hash is invented.
struct OffsetBinding {
    std::uint32_t class_hash;
    std::uint32_t property_hash;
    std::uint32_t* xor_key;
    std::uint32_t** pointer_output;
    std::uint32_t addend = 0; // fifth metadata operand in 0x43486F71
};

// Native bodies 0x4348F24A / 0x43486F71 after their metadata reads. A missing
// class performs no writes. A missing property is encoded as the native -1.
// cursor is an existing dword-key stream, not an output buffer of offsets.
void bind_receive_offset(void* base_client, const OffsetBinding& binding, std::uint32_t*& cursor);
// Native consumers decode *pointer_output ^ xor_key ^ their compile-time key.
std::uint32_t decode_receive_offset(const std::uint32_t* pointer, std::uint32_t xor_key,
                                    std::uint32_t consumer_key) noexcept;

// Complete 0x43322A25. The table hash belongs to DataMap::class_name. The
// native lookup skips unnamed/short fields and does not add embedded offsets.
std::uint32_t find_data_map_offset(DataMap& map, std::uint32_t table_hash,
                                   std::uint32_t property_hash);

// Complete native search 0x433C1154 with its wildcard skip-table helpers.
std::byte* find_byte_pattern(std::byte* first, std::uint32_t size, const std::uint8_t* pattern,
                             std::uint32_t length, std::uint8_t wildcard);
// Complete 0x4337F95A. The 29-byte constructor pattern is at native434729E0;
// module SizeOfImage is read at e_lfanew+0x50. At most43 matches are inspected.
DataMap* find_data_map(std::byte* client_module, std::uint32_t class_hash);

struct DataMapOffsetBinding {
    DataMap* map;
    std::uint32_t table_hash;
    std::uint32_t property_hash;
    std::uint32_t* xor_key;
    std::uint32_t** pointer_output;
};
// Complete native body0x434823F9 after the protected metadata reads.
void bind_data_map_offset(const DataMapOffsetBinding& binding, std::uint32_t*& cursor);

} // namespace skeet::engine
