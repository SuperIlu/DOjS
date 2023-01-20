/*
 * IP2Location C library is distributed under MIT license
 * Copyright (c) 2013-2020 IP2Location.com. support at ip2location dot com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the MIT license
 */

#if defined(_WIN32) && !defined(IS_WATT32)
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <stdint.h>
	#include <string.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#if !defined(_WIN32)
	  #include <unistd.h>
	  #include <sys/mman.h>
	#endif
	#if defined(IS_WATT32)
	  #include <tcp.h>
	#endif
#endif

#if defined(USE_IP2LOCATION) /* Rest of file */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "IP2Location.h"

#ifdef _WIN32
	#define _STR2(x) #x
	#define _STR(x) _STR2(x)
	#define PACKAGE_VERSION _STR(API_VERSION)
	#include <tchar.h>
#else
	#define PACKAGE_VERSION "8.3.1"
#endif

uint8_t COUNTRY_POSITION[25]			= {0,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2};
uint8_t REGION_POSITION[25]				= {0,  0,  0,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3};
uint8_t CITY_POSITION[25]				= {0,  0,  0,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4};
uint8_t ISP_POSITION[25]				= {0,  0,  3,  0,  5,  0,  7,  5,  7,  0,  8,  0,  9,  0,  9,  0,  9,  0,  9,  7,  9,  0,  9,  7,  9};
uint8_t LATITUDE_POSITION[25]			= {0,  0,  0,  0,  0,  5,  5,  0,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5};
uint8_t LONGITUDE_POSITION[25]			= {0,  0,  0,  0,  0,  6,  6,  0,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6};
uint8_t DOMAIN_POSITION[25]				= {0,  0,  0,  0,  0,  0,  0,  6,  8,  0,  9,  0, 10,  0, 10,  0, 10,  0, 10,  8, 10,  0, 10,  8, 10};
uint8_t ZIPCODE_POSITION[25]			= {0,  0,  0,  0,  0,  0,  0,  0,  0,  7,  7,  7,  7,  0,  7,  7,  7,  0,  7,  0,  7,  7,  7,  0,  7};
uint8_t TIMEZONE_POSITION[25]			= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  7,  8,  8,  8,  7,  8,  0,  8,  8,  8,  0,  8};
uint8_t NETSPEED_POSITION[25]			= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8, 11,  0, 11,  8, 11,  0, 11,  0, 11,  0, 11};
uint8_t IDDCODE_POSITION[25]			= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9, 12,  0, 12,  0, 12,  9, 12,  0, 12};
uint8_t AREACODE_POSITION[25]			= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10, 13,  0, 13,  0, 13, 10, 13,  0, 13};
uint8_t WEATHERSTATIONCODE_POSITION[25]	= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9, 14,  0, 14,  0, 14,  0, 14};
uint8_t WEATHERSTATIONNAME_POSITION[25]	= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10, 15,  0, 15,  0, 15,  0, 15};
uint8_t MCC_POSITION[25]				= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9, 16,  0, 16,  9, 16};
uint8_t MNC_POSITION[25]				= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 10, 17,  0, 17, 10, 17};
uint8_t MOBILEBRAND_POSITION[25]		= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 11, 18,  0, 18, 11, 18};
uint8_t ELEVATION_POSITION[25]			= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 11, 19,  0, 19};
uint8_t USAGETYPE_POSITION[25]			= {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 12, 20};

// Static variables
static int32_t is_in_memory = 0;
static enum IP2Location_lookup_mode lookup_mode = IP2LOCATION_FILE_IO; /* Set default lookup mode as File I/O */
static void *memory_pointer;

// Static functions
static int IP2Location_initialize(IP2Location *handler);
static int IP2Location_is_ipv4(char *ip);
static int IP2Location_is_ipv6(char *ip);
static int32_t IP2Location_load_database_into_memory(FILE *file, void *memory_pointer, int64_t size);
static IP2LocationRecord *IP2Location_new_record();
static IP2LocationRecord *IP2Location_get_record(IP2Location *handler, char *ip, uint32_t mode);

#if !defined(_WIN32) && !defined(MSDOS)
  static int32_t shm_fd;
#elif defined(_WIN32)
  HANDLE shm_fd;
#endif

// Open IP2Location BIN database file
IP2Location *IP2Location_open(char *bin)
{
	FILE *f;
	IP2Location *handler;

	if ((f = fopen(bin, "rb")) == NULL) {
		printf("IP2Location library error in opening database %s.\n", bin);
		return NULL;
	}

	handler = (IP2Location *) calloc(1, sizeof(IP2Location));
	handler->file = f;

	IP2Location_initialize(handler);

	return handler;
}

