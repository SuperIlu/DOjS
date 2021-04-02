/**
 * libcpuid mapper.
 * 
 * **Note: CPUID module must be loaded by calling LoadLibrary("cpuid") before using!**
 * 
 * @see LoadLibrary()
 * 
 * @module cpuid
 */

/**
 * @returns {number} always 1.
 */
function NumCpus() { }

/**
 * @returns {boolean} true if the CPU supports the CPUID instruction, else false.
 */
function HasCpuId() { }

/**
 * 
 */
function CpuId() { }