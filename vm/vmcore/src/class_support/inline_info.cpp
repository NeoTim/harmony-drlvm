#define LOG_DOMAIN "method.inline"
#include "cxxlog.h"

#include "inline_info.h"
#include "class_member.h"
#include "jvmti_direct.h"

InlineInfo::InlineInfo(): _entries(0)
{}

void InlineInfo::add(Method* method, uint32 codeSize, void* codeAddr, 
                     uint32 mapLength, AddrLocation* addrLocationMap)
{
    Entry entry = {method, codeSize, codeAddr, mapLength, addrLocationMap};
 
    LMAutoUnlock au(& _lock);

    TRACE("Adding Inlined method: " << method->get_class()->get_name()->bytes 
            << "." << method->get_name()->bytes 
            << " " << method->get_descriptor()->bytes 
            << "\taddress: " << codeAddr 
            << " [" << codeSize << "]\t" 
            << "mapLength: " << mapLength);

   _entries.push_back(entry);
} // InlineInfo::add

void InlineInfo::send_compiled_method_load_event(Method *method)
{
    LMAutoUnlock au(& _lock);

    for (iterator i = _entries.begin(); i != _entries.end(); i++) {
        Entry& e = *i;
        jvmti_send_region_compiled_method_load_event(e.method, e.codeSize,
                e.codeAddr, e.mapLength, 
                e.addrLocationMap, NULL);
    }

} // InlineInfo::send_compiled_method_load_event