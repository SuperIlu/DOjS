/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "libcpuid.h"

#include "DOjS.h"

void init_cpuid(js_State *J);

/*********************
** static functions **
*********************/
/**
 * @brief get CPUID info object.
 *
 * @param J VM state.
 */
static void f_CpuId(js_State *J) {
    struct cpu_raw_data_t raw;
    struct cpu_id_t data;

    if (cpuid_get_raw_data(&raw) < 0) {
        js_error(J, "Error identifying the CPU: %s\n", cpuid_error());
        return;
    }
    if (cpu_identify(&raw, &data) < 0) {
        js_error(J, "Error identifying the CPU: %s\n", cpuid_error());
        return;
    }

    /*  TODO: object OK, now write what we have in `data'...: */
    js_newobject(J);
    {
        js_pushstring(J, data.vendor_str);
        js_setproperty(J, -2, "vendor_str");
        js_pushstring(J, data.brand_str);
        js_setproperty(J, -2, "brand_str");
        js_pushstring(J, data.cpu_codename);
        js_setproperty(J, -2, "cpu_codename");

        js_pushnumber(J, data.vendor);
        js_setproperty(J, -2, "vendor");
        js_pushnumber(J, data.family);
        js_setproperty(J, -2, "family");
        js_pushnumber(J, data.model);
        js_setproperty(J, -2, "model");
        js_pushnumber(J, data.stepping);
        js_setproperty(J, -2, "stepping");
        js_pushnumber(J, data.ext_family);
        js_setproperty(J, -2, "ext_family");
        js_pushnumber(J, data.ext_model);
        js_setproperty(J, -2, "ext_model");
        js_pushnumber(J, data.num_cores);
        js_setproperty(J, -2, "num_cores");
        js_pushnumber(J, data.num_logical_cpus);
        js_setproperty(J, -2, "num_logical_cpus");
        js_pushnumber(J, data.total_logical_cpus);
        js_setproperty(J, -2, "total_logical_cpus");
        js_pushnumber(J, data.l1_data_cache);
        js_setproperty(J, -2, "l1_data_cache");
        js_pushnumber(J, data.l1_instruction_cache);
        js_setproperty(J, -2, "l1_instruction_cache");
        js_pushnumber(J, data.l2_cache);
        js_setproperty(J, -2, "l2_cache");
        js_pushnumber(J, data.l3_cache);
        js_setproperty(J, -2, "l3_cache");
        js_pushnumber(J, data.l4_cache);
        js_setproperty(J, -2, "l4_cache");
        js_pushnumber(J, data.l1_data_assoc);
        js_setproperty(J, -2, "l1_data_assoc");
        js_pushnumber(J, data.l1_instruction_assoc);
        js_setproperty(J, -2, "l1_instruction_assoc");
        js_pushnumber(J, data.l2_assoc);
        js_setproperty(J, -2, "l2_assoc");
        js_pushnumber(J, data.l3_assoc);
        js_setproperty(J, -2, "l3_assoc");
        js_pushnumber(J, data.l4_assoc);
        js_setproperty(J, -2, "l4_assoc");
        js_pushnumber(J, data.l1_data_cacheline);
        js_setproperty(J, -2, "l1_data_cacheline");
        js_pushnumber(J, data.l1_instruction_cacheline);
        js_setproperty(J, -2, "l1_instruction_cacheline");
        js_pushnumber(J, data.l2_cacheline);
        js_setproperty(J, -2, "l2_cacheline");
        js_pushnumber(J, data.l3_cacheline);
        js_setproperty(J, -2, "l3_cacheline");
        js_pushnumber(J, data.l4_cacheline);
        js_setproperty(J, -2, "l4_cacheline");
        js_pushnumber(J, data.sse_size);
        js_setproperty(J, -2, "sse_size");

        js_pushnumber(J, cpu_clock_measure(400, 1));
        js_setproperty(J, -2, "cpu_clock_measure");
        js_pushnumber(J, cpu_clock());
        js_setproperty(J, -2, "cpu_clock");

        js_newarray(J);
        {
            int idx = 0;
            for (int i = 0; i < NUM_CPU_FEATURES; i++) {
                if (data.flags[i]) {
                    js_pushstring(J, cpu_feature_str(i));
                    js_setindex(J, -2, idx);
                    idx++;
                }
            }
        }
        js_setproperty(J, -2, "features");
    }
}

/**
 * @brief get number of CPUs (actually always 1).
 *
 * @param J VM state.
 */
static void f_NumCpus(js_State *J) { js_pushnumber(J, cpuid_get_total_cpus()); }

/**
 * @brief CPUID capabilities.
 *
 * @param J VM state.
 */
static void f_HasCpuId(js_State *J) { js_pushboolean(J, cpuid_present()); }

/*********************
** public functions **
*********************/
/**
 * @brief initialize cpuid subsystem.
 *
 * @param J VM state.
 */
void init_cpuid(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);
    NFUNCDEF(J, CpuId, 0);
    NFUNCDEF(J, NumCpus, 0);
    NFUNCDEF(J, HasCpuId, 0);
}
