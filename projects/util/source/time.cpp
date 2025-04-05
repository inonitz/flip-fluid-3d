#include "util/time.hpp"
#include "util/ifcrash.hpp"


Time::Timestamp g_generalPurposeStamp[TIME_NAMESPACE_STATIC_TIMESTAMP_MAXIMUM];


Time::Timestamp& Time::getGeneralPurposeStamp(u8 index)
{
    ifcrash_debug(index > TIME_NAMESPACE_STATIC_TIMESTAMP_MAXIMUM); 
    return g_generalPurposeStamp[index];
}