// Initialize database structures
static int IP2Location_initialize(IP2Location *handler)
{
	handler->database_type = IP2Location_read8(handler->file, 1);
	handler->database_column = IP2Location_read8(handler->file, 2);
	handler->database_year = IP2Location_read8(handler->file, 3);
	handler->database_month = IP2Location_read8(handler->file, 4);
	handler->database_day = IP2Location_read8(handler->file, 5);

	handler->database_count = IP2Location_read32(handler->file, 6);
	handler->database_address = IP2Location_read32(handler->file, 10);
	handler->ip_version = IP2Location_read32(handler->file, 14);

	handler->ipv4_database_count = IP2Location_read32(handler->file, 6);
	handler->ipv4_database_address = IP2Location_read32(handler->file, 10);
	handler->ipv6_database_count = IP2Location_read32(handler->file, 14);
	handler->ipv6_database_address = IP2Location_read32(handler->file, 18);

	handler->ipv4_index_base_address = IP2Location_read32(handler->file, 22);
	handler->ipv6_index_base_address = IP2Location_read32(handler->file, 26);

	return 0;
}

#if !defined(MSDOS)

// This function to set the DB access type.
int32_t IP2Location_open_mem(IP2Location *handler, enum IP2Location_lookup_mode mode)
{
	// BIN database is not loaded
	if (handler == NULL) {
		return -1;
	}

	// Existing database already loaded into memory
	if (is_in_memory != 0) {
		return -1;
	}

	// Mark database loaded into memory
	is_in_memory = 1;

	if (mode == IP2LOCATION_FILE_IO) {
		return 0;
	} else if (mode == IP2LOCATION_CACHE_MEMORY) {
		return IP2Location_DB_set_memory_cache(handler->file);
	} else if (mode == IP2LOCATION_SHARED_MEMORY) {
		return IP2Location_DB_set_shared_memory(handler->file);
	} else {
		return -1;
	}
}

// Alias to IP2Location_open_mem()
int32_t IP2Location_set_lookup_mode(IP2Location *handler, enum IP2Location_lookup_mode mode)
{
	return IP2Location_open_mem(handler, mode);
}

// Delete IP2Location shared memory if its present
void IP2Location_delete_shm()
{
    IP2Location_DB_del_shm();
}

// Alias to IP2Location_delete_shm()
void IP2Location_clear_memory()
{
	IP2Location_delete_shm();
}

// Alias to IP2Location_DB_del_shm()
void IP2Location_delete_shared_memory()
{
	IP2Location_DB_del_shm();
}
#endif  /* MSDOS */

// Close the IP2Location database file
uint32_t IP2Location_close(IP2Location *handler)
{
	is_in_memory = 0;

	if (handler != NULL) {
		IP2Location_DB_close(handler->file);
		free(handler);
	}

	return 0;
}

// Compare IPv6 address
int ipv6_compare(struct in6_addr *addr1, struct in6_addr *addr2)
{
	int i, ret = 0;
	for (i = 0; i < 16; i++) {
		if (addr1->s6_addr[i] > addr2->s6_addr[i]) {
			ret = 1;
			break;
		} else if (addr1->s6_addr[i] < addr2->s6_addr[i]) {
			ret = -1;
			break;
		}
	}

	return ret;
}

// Alias to
int IP2Location_ipv6_compare(struct in6_addr *addr1, struct in6_addr *addr2)
{
	return ipv6_compare(addr1, addr2);
}

// Parse IP address into binary address for lookup purpose
static ip_container IP2Location_parse_address(const char *ip)
{
	ip_container parsed;

	if (IP2Location_is_ipv4((char *) ip)) {
		// Parse IPv4 address
		parsed.version = 4;
		inet_pton(AF_INET, ip, &parsed.ipv4);
		parsed.ipv4 = htonl(parsed.ipv4);
	} else if (IP2Location_is_ipv6((char *) ip)) {
		// Parse IPv6 address
		inet_pton(AF_INET6, ip, &parsed.ipv6);

		// IPv4 Address in IPv6
		if (parsed.ipv6.s6_addr[0] == 0 && parsed.ipv6.s6_addr[1] == 0 && parsed.ipv6.s6_addr[2] == 0 && parsed.ipv6.s6_addr[3] == 0 && parsed.ipv6.s6_addr[4] == 0 && parsed.ipv6.s6_addr[5] == 0 && parsed.ipv6.s6_addr[6] == 0 && parsed.ipv6.s6_addr[7] == 0 && parsed.ipv6.s6_addr[8] == 0 && parsed.ipv6.s6_addr[9] == 0 && parsed.ipv6.s6_addr[10] == 255 && parsed.ipv6.s6_addr[11] == 255) {
			parsed.version = 4;
			parsed.ipv4 = (parsed.ipv6.s6_addr[12] << 24) + (parsed.ipv6.s6_addr[13] << 16) + (parsed.ipv6.s6_addr[14] << 8) + parsed.ipv6.s6_addr[15];
		}

		// 6to4 Address - 2002::/16
		else if (parsed.ipv6.s6_addr[0] == 32 && parsed.ipv6.s6_addr[1] == 2) {
			parsed.version = 4;
			parsed.ipv4 = (parsed.ipv6.s6_addr[2] << 24) + (parsed.ipv6.s6_addr[3] << 16) + (parsed.ipv6.s6_addr[4] << 8) + parsed.ipv6.s6_addr[5];
		}

		// Teredo Address - 2001:0::/32
		else if (parsed.ipv6.s6_addr[0] == 32 && parsed.ipv6.s6_addr[1] == 1 && parsed.ipv6.s6_addr[2] == 0 && parsed.ipv6.s6_addr[3] == 0) {
			parsed.version = 4;
			parsed.ipv4 = ~((parsed.ipv6.s6_addr[12] << 24) + (parsed.ipv6.s6_addr[13] << 16) + (parsed.ipv6.s6_addr[14] << 8) + parsed.ipv6.s6_addr[15]);
		}

		// Common IPv6 Address
		else {
			parsed.version = 6;
		}
	} else {
		// Invalid IP address
		parsed.version = -1;
	}

	return parsed;
}

