/*
 * SPDX-FileCopyrightText: 2022 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <android-base/logging.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"

using android::base::GetProperty;
using android::base::SetProperty;

constexpr const char* RO_PROP_SOURCES[] = {
    nullptr,
    "bootimage.",
    "odm.",
    "odm_dlkm.",
    "product.",
    "system.",
    "system_dlkm.",
    "system_ext.",
    "vendor.",
    "vendor_dlkm.",
};

/*
 * SetProperty does not allow updating read only properties and as a result
 * does not work for our use case. Write "OverrideProperty" to do practically
 * the same thing as "SetProperty" without this restriction.
 */
void OverrideProperty(const char* name, const char* value) {
    size_t valuelen = strlen(value);

    prop_info* pi = (prop_info*)__system_property_find(name);
    if (pi != nullptr) {
        __system_property_update(pi, value, valuelen);
    } else {
        __system_property_add(name, strlen(name), value, valuelen);
    }
}

void OverrideCarrierProperties() {
    const auto ro_prop_override = [](const char* source, const char* prop, const char* value,
                                     bool product) {
        std::string prop_name = "ro.";

        if (product) prop_name += "product.";
        if (source != nullptr) prop_name += source;
        if (!product) prop_name += "build.";
        prop_name += prop;

        OverrideProperty(prop_name.c_str(), value);
    };

    // Setting carrier prop
    std::string carrier = GetProperty("ro.boot.carrier", "unknown");
    OverrideProperty("ro.carrier", carrier.c_str());

    std::string bootsku = GetProperty("ro.boot.hardware.sku", "");
    if (bootsku == "XT2127-3") {
        /* Lenovo K13 Note */
        for (const auto &source : RO_PROP_SOURCES) {
            ro_prop_override(source, "device", "capri", true);
            ro_prop_override(source, "model", "Lenovo K13 Note", true);
        }
    } else if (bootsku == "XT2127-4") {
        /* Moto G10n Power */
        if (carrier == "retmea") {
            for (const auto &source : RO_PROP_SOURCES) {
                ro_prop_override(source, "device", "capri", true);
                ro_prop_override(source, "model", "moto g(10)n power", true);
            }
        } else {
            /* Moto G10 Power */
            for (const auto &source : RO_PROP_SOURCES) {
                ro_prop_override(source, "device", "capri", true);
                ro_prop_override(source, "model", "moto g(10) power", true);
            }
        }
    }
}

void vendor_load_properties() {
    OverrideCarrierProperties();
}
