set(MAF_PRECOMPILE_HEADERS  <ctime> <cassert> <iomanip> <chrono>
    <unordered_map> <unordered_set>  <map> <set>
    <list> <forward_list> <queue> <vector>
    <sstream> <ostream> <streambuf> <string> <string_view>
    <tuple> <type_traits> <iterator>
    <future> <thread> <mutex> <atomic>
    <memory> <algorithm> <functional> <typeindex> <typeinfo>
    <any> <variant>
    <maf/logging/Logger.h>)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${MAF_PRECOMPILE_HEADERS})