// Get country code
IP2LocationRecord *IP2Location_get_country_short(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, COUNTRYSHORT);
}

// Get country name
IP2LocationRecord *IP2Location_get_country_long(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, COUNTRYLONG);
}

// Get the name of state/region
IP2LocationRecord *IP2Location_get_region(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, REGION);
}

// Get city name
IP2LocationRecord *IP2Location_get_city(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, CITY);
}

// Get ISP name
IP2LocationRecord *IP2Location_get_isp(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, ISP);
}

// Get latitude
IP2LocationRecord *IP2Location_get_latitude(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, LATITUDE);
}

// Get longitude
IP2LocationRecord *IP2Location_get_longitude(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, LONGITUDE);
}

// Get domain name
IP2LocationRecord *IP2Location_get_domain(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, DOMAINNAME);
}

// Get ZIP code
IP2LocationRecord *IP2Location_get_zipcode(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, ZIPCODE);
}

// Get time zone
IP2LocationRecord *IP2Location_get_timezone(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, TIMEZONE);
}

// Get net speed
IP2LocationRecord *IP2Location_get_netspeed(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, NETSPEED);
}

// Get IDD code
IP2LocationRecord *IP2Location_get_iddcode(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, IDDCODE);
}

// Get area code
IP2LocationRecord *IP2Location_get_areacode(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, AREACODE);
}

// Get weather station code
IP2LocationRecord *IP2Location_get_weatherstationcode(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, WEATHERSTATIONCODE);
}

// Get weather station name
IP2LocationRecord *IP2Location_get_weatherstationname(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, WEATHERSTATIONNAME);
}

// Get mobile country code
IP2LocationRecord *IP2Location_get_mcc(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, MCC);
}

// Get mobile national code
IP2LocationRecord *IP2Location_get_mnc(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, MNC);
}

// Get mobile carrier brand
IP2LocationRecord *IP2Location_get_mobilebrand(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, MOBILEBRAND);
}

// Get elevation
IP2LocationRecord *IP2Location_get_elevation(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, ELEVATION);
}

// Get usage type
IP2LocationRecord *IP2Location_get_usagetype(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, USAGETYPE);
}

// Get all records of an IP address
IP2LocationRecord *IP2Location_get_all(IP2Location *handler, char *ip)
{
	return IP2Location_get_record(handler, ip, ALL);
}

// fill the record fields with error message
static IP2LocationRecord *IP2Location_bad_record(const char *message)
{
	IP2LocationRecord *record = IP2Location_new_record();
	record->country_short = strdup(message);
	record->country_long = strdup(message);
	record->region = strdup(message);
	record->city = strdup(message);
	record->isp = strdup(message);
	record->latitude = 0;
	record->longitude = 0;
	record->domain = strdup(message);
	record->zipcode = strdup(message);
	record->timezone = strdup(message);
	record->netspeed = strdup(message);
	record->iddcode = strdup(message);
	record->areacode = strdup(message);
	record->weatherstationcode = strdup(message);
	record->weatherstationname = strdup(message);
	record->mcc = strdup(message);
	record->mnc = strdup(message);
	record->mobilebrand = strdup(message);
	record->elevation = 0;
	record->usagetype = strdup(message);

	// Create alias for the new variables
	record->zip_code = record->zipcode;
	record->time_zone = record->timezone;
	record->net_speed = record->netspeed;
	record->idd_code = record->iddcode;
	record->area_code = record->areacode;
	record->weather_station_code = record->weatherstationcode;
	record->weather_station_name = record->weatherstationname;
	record->mobile_brand = record->mobilebrand;
	record->usage_type = record->usagetype;

	return record;
}

static IP2LocationRecord *IP2Location_read_record(IP2Location *handler, uint32_t rowaddr, uint32_t mode) {
	uint8_t database_type = handler->database_type;
	FILE *handle = handler->file;
	IP2LocationRecord *record = IP2Location_new_record();

	if ((mode & COUNTRYSHORT) && (COUNTRY_POSITION[database_type] != 0)) {
		if (!record->country_short) {
			record->country_short = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (COUNTRY_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->country_short) {
			record->country_short = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & COUNTRYLONG) && (COUNTRY_POSITION[database_type] != 0)) {
		if (!record->country_long) {
			record->country_long = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (COUNTRY_POSITION[database_type] - 1)) + 3);
		}
	} else {
		if (!record->country_long) {
			record->country_long = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & REGION) && (REGION_POSITION[database_type] != 0)) {
		if (!record->region) {
			record->region = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (REGION_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->region) {
			record->region = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & CITY) && (CITY_POSITION[database_type] != 0)) {
		if (!record->city) {
			record->city = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (CITY_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->city) {
			record->city = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & ISP) && (ISP_POSITION[database_type] != 0)) {
		if (!record->isp) {
			record->isp = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (ISP_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->isp) {
			record->isp = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & LATITUDE) && (LATITUDE_POSITION[database_type] != 0)) {
		record->latitude = IP2Location_readFloat(handle, rowaddr + 4 * (LATITUDE_POSITION[database_type] - 1));
	} else {
		record->latitude = 0.0;
	}

	if ((mode & LONGITUDE) && (LONGITUDE_POSITION[database_type] != 0)) {
		record->longitude = IP2Location_readFloat(handle, rowaddr + 4 * (LONGITUDE_POSITION[database_type] - 1));
	} else {
		record->longitude = 0.0;
	}

	if ((mode & DOMAINNAME) && (DOMAIN_POSITION[database_type] != 0)) {
		if (!record->domain) {
			record->domain = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (DOMAIN_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->domain) {
			record->domain = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & ZIPCODE) && (ZIPCODE_POSITION[database_type] != 0)) {
		if (!record->zipcode) {
			record->zipcode = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (ZIPCODE_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->zipcode) {
			record->zipcode = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & TIMEZONE) && (TIMEZONE_POSITION[database_type] != 0)) {
		if (!record->timezone) {
			record->timezone = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (TIMEZONE_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->timezone) {
			record->timezone = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & NETSPEED) && (NETSPEED_POSITION[database_type] != 0)) {
		if (!record->netspeed) {
			record->netspeed = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (NETSPEED_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->netspeed) {
			record->netspeed = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & IDDCODE) && (IDDCODE_POSITION[database_type] != 0)) {
		if (!record->iddcode) {
			record->iddcode = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (IDDCODE_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->iddcode) {
			record->iddcode = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & AREACODE) && (AREACODE_POSITION[database_type] != 0)) {
		if (!record->areacode) {
			record->areacode = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (AREACODE_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->areacode) {
			record->areacode = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & WEATHERSTATIONCODE) && (WEATHERSTATIONCODE_POSITION[database_type] != 0)) {
		if (!record->weatherstationcode) {
			record->weatherstationcode = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (WEATHERSTATIONCODE_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->weatherstationcode) {
			record->weatherstationcode = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & WEATHERSTATIONNAME) && (WEATHERSTATIONNAME_POSITION[database_type] != 0)) {
		if (!record->weatherstationname) {
			record->weatherstationname = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (WEATHERSTATIONNAME_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->weatherstationname) {
			record->weatherstationname = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & MCC) && (MCC_POSITION[database_type] != 0)) {
		if (!record->mcc) {
			record->mcc = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (MCC_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->mcc) {
			record->mcc = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & MNC) && (MNC_POSITION[database_type] != 0)) {
		if (!record->mnc) {
			record->mnc = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (MNC_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->mnc) {
			record->mnc = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & MOBILEBRAND) && (MOBILEBRAND_POSITION[database_type] != 0)) {
		if (!record->mobilebrand) {
			record->mobilebrand = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (MOBILEBRAND_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->mobilebrand) {
			record->mobilebrand = strdup(NOT_SUPPORTED);
		}
	}

	if ((mode & ELEVATION) && (ELEVATION_POSITION[database_type] != 0)) {
		char * mem = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (ELEVATION_POSITION[database_type] - 1)));
		record->elevation = (float) atof(mem);
		free(mem);
	} else {
		record->elevation = 0.0;
	}

	if ((mode & USAGETYPE) && (USAGETYPE_POSITION[database_type] != 0)) {
		if (!record->usagetype) {
			record->usagetype = IP2Location_readStr(handle, IP2Location_read32(handle, rowaddr + 4 * (USAGETYPE_POSITION[database_type] - 1)));
		}
	} else {
		if (!record->usagetype) {
			record->usagetype = strdup(NOT_SUPPORTED);
		}
	}

	// Create alias for the new variables
	record->zip_code = record->zipcode;
	record->time_zone = record->timezone;
	record->net_speed = record->netspeed;
	record->idd_code = record->iddcode;
	record->area_code = record->areacode;
	record->weather_station_code = record->weatherstationcode;
	record->weather_station_name = record->weatherstationname;
	record->mobile_brand = record->mobilebrand;
	record->usage_type = record->usagetype;

	return record;
}

// Get record for a IPv6 from database
IP2LocationRecord * IP2Location_get_ipv6_record(IP2Location *handler, uint32_t mode, ip_container parsed_ip) {
	FILE * handle = handler->file;
	uint32_t base_address = handler->ipv6_database_address;
	uint32_t database_column = handler->database_column;
	uint32_t ipv6_index_base_address = handler->ipv6_index_base_address;

	uint32_t low = 0;
	uint32_t high = handler->ipv6_database_count;
	uint32_t mid = 0;

	struct in6_addr ip_from;
	struct in6_addr ip_to;
	struct in6_addr ip_number;

	ip_number = parsed_ip.ipv6;

	if (!high) {
		return NULL;
	}

	if (ipv6_index_base_address > 0) {
		uint32_t ipnum1 = (ip_number.s6_addr[0] * 256) + ip_number.s6_addr[1];
		uint32_t indexpos = ipv6_index_base_address + (ipnum1 << 3);

		low = IP2Location_read32(handle, indexpos);
		high = IP2Location_read32(handle, indexpos + 4);
	}

	while (low <= high) {
		mid = (uint32_t)((low + high) >> 1);
		ip_from = IP2Location_readIPv6Address(handle, base_address + mid * (database_column * 4 + 12));
		ip_to = IP2Location_readIPv6Address(handle, base_address + (mid + 1) * (database_column * 4 + 12));

		if ((IP2Location_ipv6_compare(&ip_number, &ip_from) >= 0) && (IP2Location_ipv6_compare(&ip_number, &ip_to) < 0)) {
			return IP2Location_read_record(handler, base_address + mid * (database_column * 4 + 12) + 12, mode);
		} else {
			if (IP2Location_ipv6_compare(&ip_number, &ip_from) < 0) {
				high = mid - 1;
			} else {
				low = mid + 1;
			}
		}
	}

	return NULL;
}

// Get record for a IPv4 from database
IP2LocationRecord *IP2Location_get_ipv4_record(IP2Location *handler, uint32_t mode, ip_container parsed_ip) {
	FILE *handle = handler->file;
	uint32_t base_address = handler->ipv4_database_address;
	uint32_t database_column = handler->database_column;
	uint32_t ipv4_index_base_address = handler->ipv4_index_base_address;

	uint32_t low = 0;
	uint32_t high = handler->ipv4_database_count;
	uint32_t mid = 0;

	uint32_t ip_number;
	uint32_t ip_from;
	uint32_t ip_to;

	ip_number = parsed_ip.ipv4;

	if (ip_number == (uint32_t) MAX_IPV4_RANGE) {
		ip_number = ip_number - 1;
	}

	if (ipv4_index_base_address > 0) {
		uint32_t ipnum1n2 = (uint32_t) ip_number >> 16;
		uint32_t indexpos = ipv4_index_base_address + (ipnum1n2 << 3);

		low = IP2Location_read32(handle, indexpos);
		high = IP2Location_read32(handle, indexpos + 4);
	}

	while (low <= high) {
		mid = (uint32_t)((low + high) >> 1);
		ip_from = IP2Location_read32(handle, base_address + mid * database_column * 4);
		ip_to = IP2Location_read32(handle, base_address + (mid + 1) * database_column * 4);

		if ((ip_number >= ip_from) && (ip_number < ip_to)) {
			return IP2Location_read_record(handler, base_address + (mid * database_column * 4), mode);
		} else {
			if (ip_number < ip_from) {
				high = mid - 1;
			} else {
				low = mid + 1;
			}
		}
	}
	return NULL;
}

// Get the location data
static IP2LocationRecord *IP2Location_get_record(IP2Location *handler, char *ip, uint32_t mode) {
	ip_container parsed_ip = IP2Location_parse_address(ip);

	if (parsed_ip.version == 4) {
		return IP2Location_get_ipv4_record(handler, mode, parsed_ip);
	}
	if (parsed_ip.version == 6) {
		if (handler->ipv6_database_count == 0) {
			return IP2Location_bad_record(IPV6_ADDRESS_MISSING_IN_IPV4_BIN);
		}

		return IP2Location_get_ipv6_record(handler, mode, parsed_ip);
	} else {
		return IP2Location_bad_record(INVALID_IP_ADDRESS);
	}
}

// Initialize the record object
static IP2LocationRecord *IP2Location_new_record()
{
	IP2LocationRecord *record = (IP2LocationRecord *) calloc(1, sizeof(IP2LocationRecord));
	return record;
}

// Free the record object
void IP2Location_free_record(IP2LocationRecord *record) {
	if (record == NULL) {
		return;
	}

	free(record->city);
	free(record->country_long);
	free(record->country_short);
	free(record->domain);
	free(record->isp);
	free(record->region);
	free(record->zipcode);
	free(record->timezone);
	free(record->netspeed);
	free(record->iddcode);
	free(record->areacode);
	free(record->weatherstationcode);
	free(record->weatherstationname);
	free(record->mcc);
	free(record->mnc);
	free(record->mobilebrand);
	free(record->usagetype);

	free(record);
}

// Check if address is IPv4
static int IP2Location_is_ipv4(char *ip)
{
	struct sockaddr_in sa;
	return inet_pton(AF_INET, ip, &sa.sin_addr);
}

// Check if address is IPv6
static int IP2Location_is_ipv6(char *ip)
{
	struct in6_addr result;
	return inet_pton(AF_INET6, ip, &result);
}

// Get API version numeric (Will deprecate in coming major version update)
unsigned long int IP2Location_api_version_num(void)
{
	return (API_VERSION_NUMERIC);
}

// Alias to IP2Location_api_version_num()
unsigned long int IP2Location_api_version_number(void)
{
	return IP2Location_api_version_num();
}

// Get API version as string
char *IP2Location_api_version_string(void)
{
	static char version[64];
	sprintf(version, "%d.%d.%d", API_VERSION_MAJOR, API_VERSION_MINOR, API_VERSION_RELEASE);
	return (version);
}

// Get library version as string
char *IP2Location_lib_version_string(void)
{
	return (PACKAGE_VERSION);
}

// Get BIN database version
char *IP2Location_bin_version(IP2Location *handler)
{
	if (handler == NULL) {
		return NULL;
	}

	static char version[64];

	sprintf(version, "%d-%d-%d", handler->database_year + 2000, handler->database_month, handler->database_day);

	return (version);
}

// Set to use memory caching
int32_t IP2Location_DB_set_memory_cache(FILE *file)
{
	struct stat buffer;
	lookup_mode = IP2LOCATION_CACHE_MEMORY;

	if (fstat(fileno(file), &buffer) == -1) {
		lookup_mode = IP2LOCATION_FILE_IO;
		return -1;
	}

	if ((memory_pointer = malloc(buffer.st_size + 1)) == NULL) {
		lookup_mode = IP2LOCATION_FILE_IO;
		return -1;
	}

	if (IP2Location_load_database_into_memory(file, memory_pointer, buffer.st_size) == -1) {
		lookup_mode = IP2LOCATION_FILE_IO;
		free(memory_pointer);
		return -1;
	}

	return 0;
}

// Alias to IP2Location_DB_set_memory_cache()
int32_t IP2Location_set_memory_cache(FILE *file)
{
	return IP2Location_DB_set_memory_cache(file);
}

// Set to use shared memory
#if !defined(_WIN32) && !defined(MSDOS)
int32_t IP2Location_DB_set_shared_memory(FILE *file)
{
	struct stat buffer;
	int32_t is_dababase_loaded = 1;
	void *addr = (void*)MAP_ADDR;

	lookup_mode = IP2LOCATION_SHARED_MEMORY;

	// New shared memory object is created
	if ((shm_fd = shm_open(IP2LOCATION_SHM, O_RDWR | O_CREAT | O_EXCL, 0777)) != -1) {
		is_dababase_loaded = 0;
	}

	// Failed to create new shared memory object
	else if ((shm_fd = shm_open(IP2LOCATION_SHM, O_RDWR , 0777)) == -1) {
		lookup_mode = IP2LOCATION_FILE_IO;
		return -1;
	}

	if (fstat(fileno(file), &buffer) == -1) {
		close(shm_fd);

		if (is_dababase_loaded == 0) {
			shm_unlink(IP2LOCATION_SHM);
		}

		lookup_mode = IP2LOCATION_FILE_IO;

		return -1;
	}

	if (is_dababase_loaded == 0 && ftruncate(shm_fd, buffer.st_size + 1) == -1) {
		close(shm_fd);
		shm_unlink(IP2LOCATION_SHM);
		lookup_mode = IP2LOCATION_FILE_IO;
		return -1;
	}

	memory_pointer = mmap(addr, buffer.st_size + 1, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

	if (memory_pointer == (void *) -1) {
		close(shm_fd);

		if (is_dababase_loaded == 0) {
			shm_unlink(IP2LOCATION_SHM);
		}

		lookup_mode = IP2LOCATION_FILE_IO;

		return -1;
	}

	if (is_dababase_loaded == 0) {
		if (IP2Location_load_database_into_memory(file, memory_pointer, buffer.st_size) == -1) {
			munmap(memory_pointer, buffer.st_size);
			close(shm_fd);
			shm_unlink(IP2LOCATION_SHM);
			lookup_mode = IP2LOCATION_FILE_IO;
			return -1;
		}
	}

	return 0;
}

#elif defined(_WIN32)

// Return a name for the shared-memory object
// For Windows, this depends on whether 'UNICODE' is defined
// (hence the use of 'TCHAR').
static const TCHAR *get_shm_name(void) {
	static TCHAR name[64] = _T("");

	if (!name[0]) {
		_sntprintf(name, sizeof(name)/sizeof(name[0]), _T("%s_%lu"),
                   _T(IP2LOCATION_SHM), GetProcessId(NULL));
	}
	return name;
}

int32_t IP2Location_DB_set_shared_memory(FILE *file)
{
	struct stat buffer;
	int32_t is_dababase_loaded = 1;

	lookup_mode = IP2LOCATION_SHARED_MEMORY;

	if (fstat(fileno(file), &buffer) == -1) {
		lookup_mode = IP2LOCATION_FILE_IO;
		return -1;
	}

	shm_fd = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, buffer.st_size + 1, get_shm_name());

	if (shm_fd == NULL) {
		lookup_mode = IP2LOCATION_FILE_IO;
		return -1;
	}

	is_dababase_loaded = (GetLastError() == ERROR_ALREADY_EXISTS);
	memory_pointer = MapViewOfFile(shm_fd, FILE_MAP_WRITE, 0, 0, 0);

	if (memory_pointer == NULL) {
		UnmapViewOfFile(memory_pointer);
		lookup_mode = IP2LOCATION_FILE_IO;
		return -1;
	}

	if (is_dababase_loaded == 0) {
		if (IP2Location_load_database_into_memory(file, memory_pointer, buffer.st_size) == -1) {
			UnmapViewOfFile(memory_pointer);
			CloseHandle(shm_fd);
			lookup_mode = IP2LOCATION_FILE_IO;
			return -1;
		}
	}

	return 0;
}

#else /* MSDOS etc. */
int32_t IP2Location_DB_set_shared_memory(FILE *file)
{
	lookup_mode = IP2LOCATION_FILE_IO;
	(void) file;
	return -1;
}
#endif

// Alias to IP2Location_DB_set_shared_memory()
int32_t IP2Location_set_shared_memory(FILE *file)
{
	return IP2Location_DB_set_shared_memory(file);
}

// Load BIN file into memory
int32_t IP2Location_load_database_into_memory(FILE *file, void *memory, int64_t size)
{
	fseek(file, SEEK_SET, 0);

	if (fread(memory, (size_t)size, 1, file) != 1) {
		return -1;
	}

	return 0;
}

// Close the corresponding memory, based on the opened option
int32_t IP2Location_DB_close(FILE *file)
{
	if (lookup_mode == IP2LOCATION_CACHE_MEMORY) {
		if (memory_pointer != NULL) {
			free(memory_pointer);
		}
	} else if (lookup_mode == IP2LOCATION_SHARED_MEMORY) {
		if (memory_pointer != NULL) {
#if !defined(_WIN32) && !defined(MSDOS)
			struct stat buffer;

			if (fstat(fileno(file), &buffer) == 0) {
				munmap(memory_pointer, buffer.st_size);
			}

			close(shm_fd);
#elif defined(_WIN32)
			UnmapViewOfFile(memory_pointer);
			CloseHandle(shm_fd);
#endif
		}
	}

	if (file != NULL) {
		fclose(file);
	}

	lookup_mode = IP2LOCATION_FILE_IO;
	return 0;
}


// Alias to IP2Location_DB_close
int32_t IP2Location_close_memory(FILE *file)
{
	return IP2Location_DB_close(file);
}

// Remove shared memory object
#if defined(_WIN32) || defined(MSDOS)
void IP2Location_DB_del_shm()
{
}
#else
void IP2Location_DB_del_shm()
{
	shm_unlink(IP2LOCATION_SHM);
}
#endif

struct in6_addr IP2Location_readIPv6Address(FILE *handle, uint32_t position)
{
	int i, j;
	struct in6_addr addr6;

	for (i = 0, j = 15; i < 16; i++, j--) {
		addr6.s6_addr[i] = IP2Location_read8(handle, position + j);
	}

	return addr6;
}

// Alias to IP2Location_readIPv6Address()
struct in6_addr IP2Location_read_ipv6_address(FILE *handle, uint32_t position)
{
	return IP2Location_readIPv6Address(handle, position);
}

uint32_t IP2Location_read32(FILE *handle, uint32_t position)
{
	uint8_t byte1 = 0;
	uint8_t byte2 = 0;
	uint8_t byte3 = 0;
	uint8_t byte4 = 0;
	uint8_t *cache_shm = memory_pointer;
	size_t temp;

	// Read from file
	if (lookup_mode == IP2LOCATION_FILE_IO && handle != NULL) {
		fseek(handle, position - 1, 0);
		temp = fread(&byte1, 1, 1, handle);

		if (temp == 0) {
			return 0;
		}

		temp = fread(&byte2, 1, 1, handle);

		if (temp == 0) {
			return 0;
		}

		temp = fread(&byte3, 1, 1, handle);

		if (temp == 0) {
			return 0;
		}

		temp = fread(&byte4, 1, 1, handle);

		if (temp == 0) {
			return 0;
		}
	} else {
		byte1 = cache_shm[position - 1];
		byte2 = cache_shm[position];
		byte3 = cache_shm[position + 1];
		byte4 = cache_shm[position + 2];
	}

	return ((byte4 << 24) | (byte3 << 16) | (byte2 << 8) | (byte1));
}

uint8_t IP2Location_read8(FILE *handle, uint32_t position)
{
	uint8_t ret = 0;
	uint8_t *cache_shm = memory_pointer;
	size_t temp;

	if (lookup_mode == IP2LOCATION_FILE_IO && handle != NULL) {
		fseek(handle, position - 1, 0);
		temp = fread(&ret, 1, 1, handle);

		if (temp == 0) {
			return 0;
		}
	} else {
		ret = cache_shm[position - 1];
	}

	return ret;
}

char *IP2Location_readStr(FILE *handle, uint32_t position)
{
	uint8_t size = 0;
	char *str = 0;
	uint8_t *cache_shm = memory_pointer;
	size_t temp;

	if (lookup_mode == IP2LOCATION_FILE_IO && handle != NULL) {
		fseek(handle, position, 0);
		temp = fread(&size, 1, 1, handle);

		if (temp == 0) {
			return strdup("");
		}

		str = (char *)malloc(size+1);
		memset(str, 0, size+1);

		temp = fread(str, size, 1, handle);

		if (temp == 0) {
			free(str);
			return strdup("");
		}
	} else {
		size = cache_shm[position];
		str = (char *)malloc(size + 1);
		memset(str, 0, size + 1);
		memcpy((void*) str, (void*)&cache_shm[position + 1], size);
	}

	return str;
}

// Alias to IP2Location_readStr()
char *IP2Location_read_string(FILE *handle, uint32_t position)
{
	return IP2Location_readStr(handle, position);
}

float IP2Location_readFloat(FILE *handle, uint32_t position)
{
	float ret = 0.0;
	uint8_t *cache_shm = memory_pointer;
	size_t temp;

#if defined(_SUN_) || defined(__powerpc__) || defined(__ppc__) || defined(__ppc64__) || defined(__powerpc64__)
	char *p = (char *) &ret;

	// for SUN SPARC, have to reverse the byte order
	if (lookup_mode == IP2LOCATION_FILE_IO && handle != NULL) {
		fseek(handle, position - 1, 0);

		temp = fread(p + 3, 1, 1, handle);

		if (temp == 0) {
			return 0.0;
		}

		temp = fread(p + 2, 1, 1, handle);

		if (temp == 0) {
			return 0.0;
		}

		temp = fread(p + 1, 1, 1, handle);

		if (temp == 0) {
			return 0.0;
		}

		temp = fread(p, 1, 1, handle);

		if (temp == 0) {
			return 0.0;
		}
	} else {
		*(p+3) = cache_shm[position - 1];
		*(p+2) = cache_shm[position];
		*(p+1) = cache_shm[position + 1];
		*(p) = cache_shm[position + 2];
	}
#else
	if (lookup_mode == IP2LOCATION_FILE_IO && handle != NULL) {
		fseek(handle, position - 1, 0);
		temp = fread(&ret, 4, 1, handle);

		if (temp == 0) {
			return 0.0;
		}
	} else {
		memcpy((void*) &ret, (void*)&cache_shm[position - 1], 4);
	}
#endif
	return ret;
}

// Alias to IP2Location_readFloat()
float IP2Location_read_float(FILE *handle, uint32_t position)
{
	return IP2Location_readFloat(handle, position);
}
#endif  /* USE_IP2LOCATION */
